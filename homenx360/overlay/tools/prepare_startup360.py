#!/usr/bin/env python3
"""Preserve a startup recording as lossless WebP frames plus PCM sound."""
import argparse,hashlib,io,json,struct,subprocess,tempfile
from pathlib import Path
from PIL import Image
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('input',type=Path);p.add_argument('output',type=Path)
a=p.parse_args()
info=json.loads(subprocess.check_output(['ffprobe','-v','error','-show_streams','-show_format','-of','json',str(a.input)]))
v=next(s for s in info['streams'] if s['codec_type']=='video')
num,den=map(int,v['avg_frame_rate'].split('/'))
width,height=1280,720
with tempfile.TemporaryDirectory() as tmp:
 wav=Path(tmp)/'audio.wav'
 subprocess.run(['ffmpeg','-v','error','-i',str(a.input),'-vn','-ar','44100','-ac','2','-c:a','pcm_s16le','-y',str(wav)],check=True)
 command=['ffmpeg','-v','error','-i',str(a.input),'-map','0:v:0','-vf','scale=1280:720:flags=lanczos','-vsync','0','-pix_fmt','rgb24','-f','rawvideo','pipe:1']
 child=subprocess.Popen(command,stdout=subprocess.PIPE)
 hashes=[];frames=[]
 try:
  while True:
   raw=child.stdout.read(width*height*3)
   if not raw:break
   if len(raw)!=width*height*3:raise ValueError('Truncated video frame')
   image=Image.frombytes('RGB',(width,height),raw)
   blob=io.BytesIO();image.save(blob,format='WEBP',lossless=True,method=3)
   data=blob.getvalue()
   # Verify every encoded frame against the resized source pixels.
   if Image.open(io.BytesIO(data)).convert('RGB').tobytes()!=raw:raise ValueError('Lossless frame verification failed')
   frames.append(data);hashes.append(hashlib.sha256(raw).hexdigest())
  if child.wait()!=0:raise ValueError('Video decoder failed')
 finally:
  if child.poll() is None:child.kill();child.wait()
 audio=wav.read_bytes();offset=40+(len(frames)+1)*4+len(audio)
 offsets=[offset]
 for f in frames:offset+=len(f);offsets.append(offset)
 duration=round(float(info['format']['duration'])*1000000)
 header=struct.pack('<8s8I',b'NXBOOT1\0',width,height,num,den,len(frames),duration,len(audio),offsets[0])
 a.output.parent.mkdir(parents=True,exist_ok=True)
 with a.output.open('wb') as out:
  out.write(header);out.write(struct.pack('<'+'I'*len(offsets),*offsets));out.write(audio)
  for f in frames:out.write(f)
 report={'source_sha256':hashlib.sha256(a.input.read_bytes()).hexdigest(),'source_size':[v['width'],v['height']],'playback_size':[width,height],'frame_rate':[num,den],'frames':len(frames),'duration_us':duration,'resampling':'Lanczos to Switch logical 1280x720; no cropping or overlays','frame_encoding':'lossless WebP; every decoded frame compared with resized source RGB','audio':'PCM 16-bit stereo 44100 Hz','bank_sha256':hashlib.sha256(a.output.read_bytes()).hexdigest(),'frame_rgb_sha256':hashes}
 a.output.with_suffix('.json').write_text(json.dumps(report,indent=2)+'\n')
 print('Verified',len(frames),'lossless frames;',a.output.stat().st_size,'bytes')
