#!/usr/bin/env python3
"""Compress complete native banks without changing their decoded contents."""
import argparse,json,os,struct,zlib
from pathlib import Path

def main(root):
 d=json.loads((root/'catalog.json').read_text());keys={x['key'] for x in d['items']};before=after=0
 paths=[root/'base-1.avb',root/'base-2.avb']
 for p in root.glob('*.avb'):
  key=p.stem.split('-prop-')[0]
  if key in keys:paths.append(p)
 for p in paths:
  b=p.read_bytes();before+=len(b)
  if b[:8]==b'NXAZIP1\0':after+=len(b);continue
  assert b[:8] in [b'NXAVTR2\0',b'NXAVTR3\0'] and len(b)<=8*1024*1024
  packed=struct.pack('<8sI',b'NXAZIP1\0',len(b))+zlib.compress(b,9)
  tmp=p.with_suffix('.packing');tmp.write_bytes(packed);os.replace(tmp,p);after+=len(packed)
 print('Native banks:',before,'->',after,'bytes')
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('catalog',type=Path);main(p.parse_args().catalog)
