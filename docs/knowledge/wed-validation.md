# WED Validation

> Source: `src/WEDCore/WED_Validate.{h,cpp}`, `src/WEDCore/WED_ValidateATCRunwayChecks.{h,cpp}`,
> `src/WEDCore/WED_ValidateList.{h,cpp}`. Callers: `WEDWindows/WED_DocumentWindow.cpp`,
> `WEDImportExport/WED_SceneryPackExport.cpp`, `WEDImportExport/WED_AptIE.cpp`,
> `WEDImportExport/WED_GatewayExport.cpp`. Export target enum: `WEDCore/WED_Globals.h`.

Validation is one entry point, `WED_ValidateApt()`. It walks the document, builds a flat
`validation_error_vector`, writes a text report, optionally opens the results dialog, and
returns a three-state `validation_result_t`. It never edits the document. Every rule reads
the process-global `gExportTarget`, so the same document can be valid for one target and
invalid for another.

## Things That Will Bite You

### Severity comes from where the code sits in the enum, not from a flag
`validate_error_t` (in `WED_Validate.h`) contains a sentinel, `warnings_start_here`. Any code
declared **after** the sentinel counts as a warning, and any code before it counts as an
error. The code tests this as `v.err_code > warnings_start_here` in two places: the
result/report loop in `WED_ValidateApt` and the dialog constructor in `WED_ValidateList.cpp`.
Put a new `err_*` below the sentinel and you have quietly made it a non-blocking warning.
The enum is not persisted anywhere, so reordering it is safe. The header comment asks you to
keep it alphabetical within each half (errors, then warnings).

### Severity is often chosen per target at the push site
Many rules have both an `err_X` and a `warn_X` code, and pick one at the point where they
push the message: `gExportTarget == wet_gateway ? err_X : warn_X`. Examples are airport
name style, impossible size, no runways, ATC taxi route length (`grievance` in
`WED_ValidateATCRunwayChecks.cpp`), runway misaligned with its name, exclusion polygons that
exclude forests, and set_AGL range. The same problem blocks a Gateway upload but only warns
for a custom-scenery export. The history shows Gateway-only severity being changed back and
forth (forest exclusions: `02a6479d7` made it a warning, then `29da8b72a` made it an error
again for the GW target; see also `812e574d8` and `b5f7d39c5`). Expect the choice to be
debated and keep it in one ternary so it is easy to flip. One oddity:
`ValidateOneFacadePlacement` has `gExportTarget == wet_gateway ? warn_facade_height :
warn_facade_height` ("only warn for now"). It is a deliberate placeholder for a future error.

### `wet_gateway` is 99, so it compares greater than every X-Plane version
`WED_Export_Target` in `WED_Globals.h` has `wet_xplane_900 … wet_xplane_1212`, then
`wet_gateway = 99`, and `wet_latest_xplane` as an alias for the newest real version. The
consequences:
- `gExportTarget >= wet_xplane_1200` (or `>= wet_xplane_1050`) is **true for the Gateway**.
  This is intended, because the Gateway means "latest format plus strict rules".
- `gExportTarget < wet_xplane_1200` is **false for the Gateway**.
- `>= wet_gateway` is just `== wet_gateway`. Both spellings appear in the file.
- A rule meant for "XP 11.30+ but not the Gateway" has to say so explicitly, as the
  gui_label check in `ValidateAirportMetadata` does:
  `>= wet_xplane_1130 && != wet_gateway`.
- `WED_DSFExport.cpp` uses `gExportTarget > wet_xplane_1200` for string spacing. That is true
  for `wet_xplane_1212` and the Gateway, and it will also cover any newer `wet_xplane_*`. A
  validation rule about fractional string spacing has to use the same test.

How the target is stored, loaded, clamped and silently switched, and how to add a new X-Plane
version without shifting saved documents, is in
[wed-import-export.md](wed-import-export.md#the-export-target-is-a-global-it-is-sticky-and-some-code-switches-it-for-you).

### Entity accessors also depend on the target
Validation is not the only code that reads `gExportTarget`.
`WED_FacadePlacement::HasDockingCabin()` returns false when the target is below XP12, and
`WED_Runway` changes how it computes shoulder geometry by target. The jetway checks in
`ValidateOneAirport` and the "docking jetway facade must be inside an airport" check only
run if `HasDockingCabin()` is true, so for older targets they silently disappear. Before you
reason about which rules fire, check what the entity methods you call do with the target.

### "Truthy" means "no errors", not "clean"
`validation_result_t` is `validation_errors = 0, validation_warnings_only,
validation_clean`. The callers depend on that ordering:
- `WED_DoExportPack` and `WED_DoExportApt` use `if(!WED_ValidateApt(...)) return;`. Only
  errors block, and export goes ahead when there are only warnings. The WED-1007 fix
  (`1fd78cadc`) introduced the enum so that warnings would stop blocking DSF export.
- The Validate menu command tests `== validation_clean` so that it can show "Your layout is
  valid".
- Gateway upload handles each state separately (see below).

If you reorder the enum or add a fourth state, you break every caller that uses `!`.

### The results dialog does not gate anything
`WED_ValidateDialog` is created with `new` and flagged `xwin_style_modal`. On macOS,
`XWin.mac.mm` starts the modal loop later, from a `dispatch_async` block, so
`WED_ValidateApt` returns **before the user sees the list**. When there are only warnings,
the export has already finished while the dialog is still on screen. The button labelled
"Waive warnings & proceed" (warnings only), or the caller's `abortMsg` (errors), only closes
the dialog. Nothing in the export path waits for it. Considered a bug (Ben, 2026-09-25) — on [the punch list](../bug-punch-list.md).

`abortMsg` defaults to `"Dismiss"`. `WED_DoExportApt` passes `"Cancel export"`, which adds
an exclamation icon because the label contains "ancel". `WED_DoExportPack` used to pass
`"Cancel Export"`, but the argument was dropped in the TYLER_MODE merge `e25e63964`, so pack
export now shows "Dismiss". Unintended (Ben, 2026-09-25) — on [the punch list](../bug-punch-list.md), low priority.

### Gateway upload suppresses the dialog; the user only gets an alert
`WED_GatewayExportDialog::Submit` calls `WED_ValidateApt(mResolver, NULL, apt, true)`. The
`true` is `skipErrorDialog`, because (per the code comment) "OSX can't handle nested modal
windows". The upload form is itself a modal `GUI_FormWindow`. When there are errors, the
user sees only "Can not export to gateway due to validation errors." When there are only
warnings, they get a Continue/Cancel `ConfirmMessage`. When validation fails, the code
**leaves `gExportTarget` at `wet_gateway`** on purpose ("help users oblivious about their
exportTarget settings"). The user's next Validate command then shows the Gateway rules in
the list. Two consequences:
- `pane` is NULL on this path. `WED_ValidateDialog` calls `mMapPane->ZoomShowSel()`
  without a null check. Never pass a NULL pane unless you also pass `skipErrorDialog=true`.
- The full list is still written to `validation_report.txt` in the local package
  (`gPackageMgr->ComputePath(lib_mgr->GetLocalPackage(), …)`).

### When the root is an airport, messages are duplicated
`WED_ValidateApt` runs `ValidateOneAirport` for each airport, and each of those runs
`ValidatePointSequencesRecursive` and `ValidateDSFRecursive` on its airport. After that it
runs both recursions again from `wrl`, to reach off-airport content. The recursions skip
*child* airports but not the *root*. When `wrl` is itself an airport (the Gateway path is
the only such caller today), every point-sequence and DSF-level message is produced twice.
Users do not notice because the Gateway path hides the dialog, but `validation_report.txt`
contains the duplicates. On [the punch list](../bug-punch-list.md), low priority (Ben, 2026-09-25). A fix is to skip the
top-level pass when `dynamic_cast<WED_Airport*>(wrl)` is non-null.

### The "elements outside hierarchy" check never runs during Gateway upload
`ValidateDSFRecursive` emits `err_airport_elements_outside_hierarchy` when
`gExportTarget == wet_gateway && !parent_apt`, for anything that is not a `WED_Group` or
`WED_OverlayImage`. During upload, `parent_apt` is always the airport, so this rule only
fires when the user runs Validate or Export Scenery Pack on the whole world with the Gateway
target selected. That is fine, because only the selected airport is uploaded. Do not treat
this rule as upload protection.

### Hidden objects are not validated, and the collector matches exact classes only
Everything that is hidden is skipped: `CollectRecursive`'s `ThingNotHidden`, the
`GetHidden()` early-outs in both recursions, and the `COLLECT` macro. The reasoning is that
hidden objects are not exported. That includes a hidden airport.

`ValidateOneAirport` collects with **one** lambda, `CollectEntitiesRecursive`, that compares
`GetClass() == X::sClass` and uses `static_cast`. A comment says "50% of CPU time in
validation is for casting". Consequences of that design:
- A subclass of a collected type is **not** collected. The comparison is exact.
- When an object matches, its children are not visited (`return` inside `COLLECT`).
- The final `else` branch only recurses into `WED_Entity` objects, so non-entity
  containers are opaque unless they are special-cased (as `WED_ATCFlow` and
  `WED_ATCFrequency` are).
- Off-airport collection (`CollectEntitiesRecursiveNoApts`) recurses **only** through
  `WED_Group`.

WED-1493 (`7493e1789`) is an example of this kind of bug: the airport list was collected
with `CollectRecursiveNoNesting`, which only looks one level deep, so airports inside groups
were not validated at all. It is now `CollectRecursive`.

### Duplicate runway names turn off the ATC checks
In `ValidateOneAirport`, `WED_DoATCRunwayChecks` and `ValidateATCFlows` only run if
`CheckDuplicateNames(runway_or_sealane, …)` returns false. The comment explains that these
checks give "utterly misleading results if runway names are ambiguous". `ValidateATCFlows`
also returns early if flow names are duplicated. When a user reports "validation missed my
taxi route problem", first check whether a duplicate name error is hiding it.

Inside `WED_DoATCRunwayChecks`, each runway's checks form a nested short-circuit chain:
`AllTaxiRouteNodesInRunway` → `TaxiRouteParallelCheck` → `TaxiRouteCenterlineCheck` →
`DoTaxiRouteConnectivityChecks` → `RunwayHasCorrectCoverage`. A failure stops the later
checks for that runway. The hot-zone checks run afterwards regardless. Add a new
per-runway geometry check at the right depth of that chain (the placeholder comment is
"Add additional checks as needed here").

### Gateway validation depends on the network and blocks the main thread
With the Gateway target, `ReadCIFP()` fetches `WED_URL_CIFP_RUNWAYS` through `gFileCache`
(domain `cache_domain_metadata_csv`, 1-day expiry). While the download is in progress it
calls `this_thread::sleep_for(1s)` up to 5 times **on the main thread**. If the file is
still unavailable it shows an alert and **skips the CIFP checks**, and validation, and
therefore upload, can still succeed. The `ToDo` comment says the upload should really wait
for full verification. **Policy (Ben, 2026-09-25): skipping is fine** — don't "fix" this by
blocking uploads when the CIFP file is unavailable. See [wed-network-and-filecache.md](wed-network-and-filecache.md).

The resource checks also depend on `WED_LibraryMgr` state. `IsResourceDefault` and
`IsResourceDeprecatedOrPrivate` return correct answers only after the X-Plane default
library has been scanned. An unknown path counts as not default and as private. `::FLATTEN::.pol` is
special-cased as default, public, and existing (`CheckFlattenPolygon`, added in
`40a97b916`). It was blocked on the GW earlier (`65900d4e8`).

### Validation does not see the state the exporter writes
- **Gateway upload:** `fill_in_airport_metadata_defaults` (which includes
  `Enforce_MetaDataGuiLabel`) runs as a committed `"Update metadata"` operation **before**
  validation. It stays in the undo history even if validation then fails. This is why
  `ValidateAirportMetadata` skips the gui_label check for the Gateway target.
- **Export Scenery Pack with the Gateway target:** `DoHueristicAnalysisAndAutoUpgrade`
  runs **after** validation, inside `MarkUndo`/`UndoToMark`. It changes content that
  validation never saw (ICAO cleanup, country prefixes, exclusions, grass). If
  `UndoToMark` fails, the user is told the scenery was permanently altered. **This is by
  design (Ben, 2026-09-25):** the heuristics are trusted to produce valid output; don't add a
  re-validation pass.
- `GATEWAY_IMPORT_MODE` builds (`Obj/XDefs.h`, normally 0) skip validation entirely in
  both `WED_DoExportPack` and `WED_DoExportApt`.
- Importing from the Gateway (`WED_GatewayImport.cpp`, `WED_SceneryImport.cpp`) silently
  sets `gExportTarget = wet_gateway`.
- Export has its own failure channel. `problem_children` from `WED_ExportPackToPath` or
  `DSF_ExportAirportOverlay` (self-intersecting polygons, closed facades crossing tiles) is
  reported by a separate alert that selects the objects. Validation that passes does not
  guarantee DSF export succeeds.

### Small crashers to know about
- `validation_error_t` needs at least one entry in `bad_objects`. The dialog passes them to
  `ISelection::Insert` without checking. Where a helper can return NULL,
  `dynamic_cast<WED_Thing*>(ps)` needs a fallback, as the taxiway-hole case has with
  `h ? h : twy`.
- The dialog stores raw `WED_Thing*` in `msgs_orig` and listens only for
  `msg_DocumentDestroyed`. That is safe only while the dialog stays modal.

## Architecture

### Flow of one validation run
1. `WED_ValidateApt(resolver, pane, root=NULL, skipErrorDialog=false, abortMsg="Dismiss")`.
   It clears the debug overlay lines, and if the target is Gateway it reads the CIFP file.
2. For each non-hidden `WED_Airport` under the root (at any depth), it calls
   `ValidateOneAirport`:
   - It collects entities once into typed vectors and precomputes `legal_rwy_oneway` and
     `legal_rwy_twoway` (`WED_GetAllRunwaysOneway/Twoway`).
   - Name/ICAO → duplicate names → (if names are unique) ATC runway checks and flows →
     frequencies → signs, viewpoints, taxiways, trucks, runways/sealanes, helipads, ramps →
     jetway door pairing → metadata (for targets ≥ 1050) → APAPI (for targets < 1200) → size →
     truck/route consistency → Gateway-only block (boundary presence, containment, far-out
     scenery, orthophotos, CIFP, roads bbox) → `ValidatePointSequencesRecursive` and
     `ValidateDSFRecursive` for the airport.
3. Off-airport: roads found through groups, then both recursions from the root with
   `parent_apt = dynamic_cast<WED_Airport*>(root)`. Both recursions stop at child airports.
4. It writes `validation_report.txt` to the local package, with one line per message
   prefixed by ICAO; warnings end with "(warning only)".
5. If there are messages: unless `skipErrorDialog` is set, it creates a `WED_ValidateDialog`,
   then returns `warnings_only ? validation_warnings_only : validation_errors`. Otherwise it
   returns `validation_clean`.

`ValidateDSFRecursive` holds the checks that are not specific to airports: facade walls,
holes, and curves; forest fill modes; string spacing; exclusion polygons; set_MSL/AGL;
resource existence, default-library status, and file extension (`EXTENSION_DOES_MATCH`);
the DEV/ folder warning; polygon winding and self-intersection (`ValidateOnePolygon`). It
does not recurse below a `WED_GISPolygon`.

`ValidateOnePointSequence` checks the minimum number of points, orthophoto UV bounds and
UV collisions, and zero-length or too-close segments. The Gateway uses a 30 m minimum for
airport boundaries and 0.1 m for everything else. The deeper checks only run when the
parent is one of the area classes listed there. Plain lines and strings `return` early
(see the comment "Comment this out to enable checks").

### Error record → selection
`validation_error_t` holds `msg`, `err_code`, `bad_objects` (a vector of `WED_Thing*`) and
`airport`, which is the owning airport or NULL for "Off Airport". It has two templated
constructors: one takes a single `T*`, the other takes any container of pointers. A
container of subclasses such as `vector<WED_TaxiRoute*>` or `set<WED_GISPoint*>` is copied
in. Choose the objects the user must *act on*, not just the owner. For example, zero-length
segments list the first vertex of each bad side so that deleting the selection fixes them.
The taxiway-hole check deliberately selects the hole ring, per the comment "Ben says".

`WED_ValidateDialog` reuses `WED_AptTable` and `AptVector` to show a searchable
two-column list, putting the ICAO in `icao` and "Error: "/"Warning: " plus the message in
`name`. `WED_AptTable::GetSelection` returns indices into the **unfiltered** vector, so
`msgs_orig[i]` stays correct while a search filter is active. A selection change
(`GUI_TABLE_CONTENT_CHANGED`) falls through into the `kMsg_ZoomTo` case. That code wraps a
`ISelection::Clear/Insert` in `StartOperation("Select Invalid")`, so every click in the list
is an undoable selection change, and then stops before zooming. Only the Zoom buttons call
`WED_MapPane::ZoomShowSel`.

### Debug overlays
`Obj/XDefs.h` sets `DEBUG_VIS_LINES 1` for every build, and `WED_ValidateATCRunwayChecks.cpp`
also sets it under `DEV`. After validation, magenta runway, hot-zone, and boundary-box
outlines for failing checks are left in `gMeshLines`/`gMeshPolygons` (`WED_Globals`). The
next validation run clears them.

## Recipe: adding a new check

1. **Pick or add the code** in `validate_error_t`. Put it above `warnings_start_here` for an
   error and below for a warning, in alphabetical position. If the severity depends on the
   target, add both `err_` and `warn_` codes and choose between them with a ternary where you
   push the message.
   Add every new code to `WEDMCP/WED_MCPValidateNames.cpp` as well. A `static_assert` fails the
   build if the count doesn't match; see [wed-mcp.md](wed-mcp.md).
2. **Put the check where the objects already are.**
   - Airport-scoped entities: use the typed vectors in `ValidateOneAirport`. If your class
     is not collected yet, add a `COLLECT(WED_Foo, foos)` line before the final `else`.
     The match is exact, so list subclasses explicitly.
   - DSF/overlay objects that can also live outside airports: add a branch in
     `ValidateDSFRecursive` keyed on `who->GetClass() == X::sClass`. Pass `parent_apt`
     through.
   - Point-sequence geometry: `ValidateOnePointSequence`. Taxi route and runway-coupled ATC
     geometry: `WED_DoATCRunwayChecks`, at the right depth of its short-circuit chain.
3. **Choose the target gate deliberately.**
   - Format-capability rules ("only supported in X-Plane N+") should test
     `gExportTarget < wet_xplane_N`. Match the version the writer uses (`get_apt_export_version`
     in `WED_AptIE.cpp`, the `gExportTarget` tests in `WED_DSFExport.cpp`). If they
     disagree, validation passes content that the exporter then silently drops or
     downgrades.
   - Gateway policy rules test `gExportTarget == wet_gateway`. Put airport-level ones inside
     the existing Gateway block in `ValidateOneAirport`.
   - Remember that `wet_gateway` also passes every `>= wet_xplane_N` test.
4. **Skip hidden objects.** The collectors already do this. Custom walks must do it too.
5. **Set `bad_objects` to what the user should select**, and pass the owning airport
   (or NULL).
6. **Keep checks independent.** Commit `23a0718b5` made validation keep going after errors,
   and `b9fff7ca9` fixed checks that were not running. An early `return` in a per-object
   validator is acceptable only when later checks on the same object would be meaningless
   (as in the airline string parsing in `ValidateOneRampPosition`).
7. Test by hand with **each relevant target**. There are no automated validation tests
   under `test/`. Also add a line to the changelog in `src/WEDCore/README.WorldEditor`, as
   fix commits do. A new source file must be listed in `cmake/WED.cmake`.

## Recurring regression themes (from git log)
- **Traversal misses objects:** WED-1493 (nested airports), WED-1340 (`222d39097`, only the
  first of several airport boundaries was recognised), `3e940acd8` (the far-outside check did
  not skip hidden objects).
- **Crashes on unexpected resource types:** WED-1299 (`e908ee5ab`, an orthophoto with an
  image resource made `GetPol` fail), `88596f495` (helipads), `e74230217` (taxi route runway
  segments).
- **Tuning thresholds and false positives:** the magnetic model for runway-name alignment
  (`f8ae772d6`), ATC segment minimum lengths (`4b8edd0ae`, `812e574d8`), the out-of-boundary
  margin going from 0.5 to 0.6 nm (`6cb1d7bd9`), facade height (`c90c8d94e`, then `e9ada8feb`).
- **Gateway policy flip-flops:** FLATTEN polygons, roads on the GW (`117aadb0f` banned them,
  `d9a604731` allowed them near airports), forest exclusions.

## Connections to Other Systems

- [wed-mcp.md](wed-mcp.md): the MCP `validate` / `export_*` tools call `WED_ValidateApt` with `skipErrorDialog` and the `out_msgs` parameter (messages keyed by `validate_error_t` name via `WEDMCP/WED_MCPValidateNames.cpp`, which must track the enum - a `static_assert` checks the count).

- **Design rules** ([wed-design-principles.md](wed-design-principles.md)): the stated rules for new code and reviews that this subsystem's conventions should follow — command ownership, pointer vs ID, class vs interface casts, error handling, layering, per-doc prefs, platform reference.
- [wed-import-export.md](wed-import-export.md): every export entry point calls validation
  first. The apt.dat and DSF writers apply their own target-dependent downgrades, and those
  must agree with the capability rules here.
- [wed-network-and-filecache.md](wed-network-and-filecache.md): the Gateway upload dialog
  and the CIFP runway file are fetched through `gFileCache`.
- [wed-core-services.md](wed-core-services.md): `WED_LibraryMgr` (default/private resource
  status), `WED_ResourceMgr` (fac/pol/agp info used by the checks), `gPackageMgr` (report
  path), `gExportTarget` in `WED_Globals`.
- [wed-entities.md](wed-entities.md): the checks call entity `Export()` into `AptDefs`
  structs, and some entity methods depend on the target.
- [wed-object-model.md](wed-object-model.md): `WED_Thing`/`sClass` identity, hierarchy
  walking, and the undo operation used when selecting.
- [wed-ui-panes.md](wed-ui-panes.md) and [gui-framework.md](gui-framework.md): how
  `WED_ValidateDialog` is built from `GUI_Window`, `GUI_TextTable`, and the borrowed
  `WED_AptTable`. The macOS async modal loop is in `UI/XWin.mac.mm`.
- [wed-map-and-tce.md](wed-map-and-tce.md): `WED_MapPane::ZoomShowSel`, and the map drawing
  of the debug overlay lines.
- [utils-geometry.md](utils-geometry.md): the geometry behind the checks. Winding
  (`err_gis_poly_wound_clockwise`) uses vertex-only CCW tests that ignore bezier handles, bezier
  self-intersection uses the approximate, depth-bounded `Bezier2::intersect`, and
  `Segment2::intersect` reports shared endpoints as crossings, so callers must filter them.
