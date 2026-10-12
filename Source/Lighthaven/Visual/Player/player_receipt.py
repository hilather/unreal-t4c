"""Content-addressed import inputs; runtime C++ and compiled modules are not inputs."""
import hashlib
from pathlib import Path


def native_helpers(path, names):
    """Hash complete, self-contained import hooks, ending at a column-zero brace.

    Fail closed if formatting changes. These hooks currently call only engine APIs;
    any new project helper must be added here as an explicit dependency.
    """
    text = path.read_text()
    result = []
    for name in names:
        marker = 'bool ' + name + '('
        if text.count(marker) != 1:
            raise RuntimeError('Review receipt native hook boundary: ' + name)
        start = text.index(marker)
        end = text.index('\n}', start) + 2
        result.append(text[start:end].encode())
    return b'\n'.join(result)


def signature(root, engine_version):
    # Versioned authored-input scheme: exports contain bake/export variability and
    # are disposable. Never use generated GLBs, PNGs or packages as input identity.
    paths = sorted((root/'artsource/player').glob('*.py'))
    paths += [root/'artsource/player/appearances.json',
              root/'Source/Lighthaven/Visual/Player/import_player.py',
              root/'Source/Lighthaven/Visual/Player/player_receipt.py',
              root/'Source/Lighthaven/Visual/Player/LHPlayerVisual.h']
    digest = hashlib.sha256(b'player-authored-v2\0' + engine_version.encode())
    for path in paths:
        digest.update(str(path.relative_to(root)).encode()); digest.update(path.read_bytes())
    digest.update(native_helpers(root/'Source/Lighthaven/Visual/Player/LHPlayerVisual.cpp',
        ('ULHPlayerVisualComponent::ConfigureImportedMesh', 'ULHPlayerVisualComponent::CompactImportedMesh')))
    return digest.hexdigest()
