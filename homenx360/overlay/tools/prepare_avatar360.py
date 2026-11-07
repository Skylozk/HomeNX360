#!/usr/bin/env python3
"""Convert a public 360sona GLB export into the bounded native avatar format.

Original meshes, skin weights, inverse bind matrices and baked surface colours
are retained. Only textures (384 px maximum) and animation sampling (15 Hz) are
reduced for the Switch HOME memory budget. No network access is performed.
"""
import argparse, bisect, hashlib, io, json, math, struct
from pathlib import Path
from PIL import Image

IDENTITY=[1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.]
def mul(a,b):return [sum(a[k*4+r]*b[c*4+k] for k in range(4)) for c in range(4) for r in range(4)]
def trs(t,q,s):
    x,y,z,w=q
    return [(1-2*y*y-2*z*z)*s[0],(2*x*y+2*z*w)*s[0],(2*x*z-2*y*w)*s[0],0.,
            (2*x*y-2*z*w)*s[1],(1-2*x*x-2*z*z)*s[1],(2*y*z+2*x*w)*s[1],0.,
            (2*x*z+2*y*w)*s[2],(2*y*z-2*x*w)*s[2],(1-2*x*x-2*y*y)*s[2],0.,*t,1.]
def slerp(a,b,t):
    d=sum(x*y for x,y in zip(a,b))
    if d<0:b=[-x for x in b];d=-d
    if d>.9995:r=[x+(y-x)*t for x,y in zip(a,b)]
    else:
        theta=math.acos(max(-1.,min(1.,d)));sn=math.sin(theta)
        r=[(x*math.sin((1-t)*theta)+y*math.sin(t*theta))/sn for x,y in zip(a,b)]
    length=math.sqrt(sum(x*x for x in r));return [x/length for x in r]
def namehash(name):
    h=2166136261
    for x in name.encode():h=((h^x)*16777619)&0xffffffff
    return h
def convert(src,dest):
    blob=src.read_bytes()
    if struct.unpack_from('<4sII',blob)!=(b'glTF',2,len(blob)):raise ValueError('Invalid GLB')
    size,tag=struct.unpack_from('<I4s',blob,12);doc=json.loads(blob[20:20+size])
    pos=20+size;length,tag=struct.unpack_from('<I4s',blob,pos)
    if tag!=b'BIN\0':raise ValueError('Expected embedded BIN chunk')
    data=blob[pos+8:pos+8+length]
    def view(i):
        v=doc['bufferViews'][i];off=v.get('byteOffset',0);return data[off:off+v['byteLength']]
    cache={}
    def acc(i):
        if i in cache:return cache[i]
        a=doc['accessors'][i];v=doc['bufferViews'][a['bufferView']]
        if 'sparse' in a:raise ValueError('Sparse accessors unsupported')
        n={'SCALAR':1,'VEC2':2,'VEC3':3,'VEC4':4,'MAT4':16}[a['type']]
        code={5121:'B',5123:'H',5125:'I',5126:'f'}[a['componentType']]
        fmt='<'+code*n;stride=v.get('byteStride',struct.calcsize(fmt));off=v.get('byteOffset',0)+a.get('byteOffset',0)
        result=[struct.unpack_from(fmt,data,off+j*stride) for j in range(a['count'])]
        cache[i]=result;return result
    skin=doc['skins'][0];joints=skin['joints'];nodes=doc['nodes']
    if len(doc['skins'])!=1 or not 1<=len(joints)<=128:raise ValueError('Unsupported rig')
    parents={c:i for i,n in enumerate(nodes) for c in n.get('children',[])}
    def worlds(animation,time):
        values={}
        if animation:
            for ch in animation['channels']:
                target=ch['target'];sam=animation['samplers'][ch['sampler']]
                keys=[v[0] for v in acc(sam['input'])];vals=acc(sam['output'])
                lo=max(0,min(len(keys)-1,bisect.bisect_right(keys,time)-1));hi=min(lo+1,len(keys)-1)
                u=0 if hi==lo else max(0,min(1,(time-keys[lo])/(keys[hi]-keys[lo])))
                interp=sam.get('interpolation','LINEAR')
                if interp=='STEP':v=vals[lo]
                elif interp=='LINEAR':v=slerp(vals[lo],vals[hi],u) if target['path']=='rotation' else [a+(b-a)*u for a,b in zip(vals[lo],vals[hi])]
                else:raise ValueError('Unsupported interpolation')
                values[target['node'],target['path']]=v
        out={}
        def world(i):
            if i in out:return out[i]
            n=nodes[i];m=n.get('matrix')
            if m is None:m=trs(values.get((i,'translation'),n.get('translation',[0,0,0])),values.get((i,'rotation'),n.get('rotation',[0,0,0,1])),values.get((i,'scale'),n.get('scale',[1,1,1])))
            out[i]=mul(world(parents[i]),m) if i in parents else m
            return out[i]
        return [world(j) for j in joints]
    parts=[];materials=doc['materials'];images=[];image_lookup={}
    for node in nodes:
        if 'mesh' not in node:continue
        if node.get('skin')!=0:raise ValueError('Expected skinned mesh')
        name=node.get('name','').lower()
        slot=0 if name=='body' else 1 if name=='head' else 2 if name.startswith('hair:') else 3 if name.startswith('shirt:') else 4 if name.startswith('trousers:') else 5 if name.startswith('shoes:') else 7 if name.startswith('headwear:') else 6
        for prim in doc['meshes'][node['mesh']]['primitives']:
            if prim.get('mode',4)!=4:raise ValueError('Expected triangles')
            mat=materials[prim['material']]
            mode=mat.get('alphaMode','OPAQUE')
            if mode not in ('OPAQUE','MASK','BLEND'):raise ValueError('Unsupported material')
            pbr=mat['pbrMetallicRoughness'];factor=pbr.get('baseColorFactor',[1,1,1,1])
            image=doc['textures'][pbr['baseColorTexture']['index']]['source']
            key=(image,tuple(factor),mode,mat.get('alphaCutoff',.5))
            if key not in image_lookup:
                image_lookup[key]=len(images)
                im=Image.open(io.BytesIO(view(doc['images'][image]['bufferView']))).convert('RGBA');im.thumbnail((384,384),Image.Resampling.LANCZOS)
                channels=list(im.split())
                for channel in range(4):channels[channel]=channels[channel].point(lambda x,k=factor[channel]:round(x*k))
                if mode=='OPAQUE':channels[3]=channels[3].point(lambda _:255)
                elif mode=='MASK':channels[3]=channels[3].point(lambda x:255 if x>=mat.get('alphaCutoff',.5)*255 else 0)
                im=Image.merge('RGBA',channels)
                out=io.BytesIO();im.save(out,format='PNG',optimize=True);images.append(out.getvalue())
            attrs=prim['attributes'];ps=acc(attrs['POSITION']);uv=acc(attrs['TEXCOORD_0']);js=acc(attrs['JOINTS_0']);ws=acc(attrs['WEIGHTS_0']);idx=[x[0] for x in acc(prim['indices'])]
            if not len(idx)%3==0 or max(idx)>=len(ps):raise ValueError('Invalid index')
            vertices=bytearray()
            for p,u,j,w in zip(ps,uv,js,ws):
                if any(v>=len(joints) for v in j) or abs(sum(w)-1)>.002 or not all(math.isfinite(x) for x in (*p,*u,*w)):raise ValueError('Invalid vertex')
                vertices+=struct.pack('<9f4B',*p,*u,*w,*j)
            parts.append((slot,image_lookup[key],vertices,idx,mode=='BLEND'))
    animations=doc.get('animations',[])
    idle=next((x for x in animations if x['name']=='Just... standing around'),None)
    wave=next((x for x in animations if x['name']=='Hello there!'),None)
    if idle is None or wave is None:raise ValueError('Export idle and wave animations')
    dest.parent.mkdir(parents=True,exist_ok=True)
    with dest.open('wb') as f:
        f.write(struct.pack('<8s4I',b'NXAVTR1\0',len(joints),len(parts),len(images),2))
        for j,m in zip(joints,acc(skin['inverseBindMatrices'])):f.write(struct.pack('<I16f',namehash(nodes[j]['name']),*m))
        for slot,texture,vertices,indices,blend in parts:
            f.write(struct.pack('<5I',slot,texture,len(vertices)//40,len(indices),int(blend)));f.write(vertices);f.write(struct.pack('<'+'I'*len(indices),*indices))
        for image in images:f.write(struct.pack('<I',len(image)));f.write(image)
        clip_info=[]
        for clip_id,clip in enumerate((idle,wave)):
            duration=max(acc(s['input'])[-1][0] for s in clip['samplers']);frames=math.ceil(duration*15)+1
            if not 0<duration<=60:raise ValueError('Unexpected animation duration')
            f.write(struct.pack('<IIf',clip_id,frames,duration))
            for frame in range(frames):
                for m in worlds(clip,duration*frame/(frames-1)):f.write(struct.pack('<12f',*[m[c*4+r] for r in range(3) for c in range(4)]))
            clip_info.append({'name':clip['name'],'duration':duration,'frames':frames})
    report={'source':src.name,'source_sha256':hashlib.sha256(blob).hexdigest(),'bank_sha256':hashlib.sha256(dest.read_bytes()).hexdigest(),'joints':len(joints),'parts':len(parts),'vertices':sum(len(x[2])//40 for x in parts),'triangles':sum(len(x[3])//3 for x in parts),'textures':len(images),'texture_max_side':384,'animations':clip_info,'artwork':'Microsoft; exported through the public 360sona editor demo','export_generator':doc['asset']}
    dest.with_suffix('.json').write_text(json.dumps(report,indent=2)+'\n')
    print(dest.name,dest.stat().st_size,'bytes;',report['vertices'],'vertices;',report['triangles'],'triangles')
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('source',type=Path);p.add_argument('output',type=Path);a=p.parse_args();convert(a.source,a.output)
