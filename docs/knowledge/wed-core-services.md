# WED Core Services (app lifecycle, packages, library, resources, textures, prefs, enums)

> Source: `src/WEDCore/` (the non-object-model parts: `WED_AppMain`, `WED_Application`,
> `WED_Document` as a service host, `WED_PackageMgr`, `WED_LibraryMgr`, `WED_ResourceMgr`,
> `WED_TexMgr`, `WED_EnumSystem` / `WED_Enums.h`, `WED_Globals`, `WED_Messages.h`,
> `WED_Assert`, `WED_Errors`), plus `Interfaces/ILibrarian.h`, `ITexMgr.h`, `IDocPrefs.h`.
> Object model (`WED_Thing`, archive, undo, XML) is in [wed-object-model.md](wed-object-model.md);
> validation in [wed-validation.md](wed-validation.md).

## Things That Will Bite You

### Each document has its OWN TexMgr, LibraryMgr and ResourceMgr — only the PackageMgr is global
`WED_Document`'s constructor `new`s a `WED_TexMgr`, `WED_LibraryMgr` and `WED_ResourceMgr` for
that document. The one real singleton is `gPackageMgr` (a stack object in `main()` in
`WED_AppMain.cpp`, which sets the global in its constructor). What this means in practice:
- With N documents open, a package rescan runs N full library scans (each `WED_LibraryMgr`
  listens to `gPackageMgr` separately), and every asset/texture is loaded once per document.
- Get the managers through the document's `IResolver` magic names: `Resolver_Find("librarian" |
  "texmgr" | "resmgr" | "libmgr" | "docprefs")`, wrapped by `WED_GetLibrarian` / `WED_GetTexMgr` /
  `WED_GetResourceMgr` / `WED_GetLibraryMgr` in `WEDMap/WED_ToolUtils`. Don't hold them in statics.
- `WED_ResourceMgr` takes the `WED_LibraryMgr` pointer in its constructor. The destructor
  deletes TexMgr, then ResourceMgr, then LibraryMgr, and only after that broadcasts
  `msg_DocumentDestroyed`. A listener that handles `msg_DocumentDestroyed` (for example
  `WED_DocumentWindow`, which runs `delete this`) must not touch the managers from its destructor.

### `WED_ResourceMgr` never gets its invalidation messages
A bug, not a design choice (Ben, 2026-09-25) — on [the punch list](../bug-punch-list.md). The fix must handle the dangling-pointer
problem described below.

`WED_ResourceMgr::ReceiveMessage` would `Purge()` on `msg_SystemFolderChanged` /
`msg_SystemFolderUpdated`, but nothing ever calls `AddListener` for it (not its constructor,
not `WED_Document`). That handler is dead code, and it has been since the class was added. So
after a package rescan, or even after changing the X-Plane folder, cached `.obj/.fac/.for/.pol/
.lin/.str/.agp` data stays until the document closes. The only targeted invalidation is
`Purge(vpath)`, which clears **objects only** (`mObj`). `WED_OrthoExport` calls it after rewriting
an `.obj`. A re-written `.pol` is not purged from `mPol`. Before you "fix" this by registering
the listener, read the next entry: a global `Purge()` frees objects that other caches and the
UI still point to.

### Resource-cache pointers are raw and long-lived, and some are unstable
`Get*()` return `const T*` into `unordered_map` values. Those are node-based, so a rehash leaves
the pointers valid. Callers (map layers, `agp_t::obj_t::obj`, `agp_t::fac_t::fac`,
`fac_info_t::xobjs`, `for_info_t::preview`) keep them across frames. Hazards:
- **Facade variants live in a `vector<fac_info_t>`.** `GetFac(vpath, info, variant)` calls
  `push_back` to load a missing variant, and a reallocation invalidates every `fac_info_t*`
  handed out earlier for that vpath, including the one an `.agp` cached in `fac_t::fac`.
  `WED_LibraryPreviewPane` requests variants greater than 0. Not seen as a crash, but the
  invalidation is real C++ semantics — on [the punch list](../bug-punch-list.md), low priority (Ben, 2026-09-25).
- Calling `Purge()` would leave all of the above dangling. That is one reason not to wire up the
  message handler naively.
- `WED_JWFacades` (jetway tunnel → facade map) is built once, lazily, on the first
  `GetJetwayVpath`. `Purge()` never resets it.

### Failed loads are not cached, so they retry every frame (mostly)
`GetObj`, `GetFac`, `GetPol`, `GetLin`, `GetStr`, `GetAGP` and `WED_TexMgr::LookupTexture` return
false/NULL without recording the miss. A missing asset or texture is re-opened from disk on
every redraw. `LookupTexture` also leaks the `glGenTextures` name on each failure. `GetDem` does
the opposite: it inserts `mDem[path]` **before** parsing, so the first call on a bad GeoTIFF
returns false and every later call returns **true with a 0×0 DEM**. `GetAGP` also inserts into
`mAGP` before parsing, so a parse error part-way through leaves a partial entry cached. Both on [the punch list](../bug-punch-list.md).

### `WED_TexMgr` keys on the path string alone and ignores `flags`
(On [the punch list](../bug-punch-list.md).) The cache key is the `path` exactly as passed. It isn't normalized, and relative and absolute
spellings of the same file make separate entries. The `flags` (wrap, mipmap, linear, compress)
are **not** part of the key, so whoever loads a file first decides its sampling for everyone
(for example `WED_TCE` asks without `tex_Linear|tex_Mipmap`, `WED_PreviewLayer` asks with them).
A `TexRef` is a raw `TexInfo*`. `DropTexture` (used only by `WED_OrthoExport`) frees it, so any
cached `TexRef` dangles. The TexMgr listens to nothing. Relative paths resolve through
`gPackageMgr->ComputePath(package, …)` **at load time**. The destructor calls `glDeleteTextures`,
which needs a live shared GL context (see the start-window note under Architecture).

### Package indices are positional and get re-sorted
`WED_PackageMgr` addresses packages by an `int` into custom, then global, then default lists.
Each list is sorted by `SortPackageList`: packages with an airport (earth.wed.xml or apt.dat)
first, pure libraries last, then case-insensitive name order. That is **not**
`scenery_packs.ini` order. Any rescan that changes the list shifts the indices.
`WED_LibraryMgr::res_info_t::packages` rebuilds its data on rescan, but UI state that stores an
index (the `WED_LibraryFilterBar` enum value is a package index) can end up pointing at a
different package.

### `WED_StartWindow::Activate` calls `gPackageMgr->Rescan()` every time it gets focus
If the Custom Scenery listing changed on disk (names, disabled flags), this broadcasts
`msg_SystemFolderChanged`, and every open document does a full library rescan, which is slow on
big installs. If nothing changed, nothing is broadcast. So an edit to a pack's `library.txt`
that doesn't change the package list is **not** picked up. Only `Rescan(true)` (used by
`WED_OrthoExport` after it adds resources) forces a library rescan. Known bug in the
change-detection loop in `Rescan`: `if(o->hasAnyItems != o->hasAnyItems)` compares an entry
with itself, so a package that gains or loses `library.txt` doesn't count as a change. On [the punch list](../bug-punch-list.md).

### Changing the X-Plane folder with documents open leaves mixed state
`SetXPlaneFolder` requires `Custom Scenery/` and `Resources/default scenery/` (`Global Scenery`
is optional). It then calls `Rescan(true)`, which always broadcasts. Afterwards:
- `WED_Document::mFilePath` was computed once in the constructor, so **Save still writes to the
  old install**.
- `ILibrarian::LookupPath`/`ReducePath`, `WED_LibraryMgr::CreateLocalResourcePath`, TexMgr
  relative loads, and scenery export **resolve against the new install**.
- Each LibraryMgr rescans the new install, including a local package of the same name that may
  not exist there. The ResourceMgr and TexMgr caches keep the old install's assets (see above).

`wed_ChangeSystem` is handled only by `WED_StartWindow`, so it is reachable only when that window
is in the command chain. **Unsupported (Ben, 2026-09-25):** changing the folder with documents
open should be blocked — on [the punch list](../bug-punch-list.md).

### `library.txt` parsing: things you'd expect to work differently
All in `WED_LibraryMgr::Rescan`:
- An `EXPORT*` line whose rpath climbs out of the pack (`../`, per `is_no_true_subdir_path`)
  does a `break`. That **stops parsing the rest of that `library.txt`**, not just the one line.
  A bug — it should skip just that line (Ben, 2026-09-25); on [the punch list](../bug-punch-list.md).
- `EXPORT_EXCLUDE` is treated like `EXPORT` (the item is added). `EXPORT_*_SEASON` lines count
  only if the season token contains `"sum"`. `EXPORT_BACKUP` never adds a variant, and a later
  non-backup export replaces the backup's paths.
- Disabled packs (`SCENERY_PACK_DISABLED` in `scenery_packs.ini`, matched on the last directory
  component only) are skipped. Only custom packs can be disabled.
- Status: once a default (LR) pack exports a vpath, **its** status wins and overrides a custom
  library's higher status ("LR libs will always override/downgrade", WED-1123). Otherwise the
  highest status seen wins. `is_customized` marks an LR vpath that a third party also exports.
- `PUBLIC <yyyymmdd>` marks an item "New" until that date. The "now" it compares against uses
  0-based `tm_mon`, so "new" lasts roughly one month longer than written.
- rpaths get `FILE_case_correct`ed in place, because derived texture paths are later opened
  with case-sensitive `fopen`. Trailing non-ASCII and whitespace are stripped
  (`WED_clean_rpath`, WED-1248).
- `real_paths[0]` (the default variant used for previews) comes from the **first pack scanned**.
  With the sort above, that means a custom library that exports the same vpath beats the LR
  default. Local-package files are scanned **last**, so a local file whose relative path matches
  an existing vpath becomes an extra variant, not variant 0.
- The local package is walked recursively. Every file with a known suffix (`obj agp fac for str
  lin pol ags agb`, plus `net` under `ROAD_EDITING`) becomes a public vpath equal to its path
  relative to the pack. `ResourceMgr::GetObjRelative` has a long comment about the resulting
  ambiguity between vpaths and asset-relative paths.

### Status semantics differ between browsing and validation
`res_status` is ordered on purpose: Private < Deprecated < SemiDeprecated < Public < New.
`GetResourceChildren` (library browser) shows only `>= status_Public`, so SemiDeprecated is
**hidden**. `IsResourceDeprecatedOrPrivate` (validation, gateway) flags only
`< status_SemiDeprecated`, so SemiDeprecated **passes**. A vpath that isn't in the table
at all counts as "deprecated or private". `::FLATTEN::.pol` is special-cased as default and public
(`CheckFlattenPolygon`). `GetResourcePath` returns empty on a case mismatch even though X-Plane
itself is case-insensitive here. `GetResourcePath`/`GetObj`/`GetFac` check the variant index
with `DebugAssert` only, which compiles out in release builds, so an out-of-range variant is UB there.

### The library scan mutates the global enum system
`WED_LibraryMgr::RescanLines` calls `ENUM_Create(LinearFeature, …)` for every public
`lib/airport/lines/NN_*.lin` and `lib/airport/lights/slow/NNN_*.str` whose number has no enum
yet. The description is synthesized from the file name. `RescanSurfaces` calls
`ENUM_Import(Surface_Type, …)`. Consequences:
- `ENUM_Init()` must run before the first `WED_LibraryMgr` is constructed (`main()` guarantees it).
- In the `WED_Document` constructor the LibraryMgr is created **before** `Revert()` loads the
  XML. That order matters, because enum properties are stored in XML **by description**
  (`WED_PropIntEnum::ToXML` writes `ENUM_Desc`, and reading uses `ENUM_LookupDesc`). A document
  that uses a library-derived line type loads correctly only if that library is present. If it
  isn't, the property reads back as -1, silently.
- Runtime-created enum ints depend on scan order, so they are valid only within one session.
  They are never removed, even if you switch X-Plane folders.

### Enum descriptions are a file format, and they must be unique within a domain
The full enum rules (the double expansion of `WED_Enums.h`, the (domain, description)
de-duplication that shifts every later constant by one, why 0 is never a valid member, which
forms get persisted) are in [wed-entities.md](wed-entities.md#enum-system-invariants) and its
"What is persisted" table. What matters here:
- Renaming an enum's description text breaks loading of existing `earth.wed.xml` files (see
  above). The export value (third `ENUM` argument) must not change either: apt.dat and DSF I/O
  use it (`ENUM_Export`/`ENUM_Import`), and `WED_PropIntEnumBitfield` writes it to XML. The
  "Ben says: EXPORT values do NOT have to match" comment in `ENUM_Create` is only about the
  de-duplication path. Re-creating an existing (domain, description) with a different export
  value returns the old entry instead of asserting.
- Binary undo (`ReadFrom`/`WriteTo`) stores the raw int.

### `WED_Document::Revert` cancel path calls `throw;` with no active exception
Cancelling "Open backup" after a failed XML parse runs a bare `throw;`, which is
`std::terminate`. Details are under "Load failure" in
[wed-object-model.md](wed-object-model.md#persistence-earthwedxml).

### Save/backup uses plain `rename()`
`Save` backs up with plain `rename()`, which misbehaves on Windows, and one failure path leaves
no `earth.wed.xml` at all. Details are under "Save" in
[wed-object-model.md](wed-object-model.md#persistence-earthwedxml). `Panic()` (the undo system
blew up) writes `earth.wed.crash.xml`.

### `TryClose` deletes the document synchronously
`TryClose` calls `AsyncDestroy()` and then immediately `delete this`, because async destruction
didn't run destructors in all cases (commit e166f33d7). The `GUI_Destroyable` destructor removes
the pending entry. `WED_DocumentWindow::Closed()` calls `TryClose`, and that deletes the window
from inside its own callback (via `msg_DocumentDestroyed`). Nothing may touch `this` after
`TryClose` returns true. `WED_Application::CanQuit` → `TryCloseAll` iterates over a **copy** of
the document set for this reason.

## Architecture

### Startup order (`WED_AppMain.cpp` `main`)
1. Platform init, `GUI_InitClipboard`, open `WED_Log.txt` next to the executable.
2. Construct `WED_Application app`, then `WED_PackageMgr pMgr(NULL)`, which sets `gPackageMgr`.
3. `GUI_Prefs_Read("WED")`, then `WED_Document::ReadGlobalPrefs()`.
4. Create `WED_StartWindow` and the menus. The start window is the first GL window. "Ben says":
   WED can't survive its textures being purged, so one window must always exist to keep the
   shared GL context alive. (The comment names the about box, but "mroe" updated it: it is the
   StartWindow now.)
5. `pMgr.SetXPlaneFolder(pref "packages/xsystem")`, `SetRecentName`.
6. `gFileCache.init()` (see [wed-network-and-filecache.md](wed-network-and-filecache.md)).
7. `WED_AssertInit()` (installs the handler that turns `Assert`/`DebugAssert` into an alert plus a
   `wed_assert_fail_exception`), then `ENUM_Init()`.
8. `*_Register()` for every persistent class (`REGISTER_LIST` / `REGISTER_LIST_ATC`).
9. `start->ShowMessage(string())`. An empty caption enables the buttons **and** handles
   `--package <name>` autostart by dispatching `wed_OpenPackage`. That is why this call comes
   after enum init and class registration: it can open a document.
10. `app.Run()`. On exit: write `packages/xsystem` and `Recent`, `WriteGlobalPrefs`,
    `GUI_Prefs_Write`.

Asserts that fire during the package scan in step 5 use the default handler, not WED's.
Global prefs are written **only** at a clean shutdown, so a crash loses them.

### App vs. document lifecycle
`WED_Application` is thin. `OpenFiles` is empty. It handles the help/URL commands, About,
Preferences (`WED_Settings` window, which writes straight into the `g*` globals), and `CanQuit →
WED_Document::TryCloseAll`. Documents are opened by `WED_StartWindow::HandleCommand
(wed_OpenPackage)`, which runs `new WED_Document(name, bounds)` and then `new
WED_DocumentWindow`. A doc that's already open just gets its window shown. The constructor does
this, in order:
build the managers → `Revert()` (loads the XML, or creates root/selection/choices/world, then
`WED_Repair`, then broadcasts `msg_DocLoaded`) → if there is no XML but the pack has apt.dat,
offer `WED_SceneryImport` → purge undo/redo → (`WITHNWLINK`) start `WED_Server` **only for the
first open document**. The window doesn't exist yet during the constructor's `msg_DocLoaded`, so
`WED_DocumentWindow`'s constructor reads prefs itself. Later `Revert` calls reach it through the
message. Possible off-by-one in the import prompt's package search: the loop runs `this_pkg`
from `CountCustomPackages()` down to 1, so it never checks index 0 and does check one index past
the custom list. Confirmed bug (Ben, 2026-09-25) — on [the punch list](../bug-punch-list.md), high priority.

### Preferences (`IDocPrefs`, implemented by `WED_Document`)
- There are two stores. Per-document `mDocPrefs` / `mDocPrefsItems` are saved in the
  `<prefs>` element of `earth.wed.xml`. The global `sGlobalPrefs` (static in `WED_Document.cpp`)
  is saved in the `[doc_prefs]` section of the `GUI_Prefs` file.
- The default `type` is `doc|global`. **Every write goes to both**, so the last value any
  document wrote becomes the default for new documents. Reads try doc first, then global.
  `ReadIntPref` has a special case: when a value comes from *global*, `doc/export_target`
  returns `wet_latest_xplane` and `map/obj_density` returns the caller's default, so new docs
  don't inherit "unusual" settings. `ReadDoublePref` and `ReadStringPref` have no such filter.
  Int-set prefs are doc-only. `doc/xml_compatibility` is never written globally, because WED
  2.0–2.2 would read it and give false warnings.
- Writing a pref does **not** mark the doc dirty. Panes write their state from their
  `msg_DocWillSave` handler (in `WED_DocumentWindow::ReceiveMessage`). `SetDirty()` sets
  `mPrefsChanged` explicitly (the export-target menu uses it).
- `msg_DocWillSave` and `msg_DocLoaded` carry `reinterpret_cast<uintptr_t>(static_cast<IDocPrefs*>(doc))`.
  A receiver must cast it back to **`IDocPrefs*`**, never to `WED_Document*`: the class has
  multiple and virtual bases, so the pointer values differ.
- App-wide settings are plain globals: `gIsFeet`, `gInfoDMS`, `gFontSize` (clamped 10–18),
  `gCustomSlippyMap` and `gOrthoExport` live in the `[preferences]` section. They are *defined*
  in `WED_Document.cpp` but declared in `WED_Globals.h`. `gModeratorMode` is **not persisted**
  and resets every launch.
- `gExportTarget` (defined in `WED_Globals.cpp`) is **one process-wide value**, but it is stored
  per document as the int pref `doc/export_target`, loaded by `WED_DocumentWindow` and written in
  that doc's `msg_DocWillSave`. With two docs open, whichever loaded last sets it, and saving
  either doc records that value. Its lifecycle, the code that silently switches it to
  `wet_gateway`, and how to add a target are in
  [wed-import-export.md](wed-import-export.md#the-export-target-is-a-global-it-is-sticky-and-some-code-switches-it-for-you).

### `ILibrarian` is path translation, not the library
`ILibrarian` (implemented by `WED_Document`) only converts between package-relative and absolute
paths. It is not the library. `LookupPath` = `gPackageMgr->ComputePath(package, rel)`: a path is
treated as absolute only if `rel[1]==':'` (a Windows drive), so a POSIX absolute path gets
glued under the package. `ReducePath` makes a `../`-relative path with `/` separators, and
returns the input unchanged if it is on a different Windows drive. On macOS, `ReducePath` runs
`popen("cd '<pkg>'; pwd -P")` to resolve symlinks the way NSOpenPanel does. That is a shell
spawn on every call. It breaks on a package path containing `'`, and if the `cd` fails, `pwd`
prints the cwd. Virtual library paths (vpaths) are a separate thing, handled by
`WED_LibraryMgr` (`GetResourcePath(vpath, variant)` returns the absolute file).

### `WED_PackageMgr`
It scans `Custom Scenery` (each pack also gets `hasXML` / `hasAPT` / `hasAnyItems`(library.txt)
flags and `scenery_packs.ini` disabled flags), plus `Global Scenery` and
`Resources/default scenery`. Only the default list counts as "default"
(`IsPackageDefault`). `hasPublicItems` is **not** set by the package scan. It is set as a side
effect of any document's `WED_LibraryMgr::AccumResource` (`AddPublicItems`), so the library
filter list has no per-pack entries until a document is open, and the flag is never cleared.
`CreateNewCustomPackage` / `RenameCustomPackage` broadcast `msg_SystemFolderUpdated`. `Rescan`
broadcasts `msg_SystemFolderChanged` only when the list changed or `alwaysBroadcast` is set.
If the system folder vanishes, `Rescan` just sets `system_exists=false` and broadcasts nothing.
It also reads the X-Plane version from the start of `<xsystem>/Log.txt`
(`GetXPversion`, reported to the gateway as `validatedAgainst`).

### Messages (`WED_Messages.h`) and who listens

| Message | Sent by | Received by |
|---|---|---|
| `msg_SystemFolderChanged` / `msg_SystemFolderUpdated` | `WED_PackageMgr` | every `WED_LibraryMgr` (full `Rescan`), `WED_StartWindow`, `WED_PackageListAdapter`. **Not** the ResourceMgr, TexMgr or map layers |
| `msg_LibraryChanged` | `WED_LibraryMgr::Rescan` (end) | only `WED_LibraryListAdapter` |
| `msg_DocLoaded`, `msg_DocWillSave` | `WED_Document` | `WED_DocumentWindow` (pane prefs, export target) |
| `msg_DocumentDestroyed` | `~WED_Document` | `WED_DocumentWindow` (deletes itself), `WED_StartWindow` |

`msg_PackageDestroyed` is declared but never sent. Private message IDs start at
`WED_PRIVATE_MSG_BASE`. Messages are delivered synchronously (see [gui-framework.md](gui-framework.md)).

### `WED_ResourceMgr` loading notes
- `LoadObj` balances unmatched `ANIM_begin` with synthetic `ANIM_end` commands, so broken assets
  don't unbalance the GL matrix stack. It resolves textures with `process_texture_path`:
  strip the extension, apply `../`, then try `.dds`, then `.png`, and fall back to `.bmp`. If
  there is only `texture_draped`, it is copied to `texture`, and the reverse.
- Objects referenced from `.fac` / `.agp` / `.str` go through `GetObjRelative`. If the name is a
  known vpath it is loaded as a vpath. Otherwise it is resolved relative to the parent asset's
  real path and cached in `mObj` **under that absolute path**, so `mObj` keys mix vpaths and
  absolute paths.
- `GetObj` refuses anything whose third-from-last character isn't `o`/`O`, a cheap way to skip
  `.agp`. It indexes `vpath[size()-3]` with no length check.
- `WritePol` writes a `.pol` file but doesn't update `mPol` or ask for a rescan. The caller has
  to do that (ortho export calls `Rescan(true)` from `~DSF_export_info_t`).

## Other WEDCore files: where they're covered
- `WED_Thing`, `WED_Archive`, `WED_UndoMgr`/`UndoLayer`, `WED_Persistent`, `WED_XMLReader/Writer`,
  `WED_Buffer`/`FastBuffer`: [wed-object-model.md](wed-object-model.md).
- `WED_PropertyHelper` (property items, XML names, enum properties):
  [wed-entities.md](wed-entities.md).
- `WED_Validate*`: [wed-validation.md](wed-validation.md).
- `WED_Clipping`, `WED_GISUtils`, `WED_HierarchyUtils`: geometry helpers used by the entities and
  export ([wed-entities.md](wed-entities.md), [wed-import-export.md](wed-import-export.md),
  [utils-geometry.md](utils-geometry.md)).
- `WED_TCEDebugLayer`: [wed-map-and-tce.md](wed-map-and-tce.md). `WED_Orthophoto`, `WED_Sign_Parser`:
  [wed-entities.md](wed-entities.md) / [wed-import-export.md](wed-import-export.md).
- There is no `WED_Clipboard` class. Copy/paste goes through `GUI_Clipboard`
  ([gui-framework.md](gui-framework.md)).
- `WED_Errors`: `wed_error_exception` (fixed 1 KB message) and `WED_ReportExceptionUI`.
  `WED_Globals.h` also holds DEV-only `debug_mesh_*` helpers that feed `gMeshPoints` / `Lines` /
  `Polygons`, drawn by the debug layer.

## Connections to Other Systems

- **Design rules** ([wed-design-principles.md](wed-design-principles.md)): the stated rules for new code and reviews that this subsystem's conventions should follow — command ownership, pointer vs ID, class vs interface casts, error handling, layering, per-doc prefs, platform reference.
- [wed-object-model.md](wed-object-model.md): `WED_Document` owns the `WED_Archive` and
  `WED_UndoMgr`. `Revert` loads through them inside an undo command. Enum properties persist by
  description.
- [wed-map-and-tce.md](wed-map-and-tce.md): the preview and structure layers are the biggest
  consumers of `WED_ResourceMgr` and `ITexMgr`, and they hold raw cache pointers across frames.
- [wed-ui-panes.md](wed-ui-panes.md): `WED_StartWindow` (opens docs, changes the X-Plane folder,
  rescans on activate), `WED_DocumentWindow` (pref round-trip, export-target menu),
  `WED_LibraryListAdapter` / `WED_LibraryFilterBar` / `WED_LibraryPreviewPane`.
- [wed-entities.md](wed-entities.md): entities query the ResourceMgr (`GetFac`, `GetObj`) for
  sizes and walls. Line and surface enums come from the library scan.
- [wed-validation.md](wed-validation.md): relies on `IsResourceDeprecatedOrPrivate`, `IsResourceDefault`,
  and package paths.
- [wed-import-export.md](wed-import-export.md): exports resolve paths through `ILibrarian` /
  `gPackageMgr`. Gateway code overrides `gExportTarget`. Ortho export writes assets and then
  forces `Rescan(true)`, `Purge(vpath)` and `DropTexture`.
- [wed-network-and-filecache.md](wed-network-and-filecache.md): `gFileCache.init()` in startup,
  and `WED_Server` created by the first document.
- [gui-framework.md](gui-framework.md): `GUI_Broadcaster`/`GUI_Listener` (listeners unregister
  in their destructors), `GUI_Destroyable`, `GUI_Prefs`.
- [bitmap-texture-and-ddstool.md](bitmap-texture-and-ddstool.md): `WED_TexMgr` loads through
  `TexUtils` (`LoadTextureFromFile`, plus DDS/KTX2 direct upload under `LOAD_DDS_DIRECT` /
  `LOAD_KTX2_DIRECT` in `Obj/XDefs.h`).
- [utils-platform-and-files.md](utils-platform-and-files.md): `MF_*` directory iteration,
  `FILE_case_correct`, and the `x_fopen` wrapper.
- [dsf-library-and-dsftool.md](dsf-library-and-dsftool.md): the DSFLib writer reports
  out-of-range values only through `Assert`, so inside WED a bad DSF export reaches
  `WED_AssertHandler_f` (installed by `WED_AssertInit`), which shows an alert and **throws**
  `wed_assert_fail_exception`.
