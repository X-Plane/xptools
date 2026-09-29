# Utils: Platform, Files, and the OBJ Library

> Source: `src/Obj/XDefs.h`, `src/Utils/` (PlatformUtils, FileUtils, MemFileUtils, XChunkyFileUtils,
> zip/unzip, EndianUtils, AssertUtils, CmdLine, CSVParser, STLUtils, XUtils, ObjUtils, PerfUtils,
> ProgressUtils), `src/Obj/` (XObjDefs, XObjReadWrite, ObjDraw, ObjPointPool) · Build glue:
> top-level `CMakeLists.txt`, `cmake/WED.cmake`, `cmake/DSFTool.cmake`, `cmake/DDSTool.cmake`

## Things That Will Bite You

### Build flags and the forced include

- **`src/Obj/XDefs.h` is force-included into every translation unit** of WED, DSFTool, DDSTool,
  ObjView and XGrinder (`/FI` on MSVC, `-include` elsewhere, in each `cmake/*.cmake`). That is
  why most source files use `vector`, `string`, `map`, `min`, `max` etc. with no `std::` and no
  `#include`. XDefs includes the STL headers and adds `using std::...` for dozens of names. A file
  built outside these targets (a unit test, a new tool) won't compile unless it gets the same
  forced include.
- **`DEV` depends on `CMAKE_BUILD_TYPE` at *configure* time.** The top-level `CMakeLists.txt` sets
  `DEV=1` only when `CMAKE_BUILD_TYPE STREQUAL "Debug"`. Anything else gives `DEV=0`, including
  RelWithDebInfo and an empty value. Multi-config generators never see the configuration you pick
  at build time. `cmake.sh` avoids the problem by making one build dir per config
  (`build_Debug`, `build_RelWithDebInfo`, `build_Release`). `cmake.ps1` makes a single `vs_build`
  configured with `-DCMAKE_BUILD_TYPE=Release` by default, but its closing message suggests
  `cmake --build vs_build --config Debug`. That produces an unoptimized binary with `DEV=0`: no
  `DebugAssert`, no `CHECK_GL_ERR`, no debug console.
- **`WED=1` is defined only for the WED target** (`cmake/WED.cmake`). Shared code uses it to change
  behavior. In other tools `LOG_MSG`/`LOG_FLUSH` compile to nothing, so every `LOG_MSG("E/...")`
  error report in shared code (for example all of `XObj8Read`'s diagnostics) is silent outside WED.
  `FILE_compress_dir` and `FILE_find_dsfs` exist only in WED. `MemFile_Open` unzips single-entry
  zips only in *non*-WED tools (see below).
- **`OBJVIEW` is never defined** (`cmake/ObjView.cmake` doesn't set it), even though XDefs tests
  `#if (WED || OBJVIEW)` to turn on `SUPPORT_UNICODE`. On Windows, only WED gets the UTF-8
  `fopen` wrapper.
- `BIG`/`LIL` are set twice: by CMake (`BIG=0;LIL=1`) and again by XDefs from the platform (a
  leftover from PowerPC). The values match. XDefs is what actually decides.
- XDefs has compile-time feature switches (`ROAD_EDITING`, `GATEWAY_IMPORT_FEATURES`,
  `GATEWAY_IMPORT_MODE`, `XOBJ8_USE_VBO`, `LIBTESS`, `USE_7Z`, `HAS_GATEWAY`,
  `SAFE_VECTORS` …). They apply to every target at once. Don't flip `SAFE_VECTORS`: the
  `dev_vector` block it enables won't compile (its constructors are named `__dev_vector`).
- On Windows, XDefs includes `<winsock2.h>`/`<windows.h>` everywhere. It defines
  `WINDOWS_LEAN_AND_MEAN`, which is a typo for `WIN32_LEAN_AND_MEAN`, so the full `windows.h` is
  pulled in. No `NOMINMAX` is defined anywhere in the repo. You would expect the `min`/`max`
  macros to collide with `std::min(...)` (for example in `DSFPointPool.cpp`), yet Windows CI
  builds. [Needs Human] Why does this work? Is `NOMINMAX` coming from the Conan toolchain?

### Paths and Unicode (the classic bug source)

- **The internal convention is UTF-8 `char*` paths everywhere.** Nothing enforces it. On Windows
  it holds only where a call goes through one of these:
  1. The **`fopen` macro**. With `IBM && SUPPORT_UNICODE` (WED only), XDefs `#define`s `fopen` to
     `x_fopen`, and `FileUtils.cpp` implements that as `_wfopen(convert_str_to_utf16(...))`.
     The macro also covers C files like `zip.c`/`unzip.c`, so `zipOpen`/`unzOpen` are UTF-8-safe
     in WED.
  2. **`ofstream`**, which XDefs `#define`s to `x_ofstream` (UTF-8 → wide open). It has no
     filename constructor: `ofstream f(path)` won't compile on Windows WED. Use `.open()`.
  3. The explicit wide-char code in `FileUtils.cpp`, `MemFileUtils.cpp` and
     `PlatformUtils.win.cpp` (`_waccess`, `_wstat`, `CreateFileW`, `FindFirstFileW`, `MoveFileW`, …).
- **Anything else is ANSI-codepage on Windows and breaks on non-ASCII paths**, for example a
  user profile or scenery folder with accented characters. Known cases:
  - `std::ifstream` is *not* wrapped. `WED_MetaDataDefaults.cpp` and `WED_MetadataUpdate.cpp`
    open the cached metadata CSV (under `%APPDATA%`) with it. They also check `t.bad()`, which a
    failed open doesn't set, so the failure is silent.
  - Plain `rename()` in `WED_Document::Save`, which makes the `.bak` of `earth.wed.xml` (and
    also fails on Windows when the target exists). See "Save" in
    [wed-object-model.md](wed-object-model.md#persistence-earthwedxml).
  - `DSFLib.cpp`'s 7z path calls lzma's `InFile_Open` → `CreateFileA`. If that fails, the code
    returns `dsf_ErrCouldNotOpenFile` without trying the plain `fopen` fallback. So DSF import
    from a non-ASCII path on Windows plausibly fails. [Needs Runtime] See
    [dsf-library-and-dsftool.md](dsf-library-and-dsftool.md).
  - `WinMain`'s `lpCmdLine` is ANSI. `CmdLine` passes it through untouched, so `--package=<non-ASCII>`
    won't match.
  - `ConfirmMessage` on Windows sets button labels with `SetDlgItemTextA`.
- **DSFTool/DDSTool on Windows mix the two encodings.** They don't get `SUPPORT_UNICODE`, so
  `fopen` is plain ANSI and `argv` is ANSI. But `FileUtils`/`MemFileUtils` still treat paths as
  UTF-8 (both tools link `GUI_Unicode.cpp` for this). `MemFile_Open` survives because a failed
  `CreateFileW` falls back to plain `fopen`. `FILE_exists`/`FILE_make_dir`/`FILE_rename_file` on
  non-ASCII input will misbehave. [Needs Runtime]
- **Windows `mkdtemp` shim** (`FileUtils.cpp`) runs `_wmktemp` on a *temporary* UTF-16 copy. The
  caller's buffer is never changed, so `WED_GatewayExport` always uses a literal `tempXXXXXX`
  folder. If one is left over, the shim calls `FILE_delete_file(dir, is_dir=false)`, which can't
  remove a directory, so stale content can survive into the next export.
- `PlatformUtils.win.cpp` `GetFilePathFromUser`/`GetMultiFilePathFromUser` set
  `ofn.lpstrTitle = convert_str_to_utf16(inPrompt).c_str()`. That is a pointer into a temporary
  that dies at the end of the statement (use-after-free, which can garble the dialog title). The
  functions also cap paths at `MAX_PATH`, and the `strncpy` into `outFileName` doesn't guarantee a
  terminating NUL.

### Separators and path shape

- `DIR_CHAR`/`DIR_STR` (`PlatformUtils.h`) are `'\\'` on IBM and `'/'` elsewhere. Several helpers
  only recognize `DIR_CHAR`. For example `FILE_make_dir_exist` walks back by `DIR_CHAR`, so on
  Windows a path containing `/` won't get its parents created. Normalize first.
- WED has two path kinds (`WED_LibraryMgr.h`): **vpaths** (library virtual paths, always `/`,
  made by `WED_clean_vpath`) and **rpaths** (real disk paths, OS `DIR_CHAR`, made by
  `WED_clean_rpath`). `WED_clean_rpath` also strips any trailing byte outside `'!'..'z'`. That
  removes trailing spaces and control characters, and also trailing UTF-8 bytes.
- **`:` counts as a separator** in `FILE_get_file_name`/`FILE_get_dir_name` and in both
  `WED_clean_*` functions (a leftover from classic Mac OS). A Linux/macOS filename containing `:`
  gets split.
- **Trailing separators are inconsistent.** `GetCacheFolder()` ends with `/` on macOS and has no
  trailing separator on Windows/Linux. `FILE_get_dir_name` *includes* the trailing separator.
  `FILE_delete_dir_recursive` *requires* one. `FILE_get_directory_recursive` joins with `DIR_STR`
  and expects none. `FILE_delete_file(…, true)` must not have one.
- `GetApplicationPath()` returns the `.app` bundle path on macOS but the executable's path on
  Windows/Linux. Callers use `FILE_get_dir_name()` of it, so `WED_Log.txt` goes next to `WED.app`
  or `WED.exe`. If that folder isn't writable, `gLogFile` stays NULL and all logging silently
  stops. [Needs Runtime] Check behavior under `C:\Program Files` and `/Applications`.
- `FILE_get_file_name_wo_extensions` searches for `.` in the *whole path* and strips only the last
  extension, despite the plural name. `"a.b/c"` comes back as `"a"`.
  `WED_ResourceMgr::process_texture_path` relies on it.
- `FILE_get_file_extension` returns the extension lower-cased *and without the dot*. The overview
  table in `FileUtils.h` that says otherwise is out of date.
- `GetTempFilesFolder()` is declared in `PlatformUtils.h` and implemented for Windows and Linux
  only. `PlatformUtils.mac.mm` has no definition, so calling it breaks the macOS link. Nothing in
  scope calls it yet.

### Case sensitivity

- On **both Linux and macOS**, XDefs redirects `fopen` to `x_fopen`, which runs
  `FILE_case_correct` on the path first. It tries `stat`, and if that fails it walks each path
  component with `readdir`/`strcasecmp`. macOS was added for WED-1455 (commit 02ae61042) to
  support case-sensitive APFS volumes. Two comments are now out of date: `FileUtils.h` says it's a
  no-op on APL, and XDefs says "APL: no fopen magic needed".
- **Only some calls are case-corrected.** These are: `fopen`, `FILE_exists`, and `MemFile_Open`'s
  mmap path. These are **not**: `FILE_get_file_meta_data`, `FILE_date_cmpr`, `FILE_delete_file`,
  `FILE_rename_file`, `FILE_get_directory`, `MF_GetFileType`, `MF_IterateDirectory`. So on
  Linux, `FILE_exists(p)` can return true while `FILE_delete_file(p)` or
  `MF_GetFileType(p) == mf_Directory` fails for the same `p`.
- For a path that doesn't exist yet (writing a new file), `FILE_case_correct` fixes the existing
  directory components and leaves the leaf name as given, then returns 0. `x_fopen` ignores the
  return value, so creating files works.
- Windows is treated as case-insensitive and `FILE_case_correct` returns 1 without doing anything.

### MemFileUtils: mapping, lifetime, termination

- `MemFile_Open` returns `[begin, end)`. **The buffer is not NUL-terminated.** Never call
  `atof`/`strtod`/`sscanf`/`strlen` on it directly. Use the `MFS_*` / `TextScanner_*` helpers,
  which are bounded by `end`, or copy into a `string`.
- The pointers are valid **only until `MemFile_Close`**. Anything that keeps a `const char*`
  into the file (names, tokens) must copy it first.
- Backing storage depends on the platform and the fallback taken:
  - **POSIX:** `open` + `mmap(PROT_READ, MAP_FILE)`. It passes neither `MAP_SHARED` nor
    `MAP_PRIVATE`. Linux rejects that with `EINVAL`, so on Linux every file probably takes the
    `fopen`+`malloc`+`fread` fallback. macOS accepts it. [Needs Runtime] If another process
    truncates a mapped file on macOS, the reader gets SIGBUS.
  - **Windows:** `CreateFileW(…, FILE_SHARE_READ …)` + `CreateFileMapping` + `MapViewOfFile`.
    **While the MemFile is open, the file can't be written, renamed or deleted, including by WED
    itself.** The failure check is `!winFile`, but `CreateFileW` returns `INVALID_HANDLE_VALUE`, so
    a failed open goes on to a failing `CreateFileMapping` before reaching the fallback.
    `MapViewOfFile` returning NULL isn't checked.
  - **Empty files:** mmap/`CreateFileMapping` fail on 0 bytes and the code falls back to
    `malloc(0)`. That returns NULL on some CRTs, which makes `MemFile_Open` return NULL for an
    empty file on some platforms and an empty span on others. Treat "NULL" and "empty" the same
    way.
  - Size is an `int`, so files are limited to 2 GB.
- **`MemFile_Open` unzips single-entry `.zip` files transparently, except in WED** (`#if !WED`,
  commit bdfd9bbd5 "improve startup & import time"). For example, DSFTool's `DSFLib_Print` reads a
  zipped DSF, but WED's library/resource loading doesn't. `FileSet_Open` (used by
  `WED_GatewayImport` for gateway zips) still handles zips in WED.
- `MFS_double` is a hand-written fast parser (8–10× faster than libc). It **doesn't understand
  exponents** (`1e-05` stops at the `e`) and ignores locale. Any file WED reads through `MFS_*`
  (OBJ, apt.dat helpers, library files) must be written in plain `%f` style. Writers that use
  `%g` will produce values the reader gets wrong.
- `MFS_string_match` requires a space/tab after the keyword (or EOL if `eol_ok`), so `TEXTURE`
  doesn't match `TEXTURE_LIT`. At EOF it reads `*s->cur` one byte past `end`.
- `TextScanner_TokenizeLine` uses function-`static` lookup tables and `AssertPrintf` uses a static
  buffer. Neither is thread-safe. WED's file parsing is main-thread only today. The one known
  worker-thread user in `Utils` is `BitmapUtils` (see
  [bitmap-texture-and-ddstool.md](bitmap-texture-and-ddstool.md)).
- `MemFileUtils.cpp` `#define`s `isspace(c)` to mean space/tab only, partway through the file.
  Code after that point in the same `.cpp` gets the macro, not `<ctype.h>`.

### Asserts: what they compile to

| Macro | `DEV=1` (Debug config) | `DEV=0` (Release, RelWithDebInfo, VS-Debug-with-Release-configure) |
|---|---|---|
| `DebugAssert(c)` | evaluates `c`; on failure calls the installed debug handler | `do {} while(false)`, so **`c` is not evaluated**. Side effects disappear, and using it in an expression position breaks only in release |
| `Assert(c)` | evaluates `c`; handler on failure | **same, always on** |
| `AssertPrintf(fmt,…)` | always calls the assert handler | same |
| C `assert()` | aborts (no dialog, no log) | compiled out if CMake's release flags define `NDEBUG` |

- **Handlers throw.** The default (`AssertUtils.cpp`) `printf`s and throws `assert_fail_exception`.
  WED installs `WED_AssertHandler_f` in `WED_AssertInit()`. It shows a `DoUserAlert` telling the
  user to report the bug and attach `WED_Log.txt`, then throws `wed_assert_fail_exception`.
  Almost nothing catches these. The catch sites are `WED_StartWindow` document open
  (`catch(exception&)` → alert), `WED_Document`'s load (aborts the undo command and rethrows), and
  Linux `GUI_Application::event_dispatch_cb` (logs and rethrows). Elsewhere an assert means
  `std::terminate`. **Asserts are crash-with-dialog, not recoverable errors.** Don't use them to
  validate user data.
- `WED_AssertInit()` runs late in `WED_AppMain` (after prefs, start window, package manager and
  `gFileCache.init()`). An `AssertPrintf` before that point, such as `WED_FileCache::init`'s "Could
  not get OS cache folder", uses the default handler: stdout only, no dialog, then an uncaught
  exception.
- `AssertPrintf` passes its *own* `__FILE__`/`__LINE__` (AssertUtils.cpp), so the file/line in
  the message is useless. Put the location in the format string.
- In `DEV` builds, the `CmdLine` constructor runs its self-test and `DebugAssert`s on it. This
  happens when `GUI_Application` is constructed, before `WED_AssertInit`.

### File-read error conventions (there is no single one)

| API | Failure signal |
|---|---|
| `FILE_*` in FileUtils | `0` = success, otherwise `errno` / `GetLastError()`; `FILE_get_directory*` return a count or `-1`. The header itself says the return value of `FILE_get_directory` is unreliable: use `out_files->size()` |
| `FILE_read_file_to_string` | `errno`/`GetLastError()`. Opens in **text mode `"r"`**, so on Windows CRLF translation makes `fread` return fewer bytes than `ftell`. The string keeps the `ftell` length with trailing NULs |
| `MemFile_Open`, `FileSet_Open` | `NULL` |
| `MF_GetFileType` | `mf_BadFile` |
| `XObj8Read` | `false` **only** if the file won't open or the header isn't `A|I / 800 / OBJ`. Bad counts, out-of-range indices, too many `VT`s and unknown commands are logged (WED only) and the call **still returns true** with partial data. Past a `POINT_COUNTS` overflow the rest of the file is dropped |
| `CSVParser::ParseCSV` | empty table |
| Tar/zip helpers | `bool` / zlib codes |

`CRLF` (XDefs) is `"\r\n"` on Windows. Use it only with binary-mode (`"wb"`) streams, as
`XObj8Write` does. `GUI_Prefs` writes `CRLF` to a `"w"` stream, which produces `\r\r\n` on Windows.
The line scanners treat each `\r` as a line break, so this shows up as extra blank lines.

## Architecture

### Layering of file I/O

```
callers (WED*, DSF, GUI_Prefs/Resources, XObjReadWrite)
   │
   ├─ fopen(...)            ── macro → x_fopen:  Win+WED: UTF-8→_wfopen   POSIX: case-correct then real fopen
   ├─ FileUtils  FILE_*     ── stat/dir/rename/delete/mkdir/zip-a-folder; wide APIs on Win, case-correct on some POSIX calls
   ├─ MemFileUtils          ── whole-file mmap or read; FileSet over dir / zip / tar.gz; line scanners (TextScanner, MFS_*)
   │     └─ unzip.c / zip.c (minizip; opens files through the fopen macro)
   └─ XChunkyFileUtils      ── DSF atom containers over a memory span + StAtomWriter (DSF only)
```

`PlatformUtils` is the OS layer for non-file tasks: app/cache paths, native file pickers,
`DoUserAlert`/`ConfirmMessage`/`DoSaveDiscardDialog`. One file per OS: `.mac.mm` (AppKit; the
file picker needs `InitMacAppKit()` at startup and the bundle Info.plist from
`cmake/Info.plist.in`, per commits 65dcdd4cb/578afd7db), `.win.cpp` (Win32 wide APIs), and
`.lin.cpp` (FLTK). Button order and return mapping differ per OS but are normalized to the
`close_*` enum or 1/2/0. On Linux, `DoUserAlert` passes the message to `fl_alert` as the
**format string**, so a `%` in a message (a path, say) is interpreted as a format directive.

### Which target actually compiles what

This comes from the source lists in `cmake/*.cmake`. It is the authoritative answer to "is this
file dead?".

| File | WED | DSFTool | DDSTool | Notes |
|---|---|---|---|---|
| `Obj/XDefs.h` (forced include) | ✓ | ✓ | ✓ | also ObjView, XGrinder |
| AssertUtils, EndianUtils, FileUtils, zip, unzip | ✓ | ✓ | ✓ | |
| MemFileUtils, XChunkyFileUtils, md5 | ✓ | ✓ | – | |
| PlatformUtils.{mac,win,lin} | ✓ | – | – | also ObjView, XGrinder |
| CmdLine, CSVParser, STLUtils | ✓ | – | – | |
| `Obj/` XObjDefs, XObjReadWrite, ObjDraw, ObjPointPool | ✓ | – | – | also ObjView |
| ObjUtils | ✓ (compiled, **no WED callers**) | – | – | only `XPTools/ViewObj.cpp` (ObjView) calls it |
| XUtils | – | – | – | ObjView, XGrinder (`UI/XGUIApp.cpp`, `UI/XGrinderApp.cpp`) and out-of-scope tools |
| PerfUtils.h (header-only) | ✓ | – | – | `StElapsedTime` logs via `LOG_MSG`, so it prints only in WED |
| ProgressUtils.h (header-only) | – | – | – | only out-of-scope XESCore/RenderFarm code includes it |
| CarbonMemMap.h, MemIStreamBuf.h | – | – | – | **not included anywhere in `src/`**: dead |

DDSTool also compiles `BitmapUtils`/`QuiltUtils` (see
[bitmap-texture-and-ddstool.md](bitmap-texture-and-ddstool.md)). WED compiles two XESCore files,
`AptIO.cpp` and `DEMDefs.cpp`. The rest of XESCore isn't linked.

### XDefs.h, what it provides

In order: the `CRLF` macro; `BIG`/`LIL`; `CHECK_GL_ERR` (DEV only); the global feature switches;
MSVC shims (`strcasecmp` → `_stricmp`, `S_ISDIR`, `_USE_MATH_DEFINES`, `_CRT_SECURE_NO_WARNINGS`);
STL includes and `using std::…`; `hash_map` → `unordered_map` macros; `LOG_MSG`/`LOG_FLUSH` (real
only `#if WED`, writing to `gLogFile`); the `fopen`/`ofstream` redirection; and CGAL adapter macros
(`POINT2` → `Point2`, …) that survived CGAL's removal. Changing anything here recompiles every
target, and the change applies to all of them.

### OBJ library as WED uses it

- **Reader:** `XObj8Read` accepts only OBJ8 version 800 (the OBJ2/7 structs `XObj`/`XObjCmd` remain
  in `XObjDefs.h` for ObjUtils/ObjView only). It reads through `MemFile_Open` + `MFS_*`, with all
  the limits above: no exponents, and whitespace ends a token, so **texture names containing spaces
  are truncated**. Unknown attributes are looked up in the `gCmds` table (`XObjDefs.cpp`) and
  dropped silently if missing. Adding support for a new OBJ command means changing the enum in
  `XObjDefs.h`, `gCmds`, and `ObjDraw.cpp` to draw it.
- **WED-only metadata** sits in OBJ comments: `#fixed_heading`, `#viewpoint_height`,
  `#wed_text` → `XObj8::fixed_heading/viewpoint_height/description`. `LOAD_CENTER` forces
  `fixed_heading = 0`. These fields are never written back.
- **Post-processing lives in `WED_ResourceMgr::LoadObj`, not in the reader.** It appends
  `ANIM_end`s to balance broken assets (otherwise glPush/PopMatrix get unbalanced). It turns
  `TEXTURE`/`TEXTURE_DRAPED` into absolute paths and **replaces the extension**, trying `.dds`,
  then `.png`, then `.bmp` (`process_texture_path`) and ignoring what the OBJ said. It also
  copies texture ↔ draped texture when only one is present. Code that calls `XObj8Read` directly
  gets none of this. See [wed-core-services.md](wed-core-services.md).
- **GPU buffers:** with `XOBJ8_USE_VBO`, `ObjDraw` creates `geo_VBO`/`idx_VBO` on first draw by
  `const_cast`ing the `const XObj8`. They're shared across windows only because all WED GL
  contexts share one group (the start window is kept alive for this; see the "Ben says" comment
  in `WED_AppMain.cpp`). **Nothing ever calls `glDeleteBuffers`.** `WED_ResourceMgr::Purge`
  deletes `XObj8`s and leaks their VBOs. Copying an `XObj8` after it has been drawn copies the
  handles (shallow). See [wed-map-and-tce.md](wed-map-and-tce.md) for the drawing side.
- **Writer:** `XObj8Write` ("hasn't been updated since XP 10.00", per the header) has one WED
  caller, `WED_OrthoExport`, which uses it for terrain/draped OBJs. If export needs a newer OBJ
  command, the writer has to learn it first.
- `XObjDefs.h` has `using namespace std;` at file scope.

### Smaller utilities

- `CmdLine`: only `--key=value` or `--flag`. `--key value` gives two flags. Windows parses the raw
  `lpCmdLine` string and POSIX parses `argv`. WED reads `--package` (`WED_StartWindow`) and
  `--gateway_api_url` (`WED_GatewayExport`).
- `CSVParser` keeps a **`const string&`** to its input. Parse right away, e.g.
  `CSVParser(',', str).ParseCSV()`. Storing the parser past the input's lifetime leaves a dangling
  reference.
- `PerfUtils`: `query_hpc()` units are platform ticks (microseconds on Linux). Always convert with
  `hpc_to_microseconds`.
- `ProgressUtils` macros expand to bare `if` statements with no braces, so a following `else`
  binds to them.
- `XChunkyFileUtils`: `XAtomPackedData::Read*` don't check bounds (call `Overrun()` afterward).
  `StAtomWriter` patches the atom length in its destructor using `int32_t` `ftell`, so its C++
  scope *is* the atom boundary. DSF details are in
  [dsf-library-and-dsftool.md](dsf-library-and-dsftool.md).

## Connections to Other Systems

- **Design rules** ([wed-design-principles.md](wed-design-principles.md)): the stated rules for new code and reviews that this subsystem's conventions should follow — command ownership, pointer vs ID, class vs interface casts, error handling, layering, per-doc prefs, platform reference.
- [wed-core-services.md](wed-core-services.md): `WED_ResourceMgr` (OBJ loading/post-processing,
  MemFile users), `WED_LibraryMgr` (vpath/rpath, `MF_IterateDirectory`), `WED_PackageMgr`
  (`MF_GetFileType`), `WED_Document` save/backup (`rename`, `fopen`), `WED_Assert`, `WED_AppMain`
  startup order and log file.
- [wed-network-and-filecache.md](wed-network-and-filecache.md): `WED_FileCache::init` uses
  `GetCacheFolder`/`FILE_make_dir_exist`/`FILE_get_directory_recursive`. The metadata CSV is read
  with `ifstream` + `CSVParser`.
- [wed-import-export.md](wed-import-export.md): gateway export (`mkdtemp` shim,
  `FILE_compress_dir`), gateway import (`FileSet_Open` on zips), ortho export (`XObj8Write`),
  scenery import (`FILE_find_dsfs`).
- [dsf-library-and-dsftool.md](dsf-library-and-dsftool.md): `XChunkyFileUtils`, `MemFile_Open`
  (zip-transparent in DSFTool), the 7z `CreateFileA` path.
- [bitmap-texture-and-ddstool.md](bitmap-texture-and-ddstool.md): `BitmapUtils`/`TexUtils` use
  `EndianUtils` and `fopen`. DDSTool shares `FileUtils`/`AssertUtils` with no Unicode wrapper.
- [wed-map-and-tce.md](wed-map-and-tce.md): `ObjDraw` and VBO lifetime for previews.
- [gui-framework.md](gui-framework.md): `GUI_Unicode` (the UTF-8 ↔ UTF-16 converters all of
  this relies on), `GUI_Application` (owns `CmdLine args`), `GUI_Prefs`/`GUI_Resources` (MemFile
  users, the `CRLF` quirk).
- [utils-geometry.md](utils-geometry.md): the rest of `src/Utils` (CompGeom, GISUtils,
  MatrixUtils, …) lives there.
