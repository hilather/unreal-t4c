"""Source-only receipt invariants; no Unreal dependency."""
import json
import subprocess
import tempfile
import unittest
from pathlib import Path
from props_receipt import signature
class ReceiptTests(unittest.TestCase):
    def test_authored_inputs_and_relocation(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder)/'one'; root.mkdir()
            subprocess.run(['git','init','-q',str(root)],check=True)
            (root/'artsource/props/output').mkdir(parents=True)
            (root/'Source/Lighthaven/Visual/Props').mkdir(parents=True)
            (root/'build').mkdir()
            authored=root/'artsource/props/models.py'; authored.write_text('authored')
            script=root/'build/build-props-art.sh'; script.write_text('stage')
            subprocess.run(['git','add','artsource/props/models.py','build/build-props-art.sh'],cwd=root,check=True)
            engine={'MajorVersion':5,'MinorVersion':8,'PatchVersion':3}
            first=signature(root,engine)
            bytecode=root/'Source/Lighthaven/Visual/Props/generated.pyc'; bytecode.write_bytes(b'generated cache')
            subprocess.run(['git','add',str(bytecode.relative_to(root))],cwd=root,check=True)
            self.assertEqual(first,signature(root,engine))
            bytecode.write_bytes(b'changed generated cache'); self.assertEqual(first,signature(root,engine))
            (root/'Source/Lighthaven/Visual/Props/untracked.py').write_text('untracked')
            self.assertEqual(first,signature(root,engine))
            generated=root/'artsource/props/output/mesh.glb'; generated.write_bytes(b'export')
            generated.touch(); self.assertEqual(first,signature(root,engine))
            generated.write_bytes(b'different'); self.assertEqual(first,signature(root,engine))
            renamed=Path(folder)/'output'; root.rename(renamed)
            self.assertEqual(first,signature(renamed,engine))
            (renamed/'artsource/props/models.py').write_text('changed')
            self.assertNotEqual(first,signature(renamed,engine))
            self.assertNotEqual(first,signature(renamed,dict(engine,PatchVersion=4)))
if __name__=='__main__': unittest.main()
