#!/usr/bin/env python3
"""Build six original-style avatar banks with complete underlying body geometry.

Requires the unchanged clothed AND complete-body public editor exports in
media/avatars. Original clothing-fitted body meshes avoid replacement-part
intersections. Body UV transfer retains the selected starter's skin appearance;
complete calf geometry repairs shorts. The native depth buffer occludes skin
covered by clothing. Requires Pillow and NumPy.
"""
import hashlib,json,struct,tempfile
import numpy as np
from pathlib import Path
from prepare_avatar360 import convert
ROOT=Path(__file__).resolve().parents[1]
NAMES=['regular','player','fan','street','smart','night']
def read(path):
    data=path.read_bytes();magic,nj,np,nt,nc=struct.unpack_from('<8s4I',data);pos=24
    if magic!=b'NXAVTR1\0':raise ValueError('Invalid native bank')
    rig=data[pos:pos+68*nj];pos+=68*nj;parts=[];images=[]
    for _ in range(np):
        slot,texture,nv,ni,blend=struct.unpack_from('<5I',data,pos);pos+=20
        vertices=[data[pos+i*40:pos+(i+1)*40] for i in range(nv)];pos+=nv*40
        indices=struct.unpack_from('<'+'I'*ni,data,pos);pos+=ni*4
        parts.append({'slot':slot,'texture':texture,'vertices':vertices,'indices':indices,'blend':blend})
    for _ in range(nt):size=struct.unpack_from('<I',data,pos)[0];pos+=4;images.append(data[pos:pos+size]);pos+=size
    return {'joints':nj,'clips':nc,'rig':rig,'parts':parts,'images':images,'animations':data[pos:]}
def point(vertex):return tuple(round(v,5) for v in struct.unpack_from('<3f',vertex))
def key(points):return tuple(sorted(points))
def regionOf(points):
    y=sum(p[1] for p in points)/3
    return 0 if y>.78 else 1 if y>.16 else 2
class SkinUV:
    # Transfer body UVs by triangle, preserving atlas seams. Nearest triangle
    # transfer covers the small clothing-driven changes to the bare body mesh.
    def __init__(self,body):
        self.triangles=[];self.exact={}
        for part in body['parts']:
            if part['slot']!=0:continue
            self.texture=part['texture']
            for i in range(0,len(part['indices']),3):
                vertices=[part['vertices'][j] for j in part['indices'][i:i+3]]
                ps=np.array([struct.unpack_from('<3f',v) for v in vertices]);uv=np.array([struct.unpack_from('<2f',v,12) for v in vertices]);index=len(self.triangles)
                self.triangles.append((ps,uv));self.exact[key(point(v) for v in vertices)]=index
        self.centers=np.array([ps.mean(axis=0) for ps,uv in self.triangles])
    def transfer(self,vertices):
        ps=np.array([struct.unpack_from('<3f',v) for v in vertices]);rounded=[point(v) for v in vertices]
        index=self.exact.get(key(rounded))
        if index is None:index=int(np.argmin(((self.centers-ps.mean(axis=0))**2).sum(axis=1)))
        source,uv=self.triangles[index];result=[]
        for vertex,p in zip(vertices,ps):
            exact=next((i for i,q in enumerate(source) if np.linalg.norm(p-q)<.00003),None)
            if exact is not None:mapped=uv[exact]
            else:
                edges=np.array([source[1]-source[0],source[2]-source[0]]).T
                blend=np.linalg.lstsq(edges,p-source[0],rcond=None)[0];weights=np.maximum(0,[1-blend.sum(),*blend]);weights/=weights.sum();mapped=weights@uv
            result.append(vertex[:12]+struct.pack('<2f',*mapped)+vertex[20:])
        return result

def compact(slot,texture,triangles):
    vertices=[];indices=[];seen={}
    for triangle in triangles:
        for vertex in triangle:
            if vertex not in seen:seen[vertex]=len(vertices);vertices.append(vertex)
            indices.append(seen[vertex])
    return {'slot':slot,'texture':texture,'vertices':vertices,'indices':indices,'blend':0}
def check_rig(a,b):
    if a['joints']!=b['joints']:raise ValueError('Body rig differs')
    for j in range(a['joints']):
        x=struct.unpack_from('<I16f',a['rig'],j*68);y=struct.unpack_from('<I16f',b['rig'],j*68)
        if x[0]!=y[0] or max(abs(v-w) for v,w in zip(x[1:],y[1:]))>1e-5:raise ValueError('Complete body inverse bind matrices differ')
def write(path,bank):
    # Compact textures and vertices after selecting the visible submeshes.
    images=[];image_map={}
    with path.open('wb') as out:
        for part in bank['parts']:
            image=bank['images'][part['texture']];h=hashlib.sha256(image).hexdigest()
            if h not in image_map:image_map[h]=len(images);images.append(image)
            part['texture']=image_map[h]
        out.write(struct.pack('<8s4I',b'NXAVTR1\0',bank['joints'],len(bank['parts']),len(images),bank['clips']));out.write(bank['rig'])
        for part in bank['parts']:
            out.write(struct.pack('<5I',part['slot'],part['texture'],len(part['vertices']),len(part['indices']),part['blend']))
            for v in part['vertices']:out.write(v)
            out.write(struct.pack('<'+'I'*len(part['indices']),*part['indices']))
        for image in images:out.write(struct.pack('<I',len(image)));out.write(image)
        out.write(bank['animations'])
    return len(images)
def main():
    media=ROOT/'media/avatars';dest=ROOT/'projects/cards/romfs/avatars';dest.mkdir(parents=True,exist_ok=True)
    with tempfile.TemporaryDirectory() as tmp:
        tmp=Path(tmp);clothed=[];full=[]
        for name in NAMES:
            convert(media/f'xbox-{name}.glb',tmp/f'{name}.avb');clothed.append(read(tmp/f'{name}.avb'))
            convert(media/f'body-{name}.glb',tmp/f'body-{name}.avb');full.append(read(tmp/f'body-{name}.avb'))
        for index,name in enumerate(NAMES):
            original=clothed[index];body=full[index];check_rig(original,body)
            bank={**original,'parts':[p.copy() for p in original['parts'] if p['slot']!=0],'images':original['images']+body['images']}
            uv=SkinUV(body)
            for family in range(3):
                donor=index%2+family*2;check_rig(original,clothed[donor])
                for part in clothed[donor]['parts']:
                    if part['slot']!=0:continue
                    groups=[[],[],[]]
                    for at in range(0,len(part['indices']),3):
                        vertices=[part['vertices'][v] for v in part['indices'][at:at+3]];region=regionOf([point(v) for v in vertices])
                        groups[region].append(vertices if donor==index else uv.transfer(vertices))
                    for region,triangles in enumerate(groups):
                        if triangles:bank['parts'].append(compact(8+region*3+family,part['texture'] if donor==index else len(original['images'])+uv.texture,triangles))
            if index%2==0:
                calves=[]
                for part in body['parts']:
                    if part['slot']!=0:continue
                    for at in range(0,len(part['indices']),3):
                        vertices=[part['vertices'][v] for v in part['indices'][at:at+3]];y=sum(point(v)[1] for v in vertices)/3
                        if .30<y<.56:calves.append(vertices)
                if calves:bank['parts'].append(compact(17,len(original['images'])+uv.texture,calves))
            target=dest/f'{name}.avb';texture_count=write(target,bank)
            report=json.loads((tmp/f'{name}.json').read_text());report.update({'bank_sha256':hashlib.sha256(target.read_bytes()).hexdigest(),'parts':len(bank['parts']),'vertices':sum(len(p['vertices']) for p in bank['parts']),'triangles':sum(len(p['indices'])//3 for p in bank['parts']),'body_visibility':'Original clothing-fitted body meshes, body UV transfer within each compatible rig family, and complete calf geometry for shorts','body_source':f'body-{name}.glb','body_source_sha256':hashlib.sha256((media/f'body-{name}.glb').read_bytes()).hexdigest()})
            report['textures']=texture_count
            target.with_suffix('.json').write_text(json.dumps(report,indent=2)+'\n');print('Wardrobe',name,target.stat().st_size,'bytes')
if __name__=='__main__':main()
