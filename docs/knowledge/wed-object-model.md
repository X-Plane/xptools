# WED Object Model, Archive and Undo

> Source: `src/WEDCore/` (`WED_Persistent`, `WED_Archive`, `WED_UndoMgr`, `WED_UndoLayer`,
> `WED_Document`, `WED_XMLReader`, `WED_XMLWriter`), `src/WEDEntities/` (`WED_Thing`,
> `WED_Entity`), `src/Interfaces/` (`IOperation`, `IResolver`, `IBase`), class registration
> in `src/WEDCore/WED_AppMain.cpp`.

## Things That Will Bite You

### Every mutation must be inside a command, and `StateChanged()` goes BEFORE the change
- `WED_Persistent::StateChanged(kind)` calls `WED_Archive::ChangedObject`, which hands the
  object to the active `WED_UndoLayer`. The first `StateChanged` for an object in a command
  snapshots it by calling `WriteTo`. So you must call it **before** you touch any member,
  even though the name is past tense (the header says so in capitals). If you call it after
  the change, undo "restores" the new state and does nothing.
- Property members (`WED_PropIntText` etc.) do this for you: assigning one calls
  `PropEditCallback(1)` first, which in `WED_Thing` calls `StateChanged(wed_Change_Properties)`.
  Any **non-property** state (hand-written maps or vectors like `WED_KeyObjects::choices`,
  `WED_Airport` metadata, `WED_Select::mSelected`) needs an explicit `StateChanged()` in every
  setter. Missing calls have caused real bugs, e.g. WED-995 (runway edits not showing) and the
  metadata undo fixes in the git log.
- **Mutating with no command open**: `ChangedObject`, `AddObject` and `RemoveObject` hit
  `DebugAssert(!"Error: object changed outside of a command.")`. In DEV builds the WED assert
  handler (`WED_Assert.cpp`) shows an alert and **throws** `wed_assert_fail_exception`. In
  release builds `DebugAssert` compiles away. The change is applied, but it is not undoable,
  the document is not marked dirty, and no `msg_ArchiveChanged` is sent. That means a release
  build fails silently where a DEV build crashes.
- Creating an object (`CreateTyped` → `PostCtor` → `AddObject`), `Clone()`, `Delete()`,
  `SetParent`, `AddSource` and selection changes all count as mutations and need a command.

### Commands do not nest
- `WED_UndoMgr::__StartCommand` while a command is already open counts as a bug. It backs out the
  open command (executing its layer with `UNDO_DISCARD`), calls `WED_Document::Panic()` to
  write `earth.wed.crash.xml`, then `AssertPrintf`s, which throws in all builds. Two edits are
  lost.
- `IOperation` gives you no way to nest either. `WED_Thing::__StartOperation` just forwards to
  `__StartCommand`. The interface only exists so that code holding an `IBase`/`ISelection`
  can open a command without knowing about `WED_Persistent`.
- Convention for reusable helpers: take a flag (`WED_DoDuplicate(resolver, wrap_in_cmd)`,
  `WED_DoConvertTo(..., in_cmd = true)`) or document that the caller opens the command.
  Before calling any `WED_Do*` function from inside your own command, check whether it opens
  its own.
- Close the command (Commit or Abort) **before** any modal UI (`DoUserAlert`,
  `ConfirmMessage`). Commit 2ca21cd2d fixed a crash that came from calling `DoUserAlert`
  before Abort/Commit. The mechanism (confirmed by Ben, 2026-09-25): the modal loop dispatches
  events — timers, redraws — into code that assumes no command is open.
- Every early `return` inside a command must Abort or Commit first. The only
  `try/catch` that protects a command is in `WED_Document::Revert`.
- `StartCommand(x)` and `StartOperation(x)` are **macros** (in `WED_Persistent.h` and
  `IOperation.h`) that add `__FILE__/__LINE__` and call `__StartCommand`/`__StartOperation`.
  Do not declare any other function with those names. The file/line pair is what the
  nested-command assert prints, so it is the first clue when chasing a panic.

### Raw pointers do not survive undo, redo or revert
- `WED_UndoLayer::Execute` restores objects in three ways:
  - **changed** objects: `ReadFrom` into the *same* C++ object, so their address stays the same;
  - **created** objects: `Delete()`;
  - **destroyed** objects: `WED_Persistent::CreateByClass(class, archive, same_id)`, which
    builds a **new C++ object at a new address** with the old ID.
- The **object ID** is the only identity that stays stable. Store IDs (`GetID()`) and
  re-`Fetch` them, or re-resolve them through `IResolver`, whenever you cross a command
  boundary. All references inside the model are already IDs: parent, children, sources,
  viewers, selection (`WED_Select::mSelected` is a `set<int>`) and key objects.
- `WED_Archive::Fetch` returns NULL for a deleted ID. `RemoveObject` leaves a NULL
  tombstone in the map rather than erasing the entry. That fix (WED-1520) stopped
  `ClearAll()` from invalidating its own iterator on Revert. Any loop over `mObjects` must
  skip NULLs.
- `WED_PropertyTable` caches pointers and drops the cache on
  `wed_Change_CreateDestroy | wed_Change_Topology`. Pure property edits send only
  `wed_Change_Properties`, so a cache keyed on pointers stays valid. Any cache you add needs
  the same mask discipline.
- Suspect case: `WED_SceneryPackExport` (Gateway target) does `MarkUndo()`, runs the
  upgrade heuristics, exports, collects `set<WED_Thing*> problem_children`, then calls
  `UndoToMark()`, and only after that selects `problem_children`. If a problem object was
  *created* by the heuristics, its pointer dangles by then. Confirmed from source 2026-09-25 (heuristics do create objects, e.g. vehicle replacements); low-likelihood — on [the punch list](../bug-punch-list.md).

### `ReadFrom` must not touch peers, and cross-object fixups belong in `PostChangeNotify`
- `Execute` walks a `hash_map` in arbitrary order. While `ReadFrom` runs on one object, its
  parent, children or sources may not exist yet or may still hold stale state. Any work that
  follows links (cache invalidation, for example) must go in `PostChangeNotify()`, which runs
  only if `ReadFrom` returned `true`, and only after every object in the layer has been
  restored. That is the WED-728 fix (crash undoing edge creation). `WED_Entity::ReadFrom`
  returns `true`, and its `PostChangeNotify` does `CacheInval(cache_All)`.
- If you override `ReadFrom`, return the base class's result (see `WED_Airport`,
  `WED_KeyObjects`). Dropping it disables the post-restore cache invalidation.
- `WriteTo` and `ReadFrom` must be exactly symmetric: the format is positional binary with no
  tags. `WED_UndoLayer` also appends the per-object dirty int after your data.
- XML load does **not** call `PostChangeNotify`. `FromXML`/`StartElement` must also avoid
  dereferencing peers, because references may point forward in the file. That is harmless
  because entity caches start out invalid (`cache_valid_ = 0`).

### Entity caches: invalidation is edge-triggered
- `WED_Entity`'s bounds cache (`CacheInval`/`CacheBuild`) only passes an invalidation up while
  the bit is still valid, so leaves must re-arm themselves. The per-class protocol, the WED-828
  re-arm in `WED_GISChain::RebuildCache` and the fragile `WED_GISEdge` cache are in
  [wed-entities.md](wed-entities.md#bounds-cache-protocol-wed_entity). What the object model
  adds is the restore path: undo restores go through `PostChangeNotify` (above), and XML load
  relies on caches starting invalid.
- **Archive-level cache key.** Tools that cache derived data (vertex tool, marquee tool, TCE
  marquee) compare `WED_Archive::CacheKey()`. Every create, change and destroy, every
  undo/redo, every `AbortCommand` and the end of XML load all bump it. The undo/redo bump in
  `WED_UndoMgr::Undo/Redo` was the second half of the WED-828 fix. Use the key for any new
  cache of archive-derived data.

### Registering a new persistent class (three places, or load/undo breaks)
1. `DECLARE_PERSISTENT(Class)` in the class body. Abstract intermediates use
   `DECLARE_INTERMEDIATE`. The macro makes the ctor/dtor protected.
2. `DEFINE_PERSISTENT(Class)` in the .cpp, plus a `CopyFrom(const Class*)` definition:
   `TRIVIAL_COPY(Class, Base)`, or a custom one that calls `Base::CopyFrom` first. The macro
   declares `CopyFrom` non-virtually and `Clone()` calls the most-derived one, so leaving it
   out gives a link error. Leaving *state* out of it produces a clone that silently lacks that
   state.
3. Add `_R(Class)` to `REGISTER_LIST` (or `REGISTER_LIST_ATC`) in `WEDCore/WED_AppMain.cpp`.
   Without it, `CreateByClass` returns NULL. Loading any file that contains the class then
   fails with "Create obj failed.", and **undoing the deletion** of such an object crashes
   (`DebugAssert` only). The factory table is in `WED_AppMain.cpp`, not `WED_Entity.cpp`.
- Other notes:
  - The class name string (`sClass = #Class`) is written into the XML `class=` attribute and
    stored in undo records. **Renaming a C++ class breaks every existing earth.wed.xml.**
  - The casting macros that the `WED_Persistent.h` comment lists (`IMPLEMENTS_INTERFACE`,
    `INHERITS_FROM`, `BASE_CASE`, `END_CASTING`) do not exist. Casting is `dynamic_cast`
    through `SAFE_CAST` (`IBase.h`), despite the "no RTTI" comment in `IBase.h`.
  - `CreateEntity<T>(archive, parent, pos, name)` in `WED_Persistent.h` is the short form of
    create + `SetParent` + `SetName`.

### Clone semantics
- `Clone()` allocates a **new ID** and calls `CopyFrom`. `WED_Thing::CopyFrom` deep-clones
  children, but **shares sources**: the clone becomes another viewer of the same source
  objects. It clears viewers, and copies only properties that are `can_edit && !synthetic`
  (through `SetNthProperty`, which records undo). The clone has no parent until the caller
  sets one. A command must be open.

## Architecture

### Ownership
`WED_Document` owns exactly one `WED_Archive` and one `WED_UndoMgr` (both are value members).
It passes itself to the archive as the `IResolver`. That is how entities reach the library,
resource and texture managers: `GetArchive()->GetResolver()`. `WED_Archive` owns every
`WED_Persistent` through an `id → pointer` map. The archive destructor sets `mDying` and
deletes everything without recording undo. Objects are heap-only: they are created through
`CreateTyped`, `Create(archive,id)` or `CreateByClass`, and destroyed with `Delete()`. The
destructor is protected.

### IDs
- `WED_Archive::NewID()` hands out `mID++`, and `AddObject` bumps `mID` past any explicit ID
  (from XML or undo). IDs are never reused within a session. `ClearAll()` does not reset
  `mID`.
- Object **1 is the root** by convention: `WED_Document::GetRoot()` and `Resolver_Find` start
  at `Fetch(1)`. A brand-new document creates `WED_Root` first, then `WED_Select`
  ("selection"), `WED_KeyObjects` ("choices") and `WED_Group` ("world") as its children, in
  `WED_Document::Revert`.
- `WED_Thing` keeps four ID lists:
  - `parent_id`;
  - ordered `child_id`;
  - ordered `source_id` ("I watch these", e.g. an edge's nodes);
  - `viewer_id` ("these watch me").

  Source and viewer lists are kept bidirectional by `AddSource`, `RemoveSource` and
  `ReplaceSource`. DEV builds run `WED_Archive::Validate()` (→ `WED_Thing::Validate`, which
  checks that every link is mirrored) after **every** `CommitCommand`.
- `Delete()` does not unlink anything. Callers must detach the object from its parent,
  children, sources and viewers, and clear it from the selection. `WED_RecursiveDelete`
  (in `WEDWindows/WED_GroupCommands.cpp`) is the correct way to delete. It also cascades to
  parents and viewers that `WED_NoLongerViable` flags.

### IResolver paths
`WED_Document::Resolver_Find` first recognises a few fixed names ("librarian", "texmgr",
"resmgr", "libmgr", "docprefs"). Anything else is walked from object 1 as `name.name[idx]`,
using `IDirectory::Directory_Find` (a linear scan of child names) and `IArray::Array_GetNth`.
`WED_KeyObjects` overrides `Directory_Find` so that "choices.airport" maps a key to an ID.
The `WED_Get*` helpers in `WEDMap/WED_ToolUtils.cpp` wrap these lookups. The names
"selection", "choices" and "world" on the root's children are therefore **load-bearing**.

### Undo machinery
- **Commit sequence.** `WED_Archive::__StartCommand` → `WED_UndoMgr::__StartCommand` creates a
  `WED_UndoLayer` and installs it with `WED_Archive::SetUndo`. `WED_Archive::CommitCommand`
  bumps `mOpCount` (the dirty counter). Then `WED_UndoMgr::CommitCommand` detaches the layer,
  throws it away if empty, otherwise purges redo, pushes the layer, and broadcasts
  `msg_ArchiveChanged` with the layer's change mask.
- **One record per object per command.**
  - `WED_UndoLayer` keeps one record per object ID for the whole command: `op_Created`
    (no buffer), `op_Changed` or `op_Destroyed` (with a `WriteTo` snapshot in a
    `WED_FastBufferGroup`).
  - Records coalesce: create+change = create; create+destroy = erase (the buffer space leaks
    until the layer dies, by design); destroy+recreate with the same ID = change, keeping the
    earliest data.
- **Undo and redo.** `Undo()`/`Redo()` execute a layer while a fresh layer records the
  inverse. They then adjust `mOpCount` ±1, bump `mCacheKey`, and broadcast
  `msg_ArchiveChanged`.
- **Abort.** `AbortCommand` executes the open layer with `SetUndo(UNDO_DISCARD)`, so the
  rollback itself records nothing. It does **not** broadcast `msg_ArchiveChanged`. It only
  bumps `mCacheKey`. Views that refresh only on that message may show stale state after an
  abort. On [the punch list](../bug-punch-list.md), low priority (Ben, 2026-09-25).
- **Undo depth.** The stack is capped at `MAX_UNDO_LEVELS` (100), and the oldest entry is
  trimmed at the next `StartCommand`. `GUI_MemoryHog::ReleaseMemory` can drop more under
  memory pressure, and it clears any mark set by `MarkUndo`.
- **Undo/Redo during an open command.** `Undo()`/`Redo()` do not check for an open command.
  What keeps the Undo menu or Cmd-Z out of a drag is the GUI layer: `GUI_Window::ClickDown`
  makes the app-root commander defer every command and key until after `MouseUp` has run (see
  [gui-framework.md](gui-framework.md#commands-and-keys-are-deferred-while-any-mouse-button-is-down)).
  Anything that calls `Undo()` without going through that dispatch has no protection.

### Change notification
- `msg_ArchiveChanged` (param = OR of `wed_Change_*` bits) is broadcast by the archive
  (`GUI_Broadcaster`) on commit, undo and redo. It is **not** sent for empty commands or
  aborts. Listeners: `WED_Map`, `WED_TCE`, `WED_PropertyTable`, `WED_DocumentWindow` (title
  dirty mark), `WED_MapPreviewPane`.
- `msg_ArchiveChangedEphemerally` is sent **synchronously from every
  `WED_UndoLayer::ObjectChanged`**, so it arrives once per `StateChanged` call during a live
  command (drags), and *before* the mutation is applied. Listeners must only schedule a
  redraw (`WED_MapPreviewPane` calls `Refresh()`), not read the model immediately. It is not
  sent for creates and destroys.
- Change bits:
  - `wed_Change_Any = -1` is the default `StateChanged()` argument and sets every bit;
  - `wed_Change_CreateDestroy = 1` is set by the layer itself;
  - `Selection = 2`, `Topology = 4` (parent, child, source and viewer edits) and
    `Properties = 8` are defined in `WED_Thing.h`.

  Pass the narrowest bit that is honest. Passing `wed_Change_Properties` when topology really
  changed leaves `WED_PropertyTable` holding a stale cache.

### Dirtiness
- Document dirtiness is `WED_Archive::mOpCount != 0 || WED_Document::mPrefsChanged`.
  `mOpCount` is zeroed by `SaveToXML` and at the end of XML load (`WED_Archive::PopHandler`).
  Undo decrements it and can make it negative. `WED_Archive::CommitCommand` increments it
  even for an **empty** command, so a no-op command marks the doc dirty but sends no message.
  **Selection is persistent and undoable**: selection changes go through commands
  ("Change Selection") and dirty the document.
- The per-object `mDirty`/`GetDirty`/`SetDirty` is a leftover from the SQLite era. It is
  round-tripped through undo buffers but nothing reads it.
- `WED_Document::Revert` and the scenery-import path in its constructor call
  `mUndo.__StartCommand`/`CommitCommand` **directly on the undo manager**, which bypasses
  `WED_Archive::CommitCommand` (no `mOpCount` bump, no DEV `Validate`). The constructor then
  purges undo. A menu Revert does not, so "Revert from Saved." stays on the undo stack.
  **Revert being undoable is intended (Ben, 2026-09-25)** — don't add an undo purge. Whether
  undoing it works cleanly is still untested. [Needs Runtime]

## Persistence: earth.wed.xml

- **Only XML.** SQLite persistence was removed entirely (the `wed_nuke_sqlite` merge). The
  only leftovers are the stale "via sqlite" line in the `WED_Persistent.h` comment and a
  `struct sqlite3;` forward declaration in `WED_EnumSystem.h`.
- **Save.** (Confirmed buggy by Ben, 2026-09-25 — on [the punch list](../bug-punch-list.md).) `WED_Document::Save` renames `earth.wed.xml` → `earth.wed.bak.xml` with plain C
  `rename()`, writes the new file, and renames the backup back if `ferror`/`fclose` report an
  error. Traps:
  - If `fopen` of the new file fails, `Save` alerts and returns **without** renaming the
    backup back. On POSIX the document then exists only as `earth.wed.bak.xml`.
  - `Obj/XDefs.h` wraps `fopen` for Windows UTF-8 but not `rename`, so non-ASCII paths may
    fail there. The MSVC CRT `rename` also fails when the destination exists, so on Windows
    the `.bak` may never refresh after the first save and the recovery rename can't restore
    the original. [Needs Runtime] on Windows.
  - The multi-stage `.bak.bak` scheme (which used `FILE_rename_file`, i.e. `MoveFileW` on
    Windows) is `#if 0`'d out.

  `WriteXML` writes
  `<doc><objects>…</objects><prefs>…</prefs></doc>`. `WED_Archive::SaveToXML` calls
  `ToXML` on every live object, in hash order, and `flush()`es after each one to keep memory
  down. `WED_XMLElement` requires attribute and element *name* strings to outlive the
  element (they are stored as `const char*`).
- **Load.** `WED_Document::Revert` pushes itself onto `WED_XMLReader` (expat, with a handler
  stack), and pushes the archive on `<objects>`. `WED_Archive::StartElement` reads `class` and
  `id`, calls `CreateByClass`, then `FromXML`, which pushes the object as the handler for its
  sub-elements. `WED_Thing::StartElement` handles `<child>`, `<source>` and `<viewer>`. Anything else
  goes to `WED_PropertyHelper::StartElement`, which offers each (element, attribute) pair to
  the property items through `WantsAttribute`/`WantsElement`. Extra non-property data goes out
  through `AddExtraXML` and comes back through an overridden `StartElement`
  (pattern: `WED_KeyObjects`).
- **Load failure.** If the main file fails to parse, the archive is cleared and
  `earth.wed.bak.xml` is tried. If the user then **cancels** "Open backup", the code runs a
  bare `throw;` inside the `try` block. No exception is being handled there, so by the
  language rules that is `std::terminate`, and the surrounding `catch(...)` that aborts the
  undo command never runs. Confirmed bug (2026-09-25) — on [the punch list](../bug-punch-list.md);
  the intent was presumably a thrown "user cancelled" exception.
  After load, `WED_Repair` deletes non-viable objects in a "Repair" command and purges undo.

### Format compatibility rules (non-obvious)
- There is **no per-object or per-file schema version.** Compatibility comes from these rules:
  - A missing attribute keeps the constructor default. Adding a property is backward
    compatible.
  - Unknown attributes and elements are **silently dropped**. An older WED opening a newer
    file loses the new data on save.
  - An unknown `class=` is a hard load failure.
  - Enums are stored as their **description string** (`ENUM_Desc`) and read back with
    `ENUM_LookupDesc`, so renumbering enums is safe but renaming a description string turns the
    value into `-1` in old files. Writing a `-1` enum later hands NULL to `add_attr_c_str`.
    That is only caught by `DebugAssert`, so a release build would construct `std::string`
    from NULL. [Needs Runtime]
  - Renaming a property's XML element or attribute name silently resets it to the default in
    every old file.
  - Several properties share one sub-element through `add_or_find_sub_element` (for example
    `<hierarchy name= locked= hidden=>`). A duplicated (element, attribute) pair is read into
    whichever item matches first.
- **The only version gate.** On every save, the `msg_DocWillSave` handler in
  `WED_DocumentWindow` writes the doc pref `doc/xml_compatibility` as a hard-coded `205`
  (100×major + minor = the minimum WED that reads the file correctly). On open, a warning is
  shown if that number is newer than the running WED. If you add XML content that an older
  WED would lose or choke on, **bump that constant**. **Policy (Ben, 2026-09-25): bump it on
  every format change** — any new persistent class, new persisted property / XML attribute or
  element, or new enum description. Treat this as a code-review item: a change that adds any of
  these without bumping `205` is a regression. Note the warning only appears if the load
  *succeeds*; an unknown class fails the load first ("Create obj failed.") with no version hint.
  `WED_Document::WriteGlobalPrefs`
  deliberately never copies this pref into global prefs, because WED 2.0–2.2 would otherwise
  apply it to pre-2.0 files.
- `WED_Thing::ToXML` always writes `<children>`, even when empty, because "wed-O-maker won't
  like that" (commit 323858723 rolled back that simplification). External tools parse
  earth.wed.xml, so keep the structure stable.
- The undo buffer format (`ReadFrom`/`WriteTo`) is never persisted, so it can change freely
  between versions. The XML format cannot.

## Connections to Other Systems

- **Design rules** ([wed-design-principles.md](wed-design-principles.md)): the stated rules for new code and reviews that this subsystem's conventions should follow — command ownership, pointer vs ID, class vs interface casts, error handling, layering, per-doc prefs, platform reference.
- [wed-entities.md](wed-entities.md): concrete `WED_Thing`/`WED_Entity` subclasses, GIS
  cache protocols, `WED_Select`, `WED_KeyObjects`, `WED_Airport` extra state.
- [wed-entities.md](wed-entities.md) also covers `WED_PropertyHelper` and the property items
  (the undo/XML plumbing for properties) and the enum system.
- [wed-core-services.md](wed-core-services.md): `WED_Document` as a service host (prefs,
  library/resource/texture managers), and the library scan that creates enums at runtime.
- [wed-map-and-tce.md](wed-map-and-tce.md): tools that open commands across click-drag
  gestures (`WED_HandleToolBase` "Change Selection", create tools) and use `CacheKey()`.
- [wed-ui-panes.md](wed-ui-panes.md): `WED_DocumentWindow` undo/redo menu handling,
  `WED_PropertyTable` cache invalidation on change masks, `WED_GroupCommands`
  (`WED_RecursiveDelete`, `WED_Repair`, `wrap_in_cmd` helpers).
- [wed-import-export.md](wed-import-export.md): import runs inside one command; Gateway
  export uses `MarkUndo`/`UndoToMark` to undo the upgrade heuristics.
- [wed-validation.md](wed-validation.md): validation selects offending objects inside a
  command ("Select Invalid").
- [wed-mcp.md](wed-mcp.md): the MCP server opens one command per mutating tool, only at a "safe point" (no command
  open, mouse up), and checks afterwards that none was left open; its JSON dump reads property items directly
  (persisted XML names) and must not use `SaveToXML`, which clears dirtiness.
- [gui-framework.md](gui-framework.md): `GUI_Broadcaster`/`GUI_Listener` (synchronous
  delivery over a copy of the listener set), `GUI_MemoryHog`.
- [utils-platform-and-files.md](utils-platform-and-files.md): which file calls are UTF-8-safe
  on Windows (`fopen` is wrapped, the `rename()` used by `Save` is not), and why asserts that
  fire during load are crash-with-dialog rather than recoverable.
- [wed-network-and-filecache.md](wed-network-and-filecache.md): `WED_NWLinkAdapter`, which
  would receive every create, change and destroy. It is compiled out (`WITHNWLINK 0` in
  `Obj/XDefs.h`).
