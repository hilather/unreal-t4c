"""Run with python3; no Unreal, Blender or exported inputs required."""
import os
from pathlib import Path
import shutil
import struct
import sys
import tempfile
import unittest
sys.dont_write_bytecode=True
from player_receipt import signature as player_signature
ROOT=Path(__file__).resolve().parents[4]
sys.path.insert(0,str(ROOT/'Source/Lighthaven/Visual/Npc'))
from npc_receipt import signature as npc_signature

class ReceiptInputs(unittest.TestCase):
    def test_content_addressed_dependencies(self):
        with tempfile.TemporaryDirectory(prefix='lh-receipt-') as temporary:
            root=Path(temporary)/'checkout'
            for directory in ('Source/Lighthaven/Visual/Player','Source/Lighthaven/Visual/Npc',
                              'artsource/player','artsource/npc'):
                shutil.copytree(ROOT/directory,root/directory,ignore=shutil.ignore_patterns('output', '__pycache__'))
            engine=Path(temporary)/'engine/Engine/Build/Build.version'
            engine.parent.mkdir(parents=True); engine.write_text('test-engine')
            os.environ['UE_ROOT']=str(engine.parents[2])
            def signatures(): return player_signature(root,'test-engine'),npc_signature(root)
            before=signatures()
            # An unrelated visual runtime edit and its compiled module must not invalidate art.
            for relative in ('Player/LHPlayerVisual.cpp','Npc/LHNpcVisual.cpp'):
                path=root/'Source/Lighthaven/Visual'/relative
                path.write_text(path.read_text()+'\n// unrelated runtime edit\n')
                os.utime(path,(100,100))
            binary=root/'Binaries/Linux/libUnrealEditor-Lighthaven.so'
            binary.parent.mkdir(parents=True); binary.write_bytes(b'unrelated rebuild')
            self.assertEqual(before,signatures())
            relocated=Path(temporary)/'other-checkout'; root.rename(relocated); root=relocated
            self.assertEqual(before,signatures())
            helper=root/'Source/Lighthaven/Visual/Player/LHPlayerVisual.cpp'
            text=helper.read_text(); helper.write_text(text.replace('bool ULHPlayerVisualComponent::CompactImportedMesh(',
                '// import change\nbool ULHPlayerVisualComponent::CompactImportedMesh(').replace(
                'Mesh->GetLODNum()!=1','Mesh->GetLODNum()!=2'))
            after=signatures()
            self.assertNotEqual(before[0],after[0]); self.assertNotEqual(before[1],after[1])
            self.assertNotEqual(player_signature(root,'other-engine'),after[0])
            # A fresh clone has no outputs. Recreated or changed generated files
            # must not invalidate either authored signature.
            stable=signatures()
            for relative in ('artsource/player/output/player_a.glb',
                             'artsource/npc/output/manifest.json',
                             'Content/Lighthaven/Art/Player/test.uasset'):
                source=root/relative; source.parent.mkdir(parents=True,exist_ok=True)
                source.write_bytes(struct.pack('<f', 1.0))
            self.assertEqual(stable,signatures())
            # Observed glTF TANGENT noise changed one float by about 0.0001.
            for relative in ('artsource/player/output/player_a.glb',
                             'artsource/npc/output/manifest.json',
                             'Content/Lighthaven/Art/Player/test.uasset'):
                (root/relative).write_bytes(struct.pack('<f', 1.0001))
            self.assertEqual(stable,signatures())
            authored=root/'artsource/player/models.py'
            authored.write_text(authored.read_text()+'\n# authored edit\n')
            changed=signatures()
            self.assertNotEqual(stable[0],changed[0]); self.assertNotEqual(stable[1],changed[1])
            authored=root/'artsource/npc/costumes.py'
            authored.write_text(authored.read_text()+'\n# authored edit\n')
            self.assertEqual(changed[0],signatures()[0])
            self.assertNotEqual(changed[1],signatures()[1])
            stable=signatures(); engine.write_text('other-engine')
            self.assertNotEqual(stable[1],signatures()[1])

if __name__=='__main__': unittest.main()
