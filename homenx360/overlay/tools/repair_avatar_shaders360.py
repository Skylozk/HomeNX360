#!/usr/bin/env python3
"""Repair shader packaging using the already tested, unchanged native binaries.

The source CI run produced correct NRO/cards shaders but retained upstream HOME
DKSH because their checkout timestamps were newer than the overlaid GLSL. This
repair changes only the HOME shader copies, source/build documentation and
packaging provenance. It rejects any change to native code, GLSL or ROMFS media.
"""
import argparse,hashlib,json,struct,zipfile,os
from pathlib import Path

FRONTEND_REVISION='4d4609f1d0ccecad67e5cdb01c48bed77bfbc038'
def sha(data):return hashlib.sha256(data).hexdigest()
def nro_files(data):
    assert data[16:20]==b'NRO0'
    asset=struct.unpack_from('<I',data,24)[0];assert data[asset:asset+4]==b'ASET'
    offset,size=struct.unpack_from('<QQ',data,asset+40);base=asset+offset
    assert base+size<=len(data)
    header=struct.unpack_from('<10Q',data,base);assert header[0]==80
    dirs=base+header[3];files=base+header[7];pixels=base+header[9];result={}
    def walk(at,path):
        assert len(path)<1024 and at<header[4]
        parent,sibling,child_dir,child_file,link,n=struct.unpack_from('<6I',data,dirs+at)
        name=data[dirs+at+24:dirs+at+24+n].decode();path=path+'/'+name if path else name
        while child_file!=0xffffffff:
            assert child_file<header[8]
            parent,next_file,start,length,link,n=struct.unpack_from('<IIQQII',data,files+child_file)
            name=data[files+child_file+32:files+child_file+32+n].decode();key=path+'/'+name if path else name
            assert pixels+start+length<=base+size;result[key]=data[pixels+start:pixels+start+length];child_file=next_file
        while child_dir!=0xffffffff:
            next_dir=struct.unpack_from('<I',data,dirs+child_dir+4)[0];walk(child_dir,path);child_dir=next_dir
    walk(0,'');return result
def rewrite(path,replacements):
    temporary=path.with_suffix('.repair.zip')
    with zipfile.ZipFile(path) as old,zipfile.ZipFile(temporary,'w',zipfile.ZIP_DEFLATED) as new:
        assert old.testzip() is None
        for info in old.infolist():
            if info.filename not in replacements:new.writestr(info,old.read(info.filename))
        for name,data in sorted(replacements.items()):new.writestr(name,data)
    os.replace(temporary,path)
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('artifacts',type=Path);p.add_argument('repository',type=Path);p.add_argument('--packaging-revision',required=True);a=p.parse_args()
    artifacts=a.artifacts;repo=a.repository;overlay=repo/'homenx360/overlay'
    for line in (artifacts/'SHA256SUMS.txt').read_text().splitlines():
        expected,name=line.split(maxsplit=1);assert sha((artifacts/name.lstrip('*')).read_bytes())==expected,name
    manifest=json.loads((artifacts/'BUILD-MANIFEST.json').read_text())
    assert manifest['frontend_source_revision']==FRONTEND_REVISION and 'shader_packaging_repair' not in manifest
    source=artifacts/'HomeNX360-source.zip';changes={}
    with zipfile.ZipFile(source) as z:
        assert z.testzip() is None
        for file in overlay.rglob('*'):
            if not file.is_file() or '__pycache__' in file.parts or file.suffix=='.pyc':continue
            rel=str(file.relative_to(overlay));name='HomeNX360-source/'+rel;data=file.read_bytes()
            if file.suffix in ('.cpp','.hpp','.inc','.glsl','.bin','.avb','.glb','.ttf','.png') or rel=='xmake.lua':
                assert z.read(name)==data,'Native input changed; a fresh compile is required: '+rel
            changes[name]=data
    update=artifacts/'HomeNX360-update.zip';replacements={};fixed=[]
    with zipfile.ZipFile(update) as z:
        assert z.testzip() is None
        for name,expected in manifest['sha256'].items():assert sha(z.read(name))==expected,name
        nro=z.read('switch/SwitchU/bin/cardsnx.nro');assert nro==(artifacts/'HomeNX360-preview.nro').read_bytes()
        embedded=nro_files(nro)
        for name in z.namelist():
            prefix='switch/SwitchU/cards/shaders/'
            if name.startswith(prefix) and name.endswith('.dksh'):
                shader=name[len(prefix):];data=z.read(name);assert embedded['shaders/'+shader]==data
                target='switch/SwitchU/shaders/'+shader
                if z.read(target)!=data:fixed.append(shader)
                replacements[target]=data;manifest['sha256'][target]=sha(data)
        for file in (overlay/'projects/cards/romfs/avatars').iterdir():
            if file.is_file():assert embedded['avatars/'+file.name]==file.read_bytes()==z.read('switch/SwitchU/cards/avatars/'+file.name)
    assert sorted(fixed)==['basic_fsh.dksh','basic_vsh.dksh'],fixed
    manifest['packaging_revision']=a.packaging_revision
    manifest['shader_packaging_repair']={'source_ci_run':42374452112,'frontend_source_revision':FRONTEND_REVISION,'changed_home_shaders':sorted(fixed),'native_binaries':'Unchanged from the successful source CI run; verified byte-for-byte','source_changes':'Shader build freshness and package validation; native/GLSL/media inputs unchanged'}
    text=json.dumps(manifest,indent=2)+'\n';(artifacts/'BUILD-MANIFEST.json').write_text(text)
    replacements['switch/SwitchU/dashboard360-build.json']=text.encode()
    rewrite(update,replacements);rewrite(source,changes)
    for name in ('AVATAR-GUIDE.md','RELEASE-NOTES.md'): (artifacts/name).write_bytes((repo/'homenx360'/name).read_bytes())
    with zipfile.ZipFile(update) as z:
        assert z.testzip() is None
        for name,expected in manifest['sha256'].items():assert sha(z.read(name))==expected,name
        embedded=nro_files(z.read('switch/SwitchU/bin/cardsnx.nro'))
        for shader in fixed:assert embedded['shaders/'+shader]==z.read('switch/SwitchU/cards/shaders/'+shader)==z.read('switch/SwitchU/shaders/'+shader)
    with zipfile.ZipFile(source) as z:
        assert z.testzip() is None
        for name,data in changes.items():assert z.read(name)==data,name
    sums=[]
    for file in sorted(artifacts.iterdir()):
        if file.is_file() and file.name!='SHA256SUMS.txt':sums.append(sha(file.read_bytes())+'  '+file.name)
    (artifacts/'SHA256SUMS.txt').write_text('\n'.join(sums)+'\n')
    print(json.dumps({'fixed_shaders':fixed,'frontend_source_revision':FRONTEND_REVISION,'packaging_revision':a.packaging_revision,'checked_source_overlay_files':len(changes),'checks':'All input/output hashes, ZIP CRCs, unchanged native binaries/code/GLSL/media, NRO/SD shaders and avatars'},indent=2))
if __name__=='__main__':main()
