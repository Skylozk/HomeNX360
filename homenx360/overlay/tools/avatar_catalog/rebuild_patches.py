#!/usr/bin/env python3
"""Regenerate geometry-only clothing and face patches (no texture/animation bake)."""
from build_catalog import *
p=argparse.ArgumentParser();p.add_argument('--converter',type=Path,required=True);p.add_argument('--toc',type=Path,required=True);p.add_argument('--legacy',type=Path,required=True);p.add_argument('--closet',type=Path);p.add_argument('--output',type=Path,required=True);a=p.parse_args();a.patch_only=True
rows=toc(a.toc);summary=json.loads((a.output/'catalog.json').read_text());entries=[x for x in summary['items'] if x['category'] not in FACE and x['category']!=4096]
with tempfile.TemporaryDirectory(prefix='nxe-patch-') as t:
 t=Path(t);base={g:bake(a,rows,g,None,t/'bake') for g in [1,2]}
 for n,item in enumerate(entries):
  for gender in [1,2]:
   if not item['bodies']&gender:continue
   current=bake(a,rows,gender,item,t/'bake');target=a.output/f"{item['key']}-{gender}.avp"
   if not patch(base[gender],current,target) and target.exists():target.unlink()
  if n%100==0:print(n,'/',len(entries),item['name'],flush=True)
print('All geometry patches regenerated.')
