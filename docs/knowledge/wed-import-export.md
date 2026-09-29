# WED Import / Export (apt.dat, DSF, Scenery Pack, Ortho, Gateway)

> Source: `src/WEDImportExport/`, plus the apt.dat reader/writer WED borrows from
> `src/XESCore/AptIO.{h,cpp}` and `src/XESCore/AptDefs.h`. Per-entity field mapping lives in each
> entity's `Import()`/`Export()` in `src/WEDEntities/` (see [wed-entities.md](wed-entities.md)).

## Things That Will Bite You

### The export target is a global, it is sticky, and some code switches it for you

- **`gExportTarget` (`WEDCore/WED_Globals.h`) is one process-wide global**, not a document
  property. `WED_DocumentWindow` loads it from the doc pref `doc/export_target` when a document
  is created or loaded, and writes it back on save. `WED_Document::ReadIntPref` ignores the
  *global* value of `doc/export_target` on purpose, so new documents always start at
  `wet_latest_xplane`. An unknown value (above `wet_latest_xplane` and not `wet_gateway`, e.g.
  from a newer WED) is clamped only in the `msg_DocLoaded` handler, not in the
  `WED_DocumentWindow` constructor, which is the path taken when a document is first opened.
  `GATEWAY_IMPORT_MODE` builds ignore the pref and always use `wet_latest_xplane`.
- **`wet_gateway = 99` sits above every X-Plane target**, so `gExportTarget >= wet_xplane_1200`
  is true for gateway and `< wet_xplane_1200` is false. Most code relies on this ("gateway means
  latest format plus strict checks"). A new `>`/`>=` test that is meant for real X-Plane versions
  only also fires for gateway. `encode_spacing()` in `WED_DSFExport.cpp` uses
  `> wet_xplane_1200`, so gateway output gets the 12.1.2 string-spacing encoding.
- **The target is saved in the document as a raw integer.** The `wet_xplane_*` values must stay
  contiguous and in the same order as the `wed_Export900…wed_Export1212` menu commands
  (`WED_DocumentWindow` computes `wet_xplane_900 + (cmd - wed_Export900)`) and as
  `kExportTargetMenu` in `WED_Menus.cpp`. Inserting a target in the middle silently moves every
  saved document to a different target. To add a target, append it before `wet_gateway`, move
  `wet_latest_xplane`, add a menu row, and add a `case` in `get_apt_export_version()`
  (`WED_AptIE.cpp`). That function hits `DebugAssert` on an unknown target and falls back to
  1000. `WriteAptFileProcs` also asserts the version is one of 850/1000/1050/1100/1130/1200.
- **Code that silently switches the target to `wet_gateway`:** a successful Gateway import
  (`WED_GatewayImportDialog::TimerFired`), a Gateway upload whose validation fails (left at
  gateway on purpose, see the comment in `WED_GatewayExportDialog::Submit`), and
  `WED_DoImportExtracts`. After a Gateway import, a plain "Export Scenery Pack" runs the gateway
  heuristics and the strict validation.
- `wet_xplane_1212` ("X-Plane 12.1.2") changes **no apt.dat output**, because it maps to apt 1200.
  It exists only for the fractional string-spacing DSF encoding.

### apt.dat round-trip is lossy by design — know what gets normalized

- **Taxi-route topology is rewritten on every import.** In `WED_AptImport`, each 1202/1206 edge
  that has 1203 shape points is split into several `WED_TaxiRouteNode`/`WED_TaxiRoute` pieces
  (one new node per shape point). On export, node IDs are renumbered in hierarchy order
  (`MakeNodeRouting`), every 1201 node is written with type `both`, and only nodes that touch a
  **visible** edge are written (WED-824).
- **Curved taxi routes do not round-trip.** `HAS_CURVED_ATC_ROUTE` is 0 in `Obj/XDefs.h`. If a
  `WED_TaxiRoute` has bezier handles, `MakeEdgeRouting` still pushes the control points and the
  writer emits them as plain **1203 shape points**. On re-import they become real vertices (more
  split edges). 1205 rows are silently dropped on read.
- **Legacy ramp-start row trap.** `WriteAptFileProcs` writes the old row **15** (location,
  heading and name only, with **no 1301 row**) when a ramp is type `misc` *and* equipment is
  `all`, when equipment is empty, or when the target is below 1000. That drops size, operation
  type and airlines. Otherwise it writes 1300, plus 1301 when the version is 1050 or later.
  **Deliberate (Ben, 2026-09-25)** — legacy compatibility; don't "modernize" it to 1300.
  Airline codes are lowercased on export (`WED_RampPosition::CorrectAirlinesString`).
- **`WED_RampPosition::Import` bug:** on an illegal width it assigns `ramp_type = width_E`
  instead of `width`, so the width stays -1.
- **Jetways split on user round-trip.** For targets 12.00 and later, a facade whose ring ends in a
  docking cabin (`WED_FacadePlacement::HasDockingCabin`, which returns false below
  `wet_xplane_1200`) has its last two segments popped off in the DSF, and the tunnel/cabin is
  written as an apt.dat 1500 row (`ExportJetway`). Re-importing a user pack produces **two**
  things: the building facade without its tunnel (from the DSF) and a separate jetway facade in
  a "Jetways" group (from `ImportJetway`). Gateway uploads avoid this by passing
  `DockingJetways=false` to both `WED_AptExport` and `DSF_ExportAirportOverlay` (commit 48cb98d20).
- **Jetway style codes are the facade's `#cabin <n>` value**, read by `WED_ResourceMgr` into
  `fac_info_t::style_code` and written verbatim into row 1500. X-Plane's codes (confirmed by Ben,
  2026-09-25): 0 = light_solid (`1s`, `Jetway_1_solid.fac`), 1 = light_glass (`1g`), 2 = dark_solid
  (`2s`), 3 = dark_glass (`2g`). The hard-coded fallback table in `WED_FacadePlacement::ImportJetway`
  (used when the row has no vpath) is **wrong for every code** (0→1_glass, 1→1_solid, 2→2_glass,
  3→1_solid), so a vpath-less round trip swaps glass/solid — on [the punch list](../bug-punch-list.md), high.
- **Airport-level normalizations** (`WED_Airport::Import/Export`): the `flatten` and
  `drive_on_left` 1302 keys are pulled out of the metadata into bool properties. Presence alone
  means true, whatever the value, and they are written back as `… 1` at the end of the
  metadata. Elevation is stored in meters and written as a truncated `int` of feet. The has-ATC
  flag in row 1 is written only for 850. `faa_code`, `iata_code`, `icao_code` and `region_code`
  values are uppercased at write time (`WriteAptFileProcs`). A duplicate metadata key is only
  logged on import. Both copies are kept.
- **Row 105 (runway skids and marking size) is read by AptIO but ignored by WED** (`ROWCODE_105 0`).
  It is lost on the next export.
- **Scenery ID is not in apt.dat.** `WED_Airport::scenery_id` survives only in the `.earth.wed.xml`
  (and is set by Gateway import and by a successful upload). Importing the same airport from a
  pack on disk gives -1. That means a null `parentId` on upload, and the "pre-94010" XP12-reset
  heuristics treat the airport as old.
- **The apt.dat reader is all-or-nothing.** `ReadAptFileMem` stops at the first bad row: a row
  code not allowed for the header version, a second 1301 for one gate, 1302 before any airport
  (no guard, UB), and so on. WED then refuses the **whole file**. The 1302 parser assumes exactly
  `"1302 "` (5 chars). A key without a value gets its value set to the key text. Hierarchy
  buckets are deleted when empty only if taxi-routing checks passed (`if(apt_ok)` wraps the
  cleanup), so a broken airport keeps its empty groups.

### Version gating and precision in `WriteAptFileProcs` (XESCore/AptIO.cpp)

| Target → apt ver | What changes |
|---|---|
| < 1000 (XP9) | no 1300/1301, no ATC flows (1000–1110), no taxi routes |
| < 1050 | no 1301, taxi edge width suffix (`taxiway_A…`) dropped |
| < 1100 | no 1206 truck edges, no 1400/1401 |
| < 1130 | freqs written as old 50–56 codes with `freq/10` (8.33 kHz and 1 kHz precision lost), 1100 instead of 1110, XP11.26+ line styles mapped to "classic" ones |
| < 1200 | pavement types 20–39 become asphalt 1, 50–59 become concrete 2 (`backport_pave_type`), ESA markings 6/7 become 2/3, REIL codes above 2 become 0, A-PAPI light codes shifted down, no 1402 truck vpath, no 1500/1501 jetways |

Fixed formats that quietly round: runway displaced threshold and blast pad lengths are `%.0f`
(whole meters), gate/sign/helipad/taxiway headings are `%.1f`, tower height is `%.0f` ft.
Lat/lon is `% 012.8lf`. In `GATEWAY_IMPORT_MODE` builds only, it is `%.7lf` (`LLFMT`) or
`%.6lf` for "dynamic" rows (`LLFMT2`), and names are dropped from most rows (`NFMT`/`N()`).

### DSF export: tiling, clipping and the "safe bounds"

- **Tiling depends on the target.** For `wet_xplane_1021` and later, facades are **not clipped**.
  The whole facade goes into the one tile containing its **bbox centroid**, and points may lie up
  to `DSF_EXTRA_1021` (0.25°) outside the tile (`safe_bounds`). Below 1021 facades are
  `clip_polygon`/`clip_segments`-cut to the tile. **Forests, draped polygons, orthophotos, lines,
  strings, autogen and exclusion polygons are always clipped** to the tile, whatever the target.
  Objects go to the tile containing their point.
- **A point is clamped whenever it falls outside `safe_bounds`** (`gentle_crop` sets
  `g_dropped_pts`). In that case `DSF_Export` shows the "bezier curves cross a DSF tile boundary
  … X-Plane 9" alert, even for non-bezier geometry on 10.21+ targets, and **returns -1**.
  `WED_ExportPackToPath` then returns **before writing apt.dat**, while the DSFs are already on disk.
- **Non-object/facade primitives are emitted only in the `show_level == 6` pass.**
  `DSF_ExportTile` walks the tree six times (show levels 6 down to 1). Objects and facades match
  on their own show level. Everything else sits inside `if(show_level == 6)`. A new primitive type
  added outside that block is written six times.
- **Exclusions inside an airport become per-airport** (they follow that airport's
  `sim/filter/aptid` property in `write_tables`). The filter ID is uppercased because X-Plane
  compares filters case-sensitively against its uppercased apt.dat IDs (`accum_filter`).
- **Precedence bugs that swallow aborts.** In `DSF_ExportTileRecursive`, both
  `if (int result = WED_ExportTerrObj(...) < 0) return result;` and the matching
  `WED_ExportOrtho` line assign the *bool* `(x < 0)`, so a failure returns 1 ("one entity"), not -1.
  The user gets "aborting DSF Export" alerts, but the export keeps going. Intended behavior is
  to abort (Ben, 2026-09-25); on [the punch list](../bug-punch-list.md), low priority.
- **Stale-DSF auto-deletion** (`DSF_export_info_t` in `WED_OrthoExport.{h,cpp}`). The doc pref
  `export/last` remembers the DSFs written last time, capped at about 200 characters (roughly 10
  files; beyond that they are not remembered). The destructor **deletes any previously written
  DSF not written again this time**, under the *document's own* package `Earth nav data/`. It runs
  even after an aborted export. [Needs Runtime — Ben unsure; repro recipe on [the punch list](../bug-punch-list.md)] `WED_GatewayExportDialog::Submit` builds its
  preview pack through `WED_ExportPackToPath` → `DSF_Export(…, mResolver, temp folder…)`. That
  constructs `DSF_export_info_t(resolver)`, so a single-airport upload appears to delete the
  user's other exported DSFs from their real package and reset `export/last`.
- **Parameter encodings that the importer must mirror**:
  - Draped polygon heading: `encode_heading` → `whole_deg + 360*(int)(128*frac)`, which gives
    1/128° resolution. Flatten polygons (`WED_LibraryMgr::CheckFlattenPolygon`) take the raw heading
    on both sides.
  - Orthophoto: param `65535`, coord depth 4 or 8 (UV).
  - Forest: `density*255 + fill_mode*256`.
  - Autogen: `(round(height/4) << 8) | spelling_or_spawning_count`, so heights are quantized to
    4 m.
  - Facade: height (integer), with coord depth 3/5 when it has custom walls.
  - String spacing: for 12.1.2 and gateway targets, `10000 + round(128*s)` when not near a whole
    meter and s < 433.8. Otherwise the value is **truncated** to an int.
    **`WED_DSFImport` does not decode the 10000+ form**: it calls `SetSpacing(inParam)` raw, so a
    re-imported fractional string has spacing ≥ 10000 m. The preview (`WED_PreviewLayer`) uses
    `< wet_xplane_1200` for integer spacing while export uses `> wet_xplane_1200`, so they
    disagree at exactly 12.00. Both on [the punch list](../bug-punch-list.md), high priority (Ben, 2026-09-25).
- **Object MSL is clamped** to [-500, 10000] m (XPD-15378).

### DSF import quirks

- Exclusions are created from DSF *properties*, which come before any filter, so
  `DSF_Import_Partial(…, ICAO)` imports **every** exclusion in the file. `WED_SceneryImport`
  works around this: it imports per airport with `dsf_filter_all - dsf_filter_exclusion`, then
  once more with `""` (off-airport). It then re-parents `WED_ExclusionZone`s into airports by bbox
  overlap. `WED_ExclusionPoly` is **not** re-parented.
- Imported content lands in named groups from `k_dsf_cat_names`. The Gateway heuristics in
  `WED_SceneryPackExport.cpp` find the `"Terrain FX"` and `"Pavement FX"` groups **by name**
  (with an `IHasResource` scan outside `GATEWAY_IMPORT_MODE`). Renaming those groups changes
  heuristic behavior.
- `DSF_Import` alerts when `sim/overlay` is not 1. `DSF_Import_Partial` does not.
- Imported backslash vpaths are normalized to `/`.

### Gateway

- **What `has_dsf()` counts decides whether `ICAO.txt` is uploaded.** `k_dsf_classes` in
  `WED_GatewayExport.cpp` lists obj, facade, forest, exclusion zone, ortho, line, string and
  polygon. It leaves out `WED_ExclusionPoly`, `WED_AutogenPlacement`, `WED_TerPlacement` and
  `WED_RoadEdge`. An airport whose only DSF content is one of those uploads **no DSF**, yet it
  still advertises the `ROADS_TAG` feature. A bug (Ben, 2026-09-25) — on [the punch list](../bug-punch-list.md), low priority. `k_3d_classes` (which drives the "3D"/"2D" type and
  the `gui_label`) is obj, facade, forest and string.
- **Only known metadata keys reach the Gateway JSON.** `additionalMetadata` iterates
  `wed_AddMetaDataBegin+1 … End`. Unknown 1302 keys still go into the uploaded apt.dat, but not
  into the JSON.
- **The metadata-key enum is order-coupled.** The `wed_AddMetaData*` command IDs in `WED_Menus.h`
  must match `known_keys[]` in `WED_MetaDataKeys.cpp` **exactly, in alphabetical order**,
  including the `#if GATEWAY_IMPORT_FEATURES` `gw_credits` entry on both sides.
  `WED_Airport::FindProperty` looks keys up by **display text**, so renaming a display string
  (for example the "Local Authorithy" fix in 43843fce4) changes property identity.
- **`fill_in_airport_metadata_defaults` gives up early** when `icao_code` and `faa_code` are empty
  but `local_code` is set. It returns false *before* `Enforce_MetaDataGuiLabel`, so such airports
  never get `gui_label` enforced on that path. Its CSV lookup order is `icao_code` meta, then
  `faa_code`, then the airport ID. It only fills keys that are missing or empty, never overwrites.
- **Upload mutates the document before validation.** `Submit()` commits an "Update metadata"
  operation (CSV defaults and `gui_label`) *before* `WED_ValidateApt`, and the operation stays
  committed if validation fails.
- **Null-deref traps** (both confirmed 2026-09-25, on [the punch list](../bug-punch-list.md)):
  - `WED_GatewayImportDialog::ImportSpecificVersion` calls `g->CountChildren()` even when no
    airport was read (`g == NULL`). This regressed in 6b835b390; 584407802 had fixed it earlier.
  - The `mAirportMetadataCURLHandle` members of both `WED_GatewayExportDialog` and
    `WED_UpdateMetadataDialog` are never assigned, yet are dereferenced on
    `cache_status_error`, so a failed CSV download crashes (fix and the other status-handling
    gaps: [wed-network-and-filecache.md](wed-network-and-filecache.md#what-consumers-do-with-each-status)).
- `WED_DoInvisibleUpdateMetadata` **busy-waits** on `gFileCache.request_file` on the main thread
  the first time it runs.
- **`add_iso3166_country_metadata` has bad table entries**: `EK` becomes `"DAN "` (should be DNK)
  and `LK` becomes `"SWI "` (LK is Czechia). Its fallback reads `apt.GetName()` (the airport
  *name*) where the comment says airport ID. When nothing matches it still adds an empty
  `country` key and returns true. On [the punch list](../bug-punch-list.md), low priority.
- The **94010 scenery-ID cutoff** ("XP12-era submission") is hardcoded in both
  `WED_GatewayImport.cpp` (prompt to delete exclusion zones and `flatten`) and
  `WED_SceneryPackExport.cpp` (the `GATEWAY_IMPORT_MODE` reset).

### Gateway-target export mutates, then undoes

In a normal build, `WED_DoExportPack` with `gExportTarget == wet_gateway` does four things in
order: `MarkUndo()`, `DoHueristicAnalysisAndAutoUpgrade()` (one `StartCommand`), export, then
`UndoToMark()`. If undo cannot rewind, the user is told the scenery was permanently altered. The
heuristics that run in a normal build are:

- illegal ICAO → `local_code`, and a `local_code` guard when the airport ID is not a legal ICAO
- oil-rig tagging
- iso3166 country prefix
- `wed_upgrade_ramps`, seeded by airport location so the output is repeatable (its `rand()&1`
  half-split erased airlines until 51ce190ca)
- leading-zero runway names removed inside the FAA box
- `WED_DoConvertToJW`

Everything after `#if GATEWAY_IMPORT_MODE` runs only in LR's bulk Global Airports build: grass
mowing, trees to forests, AG_ forests to 3D, the XP12 reset (delete terrain polys, grunges,
exclusions and flatten), and `allows_circuits=0` on one-way patterns. When changing a heuristic,
check which side of that `#if` it is on. Normal builds also run `WED_ValidateApt` before any
pack export, and `GATEWAY_IMPORT_MODE` builds skip it.

### Small latent bugs worth knowing before touching the code

- `escape()` in `WED_SceneryPackExport.cpp` (KML/OSM): `*b & 0xC0 == 0xC0` parses as `*b & 1`.
  Any odd byte copies the *next* byte unescaped, so `a<b` produces raw `<`, and the check can
  read one past the end.
- `WED_DoImportScenery` compares against `sizeof("Earth nav data")`, which counts the NUL and so
  is off by one. Picking the `Earth nav data` folder itself appends it a second time and fails.

## Architecture

**Data flow.** WED never writes apt.dat text itself.
`AptExportRecursive` (WED_AptIE) walks the tree (hidden entities and their subtrees are skipped;
`WED_OverlayImage` is never recursed). Each entity's `Export()` fills an `AptInfo_t`
(`XESCore/AptDefs.h`), and `WriteAptFile`/`WriteAptFileProcs` (`XESCore/AptIO.cpp`) formats it
for a version from `get_apt_export_version()`. Import runs the other way:
`ReadAptFile` → `CheckATCRouting` → `ConvertForward` (upgrades legacy row-10 pavements) →
`WED_AptImport`. `WED_AptImport` creates the airport, then fixed buckets ("Runways", "Taxiways",
"Ramp Starts", …), then entities via `Import()`, then the taxi graph. Linear features go
through `ImportLinearPath`/`ExportLinearPath`. apt.dat cannot express a split bezier, so a split
node is written as 2–3 co-located rows (curve / segment / curve), and the importer collapses them
back. See the "Ben says" comment in `ImportLinearPath`. Co-located **non**-bezier nodes are
deliberately kept (WED-787).

DSF export (`DSF_Export` → `DSF_ExportTile` per 1°×1° tile over the world bbox →
`DSF_ExportTileRecursive`) writes through the `DSFCallbacks_t` writer from `DSF/DSFLib`. Only
`WED_Airport` and `WED_Group` are recursed into. `DSF_HeightRangeRecursive` pre-culls empty
tiles and picks the MSL range. Empty tiles are not written. `DSF_ExportAirportOverlay` reuses the
same recursion with world-sized bounds and the DSF2Text writer to produce the Gateway `ICAO.txt`.
DSF import (`DSF_Importer` in WED_DSFImport) is the matching callback sink. Show levels come back
from `sim/require_*` properties (`GetShowForObjID`/`GetShowForFacID`).

Orthophoto export (`WED_ExportOrtho`) runs in the middle of DSF export for any
`WED_DrapedOrthophoto` whose resource is still an image (`IsNew()`). It cuts the UV-bounded part
of the source image into `<dir>/<polygon name>.dds` (or `.png` when `gOrthoExport` is 0),
regenerating only when the source image is not older than the output. It writes a `.pol`
**only if none exists**, so later decal or other changes do not update an existing `.pol`. It then
temporarily rescales UVs inside a `StartOperation("Norm Ortho")` that the DSF exporter later
`AbortOperation()`s. Because regeneration is keyed on file dates only, reshaping the polygon
(new UV crop) with an unchanged source image leaves a stale DDS crop / `.pol` — on [the punch list](../bug-punch-list.md),
high priority (Ben, 2026-09-25). The
same file also holds `WED_ExportTerrObj` (terrain OBJ from DEM) and `DSF_export_info_t`.

**Scenery pack export.** `WED_DoExportPack` → `WED_ExportPackToPath` runs DSFs first, then
`Earth nav data/apt.dat`, then (normal builds) `doc.kml`/`doc.osm` from `WED_ShapePlacement`s.

**Gateway upload** (`WED_GatewayExportDialog`):

1. Download the metadata CSV through `gFileCache`.
2. Fill metadata defaults.
3. Force `wet_gateway` and run `WED_ValidateApt(…, apt, true)`. Warnings get a confirm; errors
   abort.
4. In a temp dir, write `ICAO.dat` (`DockingJetways=false`) and `ICAO.txt`, but only if
   `has_dsf`.
5. Build `ICAO_Scenery_Pack/` with **`wet_latest_xplane`** (README, and COPYING from the GUI
   resource), and zip it inside the master zip.
6. Base64 the master zip into `masterZipBlob`.
7. PUT `{scenery:{…}}` to `<api>/scenery` (`curl_http_get_file` with the body as its
   put argument, not through `gFileCache`). The API URL can be overridden with
   `--gateway_api_url`.
8. On success, store the returned `sceneryId`.

Other JSON fields: `additionalMetadata`, `aptName`, `artistComments`, `clientVersion`
(`WED_VERSION_NUMERIC`, which the Gateway uses to detect outdated WED), `latitude`/`longitude`
(bbox centroid), `features` (1 flow, 2 taxi routes, 8 truck routes, 86 roads), `validatedAgainst`,
`icao`, `parentId`, `type` 2D/3D, `userId`, `password`. The export target is restored at the end.

**Gateway import** (`WED_GatewayImportDialog`): metadata CSV → `<api>/airports` → per-airport
`<api>/airport/ICAO` versions → `<api>/scenery/<id>` for each selection. All of these go through
`gFileCache`; see [wed-network-and-filecache.md](wed-network-and-filecache.md). The ICAO list
reuses `AptInfo_t` as a row model: `meta_data[0]` is a pseudo key `IcaoFaaLocal`, a second pair
holds checkout/artist info, and `kind_code` holds the recommended scenery ID. `WED_AptTable`
reads those positions. All selected versions import inside one "Import Scenery Pack" operation
(any failure aborts all). For each one, the blob is decoded into the package dir,
`ICAO.dat` is imported with `WED_ImportOneAptFile`, and `ICAO.txt` with `WED_ImportText` *into
the airport*. Then iso3166 country, invisible metadata update, the optional pre-94010 exclusion
and flatten purge, and `gExportTarget = wet_gateway`.

`WED_DoImportExtracts` (GATEWAY_IMPORT_FEATURES) is LR's bulk loader for Global Airports. It
buckets airports into latitude groups because a flat list of about 40,000 airports made per-tile
culling dominate export time (a 5× speed-up, per the comment). It is split by hemisphere so two
WED instances can export in parallel.

## Connections to Other Systems

- **Design rules** ([wed-design-principles.md](wed-design-principles.md)): the stated rules for new code and reviews that this subsystem's conventions should follow — command ownership, pointer vs ID, class vs interface casts, error handling, layering, per-doc prefs, platform reference.
- [wed-entities.md](wed-entities.md) — every entity's `Import(AptX_t, print_func, ref)` /
  `Export(AptX_t&)`. Field-level rounding and enum mapping (`ENUM_Import`/`ENUM_Export`) live there.
- [wed-validation.md](wed-validation.md) — `WED_ValidateApt` gates every export. Many rules
  switch on `gExportTarget` (gateway turns warnings into errors).
- [wed-core-services.md](wed-core-services.md) — document prefs (`doc/export_target`,
  `export/last`), `ILibrarian`/`gPackageMgr` path resolution, and the `Rescan(true)` /
  `Purge(vpath)` / `DropTexture` calls ortho export makes after writing assets.
- [utils-geometry.md](utils-geometry.md) — `WED_Clipping` (`clip_polygon`, `clip_segments`) and
  its winding and exact-equality rules, the bezier "triple" notation that DSF export writes and
  that DSF import collapses (co-located vertices are lost), and the GeoTIFF control-point grid
  used by ortho placement.
- [utils-platform-and-files.md](utils-platform-and-files.md) — the Windows `mkdtemp` shim that
  makes gateway export always use a literal `tempXXXXXX` folder, `FILE_compress_dir`,
  `FileSet_Open` for gateway zips, `XObj8Write` for terrain OBJs, and `FILE_find_dsfs`.
- [gui-framework.md](gui-framework.md) — the import/export dialogs are modal `GUI_Window`s
  (apt.dat and Gateway import) or `GUI_FormWindow`s (Gateway upload, metadata update). They are
  written as callbacks because the modal constructor returns immediately, the network ones poll
  from a `GUI_Timer`, and they close with `AsyncDestroy()`. Read its `Closed()` and
  `GUI_Destroyable` notes before changing how a dialog tears down.
- [wed-map-and-tce.md](wed-map-and-tce.md) — `WED_AptImportDialog` and
  `WED_GatewayImportDialog` hold the `WED_MapPane*` and call `ZoomShowSel()` after importing.
- [wed-network-and-filecache.md](wed-network-and-filecache.md) — `gFileCache` domains used for the
  CSV, airport list, versions and scenery blobs, and `curl_http_get_file` for upload.
- [dsf-library-and-dsftool.md](dsf-library-and-dsftool.md) — `DSFCallbacks_t` writer and reader,
  DSF2Text (used both for the Gateway `ICAO.txt` and for `.txt` DSF import).
- [bitmap-texture-and-ddstool.md](bitmap-texture-and-ddstool.md) — `WriteBitmapToDDS_MT`, BC1/BC3
  choice (`hasPartialTransparency`), bitmap loading for ortho export.
- [wed-ui-panes.md](wed-ui-panes.md) — menu wiring (`WED_DocumentWindow`), the property pane's
  `SetClosed` after Gateway import, metadata properties on `WED_Airport`.
- [wed-object-model.md](wed-object-model.md) — undo (`MarkUndo`/`UndoToMark`, the operations
  around imports), and hierarchy traversal (`CollectRecursive`, hidden flags) that decides what
  gets exported.
- `WEDWindows/WED_GroupCommands.cpp` supplies the heuristics called from here
  (`wed_upgrade_ramps`, `WED_DoConvertToJW`, `WED_DoMowGrass`, `WED_DoConvertToForest`).
