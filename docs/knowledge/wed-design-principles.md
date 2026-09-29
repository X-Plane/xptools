# WED Design Principles

> Source: cross-cutting (all `src/WED*`, `GUI/`) · Raw source: [raw/ben-design-principles-2026-09-25.md](raw/ben-design-principles-2026-09-25.md)

These are the rules for **new code and reviews**, as stated by Ben (2026-09-25). Existing code
does not always follow them; known violations are listed under each rule. When the code and a
rule disagree, the rule describes the intent — flag the code, don't copy it.

## 1. The outermost user-facing command opens the undo command

- The top-level handler for a user action (menu command, tool gesture, dialog OK — e.g. the
  `WED_Do*` functions in `WEDWindows/WED_GroupCommands.cpp` such as `WED_DoGroup`,
  `WED_DoUngroup`, `WED_DoMakeNewAirport`) calls `StartCommand` and
  `CommitCommand`/`AbortCommand`. Everything it calls runs inside that one grouping.
- **Utilities assume a command is already open** and never open their own. That's what makes
  them composable: commands can't nest (a nested `StartCommand` asserts — see
  [wed-object-model.md](wed-object-model.md#things-that-will-bite-you)).
- If a function both needs to be a top-level action *and* reusable, split it: a `WED_Do*`
  wrapper that opens the command, and a helper that assumes one.
- Known deviation: `DoHueristicAnalysisAndAutoUpgrade` (`WED_SceneryPackExport.cpp`) opens its
  own command internally.

## 2. When raw pointers to WED objects are OK

Hold **object IDs** (and re-`Fetch`) across anything that can undo, redo or revert. Raw pointers
are acceptable only for:

- **Caches with correct invalidation semantics**, for performance (e.g. `WED_PropertyTable`,
  which drops its cache on `wed_Change_CreateDestroy | wed_Change_Topology`).
- **References to things that aren't `WED_Thing`s** — art/resources, textures, library data.
  (Their own lifetime contract is rule 7.)
- **Local references where undo cannot happen** — e.g. inside a helper that must run inside an
  open command because it mutates state (rule 1).

Anything else holding a `WED_Thing*` across a command boundary is a bug (e.g. the pack-export
`problem_children` on [the punch list](../bug-punch-list.md)).

## 3. Concrete classes vs. GIS interfaces

- **Exact class match + `static_cast`** (`GetClass() == WED_Runway::sClass`) is the right,
  fast choice when the operation genuinely needs a specific final class ("we need a runway").
  Validation does this deliberately ([wed-validation.md](wed-validation.md)).
- **GIS interfaces** (`IGISEntity`, `IGISPolygon`, … in `Interfaces/IGIS.h`) when the operation
  applies to a broad geometric category.
- **Code smell:** a function that lists a large set of exact classes probably wants an interface.
- Expect **structure drawing to be abstract** (GIS interfaces) and **preview drawing to be
  concrete** (per class) — see [wed-map-and-tce.md](wed-map-and-tce.md#layers-caps-and-draw-order).
- **Never cast to a concrete intermediate implementation** of a GIS interface
  (`WED_GISPoint`, `WED_GISChain`, `WED_GISPolygon`, `WED_GISEdge`, …). Cast to the interface or to
  the final class.
- Known violations (2026-09-25 grep for `dynamic_cast`/`static_cast` to `WED_GIS*`):
  concentrated in `WEDWindows/WED_GroupCommands.cpp` and `WED_ConvertCommands.cpp`; single
  instances in `WED_DSFImport.cpp`, `WED_GISEdge.cpp`, `WED_ValidateATCRunwayChecks.h`,
  `WED_Orthophoto.cpp`. Don't add more.

## 4. Mutate-then-roll-back exports are fine — if the roll-back is last and robust

- Exports may temporarily modify the document and undo afterwards (Gateway pack export's
  `MarkUndo` → heuristics → export → `UndoToMark`; ortho export's aborted "Norm Ortho" operation).
- **The roll-back must be the last step.** It invalidates pointers (rule 2), so nothing may use
  `WED_Thing*`s collected during the export afterwards — collect IDs instead.
- **The roll-back must be completely robust. Undo bugs are data-loss bugs.** Every mutation made
  during the export must be undo-captured (e.g. `StateChanged()` before non-property edits — see
  the metadata-key bug on [the punch list](../bug-punch-list.md) and
  [wed-entities.md](wed-entities.md#writing-value-directly-bypasses-undo)).

## 5. Errors: return values for APIs, asserts only for programmer errors

- **APIs should return errors rather than throw.** Return-channel errors are type-safe;
  exceptions are not. (Existing code throws in places — `WED_ThrowPrintf`, the document load
  path, the DEV assert handler — don't extend that pattern to new APIs.)
- **`Assert` / `DebugAssert` are only for invariant violations**: they should fire only when a
  programmer should (1) see the condition and (2) change WED as a result. That covers
  precondition violations, postcondition failures and broken invariants — **not** bad user
  data, missing files, network failures or malformed input, which are reported to the user
  (alert or validation record) or returned as errors.
- User-facing alerts must not be shown inside an open command
  ([wed-object-model.md](wed-object-model.md#things-that-will-bite-you)).
- What each assert macro compiles to per build: [utils-platform-and-files.md](utils-platform-and-files.md#asserts-what-they-compile-to).

## 6. Blocking the main thread: allowed, but prefer async for I/O

- WED is not X-Plane: blocking the main thread is **allowed but not great** — it is not a
  hard rule.
- For **long-running, I/O-bound work** (network downloads especially), prefer asynchronous
  code **with cancelability** — the `WED_FileCache` / `curl_http_get_file` worker-thread model
  ([wed-network-and-filecache.md](wed-network-and-filecache.md)).
- The existing main-thread waits (CIFP, invisible metadata update, ICAO prefetch) are on
  [the punch list](../bug-punch-list.md).

## 7. Resource handles live until the next library-changed message

- Pointers handed out by `WED_ResourceMgr` / `WED_TexMgr` (`fac_info_t*`, `pol_info_t*`, …) are
  **valid until a library-changed message goes out** (`msg_SystemFolderChanged` /
  `msg_SystemFolderUpdated`) — invalidation is the rare case, so callers may cache them between
  those messages but must drop them on the message.
- This is the contract; today nothing sends the purge (and `GetFac` can invalidate early via
  vector reallocation) — both on [the punch list](../bug-punch-list.md). See
  [wed-core-services.md](wed-core-services.md#wed_resourcemgr-never-gets-its-invalidation-messages).

## 8. WED is strictly layered on top of GUI

- `GUI/` is a toolkit. **GUI code that does WED-specific things is always a layering
  violation**, and a patch that hacks GUI to use WED internals is an architecture violation —
  reject it in review.
- WED-specific widgets (things built on GUI that know about WED) belong on the WED side of the
  line. That boundary is messy today.
- Known violations: `GUI/GUI_TextTable.cpp` includes `WED_UIDefs.h`, `WED_ToolUtils.h`,
  `WED_Sign_Editor.h`, `WED_Line_Selector.h`, `WED_Road_Selector.h`; `GUI_FilterBar.cpp` and
  `GUI_FormWindow.cpp` include `WED_Colors.h` (the latter with a comment admitting it). See
  [gui-framework.md](gui-framework.md).

## 9. Per-document prefs are authoritative; globals only seed new documents

- Document preferences — including the **export target** — belong to the document. A global
  copy (`gExportTarget`, and the same pattern for basically every per-doc pref) exists only so a
  **new** document starts out like the last one you saw.
- So with two documents open, each must use its own target. Known violation: `gExportTarget`
  is read from the document only on open (`WED_DocumentWindow` constructor and `msg_DocLoaded`),
  not on window activation, and every save writes the global back into that document's
  `doc/export_target` — on [the punch list](../bug-punch-list.md). Code that temporarily switches
  `gExportTarget` (Gateway export/import, scenery import) must restore it.
  See [wed-import-export.md](wed-import-export.md#the-export-target-is-a-global-it-is-sticky-and-some-code-switches-it-for-you).

## 10. Cross-platform behavior: Mac + Windows agreement is authoritative

- When platforms behave differently, **Linux is never the reference.**
- If **Mac and Windows agree**, that's the correct behavior; make Linux match.
- If **Mac and Windows disagree**, a human decides — flag it, don't pick one.
- Platform differences are catalogued in [gui-framework.md](gui-framework.md#platform-matrix-quick-reference).
