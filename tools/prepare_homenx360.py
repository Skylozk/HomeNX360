#!/usr/bin/env python3
"""Fetch the exact upstream source and apply the readable HomeNX 360 overlay."""
import argparse
from pathlib import Path
import shutil
import subprocess

UPSTREAM = 'https://github.com/ad3333m/homenx.git'
COMMIT = '94f2bd56698fed40f2897131ea09463775840767'
ROOT = Path(__file__).resolve().parents[1]


def git(*args, cwd):
    return subprocess.check_output(['git', *args], cwd=cwd, text=True).strip()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--dest', type=Path, default=ROOT / 'build/homenx360-source')
    parser.add_argument('--skip-submodules', action='store_true',
                        help='For host navigation checks only; native builds need the submodules')
    args = parser.parse_args()
    dest = args.dest.resolve()
    overlay = ROOT / 'homenx360/overlay'
    if not overlay.is_dir():
        raise SystemExit('The source overlay is missing')
    if dest.exists() and any(dest.iterdir()):
        if not (dest / '.git').exists():
            raise SystemExit(f'Refusing to overwrite a non-Git directory: {dest}')
        if git('rev-parse', 'HEAD', cwd=dest) != COMMIT:
            raise SystemExit('Existing destination is not at the pinned upstream commit')
        if git('status', '--porcelain', '--untracked-files=normal', cwd=dest):
            raise SystemExit('Existing destination has changes; choose an empty destination')
    else:
        dest.mkdir(parents=True, exist_ok=True)
        subprocess.run(['git', 'init', str(dest)], check=True)
        git('remote', 'add', 'origin', UPSTREAM, cwd=dest)
        git('fetch', '--depth=1', 'origin', COMMIT, cwd=dest)
        git('checkout', '--detach', 'FETCH_HEAD', cwd=dest)
    if git('rev-parse', 'HEAD', cwd=dest) != COMMIT:
        raise SystemExit('Upstream commit verification failed')
    if not args.skip_submodules:
        git('submodule', 'update', '--init', '--recursive', '--depth=1', cwd=dest)
    # Refuse overlay symlinks and copy only files under the source overlay.
    for source in sorted(overlay.rglob('*')):
        if source.is_symlink():
            raise SystemExit(f'Unexpected overlay symlink: {source}')
        if source.is_file():
            target = dest / source.relative_to(overlay)
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source, target)
    git('diff', '--check', cwd=dest)
    print(f'Prepared HomeNX 360 at {dest}\nUpstream: {COMMIT}')


if __name__ == '__main__':
    main()
