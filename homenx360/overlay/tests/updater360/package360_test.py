"""Check publisher package contracts with tiny synthetic executable fixtures."""
from pathlib import Path
import hashlib,json,shutil,subprocess,sys,tempfile,zipfile
source=Path(__file__).resolve().parents[2]/'tools/package_dashboard360.py'
with tempfile.TemporaryDirectory(prefix='homenx360-package-') as temporary:
 root=Path(temporary);(root/'tools').mkdir();shutil.copy2(source,root/'tools/package_dashboard360.py')
 release=root/'build/cross/aarch64/release'
 for name,offset,magic in [('CardsNX.nro',16,b'NRO0'),('HomeNX-Loader.nro',16,b'NRO0'),('HomeNX360-Updater.nro',16,b'NRO0'),('exefs/switchu-menu/main',0,b'NSO0'),('switchu-daemon.nsp',0,b'PFS0')]:
  p=release/name;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(bytes(offset)+magic+b'fixture')
 (release/'exefs/switchu-menu/main.npdm').write_bytes(b'npdm')
 for folder in ['fonts','icons','shaders','certs','boot','avatars','wallpapers']:
  p=root/'build/cards-romfs'/folder;p.mkdir(parents=True,exist_ok=True);(p/('basic_fsh.dksh' if folder=='shaders' else 'fixture.dat')).write_bytes(b'fixture')
 for folder in ['fonts','icons','i18n','shaders']:
  p=root/'romfs'/folder;p.mkdir(parents=True,exist_ok=True);(p/('basic_fsh.dksh' if folder=='shaders' else 'fixture.dat')).write_bytes(b'fixture')
 p=root/'media/avatar-catalog';p.mkdir(parents=True);(p/'index.avc').write_bytes(b'catalog');(p/'SHA256.json').write_text(json.dumps({'index.avc':hashlib.sha256(b'catalog').hexdigest()}))
 p=root/'projects/cards/romfs';p.mkdir(parents=True);(p/'lang.tsv').write_text('fixture')
 (root/'README-360.md').write_text('Install fixture');(root/'AVATAR-GUIDE.md').write_text('Avatar fixture')
 out=root/'out';subprocess.run([sys.executable,str(root/'tools/package_dashboard360.py'),'--output',str(out),'--revision','fixture','--version','v0.8.1-preview.1'],check=True)
 manifest=json.loads((out/'BUILD-MANIFEST.json').read_text());assert manifest['release_tag']=='v0.8.1-preview.1'
 with zipfile.ZipFile(out/'HomeNX360-update.zip') as archive:
  assert len(archive.namelist())==len(set(archive.namelist()))
  for path,h in manifest['sha256'].items():assert hashlib.sha256(archive.read(path)).hexdigest()==h
  assert archive.read('switch/SwitchU/bin/cardsnx.nro')==archive.read('switch/HomeNX360/HomeNX360.nro')
  assert archive.read('switch/HomeNX360/version.txt')==archive.read('switch/SwitchU/homenx360-version.txt')==b'v0.8.1-preview.1\n'
  assert archive.read('switch/HomeNX360-Updater/HomeNX360-Updater.nro')==(out/'HomeNX360-Updater.nro').read_bytes()
  assert 'config/CardsNX/settings.json' not in archive.namelist()
  assert archive.read('switch/SwitchU/cards/wallpapers/fixture.dat')==b'fixture'
 with zipfile.ZipFile(out/'HomeNX360-avatar-catalog.zip') as archive:
  assert all(p=='AVATAR-GUIDE.md' or p.startswith('switch/SwitchU/cards/avatars/catalog/') for p in archive.namelist())
 print('Package contracts: both app locations, updater, version markers, hashes, profile exclusion and catalog-only archive passed.')
