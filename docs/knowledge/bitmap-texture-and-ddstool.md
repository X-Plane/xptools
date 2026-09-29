# Bitmap, Texture and DDSTool

> Source: `src/Utils/BitmapUtils.*`, `src/Utils/TexUtils.*`, `src/Utils/QuiltUtils.*`,
> `src/Utils/BWImage.*`, GeoTIFF corner code in `src/Utils/GISUtils.cpp`, `src/XPTools/DDSTool.cpp`,
> `cmake/DDSTool.cmake` · Raw source: [`src/XPTools/README.DDSTool`](../../src/XPTools/README.DDSTool)
> (user-facing, shipped with releases, so it stays where it is) · Fixtures: `test/img_reading/`,
> `test/tiff_import/` (see `howto_test.txt` there)

## Things That Will Bite You

### `ImageInfo` is bottom-up BGR(A), and every loader converts to that
The `ImageInfo` in `BitmapUtils.h` is a plain C struct: `data`, `width`, `height`, `pad`,
`channels`. The first byte of `data` is the **lower-left** pixel, and the byte order is
**B,G,R(,A)** (the "OpenGL BGR" naming, not something that depends on endianness). Each loader
converts to this layout. The PNG and JPEG loaders flip rows and swap R/B, and the TIFF loader
reorders libtiff's packed `uint32`s by `#if BIG/LIL`. BMP and uncompressed DDS are stored this
way already. The writers convert back. `WriteBitmapToPNG` flips rows, and both DDS writers go to
RGBA with the top row first (see `swap_bgra_y`). Code that reads pixels directly
(`WED_ResourceMgr`'s control texture, `WED_LibraryPreviewPane`'s line luminance,
`WED_SlippyMap`'s tint) has to assume BGR with row 0 at the bottom. A comment in
`LoadTextureFromImage` calls choosing BGR "the real architectural misfortune". Don't try to
change it.

### `pad` is not reliably 4-byte alignment
The header comment says each scanline starts on a 4-byte boundary. That holds only for
`CreateNewBitmap` and the BMP loader. The PNG, JPEG, TIFF and DDS loaders always set `pad = 0`,
so an RGB image with an odd width has unaligned rows. Things only work because:
- `TexUtils` calls `UnpadImage` before uploading, and
- every `XWinGL.*` context sets `glPixelStorei(GL_UNPACK_ALIGNMENT, 1)`. This is the coupling you
  can't see from `TexUtils.cpp`. If you upload from a GL context that XWinGL didn't create, RGB
  images with odd widths come out skewed.

Many helpers ignore `pad`: `FillBitmap`, `ConvertBitmapToAlpha` (its output always has `pad=0`),
`WriteBitmapToDDS_MT`'s per-thread row offsets, `WriteUncompressedToDDS`,
`AdvanceMipmapStack`, and all of the `MakeMipmapStack*` functions. `MakeMipmapStackWithFilter`
doesn't reset `ioImage->pad` after replacing the buffer. Treat "pad == 0" as required input for
everything except the BMP writer.

### Ownership: `malloc`/`free`, shallow copies, and functions that swap out `data`
`DestroyBitmap` is just `free(data)`. `ImageInfo` has no destructor, so copying one
(`ImageInfo img(ioImage)`, which the writers do) aliases the buffer. These functions **free and
replace** `data`, which leaves every alias dangling: `ConvertBitmapToAlpha`,
`ConvertAlphaToBitmap`, `RotateBitmapCCW`, `MakeMipmapStack`, `MakeMipmapStackFromImage`,
`MakeMipmapStackWithFilter`, `CopyBitmapSectionSharp` (only when it sharpens), and DDSTool's
`HandleScale`. `AdvanceMipmapStack` moves `data` forward, so never free an advanced copy.
`LoadBitmapFromAnyFile` frees on failure and leaves `data == NULL`. It depends on the last
loader it tries (DDS) to null the pointer.

### `WriteBitmapToDDS_MT` changes the caller's image
It calls `swap_bgra_y(ioImage)` **in place** before compressing. Afterwards the caller's buffer is
RGBA with the top row first, and nothing swaps it back. Both current callers (DDSTool and
`WED_OrthoExport`) throw the buffer away afterwards. If you add a caller that keeps using the
image, copy it first. The legacy `WriteBitmapToDDS` swaps back only under `#if !WED`. It asserts
`channels == 4` and assumes `pad == 0`.

### Two DDS writers, two numbering schemes
- `WriteBitmapToDDS(img, dxt, ...)` takes `dxt` = 1, 3 or 5 (the DXT number). Nothing in
  WED or DDSTool calls it any more.
- `WriteBitmapToDDS_MT(img, BC_type, ...)` takes the **BC number**: 1 = DXT1, 2 = DXT3,
  3 = DXT5, 4 = BC4, 5 = BC5.

Mixing the two already shipped as a bug. Commit 4a45cf12a fixed ortho export, which had been
writing BC5 because it passed `5` meaning "DXT5". BC4 is written with FourCC `BC4U` and BC5 with
`BC5S`. BC5 is labelled signed even though the data is unsigned. That output is effectively
unreachable: see the DDSTool section below.

### The DDSCAPS_COMPLEX gamma flag, and DDSTool `--png2rgb`
In the legacy writers, `use_win_gamma == 0` sets `DDSCAPS_COMPLEX`, which marks the DDS as old
Mac gamma 1.8 (commit b5018d488, "adding gamma correction to all image handling APIs").
`WriteBitmapToDDS_MT` never sets it, so compressed output is always implicitly sRGB/2.2. But
DDSTool's `--png2rgb` calls `WriteUncompressedToDDS(info, outf, 0)` with the value hard-coded
since fc875f39f (2022). So uncompressed DDSTool output **is flagged 1.8**, while README.DDSTool
says everything is sRGB. **This is harmless (Ben, 2026-09-25):** X-Plane 12 runs in sRGB and
treats every DDS as sRGB; it does not "correct" DDS files based on `DDSCAPS_COMPLEX`. The flag
is legacy metadata — don't treat it as a bug, and don't add gamma handling around it.

### DDSTool `--pre_mips` with DXT compression reads past the buffer
Since fc875f39f, `--pre_mips` sets `mip_filter = nullptr` and goes straight to
`WriteBitmapToDDS_MT`. With a null filter, `WriteBitmapToDDS_MT` assumes `ioImage` is already a
packed mip stack (made by `MakeMipmapStack*`), but DDSTool builds that stack only on the
`--png2rgb` path (`MakeMipmapStackFromImage`). With `--png2dxt*`, the double-width tree is
compressed as a single 2W×H top level, and `AdvanceMipmapStack` then walks off the end of the
buffer. The old code called `MakeMipmapStackFromImage` before every write. [Needs Runtime to
confirm the crash or garbage output. The fix is to call `MakeMipmapStackFromImage` on the
compressed path too.]

### TIFF loads come back premultiplied, always 4-channel, and floats are rejected
`CreateBitmapFromTIF` uses `TIFFReadRGBAImage`, which:
- always returns 8-bit RGBA with the bottom row first. That is why TIFF, unlike PNG and JPEG,
  needs no flip (the "totally dumbfounds Michael" comment in `LoadBitmapFromAnyFile`). An RGB
  TIFF comes back as 4 channels with alpha 255.
- **premultiplies unassociated alpha.** Checked against libtiff 4.7.1 on
  `stbarts_rgba_lzw.tif`: raw (62,71,49,a=21) comes back as (5,6,4,a=21). PNG alpha is straight.
  So a semi-transparent TIFF converted by DDSTool or WED ortho export gets dark fringes and
  darker mips. Nothing un-premultiplies it. A bug (Ben, 2026-09-25) — on [the punch list](../bug-punch-list.md).
- rejects IEEE float samples ("Sorry, can not handle images with IEEE floating-point
  samples"). `stbarts_rgba16float_lzw.tif` therefore **fails** in both WED and DDSTool, so that
  fixture shows a known gap, not a supported format. 16-bit integer TIFFs load, reduced to 8
  bits by libtiff.

The TIFF loader silences both the warning and error handlers but restores only the warning
handler.

### The other loaders have their own gaps
- **PNG**: 16-bit is stripped to 8 (`stbarts_rgba16.png` works). Gray becomes 3 channels, gray
  with alpha becomes 4, palette becomes 3 (4 with tRNS), unless `leaveIndexed` is set. With
  `leaveIndexed = true`, which `GUI_GetImageResource` uses for **all** UI PNGs, a palette or gray
  PNG that has a tRNS chunk gets expanded by libpng anyway, while `channels` stays at 1. libpng
  then writes 4 bytes per pixel into a 1-byte-per-pixel buffer. Keep UI resource PNGs RGB, RGBA
  or plain gray, never paletted. A 2-channel (gray+alpha, `leaveIndexed`) image has no branch in
  `LoadTextureFromImage`, which uploads it as `GL_BGR`. The loader keeps its read cursor in
  file-scope globals (`png_current_pos` and friends), so it **isn't reentrant**. Today every
  decode runs on the main thread.
- **JPEG**: always 3 channels, with gray expanded. The row buffer is `width*3`, so a CMYK or
  4-component JPEG overflows it [Needs Runtime]. When the setjmp error path fires after the
  `malloc`, it leaks `data`.
- **BMP** (`CreateBitmapFromFile`): only uncompressed 24-bit files with positive height. It
  reads pixels right after the 14+40-byte header and **ignores `dataOffset`**. Both
  `stbarts_*.bmp` fixtures have 124-byte V5 headers (`dataOffset` 138). `stbarts_argb.bmp`
  (32-bit, BI_BITFIELDS) is rejected. `stbarts_rgb.bmp` "loads" 84 bytes (28 pixels) out of
  step, with header bytes as its first pixels. README.DDSTool 1.4's claim that ".bmp works" is
  only true for files with 40-byte headers.
- **DDS** (`CreateBitmapFromDDS`): decodes only DXT1/3/5 (with libsquish) and uncompressed
  `DDPF_RGB`, and only the top mip. It can't read luminance, ATI1/2, BC4/5, DX10/BC7 or KTX2.

### The direct DDS/KTX2 GPU path has its own rules
`LoadTextureFromDDS` and `LoadTextureFromKTX2` (enabled by `LOAD_DDS_DIRECT` and
`LOAD_KTX2_DIRECT` in `Obj/XDefs.h`) upload blocks without decoding them. They flip Y
**losslessly in the compressed domain** (`BCx_y_flip`), which only works for some sizes:
- DDS needs the height to be a power of two **and** a multiple of 8. The check is
  `(mips && y != NextPowerOf2(y)) || y % 8 != 0`, and since `mips` is always ≥1 it applies even
  without mipmaps. KTX2 checks `mips > 1` and skips the check when `KTXorientation` is `ru`.
  Commit 60d1a5486 fixed an earlier version of this check. Files that fail it drop back to
  `LoadTextureFromFile`, which re-reads the whole file and decodes on the CPU. For BC4/5/7 there
  is no CPU fallback, so the texture just doesn't appear.
- `NextPowerOf2` stops at `GL_MAX_TEXTURE_SIZE`, so a DDS taller than the GPU limit is also
  rejected.
- BC7 modes 0, 2 and 7 can't be flipped. Those blocks are zeroed, which draws them transparent
  black. Partitions 62 and 63 flip slightly wrong.
- There is **no bounds check against `mem_end`** while walking the mip levels. Commit 5acc062ec
  removed the old "not enough data for mipmaps" guard, so a truncated DDS reads past the buffer.
- The code trusts the mip count in the file and never sets `GL_TEXTURE_MAX_LEVEL`. A DDS with a
  partial mip chain loaded with `tex_Mipmap` would be an incomplete texture [Needs Runtime].
  DDSTool and WED both always write the full chain down to 1×1.
- Which formats load depends on GL extensions detected once, in the first context
  (`init_gl_info`): DXT needs `GL_ARB_texture_compression`, BC4/5 need `..._rgtc`, BC7 needs
  `..._bptc`.

### Loads bigger than the GPU limit are shrunk, and callers see the smaller size
`LoadTextureFromImage` bicubic-downscales anything larger than `GL_MAX_TEXTURE_SIZE`. With
`NEW_TEX_LOAD_STRATEGY` (on in `WED_TexMgr.cpp`), `org_x`/`org_y` come from that clamped size,
not from the file. `WED_MakeOrthos` computes its "Kpix" tiling from `org_x`/`org_y`, so on a GPU
that caps at 16k, a 32k-wide image is tiled as 16k [Needs Runtime]. The `#if
!NEW_TEX_LOAD_STRATEGY` branch that kept the true size calls `MakeSupportedType`, and that
function body is under `#if 0` in `BitmapUtils.cpp`, so turning the flag off won't link.

### `CopyBitmapSection*` traps
- `CopyBitmapSection` computes `dstRowBytes` from **`inSrc->channels`**, so source and
  destination must have the same channel count.
- The parameters named "Top" and "Bottom" are row indices in a bottom-up buffer. "Top" is really
  the lower y. `WED_OrthoExport` passes `UVMbottom` as `inSrcTop`.
- `CopyBitmapSectionSharp` runs its unsharp mask only when `channels == 4` and it is downscaling
  by more than 10% on both axes, and it assumes `pad == 0`. In ortho export it runs **before**
  `ConvertBitmapToAlpha`, so JPEG and BMP orthos (3 channels) are never sharpened, while TIFF and
  RGBA PNG orthos are.
- The interpolation works on the stored (sRGB) values. The alpha channel is interpolated like any
  other channel, despite the header comment about "jagged edges".

## Architecture

### Load path: file → `ImageInfo` → GL
`WED_TexMgr::LoadTexture` (keyed by path string; caching rules are in
[wed-core-services.md](wed-core-services.md)) works like this:
1. It sniffs the first 8 bytes. `"DDS "` goes to `LoadTextureFromDDS`, and the KTX2 magic goes
   to `LoadTextureFromKTX2`.
2. If that fails or doesn't apply, it calls `LoadTextureFromFile`, which calls
   `LoadBitmapFromAnyFile`. That tries **PNG → TIFF → JPEG → BMP → DDS** in order, by file
   **content**, ignoring the file extension. Failed attempts are silent (the TIFF and JPEG error
   handlers are suppressed).
3. `LoadTextureFromImage` then:
   - applies magenta→alpha if asked (`tex_MagentaAlpha`),
   - unpads,
   - picks the size: NPOT when the card supports it and `tex_Always_Pad` isn't set, otherwise
     the next power of two, padded by repeating the last row and column, or rescaled if
     `tex_Rescale` is set,
   - returns `s`/`t` (the used fraction of the padded texture) as `vis_x`/`vis_y` in
     `WED_TexMgr`,
   - uploads as `GL_BGR`/`GL_BGRA` (1 channel becomes `GL_ALPHA`), with
     `GL_COMPRESSED_RGB(A)` when `tex_Compress_Ok` is set, and builds mips with
     `gluBuild2DMipmaps`.
   Most `LookupTexture` callers ignore `vis`/`act`. Only `WED_StructureLayer` (reference images)
   uses them. `WED_PreviewLayer` asks for OBJ/AGP textures with `tex_Always_Pad` and no
   `tex_Rescale`, so a non-power-of-two PNG object texture gets padded without its UVs being
   rescaled. **Intended (Ben, 2026-09-25)** — don't "fix" it.

Suffix-based dispatch still exists in `GetSupportedType`. It is used for decisions, not for
loading: `WED_Orthophoto` only tries GeoTIFF georeferencing when the suffix is `.tif`/`.tiff`,
and `WED_Validate` uses it to recognise raw-image resources.

### Colour space and alpha
- **PNG** is the only format with gamma handling. `CreateBitmapFromPNG*(…, target_gamma)` asks
  libpng to convert from the file's gAMA to `target_gamma`. An untagged file is assumed to be
  sRGB (`1/GAMMA_SRGB`, from commit 53d98c54f; ICC profiles are ignored). Passing
  `target_gamma = 0` means raw bytes, which is what `WED_SlippyMap` uses. Everything else passes
  `GAMMA_SRGB` (2.2).
- **JPEG, TIFF, BMP and DDS** bytes are used as stored and assumed to be sRGB.
- **GL upload** uses linear internal formats (`GL_RGBA`, not `GL_SRGB8_ALPHA8`), so WED draws the
  sRGB bytes as they are.
- **DXT compression** (libsquish, `kColourIterativeClusterFit`, the slowest and best cluster fit)
  works on the sRGB-encoded bytes. The output DDS has no colour-space tag (legacy FourCC).
- **Mipmaps**: DDSTool's default `mip_filter_box_with_gamma` averages colour in approximately
  linear space. It uses a fast approximation (the comments measure a 9% error, with
  `to_srgb(from_srgb(x)) == x`); the exact and `powf` versions sit beside it under `#if 0`. Alpha
  is averaged linearly. WED ortho export passes plain `mip_filter_box`, which is **not** gamma
  corrected, so WED-made orthophoto DDS mips come out slightly darker than DDSTool's.
- **Premultiplication**: TIFF is premultiplied by libtiff (see above). Everything else is straight
  alpha.

### DDS writing (`WriteBitmapToDDS_MT`)
- The input must be 4 channels with `pad == 0`, and the dimensions are effectively powers of two.
  DDSTool enforces this through `HandleScale`, and ortho export by rounding up to 4…2048.
- The top level is split into up to 3 horizontal bands (only for images of at least 256×256).
  Band heights are multiples of 4, and each band is a `std::thread` running
  `squish::CompressImage`.
- Meanwhile the calling thread builds and compresses the mip chain. With a filter it rebuilds
  each level from the one above into a scratch buffer of `w*h*2` bytes. Without one it assumes
  the data is already a stack.
- Output goes into one buffer sized 1.5× the top level. The header is written after the mip count
  is known.
- BC4/5 call `squish::CompressAlphaDxt5`, which is an **internal, unexported-by-intent libsquish
  symbol** declared by hand in `BitmapUtils.cpp`. This depends on the conan `libsquish/1.15`
  build exporting it (static). Upgrading libsquish or building it with hidden visibility breaks
  the link for WED, DDSTool and ObjView.

### GeoTIFF georeferencing (orthophoto and reference-image import)
`WED_RingfromImage` (`WEDCore/WED_Orthophoto.cpp`) calls `FetchTIFFCorners`
(`Utils/GISUtils.cpp`, `USE_TIF` only).
- It opens with `XTIFFOpen`, or on Windows `XTIFFInitialize` followed by `TIFFOpenW`, so the
  GeoTIFF tags get registered. Plain `TIFFOpen` would not see them.
- `GTIFGetDefn` and `GTIFImageToPCS` convert the pixel corners. For non-geographic models,
  `GTIFProj4ToLatLong` then inverse-projects.
- That inverse projection targets `+proj=longlat` **on the file's own ellipsoid**. There is no
  datum shift to WGS84, which I checked in the conan libgeotiff source. NAD83 files
  (`ksea_lzw_nad83.tif`) are off by the NAD83/WGS84 difference (about a metre), which is
  tolerated.
- Corners come back in image order: `c[0..1]` is pixel (0, H) (bottom-left), then bottom-right,
  top-left, top-right. They are computed at pixel-area edges (`dem_want_Area`), with
  `RasterPixelIsPoint` handled.
- `WED_RingfromImage` reorders them to a counter-clockwise SW, SE, NE, NW ring, and swaps the
  pairs when `c[1] >= c[5]` (images stored with their origin at the bottom).
- For images wider or taller than 1536 px it also returns a grid of control points (`gcp_t`),
  3–11 per axis with row 0 at the south edge. `WED_MakeOrthos` uses them to bilinearly place each
  ≤2k tile (`interpol_LonLat`), so projected (UTM or Web Mercator) imagery curves correctly.
  Commit 00533272a fixed a regression in this grid. If any grid point fails to transform, it is
  skipped, and then `pts.size() != size_x*size_y` and the indexing in `interpol_LonLat` goes
  wrong.
- A file without geo tags, or one that isn't a `.tif`, is placed to fit the current map view.
  `LoadBitmapFromAnyFile` decodes the whole image just to learn its size.
- The `test/tiff_import` fixtures cover each case (no geo tags, WGS84, NAD83, UTM10, EPSG:3785).
  `howto_test.txt` is a manual procedure. None of this is run automatically.

### DDSTool (`src/XPTools/DDSTool.cpp`)
- It links only `BitmapUtils`, `QuiltUtils`, `FileUtils`, the zip helpers and `GUI_Unicode`
  (`cmake/DDSTool.cmake`), and not `TexUtils` or GL.
- It always builds with `USE_JPEG=1 USE_TIF=1` (enabled on every platform since 8d3aa1967 / 1.4).
  `WED`, `SUPPORT_UNICODE` and `PHONE` are not defined. On Windows that means plain `fopen`, so
  non-ASCII paths may fail.
- The `PHONE` / `WANT_ATI` / PVR code is dead in shipped builds.
- `--auto_config` prints the XGrinder menu spec. `--info` appends dimensions to a text file.

**Argument parsing is position-sensitive.** After the mode flag, the order is fixed: optional mip
flag, then optional `--gamma_22` (ignored), then optional `--scale_*`, then input and output. An
unexpected flag is taken as a filename. `--night_mips`, `--fade_mips` and `--ctl_mips` are only
recognised as `argv[2]`. Output `-` means "input with its last 4 characters replaced by `.dds`",
which assumes a 3-letter extension.

**Choosing the compression mode.** `--png2dxt` counts pixels with 0 < alpha < 250. If there are
any, it uses DXT5; otherwise DXT1, where libsquish's 1-bit alpha rounds everything else.
`WED_OrthoExport::hasPartialTransparency` uses the same test but requires more than 10 such
pixels, so the two tools can choose differently for the same image. The auto-pick branches for
1 channel (BC4) and 2 channels (BC5) are unreachable, because `LoadBitmapFromAnyFile` never
returns fewer than 3 channels. BC4/5 were taken out of the UI on purpose (23bdd1db7: "XP will
support these only via KTX2").

**Scaling.** `HandleScale` rescales with bicubic `CopyBitmapSection` in sRGB space. Without
`--scale_*`, an image whose sides aren't powers of two is an error. `--scale_half` halves the
next power of two **up** (so 1000 becomes 512).

**Quilting** (`--quilt`, `QuiltUtils`):
- Input must be PNG.
- `QuiltUtils` treats each RGBA pixel as an **`unsigned long`**. That is 4 bytes on Windows but
  8 bytes on 64-bit macOS and Linux, so it strides two pixels at a time and runs past the
  buffers there [Needs Runtime].
- `splat_for_spot` measures each trial's error against `best` instead of `trial`, so the
  `trials` argument never improves anything.
- `make_texture` computes the destination stride with `src.pad`.
- Treat quilting as unmaintained. **Decision (Ben, 2026-09-25): remove `--quilt` and `QuiltUtils`**
  — on [the punch list](../bug-punch-list.md); don't fix it.

### Minor pieces
- `BWImage` is a 1-bit raster rasterizer. It is compiled into WED (`cmake/WED.cmake`), but only
  `XESCore` includes it. In non-DEV builds `BWImage.h` `#include`s `BWImage.cpp` to inline it,
  guarded by `INLINING_BW`. It uses the same `unsigned long`-word assumption.
- The legacy PHONE mode in `swap_bgra_y` and `WriteUncompressedToDDS` leaves DDS stored with its
  origin at the bottom.

## Connections to Other Systems

- [wed-core-services.md](wed-core-services.md): `WED_TexMgr` (one per document, path-keyed
  cache, flags ignored, failures not cached, `glGenTextures` leak on failure) and
  `WED_ResourceMgr`. `WED_ResourceMgr` reads a .pol `TEXTURE_TILE` control texture with
  `LoadBitmapFromAnyFile` and indexes it from row 0, which is the **bottom** row. That matches
  X-Plane (checked in the design repo 2026-09-25: X-Plane flips images on load so row 0 is the
  bottom, and `terrain_frag.glsl` samples the page table by UV). The channel→tile formula does
  **not** match: `red *= (tiles_x + 128) / 256` divides first (0 for `tiles_x < 128`), whereas
  X-Plane uses `tile = round(R/255 * tiles)` ≈ `(R*tiles + 127)/255`; `RUNWAY_TILE` also halves
  the green range. On [the punch list](../bug-punch-list.md).
  `WED_Orthophoto.cpp` holds ortho and terrain import (described above).
- [wed-import-export.md](wed-import-export.md): `WED_OrthoExport` caches one full source image in
  `export_info->orthoImg` across tiles. For each tile it crops or rescales into a 4…2048
  power-of-two `DDSInfo` and writes DDS (or PNG), then reads the DDS back with
  `CreateBitmapFromDDS` just to get its size for the .pol. The per-tile `DDSInfo` buffer is never
  freed (it leaks up to 16 MB per tile). It calls `DropTexture` so the map reloads the changed
  image.
- [wed-map-and-tce.md](wed-map-and-tce.md): `WED_PreviewLayer`, `WED_StructureLayer`,
  `WED_WorldMapLayer` and `WED_TCE` are the main `LookupTexture` callers. `WED_SlippyMap` decodes
  cached tiles with PNG (gamma 0), falling back to JPEG, and tints them in place (it assumes
  `pad == 0`).
- [wed-ui-panes.md](wed-ui-panes.md): `WED_LibraryPreviewPane` loads textures and computes line
  luminance from raw `ImageInfo` bytes.
- [gui-framework.md](gui-framework.md): `GUI_GetImageResource` decodes embedded UI PNGs and JPEGs
  from memory (`CreateBitmapFromPNGData` with `leaveIndexed = 1`).
  `GUI_GetTextureResource` uploads them with the caller's flags and caches them by name only, so
  flags are ignored just as in `WED_TexMgr`. UI drawing uses `UI_TEX_FLAGS = tex_Always_Pad`
  (`GUI_DrawUtils.cpp`; power-of-two UI textures were introduced for WED-426). The
  `UNPACK_ALIGNMENT` coupling lives in `UI/XWinGL.*`.
- [utils-geometry.md](utils-geometry.md): the rest of `GISUtils` (translators, heading math).
  Only the TIFF corner code is covered here.
- [utils-platform-and-files.md](utils-platform-and-files.md): `x_fopen` UTF-8→UTF-16 redirection
  (WED and ObjView on Windows only), and `FILE_case_correct_path`, which the TIFF loader uses on
  macOS and Linux.
- [wed-network-and-filecache.md](wed-network-and-filecache.md): the file cache supplies the
  slippy-map tile files.
- ObjView (out of scope) also links `BitmapUtils` and `TexUtils`, so changes here affect it too.
