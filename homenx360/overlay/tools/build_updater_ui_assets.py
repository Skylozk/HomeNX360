#!/usr/bin/env python3
"""Bake small Selawik coverage atlases for the minimal libnx updater renderer."""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
import math,struct,shutil
root=Path(__file__).resolve().parents[1]
out=root/'projects/homenx360-updater/romfs';out.mkdir(parents=True,exist_ok=True)
for label,size in [('small',20),('body',28),('title',42)]:
 font=ImageFont.truetype(str(root/'projects/cards/romfs/fonts/selawkl.ttf'),size)
 ascent,descent=font.getmetrics();advances=[math.ceil(font.getlength(chr(c))) for c in range(32,128)]
 cw=max(advances)+6;ch=ascent+descent+6;w=cw*16;h=ch*6
 atlas=Image.new('L',(w,h));draw=ImageDraw.Draw(atlas)
 for i,c in enumerate(range(32,128)):draw.text((i%16*cw+3,i//16*ch+3+ascent),chr(c),font=font,fill=255,anchor='ls')
 payload=b'NXUFNT1\0'+struct.pack('<4I',cw,ch,w,h)+struct.pack('<96H',*advances)+atlas.tobytes()
 (out/('font-'+label+'.bin')).write_bytes(payload)
logo=Image.open(root/'projects/cards/icon.jpg').convert('RGBA');assert logo.size==(256,256);(out/'logo.rgba').write_bytes(logo.tobytes())
shutil.copy2(root/'projects/cards/romfs/fonts/Selawik-OFL.txt',out/'Selawik-OFL.txt')
(out/'README.txt').write_text('HomeNX360 updater UI assets: Selawik font coverage under its included OFL; approved HomeNX360 launcher artwork. Rebuild with tools/build_updater_ui_assets.py.\n')
print('Baked minimal updater font/logo assets:',sum(p.stat().st_size for p in out.iterdir()),'bytes')
