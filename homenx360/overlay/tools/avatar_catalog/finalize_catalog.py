#!/usr/bin/env python3
"""Write the bounded native index and an inventory of every shipped file."""
import argparse,hashlib,json,struct
from pathlib import Path

def group(c):
 cats=[4,8,16,32,64,128,256,512,1024,2048,8192,16384,32768,65536,131072,262144,524288,1048576,2097152]
 if c==4096:return 19
 if c>8191 and not c&8191:return cats.index(c) if c in cats else -1
 c&=8191
 if c&8 and c&16:return 20
 if c&64 and not c&(8|16):return 4
 return next((i for i,x in enumerate(cats[:10]) if c&x),-1)
def main(path,partial=False):
 d=json.loads((path/'catalog.json').read_text());rows=[r for r in d['items'] if group(r['category'])>=0]
 with (path/'index.avc').open('wb') as f:
  f.write(struct.pack('<8sI',b'NXACAT1\0',len(rows)))
  for r in rows:
   key=r['key'].encode();name=r['name'].encode();assert 0<len(key)<=40 and 0<len(name)<=512
   f.write(struct.pack('<5I',r['category'],r['bodies'],r.get('partner',0),len(key),len(name)));f.write(key);f.write(name)
 if not partial:
  manifest={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(path.iterdir()) if p.is_file() and p.name!='SHA256.json'}
  (path/'SHA256.json').write_text(json.dumps(manifest,indent=2)+'\n')
 print('Indexed',len(rows),'assets;',sum(bool(r.get('marketplace')) for r in rows),'Marketplace assets;',len(d['failures']),'conversion failures')
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('catalog',type=Path);p.add_argument('--partial',action='store_true');a=p.parse_args();main(a.catalog,a.partial)
