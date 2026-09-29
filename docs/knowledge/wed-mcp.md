# WED MCP Server

> Source: `src/WEDMCP/` · hooks in `Utils/PlatformUtils.*`, `GUI/GUI_Prefs.cpp`, `UI/ObjCUtils.mm`, `WEDCore/WED_AppMain.cpp`

An in-process [MCP](https://modelcontextprotocol.io) server so agents can drive WED for automated testing:
run menu commands, dump the document as JSON, inject fixtures, edit properties and the selection.
Modelled on X-Plane's MCP (`~/code/design`, `docs/knowledge/web-server.md` there). It starts only when
WED is launched with `--mcp` or `--mcp_port=N` (default port 8087, next to X-Plane's 8086). It is
compiled into every build, Release included.

## Things That Will Bite You

### Threading and the safe point

- **HTTP threads never touch WED.** cpp-httplib runs each request on a pool thread. `tools/call` becomes a
  `WED_MCPJob` that the HTTP thread waits on. `WED_MCPServer` is a `GUI_Timer` (10 ms) that runs jobs on the main
  thread **one at a time**, and only at a safe point:
  - no job is in flight;
  - we're not inside a handler, because timers fire inside nested event loops (`GUI_Timer` on Mac uses
    `kCFRunLoopCommonModes`);
  - `gApplication->IsDeferring()` is false, i.e. no mouse button is down;
  - no open document has `WED_UndoMgr::IsCommandOpen()`.
- **A job that can't start within 10 s is cancelled** and the agent gets `busy`. Cancellation happens under the job
  lock, so a job never runs after the agent has been told it didn't. Once a job starts it runs to completion.
- **Async tools** (e.g. `close_document`, which waits for `AsyncDestroy`) finish later through `WED_MCP_RunLater`.
  That runs a continuation on each server tick. The job stays "current" and holds the queue until it replies.
- **Don't build async steps from a self-deleting `GUI_Timer`.** On Linux, `GUI_Timer::timeout_cb` reads
  `me->mTimer` after `TimerFired()` returns, so deleting the timer inside `TimerFired` is a use-after-free. That's
  why continuations run on the server's own timer.

### Transport and lifecycle

- **cpp-httplib's default socket option is `SO_REUSEPORT`.** With it, a second WED can bind the same port, and the
  kernel then load-balances requests between the two processes. That produced impossible test results
  ("shutting_down" from a WED that wasn't quitting). `WED_MCPServer::Start` replaces the socket options:
  `SO_REUSEADDR` on POSIX, `SO_EXCLUSIVEADDRUSE` on Windows.
- **If the bind fails, WED exits with code 2**, logs to stderr, and writes no prefs. It shows no alert, because the
  hooks aren't installed yet and a modal would hang a script.
- **`[NSApp stop:]` from a timer callback doesn't quit** until another event arrives. `stop_app()` in
  `UI/ObjCUtils.mm` now posts an app-defined event after `stop:`. Without it, `execute_command gui_Quit` "succeeds"
  and WED keeps running.
- **Origin check.** Any request whose `Origin` header isn't localhost gets 403. That blocks DNS rebinding from
  browser pages. The server sends no CORS headers. X-Plane's MCP sends `*` and doesn't check Origin; don't copy that.
- **Protocol.** Streamable HTTP with plain JSON responses: no SSE, no sessions.
  - `GET /mcp` → 405, `DELETE` → 200.
  - `initialize` echoes the client's version if it is one we accept.
  - All server state is main-thread only, except the tool table, which is immutable after construction.

### Modals are answered automatically (`WED_MCPHeadless`)

- `gPlatformModalHooks` (`Utils/PlatformUtils.h`) redirects `DoUserAlert`, `ConfirmMessage`, `DoSaveDiscardDialog` and
  both file pickers on all three platforms. Each dialog is logged and reported in the tool result as an extra
  `alerts: [...]` text item.
- **Defaults never destroy work:**
  - alert → OK
  - confirm → cancel
  - save/discard → cancel
  - file picker → cancel
- Agents queue specific answers with `set_dialog_answers`. `close_document` pushes its own save/discard answer to the
  **front** of that queue.
- **The hooks install only after the server starts.** Asserts that fire earlier, such as the DEV command-table check,
  still show a real modal.
- **WED's own non-OS dialog windows are not intercepted.** That covers `GUI_FormWindow`-based dialogs,
  `WED_ValidateDialog` and the Gateway dialogs. Tools must avoid code paths that open them.

### Undo discipline

- **Every document mutation over MCP happens inside exactly one undoable command, selection included.**
  - Tools that edit directly (`inject_fixture`, `set_properties`, `set_selection`) open `"MCP: ..."` themselves and
    abort on any error, so a failed call changes nothing.
  - `execute_command` lets the menu handler own its command.
  - `FinishJob` checks after every tool that no command was left open. It also checks that tools marked `read_only`
    didn't change `WED_Archive::CacheKey()`. Both are DEV asserts plus `E/MCP` log lines.
- **`WED_Runway::PropEditCallback` splits the command.** When a runway rename changes the runway enum, it calls
  `CommitCommand` and then `StartCommand("Smart Runway Rename")` in the middle of the caller's command.
  - What you see: `set_properties` renaming a runway can leave two undo steps.
  - If nothing references the runway, the second step is empty and is dropped.
  - Properties applied *after* the name, in jsoncpp's alphabetical key order (`hierarchy.name` comes before
    `runway.*`), land in the "Smart Runway Rename" step.
  - Tests should undo back to a recorded stack depth, not count steps.
- **`WED_Airport::AddMetaDataKey` doesn't call `StateChanged`** (see [the punch list](../bug-punch-list.md)).
  `inject_fixture` calls `StateChanged()` on the airport before adding metadata.

### The JSON schema reads raw property items, not IPropertyObject

- `WED_MCPDocJson` iterates `WED_PropertyHelper::mItems` directly, calls `GetProperty`/`SetProperty` on each item,
  and keys everything by `GetXmlName()` + `"."` + `GetXmlAttrName()`, the persisted names. It does **not** go through
  `GetNthProperty`. Entities override that to show a UI view: synthetic rows, dictionary filtering, and
  `WED_Runway`/`WED_Airport` remapping. Reading items directly is the same model as loading `earth.wed.xml`.
- **Skipped:** `WED_TypeField`, which has no XML storage but whose "XML name" still decodes as `"Class"`, and items
  whose `GetPropertyInfo` says `synthetic`.
- **Units:** `WED_PropDoubleTextMeters` converts to feet when `gIsFeet` is set. Dump and inject force
  `gIsFeet = 0` for their duration (the `meters_please` guard), so values are always meters.
- **Enums** are written as `ENUM_Desc` strings, as in the XML. Enum ints are assigned at runtime and are
  meaningless outside the session.
- **Non-property state lives in `extra`:**
  - `WED_Airport` meta_data and `WED_AirportChain` closed are both dumped and injectable.
  - `WED_KeyObjects` choices are private and not dumped.
  - `WED_Root`, `WED_Select` and `WED_KeyObjects` can't be injected.
- **Save + reopen is not bit-exact for doubles.** `WED_PropDoubleText::WantsAttribute` has a hand-rolled fast parser
  that can be one ulp off (-70.99 → -70.99000000000001). Use `dump_document file_precision:true`, which rounds to the
  item's persisted `decimals`, to compare across a save.
- **`ids:false`** replaces IDs with `ref`s (`o1`, `o2`, ... in preorder), so two dumps of the same tree compare equal
  and the dump can be fed back to `inject_fixture`. Sources that point outside the dumped subtree stay numeric IDs.
- **Never dump through `WED_Archive::SaveToXML`.** It resets the dirty count.

### Commands and documents

- **Commands are addressed by enum name**, via `WED_MCPCommandNames.cpp`. Their numbers shift with `HAS_GATEWAY`,
  `ROAD_EDITING` and `GATEWAY_IMPORT_FEATURES`.
  - The table is hand-copied from `GUI_Menus.h` / `WED_Menus.h` with the same `#if` guards, leaving out the
    `wed_AddMetaDataBegin/End` markers.
  - DEV builds assert at startup that the `wed_*` values are contiguous and end at `wed_ESRIUses`. **Adding a menu
    command means adding it here too.**
- **`execute_command` calls `FocusChain(1)` on the target document window first**, then dispatches from
  `gApplication`. That is what makes a command go to the right document when WED isn't frontmost. When launched
  from a terminal, no window is "active", so `WED_MCP_FindDocument` falls back to the only open document and
  otherwise asks for `doc`.
- **Documents die asynchronously.** `TryClose` → `AsyncDestroy`, and the document leaves the
  `WED_StartWindow::sDocs` list only on `msg_DocumentDestroyed`, a later loop pass. `close_document` replies only
  once the document is gone.
- **Opening a package goes through `WED_StartWindow::OpenPackage`**, which reuses the start window's `sDocs` / lock
  bookkeeping. Don't construct `WED_Document` / `WED_DocumentWindow` directly.

### Screenshots and synthetic input (`WED_MCPToolsMap`)

- **Two coordinate systems.**
  - Agents see window pixels with a **top-left** origin, as in screenshots.
  - Panes, events and `WED_Map`'s pixel space use GL coordinates with a **bottom-left** origin (`WED_Map::SetBounds` →
    `SetPixelBounds`, so `LLToPixel` returns window GL coordinates).
  - Convert with `img_y = win_h - gl_y`, where the window size comes from its *pane* bounds.
  - `GUI_Window` inherits `GetBounds` from both `XWin` and `GUI_Pane`, so cast to `GUI_Pane*`.
- **Capture reads the back buffer at the end of `GUI_Window::GLDraw`, before the platform swap**
  (`GUI_Window::RequestCapture`).
  - It forces a refresh and completes asynchronously on the next draw. A hidden or minimized window never draws,
    so the tool times out.
  - The capture is WED's own GL rendering: native menus, OS dialogs and tooltips aren't in it.
  - Mac renders at 1×, so framebuffer = points.
- **Mouse gestures go through `GUI_Window::SynthMouse`**, which calls the real `ClickDown/Drag/Up/Move` with client
  coordinates, so `BeginDefer`/`EndDefer`, mouse capture and tool dispatch behave exactly as for real events.
  - The tool sends **one event per server tick** and holds the job queue until the up.
  - A drag is interpolated (`steps`), because tools react to motion.
  - While a synthetic gesture is under way, `GUI_Window::GetMouseLocNow` returns the synthetic position.
- **Modifiers:** every map tool reads them through `GUI_Pane::GetModifiersNow`, which `SetModifiersOverride`
  replaces. It is global, so during a gesture it also masks the real keyboard.
- **Key presses must carry `gui_DownFlag`.** Real keys arrive as a down event and an up event, and the create tools
  (`WED_CreateToolBase::HandleToolKeyPress`: Return to finish, Escape, Delete) ignore anything that isn't a down.
  The `key` tool sends both.
- **Create tools commit only on emit.** Points clicked with a create tool are tool state, not document state: no
  undo step appears until Return (or a double-click) emits the object. Leftover points from an earlier gesture carry
  over into the next one, so press Escape first to be safe.
- **Double-click detection uses `GUI_Pane::GetTimeNow`**, which is `clock()` (CPU time) on Mac and Windows. Two
  synthetic clicks close together in space can count as a double-click even seconds apart, and that emits the shape.
- **Picking a tool:** `WED_MapPane::SetCurrentTool` calls the toolbar's `SetValue`, the same path as clicking the
  button. Tool settings are the tool's `WED_PropertyHelper` items, keyed by display name. They aren't document state,
  so changing them opens no command.

### Isolation flags (`WED_AppMain.cpp`)

- `--prefs=<file>` → `GUI_Prefs_SetFileOverride`: all prefs reads and writes go to that file.
- `--xsystem=<path>` → the X-Plane folder for this run only. It's never written back, even without `--prefs`.
- A scratch X-Plane folder only needs `Custom Scenery/` and `Resources/default scenery/`. With no packages at all,
  the document constructor's "importable apt.dat?" scan used to index past the package list. That off-by-one in
  `WED_Document::WED_Document` is now fixed.

## Architecture

| File | Role |
|---|---|
| `WED_MCPServer` | httplib listener, JSON-RPC envelope, argument-schema check (flat: known keys, required, top-level types, string enums), job queue, safe point, postconditions |
| `WED_MCPTools` | `WED_MCPCall` (reply once), tool table type, `WED_MCP_FindDocument`, argument helpers, `WED_MCP_RunLater` |
| `WED_MCPToolsApp` | state, packages, open/new/close, list/execute commands, alerts and dialog answers, logs |
| `WED_MCPToolsDoc` | dump_document, inject_fixture, set_properties, set_selection, search_library |
| `WED_MCPToolsMap` | capture_screenshot, get/set_viewport, list_tools, set_tool, mouse, key |
| `WED_MCPDocJson` | document ↔ JSON (dump and inject share the schema) |
| `WED_MCPHeadless` | modal hooks and the alert log |
| `WED_MCPCommandNames` | command name ↔ enum table |

Tool results are `content:[{type:"text", text:<json>}]`, with `isError:true` and
`{error:<stable code>, message, ...}` on failure. Errors carry what's needed to fix the next call without another
lookup, e.g. `unknown_property` lists `valid` keys and `invalid_enum_value` lists the valid strings.

## Recipes

- **Add a tool:** write a handler `void f(WED_MCPCall&, const Json::Value& args)` and push a `WED_MCPTool` in the
  group's `WED_MCP_Register*Tools`.
  - The schema is JSON text and is parsed at startup; a bad schema asserts.
  - Mark `read_only` honestly, because DEV checks it.
  - If the tool edits a document, open one command, commit on success and abort on error.
  - Reply exactly once, or use `WED_MCP_RunLater` if the reply has to wait.
- **Run it:** `WED --mcp --prefs=/tmp/t.prefs --xsystem=/tmp/xp`, then point the client at
  `http://localhost:8087/mcp`. The repo's `.mcp.json` does this for Claude Code.
  - `test/mcp/` has a Python client (`wed_mcp.py`, including `launch()` for a scratch X-Plane folder) and
    end-to-end tests: `test_phase1.py` (documents) and `test_phase2.py` (map and input).

## Connections to Other Systems

- Undo, commands, IDs and dirtiness → [wed-object-model.md](wed-object-model.md). Design rule 1 (outermost command
  owns undo) → [wed-design-principles.md](wed-design-principles.md).
- Property items, persisted names and enums → [wed-entities.md](wed-entities.md),
  [wed-core-services.md](wed-core-services.md).
- Menu dispatch and focus chain → [wed-ui-panes.md](wed-ui-panes.md), [gui-framework.md](gui-framework.md).
- Map tools, handles and the drag state machine the mouse tool drives → [wed-map-and-tce.md](wed-map-and-tce.md).
- Library search → [wed-core-services.md](wed-core-services.md).
