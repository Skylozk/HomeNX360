#!/usr/bin/env python3
"""Package a compiled 360 dashboard update for an existing HomeNX install."""
import argparse
from pathlib import Path
import zipfile
import shutil
import json
import hashlib

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--output', type=Path, required=True)
parser.add_argument('--daemon-provenance', type=Path)
parser.add_argument('--revision', default='local working tree')
args = parser.parse_args()
output = args.output.resolve()
release = root / 'build/cross/aarch64/release'

files = {
    'switch/SwitchU/bin/menu/main': release / 'exefs/switchu-menu/main',
    'switch/SwitchU/bin/menu/main.npdm': release / 'exefs/switchu-menu/main.npdm',
    'atmosphere/contents/0100000000001000/exefs.nsp': release / 'switchu-daemon.nsp',
    'switch/SwitchU/bin/cardsnx.nro': release / 'CardsNX.nro',
    'switch/SwitchU/bin/homenx-loader.nro': release / 'HomeNX-Loader.nro',
}
for folder in ('fonts', 'icons', 'shaders', 'certs'):
    base = root / 'build/cards-romfs' / folder
    if not base.is_dir():
        raise SystemExit(f'Missing CardsNX assets: {base}')
    for p in base.rglob('*'):
        if p.is_file(): files['switch/SwitchU/cards/' + str(p.relative_to(base.parent))] = p
for folder in ('fonts', 'icons', 'i18n', 'shaders'):
    base = root / 'romfs' / folder
    for p in base.rglob('*'):
        if p.is_file(): files['switch/SwitchU/' + str(p.relative_to(root / 'romfs'))] = p
files['switch/SwitchU/cards/lang.tsv'] = root / 'projects/cards/romfs/lang.tsv'
for name, p in files.items():
    if not p.is_file() or p.stat().st_size == 0:
        raise SystemExit(f'Missing or empty build output: {p} (for {name})')
for p, offset, magic in (
    (release / 'CardsNX.nro', 16, b'NRO0'),
    (release / 'HomeNX-Loader.nro', 16, b'NRO0'),
    (release / 'exefs/switchu-menu/main', 0, b'NSO0'),
    (release / 'switchu-daemon.nsp', 0, b'PFS0'),
):
    with p.open('rb') as f:
        f.seek(offset)
        if f.read(4) != magic:
            raise SystemExit(f'Unexpected executable format: {p}')
if not any(n.startswith('switch/SwitchU/shaders/') for n in files):
    raise SystemExit('The rebuilt frontend shaders are missing')

output.mkdir(parents=True, exist_ok=True)
manifest = {'name': 'HomeNX 360 experimental',
            'upstream_commit': '94f2bd56698fed40f2897131ea09463775840767',
            'hardware_tested': False,
            'dashboard_style': 'NXE',
            'frontend_source_revision': args.revision,
            'sha256': {name: hashlib.sha256(p.read_bytes()).hexdigest() for name, p in sorted(files.items())}}
if args.daemon_provenance:
    manifest['daemon'] = json.loads(args.daemon_provenance.read_text())
    expected = manifest['daemon']['sha256']
    actual = manifest['sha256']['atmosphere/contents/0100000000001000/exefs.nsp']
    if actual != expected: raise SystemExit('Daemon provenance does not match the binary')
else:
    manifest['daemon'] = {'origin': 'local build'}
manifest_text = json.dumps(manifest, indent=2) + '\n'
(output / 'BUILD-MANIFEST.json').write_text(manifest_text)
shutil.copy2(release / 'CardsNX.nro', output / 'HomeNX360-preview.nro')
shutil.copy2(root / 'README-360.md', output / 'INSTALL.md')
with zipfile.ZipFile(output / 'HomeNX360-update.zip', 'w', zipfile.ZIP_DEFLATED) as z:
    for name, p in sorted(files.items()): z.write(p, name)
    z.writestr('config/SwitchU/frontend.txt', 'cards\n')
    z.writestr('switch/SwitchU/dashboard360-build.json', manifest_text)
    z.write(root / 'README-360.md', 'INSTALL-360.md')
print('Packaged preview NRO and HomeNX update:', output)
