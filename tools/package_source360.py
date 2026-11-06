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
        if p.is_symlink():
            target=p.resolve()
            if not target.is_relative_to(args.source.resolve()):
                raise SystemExit(f'Source link escapes checkout: {rel}')
            # Preserve source-header aliases as ordinary files for ZIP extractors.
            if target.is_file() and not any(x in skip for x in target.relative_to(args.source.resolve()).parts):
                z.write(target,'HomeNX360-source/'+str(rel))
            continue
        if not p.is_file() or p.suffix in ext: continue
        z.write(p,'HomeNX360-source/'+str(rel))
print(args.output)
