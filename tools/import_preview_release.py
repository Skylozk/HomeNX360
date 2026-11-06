#!/usr/bin/env python3
"""Import the verified initial preview into the dedicated repository, unchanged."""
import argparse
import hashlib
import json
from pathlib import Path
import urllib.request
import zipfile

SOURCE_REVISION = 'f4484c290790d68883eb8cb687fcdbb1b1545203'
SOURCE_URL = 'https://github.com/Skylozk/randomgit/releases/download/v0.2.0-preview.1/'
# Digests recorded after the original build/publish run 42218935324 passed.
EXPECTED = {
    'BUILD-MANIFEST.json': 'ef6d86a82e76dc66b4e38591e7f30d62c1fe770cc8d0df93b4528a4c34b1b18d',
    'HomeNX360-preview.nro': '3fefce3d3de139384eac7e1d2e96a9321912995d012eea4658494d599427c6ec',
    'HomeNX360-source.zip': 'db2fb0db299fc485b06e4b0b3f5876783e1081f5852dcf85a9f33ef97957000d',
    'HomeNX360-update.zip': '59a70e46015616e0190fea04dd1cd319540092b4344f568d1c89e04ad507da7f',
    'INSTALL.md': '6c53c6bfb8edffb515044159cbd16dc959add4ca13e50846a82095c554d168d5',
    'NXE-REFERENCES.md': '675c3f68d724e1ab3f71df408f1983f9838663b68bc673583bc2a2431fdcc780',
    'RELEASE-NOTES.md': '24e9daa59aaad9db1fe32c20cff30c4bc6a0ec7c70c9f7619e93a95f5309a1c2',
    'SHA256SUMS.txt': '5fe56b9220779ab1627f3e8ef8616013e4cd2eb92fb495a75cd17be4788c0caf',
}


def digest(path):
    h = hashlib.sha256()
    with path.open('rb') as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()


def verify(folder):
    for name, expected in EXPECTED.items():
        if digest(folder / name) != expected:
            raise ValueError(f'Unexpected release file: {name}')
    sums = dict((name, value) for value, name in (
        line.split('  ', 1) for line in (folder / 'SHA256SUMS.txt').read_text().splitlines()))
    if sums != {name: value for name, value in EXPECTED.items() if name != 'SHA256SUMS.txt'}:
        raise ValueError('Checksums do not describe the complete verified release')
    manifest = json.loads((folder / 'BUILD-MANIFEST.json').read_text())
    if manifest['frontend_source_revision'] != SOURCE_REVISION or manifest['dashboard_style'] != 'NXE':
        raise ValueError('Unexpected source revision or dashboard style')
    with zipfile.ZipFile(folder / 'HomeNX360-update.zip') as z:
        if z.testzip() is not None:
            raise ValueError('Update ZIP failed CRC verification')
        if json.loads(z.read('switch/SwitchU/dashboard360-build.json')) != manifest:
            raise ValueError('Embedded executable manifest differs')
        for name, expected in manifest['sha256'].items():
            if hashlib.sha256(z.read(name)).hexdigest() != expected:
                raise ValueError(f'Executable manifest mismatch: {name}')
        if hashlib.sha256(z.read('switch/SwitchU/bin/cardsnx.nro')).hexdigest() != EXPECTED['HomeNX360-preview.nro']:
            raise ValueError('Standalone preview differs from packaged app')
        if z.read('config/SwitchU/frontend.txt') != b'cards\n':
            raise ValueError('Unexpected frontend selection')
    with zipfile.ZipFile(folder / 'HomeNX360-source.zip') as z:
        if z.testzip() is not None:
            raise ValueError('Source ZIP failed CRC verification')
        for name in ['projects/cards/src/NxeLayout.hpp', 'projects/cards/src/Dashboard360.inc',
                     'lib/espeak-ng/src/include/espeak/speak_lib.h']:
            if not z.read('HomeNX360-source/' + name):
                raise ValueError(f'Missing corresponding source: {name}')
    print('Verified all eight original assets, executable manifests and corresponding source.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('destination', type=Path)
    parser.add_argument('--verify-only', action='store_true')
    args = parser.parse_args()
    args.destination.mkdir(parents=True, exist_ok=True)
    if not args.verify_only:
        for name, expected in EXPECTED.items():
            part = args.destination / (name + '.part')
            total = 0
            try:
                with urllib.request.urlopen(SOURCE_URL + name, timeout=90) as response, part.open('wb') as out:
                    while chunk := response.read(1024 * 1024):
                        total += len(chunk)
                        if total > 80 * 1024 * 1024:
                            raise ValueError(f'Unexpected download size: {name}')
                        out.write(chunk)
                if digest(part) != expected:
                    raise ValueError(f'Download hash mismatch: {name}')
                part.replace(args.destination / name)
            finally:
                part.unlink(missing_ok=True)
    verify(args.destination)


if __name__ == '__main__':
    main()
