# Bug Punch List

Bugs confirmed (or suspected and worth verifying) during knowledge-base compilation and
triage. Not fixed yet. Each entry links to the knowledge article that describes the mechanism.
When a bug is fixed, delete its entry and update the article in the same commit.

## High priority

### Save / backup logic can lose or strand the document
`WED_Document::Save` (`src/WEDCore/WED_Document.cpp`) — confirmed real, 2026-09-25.
- If `fopen` of the new `earth.wed.xml` fails, `Save` returns without renaming
  `earth.wed.bak.xml` back — the document is left existing only as the backup.
- Uses CRT `rename()`: on Windows it fails when the destination exists (so the `.bak` may never
  refresh, and the recovery rename can't restore the original) and it isn't UTF-8 wrapped like
  `fopen` is in `Obj/XDefs.h`, so non-ASCII paths may fail.
- The older `.bak.bak` scheme using `FILE_rename_file` (`MoveFileW` on Windows) is `#if 0`'d out.
- Likely fix direction: write to a temp file, then atomically swap (`FILE_rename_file` / `MoveFileExW(MOVEFILE_REPLACE_EXISTING)`).

See [wed-object-model.md → Persistence](knowledge/wed-object-model.md#persistence-earthwedxml).

### Cancelling "Open backup" in Revert crashes WED
`WED_Document::Revert` (`src/WEDCore/WED_Document.cpp`) — confirmed from source, 2026-09-25.
When the main XML fails to load and a good `.bak.xml` exists, clicking "Cancel" on the
"Open backup" prompt runs a bare `throw;` with no active exception → `std::terminate`. The
surrounding `catch(...)` (which aborts the undo command) never runs. Fix: throw a real
exception (e.g. `WED_ThrowPrintf`) or return cleanly.
See [wed-object-model.md](knowledge/wed-object-model.md#persistence-earthwedxml).

### Failed airport-metadata CSV download crashes Gateway export / Update Metadata
`WED_GatewayExportDialog` (`WED_GatewayExport.cpp`) and `WED_UpdateMetadataDialog`
(`WED_MetadataUpdate.cpp`) — confirmed from source, 2026-09-25. `mAirportMetadataCURLHandle`
is initialized to `NULL` and never assigned, but on `cache_status_error` both call
`InterpretNetworkError(&mAirportMetadataCURLHandle->get_curl_handle())`. Any network failure
fetching the CSV → null dereference. Fix: take the error from the `WED_file_cache_response`.
See [wed-network-and-filecache.md](knowledge/wed-network-and-filecache.md).

### Airport metadata edits bypass undo
`WED_Airport::AddMetaDataKey` / `EditMetaDataKey` (`src/WEDEntities/WED_Airport.cpp`) —
confirmed 2026-09-25. They mutate `meta_data_vec_map` without calling `StateChanged()` first,
so edits made inside commands (Gateway-target pack export heuristics such as country codes;
Update Metadata) aren't captured: `UndoToMark` after Gateway export can't revert them and
Update Metadata can't be undone. Fix: call `StateChanged()` before mutating (as
`WED_Airport::SetNthProperty` does).
See [wed-entities.md](knowledge/wed-entities.md#writing-value-directly-bypasses-undo).

### Audit `doc/xml_compatibility` — may be stale since 2.5
`WED_DocumentWindow` `msg_DocWillSave` handler writes a hard-coded `205`. Policy (2026-09-25): bump
on every format change. Check what 2.6 / 2.7 added since the 2.5 tag — new `DEFINE_PERSISTENT`
classes, new `XML_Name(...)` attributes/elements, new enum descriptions in `WED_EnumSystem` /
`WED_Enums.h` — and bump to the right minimum version if any would be dropped or break an older
WED. Also consider moving the version check ahead of / into the load-failure path so an unknown
class reports "file is from a newer WED" instead of "Create obj failed."
See [wed-object-model.md](knowledge/wed-object-model.md#format-compatibility-rules-non-obvious).

### Older WED crashes saving a newer file's unknown enum values
`ENUM_LookupDesc` yields -1 for an unknown description on load; on save `ENUM_Desc(-1)` returns
NULL into `add_attr_c_str` (only a `DebugAssert` guards it), so release builds construct
`std::string` from NULL. Fix: preserve the original description string, or skip the attribute.
Confirmed from source 2026-09-25. Pairs with the `xml_compatibility` audit above.
See [wed-entities.md](knowledge/wed-entities.md#what-is-persisted-and-so-must-never-change).

### `README.dsf2text` (shipped to users) is wrong in four places
`src/DSFTools/README.dsf2text`, confirmed against the code 2026-09-25:
- `HEIGHTS` default is step 1.0, min 0.0 (range 0..65535 m), not "1.0 -32758.0".
- `DIVISIONS 8` gives 8×8 = 64 grid cells, not "sixteen point pools".
- `OBJECT_MSL/AGL` field order is `<type> <lon> <lat> <elevation> <heading>`.
- The pipe example `./DSFTool -DSFTool foo.dsf -` should say `-dsf2text`.
See [dsf-library-and-dsftool.md](knowledge/dsf-library-and-dsftool.md#where-readmedsf2text-disagrees-with-the-code).

### Jetway import maps every style code to the wrong facade
`WED_FacadePlacement::ImportJetway` (`src/WEDEntities/WED_FacadePlacement.cpp`) — confirmed
2026-09-25. For apt.dat row 1500 without a vpath, the fallback `switch (apt_data.style_code)`
maps 0→`Jetway_1_glass`, 1→`Jetway_1_solid`, 2→`Jetway_2_glass`, 3→`Jetway_1_solid`. X-Plane's
codes (= the facades' `#cabin` tags, which export writes) are 0 = light_solid (`Jetway_1_solid`),
1 = light_glass (`Jetway_1_glass`), 2 = dark_solid (`Jetway_2_solid`), 3 = dark_glass
(`Jetway_2_glass`). Round trips swap glass/solid; code 3 also loses the second tunnel.
Fix: correct the table (or pick the library jetway facade whose `#cabin` matches).
See [wed-import-export.md](knowledge/wed-import-export.md).

### DSF import doesn't decode fractional string spacing
`WED_DSFImport` passes the DSF string-spacing parameter straight to `SetSpacing`, but export
(12.1.2 / Gateway targets) encodes fractional spacing as `10000 + round(128*s)`, so re-imported
strings get spacing ≥ 10000 m. Also `WED_PreviewLayer` uses `< wet_xplane_1200` for integer
spacing while export uses `> wet_xplane_1200` — they disagree at exactly the 12.00 target.
See [wed-import-export.md](knowledge/wed-import-export.md#dsf-export-tiling-clipping-and-the-safe-bounds).

### Ortho export leaves stale DDS / .pol after reshaping
`WED_OrthoExport` regenerates the DDS only when the source image is not older than the output
(file dates) and writes the `.pol` only if none exists. Changing the polygon's UV crop, decal, or
other `.pol` settings with the same source image leaves stale output. Fix: key regeneration on
the crop/params too (or always rewrite).
See [wed-import-export.md](knowledge/wed-import-export.md).

### Splitter positions creep on every save / reopen
`GUI_Splitter::GetSplitPoint` is asymmetric: vertical splitters return child 1's bottom (split +
gap) while `AlignContentsAt` expects child 0's edge. `WED_DocumentWindow` saves `prop_split` /
`prev_split` with one and restores with the other, so they drift by the gap width each cycle.
See [gui-framework.md](knowledge/gui-framework.md#layout-traps).

### Main-thread blocking waits on network
`WED_DoInvisibleUpdateMetadata` busy-spins on `gFileCache.request_file`; `ReadCIFP` (validation) and
the moderator prefetch in `FillICAOFromJSON` `sleep_for` in loops on the main thread. UI freezes.
See [wed-network-and-filecache.md](knowledge/wed-network-and-filecache.md).

### DSFLib: 7z extraction failure returns `dsf_ErrOK` and leaks
`DSFReadFile`'s 7z path (since `be66053ad`) reports success on a failed extract and leaks the
extracted buffer.
See [dsf-library-and-dsftool.md](knowledge/dsf-library-and-dsftool.md#7z-compressed-dsfs-use_7z-set-in-objxdefsh).

### DSFTool: `PointPoolInfo_f` reads out of bounds
Reads `planeScales[p][3]` on 3-plane draped-object pools — hit on every `--dsf2text` of an overlay
with draped objects. Its local `is_overlay` flag also means the opposite of its name.
See [dsf-library-and-dsftool.md](knowledge/dsf-library-and-dsftool.md).

### DSFLib: polygon command reader tests the constant, not the flag
`dsf_Cmd_Polygon` reader: `if (dsf_CmdPolys)` should be `if (flags & dsf_CmdPolys)`.
See [dsf-library-and-dsftool.md](knowledge/dsf-library-and-dsftool.md).

### text2dsf parsing gaps
Never parses `END_SEGMENT_CURVED`; silently drops vertices whose coordinate count doesn't match the
declared depth.
See [dsf-library-and-dsftool.md](knowledge/dsf-library-and-dsftool.md#dsftool-and-the-dsf2text-format-srcdsftools).

### DDSTool `--pre_mips` with `--png2dxt*` reads past the buffer
Since `fc875f39f` this path no longer calls `MakeMipmapStackFromImage`; `WriteBitmapToDDS_MT`
with a null filter walks off the end.
See [bitmap-texture-and-ddstool.md](knowledge/bitmap-texture-and-ddstool.md#dds-writing-writebitmaptodds_mt).

### PNG with tRNS overflows when loaded indexed
Paletted/gray PNG + tRNS chunk loaded with `leaveIndexed = true` (used for all UI resources)
overflows its buffer.
See [bitmap-texture-and-ddstool.md](knowledge/bitmap-texture-and-ddstool.md).

### BMP loader ignores `dataOffset`
V5-header BMPs (both `test/img_reading/stbarts_*.bmp`) misload: 32-bit rejected, 24-bit shifted
by 28 px.
See [bitmap-texture-and-ddstool.md](knowledge/bitmap-texture-and-ddstool.md).

### CMYK JPEG overflows the row buffer
See [bitmap-texture-and-ddstool.md](knowledge/bitmap-texture-and-ddstool.md).

### DDS mip bounds check removed
`5acc062ec` removed the check; a truncated DDS reads past the buffer.
See [bitmap-texture-and-ddstool.md](knowledge/bitmap-texture-and-ddstool.md#the-direct-ddsktx2-gpu-path-has-its-own-rules).

### 16-bit float TIFFs fail to load
`TIFFReadRGBAImage` rejects float samples (fixture `test/img_reading/stbarts_rgba16float_lzw.tif`).
Needs a non-RGBA read path.
See [bitmap-texture-and-ddstool.md](knowledge/bitmap-texture-and-ddstool.md).

### `WED_OrthoExport` leaks the per-tile `DDSInfo` buffer
See [bitmap-texture-and-ddstool.md](knowledge/bitmap-texture-and-ddstool.md).

### `Polygon2cleaner` removes the wrong vertex at the end of the ring
Probably wrong when `n == size-1`.
See [utils-geometry.md](knowledge/utils-geometry.md).

### `Bezier2::is_near` can recurse forever
No depth limit; a distance ≤ 0 never terminates.
See [utils-geometry.md](knowledge/utils-geometry.md).

### Windows 7z DSF import fails on non-ASCII paths
The 7z path uses `CreateFileA` and returns an error without trying the UTF-8 `fopen` fallback.
See [utils-platform-and-files.md](knowledge/utils-platform-and-files.md#paths-and-unicode-the-classic-bug-source).

### Linux `MemFile_Open` mmap always fails
Passes `MAP_FILE` without `MAP_SHARED`/`MAP_PRIVATE` (Linux rejects it), so every file is read
whole instead of mapped.
See [utils-platform-and-files.md](knowledge/utils-platform-and-files.md#memfileutils-mapping-lifetime-termination).

### `WED_GISEdge::RebuildCache` reads out of bounds
Spatial pass loops `GetNumPoints()` (children + 2) but calls `GetNthChild(mm)` — OOB on road edges
with shape points.
See [wed-entities.md](knowledge/wed-entities.md#gis-classes-and-the-child-structure-each-one-assumes).

### Handle tools can leave a Drag/Copy operation open
`WED_HandleToolBase::KillOperation` / `PreCommandNotification` don't close the op opened for
`drag_PreMove`/`drag_Move`; any path that bypasses the GUI command defer (tool switch, etc.)
leaves it open and the next `StartCommand` asserts.
See [wed-map-and-tce.md](knowledge/wed-map-and-tce.md#handles-icontrolhandles-and-wed_handletoolbase).

### `WED_SlippyMap` leaks tile textures
Never deletes GL textures for tiles.
See [wed-map-and-tce.md](knowledge/wed-map-and-tce.md).

### Map tool prefs truncate doubles
`WED_MapPane::FromPrefs` reads `prop_Double` tool prefs with `atoi`.
See [wed-map-and-tce.md](knowledge/wed-map-and-tce.md).

### Runway-marking validation stops at the first runway-group polygon
`WED_ValidateATCRunwayChecks.cpp`: `if(lg <= group_RunwaysEnd) break;` should be `continue`.
See [wed-validation.md](knowledge/wed-validation.md).

### `WED_Document` import-prompt loop is off by one
Iterates `CountCustomPackages()` down to 1: skips index 0 and reads one past the custom list.
See [wed-core-services.md](knowledge/wed-core-services.md).

### Export target leaks between open documents
`gExportTarget` is loaded from a document only on open (`WED_DocumentWindow` constructor,
`msg_DocLoaded`), never on window activation, and `msg_DocWillSave` writes the global back into
the saving document's `doc/export_target`. With two documents open, exporting A uses the target
of whichever document loaded last, and saving A overwrites A's target. Per design principle 9
the per-document pref is authoritative. Filed high (wrong-version output); downgrade if you disagree.
See [wed-design-principles.md](knowledge/wed-design-principles.md#9-per-document-prefs-are-authoritative-globals-only-seed-new-documents).

## Needs verification

### Gateway upload may delete the user's exported DSFs
`WED_GatewayExportDialog::Submit` → `WED_ExportPackToPath` → `DSF_Export(…, mResolver, temp folder)`
constructs `DSF_export_info_t(resolver)`, whose destructor deletes previously exported DSFs (per
doc pref `export/last`) under the *document's own* package. Unconfirmed.
- Repro: in a pack with several exported DSF tiles, export the pack, then Gateway-upload a
  single airport. Check whether the other `Earth nav data/` DSFs survive and whether
  `export/last` changed.

See [wed-import-export.md → DSF export](knowledge/wed-import-export.md#dsf-export-tiling-clipping-and-the-safe-bounds).

### Is WEDNetwork's absence from `cmake/WED.cmake` an oversight?
Only `src/WEDNetwork/RAII_Classes.*` is listed in `cmake/WED.cmake`; the rest (`WED_Server`,
`WED_Connection`, `WED_NWLinkAdapter`, `WED_NWInfoLayer`) is also behind `WITHNWLINK 0` in
`Obj/XDefs.h`. Decision (2026-09-25): keep the code; check with the original author (added in
`9ec2a3a96`, 2012) whether dropping it from the build was intentional.
See [wed-network-and-filecache.md](knowledge/wed-network-and-filecache.md#nothing-in-srcwednetwork-except-raii_classes-is-compiled).

### Do shipped Mac/Linux builds find CA certificates?
Conan libcurl uses OpenSSL with `with_ca_bundle=auto` / `with_ca_path=auto`, so the CA path is
chosen on the build machine and compiled in. Unknown whether release builds work on user
machines unlike the CI runner. Test: Gateway import/upload from a release build on a different
Linux distro and a clean macOS install.
See [wed-network-and-filecache.md](knowledge/wed-network-and-filecache.md#tls--certificates).

## Low priority

### Gateway import crashes when the downloaded apt.dat has no airport
`WED_GatewayImportDialog::ImportSpecificVersion` (`WED_GatewayImport.cpp`) — confirmed from
source, 2026-09-25. `g` stays NULL if `out_apt` is empty; the code guards `has_dsf && g` but
then calls `g->CountChildren()` unguarded. Regressed in `6b835b390`. Only hit by a malformed
Gateway scenery pack.
See [wed-import-export.md](knowledge/wed-import-export.md).

### Gateway-target pack export selects possibly-deleted objects
`WED_DoExportPack` (`WED_SceneryPackExport.cpp`) — confirmed from source, 2026-09-25. It collects
`set<WED_Thing*> problem_children`, then `UndoToMark()`s the upgrade heuristics, then selects the
problem children. Undo `Delete()`s objects the heuristics created (e.g. vehicle-object
replacements), so if one of those failed export its pointer dangles. Unlikely in practice
(problem children are usually polygons/facades). Fix: collect IDs, `Fetch` after the undo,
skip NULLs.
See [wed-object-model.md](knowledge/wed-object-model.md#raw-pointers-do-not-survive-undo-redo-or-revert).

### DSF export doesn't abort when ortho / terrain-OBJ export fails
`DSF_ExportTileRecursive` (`WED_DSFExport.cpp`) — confirmed 2026-09-25. `if (int result = WED_ExportTerrObj(...) < 0) return result;`
(and the `WED_ExportOrtho` line) bind `result = (X < 0)`, so failure returns 1 and export
continues, although the user is told it's aborting. Intended: abort. Fix: parenthesize
`(result = X(...)) < 0`.
See [wed-import-export.md](knowledge/wed-import-export.md#dsf-export-tiling-clipping-and-the-safe-bounds).

### Wrong entries in the iso3166 country table
`add_iso3166_country_metadata` (`src/WEDImportExport/WED_MetaDataDefaults.cpp`) — confirmed 2026-09-25. `EK` → `"DAN "` (should
be DNK), `LK` → `"SWI "` (LK is Czechia); the fallback reads `apt.GetName()` instead of the
airport ID; with no match it still adds an empty `country` key and returns true.
See [wed-import-export.md](knowledge/wed-import-export.md).

### Block changing the X-Plane folder while documents are open
`wed_ChangeSystem` (handled in `WED_StartWindow`). Each open document keeps its file path on the
old install while library, texture and export paths move to the new one. Decision (2026-09-25):
unsupported — block it (or require closing documents first). Priority not set; filed low.
See [wed-core-services.md](knowledge/wed-core-services.md#changing-the-x-plane-folder-with-documents-open-leaves-mixed-state).

### DSF polygon pools: switch first-fit to best-fit
DSFLib assigns each polygon to the first existing pool whose bounds contain it, so a small polygon
exported after a big one inherits coarse quantization (~1.7 m in a 1° pool). Decision
(2026-09-25): worth switching to best-fit. Priority not set; filed low.
See [dsf-library-and-dsftool.md](knowledge/dsf-library-and-dsftool.md#polygon-pools-are-first-fit-so-one-big-polygon-can-wreck-precision-for-later-ones).

### DSF import collapses genuine zero-length sides
`BezierPointSeqFromTriple` merges runs of co-located consecutive points into one node on DSF
import. apt.dat import got a guard for this (WED-787, `022c6412e`); DSF import should mirror it.
See [utils-geometry.md](knowledge/utils-geometry.md#dsf-triple-bezier-notation-collapses-co-located-vertices-on-import).

### Upload stall timeout ignores upload progress
`curl_http_get_file` (`src/Network`): the 30 s stall check counts only downloaded bytes, so a slow
Gateway upload can be aborted as "Upload failed: Timeout". The POST/PUT constructor also leaves
`m_last_dl_amount` and `m_errcode` uninitialized. Not seen in the wild.
See [wed-network-and-filecache.md](knowledge/wed-network-and-filecache.md#timeouts-60-s-to-connect-30-s-of-stall-and-no-total-limit).

### Validation results dialog doesn't gate export (esp. macOS)
`WED_ValidateDialog` is `xwin_style_modal`, but XWin modality doesn't block: on macOS the modal
loop starts from `dispatch_async`, so `WED_ValidateApt` returns and export finishes before the
user sees the list. "Waive warnings & proceed" only closes the dialog. Decision (2026-09-25): bug —
export should wait for the user's choice (or the button wording must change). Priority not set; filed low.
See [wed-validation.md](knowledge/wed-validation.md#the-results-dialog-does-not-gate-anything).

### Pack export validation dialog lost its "Cancel Export" label
`WED_DoExportPack` stopped passing `"Cancel Export"` as `abortMsg` in the TYLER_MODE merge
`e25e63964`, so the dialog shows "Dismiss". Unintended; restore it.
See [wed-validation.md](knowledge/wed-validation.md).

### Gateway-path validation runs point-sequence and DSF checks twice
When validation is rooted at an airport, `ValidatePointSequencesRecursive` and
`ValidateDSFRecursive` run on it twice, duplicating messages in `validation_report.txt`. Fix: skip
the top-level pass when `wrl` is itself a `WED_Airport`.
See [wed-validation.md](knowledge/wed-validation.md).

### `has_dsf()` omits some DSF classes on Gateway upload
`k_dsf_classes` (`WED_GatewayExport.cpp`) leaves out `WED_ExclusionPoly`, `WED_AutogenPlacement`,
`WED_TerPlacement`, `WED_RoadEdge`. An airport whose only DSF content is one of those uploads no
`ICAO.txt`, yet still sets `ROADS_TAG`.
See [wed-import-export.md](knowledge/wed-import-export.md).

### `WED_ResourceMgr` never purges on library / X-Plane folder change
Nothing registers it as a listener, so `ReceiveMessage` → `Purge()` never runs; cached
obj/fac/for/pol/lin/str/agp data stays until the document closes. Decision (2026-09-25): bug. Careful:
a global `Purge()` would dangle `fac_info_t*` / resource pointers held by other caches and the
preview — the fix needs an ownership story. Priority not set; filed low.
See [wed-core-services.md](knowledge/wed-core-services.md#wed_resourcemgr-never-gets-its-invalidation-messages).

### Out-of-pack `EXPORT` line aborts the rest of `library.txt`
`WED_LibraryMgr::Rescan`: an `EXPORT*` line whose rpath climbs out of the pack does `break`
(added with WED-726), dropping every later line of that file. Should skip only that line.
See [wed-core-services.md](knowledge/wed-core-services.md#librarytxt-parsing-things-youd-expect-to-work-differently).

### `WED_PackageMgr::Rescan` change detection compares a field with itself
`if(o->hasAnyItems != o->hasAnyItems)` — a pack gaining/losing `library.txt` isn't a change.
See [wed-core-services.md](knowledge/wed-core-services.md).

### `GetDem` caches failures; other loaders never cache misses
`GetDem` inserts before parsing, so after one failure it returns true with a 0×0 DEM (`GetAGP`
similarly caches partial entries). Conversely `GetObj/Fac/Pol/Lin/Str/AGP` and
`WED_TexMgr::LookupTexture` don't record misses, so missing assets are re-read from disk every
frame (and `LookupTexture` leaks a GL texture name per failure).
See [wed-core-services.md](knowledge/wed-core-services.md#failed-loads-are-not-cached-so-they-retry-every-frame-mostly).

### `WED_TexMgr` cache key ignores load flags
Key is the raw path string; `flags` (wrap, mipmap, linear, compress) aren't part of it, so the
first loader decides sampling for all users of that texture.
See [wed-core-services.md](knowledge/wed-core-services.md#wed_texmgr-keys-on-the-path-string-alone-and-ignores-flags).

### `GetFac` facade-variant vector can invalidate handed-out pointers
`WED_ResourceMgr::GetFac` `push_back`s new variants into a `vector<fac_info_t>`; reallocation
dangles earlier `fac_info_t*` (including one cached by an `.agp`). Not seen as a crash.
See [wed-core-services.md](knowledge/wed-core-services.md).

### macOS window close box doesn't close / free windows without a `Closed()` override
`-[XWinCocoa close]` skips `super` and does `mOwner = NULL; delete mOwner;` (a no-op, since
`cb090b625`). Affects e.g. `WED_GatewayImportDialog`, `WED_ValidateDialog`.
See [gui-framework.md](knowledge/gui-framework.md#closing-a-window-differs-per-platform).

### `GUI_Pane::GetTimeNow` uses CPU time on Mac/Windows
`clock()` on Mac/Win vs. monotonic clock on Linux (WED-1405). Drives double-click detection;
the last-click state is also a static shared by every text table. Use a monotonic wall clock.
See [gui-framework.md](knowledge/gui-framework.md).

### Table cell-edit trap holds raw pointers
Tables in the window root's `mTrap` set are removed only by their own `TrapNotify`; deleting a
table mid-edit leaves a dangling pointer for the next click.
See [gui-framework.md](knowledge/gui-framework.md#table-editing-lifetime-gui_table--gui_texttable).

### macOS `GUI_Timer` start semantics differ
`Start(s)` passes `s` as CF's absolute fire date (first fire is immediate); `Start(0)` is one-shot
on Mac but repeating on Windows/Linux.
See [gui-framework.md](knowledge/gui-framework.md).

### No Windows DPI awareness
No manifest or API call; Windows bitmap-stretches WED on high-DPI monitors. Add per-monitor DPI
awareness.
See [gui-framework.md](knowledge/gui-framework.md#hidpi-and-scaling).

### File cache serves truncated downloads as valid
`RAII_FileHandle::path()` returns `""`, so failed-write cleanup deletes nothing, and a disk-write
failure still reports `cache_status_available`.
See [wed-network-and-filecache.md](knowledge/wed-network-and-filecache.md).

### Slippy map stalls on one failed tile
On error `WED_SlippyMap` records the miss under `res.out_path` = `""`, so it retries the bad tile
through its 60 s cool-down while every other tile waits.
See [wed-network-and-filecache.md](knowledge/wed-network-and-filecache.md).

### Gateway export / metadata update hang on `cache_status_cooling`
Both dialogs stop their timer on cooling and never advance (import advances silently instead).
See [wed-network-and-filecache.md](knowledge/wed-network-and-filecache.md#what-consumers-do-with-each-status).

### Ortho import "Kpix" sizing uses the clamped texture size
Uses the size after `GL_MAX_TEXTURE_SIZE` clamping rather than the file's real size.
See [bitmap-texture-and-ddstool.md](knowledge/bitmap-texture-and-ddstool.md).

### Create-tool heading readout wrong at high latitude
`WED_CreateToolBase::RecalcHeadings` passes a lon/lat vector to `VectorMeters2NorthHeading`.
See [utils-geometry.md](knowledge/utils-geometry.md#point2-is-lon-lat-in-degrees-and-every-predicate-works-in-raw-degree-space).

### `SimplifyPolygonMaxMove` allow-in/out restriction broken
`calc_error` dots against the wrong vector (WED only calls it with both allowed).
See [utils-geometry.md](knowledge/utils-geometry.md).

### Windows `mkdtemp` shim is a no-op
Never rewrites the template, so Gateway export always uses a literal `tempXXXXXX` folder that is
never cleaned up.
See [utils-platform-and-files.md](knowledge/utils-platform-and-files.md).

### `WED_Log.txt` lives next to the app
If the install folder isn't writable, logging silently stops.
See [utils-platform-and-files.md](knowledge/utils-platform-and-files.md).

### `WED_ValidateApt` `fclose`s a failed `fopen`
See [wed-validation.md](knowledge/wed-validation.md).

### Library "new until" date is off by a month
Uses 0-based `tm_mon` as if 1-based.
See [wed-core-services.md](knowledge/wed-core-services.md).

### `AbortCommand` doesn't broadcast `msg_ArchiveChanged`
Views that refresh only on that message may show stale state after an aborted command.
See [wed-object-model.md](knowledge/wed-object-model.md).

### 3D preview window isn't in the command chain
`WED_MapPreviewWindow` has a `nullptr` commander parent and no `CanHandleCommand`; while it is the
key window, menus show items enabled that do nothing. Decision (2026-09-25): route commands to
its document window. Priority not set; filed low.
See [wed-ui-panes.md](knowledge/wed-ui-panes.md).

### Control-texture tile index formula is wrong in WED previews
`WED_ResourceMgr` (.pol `TEXTURE_TILE` page table): `red *= (tiles_x + 128) / 256` divides first,
giving 0 for `tiles_x < 128`, so every pixel selects tile (0,0). X-Plane (`terrain_frag.glsl`)
uses `tile = round(R/255 * tiles_x)` (same for G/`tiles_y`; `RUNWAY_TILE` halves G). Row order
(bottom = row 0) already matches. Also ignores image size ≠ page count. Checked against X-Plane
source 2026-09-25. Priority not set; filed low.
See [bitmap-texture-and-ddstool.md](knowledge/bitmap-texture-and-ddstool.md).

### Unify all earth math on WGS84
Three models coexist: GISUtils sphere (`EARTH_MEAN_RADIUS` in `Quad_*`, `LonLatDistMeters`,
`MetersToLLE`), `WED_MapZoomerNew` GRS80, `CreateTranslatorForBounds` ellipsoid (~0.5% N-S
difference near the equator). Decision (2026-09-25): move everything to WGS84, as one coordinated
change (use `test/wgs84/` to check against X-Plane). Priority not set; filed low.
See [utils-geometry.md](knowledge/utils-geometry.md#three-different-earth-models-are-in-use-at-once).

### TIFF alpha comes back premultiplied
`TIFFReadRGBAImage` premultiplies unassociated alpha; DDSTool and WED ortho export then produce
dark fringes and darker mips. Un-premultiply or use a non-RGBA read path.
See [bitmap-texture-and-ddstool.md](knowledge/bitmap-texture-and-ddstool.md#tiff-loads-come-back-premultiplied-always-4-channel-and-floats-are-rejected).

### Remove DDSTool `--quilt` and `QuiltUtils` (cleanup)
Broken on 64-bit macOS/Linux (`unsigned long` pixels) and its trials loop never improves; decision
2026-09-25: remove rather than fix.
See [bitmap-texture-and-ddstool.md](knowledge/bitmap-texture-and-ddstool.md).

### `unpaved_runways` layer group maps to the taxiway range
`layer_group_for_string` / `kGroupNames` (`WED_PreviewLayer`) maps the `.pol`/`.lin` `LAYER_GROUP`
name `unpaved_runways` onto the unpaved taxiway range instead of `group_UnpavedRunwaysBegin/End`.
Also used by validation (`WED_ValidateATCRunwayChecks.cpp`).
See [wed-map-and-tce.md](knowledge/wed-map-and-tce.md#layers-caps-and-draw-order).

### Library filter bar and list start on different packs
`WED_LibraryFilterBar` starts on `pack_Default` ("Laminar Library"); `WED_LibraryListAdapter`
starts on `pack_Library`.
See [wed-ui-panes.md](knowledge/wed-ui-panes.md).

### Dead `AsyncDestroy()` in `WED_Document::TryClose` (cleanup)
Followed immediately by `delete this`; the destructor cancels the async entry. Leftover from
`e166f33d7` — drop it.
See [wed-ui-panes.md](knowledge/wed-ui-panes.md).

### Automate the DSF test fixtures
`test/dsftool_elevations/` (expected `--dsf2text` output predates `fe6b094c6`'s `%.5lf`/`%.3lf`
precision change; `dsf_out.txt` is an empty placeholder) and `test/dsf_export/uv_mapping.xml`
have no procedure. Decision (2026-09-25): turn them into a scripted regression check (refresh the
baseline first).
See [dsf-library-and-dsftool.md](knowledge/dsf-library-and-dsftool.md).

### Delete dead files (cleanup)
Decision (2026-09-25): remove `src/Utils/CarbonMemMap.h` and `src/Utils/MemIStreamBuf.h` (included
nowhere), `src/WEDWindows/WED_GroupCommands.cpp.better` (no build reference), and
`WED_TerraserverLayer` (never instantiated, not in `cmake/WED.cmake`). Keep `ObjUtils.cpp` in the
WED target.

### Wire up the Texture tab's (WEDTCE) keys, commands and view persistence
`WED_TCEPane::TCE_KeyPress`, `TCE_HandleCommand`, `TCE_CanHandleCommand`, `FromPrefs`, `ToPrefs`
have no callers, so the 'v'/'e' hot-keys, TCE menu commands and zoom/pan persistence do nothing.
Forward them from `WED_DocumentWindow` like `Map_KeyPress` / `Map_HandleCommand`, and fix
`FromPrefs` reading `"tce/top"` where `ToPrefs` writes `"tce/left"`.
See [wed-map-and-tce.md](knowledge/wed-map-and-tce.md#wedtce-where-it-really-differs-from-wedmap).

### GUI → WED layering violations
`GUI/GUI_TextTable.cpp` includes `WED_UIDefs.h`, `WED_ToolUtils.h`, `WED_Sign_Editor.h`,
`WED_Line_Selector.h`, `WED_Road_Selector.h`; `GUI_FilterBar.cpp` and `GUI_FormWindow.cpp`
include `WED_Colors.h`. Move the WED-specific editors/theming to the WED side (principle 8).
See [wed-design-principles.md](knowledge/wed-design-principles.md#8-wed-is-strictly-layered-on-top-of-gui).

### Casts to intermediate `WED_GIS*` classes
Principle 3 forbids casting to concrete intermediate GIS implementations. Existing casts are
concentrated in `WED_GroupCommands.cpp` and `WED_ConvertCommands.cpp` (plus single instances in
`WED_DSFImport.cpp`, `WED_GISEdge.cpp`, `WED_ValidateATCRunwayChecks.h`, `WED_Orthophoto.cpp`).
Convert to interface or final-class casts opportunistically.
See [wed-design-principles.md](knowledge/wed-design-principles.md#3-concrete-classes-vs-gis-interfaces).
