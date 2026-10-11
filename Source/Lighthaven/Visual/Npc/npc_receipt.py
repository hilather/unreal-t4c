"""Asset-only input/output evidence shared by preparation and Unreal import."""
import hashlib,json,os
from pathlib import Path
def signature(root):
    source=root/'artsource/npc/output'
    manifest=json.loads((source/'manifest.json').read_text())
    paths=sorted((root/'Source/Lighthaven/Visual/Npc').glob('*.py'))
    paths += [root/'Source/Lighthaven/Visual/Npc/LHNpcVisual.cpp',root/'Source/Lighthaven/Visual/Npc/LHNpcVisual.h',root/'Source/Lighthaven/Visual/Player/import_player.py',root/'artsource/npc/roster.json',source/'manifest.json',Path(os.environ['UE_ROOT'])/'Engine/Build/Build.version']
    paths += [source/k['file'] for k in manifest['kits']]+[source/t['file'] for t in manifest['textures'].values()]
    paths += sorted((root/'Content/Lighthaven/Art/Player').rglob('*.uasset'))
    digest=hashlib.sha256()
    for path in paths:
        label=str(path.relative_to(root)) if path.is_relative_to(root) else 'Engine/Build/Build.version'
        digest.update(label.encode()); digest.update(path.read_bytes())
    return digest.hexdigest()
def inventory(content):
    return {str(p.relative_to(content)):dict(bytes=p.stat().st_size,sha256=hashlib.sha256(p.read_bytes()).hexdigest()) for p in sorted(content.rglob('*.uasset'))}
