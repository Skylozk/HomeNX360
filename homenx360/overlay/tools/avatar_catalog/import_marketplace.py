#!/usr/bin/env python3
"""Read only avatar asset packages from the user-selected RAR collection.

Uses libarchive for RAR decoding and the pinned avatarextract STFS reader. Bundled
executables, tools, profile patchers and nested archives are never extracted.
"""
import argparse,hashlib,json,re,shutil,subprocess,tempfile
from pathlib import Path,PurePosixPath
import libarchive

def main(a):
 a.output.mkdir(parents=True,exist_ok=True);inventory=[];failures=[]
 with tempfile.TemporaryDirectory(prefix='nxe-marketplace-') as t:
  t=Path(t)
  with libarchive.file_reader(str(a.archive)) as archive:
   for e in archive:
    path=PurePosixPath(str(e).replace('\\','/'));name=path.name
    if '/Items/' not in '/'+str(path) or not re.fullmatch('[0-9A-Fa-f]{32}',name):continue
    if e.size>8*1024*1024:raise ValueError('unexpectedly large avatar package')
    data=b''.join(e.get_blocks());assert len(data)==e.size
    if data[:4] not in [b'CON ',b'PIRS',b'LIVE']:
     failures.append({'path':str(path),'reason':'not STFS'});continue
    package=t/'item';package.write_bytes(data);dest=t/'extract';shutil.rmtree(dest,ignore_errors=True);dest.mkdir()
    result=subprocess.run([str(a.converter),'--extract',str(package),str(dest)],capture_output=True,text=True)
    blobs=list(dest.rglob('asset_v2.bin')) or list(dest.rglob('asset.bin'))
    if result.returncode or not blobs:
     failures.append({'path':str(path),'reason':'no valid asset blob','log':result.stdout[-600:]});continue
    # The archive package name stores the product GUID in network-byte order.
    h=name.lower();guid=f'{h[:8]}-{h[8:12]}-{h[12:16]}-{h[16:20]}-{h[20:]}'
    category=int(h[:8],16);body=int(h[15],16)
    if body not in [1,2,3]:
     # Structural detection is authoritative for packages with another gender nibble.
     check=subprocess.run([str(a.converter),str(blobs[0]),str(t/'inspect')],capture_output=True,text=True)
     match=re.search(r'closet-import bodies verdict: (\d+)',check.stdout)
     body=int(match.group(1)) if match else 3
    raw=blobs[0].read_bytes();assert raw[:4] in [b'STRB',b'YTGR']
    target=a.output/(guid+'.bin')
    if target.exists() and target.read_bytes()!=raw:raise ValueError('conflicting duplicate GUID')
    target.write_bytes(raw)
    title=path.parent.parent.name;label=path.parent.name
    inventory.append(dict(guid=guid,category=category,bodies=body,name=label,title=title,source_path=str(path),package_sha256=hashlib.sha256(data).hexdigest(),asset_sha256=hashlib.sha256(raw).hexdigest()))
    if len(inventory)%100==0:print('Extracted',len(inventory),'packages',flush=True)
 # Deduplicate identical packages while retaining the complete source inventory.
 by_guid={i['guid']:i for i in inventory}
 (a.output/'closet_index.tsv').write_text('guid\tcategories\tbodies\tname\n'+''.join(f"{i['guid']}\t{i['category']:X}\t{i['bodies']}\t{i['name']} ({i['title']})\n" for i in sorted(by_guid.values(),key=lambda i:i['guid'])))
 (a.output/'provenance.json').write_text(json.dumps(dict(archive_sha256=hashlib.sha256(a.archive.read_bytes()).hexdigest(),packages=inventory,failures=failures),indent=2)+'\n')
 print('Imported',len(by_guid),'unique assets;',len(failures),'failures')
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('archive',type=Path);p.add_argument('output',type=Path);p.add_argument('--converter',type=Path,required=True);main(p.parse_args())
