"""Asset-only input/output evidence shared by preparation and Unreal import."""
import hashlib,json,os,sys
from pathlib import Path
def signature(root):
    paths=sorted((root/'Source/Lighthaven/Visual/Npc').glob('*.py'))
    paths += sorted((root/'artsource/npc').glob('*.py'))
    paths += [root/'Source/Lighthaven/Visual/Npc/LHNpcVisual.h',
              root/'artsource/npc/roster.json',
              Path(os.environ['UE_ROOT'])/'Engine/Build/Build.version']
    sys.dont_write_bytecode=True
    sys.path.insert(0,str(root/'Source/Lighthaven/Visual/Player'))
    from player_receipt import native_helpers, signature as player_signature
    paths.append(root/'Source/Lighthaven/Visual/Player/player_receipt.py')
    digest=hashlib.sha256(b'npc-authored-v2\0')
    # The shared rig depends on player authored identity, never package serialization.
    digest.update(player_signature(root, (Path(os.environ['UE_ROOT'])/'Engine/Build/Build.version').read_text()).encode())
    digest.update(native_helpers(root/'Source/Lighthaven/Visual/Npc/LHNpcVisual.cpp',
        ('ULHNpcVisualComponent::SharePlayerRig',)))
    digest.update(native_helpers(root/'Source/Lighthaven/Visual/Player/LHPlayerVisual.cpp',
        ('ULHPlayerVisualComponent::CompactImportedMesh',)))
    for path in paths:
        label=str(path.relative_to(root)) if path.is_relative_to(root) else 'Engine/Build/Build.version'
        digest.update(label.encode()); digest.update(path.read_bytes())
    return digest.hexdigest()
def inventory(content):
    return {str(p.relative_to(content)):dict(bytes=p.stat().st_size,sha256=hashlib.sha256(p.read_bytes()).hexdigest()) for p in sorted(content.rglob('*.uasset'))}
