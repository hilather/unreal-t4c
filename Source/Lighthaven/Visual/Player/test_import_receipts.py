"""Run with python3 on a checkout with exported player and NPC inputs."""
import os
from pathlib import Path
import shutil
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
                              'artsource/player','artsource/npc','Content/Lighthaven/Art/Player'):
                shutil.copytree(ROOT/directory,root/directory)
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
            source=root/'artsource/player/output/player_a.glb'; source.write_bytes(source.read_bytes()+b'changed')
            self.assertNotEqual(player_signature(root,'test-engine'),after[0])
            package=next((root/'Content/Lighthaven/Art/Player').rglob('*.uasset'))
            package.write_bytes(package.read_bytes()+b'changed')
            self.assertNotEqual(npc_signature(root),after[1])

if __name__=='__main__': unittest.main()
