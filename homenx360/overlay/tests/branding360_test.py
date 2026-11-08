"""Inspect the actual built NRO metadata and icon, rather than the xmake text."""
from pathlib import Path
import struct
root=Path(__file__).resolve().parents[1]
release=root/'build/cross/aarch64/release'
icon=(root/'projects/cards/icon.jpg').read_bytes()
assert icon[:2]==b'\xff\xd8'
for filename,title in [('CardsNX.nro','HomeNX360'),('HomeNX360-Updater.nro','HomeNX360 Updater'),('HomeNX-Loader.nro','HomeNX360 Loader')]:
 data=(release/filename).read_bytes()
 assert data[16:20]==b'NRO0'
 size=struct.unpack_from('<I',data,24)[0]
 assert data[size:size+4]==b'ASET'
 image_at,image_size,nacp_at,nacp_size=struct.unpack_from('<4Q',data,size+8)
 assert data[size+image_at:size+image_at+image_size]==icon,filename
 nacp=data[size+nacp_at:size+nacp_at+nacp_size]
 assert len(nacp)>=0x4000
 names=[nacp[language*0x300:language*0x300+0x200].split(b'\0',1)[0].decode() for language in range(16)]
 # Older devkitPro tools leave unsupported language slots empty; hbmenu falls
 # back to the first supported name. Every populated slot must use the brand.
 assert names[0]==title and all(name==title for name in names if name),filename
 print(filename+': '+title+'; shared HomeNX360 launcher icon verified')
