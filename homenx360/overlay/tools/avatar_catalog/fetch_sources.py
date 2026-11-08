#!/usr/bin/env python3
"""Download and verify the public source archives; extract only the avatar packs.

No firmware, executable, patcher or account data is executed or installed.
Requires AVATAR-CATALOG-SOURCES.json supplied beside the release documentation.
"""
import argparse,hashlib,json,struct,urllib.request,zipfile
from pathlib import Path

def download(url,destination,expected,size):
 if destination.exists() and destination.stat().st_size==size and hashlib.sha256(destination.read_bytes()).hexdigest()==expected:return
 temporary=destination.with_suffix('.download');h=hashlib.sha256();total=0
 with urllib.request.urlopen(url,timeout=60) as response,temporary.open('wb') as output:
  while data:=response.read(1024*1024):
   total+=len(data)
   if total>size:raise ValueError('source archive exceeds expected length')
   h.update(data);output.write(data)
 if total!=size or h.hexdigest()!=expected:raise ValueError('source archive checksum differs from recorded source')
 temporary.replace(destination)

def extract_packs(container,output):
 # The reviewed public update's signed, read-only STFS container has two
 # contiguous files. These assertions deliberately reject other layouts.
 assert container[:4]==b'PIRS' and container[0x37b]&1
 base=(int.from_bytes(container[0x340:0x344],'big')+4095)&~4095
 def offset(index):
  block=index;power=170
  for _ in range(3):
   block+=(index+power)//power
   if index<power:break
   power*=170
  return base+block*4096
 allowed={'AvatarAssetPack.toc','AvatarAssetPackLegacyV1.toc'}
 for i in range(2):
  entry=container[offset(0)+i*64:offset(0)+(i+1)*64];name=entry[:entry[40]&63].decode('ascii')
  assert name in allowed and entry[40]&64
  first=int.from_bytes(entry[47:50],'little');size=int.from_bytes(entry[52:56],'big');assert size<32*1024*1024
  data=b''.join(container[offset(b):offset(b)+4096] for b in range(first,first+(size+4095)//4096))[:size]
  assert len(data)==size;(output/name).write_bytes(data)

def main(a):
 manifest=json.loads(a.manifest.read_text());a.output.mkdir(parents=True,exist_ok=True);source=manifest['built_in_system_update'];firmware=a.output/'system-update.zip'
 download(source['archive_url'],firmware,source['archive_sha256'],source['archive_bytes'])
 with zipfile.ZipFile(firmware) as archive:
  container=archive.read(source['container'])
 assert hashlib.sha256(container).hexdigest()==source['container_sha256'];extract_packs(container,a.output)
 for name,h in source['packs'].items():assert hashlib.sha256((a.output/name).read_bytes()).hexdigest()==h
 market=manifest['marketplace'];download(market['archive_url'],a.output/'marketplace.rar',market['archive_sha256'],market['archive_bytes'])
 firmware.unlink();print('Verified both avatar packs and the user-selected Marketplace collection.')
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('manifest',type=Path);p.add_argument('output',type=Path);main(p.parse_args())
