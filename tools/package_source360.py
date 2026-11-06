#!/usr/bin/env python3
"""Package complete prepared GPL source, including populated upstream submodules."""
import argparse
from pathlib import Path
import zipfile
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('source',type=Path)
parser.add_argument('output',type=Path)
args=parser.parse_args()
skip={'.git','.xmake','build','__pycache__','.cache'}
ext={'.o','.a','.d','.pyc','.nro','.nsp','.nacp','.dksh','.elf','.nso'}
with zipfile.ZipFile(args.output,'w',zipfile.ZIP_DEFLATED) as z:
    for p in sorted(args.source.rglob('*')):
        rel=p.relative_to(args.source)
        if any(x in skip or x.endswith(('.gch','.pch')) for x in rel.parts): continue
        if rel.parts[:4]==('projects','menu','src','generated'): continue
        if p.is_symlink() or not p.is_file() or p.suffix in ext: continue
        z.write(p,'HomeNX360-source/'+str(rel))
print(args.output)
