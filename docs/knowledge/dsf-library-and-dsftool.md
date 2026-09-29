# DSF Library (DSFLib) and DSFTool / DSF2Text

> Source: `src/DSF/` (DSFLib reader/writer, point pools), `src/DSFTools/` (DSFTool CLI + DSF2Text text format) · Build: `cmake/DSFTool.cmake`, and DSFLib + `DSF2Text.cpp` are also compiled into WED (`cmake/WED.cmake`)
> Raw source: [dsf-library-readme.txt](raw/dsf-library-readme.txt) (Ben's original DSFLib notes from 2004/05; see "What still holds from the raw readme" below) · [`src/DSFTools/README.dsf2text`](../../src/DSFTools/README.dsf2text) (user-facing DSFTool reference. It ships with releases, so it stays where it is. Parts of it are wrong; see below.)

## Things That Will Bite You

### Precision: 16-bit quantization truncates, and scale/offset are stored as float32

Every non-road coordinate goes into a 16-bit point pool. `DSFTuple::encode` computes
`(v - offset) * 65535 / scale` in double. `WritePoolAtoms` then converts that to `uint16_t`
with a plain C cast, which **truncates instead of rounding**. So a decoded value is always
at or below the input, off by anything from 0 up to (but not including) one quantum. `WriteScaleAtoms` writes
scale and offset as **float32**, while the encoder used the double values. That makes the
pool bounds safe only when they are exactly representable in float. This is why:

- the terrain and object pool grids use `i / divisions` fractions of a 1° tile, and why
  `divisions` must be a **power of two**. A float32 offset near 47° has an ulp of about 3.8e-6°
  (about 0.4 m), so a non-power-of-two grid like DIVISIONS 10 gives offsets like 47.1
  that are not float-exact.
- `EndPolygon` snaps polygon pool boxes to power-of-two grids ("BEN SAYS: we MUST work in
  powers of 2").
- `DSF_ExportTile` (WED) applies `floor`/`ceil` to the MSL range before passing it as `inElevMin/inElevMax`.

One quantum is `pool_extent / 65535`. At 1/32° (WED, `DSF_DIVISIONS 32`) that is about 5 cm.
At 1/8° (DSFTool default `DIVISIONS 8`) it is about 21 cm. For a 1°-wide pool it is about 1.7 m.
Object headings are about 0.0055° per quantum. Roads use the 32-bit pool, so their precision
never matters.

[Needs Runtime] Because the writer truncates and DSF2Text prints 9 decimals, a
DSF→text→DSF round trip can plausibly move a coordinate down one quantum: a printed value
slightly below the exact decoded value re-encodes to q−1. `GuessGoodHeights` mentions
round-trip drift for its own choice, but nobody has measured this per-vertex effect.

### Polygon pools are first-fit, so one big polygon can wreck precision for later ones

Terrain and object pools sit on a fixed grid. **Polygon pools are created on the fly**
(`DSFFileWriterImp::EndPolygon`). `DSFContiguousPointPool::AccumulatePoints` puts the
polygon into the **first existing pool** (in creation order) of the same `hash_depth` that
contains every point and still has room. Only if none fits does it add a new pool. The new pool is the
polygon's bbox snapped to the smallest power-of-two fraction of a degree that covers it.
If the bbox straddles a grid line the snapped box doubles. The effects:

- A large polygon (big forest, long facade, big draped poly) gets a pool as coarse as 1° (about 1.7 m quantum).
- Any **later small polygon of the same hash_depth** inside that box lands in the coarse pool
  too. Its precision depends on export order, not on its own size.
  Decision (Ben, 2026-09-25): switch to best-fit — on [the punch list](../bug-punch-list.md).
- A whole polygon (all windings) must fit in one pool: at most 65535 points in total, and at most 255
  windings (`NestedPolygonRange` writes the count as a uint8).

`hash_depth` = depth + 100 (bezier) + 200 (has ST). It separates plain, bezier and UV
polygons, but it does **not** separate small polygons from large ones.

### Pool ranges are hard limits, and going out of range means an Assert, not an error code

The writer fixes each pool's range up front. A value outside it fails to "sink" and hits an
`Assert` (there is no return code; `DSFWriteToFile` returns void). In DSFTool,
`AssertShellBail` prints the message and calls `exit(1)`. In WED, `WED_AssertHandler_f`
shows an alert and **throws** (`WED_Assert.cpp`). The ranges are set in the
`DSFFileWriterImp` ctor, `BeginPatch` and `EndPolygon`:

| Data | Planes and fixed ranges |
|---|---|
| Terrain patch (depth ≥ 5) | lon/lat = tile, elev = `inElevMin..inElevMax`, normals **−1..1**, every extra plane (UVs) **0..1**. Depth < 5 cannot be encoded. A terrain UV outside [0,1] is fatal. |
| Draped object | lon, lat, heading **0..360** (WED normalizes to [0,360)) |
| MSL/AGL object | lon, lat, heading, elev = `inElevMin..inElevMax` |
| Road (32-bit pool) | lon, lat, elev **−32768..32767**, node ID (raw, scale 0) |
| Polygon depth 2/3/4/5/6/8 | see `EndPolygon`'s `switch(depth)`. Any other depth (e.g. 7) cannot be encoded. The third plane of depth 3/5 and the last plane of depth 6 use scale 0 (raw integer, which must fit 0..65535). Polygon UVs are snapped to *integer* bounds, so unlike terrain they may go outside [0,1]. |

A scale of 0 means "store the raw value". Both `DSFTuple::in_range` and the reader
(`DecodeNumericPlaneInterleavedScaled`) special-case it.

### Coordinate-depth conventions callers must know

- **Forest "+10" signal:** `BeginPolygon_f(..., 4 + 10, ...)` means "4 planes, but this is a
  forest with height/MSL". The writer subtracts 10 and uses the ranges height 0..8191.875 and
  MSL −1000..7191.875 (1/8 m steps). WED's elevated-forest export uses this
  (`WED_DSFExport.cpp`). The DSF2Text writer callbacks do **not** understand it: they would
  print 14 coordinates from a 4-element array. Can the gateway text-export path
  (`DSF_ExportAirportOverlay`) reach an elevated forest? Probably not: `elevated` requires a
  visible `WED_TerPlacement` with a loadable DEM covering the tile, and Gateway-target validation
  rejects resources outside the default library (`WED_Validate.cpp`), which a local DEM `.tif`
  would be. [Needs Runtime] to confirm.
- **Depth 4 polygons:** `param == 65535` means UV-mapped (ST) draped polygon. Any other param means bezier
  (control lon/lat in planes 2–3). Depth 8 is a bezier with two UV pairs.
- **Objects** always go in as `{lon, lat, heading, [elev]}`, and `obj_elev_mode` decides between 3 and 4 planes.
  The DSF2Text text format writes MSL/AGL objects as `lon lat ELEV HEADING` (see the README issues below).
- **Road node IDs must be non-zero.** The reader treats plane 3 == 0 as a shape point
  (`segCoord[3]` test), and `AddSegmentShapePoint` inserts a 0 there.
- **Curved roads are unsupported on write** (`Assert(!inCurved)` in all three segment
  callbacks). `DSFTuple::insert`'s memmove has a latent bug (`ptr + sizeof(double)` on a `double*`
  moves by 8 elements). It is harmless only because the one caller inserts at the end of
  the tuple. It would corrupt curved shape points if curves were ever enabled.
- `MAX_TUPLE_LEN` is 9 (the header comment explains why). DSF2Text's `PATCH_VERTEX` parser
  accepts 10, but a tuple beyond 9 hits `AssertPrintf`.

### `DSFCallbacks_t` is initialized positionally by WED

`WED_DSFImport.cpp` (twice) and `WED_TerrainLayer.cpp` build `DSFCallbacks_t` with
aggregate `{ NextPass, AcceptTerrainDef, ... }` initializers. Inserting or reordering a
field silently shifts every function pointer. **Only append fields at the end**, as
`PointPoolInfo_f` was. The reader calls every callback except `PointPoolInfo_f` without a
null check, including `SetFilter_f` and `NextPass_f`. Passing nullptr means "skip" only for
`PointPoolInfo_f`.

### `DSFCreateWriter` header parameter names are wrong

`DSFLib.h` declares `(inWest, inSouth, inNorth, inEast, ...)`. The implementation, and every
caller (WED export, Text2DSF), uses **(West, South, East, North, ElevMin, ElevMax,
divisions)**. Trust the .cpp. The header comment "WorldEditor currently uses 8 divisions" is
also stale: WED uses 32.

### Raster data ownership differs between read and write

- **Write:** `AddRasterData` stores only the `void *`. The buffer must stay alive until
  `DSFWriteToFile`, and DSFLib never frees it (Text2DSF `malloc`s it and leaks it on purpose).
  Bytes go out raw with `fwrite`, with no endian swap.
- **Read:** the `data` pointer points into the file image, which `DSFReadFile` frees on return.
  Copy it inside the callback, as `WED_TerrainLayer` does.
- Raster names (`DEMN`) and raster atoms (`DEMS`) are matched by index. The reader tolerates a missing
  `DEMN` atom; the other definition atoms are mandatory.

### The command-length table is duplicated three times in DSFLib

Command sizes are hand-coded in: the main loop of `DSFReadMem`, `GetAllPools` (the
DIVISIONS/HEIGHTS estimator, which silently stops at an unknown command), and
`analyze_cmd_mem_use` in `DSFLibWrite.cpp` (stats only). A wrong fall-through in
`GetAllPools` crashed dsf2text on files with more than 255 object defs (`9c1a06c7b`, 2026). The
`TerrainPatchFlagsLOD → Flags → Patch` fall-through there is intentional.

### 7z-compressed DSFs (`USE_7Z`, set in `Obj/XDefs.h`)

`DSFReadFile` first tries to open the file as a 7z archive using `src/lzma19` and extracts
entry 0. If `SzArEx_Open` fails, it falls back to reading the file raw. Known defects in this path:

- If the archive opens but **`SzArEx_Extract` fails, it returns `dsf_ErrOK` without
  reading anything**. `be66053ad` introduced this (the earlier code returned `dsf_ErrCouldNotReadFile`).
- The extracted buffer is never freed (it comes from lzma's `allocImp`, not from `malloc_func`),
  so every compressed read leaks it. The earlier code freed it with the wrong allocator
  ("illicit free", `be66053ad`).
- Only `DSFReadFile` decompresses. `DSFReadMem` and `DSFCheckSignature` expect a raw DSF.
  The writer never compresses.

### MD5 is verified on the default read

`dsf_CmdAll` (0xFF) includes `dsf_CmdSign`. So `inPasses == NULL`, which is what all current
callers pass, means the footer MD5 is checked and a mismatch returns `dsf_ErrBadChecksum`.
Only `inPasses[0]` is checked for the sign flag. The writer appends the MD5 in
`DSFSignMD5` by re-opening the finished file. Any byte edit after that makes the file
unreadable.

### Primitive limits and the tri-stripper

- `EndPatch` runs `DSFOptimizePrimitives`. **Only `dsf_Tri` primitives** are merged, deduplicated
  (exact double equality) and re-stripped with `tri_stripper_101`, then re-chunked to 255 vertices or fewer.
  Strips and fans you supply pass through unchanged and must have **255 vertices or fewer**, or the
  write asserts. The easy path is to feed plain triangles.
- An empty patch asserts (`"WARNING: Empty patch."`). That is fatal in both hosts, despite
  the wording. Empty primitives are dropped quietly.
- Patches are **not** sorted. `PatchSpec::operator<` exists but nothing calls it. Objects,
  polygons and chains are sorted, with the filter as the primary key.

### Reader bugs worth knowing before you trust DSFLib on third-party files

- `dsf_Cmd_Polygon` (the non-range form) tests `if (dsf_CmdPolys)` (a constant) instead of
  `flags & ...`, so on a pass without polygons it still calls `AddPolygonPoint_f`. DSFLib's
  writer never emits this command. Other tools might.
- `PointPoolInfo_f` estimation: the local `is_overlay` has the **opposite** meaning (it is
  true for base meshes). For overlays it reads plane `[3]` of every pool used by object
  commands, and draped-object pools have only 3 planes, so that read is out of bounds. This affects DSFTool
  `--dsf2text` on any overlay with draped objects. The garbage can then leak into the
  auto-generated `HEIGHTS` line. [Needs Runtime] Confirm what gets printed. The
  `test/dsftool_elevations` fixture contains only MSL objects, so it doesn't exercise this.
- The length sanity check `(inStart - inStop) < ...` in `DSFReadMem` can never fire (it
  compares a negative ptrdiff against size_t).
- `Assert("!Out of bounds sink.")` in `WriteToFile` has the `!` inside the string, so it never fires.

### The object writer looks broken but works: don't "optimize" it carelessly

The object loop computes a run (`objSpecNext`) but never advances past it. It emits
exactly one command per object: either `Object(loc)` or `ObjectRange(loc, loc+1)`. That
is inefficient but correct. Real range encoding needs contiguous locations **and** the same
filter, and the run test doesn't check the filter.

## Architecture

### Writer pipeline (`DSFLibWrite.cpp`)

The feeder callbacks only **accumulate** into `DSFFileWriterImp`. Nothing touches disk
until `DSFWriteToFile`. That has two consequences:

- **You can call the feeders in any order.** A definition doesn't have to come before the command that
  uses it: an index is just the order of `Accept*Def` calls per type, and DSFLib never
  renumbers. Property order is kept. `SetFilter_f` is sticky state that tags every
  later object, polygon and chain (patches ignore the filter).
- Memory holds everything: all tuples, as doubles.

`WriteToFile` then works in phases:

1. Sink terrain vertices into `DSFSharedPointPool`s, one per coordinate depth, laid out as a divisions×divisions grid
   in lon/lat. Each vertex is shared individually through a hash of its *encoded* tuple.
   `AcceptShared` clones a grid cell's sub-pool when the cell fills 65535 points
   (`4070a4a52`: this is what lets dense UHD meshes encode). The contiguous-range
   paths (`ALLOW_CONTIGUOUS_PRIMITIVES`, `ALLOW_SHARED_ROADS`) are compiled out with `0`.
2. `ProcessPoints` drops empty sub-pools and remaps pool numbers (`MapPoolNumber`). Object
   pools are pre-allocated at 8 duplicate 2D and 4 duplicate 3D sub-pools per grid cell,
   so most of them end up empty. The pool indices written to the file must stay below 10000
   (`UpdatePoolState` asserts this).
3. Roads: chains are sorted by length and sunk contiguously into the single 32-bit pool. The
   chain-merging optimizer is commented out, so roads are no longer reversed (see the README 1.4 note).
4. The file goes out as: header → `HEAD`/`PROP` → `DEFN` (TERT, OBJT, POLY, NETW, DEMN; all always
   present, possibly empty) → `GEOD`, whose pools go in the order 2D objects, 3D objects, terrain
   per depth, polygons per hash_depth, then the 32-bit road pools → `CMDS` → optional `DEMS` → MD5.
   Pool numbers in commands are global offsets into that order (`offset_to_*`).
5. Command stream order: `JunctionOffsetSelect 0`, draped objects, MSL objects, one
   `Comment AGL=1`, AGL objects, polygons, patches, roads. Filter changes come out as `Comment8`
   filter records. The AGL comment is never reset, so the objects-first order is what keeps it valid.

`StCloseAndKill` deletes a half-written file if an Assert throws during the write
phase. It does not help with exceptions thrown during accumulation, and WED's
`DSF_ExportTile` does not destroy the writer on that path.

### Reader (`DSFLib.cpp`)

`DSFReadMem` finds the atoms. It decodes **every** point pool into doubles up front
(`Decompress*ToDoubleInterleaved`, using float32 scale/offset), then makes one command-stream
walk per entry in the zero-terminated `inPasses` array, filtering callbacks by the
`dsf_Cmd*` flags. Callback coordinate pointers point into those decoded arrays. They stay valid
only during the call. The object mode for 4-plane pools starts each pass as MSL and switches to
AGL after an AGL comment. Filter comments call `SetFilter_f` on every pass, whatever that pass's flags are.
The patch depth reported to `BeginPatch_f` is the depth of the pool selected when the patch starts.

## DSFTool and the DSF2Text format (`src/DSFTools/`)

- `DSFToolCmdLine.cpp`: `--dsf2text in1.dsf [in2.dsf ...] out.txt` (the last argument is the output, and
  `-` means stdout, in which case messages go to stderr), `--text2dsf in.txt out.dsf` (`-` means stdin), plus
  `--version` and `--auto_config` (for XGrinder). Single-dash forms are accepted.
- **DSF2Text keeps state in file-level globals** (`sDSF2TEXT_CoordDepth`, `offset_*`/`count_*`,
  `base_name`, `dem_names`) and is not reentrant. WED links the same file (gateway airport
  export writes DSF text through `DSF2Text_CreateWriterCallbacks`), so those counters carry
  across calls within one WED session. `AcceptRasterDef` increments `count_net` (a bug that
  affects the merge offsets).
- **Merging:** each additional input file offsets its definition indices by the running counts.
  `BEGIN_SEGMENT_CURVED` doesn't apply `offset_net`.
- **dsf2text auto-emits `DIVISIONS` and `HEIGHTS`** from `PointPoolInfo_f`. `GuessGoodHeights`
  picks a power-of-two elevation step between 1 m and 1/32 m, based on the
  terrain pools (base mesh) or the 3D-object pools (overlay).
- **text2dsf** (`Text2DSFWithWriterAny`):
  - The first pass collects PROPERTY, DIVISIONS and HEIGHTS, and requires valid
    `sim/west|east|north|south`.
  - In file mode, a second pass collects all `*_DEF` lines, so definitions may appear anywhere.
    In pipe mode the header scan stops at the first non-header line, and definitions are handled
    inline.
  - **Lines that don't match are silently ignored.** That includes a `PATCH_VERTEX` or
    `POLYGON_POINT` whose coordinate count differs from the depth declared in `BEGIN_PATCH` or
    `BEGIN_POLYGON`: the vertex is just dropped.
  - Lines are read with a 4096-byte `fgets` buffer (raised from 512 in `338f9705c`, WED-1547),
    so a longer line gets split.
  - `END_SEGMENT_CURVED` is never parsed: the code matches the `SHAPE_POINT_CURVED` pattern
    instead.
  - `HEIGHTS <step> <min>` becomes `elev_min = min` and `elev_max = min + step*65535`. That range applies
    to terrain elevation **and** to MSL/AGL objects.
- **Raster round trip:** dsf2text writes `<outfile>.<rastername>.raw` next to the text file
  and puts that path on the `RASTER_DATA` line. When the output is stdout, no raw file is
  written and the line has no filename, so text2dsf skips it and the raster is lost.
- `Text2DSFWithWriter(file, cbs, ref)` lets a caller push text through its *own* callbacks.
  WED's text-DSF import works this way.

### Where `README.dsf2text` disagrees with the code

Fixing the shipped doc is on [the punch list](../bug-punch-list.md), high priority (Ben, 2026-09-25).

- The `HEIGHTS` default is **step 1.0, min 0.0 (range 0..65535 m)** (`hgt_scale = 1.0, hgt_offs =
  0.0`), not "1.0 −32758.0". Negative terrain or object elevations need an explicit
  `HEIGHTS` line.
- `DIVISIONS 8` produces 8×8 = 64 grid cells, not "sixteen point pools". The quoted ~20 cm is right.
- `OBJECT_MSL/AGL` field order is `<type> <lon> <lat> <elevation> <heading>`, not
  rotation-then-elevation. Both the writer (`DSF2Text_AddObjectWithMode`) and the parser agree on
  elevation-first, and so does the fixture (`432.0 0.0`).
- The example command `./DSFTool -DSFTool foo.dsf -` should read `-dsf2text`.

## What still holds from the raw readme (2004/05)

Still true:

- The API is C-style, but it compiles only as C++ (the headers pull in `XChunkyFileUtils.h`,
  STL and `hash_map`).
- It uses feeder callbacks with a `void *` ref, lets you do multiple passes with `NextPass_f` returning false to cancel,
  handles point pools and commands for you, keeps your definition indices unchanged, and does not
  build topology (but see the tri-stripper above).
- The reader does not throw.

Stale:

- "Metrowerks only".
- "Errors are printed to the console". The writer now Asserts, and what that does depends on
  the host's handler.
- "sorts your commands". Patches aren't sorted.
- "does not form tri-strips". Plain triangles are now stripped automatically.
- `DSFLib_Print.cpp` and `DSFLib_TestGen.cpp` call pre-2014 signatures (`DSFCreateWriter` with 5
  arguments, `DSFReadFile` with 4). No CMake target builds them, and they no longer compile.
- `dsflib_013005.zip` is the 2005 standalone release snapshot. It is history only.

## Connections to Other Systems

- **WED export**: [wed-import-export.md](wed-import-export.md). `DSF_ExportTile` creates one
  writer per 1° tile with `DSF_DIVISIONS 32`. It gets the MSL range from
  `DSF_HeightRangeRecursive` (or ±32767 if the tile has nothing elevated), normalizes object
  headings, and clamps custom MSL to −500..10000. If the height-range scan misses an
  elevated item, that item fails to sink (Assert). The gateway airport overlay is written as
  **DSF text** through the DSF2Text callbacks, not through the binary writer.
- **WED import**: [wed-import-export.md](wed-import-export.md). `DSF_Importer` reads binary
  DSFs with `DSFReadFile` and text DSFs with `Text2DSFWithWriter`, using the same callbacks for both.
- **WED terrain/elevation display**: [wed-map-and-tce.md](wed-map-and-tce.md).
  `WED_TerrainLayer` reads X-Plane's global-scenery DSFs, which are usually 7z-compressed, and
  copies the raster (DEM) layer out.
- **Chunky-file atoms, planar numeric encoding, MD5, file utils**:
  [utils-platform-and-files.md](utils-platform-and-files.md) (`Utils/XChunkyFileUtils`,
  `Utils/md5.c`, `FileUtils`). Assert handlers: `Utils/AssertUtils` +
  `WEDCore/WED_Assert.cpp` ([wed-core-services.md](wed-core-services.md)).
- **WED-side geometry before it reaches DSFLib**: [utils-geometry.md](utils-geometry.md).
  DSFLib only sees mirrored bezier handles, so WED's `BezierPointSeqToTriple` writes split nodes
  as up to three co-located points and `BezierPointSeqFromTriple` merges them back on import, which
  also swallows genuinely co-located vertices. Tile clipping and winding come from `WED_Clipping`.
  DSFLib's own point pools and quantization (above) are independent of that code.
- **Build**: both targets force-include `src/Obj/XDefs.h`, which sets `USE_7Z 1` and
  `DSF_WRITE_STATS 0`. DSFTool also links `GUI/GUI_Unicode.cpp` (because `FileUtils`
  uses it for UTF-16 paths on Windows) and the vendored `lzma19` and `tri_stripper_101` (both out of scope).
- No other `src/` code includes `DSFLib.h`. Outside `DSF/` and `DSFTools/`, the only includers are the WED files
  listed above plus `WED_SceneryPackExport.cpp` and `WED_EnumSystem.cpp`, which only need the enums.

## Test Fixtures (`test/`, manual, not run by CI)

- `test/dsftool_elevations/`: `+47+012.dsf` is a WED 2.5.2 overlay with MSL objects around 431 m.
  `+47+012.txt` is the expected `--dsf2text` output, and `dsf_out.txt` is an empty placeholder.
  It was added with `da4c9e034` (XST-87, HEIGHTS auto-detect for overlays). The
  `.txt` predates `fe6b094c6`, which changed object precision to `%.5lf` elev and `%.3lf` heading,
  so today's output differs textually. These fixtures should become an **automated regression
  check** (Ben, 2026-09-25) — on [the punch list](../bug-punch-list.md).
- `test/dsf_export/uv_mapping.xml`: a WED project (rename it to `earth.wed.xml`) with
  ground-painted signs and UV-mapped polygons that used to fail export or crash WED
  (WED-943/WED-1003). The check is visual: after export, there should be no seams at tile
  boundaries in X-Plane.
