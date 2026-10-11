"""Stage changed packages before native CDO hard references root their assets."""
import json,sys,time,shutil
from pathlib import Path
sys.dont_write_bytecode=True
from npc_receipt import signature,inventory
root=Path.cwd(); content=root/'Content/Lighthaven/Art/Npc'
stamp=content/'import-receipt.json'
if stamp.exists():
    old=json.loads(stamp.read_text())
    if old['signature']==signature(root) and old['assets'] and old['assets']==inventory(content):
        print('NPC_PACKAGES_UNCHANGED'); raise SystemExit(0)
if content.exists():
    backup=root/'Saved/ArtExport/npc-reimport-backup'/str(time.time_ns())
    backup.parent.mkdir(parents=True,exist_ok=True)
    shutil.move(str(content),str(backup))
    print('NPC_CHANGED_PACKAGES_STAGED '+str(backup))
