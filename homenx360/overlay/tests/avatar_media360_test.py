"""Verify shipped native banks against the unchanged source export manifests."""
import hashlib,json,struct
from pathlib import Path
root=Path(__file__).resolve().parents[1]
names=['regular','player','fan','street','smart','night']
for name in names:
    path=root/'projects/cards/romfs/avatars'/f'{name}.avb';data=path.read_bytes()
    report=json.loads(path.with_suffix('.json').read_text())
    magic,joints,parts,textures,clips=struct.unpack_from('<8s4I',data)
    assert magic==b'NXAVTR1\0' and joints==71 and clips==2
    assert parts==report['parts'] and textures==report['textures']
    assert hashlib.sha256(data).hexdigest()==report['bank_sha256']
    for field in ('source','body_source'):
        source=root/'media/avatars'/report[field]
        assert source.is_file() and hashlib.sha256(source.read_bytes()).hexdigest()==report[field+'_sha256']
print('Six native banks and all 12 unchanged source export SHA-256 hashes passed.')
