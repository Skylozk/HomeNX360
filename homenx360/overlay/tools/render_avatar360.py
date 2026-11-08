#!/usr/bin/env python3
"""Rasterize preview_avatar360.cpp's native geometry; never used on Switch."""
import argparse,io,struct
from pathlib import Path
import numpy as np
from PIL import Image,ImageDraw,ImageFont
p=argparse.ArgumentParser(description=__doc__);p.add_argument('scene',type=Path);p.add_argument('output',type=Path);p.add_argument('--editor',action='store_true');p.add_argument('--catalog',action='store_true');a=p.parse_args()
data=a.scene.read_bytes();pos=0
def unpack(fmt):
 global pos
 out=struct.unpack_from(fmt,data,pos);pos+=struct.calcsize(fmt);return out
images,parts=unpack('<II');textures=[]
for _ in range(images):
 if a.catalog:
  iw,ih=unpack('<II');n=iw*ih*4;textures.append(np.frombuffer(data,dtype=np.uint8,count=n,offset=pos).reshape(ih,iw,4).astype(np.float32)/255);pos+=n
 else:
  n,=unpack('<I');textures.append(np.asarray(Image.open(io.BytesIO(data[pos:pos+n])).convert('RGBA'),dtype=np.float32)/255);pos+=n
w,h=1280,720
top=np.array([.18,.22,.22]);mid=np.array([.56,.58,.57]);bottom=np.array([.88,.89,.88])
rgb=np.empty((h,w,3),np.float32)
for y in range(h):
 t=y/405 if y<405 else (y-405)/315;rgb[y,:,:]=(top*(1-t)+mid*t) if y<405 else (mid*(1-t)+bottom*t)
depth=np.ones((h,w),np.float32)
for _ in range(parts):
 tex,blend,nv,ni=unpack('<4I');dye=np.array(unpack('<3f'));stride=8 if a.catalog else 5;vertices=np.frombuffer(data,dtype='<f4',count=nv*stride,offset=pos).reshape(nv,stride);pos+=nv*stride*4;indices=np.frombuffer(data,dtype='<u4',count=ni,offset=pos);pos+=ni*4;texture=textures[tex]
 for ids in indices.reshape(-1,3):
  v=vertices[ids];lo=np.maximum(0,np.floor(v[:,:2].min(axis=0)).astype(int));hi=np.minimum([w-1,h-1],np.ceil(v[:,:2].max(axis=0)).astype(int))
  if np.any(lo>hi):continue
  xx,yy=np.meshgrid(np.arange(lo[0],hi[0]+1)+.5,np.arange(lo[1],hi[1]+1)+.5)
  den=(v[1,1]-v[2,1])*(v[0,0]-v[2,0])+(v[2,0]-v[1,0])*(v[0,1]-v[2,1])
  if abs(den)<1e-7:continue
  u=((v[1,1]-v[2,1])*(xx-v[2,0])+(v[2,0]-v[1,0])*(yy-v[2,1]))/den
  q=((v[2,1]-v[0,1])*(xx-v[2,0])+(v[0,0]-v[2,0])*(yy-v[2,1]))/den;r=1-u-q
  z=u*v[0,2]+q*v[1,2]+r*v[2,2];ds=depth[lo[1]:hi[1]+1,lo[0]:hi[0]+1]
  mask=(u>=0)&(q>=0)&(r>=0)&(z<=ds)
  if not mask.any():continue
  uv=u[...,None]*v[0,3:5]+q[...,None]*v[1,3:5]+r[...,None]*v[2,3:5]
  # Bilinear sampling matches the native sampler's linear surface filtering.
  tx=np.clip(uv[...,0]*texture.shape[1]-.5,0,texture.shape[1]-1);ty=np.clip(uv[...,1]*texture.shape[0]-.5,0,texture.shape[0]-1)
  x0=tx.astype(int);y0=ty.astype(int);x1=np.minimum(x0+1,texture.shape[1]-1);y1=np.minimum(y0+1,texture.shape[0]-1);fx=(tx-x0)[...,None];fy=(ty-y0)[...,None]
  c=(texture[y0,x0]*(1-fx)+texture[y0,x1]*fx)*(1-fy)+(texture[y1,x0]*(1-fx)+texture[y1,x1]*fx)*fy
  if a.catalog:c[...,:3]*=u[...,None]*v[0,5:8]+q[...,None]*v[1,5:8]+r[...,None]*v[2,5:8]
  if dye[0]>=0:
   shade=np.clip(.55+(c[...,:3]*[.2126,.7152,.0722]).sum(axis=2)*1.7,.35,1.15);c[...,:3]=dye*shade[...,None]
  mask&=c[...,3]>0
  target=rgb[lo[1]:hi[1]+1,lo[0]:hi[0]+1];alpha=c[...,3:4];mixed=c[...,:3]*alpha+target*(1-alpha);target[mask]=mixed[mask]
  if not blend:ds[mask]=z[mask]
im=Image.fromarray(np.uint8(np.clip(rgb,0,1)*255));d=ImageDraw.Draw(im)
fontbase=Path(__file__).resolve().parents[1]/'projects/cards/romfs/fonts'
def font(size,bold=False):return ImageFont.truetype(str(fontbase/('selawksb.ttf' if bold else 'selawkl.ttf')),size)
d.text((1180,55),'customize avatar' if a.editor else 'sign in or out',font=font(38),anchor='ra',fill='white');d.text((1180,103),'Make this avatar yours' if a.editor else 'Choose your profile',font=font(19),anchor='ra',fill='white')
if not a.editor:
 for i,name in enumerate(['Player one','Player two','Player three','Player four']):
  x=190+i*335;d.text((x,515),name,font=font(25,True),anchor='ma',fill='#252b29');d.text((x,559),'Signed in' if i==0 else 'Offline profile',font=font(18),anchor='ma',fill='#545955')
 d.line((135,597,245,597),fill='#5caa0f',width=3)
else:
 names=['Face & body','Hairstyle','Hair colour','Top','Bottoms','Shoes','Accessories','Height','Build'];values=['The player','Ponytail','Original','Halo tee','Skinny jeans','Neon high-tops','On','3 / 5','3 / 5']
 if a.catalog:
  names=['Body type','Hairstyle','Hair colour','Tops','Bottoms','Shoes','Outfits & costumes','Headwear','Gloves'];values=['Male','Fauxhawk','2 / 32','Long Sleeved Tee','Khaki Chinos','High Top Sneakers','None','None','None']
 for i,(name,value) in enumerate(zip(names,values)):
  y=140+i*44
  if not i:d.rectangle((650,y,1170,y+42),fill='#6ba815')
  if a.catalog:
   d.text((662,y+3),name,font=font(16),fill='white' if not i else '#252b29');d.text((1158,y+17),'<  '+value+'  >',font=font(18),anchor='ra',fill='white' if not i else '#252b29')
  else:
   d.text((662,y+10),name,font=font(21),fill='white' if not i else '#252b29');d.text((1158,y+10),'<  '+value+'  >',font=font(20),anchor='ra',fill='white' if not i else '#252b29')
 if a.catalog:
  d.line((1188,140,1188,536),fill='#929a96',width=3);d.line((1188,140,1188,255),fill='#5caa0f',width=5);d.text((665,593),'1917 catalog assets',font=font(18),fill='#545c58')
 d.text((665,564),'L / R  Rotate   ZL / ZR  Skip 10' if a.catalog else 'L / R  Rotate avatar',font=font(20),fill='#252b29');d.text((330,598),'Player one',font=font(25,True),anchor='ma',fill='#252b29')
for x,letter,label,color in ([(88,'X','Save avatar','#1065b9'),(320,'B','Cancel','#bf1512'),(520,'Y','Randomize','#d6a90a')] if a.editor else [(88,'A','Select','#5caa0f'),(270,'B','Back','#bf1512'),(440,'X','Edit avatar','#1065b9'),(680,'Y','Manage users','#d6a90a')]):
 d.ellipse((x-11,655,x+11,677),fill=color);d.text((x,657),letter,font=font(16,True),anchor='ma',fill='white');d.text((x+19,654),label,font=font(22),fill='#252b29')
a.output.parent.mkdir(parents=True,exist_ok=True);im.save(a.output);print(a.output)
