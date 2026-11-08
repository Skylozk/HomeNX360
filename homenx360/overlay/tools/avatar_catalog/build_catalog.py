#!/usr/bin/env python3
"""Build selected, independently wearable native assets from original avatar files.

Requires the pinned offline avatarextract converter prepared by
prepare_converter.py, Python/numpy/Pillow, and the original .toc files. Never
executes firmware or programs supplied in an asset archive.
"""
import argparse, hashlib, io, json, math, os, shutil, struct, subprocess, tempfile, zlib
from pathlib import Path
import numpy as np
from PIL import Image

TAIL=bytes.fromhex('c1c8f109a19cb2e0')
# UI category order. Hair companions remain in the asset directory but not the list.
GROUPS=[('hair',4),('tops',8),('bottoms',16),('shoes',32),('hats',64),
 ('gloves',128),('glasses',256),('wristwear',512),('earrings',1024),('rings',2048),
 ('eyes',8192),('brows',16384),('mouths',32768),('facial_hair',65536),
 ('face_paint',131072),('eye_shadow',262144),('noses',524288),('chins',1048576),('ears',2097152),('props',4096)]
FACE={8192:(1,'eyes'),16384:(2,'brows'),32768:(0,'mouth'),65536:(3,'facial'),131072:(5,'base'),262144:(4,'shadow')}
SHAPES={524288:0,1048576:1,2097152:2}

def toc(path):
 b=path.read_bytes();assert b[:16].hex()=='580a07d64bcd40bebf37254be826fb0b'
 count=struct.unpack_from('>I',b,28)[0];rows=[]
 for i in range(count):
  h=b[0x30+i*0x10c:0x30+(i+1)*0x10c];cat=struct.unpack_from('>I',h)[0];start,size=struct.unpack_from('>II',h,0x104);assert start+size<=len(b)
  n=struct.unpack_from('>I',h,0xc0)[0];end=n
  while n and b[end:end+2]!=b'\0\0':end+=2
  rows.append(dict(key=str(i),index=i,category=cat,bodies=h[4],name=b[n:end].decode('utf-16-be') if n else '',partner=struct.unpack_from('>H',h,0xa0)[0],offset=start,size=size))
 return rows

def asset_id(row,gender):
 if 'guid' in row:return bytes.fromhex(row['guid'].replace('-',''))
 return struct.pack('>IHH8s',row['category'],row['index'],row['bodies'],TAIL)

def manifest(rows,gender,item=None,clothed=False,white=True):
 b=bytearray(1000);struct.pack_into('>Iff',b,0,1,.5,.5)
 def comp(off,row):b[off:off+16]=asset_id(row,gender);struct.pack_into('>H',b,off+16,row['category']&8191)
 comp(0x120,next(r for r in rows if r['category']==2 and r['bodies']==gender))
 comp(0x140,next(r for r in rows if r['category']==1))
 colors=[0xffffffff,0xffffffff,0xffb97762,0xff487454,0xff392514,0xff000000,0xff392514,0xffa87956,0xffa87956] if white else [0xffedb582,0xff392514,0xffb97762,0xff487454,0xff392514,0xff000000,0xff392514,0xffa87956,0xffa87956]
 for i,c in enumerate(colors):struct.pack_into('>I',b,0xfc+i*4,c)
 # These defaults are only used for complete imported outfits / prop performances.
 if clothed:
  for k,c in enumerate([4,8,16,32]):comp(0x160+k*32,next(r for r in rows if r['category']==c and r['bodies']&gender and '(Hat)' not in r['name']))
 if item:
  c=item['category']
  if c in FACE:
   slot,_=FACE[c];off=0x3c+slot*32;b[off:off+16]=asset_id(item,gender);struct.pack_into('>f',b,off+16,1)
  elif c in SHAPES:b[0xc+SHAPES[c]*16:0x1c+SHAPES[c]*16]=asset_id(item,gender)
  else:
   if clothed:
    for k in range(13):
     off=0x160+k*32
     if struct.unpack_from('>H',b,off+16)[0] & c:b[off:off+32]=bytes(32)
   comp(0x160+(12 if clothed else 0)*32,item)
 return b

def mat(t,q,s):
 length=math.sqrt(sum(v*v for v in q));q=[v/length for v in q] if length>0 else [0,0,0,1]
 x,y,z,w=q
 return np.array([[(1-2*y*y-2*z*z)*s[0],(2*x*y-2*z*w)*s[1],(2*x*z+2*y*w)*s[2],t[0]],[(2*x*y+2*z*w)*s[0],(1-2*x*x-2*z*z)*s[1],(2*y*z-2*x*w)*s[2],t[1]],[(2*x*z-2*y*w)*s[0],(2*y*z+2*x*w)*s[1],(1-2*x*x-2*y*y)*s[2],t[2]],[0,0,0,1]],dtype=np.float64)
def namehash(s):
 h=2166136261
 for v in s.encode():h=((h^v)*16777619)&0xffffffff
 return h

def scene_matrices(skeleton,clip=None,frame=0,prop=False):
 tracks={x['joint']:x for x in (clip.get('carryable_tracks' if prop else 'tracks',[]) if clip else [])};out=[]
 for i,j in enumerate(skeleton['joints']):
  track=tracks.get(i);idx=lambda key,default:track[key][min(frame,len(track[key])-1)] if track and track.get(key) else default
  translation=idx('t',j['rest_local'])
  if prop and track and track.get('t'):translation=[a+b for a,b in zip(j['rest_local'],translation)]
  local=mat(translation,idx('r',[0,0,0,1]),idx('s',[1,1,1]))
  # Exported animation tracks are absolute LOCAL transforms. Parent order is topological.
  out.append(out[j['parent']]@local if j['parent']>=0 else local)
 return out

def png_bytes(path):
 im=Image.open(path).convert('RGBA');im.thumbnail((384,384),Image.Resampling.LANCZOS)
 out=io.BytesIO();im.save(out,format='PNG',optimize=True);return out.getvalue()

def bank(scene,directory,out,meshes=None,clips=False):
 meshes=scene['meshes'] if meshes is None else meshes
 if not meshes:return None
 skeleton=scene['skeleton'];joints=skeleton['joints'];prop=scene.get('prop_skeleton');allj=joints+(prop['joints'] if prop else [])
 keep=list(range(len(joints)))
 if prop:
  used=set()
  for mesh in meshes:
   if mesh['is_prop']:
    for j,w in zip(mesh['joints'],mesh['weights']):
     if w>0:used.add(j)
  keep += [len(joints)+j for j in sorted(used)]
 remap={old:new for new,old in enumerate(keep)}
 if len(keep)>256:raise ValueError('too many weighted joints')
 # Baked vertices are in rest world space. Inverse bind uses that same pose.
 bind=[mat(j['rest_world'],j['rest_world_rot'],j.get('scale',[1,1,1])) for j in allj]
 mats=[];images=[];parts=[]
 for mesh in meshes:
  m=scene['materials'][mesh['material']]
  if mesh['material'] not in mats:
   mats.append(mesh['material']);images.append(png_bytes(directory/m['diffuse']))
  comp=scene['components'][mesh['component']] if mesh['component']<len(scene['components']) else {'categories':4096}
  category=comp['categories'];slot=0 if category==2 else 1 if category==1 else 2 if category==4 else 4 if mesh['is_prop'] else 3
  blend=int(m['shader'] in [1,3]);nv=mesh['vertex_count'];vertices=bytearray()
  for i in range(nv):
   weights=mesh['weights'][i*4:i*4+4];total=sum(weights)
   if total<.0001:weights=[1.,0.,0.,0.]
   else:weights=[v/total for v in weights]
   js=[remap[v+(len(joints) if mesh['is_prop'] else 0)] if weights[k]>0 else 0 for k,v in enumerate(mesh['joints'][i*4:i*4+4])]
   vertices+=struct.pack('<9f4B',*mesh['positions'][i*3:i*3+3],*mesh['uv'][i*2:i*2+2],*weights,*js)
  lighting=bytearray()
  for i in range(nv):lighting+=struct.pack('<3f4B',*mesh['normals'][i*3:i*3+3],*mesh['colors'][i*4:i*4+4])
  parts.append((slot,mats.index(mesh['material']),vertices,mesh['indices'],blend,lighting))
 out.parent.mkdir(parents=True,exist_ok=True)
 with io.BytesIO() as f:
  f.write(struct.pack('<8s4I',b'NXAVTR3\0',len(keep),len(parts),len(images),2 if clips else 0))
  for i in keep:
   j,m=allj[i],bind[i]
   f.write(struct.pack('<I16f',namehash(('PROP_' if i>=len(joints) else '')+j['name']),*np.linalg.inv(m).T.flatten()))
  for slot,texture,vertices,indices,blend,lighting in parts:
   f.write(struct.pack('<5I',slot,texture,len(vertices)//40,len(indices),blend));f.write(vertices);f.write(lighting);f.write(struct.pack('<'+'I'*len(indices),*indices))
  for b in images:f.write(struct.pack('<I',len(b)));f.write(b)
  if clips:
   idle=next((a for a in scene['animations'] if a['name'].startswith('Carryable_')),None) or next(a for a in scene['animations'] if 'Stand 0' in a['name'])
   wave=next(a for a in scene['animations'] if 'Wave' in a['name'])
   for cid,clip in enumerate([idle,wave]):
    dur=clip['duration'];count=min(901,max(2,math.ceil(dur*15)+1));f.write(struct.pack('<IIf',cid,count,dur))
    for frame in range(count):
     original=round(frame*(clip['frame_count']-1)/(count-1));world=scene_matrices(skeleton,clip,original)
     if prop:
      pw=scene_matrices(prop,clip,original,True)
      # Carryable positions in the scene already use the carryable animation's world root.
      world+=pw
     for i in keep:
      matrix=world[i][:3,:].flatten()
      # Some original effects park inactive debris more than a billion metres
      # off-screen. Collapse that hidden bone instead of overflowing binary16.
      if not np.all(np.isfinite(matrix)):raise ValueError('non-finite pose')
      if np.max(np.abs(matrix))>10000:matrix=np.zeros(12)
      f.write(struct.pack('<12e',*matrix))
  raw=f.getvalue()
 if len(raw)>8*1024*1024:raise ValueError('decoded bank exceeds 8 MiB')
 packed=struct.pack('<8sI',b'NXAZIP1\0',len(raw))+zlib.compress(raw,9)
 temporary=out.with_suffix('.building');temporary.write_bytes(packed);os.replace(temporary,out)
 return dict(vertices=sum(len(p[2])//40 for p in parts),indices=sum(len(p[3]) for p in parts),parts=len(parts),bytes=len(packed),decoded_bytes=len(raw),joints=len(keep))

def bank_stats(path):
 packed=path.read_bytes();raw=zlib.decompress(packed[12:]) if packed[:8]==b'NXAZIP1\0' else packed
 magic,joints,parts,images,clips=struct.unpack_from('<8s4I',raw);at=24+joints*68;vertices=indices=0
 for i in range(parts):
  slot,texture,nv,ni,blend=struct.unpack_from('<5I',raw,at);at+=20+nv*56+ni*4;vertices+=nv;indices+=ni
 return dict(vertices=vertices,indices=indices,parts=parts,bytes=len(packed),decoded_bytes=len(raw),joints=joints)

def patch(base,current,out):
 records=[];hidden=[]
 for part,(a,b) in enumerate(zip(base['meshes'],current['meshes'])):
  if a['component']>1:continue
  if a['name']!=b['name'] or a['vertex_count']!=b['vertex_count'] or len(a['indices'])!=len(b['indices']):raise ValueError('base topology changed')
  for i in range(a['vertex_count']):
   p=np.array(b['positions'][i*3:i*3+3])-a['positions'][i*3:i*3+3]
   if np.max(np.abs(p))>1e-6:records.append((part,i,*p))
  for i in range(0,len(a['indices']),3):
   x,y,z=b['indices'][i:i+3]
   if x==y==z and len(set(a['indices'][i:i+3]))==3:hidden.append((part,i))
 if not records and not hidden:return False
 with out.open('wb') as f:
  f.write(struct.pack('<8sII',b'NXAPTC1\0',len(records),len(hidden)))
  for r in records:f.write(struct.pack('<II3f',*r))
  for r in hidden:f.write(struct.pack('<II',*r))
 return True

def bake(args,rows,gender,item,out,clips=False,clothed=False,white=True):
 mf=out.parent/'current.bin';mf.write_bytes(manifest(rows,gender,item,clothed,white));shutil.rmtree(out,ignore_errors=True);out.mkdir()
 cmd=[str(args.converter),'--avatar',str(mf),str(out),'--toc',str(args.toc),'--legacy-toc',str(args.legacy),'--no-scale','--bake-size','64' if getattr(args,'patch_only',False) else '256']
 if args.closet:cmd+=['--closet',str(args.closet)]
 if not item or item['category']!=4096:cmd+=['--no-prop']
 if clips:cmd+=['--pack-anim','Animation Generic Stand 0','--pack-anim','Animation Generic Wave']
 env=dict(os.environ)
 if not getattr(args,'patch_only',False):env['NXE_CATALOG_MAPS']='1'
 p=subprocess.run(cmd,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
 if p.returncode or not (out/'avatar.json').exists():raise RuntimeError(p.stdout[-3000:])
 return json.loads((out/'avatar.json').read_text())

def main(args):
 rows=toc(args.toc);args.output.mkdir(parents=True,exist_ok=True);out=args.output;summary={'version':1,'toc_sha256':hashlib.sha256(args.toc.read_bytes()).hexdigest(),'items':[],'failures':[]}
 categories={v for k,v in GROUPS};entries=[r for r in rows if r['category'] in categories or (r['category']&0x1fff and r['category'] not in [1,2] and not r['category']&4096)]
 if args.marketplace_only:entries=[]
 if args.closet:
  for line in (args.closet/'closet_index.tsv').read_text(encoding='utf-8-sig').splitlines():
   col=line.split('\t')
   if len(col)<4 or col[0].startswith('#') or col[0]=='guid':continue
   try:
    category=int(col[1],16);body=int(col[2]);key='m-'+col[0].replace('-','')
    entries.append(dict(key=key,guid=col[0],category=category,bodies=body,name=col[3],partner=0,marketplace=True))
   except ValueError:continue
 with tempfile.TemporaryDirectory(prefix='nxe-catalog-') as tmp:
  tmp=Path(tmp);bases={}
  for gender in [1,2]:
   s=bake(args,rows,gender,None,tmp/'bake',clips=True);bases[gender]=s
   bank(s,tmp/'bake',out/f'base-{gender}.avb',clips=True)
   for i in range(2):shutil.copy2(tmp/'bake'/f'channel_{i}_base.png',out/f'base-face-{i}.png')
  for n,item in enumerate(entries):
   try:
    c=item['category'];key=item['key'];entry={k:v for k,v in item.items() if k not in ['offset','size']};stats=[]
    for gender in [1,2]:
     if not item['bodies']&gender:continue
     # Meshes and feature masks are shared when the pack marks them unisex;
     # body hiding patches still have a distinct male/female target.
     s=bake(args,rows,gender,item,tmp/'bake',clips=c==4096,clothed=c==4096,white=c!=4096)
     if c in FACE:
      _,channel=FACE[c]
      for i in range(2):shutil.copy2(tmp/'bake'/f'channel_{i}_{channel}.png',out/f'{key}-face-{i}.png')
     elif c==4096:
      stats.append(bank(s,tmp/'bake',out/f'{key}-prop-{gender}.avb',meshes=[m for m in s['meshes'] if m['is_prop']],clips=True))
     elif c not in SHAPES:
      meshes=[m for m in s['meshes'] if s['components'][m['component']]['categories'] not in [1,2]]
      result=bank(s,tmp/'bake',out/f'{key}.avb',meshes=meshes)
      if result:stats.append(result)
      elif not item['name'].startswith('Bald'):raise ValueError('source item has no decodable mesh')
     if c!=4096:patch(bases[gender],s,out/f'{key}-{gender}.avp')
    entry['stats']=stats;summary['items'].append(entry)
   except Exception as e:
    summary['failures'].append({'key':item['key'],'name':item['name'],'reason':str(e)})
   if n%25==0:print(n,'/',len(entries),item['name'],'failures',len(summary['failures']),flush=True)
   (out/'catalog.json').write_text(json.dumps(summary,indent=2)+'\n')
 print('Converted',len(summary['items']),'failed',len(summary['failures']),flush=True)
 if summary['failures']:print(summary['failures'][:8])
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--converter',type=Path,required=True);p.add_argument('--toc',type=Path,required=True);p.add_argument('--legacy',type=Path,required=True);p.add_argument('--closet',type=Path);p.add_argument('--marketplace-only',action='store_true');p.add_argument('--output',type=Path,required=True);main(p.parse_args())
