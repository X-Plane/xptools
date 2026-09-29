# XPTools Knowledge Base — Index

> Maintained by the LLM. Catalogs every `src/` directory and its knowledge article. Read this first.

## How to Use This

- **Before modifying code:** find the directory below and read its article's "Things That Will Bite You" section.
- **Cross-cutting topic?** Use the Concept Index.
- **End-user WED behavior** (how a feature is supposed to work for authors) is in the user manual source, `src/WEDDocs/`.
- **After compiling or updating an article:** update this index and append to [log.md](log.md). Rules: [README.md](README.md).

---

## Source Map

Paths are relative to `src/`.

### WED application

| Directory | Files | Article |
|---|---|---|
| WEDCore — archive, undo, XML I/O, document | 61 | [wed-object-model.md](wed-object-model.md) |
| WEDCore — app lifecycle, packages, library, resources, textures, prefs, enums | | [wed-core-services.md](wed-core-services.md) |
| WEDCore — validation (`WED_Validate*`) | | [wed-validation.md](wed-validation.md) |
| WEDEntities | 134 | [wed-entities.md](wed-entities.md) · base classes `WED_Thing`/`WED_Entity` in [wed-object-model.md](wed-object-model.md) |
| WEDMap | 67 | [wed-map-and-tce.md](wed-map-and-tce.md) |
| WEDTCE | 14 | [wed-map-and-tce.md](wed-map-and-tce.md) |
| WEDWindows | 23 | [wed-ui-panes.md](wed-ui-panes.md) |
| WEDProperties | 4 | [wed-ui-panes.md](wed-ui-panes.md) |
| WEDLibrary | 6 | [wed-ui-panes.md](wed-ui-panes.md) |
| WEDImportExport | 30 | [wed-import-export.md](wed-import-export.md) |
| WEDFileCache | 6 | [wed-network-and-filecache.md](wed-network-and-filecache.md) |
| WEDNetwork (mostly compiled out) | 11 | [wed-network-and-filecache.md](wed-network-and-filecache.md) |
| WEDMCP — MCP server for automated testing (`--mcp`) | 15 | [wed-mcp.md](wed-mcp.md) |
| WEDResources (art, no code) | 0 | — |
| WEDDocs (user manual source) | 0 | — not developer docs |
| *(cross-cutting)* design rules | | [wed-design-principles.md](wed-design-principles.md) |

### Shared libraries

| Directory | Files | Article |
|---|---|---|
| Interfaces | 13 | [wed-entities.md](wed-entities.md) (IGIS, IPropertyObject, …) · [wed-object-model.md](wed-object-model.md) (IResolver, IOperation) · [wed-core-services.md](wed-core-services.md) (ILibrarian, ITexMgr, IDocPrefs) |
| GUI | 75 | [gui-framework.md](gui-framework.md) |
| UI (XWin platform layer) | 16 | [gui-framework.md](gui-framework.md) |
| OGLE (text-edit engine) | 2 | [gui-framework.md](gui-framework.md) |
| Utils — geometry, GIS math, libtess2 | 67 | [utils-geometry.md](utils-geometry.md) |
| Utils — platform, files, asserts, zip | | [utils-platform-and-files.md](utils-platform-and-files.md) |
| Utils — bitmaps, textures, DDS | | [bitmap-texture-and-ddstool.md](bitmap-texture-and-ddstool.md) |
| Obj (OBJ8 read/write) | 9 | [utils-platform-and-files.md](utils-platform-and-files.md) |
| DSF (DSFLib) | 14 | [dsf-library-and-dsftool.md](dsf-library-and-dsftool.md) |
| Network (curl HTTP) | 9 | [wed-network-and-filecache.md](wed-network-and-filecache.md) |
| XESCore — only `AptIO`/`AptDefs`/`DEMDefs` as WED uses them | 8 | [wed-import-export.md](wed-import-export.md) (apt.dat) · [utils-geometry.md](utils-geometry.md) |

### Command-line tools

| Directory | Files | Article |
|---|---|---|
| DSFTools (DSFTool / DSF2Text) | 3 | [dsf-library-and-dsftool.md](dsf-library-and-dsftool.md) |
| XPTools — `DDSTool.cpp` | 5 | [bitmap-texture-and-ddstool.md](bitmap-texture-and-ddstool.md) |
| XPTools — ObjView, XGrinder | | built, but not covered yet |

### Out of scope (deliberately not covered)

| Directory | Why |
|---|---|
| XESCore (beyond the files above), RenderFarm, MeshTool | Not WED; see `src/README.txt` for naming history |
| OneOffs, ObjEdit, linuxinit | One-off / discontinued tools |
| lzma19, `SDK/` (incl. libtess2), `Utils/glew*` | Vendored — don't edit. Exception: libtess2 is locally modified, see [utils-geometry.md](utils-geometry.md) |

---

## Concept Index

### Design rules (read before writing or reviewing WED code)
- Command ownership, pointers vs IDs, concrete vs GIS-interface casts, mutate-then-roll-back exports, errors vs asserts, main-thread blocking, resource-handle lifetime, GUI/WED layering, per-document prefs, platform reference → [wed-design-principles.md](wed-design-principles.md)

### Undo, persistence, object model
- Command wrapping, `StartCommand` vs `StartOperation` → [wed-object-model.md](wed-object-model.md#things-that-will-bite-you)
- Raw pointers across undo / redo / revert → [wed-object-model.md](wed-object-model.md#raw-pointers-do-not-survive-undo-redo-or-revert)
- Registering a new persistent class (`DECLARE_PERSISTENT` / `DEFINE_PERSISTENT` / `REGISTER_LIST`) → [wed-object-model.md](wed-object-model.md#registering-a-new-persistent-class-three-places-or-loadundo-breaks)
- Entity bounds cache / `CacheKey` invalidation → [wed-object-model.md](wed-object-model.md#entity-caches-invalidation-is-edge-triggered)
- Change notification (`msg_ArchiveChanged`, `wed_Change_*`) → [wed-object-model.md](wed-object-model.md#change-notification)
- `earth.wed.xml` compatibility, `doc/xml_compatibility` → [wed-object-model.md](wed-object-model.md#format-compatibility-rules-non-obvious)
- Object IDs, IResolver paths → [wed-object-model.md](wed-object-model.md#ids)
- Document dirtiness → [wed-object-model.md](wed-object-model.md#dirtiness)
- Enum descriptions are a file format → [wed-core-services.md](wed-core-services.md#enum-descriptions-are-a-file-format-and-they-must-be-unique-within-a-domain) · [wed-entities.md](wed-entities.md#enum-system-invariants)
- What is persisted (property/XML names must never change) → [wed-entities.md](wed-entities.md#what-is-persisted-and-so-must-never-change)
- Writing `.value` directly bypasses undo → [wed-entities.md](wed-entities.md#writing-value-directly-bypasses-undo)
- `PROP_Name` bit-packing limits → [wed-entities.md](wed-entities.md#prop_name-bit-packing-limits-fail-silently-in-release-builds)
- GIS classes and child-structure invariants → [wed-entities.md](wed-entities.md#gis-classes-and-the-child-structure-each-one-assumes)
- Bounds-cache protocol → [wed-entities.md](wed-entities.md#bounds-cache-protocol-wed_entity) · [wed-object-model.md](wed-object-model.md#entity-caches-invalidation-is-edge-triggered)
- Airport / ATC hierarchy, runway identity by name → [wed-entities.md](wed-entities.md#airport-and-atc-structure)
- Adding an entity type or a property → [wed-entities.md](wed-entities.md#recipes)
- Property edits and undo in the property pane → [wed-ui-panes.md](wed-ui-panes.md#property-edits-are-wrapped-in-undo-explicitly-not-automatically)
- Tool undo during drags (`BeginEdit`/`EndEdit` vs `AcceptPath`) → [wed-map-and-tce.md](wed-map-and-tce.md#things-that-will-bite-you)

### Automation (MCP server)
- Launching WED for agents (`--mcp`, `--prefs`, `--xsystem`), tools, JSON dump/fixture schema → [wed-mcp.md](wed-mcp.md)
- Main-thread handoff and the "safe point"; one job at a time → [wed-mcp.md](wed-mcp.md#threading-and-the-safe-point)
- Headless modals (`gPlatformModalHooks`) → [wed-mcp.md](wed-mcp.md#modals-are-answered-automatically-wed_mcpheadless)
- Command names ↔ enums (keep `WED_MCPCommandNames.cpp` in sync) → [wed-mcp.md](wed-mcp.md#commands-and-documents)
- Save/reopen double precision, raw property items vs IPropertyObject → [wed-mcp.md](wed-mcp.md#the-json-schema-reads-raw-property-items-not-ipropertyobject)
- Screenshots, synthetic mouse/key input, window vs GL coordinates → [wed-mcp.md](wed-mcp.md#screenshots-and-synthetic-input-wed_mcptoolsmap)
- Headless validation/export, validation code names → [wed-mcp.md](wed-mcp.md#validation-and-export-wed_mcptoolsexport)

### Startup, documents, managers
- Startup / initialization order → [wed-core-services.md](wed-core-services.md#startup-order-wed_appmaincpp-main)
- Doc vs global prefs, `IDocPrefs` → [wed-core-services.md](wed-core-services.md#preferences-idocprefs-implemented-by-wed_document)
- Changing the X-Plane folder with documents open → [wed-core-services.md](wed-core-services.md#changing-the-x-plane-folder-with-documents-open-leaves-mixed-state)
- `library.txt` parsing, vpath status → [wed-core-services.md](wed-core-services.md#librarytxt-parsing-things-youd-expect-to-work-differently)
- Resource cache invalidation / purge → [wed-core-services.md](wed-core-services.md#wed_resourcemgr-never-gets-its-invalidation-messages)
- Library-change messages → [wed-core-services.md](wed-core-services.md#messages-wed_messagesh-and-who-listens)
- Document window destroy ordering → [wed-ui-panes.md](wed-ui-panes.md#destruction-order-panes-outlive-the-documents-managers)

### Export targets, validation, Gateway
- Export target (`gExportTarget`), `wet_gateway` ordering → [wed-import-export.md](wed-import-export.md#the-export-target-is-a-global-it-is-sticky-and-some-code-switches-it-for-you) · [wed-validation.md](wed-validation.md#wet_gateway-is-99-so-it-compares-greater-than-every-x-plane-version)
- Validation severity (error vs warning) → [wed-validation.md](wed-validation.md#severity-comes-from-where-the-code-sits-in-the-enum-not-from-a-flag)
- Validation blocking export → [wed-validation.md](wed-validation.md#truthy-means-no-errors-not-clean)
- Adding a validation check → [wed-validation.md](wed-validation.md#recipe-adding-a-new-check)
- Hidden objects skipped by validation → [wed-validation.md](wed-validation.md#hidden-objects-are-not-validated-and-the-collector-matches-exact-classes-only)
- Gateway upload pipeline → [wed-import-export.md](wed-import-export.md#architecture) · [wed-validation.md](wed-validation.md#gateway-upload-suppresses-the-dialog-the-user-only-gets-an-alert)
- Gateway export mutates then undoes (`MarkUndo`/`UndoToMark`) → [wed-import-export.md](wed-import-export.md#gateway-target-export-mutates-then-undoes)
- Airport metadata keys / defaults → [wed-import-export.md](wed-import-export.md#gateway)
- CIFP runway check (network-dependent) → [wed-validation.md](wed-validation.md#gateway-validation-depends-on-the-network-and-blocks-the-main-thread)

### File formats
- apt.dat version gating and precision (`XESCore/AptIO.cpp`) → [wed-import-export.md](wed-import-export.md#version-gating-and-precision-in-writeaptfileprocs-xescoreaptiocpp)
- apt.dat round-trip loss → [wed-import-export.md](wed-import-export.md#aptdat-round-trip-is-lossy-by-design--know-what-gets-normalized)
- DSF export tiling, clipping, stale-DSF deletion, parameter encodings → [wed-import-export.md](wed-import-export.md#dsf-export-tiling-clipping-and-the-safe-bounds)
- DSF point-pool precision / quantization → [dsf-library-and-dsftool.md](dsf-library-and-dsftool.md#precision-16-bit-quantization-truncates-and-scaleoffset-are-stored-as-float32)
- DSF polygon-pool first-fit precision loss → [dsf-library-and-dsftool.md](dsf-library-and-dsftool.md#polygon-pools-are-first-fit-so-one-big-polygon-can-wreck-precision-for-later-ones)
- DSF coordinate depth rules → [dsf-library-and-dsftool.md](dsf-library-and-dsftool.md#coordinate-depth-conventions-callers-must-know)
- "Could not sink" asserts → [dsf-library-and-dsftool.md](dsf-library-and-dsftool.md#pool-ranges-are-hard-limits-and-going-out-of-range-means-an-assert-not-an-error-code)
- 7z-compressed DSFs → [dsf-library-and-dsftool.md](dsf-library-and-dsftool.md#7z-compressed-dsfs-use_7z-set-in-objxdefsh)
- DSF2Text text format → [dsf-library-and-dsftool.md](dsf-library-and-dsftool.md#dsftool-and-the-dsf2text-format-srcdsftools)
- DSF bezier triple notation → [utils-geometry.md](utils-geometry.md#dsf-triple-bezier-notation-collapses-co-located-vertices-on-import)
- OBJ8 reading for previews → [utils-platform-and-files.md](utils-platform-and-files.md#obj-library-as-wed-uses-it)

### Geometry and coordinates
- Lon/lat degree space everywhere → [utils-geometry.md](utils-geometry.md#point2-is-lon-lat-in-degrees-and-every-predicate-works-in-raw-degree-space)
- Three earth models in use at once → [utils-geometry.md](utils-geometry.md#three-different-earth-models-are-in-use-at-once)
- Map projection and precision → [wed-map-and-tce.md](wed-map-and-tce.md#coordinate-systems)
- Polygon winding → [utils-geometry.md](utils-geometry.md#winding-convention-outer-ring-ccw-holes-cw-enforced-only-in-some-places)
- Bezier handles ("no handle == exactly 0") → [utils-geometry.md](utils-geometry.md#no-bezier-handle-means-exactly-zero)
- libtess2 is locally modified to `double` → [utils-geometry.md](utils-geometry.md#vendored-libtess2-was-changed-to-double-and-callers-depend-on-that-layout)
- CGAL was removed → [utils-geometry.md](utils-geometry.md#cgal-is-gone-but-comments-still-describe-it)
- Antimeridian / poles → [utils-geometry.md](utils-geometry.md#no-antimeridian-or-pole-handling-anywhere)

### Map, tools, UI
- Map layer caps and draw order → [wed-map-and-tce.md](wed-map-and-tce.md#layers-caps-and-draw-order)
- Handles / `WED_HandleToolBase` drag state machine → [wed-map-and-tce.md](wed-map-and-tce.md#handles-icontrolhandles-and-wed_handletoolbase)
- Adding a map tool → [wed-map-and-tce.md](wed-map-and-tce.md#recipe-add-a-map-tool)
- Texture coordinate editor → [wed-map-and-tce.md](wed-map-and-tce.md#wedtce-where-it-really-differs-from-wedmap)
- Menu command dispatch / enable state → [wed-ui-panes.md](wed-ui-panes.md#command-travel-os-menu--handler)
- Adding a menu command → [wed-ui-panes.md](wed-ui-panes.md#recipe-add-a-new-menu-command)
- Hard-coded submenu indices → [wed-ui-panes.md](wed-ui-panes.md#submenus-are-attached-by-hard-coded-item-index)
- Per-platform menus → [wed-ui-panes.md](wed-ui-panes.md#per-platform-menu-differences)
- Multi-selection in the property pane → [wed-ui-panes.md](wed-ui-panes.md#property-table-model-wed_propertytable)
- Commands deferred while the mouse is down → [gui-framework.md](gui-framework.md#commands-and-keys-are-deferred-while-any-mouse-button-is-down)
- Re-entrant event loops, timers firing inside handlers → [gui-framework.md](gui-framework.md#handlers-can-re-enter-the-event-loop-and-timers-fire-inside-them)
- Window close / destroy per platform → [gui-framework.md](gui-framework.md#closing-a-window-differs-per-platform)
- `GUI_Destroyable` double delete → [gui-framework.md](gui-framework.md#gui_destroyable-batch-hazard)
- Table cell edit lifetime → [gui-framework.md](gui-framework.md#table-editing-lifetime-gui_table--gui_texttable)
- OpenGL state assumptions → [gui-framework.md](gui-framework.md#opengl-state-is-barely-managed)
- HiDPI → [gui-framework.md](gui-framework.md#hidpi-and-scaling)

### Images and textures
- `ImageInfo` layout (bottom-up BGRA, padding) → [bitmap-texture-and-ddstool.md](bitmap-texture-and-ddstool.md#imageinfo-is-bottom-up-bgra-and-every-loader-converts-to-that)
- Image buffer ownership → [bitmap-texture-and-ddstool.md](bitmap-texture-and-ddstool.md#ownership-mallocfree-shallow-copies-and-functions-that-swap-out-data)
- DDS / BC writing → [bitmap-texture-and-ddstool.md](bitmap-texture-and-ddstool.md#dds-writing-writebitmaptodds_mt)
- Direct DDS/KTX2 GPU upload → [bitmap-texture-and-ddstool.md](bitmap-texture-and-ddstool.md#the-direct-ddsktx2-gpu-path-has-its-own-rules)
- sRGB, gamma, premultiplied alpha → [bitmap-texture-and-ddstool.md](bitmap-texture-and-ddstool.md#colour-space-and-alpha)
- GeoTIFF georeferencing for orthophotos → [bitmap-texture-and-ddstool.md](bitmap-texture-and-ddstool.md#geotiff-georeferencing-orthophoto-and-reference-image-import)
- DDSTool CLI → [bitmap-texture-and-ddstool.md](bitmap-texture-and-ddstool.md#ddstool-srcxptoolsddstoolcpp)

### Platform, files, networking
- Forced include `XDefs.h` and build flags → [utils-platform-and-files.md](utils-platform-and-files.md#build-flags-and-the-forced-include)
- UTF-8 paths on Windows → [utils-platform-and-files.md](utils-platform-and-files.md#paths-and-unicode-the-classic-bug-source)
- Case-insensitive lookup on Linux/macOS → [utils-platform-and-files.md](utils-platform-and-files.md#case-sensitivity)
- Memory-mapped file lifetime → [utils-platform-and-files.md](utils-platform-and-files.md#memfileutils-mapping-lifetime-termination)
- Asserts in DEV vs release → [utils-platform-and-files.md](utils-platform-and-files.md#asserts-what-they-compile-to)
- Which target compiles which Utils file → [utils-platform-and-files.md](utils-platform-and-files.md#which-target-actually-compiles-what)
- Network worker threads → [wed-network-and-filecache.md](wed-network-and-filecache.md#every-curl_http_get_file-is-its-own-os-thread-and-destroying-it-blocks)
- Cross-thread handoff (`volatile`, not atomics) → [wed-network-and-filecache.md](wed-network-and-filecache.md#cross-thread-handoff-is-volatile-int-not-atomics)
- Download cache, cool-down → [wed-network-and-filecache.md](wed-network-and-filecache.md#cache-identity-is-folder_prefix--url-basename)
- HTTP timeouts → [wed-network-and-filecache.md](wed-network-and-filecache.md#timeouts-60-s-to-connect-30-s-of-stall-and-no-total-limit)
- TLS / certificates → [wed-network-and-filecache.md](wed-network-and-filecache.md#tls--certificates)
- `WITHNWLINK` live mode (compiled out) → [wed-network-and-filecache.md](wed-network-and-filecache.md#nothing-in-srcwednetwork-except-raii_classes-is-compiled)

---

## Raw Sources

| File | Location | Informs |
|---|---|---|
| DSFLib README (2004-era notes) | [raw/dsf-library-readme.txt](raw/dsf-library-readme.txt) | [dsf-library-and-dsftool.md](dsf-library-and-dsftool.md) |
| DSF2Text user reference | `src/DSFTools/README.dsf2text` (in place — shipped by `scripts/bundle.sh`) | [dsf-library-and-dsftool.md](dsf-library-and-dsftool.md) |
| DDSTool user notes | `src/XPTools/README.DDSTool` (in place — shipped) | [bitmap-texture-and-ddstool.md](bitmap-texture-and-ddstool.md) |
| Ben's design-principle answers (2026-09-25) | [raw/ben-design-principles-2026-09-25.md](raw/ben-design-principles-2026-09-25.md) | [wed-design-principles.md](wed-design-principles.md) |
| Legacy source roadmap, `#define`s, naming history | `src/README.txt` (in place) | CLAUDE.md, [utils-platform-and-files.md](utils-platform-and-files.md) |
