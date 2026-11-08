from pathlib import Path
import sys,zipfile
root=Path(sys.argv[1]);root.mkdir(parents=True,exist_ok=True)
files={'switch/HomeNX360/HomeNX360.nro':b'new-preview','switch/HomeNX360/version.txt':b'v0.8.1-preview.1\n','switch/HomeNX360-Updater/HomeNX360-Updater.nro':b'new-updater','atmosphere/contents/0100000000001000/exefs.nsp':b'daemon','switch/SwitchU/bin/menu/main':b'menu','switch/SwitchU/bin/menu/main.npdm':b'npdm','switch/SwitchU/bin/cardsnx.nro':b'new-preview','switch/SwitchU/cards/avatars/catalog/index.avc':b'catalog','switch/SwitchU/homenx360-version.txt':b'v0.8.1-preview.1\n','config/CardsNX/settings.json':b'do not install this','../escaped.txt':b'do not extract this'}
with zipfile.ZipFile(root/'valid.zip','w',zipfile.ZIP_DEFLATED) as z:
 for path,data in files.items():z.writestr(path,data)
with zipfile.ZipFile(root/'incomplete.zip','w') as z:z.writestr('switch/SwitchU/bin/menu/main',b'incomplete')
