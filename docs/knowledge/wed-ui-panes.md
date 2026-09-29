# WED Application UI: Windows, Menus, Property & Library Panes

> Source: `src/WEDWindows/`, `src/WEDProperties/`, `src/WEDLibrary/` (plus the glue in
> `WEDCore/WED_Application.cpp`, `WEDCore/WED_AppMain.cpp`, and the menu plumbing in
> `GUI/GUI_Application.cpp`, `GUI/GUI_Window.cpp`, `GUI/GUI_Commander.cpp`, `UI/ObjCUtils.mm`)

## Things That Will Bite You

### Submenus are attached by hard-coded item index
`WED_MakeMenus` (`WED_Menus.cpp`) attaches every submenu with `CreateMenu(title, items, parent, N)`,
where `N` is the parent item's **position in the parent's `GUI_MenuItem_t[]` table, counting
separators**. Insert, remove, or `#if` out a row above an anchor and the submenu lands on the wrong
item. It has broken in practice: `ff5636e5e` ("fix menus entries out of sync") and `cd4f16b18`
("fix duplicate menu entry", the Advanced index going from 13 to 14). Two anchors are computed from
feature flags. File→Advanced is `14 + 2*HAS_GATEWAY + ROAD_EDITING`, and the View→3D Preview anchor
switches on `WITHNWLINK`. These formulas assume the flags are exactly 0 or 1. Linux FLTK does not
store separators as items, so `GUI_Application::CreateMenu` subtracts them. You still count them.
The FLTK path also `DebugAssert`s that a submenu's anchor row has `cmd == 0`.

### Several enum ranges in `WED_Menus.h` are order-coupled to other files
- `wed_Export900 … wed_Export1212` must stay parallel to `WED_Export_Target` (`WEDCore/WED_Globals.h`).
  `WED_DocumentWindow` converts with `wet_xplane_900 + (command - wed_Export900)`, and
  `WED_GetTargetMenuName` indexes `kExportTargetMenu[]` by target number. `wed_ExportGateway` is
  special-cased because `wet_gateway == 99`.
- `wed_AddMetaDataBegin+1 … wed_AddMetaDataEnd-1` must match `known_keys[]` in
  `WEDImportExport/WED_MetaDataKeys.cpp` row for row, including the `#if GATEWAY_IMPORT_FEATURES`
  hole for Credits. That submenu is built at runtime from `META_KeyDisplayText`. The
  Begin/End sentinels also size the static `kAddMetaDataMenu[]` (one spare slot for the NULL terminator).
- Command values change with build flags because some enumerators sit inside `#if HAS_GATEWAY` etc.
  **Never persist a `wed_*` command id.**

### Returning 0 from `CanHandleCommand` passes the question up the chain; it does not mean "disabled"
`GUI_Commander::DispatchCanHandleCommand` walks from the focused commander up to the root and
stops at the **first** handler that returns non-zero. So "disabled" really means that nobody in
the chain returned 1. `WED_Application::CanHandleCommand` depends on this. It catches
`gui_Undo`/`gui_Redo` when the document window has nothing to undo, resets the label to
"&Undo"/"&Redo", and returns 0.

### Menu labels set through `ioName` are sticky on every platform
Mac passes the item's current title in as `ioName`. The Windows path (`EnableMenusWin`) and the
Linux path (`update_menu_recursive`) only rewrite the label when `ioName` comes back non-empty.
If your `CanHandleCommand` customizes a label, it has to set one on **every** return path. For
example, `WED_CanSetCurrentAirport` returns 0 without touching the name when the selection is not
exactly one airport. By code reading, the item then keeps its last "Edit Airport X" label rather
than going back to "No Airport Selected".

### `CanHandleCommand` runs for every item, every time a menu opens
On Windows, `WM_INITMENU` → `GUI_Window::EnableMenusWin` evaluates **every** command in the menu bar.
On Mac, the `NSMenu` delegate asks per item. `CanX` functions have to be cheap and null-safe. Past
bugs: `b01a002b5` (the menu got very slow on large files because a `Can` function recursed the
whole world; it was changed to look only at the selection) and `7ffe6656a` (clicking the menu bar
crashed on a new empty document because `WED_CanCopyToAirport` had no null check on the current
airport).

### `HandleCommand` must re-validate; not every path goes through `CanHandleCommand`
Buttons (`WED_StartWindow::HandleCommand` carries the comment "buttons do NOT check whether we are
command-enabled, so recheck!"), programmatic `DispatchHandleCommand` calls, and commands queued by
`BeginDefer` and replayed at mouse-up (`GUI_Commander::EndDefer`) all skip the enable check. By
the time a deferred command replays, the state may have changed. Follow the pattern in
`WED_DocumentWindow::HandleCommand` (`gui_Undo: if (um->HasUndo())`), or make `WED_DoX` safe on
empty input.

### Map-pane commands work because the document window forwards them
`WED_MapPane` is a `GUI_Commander` but does **not** override `HandleCommand`. Its handlers are
named `Map_HandleCommand` / `Map_CanHandleCommand`, and the only caller is the `default:` branch
of `WED_DocumentWindow`'s switch. That is why View-menu commands work while the property table
has focus. The TCE pane's equivalents are never forwarded, so they are dead code (see
[wed-map-and-tce.md](wed-map-and-tce.md#wedtce-where-it-really-differs-from-wedmap)).

### Text fields take Cut/Copy/Paste/Clear/Select All, but not Undo
When a `GUI_TextField` has focus (for example, editing a property cell), it handles
`gui_Cut/Copy/Paste/Clear/SelectAll` itself. `gui_Undo` falls through to the document window, so
Cmd/Ctrl-Z during a cell edit undoes the last **document** command. The Edit menu's "Clear" item
deliberately has no Delete-key shortcut (see the comment in `kEditMenu`). Delete/Backspace is
caught in `WED_DocumentWindow::HandleKeyPress` instead, after `Map_KeyPress` gets first try, and
re-dispatched as `gui_Clear` so that `PreCommandNotification` hooks still run.

### A command that fires mid-drag would nest undo commands
`WED_UndoMgr::StartCommand` asserts (and can "panic"-abort) when a command starts while another
is open. The UI prevents this in two ways. `GUI_Window::ClickDown` calls `BeginDefer` on the
**root commander (the app)**, so commands and keys are queued until `ClickUp` has run the pane's
`MouseUp`. And `WED_HandleToolBase` registers as a `GUI_Commander_Notifiable`, so its
`PreCommandNotification` commits any in-flight drag or selection operation before a command
runs. The notifiables are called only for commands that are not deferred, so the second mechanism
is a backstop for paths that skip the first (a different commander root, a programmatic dispatch
with no button down). If you add a UI path that calls `StartOperation`, keep both in mind.

### Property edits are wrapped in undo explicitly, not "automatically"
`WED_PropertyTable::AcceptEdit` calls `StartCommand("Change <prop_name>")` on the **first object it
actually writes** and calls `CommitCommand` after the loop, so it produces one undo step for all
targets. The file picker for `gui_Cell_FileText` runs **before** the command opens, so no modal
dialog happens inside an open command. Keep it that way. If no target matched, no command is
started. Selection clicks in the tables use `StartOperation("Change Selection")` in
`SelectionStart` and `CommitOperation` in `SelectionEnd`. `TabAdvance` uses "Select Next".
Selection changes are therefore undoable.

### Apply-to-all copies the clicked object's whole value
Holding Option/Alt or Ctrl/Cmd while editing (`GUI_TextTable`: `GetModifiersNow() &
(gui_OptionAltFlag|gui_ControlFlag)`) sets `apply_all`. `AcceptEdit` then writes to every selected
`WED_Thing` that has a property **with the same name and the same `prop_kind`**. It never checks
enum domain. For a non-exclusive `prop_EnumSet`, the popup toggles one value in the **clicked
cell's** set, and that whole set is then written to all targets. It is not a per-object toggle.

### The property UI binds to property names as strings
The hierarchy pane's columns are the literal names `"Locked"`, `"Hidden"`, `"Name"`, set in
`WED_DocumentWindow`. `WED_PropertyTable::GetCellContent` special-cases `"Locked"`/`"Hidden"` to
draw lock/eye icons and to show inherited state as `bool_partial`, using `AnyLocked` and
`AnyHidden`. These come from `PROP_Name("Locked", …)` etc. in `WED_Entity.cpp` and
`WED_Thing.cpp`. If you rename one, the hierarchy pane silently goes blank. Property names that
start with `.` are hidden from the Selection tabs (`RecalculateColumns`).

### Tab order in the property tab pane is an implicit enum
`WED_DocumentWindow` adds the tabs Selection, Pavement, Taxi+Routes, Lights+Marking, 3D+Objects,
Exclusion+Boundary, Texture. `WED_MapPane` listens to the `GUI_TabPane` and handles **any
non-toolbar message** as `SetTabFilterMode(inParam)`. That value must line up with the private
`tab_*` enum in `WED_MapPane.cpp` (marked "Must be kept in sync with TabPane"). All six property
tabs are identical `propPane_Selection` panes. The per-tab behavior comes entirely from the map's
hide and lock filters.

### Destruction order: panes outlive the document's managers
`gui_Close` or the window's close box calls `WED_Document::TryClose` (save prompt), which calls
`AsyncDestroy()` and then immediately `delete this`. `~GUI_Destroyable` removes the object from the
dead list, so the async path is effectively cancelled. `~WED_Document` deletes
`mTexMgr`, `mResourceMgr`, `mLibraryMgr`, **then** broadcasts `msg_DocumentDestroyed`. On that
message `WED_DocumentWindow::ReceiveMessage` does `delete this`, and `WED_StartWindow` unlocks the
package. So every pane is destroyed after the texture, resource, and library managers are gone,
but while `mArchive` is still alive (it is a member and is destroyed after the dtor body). Pane
destructors must not touch those managers. Today they are all empty: `WED_LibraryPane`,
`WED_LibraryListAdapter`, `WED_PropertyPane`, `WED_PropertyTable`. `GUI_Broadcaster`'s dtor
detaches its listeners, so the adapter's listener registration on the dead `WED_LibraryMgr` is
safe. Also, the window is gone by the time `HandleCommand(gui_Close)` returns 1. Never touch a
member after `TryClose()`. `WED_DocumentWindow::Closed()` returns `false` because the close
happens through the document.

### The start window must exist before the menus and must never be deleted
`WED_AppMain` creates `WED_StartWindow` first and then calls `WED_MakeMenus`. On Windows,
`GUI_Application::GetMenuBar` needs an existing HWND. Every later non-modal, non-popup
`GUI_Window` **copies** the first window's menu bar: `CopyMenusRecursive` on Windows,
`mMenuBar->copy(...)` on Linux. The comment in `WED_AppMain` explains that the first window holds
the shared GL context that keeps textures alive. `WED_StartWindow::Closed()` therefore dispatches
`gui_Quit` instead of closing. `delete start` happens only after `app.Run()` returns.

### The 3D preview window is its own commander root
`WED_MapPreviewWindow` is created with a `nullptr` commander parent, so it is not in the app's
focus chain. Its `HandleCommand` forwards only the three preview commands to the document window.
Everything else returns 0 on purpose, so that editing never happens from a window where the user
can't see the result. Menu **enable** state, however, is computed through the app's chain (Mac
`MenuUpdateCB`, Windows `EnableMenusWin`). While the preview window is key, items may show as
enabled and then do nothing. [Needs Runtime]

### Don't edit `WED_GroupCommands.cpp.better`
It is a stale copy (last touched in `c89843f9d`). No build file references it. All command
implementations live in `WED_GroupCommands.cpp`.

## Architecture

### Window assembly (`WED_DocumentWindow` constructor)
One window per open package. `WED_StartWindow::HandleCommand(wed_OpenPackage)` constructs a
`WED_Document` and a `WED_DocumentWindow`, with the **app** as commander parent (`GetCmdParent()`
of the start window, not the start window itself). The pair is tracked in the static `sDocs` so a
second open just `Show()`s the existing window. Layout is a `GUI_Packer` holding the map's top bar
and three nested `GUI_Splitter`s. In the code, `gui_Split_Horizontal` means a left/right split
("BEWARE! HORIZONTAL IS VERTICAL"). The splitters hold:
- library side: `WED_LibraryPreviewPane` over `WED_LibraryPane`
- centre: `WED_MapPane`
- property side: a `GUI_TabPane` (6 × `WED_PropertyPane(propPane_Selection)` plus `WED_TCEPane` as the "Texture" tab) over the hierarchy `WED_PropertyPane(propPane_Hierarchy)`.

Cross-wiring happens in the constructor and depends on order. The library pane is built before
the map because `WED_MapPane` takes `lib->GetAdapter()`. After that,
`lib->GetAdapter()->SetMap(mMapPane, libprev)` closes the loop. `prop_tabs->AddListener(mMapPane)`
connects tab changes to the map filter. Each property pane registers its `WED_PropertyTable` as a
listener on the archive. Splitter positions, window rect, map state, preview-window state,
hierarchy-closed set, and export target round-trip through doc prefs on `msg_DocWillSave` /
`msg_DocLoaded` (`window/*`, `PropertyPane0/Closed`, `doc/export_target`). `gExportTarget` is a
**global**, so the export target of whichever document loaded or saved last applies. The window also
writes `doc/xml_compatibility` and warns on open when the file came from a newer WED.

`wed_autoOpenLibPane` / `wed_autoOpenPropPane` / `wed_autoClosePane` are menu-less internal
commands. `WED_LibraryPane::MouseMove` and `WED_PropertyPane::MouseMove` dispatch them when a side
pane is narrower than 100 px, so the window temporarily widens it. `wed_Map3D/ATC/Pavement/Selection`
are also menu-less. In moderator mode, `WED_PropertyTable::SelectionEnd` dispatches them together
with `wed_ZoomSelection`.

### Command travel: OS menu → handler
Command ids are the `GUI_MenuItem_t::cmd` values. `gui_*` ids (`GUI/GUI_Menus.h`) start at 1000,
and `wed_*` ids start at `GUI_APP_MENUS` (2000).

| Platform | Item picked | Enable/check/label update |
|---|---|---|
| Mac | `menu_picked:` on the key window's `XWin` → `GUI_Window::GotCommandHack` → `DispatchHandleCommand` | `NSMenu` delegate `menu:updateItem:` → `GUI_Application::MenuUpdateCB` → **app**`->DispatchCanHandleCommand`; strips `&` |
| Windows | `WM_COMMAND` (menu or accelerator) → that window's `DispatchHandleCommand` | `WM_INITMENU` → `EnableMenusWin`: evaluates all ids once via the first window's root (the app) and applies the result to **every** window's copy of the bar |
| Linux (FLTK) | `menu_cb` → the bar's parent `GUI_Window::DispatchHandleCommand`; windows without a bar use `GUI_Window::handle(FL_SHORTCUT)` → `update_menus` → `test_shortcut` | `update_menus_cb` when the bar is clicked |

Dispatch starts at the focused leaf of that window's commander chain and walks up: text field →
`WED_PropertyTable` → `WED_PropertyPane` → (tab pane owner →) `WED_DocumentWindow` → `WED_Application`
→ `GUI_Application`. If the root's defer level is above 0, `DispatchHandleCommand` queues the
command and returns. Otherwise, before the walk, it calls every registered
`GUI_Commander_Notifiable::PreCommandNotification`. In practice, document commands live in the big switch in
`WED_DocumentWindow::HandleCommand` / `CanHandleCommand`, which calls `WED_DoX(mDocument)` /
`WED_CanX(mDocument)` in `WED_GroupCommands.cpp` (and `WED_ConvertCommands.cpp`, importers,
exporters). Those functions take `IResolver*` and do their own `StartCommand`/`CommitCommand` or
`StartOperation`/`CommitOperation`. Commands that need no document (Help URLs, About, Prefs, Quit)
live in `WED_Application` / `GUI_Application`. Start-window commands (New/Open Package, Change
X-System) live in `WED_StartWindow`. That window is a **sibling** of each document window under
the app, not an ancestor. So by code reading, New/Open/Change X-System are disabled whenever a
document window has focus: nothing in the document chain returns 1 for them. [Needs Runtime]

### Per-platform menu differences
- **App menu.** On Mac, About/Prefs come from the `WEDMainMenu` nib (tags 1000/1001 = `gui_About`/`gui_Prefs`),
  and Quit goes through `applicationShouldTerminate` → `TryQuitCB` → `CanQuit` →
  `WED_Document::TryCloseAll`. `kAppMenu[]` in `WED_Menus.cpp` is unused. On Windows/Linux,
  `#if IBM || LIN` rows add Preferences/Exit to the end of File and About to the end of Help. They
  are placed after all submenu anchors, so indices stay the same across platforms.
- **Top-level placement.** Mac inserts each top-level menu before the nib's last menu, except the one titled "Help".
- **Shortcuts.** `gui_ControlFlag` maps to Cmd on Mac and Ctrl elsewhere. Windows builds one global
  accelerator table in `GUI_Application::Run` from rows registered by `RebuildMenu`, so shortcuts
  added after startup never take effect. It uses `VkKeyScan`, which depends on the keyboard layout.
  Linux maps to FLTK shortcut codes.
- **Label syntax.** `&` marks a mnemonic (stripped on Mac). A leading `;` means disabled
  (`IsDisabledString`). Linux works around FLTK treating `/` as a submenu separator.
- **Per-window bars.** Windows and Linux give every window its own menu-bar copy (see the `Ben says`
  comment in `WED_Menus.cpp`; it mentions Qt, but the Linux port is now FLTK). Modal and popup
  windows get no bar. WED never rebuilds menus after `WED_MakeMenus`. Dynamic text is done only
  through `ioName`.

### Selection → panes
Selection is the persistent `WED_Select`. Its mutators call `StateChanged(wed_Change_Selection)`.
Panes see nothing until `WED_UndoMgr::CommitCommand` broadcasts `msg_ArchiveChanged(change_mask)`
on the archive. `WED_PropertyTable` ignores `msg_ArchiveChangedEphemerally`, so its cache is stale
during an open command. It is not told about the change until commit. On `msg_ArchiveChanged`,
each table:
- invalidates its cache on `CreateDestroy | Topology`, and also on `Selection` when it is a
  selection-only table ("Set this to false FIRST, lest we have an explosion due to a stale cache");
- calls `RecalculateColumns()`. For the Selection panes this calls `GetColCount()` and so rebuilds
  the cache **eagerly**, even on hidden tabs. That is six rebuilds per document window per commit;
- broadcasts `GUI_TABLE_CONTENT_RESIZED` so its `GUI_Table` redraws.

The same broadcast reaches `WED_DocumentWindow`, which updates the title's dirty mark, and the map
and TCE (see [wed-map-and-tce.md](wed-map-and-tce.md)).

### Property table model (`WED_PropertyTable`)
One class drives both layouts:
- **Hierarchy** (`propPane_Hierarchy`): one row per `WED_Thing`, walked from `WED_GetWorld`.
  `GUI_Table` rows count from the bottom, so `FetchNth` reverses the index. Disclosure state is
  `mOpen` (id→open; a missing entry means open, so new things show expanded). The closed set
  persists as `PropertyPane0/Closed`. The Gateway importer receives `mPropPane` so it can collapse
  groups. Drag and drop re-parents through `WED_DoMoveSelectionTo`. This is only allowed when the
  pane is not selection-only and has no class filter (`mFilter`, always empty in WED today). The
  root world row is not editable.
- **Selection** (`propPane_Selection` → vertical, dynamic columns, selection-only): **one column per
  selected object**, in hierarchy order. `RebuildCache` prunes the walk to "selection and its
  ancestors". Rows are the **union of property names** across the selection (`RecalculateColumns`
  inserts at the front so "Name" ends up on top). A cell whose object lacks that property is
  `gui_Cell_None`. **There is no value merging across objects.** Heterogeneous selections just show
  sparse columns, and "edit all" is the modifier-key `apply_all` path described above. Aggregating
  values inside one object (a chain's lines or lights shown as the union over its nodes) is
  `WED_PropIntEnumSetUnion` in `WEDCore/WED_PropertyHelper.cpp` (see [wed-entities.md](wed-entities.md)).

`propPane_Filtered` / `propPane_FilteredVertical` exist, but no caller uses them.
**Search** (hierarchy filter bar → `SetFilter`) switches to `mSortedCache`, built by
`collect_recusive`. It matches the name case-insensitively, and also the resource path wrapped as
`^path$` so that `^Red Line$`-style exact queries work. Groups, airports, and flows stay visible
when any child matches. While a search is active, disclosure toggles are no-ops and every
disclosable row reports open. Invalidation re-runs `Resort()` rather than `RebuildCache()`, and
clearing the search rebuilds the main cache. `DoDeleteCell` exists only for airport metadata rows.
It `static_cast`s `FetchNth(0)` to `WED_Airport` and assumes metadata keys are the last properties.
Polygon rings (children of `WED_GISPolygon`) always show Locked/Hidden as partial and can never be
set.

### Cell editors (sign editor, line and road selectors)
`GUI_TextTable::CreateEdit` (in the **GUI** layer) directly constructs `WED_Sign_Editor`
(`gui_Cell_TaxiText`), `WED_Road_Selector` (`gui_Cell_RoadType`), and, under `#if WED`,
`WED_Line_Selector` (`gui_Cell_LineEnumSet`). All are `GUI_EditorInsert` subclasses. This is a
layering inversion: `GUI/` depends on `WEDWindows/`, so the GUI text table can't be linked without
them. Only the WED target builds `GUI_TextTable.cpp`. Each editor is parented to a full-window
`GUI_MouseCatcher`, so clicking outside ends the edit. When `SetData` returns false, the code falls
back to a plain `GUI_TextField`. That happens when `WED_Sign_Parser` rejects the sign code, or when
the current value is missing from the line dictionary. So a malformed sign string is edited as raw
text. Results come back through `GetData` → `AcceptEdit`. Enum and enum-set cells without a custom
editor use `PopupMenuDynamic`.

### Library browser (`WEDLibrary/`)
`WED_LibraryListAdapter` is a `GUI_TextTableProvider` over `WED_LibraryMgr` virtual paths. It
builds a flat cache with synthetic `Local/` and `Library/` roots and reverses it for bottom-up rows.
The long comment block in the header documents the prefix convention: always go through
`GetNthCacheIndex(i, noPrefix)`. Library selection is **not** document state and is **not** undoable.
`SetSel` pushes the resource to `WED_LibraryPreviewPane::SetResource` and to
`WED_MapPane::SetResource`, which arms the matching create tool **and switches the map toolbar to it**.
The preview pane broadcasts `WED_PRIVATE_MSG_BASE` with a `const char*` into its own `mRess` vector
when a tile is clicked in directory-grid mode. The adapter must copy it synchronously, and it does.
Clicking a `.pol` preview calls `WED_ResourceMgr::SetPolUV`, which sets the sub-texture UV box the
polygon tool will use. That is shared resource-manager state, not undoable. The preview constructor
queries GL (`GL_SAMPLES`, FBO support, retina scale) to decide on MSAA, so a GL context must be
current when it runs. The filter bar (`WED_LibraryFilterBar`, which lives in `WEDWindows/`) starts
at `pack_Default` while the adapter's `mCurPakVal` starts at `pack_Library`. The header comment
says the two must be kept in sync. [Needs Runtime: whether the first list shows all libraries or
Laminar only]

`WED_Colors` (the `wed_Table_*`, `wed_TextField_*`, `wed_Tabs_Text` tokens the panes use) is in
`WEDMap/`, not here.

## Recipe: add a new menu command

Checked against the diffs of recent additions (`cfd8a193a` Import Scenery, `abce06a6b` 12.1.2 export
target). This recipe has not been build-tested.

1. **Id.** Add `wed_MyCmd` to the enum in `WEDWindows/WED_Menus.h`. Put it next to its menu's
   group, and **not** inside the Export-target or AddMetaData ranges, whose order is load-bearing.
   Shifting later values is harmless because ids are never persisted.
2. **Menu row.** Add `{ "&My Command...", key, flags, 0, wed_MyCmd }` to the right table in
   `WED_Menus.cpp`. `key` is 0 or a capital letter or `GUI_KEY_*`, and `flags` is a combination of
   `gui_ControlFlag|gui_ShiftFlag|gui_OptionAltFlag`. Grep the tables for the same key and flags to
   avoid a collision (both `'B'`+Ctrl and `'B'`+Ctrl+Shift are already taken).
3. **Fix anchors.** If the new row sits **above** any `CreateMenu(..., parent, N)` anchor in the
   same parent table, increase `N` (and any flag formula). For a new submenu, add a row with
   `cmd = 0` as its anchor, then a `CreateMenu` call. Put platform-conditional rows below all anchors.
4. **Handler.** Choose the owner:
   - document-level: add `case wed_MyCmd:` to **both** `WED_DocumentWindow::HandleCommand` and `CanHandleCommand`;
   - map/view state: add it to `WED_MapPane::Map_HandleCommand` / `Map_CanHandleCommand` (reached through the window's `default:`);
   - no document needed: add it to `WED_Application` (and to `WED_StartWindow` if it acts on the package list).
   Put the logic in `WED_CanMyCmd(IResolver*)` / `WED_DoMyCmd(IResolver*)` (usually in
   `WED_GroupCommands.cpp/.h`).
5. **Can function.** Return 1 to enable, and set `ioCheck` for a checkmark. Keep it O(selection),
   make it null-safe on an empty document, and set `ioName` on every path if you change the label.
6. **Do function.** Wrap every mutation in `StartCommand("My Command")` … `CommitCommand()` (or
   `AbortCommand`), or use `StartOperation` on the selection. Run modal dialogs and alerts
   **outside** the open command. Re-check preconditions. See [wed-object-model.md](wed-object-model.md).
7. **MCP name.** Add `C(wed_MyCmd)` to `WEDMCP/WED_MCPCommandNames.cpp` at the same position and under the same
   `#if` guards. DEV builds assert at startup if the table has a gap. See [wed-mcp.md](wed-mcp.md).
8. **New files.** List them explicitly in `cmake/WED.cmake`. Historic commits also touch the Xcode,
   MSVC, and Code::Blocks project files.
9. **Test.** Test on each OS if you can. The shortcut paths (Windows accelerators, Mac key
   equivalents, FLTK shortcuts) and label refresh differ. [Needs Runtime]

## Connections to Other Systems

- [wed-mcp.md](wed-mcp.md): the MCP server runs menu commands by name through the same focus chain (`FocusChain(1)` on the document window, then `gApplication->DispatchHandleCommand`).
- [gui-framework.md](gui-framework.md): `GUI_Commander` (focus chain, defer), `GUI_Application`/`GUI_Window` menu plumbing, `GUI_TextTable`, `GUI_Broadcaster` lifetime semantics.
- [wed-core-services.md](wed-core-services.md): `WED_Document` lifetime (`TryClose`, prefs messages), `msg_*` ids, `WED_PackageMgr`, `WED_LibraryMgr`, `WED_ResourceMgr`.
- [wed-object-model.md](wed-object-model.md): `WED_UndoMgr` and command nesting, `WED_Select` as persistent state, change-mask bits.
- [wed-entities.md](wed-entities.md): `IPropertyObject` / `PropertyInfo_t`, `WED_PropertyHelper` (including the `SetUnion` aggregates), and the entity property names the panes bind to (`Locked`/`Hidden`/`Name`, `.`-prefixed hidden props, airport metadata).
- [bitmap-texture-and-ddstool.md](bitmap-texture-and-ddstool.md): `WED_LibraryPreviewPane` decodes `.lin` base textures with `LoadBitmapFromAnyFile` and reads the raw `ImageInfo` bytes for line luminance, so it must assume BGR(A) with row 0 at the bottom. Its `.pol`/`.lin` previews go through `WED_TexMgr::LookupTexture`.
- [wed-map-and-tce.md](wed-map-and-tce.md): `WED_MapPane` (tab filter modes, `Map_*` commands, create tools armed from the library pane), `WED_TCEPane`, `WED_HandleToolBase::PreCommandNotification`, `WED_Colors`.
- [wed-import-export.md](wed-import-export.md): the File-menu import and export commands, `WED_MetaDataKeys` ordering, `gExportTarget`.
- [wed-validation.md](wed-validation.md): `wed_Validate` → `WED_ValidateApt`.

## Open Questions (all answered 2026-09-25)

- ~~`TryClose` `AsyncDestroy()` + `delete this`~~ — answered 2026-09-25: leftover from `e166f33d7`
  (the async path never runs); cleanup on [the punch list](../bug-punch-list.md).
- ~~Preview window commands~~ — answered 2026-09-25: `WED_MapPreviewWindow` should route commands to
  its document window; on [the punch list](../bug-punch-list.md).
- ~~`wed_UpgradeArt` / `wed_EdgePavement`~~ — answered 2026-09-25: both are **planned features**, not
  dead code. Leave the menu items in place.
- ~~`pack_Default` / `pack_Library` mismatch~~ — answered 2026-09-25: a bug; on [the punch list](../bug-punch-list.md).
