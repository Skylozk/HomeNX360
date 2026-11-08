#!/usr/bin/env python3
"""Rebuild only prop performances after a converter change."""
from build_catalog import *
p=argparse.ArgumentParser();p.add_argument('--converter',type=Path,required=True);p.add_argument('--toc',type=Path,required=True);p.add_argument('--legacy',type=Path,required=True);p.add_argument('--closet',type=Path);p.add_argument('--output',type=Path,required=True);a=p.parse_args();a.patch_only=True
rows=toc(a.toc);summary=json.loads((a.output/'catalog.json').read_text());props=[x for x in summary['items'] if x['category']==4096]
# Retry every failed package too; geometry extraction itself already passed.
lookup={}
for line in (a.closet/'closet_index.tsv').read_text().splitlines()[1:]:
 col=line.split('\t')
 if len(col)>=4:
  key='m-'+col[0].replace('-','');lookup[key]=dict(key=key,guid=col[0],category=int(col[1],16),bodies=int(col[2]),name=col[3],partner=0,marketplace=True)
for failure in summary['failures']:
 item=lookup[failure['key']];assert item['category']==4096;props.append(item);summary['items'].append(item)
summary['failures']=[]
with tempfile.TemporaryDirectory(prefix='nxe-prop-') as t:
 for n,item in enumerate(props):
  stats=[]
  for gender in [1,2]:
   if not item['bodies']&gender:continue
   target=a.output/f"{item['key']}-prop-{gender}.avb"
   if target.exists():
    packed=target.read_bytes()
    if packed[:8]==b'NXAZIP1\0' and zlib.decompress(packed[12:])[:8]==b'NXAVTR3\0':stats.append(bank_stats(target));continue
   d=Path(t)/'bake';s=bake(a,rows,gender,item,d,clips=True,clothed=True,white=False);stats.append(bank(s,d,target,meshes=[m for m in s['meshes'] if m['is_prop']],clips=True))
  item['stats']=stats
  (a.output/'catalog.json').write_text(json.dumps(summary,indent=2)+'\n')
  if n%20==0:print(n,'/',len(props),item['name'],flush=True)
(a.output/'catalog.json').write_text(json.dumps(summary,indent=2)+'\n')
