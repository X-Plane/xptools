# CLAUDE.md — XPTools / WED Codebase Context

This repo builds **WorldEditor (WED)**, X-Plane's airport and overlay scenery editor, plus
several smaller Laminar Research scenery tools (DSFTool, DDSTool, ObjView, XGrinder, and the
out-of-scope RenderFarm/MeshTool code). The primary focus is WED.

## Code Quality Standards

WED edits users' scenery — often years of hand work — and uploads it to the shared X-Plane
Scenery Gateway. Data preservation, undo correctness, and file-format fidelity outrank speed
of delivery. These principles bias toward caution; on trivial changes, use judgment instead
of ceremony.

**Think before coding.**
- Read the relevant knowledge article before modifying a subsystem — it encodes cross-file coupling and invariants you cannot derive from the source.
- State assumptions about ownership, lifetime, and undo explicitly. If uncertain, ask.
- If multiple approaches exist, present the tradeoffs — do not pick silently.
- Push back on bad ideas with technical reasoning. Surface what is unclear instead of guessing.

**Simplicity first.**
- The minimum code that correctly solves the problem. No speculative features, abstractions, or configurability.
- If you wrote 200 lines and it could be 50, rewrite it.

**Surgical changes.**
- Touch only what the task requires; every changed line should trace to the request.
- Match existing patterns and conventions, even if you would do it differently. Do not reformat adjacent code.
- Remove only the orphans your own change created. Do not delete code that looks dead — much of it is live platform-specific or conditionally compiled (see below), or used by another tool target. Flag it instead.
- Never change the meaning of an existing persisted property or file-format field without a compatibility plan — old `.xml` documents and apt.dat/DSF files must still load.

**Design rules.** `docs/knowledge/wed-design-principles.md` states the rules for new code and
reviews: the outermost user command owns the undo command (utilities assume one is open); when
raw `WED_Thing*` pointers are allowed; exact-class `static_cast` vs GIS interfaces (never cast to
`WED_GIS*` intermediates); mutate-then-roll-back exports; return errors rather than throw, and
assert only on programmer errors; prefer async I/O; resource handles live until a library-changed
message; WED layers strictly on top of GUI; per-document prefs are authoritative; and Mac +
Windows agreement defines correct cross-platform behavior (never Linux). Read it before writing
or reviewing WED code; flag violations rather than copying them.

**Code review: `earth.wed.xml` format changes.** Any change that adds a persistent class, a
persisted property / XML attribute or element, or an enum description must bump the hard-coded
`doc/xml_compatibility` value (written in `WED_DocumentWindow`'s `msg_DocWillSave` handler) to the
new minimum WED version. Older WEDs silently drop unknown properties, may crash saving unknown
enum values, and refuse to load unknown classes — that pref is their only warning. Flag a missing
bump as a regression. Details: `docs/knowledge/wed-object-model.md` → "Format compatibility rules".

**Verify against a goal.**
- Turn vague requests into verifiable outcomes before implementing.
- For multi-step work, state a brief plan and how each step will be verified — a build, a round-trip of a test file, or a concrete check the user can run in WED.

## Knowledge Base

A compiled knowledge base covers WED, DSFTool, DDSTool, and the shared libraries. Before
working on, analyzing, or reviewing a subsystem, read its article. The index is imported
below; operating instructions (quality bar, article format, how to update) are in
`docs/knowledge/README.md`.

@docs/knowledge/index.md

## Build

Dependencies come from Conan; CMake generates projects for three configurations at once.

```
macOS / Linux:  ./cmake.sh                  # Xcode on macOS, Ninja on Linux; GENERATOR=Ninja ./cmake.sh to override
                cmake --build build_Release --config Release --target WED
Windows:        ./cmake.ps1
                cmake --build vs_build --config Release --target WED
```

- Build dirs: `build_Debug`, `build_RelWithDebInfo`, `build_Release` (CMake presets are included via `CMakeUserPresets.json`).
- Targets: `WED`, `DSFTool`, `DDSTool`, `ObjView`, `XGrinder`, plus OneOffs. Per-target sources and defines live in `cmake/<Target>.cmake`.
- CI (`.github/workflows/build_wed.yml`) builds Release on windows-2022, macos-15, and ubuntu-22.04 — a change must compile on all three.
- CMake 4.x breaks some third-party packages; use CMake 3.x.
- `Building.md` is the human-facing setup guide (toolchain versions, Linux packages).

There is no unit-test target. `test/` holds fixture files (DSF, images, TIFFs) for manual
round-trip checks.

**Driving WED from an agent (MCP).** WED has a built-in MCP server; the `WED` entry in `.mcp.json` points at
`http://127.0.0.1:8087/mcp`. It can:
- run menu commands;
- dump the document as JSON and inject fixtures;
- edit properties and the selection, all undoably;
- take screenshots and drive map tools with synthetic mouse and key input;
- validate and export.

The agent launches WED itself:

1. **Build** WED (see Build above). On macOS the binary is `build_<Config>/<Config>/WED.app/Contents/MacOS/WED`.
2. **Make a scratch X-Plane folder** containing `Custom Scenery/` and `Resources/default scenery/` (empty is fine), or
   use a copy of a real install for real library art. Never point it at the user's own X-Plane folder: WED writes
   packages there.
3. **Launch in the background:**
   `WED --mcp --prefs=<scratch>/wed.prefs --xsystem=<scratch>/xp`
   - `--prefs` and `--xsystem` keep the user's WED prefs and X-Plane folder untouched.
   - WED is ready when a JSON-RPC `ping` POSTed to the URL answers.
   - If port 8087 is taken, WED prints an error and exits with code 2. Use `--mcp_port=N` (then use that port in the
     URL).
4. **Connect.**
   - If WED was running when the Claude Code session started, the `mcp__WED__*` tools are already there.
   - Otherwise the user runs `/mcp` to reconnect; an agent can't do that itself.
   - Without the MCP connection, use the same tools over HTTP with `test/mcp/wed_mcp.py` (`call("get_state")`) from
     Bash.
5. **Start with `get_state`**, then `new_package` / `open_package`.
6. **Finish** with `close_document` (`if_dirty: "discard"`), then `execute_command gui_Quit`.

Keep tool output small:
- `dump_document` of a real airport can exceed Claude Code's MCP output limit (~25k tokens). Use `path`, `root_id`
  or `max_depth`.
- Screenshots default to 1280 px wide.

Details and gotchas: `docs/knowledge/wed-mcp.md`. `test/mcp/test_phase*.py` are end-to-end checks.

## Conditional Compilation — Not Dead Code

- `APL` / `IBM` / `LIN` — macOS / Windows / Linux; exactly one is 1 (set in top-level `CMakeLists.txt` as `BASIC_PLATFORM_DEFINES`)
- `DEV` — 1 when `CMAKE_BUILD_TYPE` is Debug at *configure* time, else 0. On macOS/Linux each `build_<Type>` dir is configured separately, so `build_Debug` has `DEV=1`. On Windows `cmake.ps1` configures one multi-config `vs_build` with `-BuildType` (default Release), so `--config Debug` still gets `DEV=0` — use `./cmake.ps1 -BuildType Debug` for a DEV build
- `LIL` / `BIG` — endianness (always little-endian today)
- `USE_JPEG`, `USE_TIF` — image format support; enabled for WED, DDSTool, ObjView
- `WED=1` — set only on the WED target; shared code in `Utils/`, `DSF/`, etc. uses it to branch between WED and the other tools
- `src/Obj/XDefs.h` is force-included into every WED translation unit (`-include` / `/FI` in `cmake/WED.cmake`)
- Platform-specific files use `.mac.mm` / `.win.cpp` / `.lin.cpp` suffixes; code inside `#if APL` / `IBM` / `LIN` is live

## Naming History

Code named `GISTool_*`, and anything in a folder called `WorldEditor`, belongs to *RenderFarm*,
not WED — old vocabulary. `XESCore/` is RenderFarm's GIS engine; WED uses little of it.
See `src/README.txt` for the history.

## Git Workflow

- `master` — current development; kept release-ready when possible.
- `wed_<NNN>_release` — staging / patch branches for a WED version (e.g. `wed_270_release`). Fixes land on the release branch and are merged into master (not cherry-picked).
- Tags like `wed_271r1` mark public releases.
- Hosted on GitHub (`X-Plane/xptools`); changes go in via pull requests.
- Commit messages: casual but meaningful — describe what, and highlight non-obvious choices.

## Other Docs

- `docs/bug-punch-list.md` — triaged known bugs, not yet fixed. When you fix one, delete its entry and update the linked knowledge article in the same commit.
- `README.md` — project overview, licensing (GitHub-facing).
- `Building.md` — human build and dev-environment setup.
- `src/WEDDocs/` — source for the *user-facing* WED manual. Not developer docs.
- `src/WEDCore/README.WorldEditor`, `src/XPTools/README.*`, `src/DSFTools/README.dsf2text` — user-facing release notes and tool docs shipped by `scripts/bundle.sh`. Don't move them.
