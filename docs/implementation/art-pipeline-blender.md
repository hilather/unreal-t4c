# W5-07 Blender prerequisite and blocked handoff

Task ID: W5-07.

Base revision: `b1bfe7273a98bfd1a5b3992b63a4ea879771a3df`. Result revision: the submission commit containing this report (recorded in the submission receipt).

Contract revision: 1.

Owned paths changed: `docs/implementation/wave4/_env.md`, `docs/implementation/art-pipeline-blender.md`. Binary assets: none.

Behavior changed: none. Recorded the Blender installation and successful prerequisite check; implementation stopped at the source-path ownership checkpoint.

Source-backed mechanics / provisional tuning introduced: none.

## Check actually run

On 2026-10-10, uid 1000, ran the following from the attempt checkout. `OUTPUT` denotes this attempt's worker-output directory, not a shared output destination.

```sh
mkdir -p .blender-config "$OUTPUT/library"
XDG_CONFIG_HOME="$PWD/.blender-config" \
  /home/brewerm/Downloads/blender-5.2.2-linux-x64/blender \
  --background --factory-startup --python-expr \
  'import bpy; s=bpy.context.scene; s.render.engine="CYCLES"; s.cycles.device="CPU"; s.cycles.samples=1; s.render.resolution_x=64; s.render.resolution_y=64; s.render.resolution_percentage=100; s.render.filepath="/home/brewerm/.herdr-projects/unreal-t4c/.state/worker-output/attempt-0dd0c3762e68f8edaf2e894536d1a079548915d6a3ddaa8fa0c29bc4ad7b6c5b/library/blender-smoke.png"; bpy.ops.render.render(write_still=True)'
```

Observed exit 0, render timestamp 1.780 seconds, PNG saved. Version banner: Blender 5.2.2 LTS, hash `d13f752e3b9c`, built 2026-09-15. Log contains two `socket(): Operation not permitted` messages. This demonstrates a tiny CPU render only; it does not demonstrate baking, export, import or visual quality.

Evidence directory: `/home/brewerm/.herdr-projects/unreal-t4c/.state/worker-output/attempt-0dd0c3762e68f8edaf2e894536d1a079548915d6a3ddaa8fa0c29bc4ad7b6c5b/library/`.

Renders list: `blender-smoke.png` (factory-startup cube, 64×64 prerequisite check). Log: `blender-smoke.log`. No concept comparison renders produced.

## Required ownership checkpoint

The retained lowercase-project-directories instruction says to use `artsource/` wherever the plan says `ArtSource/`; the checkout already contains `artsource/concepts/README.md`. The task's ownership list and mandatory submission script permit only `ArtSource/Blender/`, excluding `artsource/Blender/`. The worker instruction says: “If the right change needs a file outside them, stop and say so in your report.”

Assumption: the lowercase instruction remains applicable because the task does not expressly revoke it. Neither creating the uppercase sibling nor writing an unowned lowercase path resolves both constraints. Stopped before creating either directory. No memory projection was edited; canonical receipts query returned `[]`.

## Checks not run and remaining work

Both Unreal target builds, full headless suite, map generation, review-arrivals 9/9, 15-camera framing, Dressing.Maps and asset-resolution tests were not run: no implementation was made after the required ownership checkpoint. These are unverified, not passed. No `.umap`, `.uasset` or reviewed-arrival file was changed.

Kit generation, procedural shader baking, triangle budgets, glTF export, engine import discovery/tooling, B1 code mapping and cook guarantee remain unimplemented. No imported meshes/textures exist from this attempt; per-asset bytes and the 15 MB pilot budget cannot yet be assessed. No gameplay, collision, nav, lighting, exposure or post-process behavior changed.

Next task and integration notes: coordinator reconciles the owned source path and submission scope with the lowercase policy, then resumes W5-07 from the passing Blender prerequisite. Visual creation/review must use the prescribed codex-astra visual lane; this attempt ran only the mechanical prerequisite under codex-sol. Do not treat this blocked evidence candidate as completion of the pilot or a gate pass.
