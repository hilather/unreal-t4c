"""Stable authored-input signatures, independent of generated exports and location."""
import hashlib
import json
import subprocess
from pathlib import Path

def signature(root, engine):
    paths = subprocess.check_output(['git','ls-files','-z','--','artsource/props','Source/Lighthaven/Visual/Props','build/build-props-art.sh'],cwd=root).decode().split('\0')
    digest=hashlib.sha256(json.dumps(engine,sort_keys=True,separators=(',',':')).encode())
    for name in sorted(set(filter(None,paths))):
        path=root/name
        if '__pycache__' in Path(name).parts or 'output' in Path(name).parts or path.suffix not in ('.py','.h','.cpp','.inl','.json','.sh','.md'):
            continue
        if path.is_file():
            digest.update(name.encode()+b'\0'); digest.update(path.read_bytes()); digest.update(b'\0')
    return digest.hexdigest()

def inventory(content):
    return {str(p.relative_to(content)):dict(bytes=p.stat().st_size,sha256=hashlib.sha256(p.read_bytes()).hexdigest()) for p in sorted(content.rglob('*.uasset'))}

if __name__=='__main__':
    import os,sys
    root=Path(__file__).resolve().parents[4]
    engine=json.loads((Path(os.environ['UE_ROOT'])/'Engine/Build/Build.version').read_text())
    stamp=root/'Content/Lighthaven/Art/Props/import-receipt.json'
    if stamp.exists():
        old=json.loads(stamp.read_text())
        if old['signature']==signature(root,engine) and old['assets'] and old['assets']==inventory(stamp.parent):
            print('PROPS_IMPORT_VERIFIED' if '--verify' in sys.argv else 'PROPS_IMPORT_UNCHANGED'); sys.exit(0)
    sys.exit(1)
