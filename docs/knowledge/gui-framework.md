# GUI Framework (GUI / UI / OGLE)

> Source: `src/GUI/`, `src/UI/` (XWin platform layer), `src/OGLE/` (text-edit engine)

WED's widget toolkit: a pane tree drawn with immediate-mode OpenGL on top of a thin
per-platform window layer (`XWin` / `XWinGL`: Cocoa on macOS, Win32 on Windows, FLTK on
Linux). Only WED links the `GUI_*` classes. Everything else that shows up in `cmake/*.cmake`
takes a small slice (see [Who Uses What](#who-uses-what)).

## Things That Will Bite You

### Three trees, only one owns anything

- **Pane tree** (`GUI_Pane::mParent/mChildren`). This one owns. `~GUI_Pane` detaches from the
  parent and deletes every child. It first sets each child's `mParent = NULL`, so a child's
  destructor can't `Refresh()` or walk up any more. Content/geometry/provider "behavior"
  objects plugged into panes (`GUI_TableGeometry`, `GUI_TextTableProvider`,
  `GUI_ScrollerPaneContent`, …) are **never** deleted by the pane.
- **Commander tree** (`GUI_Commander::mCmdParent/mCmdChildren`). This is keyboard and command
  focus, and it is a **separate tree from the pane tree**. The predecessor doc called it
  "routing along the pane chain", which is wrong. It owns nothing. `~GUI_Commander` just
  NULLs its children's `mCmdParent`, so they become orphan roots.
- **Window list** (`static set<GUI_Window*> sWindows` in `GUI_Window.cpp`, plus a separate
  `HWND→XWin*` map in `XWin.win.cpp`).

A `GUI_Window` is the root of its pane tree. Its commander parent is normally
`gApplication`, so the **app is the commander root for every window**. The exception is
`WED_MapPreviewWindow`, which passes `nullptr` and is its own root: commands and defers inside
it never reach the app.

### Mouse capture and the single-gesture rule

- `GUI_Window::ClickDown` stores whichever pane's `MouseDown` returned non-zero in
  `mMouseFocusPane[button]`. Drag, up, and timer-driven drags then go **only to that raw
  pointer**, wherever the mouse is. If that pane is deleted mid-gesture, the next drag or up
  dereferences freed memory. Nothing clears it except `ClickUp`.
- `ClickUp` with no focus pane silently eats the up-click. This is on purpose: Win32 DnD and
  popup menus swallow real ups, so synthetic ones get posted later.
- Only one button gesture at a time is enforced **below** GUI, in each `XWin` backend
  (`mInDrag` on Mac, `mDragging` on Win/Lin). A second button pressed during a gesture is
  dropped. GUI relies on this: `ClickDown` calls `GetRootForCommander()->BeginDefer()`, and
  `BeginDefer` has `DebugAssert(mDeferLevel < 1)`. The root is the app, shared by all
  windows. If you bypass the XWin rule, you trip the assert.
- `ClickUp` calls `SetTimerInterval(0.0)`, which kills the window's single `XWin` timer on
  every mouse-up. Don't use `XWin::SetTimerInterval` for your own purposes in a `GUI_Window`.
  Use a `GUI_Timer`.

### Commands and keys are deferred while any mouse button is down

Between `ClickDown` and `ClickUp`, the app-root commander has `mDeferLevel > 0`:
- `DispatchHandleCommand` queues the command and **returns 0** ("not handled"). The same goes
  for `DispatchKeyPress`, for every commander in the chain whose `ShouldDeferKeypress()` is
  true (the default). `WED_MapPreviewPane` and `WED_MapPreviewWindow` override it to false, so
  WASD works while you mouse-look.
- `EndDefer` replays the queue through `DispatchHandleCommand`/`DispatchKeyPress` on the root.
  The **focus at replay time** decides the target, not the focus when the key was pressed.
  `GUI_Window::ClickUp` calls the pane's `MouseUp` **before** `EndDefer`, so a replayed command
  runs after the tool has already committed its gesture.
- Why: a keyboard shortcut (e.g. delete, undo) fired in the middle of a map drag would run a
  second undoable operation inside the drag's operation. `GUI_Commander_Notifiable`
  (`RegisterNotifiable`) is only a backstop: `DispatchHandleCommand` checks the defer level
  *before* calling the notifiables, so `WED_HandleToolBase::PreCommandNotification` sees only
  commands that bypass the defer. See [wed-map-and-tce.md](wed-map-and-tce.md).
- A key handled by a non-deferring commander that nobody consumes is still queued at the end
  of `DispatchKeyPress`. Non-deferring handlers can therefore see the same key twice.

### Handlers can re-enter the event loop, and timers fire inside them

Each of these calls runs a **nested platform event loop** before it returns:
- `IsDragClick`:
  - Mac: `run_event_tracking_until_move_or_up`.
  - Win: `DragDetect`.
  - Linux: a hand-rolled `Fl::wait()` loop under `mBlockEvents`.
- `PopupMenu` / `PopupMenuDynamic` (`TrackPopupCommands`).
- `DoDragAndDrop`:
  - Mac: a modal `nextEventMatchingMask` loop.
  - Win: `DoDragDrop`.
  - Linux: `Fl::dnd()`.
- Mac modal windows: `runModalForWindow`, started with `dispatch_async` after the constructor
  returns.

Mac `GUI_Timer`s are added in `kCFRunLoopCommonModes`, Win `GUI_Timer`s are thread
`SetTimer(NULL,…)`, and FLTK timeouts run in any `Fl::wait()`. So **timer callbacks, including
the `GUI_Destroyable` reaper, can run inside your `MouseDown` while it waits in
`IsDragClick`**. "Async" destruction means "later on the main thread", not "after the current
handler returns". [Needs Runtime] to confirm the reaper actually fires inside each of these
loops on each platform.

Two more timer traps, both hit while building the MCP server ([wed-mcp.md](wed-mcp.md)):
- **Don't delete a `GUI_Timer` inside its own `TimerFired`.** On Linux, `GUI_Timer::timeout_cb` reads `me->mTimer`
  afterwards to re-arm, so that is a use-after-free.
- **Quitting from a timer callback on Mac:** `[NSApp stop:]` takes effect only after the run loop processes an
  event. `stop_app()` now posts an app-defined event so `GUI_Application::Quit` works from a timer. Win and Linux
  check `mDone` after every message or wait anyway.

These loops eat mouse-ups, so every backend has a **fake-up protocol**:
- `TrackPopupCommands` or the DnD code sets `mWantFakeUp`, and the XWin layer then issues
  `ClickUp` itself.
- On Windows, `GUI_Window::IsDragClick` and `DoDragAndDrop` `PostMessage` a synthetic
  `WM_LBUTTONUP`/`WM_RBUTTONUP`.
- On Linux the nested loop can deliver `FL_RELEASE` **re-entrantly** inside `ClickDown`. The
  `FL_PUSH` handler in `XWin.lin.cpp` has to check that `mDragging` is still set before it
  fakes an up, or `EndDefer` underflows. This was fixed twice in April 2026 (`fd30ca555`,
  `87cd91818`). Treat that block as fragile.
- Mac `mInDragOp` is tri-state (0/1/2). This fixed WED-566, where a mouse-down was re-dispatched
  inside a mouse-down after a drag session ended.

### Closing a window differs per platform

`XWin::Closed()` is called when the user clicks the close box. What "return true" does
depends on the platform:

| Platform | `Closed()` returns true → |
|---|---|
| Windows | `DefWindowProc` → `WM_CLOSE` → `GUI_Window::SubclassFunc` does **`delete me` synchronously** inside the message handler |
| macOS | `-[XWinCocoa close]` is overridden, doesn't call `super`, and deletes `mOwner` *after* NULLing it (a no-op). The C++ object is **not** deleted, and the native close appears not to happen. [Needs Runtime] |
| Linux | `window_cb` → `hide()` only |

The portable convention in WED: override `Closed()`, destroy yourself (`AsyncDestroy()`, or
`Hide()` for the reusable `WED_Settings`), and **return false**. `WED_DocumentWindow::Closed`
calls `TryClose()` and returns false. `WED_AptImportDialog::Closed` does `AsyncDestroy()` and
returns true. That's still safe on Windows because `~GUI_Destroyable` removes itself from the
dead list. [Needs Runtime]: `WED_GatewayImport` and `WED_ValidateList` don't override
`Closed()`. Check what their close box does on Mac.

### `GUI_Destroyable` batch hazard

`GUI_DestroyableTask::TimerFired` swaps the dead list into a local set and deletes each entry.
`~GUI_Destroyable` erases itself from the *global* list only, not from that local copy. If
deleting A also deletes B (e.g. B is a child pane of A), and **both** were `AsyncDestroy`ed
in the same batch, B is deleted twice. Only `AsyncDestroy` the owner. Objects that get
`AsyncDestroy`ed from inside another's destructor land in the next batch. That's the "doc kills
import dialog" case in the comment, and it's handled.

### Broadcaster/Listener

- `BroadcastMessage` iterates a **copy** of the listener set and re-checks membership. A
  listener may remove or delete itself or other listeners during a broadcast (`b11a2db92`).
  But deleting the **broadcaster** from inside a `ReceiveMessage` is still use-after-free.
  The membership check reads `this->mListeners`.
- Listener order is `set<GUI_Listener*>` order, which is heap-address order. It's effectively
  random. Never rely on one listener being notified before another.
- Message IDs are bare `intptr_t`. GUI's own messages are in `GUI_Messages.h`, and apps start at
  `GUI_APP_MESSAGES` (2000). WED also uses addresses as IDs, e.g. `(intptr_t)&gFontSize` in
  `WED_Application.cpp`.

### Focus side effects

- `~GUI_Commander` calls `LoseFocus(1)`. That sets the parent's `mCmdFocus = NULL`
  **whether or not this commander was the one focused**, and keeps walking up while parents
  refuse focus. Destroying an unfocused child can therefore steal focus from its focused
  sibling.
- The `GUI_Window` constructor calls `FocusChain(1)`. Its own comment says "BEN SEZ: this is
  probably a bad idea". Any window you create, even hidden, becomes the app's key and command
  target until another window's `Activate` re-chains focus. `Activate(0)` does nothing, so focus
  stays with the last active window.
- `FocusChain` moves focus **without** calling `AcceptLoseFocus` on the old holder.
  `GUI_TextField` starts its caret-blink timer in `AcceptTakeFocus` and stops it in
  `AcceptLoseFocus`, so it can keep blinking after focus has left. That's why
  `WED_Settings::Closed` calls `TakeFocus()` first. `AcceptTakeFocus` is also a *query* that
  `LoseFocus` calls on each parent in turn. Side effects in it run more often than you'd think.

### Table editing lifetime (`GUI_Table` / `GUI_TextTable`)

- When a text edit begins, `GUI_TextTable::CreateEdit` creates a `GUI_TextField` as a child of
  the table and calls `mParent->TrapFocus()`. That adds the table to the **window root's
  `mTrap` set** (raw pointers). On every later mouse-down, the root first calls
  `TrapNotify` on trapped panes, **before** normal dispatch. `GUI_Table::TrapNotify` commits the
  edit (`KillEditing(true)` → `AcceptEdit` → an undoable model change) if the click is outside
  the table. So clicking elsewhere commits the edit before the clicked widget sees the click.
  Nothing removes a pane from `mTrap` except its own `TrapNotify` returning 0. Deleting a table
  while it is trapped leaves a dangling pointer for the next click. (On [the punch list](../bug-punch-list.md).)
- `GUI_TABLE_CONTENT_RESIZED` / `GUI_TABLE_SHAPE_RESIZED` received by `GUI_Table` call
  `KillEditing(false)`, which **discards** the in-progress edit. `ScrollH`/`ScrollV`, which
  `GUI_Table::SetBounds` also calls on every resize, call `KillEditing(true)`, which **commits**
  it. `TerminateEdit` NULLs `mTextField`/`mEditor` *before* calling `AcceptEdit`, because the
  edit's own model change can broadcast `CONTENT_RESIZED` and re-enter `KillEditing`. Keep that
  ordering if you touch it.
- `GUI_TABLE_CONTENT_RESIZED` must be sent only **after** the geometry already reports the new
  row count (`GUI_Messages.h`).
- `GUI_Table::SetBounds` → `ScrollV` → `mGeometry->GetCellTop(GetRowCount()-1)`. Call
  `SetGeometry` before the first `SetBounds`, and a geometry must return 0 from
  `GetCellTop(-1)` for empty tables. `GUI_SimpleTableGeometry` happens to.
- Row 0 is the **bottom** row, because Y is up. Providers flip the index, e.g.
  `WED_PropertyTable::FetchNth` uses `size - row - 1`.
- Double-click detection in `GUI_TextTable::CellMouseDown` uses **function-static** last-click
  state shared by every text table, with a 0.1 s threshold on `GetTimeNow()` (next item).

### `GUI_Pane::GetTimeNow` is CPU time on Mac/Windows

It returns `clock()/CLOCKS_PER_SEC` except on Linux, where WED-1405 (`f824e294e`) switched to
`CLOCK_MONOTONIC`. On macOS/POSIX, `clock()` is process CPU time, so intervals measured while
idle come out short. It also returns a `float`, which loses precision after long uptimes. It's
used for text-table double-click and `WED_CreateToolBase` click timing, so the double-click
behavior differs by platform. The Mac/Win `clock()` is a bug (Ben, 2026-09-25) — should be a
monotonic wall clock everywhere; on [the punch list](../bug-punch-list.md).

### Layout traps

- **Sticky edges are fractions, not flags.** In `GUI_Pane::ParentResized`, each edge moves
  by `sticky·Δ(own side) + (1−sticky)·Δ(opposite side)`. 1 means "follow my side", 0 means
  "follow the opposite side", and 0.5 means "follow the center". WED uses 0.5 for centered
  buttons and half-height panes. The in-code comment "We no-op on NO bits" is **false**: sticky
  `(0,*,0,*)` swaps which parent edges the left and right follow, and the pane collapses on
  resize. Results are truncated to `int` on every resize, so fractional stickies can drift by a
  pixel over many resizes.
- `GUI_Packer` is one-shot. `PackPane` carves from `mPackArea` using the child's current size
  along the packing axis, and sets no stickies. Set sticky flags yourself. Any
  `GUI_Packer::SetBounds` resets `mPackArea` to the full bounds, so packing after a resize
  starts over.
- `GUI_Splitter` uses only its first two children. The gap between them is the handle, and its
  width comes from the **image resource size** (`splitter_h.png` for vertical splits,
  `splitter_v.png` for horizontal, divided by 2). `GUI_Splitter::MouseDown` returns 1 for
  **any** click that reaches it. A child that declines a click (returns 0) starts a splitter
  drag.
- `GUI_Splitter::GetSplitPoint` isn't symmetric. Horizontal returns child 0's right edge,
  vertical returns child 1's bottom (split + gap). `AlignContentsAt(split)` treats the value as
  child 0's edge. `WED_DocumentWindow` saves the vertical `prop_split`/`prev_split` with
  `GetSplitPoint` and restores with `AlignContentsAt`, so those splitters should creep by the
  gap width on each save/reopen. On [the punch list](../bug-punch-list.md), high priority (Ben, 2026-09-25).
- Hit-testing is inclusive on all four edges, so abutting siblings overlap by one pixel. Mouse,
  wheel, cursor, and tooltip dispatch walk children **last-to-first**: the last child added is
  on top, matching draw order. `FindByPoint`, used for drag-enter/over/drop, walks
  **first-to-last**. For overlapping siblings, drops go to the bottom one.

### OpenGL state is barely managed

- `GUI_GraphState` is **not** a state cache. Every call hits GL. `GUI_Window::GLDraw` sets
  the viewport and ortho projection (origin bottom-left, window pixels), clears, calls
  `mState.Reset()` **once per frame**, enables `GL_SCISSOR_TEST`, and draws the tree. A pane
  that changes blend, depth, matrices, viewport, or line width and doesn't restore them leaks
  that state into every pane drawn after it. `InternalDraw` re-sets `glScissor` for each pane
  but never re-enables the scissor test.
- `GUI_GraphState::Init` enables `GL_CULL_FACE` with `glFrontFace(GL_CW)`, and `Reset()`
  doesn't touch culling. In y-up GUI coordinates, **quads must wind clockwise or they vanish**.
  Several WED preview paths wrap their drawing in `glDisable/glEnable(GL_CULL_FACE)`.
- Fixed-function, compatibility-profile GL throughout (`glBegin`, `GL_LIGHTING`, `GL_FOG`,
  `glActiveTexture` for 4 units).
- One context per window, with resources shared:
  - Mac: `NSOpenGLContext shareContext:`.
  - Win: `wglShareLists`.
  - The share source is `*sWindows.begin()`, the **lowest-addressed** existing window, not the
    oldest.
  - On **Linux**, `glWidget::draw` reuses the first window's context object itself (not a share
    group), and `GUI_GraphState::Init` re-runs whenever `mCtxValid` is false. This is safe in
    practice because the first window is the `WED_StartWindow`, which is never closed (Ben,
    2026-09-25) — don't add code that destroys it.
  The font atlas textures and the `GUI_GetTextureResource` cache (a global name→texture map,
  never freed) depend on this sharing.
- Font drawing (`GUI_FontDrawScaled`) calls `GUI_MeasureRange` first. That forces every glyph
  into the atlas so `sync_tex` can upload once **before** `glBegin(GL_QUADS)`. You can't upload
  a texture between `glBegin` and `glEnd`. New text-drawing code must follow the same
  measure → sync → draw order. `draw_char` returns the *unscaled* advance, so scaled text
  (`GUI_FontDrawScaled` with a box taller or shorter than the line height) spaces glyphs
  incorrectly.

### The `&*str.end()` pattern is UB and crashes MSVC debug builds

Many GUI text APIs take `(const char* begin, const char* end)`. The old idiom
`&*s.begin(), &*s.end()` dereferences an end iterator. `1a90750cd` replaced it with
`s.data(), s.data()+s.size()`. A follow-up added a `std::string::const_iterator` overload of
`GUI_FitForward` (`8164f0c44`), which also stepped **bytes rather than UTF-8 code points**.
`9a74e4be3` removed that overload again. Use `data()`/`data()+size()` and keep all text
measurement UTF-8-aware (`UTF8_next`, `UTF8_decode` from `GUI_Unicode.h`).

### Platform input differences

- Mac `GUI_Window::KeyPressed` maps **Command → `gui_ControlFlag`**.
  `GUI_Pane::GetModifiersNow` reports Control **or** Command as `gui_ControlFlag`.
  Control-click on Mac arrives as button 1 (right). Mac `GetModifiersNow` reads
  `[NSEvent modifierFlags]`, which is live hardware state, not the event being handled
  (contrary to the `GUI_Pane.h` comment).
- Linux keyboard mapping in `KeyPressed` covers only letters, digits, arrows, and a handful of
  navigation keys. Other virtual keys come through as 0.
- Mac DnD `DoDragAndDrop` always returns `gui_Drag_None`, while Windows returns the real
  effect. Never make the drag *source* act on the result. WED's own selection drag carries a
  dummy pointer, and the drop target does the move itself (`WED_DoDragSelection` in
  `WED_ToolUtils.cpp`).
- Linux DnD and clipboard are partial stubs. `GUI_DragData_Adapter::NthItemHasClipType`
  always returns true and `GetNthItemData` returns false, so any external drag looks like a WED
  selection drag. The clipboard handles plain text only, via a `Fl::paste` +
  `Fl::wait()`-until-`FL_KEYUP` loop.

### Menus

- Windows copies the first window's `HMENU` into each new non-modal, non-popup window when that
  window is created. It builds the accelerator table once, in `Run()` (`BuildAccels`). The class
  comment in `GUI_Application.h` warns that **menus must be complete before `Run()`**. Items
  added later never reach existing windows.
- Menu enable state and dispatch take different paths:
  - Enable state is computed from the **app root's** focus chain: Mac `MenuUpdateCB`, Win
    `EnableMenusWin`, Linux `update_menu_recursive`.
  - Picks are dispatched from the **receiving window**: Mac first-responder
    `GotCommandHack` (see the "MASSIVE HACK ALERT" in `XWin.mac.mm`; it's what makes menus
    work inside modal loops), Win `WM_COMMAND`, Linux `menu_cb`.
  - Because of the `FocusChain(1)` quirk above, these two paths can disagree.
- The Windows `GUI_Window` constructor subtracts 20 px from the first window's height to
  approximate the menu bar, which doesn't exist yet at that point.

### Layer violation

`GUI_TextTable.cpp` includes `WED_Sign_Editor.h`, `WED_Road_Selector.h`, `WED_Line_Selector.h`,
`WED_UIDefs.h`, and `WED_ToolUtils.h`. `GUI_FormWindow.cpp` and `GUI_FilterBar.cpp` include
`WED_Colors.h`. So `src/GUI` can't be built without WED. Popup cell editors (`gui_Cell_TaxiText`,
`gui_Cell_RoadType`, `gui_Cell_LineEnumSet`) are WED classes hosted in a full-window
`GUI_MouseCatcher` pane, and a click outside the editor broadcasts `GUI_MOUSE_OUTSIDE_BOUNDS`.

## Architecture

### Layers

```
WED_* windows/panes
  GUI_Window  : XWinGL, GUI_Pane, GUI_Commander   (src/GUI)
    XWinGL    : XWin                              (src/UI, per-platform .mm / .win.cpp / .lin.cpp)
      XWin    : NSWindow wrapper | HWND | Fl_Window (LIN: XWin *is* an Fl_Window)
GUI_Application : GUI_Commander   — owns the run loop, menus, gApplication singleton
```

- `XWin` turns native events into virtual callbacks: `ClickDown/Up/Drag/Move`, `MouseWheel`,
  `KeyPressed`, `Timer`, `Closed`, `Activate`, `Resized`. Mouse coordinates arrive
  client-relative with y down.
- `GUI_Window` flips them to GL coordinates (`Client2OGL_Y`: origin bottom-left) and hands them
  to the pane tree (`InternalMouseDown` etc., which are `GUI_Pane` privates that only
  `GUI_Window` can call).
- Keys go the other way. They never touch the pane tree: `KeyPressed` → `DispatchKeyPress` on
  the window's commander chain, starting at the deepest focused commander.
- Commands work the same way: menu pick → `DispatchHandleCommand` → focused commander →
  parents → app (`gui_Quit` etc. in `GUI_Menus.h`; apps start at `GUI_APP_MENUS`).
- `GUI_Window` also answers the parent-delegated pane services: `Refresh` → `ForceRefresh`
  (always asynchronous), `PopupMenu*`, `IsDragClick`, `DoDragAndDrop`, `GetMouseLocNow`,
  `IsKeyPressedNow`.

### Refresh and drawing

`Refresh()` walks up to the window and calls `XWin::ForceRefresh`. You can never draw
synchronously. On Linux, `ForceRefresh` goes through a one-shot FLTK idle callback, because
FLTK ignores `redraw()` calls made during a draw and FLTK < 1.3.6 drops timeouts added during
`Fl::flush` (see the comment in `XWin.lin.cpp`). Mac drag autoscroll is the one place that
draws synchronously: `GUI_Window::Timer` → `UpdateNow`, because the Mac drag manager blocks
update events.

### Modality is "modeless code flow"

The `xwin_style_modal` constructor returns immediately. Windows disables every other `HWND`,
Linux calls `set_modal()`, and Mac `dispatch_async`s a `runModalForWindow`, which `~XWin` stops.
WED's import/export dialogs are written as callbacks, not as linear "wait for answer" code.

### Text

- `GUI_Fonts`: FreeType rasterizes `sans.ttf` (a GUI resource) at 72 dpi into one growable
  1024-wide alpha atlas per font id (`font_UI_Basic`, `font_UI_Small`), with a gamma tweak
  (`FONT_GAMMA`).
- `GUI_SetFontSizes` rebuilds both fonts and deletes their textures, so a GL context must be
  current. WED calls it from the FontSize pref (clamped 10–18) in `WED_Document` /
  `WED_Application`.
- `GUI_TextField` is a pane plus a commander plus a `GUI_Timer` (caret). It wraps **OGLE**
  (`src/OGLE/ogle.cpp`), a C text-editing engine that works through callbacks. OGLE's header
  comment keeps a numbered list of known bugs (repagination on cut/paste, scroll-reveal
  off-by-one, …). `4ce9ddaa6` fixed part of it. Expect multi-line fields to misbehave.
- All strings are UTF-8, and the key path is UTF-32 (`GUI_Unicode`).

### Tables as a framework

- `GUI_Table` (the scrollable grid pane) gets its brains from two plug-ins:
  - `GUI_TableGeometry` for rows, columns, and extents, in a zero-origin y-up space.
  - `GUI_TableContent` for draw and mouse per cell, with cell bounds passed in window
    coordinates.
- `GUI_Header` and `GUI_Side` follow the table's scroll by listening to it.
- `GUI_TextTable` is a `GUI_TableContent`, **not** a pane. It is also a commander and a
  broadcaster, and it asks a `GUI_TextTableProvider` for `GUI_CellContent` structs. Cell types
  in `GUI_CellContentType` include WED-specific ones (`gui_Cell_TaxiText`,
  `gui_Cell_RoadType`, `gui_Cell_LineEnumSet`).
- WED's property and hierarchy views implement the provider in `WED_PropertyTable`; see
  [wed-ui-panes.md](wed-ui-panes.md). The broadcast messages (`GUI_TABLE_*`,
  `GUI_SCROLL_CONTENT_SIZE_CHANGED`) are how content, table, scroller, header, and side stay in
  sync. The table is not a "two-column" widget, whatever the old doc said.

### Resources, prefs, timers

- **Resources** (`GUI_Resources`): Mac reads them from the bundle via `CFBundle`, Windows from
  `GUI_RES` resources in `WED.rc`, and Linux from `_binary_<name>_<ext>` symbols linked in by
  `objcopy` (`embed_resource` in `cmake/WED.cmake`) and looked up with `dlopen(0)`/`dlsym`.
  `GUI_GetTextureResource` caches GL textures by name forever. `GUI_GetImageResource`
  decodes a fresh `ImageInfo` on every call, and the caller owns it.
- **Prefs** (`GUI_Prefs`): a global section→key→value map. It's read from and written to
  `<app>.prefs` in `GUI_GetPrefsDir()`. On Linux the file is `.<app>.prefs` in the home
  directory.
- **Timers** (`GUI_Timer`): repeating timers. The implementation is a CFRunLoopTimer on Mac,
  a thread `SetTimer` on Windows (looked up through the static `sTimerMap`), and
  `Fl::add_timeout`/`repeat_timeout` on Linux.
  - On Mac, `Start(s)` passes `s` as the CF *absolute* fire date, so the first fire happens
    immediately. (On [the punch list](../bug-punch-list.md).)
  - Also on Mac, `Start(0)` has a zero interval, which CF treats as one-shot. Windows and Linux
    treat a zero interval as repeating as fast as possible.
  - `GUI_DestroyableTask` works either way because it re-`Start`s on every `AsyncDestroy`.
  - [Needs Runtime] to confirm the first-fire timing.
- **Memory** (`GUI_MemoryHog`): a `new_handler` hook. WED's undo manager releases memory
  through it.

### HiDPI and scaling

There is no real HiDPI support:
- Mac: `XWinGL.mac.mm` sets `setWantsBestResolutionOpenGLSurface:NO`. Retina renders at 1×
  and the OS upscales it, so GUI units are points and GL pixels at the same time.
  `get_retina_backing()` / `XWin::GetRetinaBounds` exist only for logging and for
  `WED_LibraryPreviewPane` to decide on MSAA.
- Windows: no DPI-awareness manifest or API call turned up in `src/UI`, `src/WEDResources`, or
  the cmake files, so Windows bitmap-stretches WED on high-DPI monitors. Adding per-monitor DPI
  awareness is on [the punch list](../bug-punch-list.md) (Ben, 2026-09-25).
- Linux: no FLTK screen scaling is used.
- The only user-facing knob is the font-size preference. Images, splitter widths, and table row
  heights are fixed in pixels.

## Platform Matrix (quick reference)

| Concern | macOS (Cocoa) | Windows (Win32) | Linux (FLTK 1.3) |
|---|---|---|---|
| Event loop | `run_app()` (NSApp) | `GetMessage` + `TranslateAccelerator` | `Fl::wait()` until `mDone` |
| Close box, `Closed()` = true | no delete (see above) | synchronous `delete` | `hide()` |
| Drag detect | nested tracking loop | `DragDetect` + posted fake up | nested `Fl::wait` with `mBlockEvents` |
| DnD result to source | always none | real effect | none; stubbed data |
| GL sharing | shared context | `wglShareLists` | same context reused |
| Tooltips | `CalcHelpTip` from ObjC | Win32 tooltip control, `TTN_GETDISPINFOW` | `Fl_Tooltip` in `GUI_Window::handle` |

## Who Uses What

From `cmake/*.cmake`:
- **WED** is the only target that compiles the `GUI_*` widgets, `GUI_Application`,
  `GUI_Window`, and `OGLE`. On Linux it links `fltk::fltk` and `egl::egl`.
  `cmake/WED.cmake` lists `src/GUI/mmenu` as an include directory, but that directory doesn't
  exist. It's a stale entry.
- **ObjView** and **XGrinder** use the `src/UI` layer directly (`XWin`, `XWinGL` for ObjView,
  `XGUIApp` / `XGrinderApp`) with its simple index-based `HandleMenuCmd` menus
  (`menuItemPicked:` on Mac). They also compile `GUI_Unicode.cpp`.
- **DSFTool** and **DDSTool** compile only `GUI_Unicode.cpp` (UTF-8/16/32 conversions).

A change to `XWin` affects ObjView and XGrinder as well as WED. A change to `GUI_Unicode`
affects every tool.

## Connections to Other Systems

- [wed-mcp.md](wed-mcp.md): uses a `GUI_Timer` as its main-thread queue, `GUI_Commander::IsDeferring` for its safe point, and the modal hooks in `Utils/PlatformUtils.h`.

- **Design rules** ([wed-design-principles.md](wed-design-principles.md)): the stated rules for new code and reviews that this subsystem's conventions should follow — command ownership, pointer vs ID, class vs interface casts, error handling, layering, per-doc prefs, platform reference.
- [wed-ui-panes.md](wed-ui-panes.md): WED windows, the document-window layout
  (splitters/packer/stickies), property and hierarchy tables (`WED_PropertyTable` as the
  `GUI_TextTableProvider`).
- [wed-map-and-tce.md](wed-map-and-tce.md): map panes. Mouse capture, command deferral, and
  `PreCommandNotification` matter most for `WED_HandleToolBase` drags.
- [wed-core-services.md](wed-core-services.md): `WED_Application` (`GUI_Application`
  subclass, prefs broadcast, font size) and `WED_Document` (`GUI_Destroyable`).
- [wed-object-model.md](wed-object-model.md): model-change broadcasts arrive as
  `GUI_Broadcaster` messages, table content-resize messages kill in-progress edits, and the
  undo manager is a `GUI_MemoryHog`.
- [wed-import-export.md](wed-import-export.md): modal import/export dialogs (`AsyncDestroy`,
  `Closed()` conventions).
- [wed-network-and-filecache.md](wed-network-and-filecache.md): downloads never call back, so
  `GUI_Timer` polls drive them all; the upload dialog relies on `GUI_FormWindow::Reset` removing its buttons.
- [wed-validation.md](wed-validation.md): `WED_ValidateDialog` (`GUI_Window` + `GUI_TextTable`) is
  modal but returns at once, so export proceeds before the user has seen the list.
- [utils-platform-and-files.md](utils-platform-and-files.md): `PlatformUtils`, `ObjCUtils`,
  and the forced-include `XDefs.h` platform macros (`APL`/`IBM`/`LIN`).
- [bitmap-texture-and-ddstool.md](bitmap-texture-and-ddstool.md): `BitmapUtils`/`TexUtils`,
  which `GUI_Resources` and `GUI_Fonts` use for PNG decoding and texture upload.
