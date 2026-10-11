"""Deterministic export cache. No receipt is emitted without a validated full roster."""
import hashlib, json, os, subprocess
from pathlib import Path
root=Path.cwd(); blender=Path(os.environ['BLENDER_ROOT'])/'blender'
sign=hashlib.sha256(subprocess.check_output([str(blender),'--version']))
inputs=sorted(Path('artsource/npc').glob('*.py'))+[Path('artsource/npc/roster.json')]+sorted(Path('artsource/player').glob('*.py'))
for p in inputs: sign.update(str(p).encode()); sign.update(p.read_bytes())
signature=sign.hexdigest(); output=Path('artsource/npc/output')
stamp=Path('Saved/ArtExport/npc-export-receipt.json')
def inventory():
    paths=[output/'manifest.json',output/'NPC_Textile_base.png',output/'NPC_Textile_normal.png',output/'NPC_Kit_A.glb',output/'NPC_Kit_B.glb']
    paths += [output/(n['id']+'.glb') for n in json.loads(Path('artsource/npc/roster.json').read_text())['npcs']]
    return {str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths if p.is_file()}
files=inventory(); current=dict(signature=signature,outputs=files)
unchanged=len(files)==20 and stamp.exists() and json.loads(stamp.read_text())==current
if unchanged: print('NPC_EXPORT_UNCHANGED',flush=True)
elif os.environ.get('LH_ART_SKIP_EXPORT')!='1':
    if stamp.exists(): stamp.unlink()
    config=root/'Saved/NpcBlenderConfig'; config.mkdir(parents=True,exist_ok=True)
    subprocess.run([str(blender),'--background','--factory-startup','--threads','8','--python-exit-code','1','--python','artsource/npc/build.py'],env=dict(os.environ,XDG_CONFIG_HOME=str(config)),check=True)
subprocess.run(['python3','artsource/npc/validate_glb.py'],check=True)
files=inventory()
if len(files)!=20: raise RuntimeError('Incomplete NPC export inventory')
if os.environ.get('LH_ART_SKIP_EXPORT')!='1' or unchanged:
    stamp.write_text(json.dumps(dict(signature=signature,outputs=files),indent=2)+'\n')
