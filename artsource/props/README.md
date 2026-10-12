# W5-23 traveller props and mesh effects

Original scripted Blender source. All dimensions, pigments, icon compositions and
suggested effect envelopes are **Prototype presentation tuning**, W5-23,
LH_Prototype_v1, 2026-10-11, source URL null. No game rules are implemented here.
No downloaded assets or concept pixels enter the meshes, atlas or icons.

Run serially from the checkout root with Blender **5.2.2 LTS / d13f752e3b9c**:

```sh
mkdir -p artsource/props/.config artsource/props/output
BLENDER_ROOT=/home/brewerm/Downloads/blender-5.2.2-linux-x64
XDG_CONFIG_HOME="$PWD/artsource/props/.config" "$BLENDER_ROOT/blender" \
  --background --factory-startup --threads 8 --python-exit-code 1 \
  --python artsource/props/build.py
XDG_CONFIG_HOME="$PWD/artsource/props/.config" "$BLENDER_ROOT/blender" \
  --background --factory-startup --threads 8 --python-exit-code 1 \
  --python artsource/props/preview.py -- --view all
python3 artsource/props/validate_glb.py
XDG_CONFIG_HOME="$PWD/artsource/props/.config" "$BLENDER_ROOT/blender" \
  --background --factory-startup --threads 8 --python-exit-code 1 \
  --python artsource/props/validate_blender.py
```

`build.py -- --output artsource/props/output-repeat` independently rebuilds all
meshes and shared textures. Compare them with:

```sh
python3 artsource/props/validate_glb.py --meshes-only \
  --compare artsource/props/output-repeat
```

Both builders default to ignored `artsource/props/output/`. Fixed construction,
Cycles and scatter seed: **523**. GLBs reference the three external
`textures/props_{base,normal,orm}.png` images: **move the whole output directory**.
There are no embedded images, animation clips, skins, collision or lights.
The normal atlas is tangent-space OpenGL +Y; ORM is constant neutral AO=1,
roughness and metallic by palette tile. It is not an object AO bake.

`preview.py -- --view icons|items|effects|icon-sheet|rig` regenerates individual
sets without rebuilding. The icons are 128×128 transparent RGBA, all using the
same orthographic camera direction, lights, exposure and 32 CPU Cycles samples.
The icon contact sheet includes actual 36px display sizes. The item and effects
sheets are 1600×1000, CPU Cycles 32 samples. Rig evidence uses the existing
`artsource/player/` source geometry and pose functions, plus **reimported prop
GLBs**; the player is an explicitly unbaked source preview. Each of its four
800×500 panels uses a 12m perspective camera, 55° pitch, 45° FOV and 45° yaw
(the quiver panel reverses yaw). This is Blender evidence, not Unreal play.
Temporary EXRs used to assemble the rig sheet are removed on normal completion.

The normal output is 28 PNGs: 21 icons, three shared textures, four sheets.
At most two comparison renders may be retained (30 total). Generated `.glb`,
`.png`, `.blend`, logs and configuration are ignored, never committed. There is
no dependency on pip packages or external image tools. Pure Python validation
needs only Python 3's standard library; construction/rendering needs Blender.

See [the importer handoff](../../docs/implementation/art-props-blender.md) for
asset sizes, pivots, socket conventions, effect/material intent, review findings
and checks that require the subsequent Unreal import task.
