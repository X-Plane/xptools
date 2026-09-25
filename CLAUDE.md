# XPTools / WED — Claude Quick Reference

This repo builds **WorldEditor (WED)** plus several other Laminar Research scenery
tools (RenderFarm, MeshTool, DSF2Text, ObjView, …). The primary focus here is WED.

## Where to look first

- **`WED_Architecture.md`** — developer reference for the WED codebase. Subsystem map,
  class-level descriptions, key interfaces, how-to recipes (add an entity, add a map
  tool, trace a UI bug), suggested reading order. Read this before touching WED code.
- **`Building.md`** — build setup and dev environment.
- **`README.md`** — top-level project overview and licensing.
- **`src/README.txt`** — legacy package overview. Useful for historical context
  (especially the RenderFarm / WorldEditor / GISTool naming history) but partly stale.
- **`src/WEDDocs/`** — source for the *user-facing* WED manual (markdown → HTML).
  Not developer documentation.

## Source layout (WED-specific)

WED-relevant directories all live under `src/` and are prefixed `WED*`. See
`WED_Architecture.md` §2 for the full table. The most-touched ones are
`WEDCore/`, `WEDEntities/`, `WEDMap/`, `WEDProperties/`, `WEDWindows/`,
`WEDImportExport/`. Cross-cutting abstract interfaces live in `Interfaces/`,
and the portable widget framework in `GUI/`.

## Branch conventions

- `master` — current development; kept release-ready when possible.
- `wed_<NNN>_release` — staging / patch branches for a specific WED version
  (e.g. current branch `wed_270_release` for WED 2.7.0 line).
- Tags like `wed_271r1` mark public releases.

## When in doubt

If `WED_Architecture.md` and the source disagree, **the source wins** — the doc is
maintained by hand and may drift. Fix the doc in place when you spot drift.
