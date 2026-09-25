# WED Architecture & Code Reference

A developer-onboarding guide to the WorldEditor (WED) source tree. Audience: a C++ developer
who is new to this specific codebase. The goal is to give you a *map* — where things live,
how subsystems fit together, and what to read first when you need to fix or extend something.

For build instructions see `Building.md`. For the user-facing manual see `src/WEDDocs/`.

---

## 1. What WED Is

WED (WorldEditor) is the GUI scenery / airport editor for X-Plane. It edits:

- **Airports** — runways, taxiways, taxi signs, beacons, windsocks, ATC flows/frequencies.
- **Overlays** — facade (building) placements, forest placements, line/string placements,
  polygon placements, draped orthophotos, exclusion zones, autogen placements.
- **Documents** persisted as XML (modern) or SQLite (legacy) via WED's own archive layer,
  with full undo/redo.

WED reads/writes `apt.dat`, X-Plane DSF scenery, and talks to the X-Plane Scenery Gateway.

The xptools tree also builds *other* tools (RenderFarm, MeshTool, DSF2Text, ObjView, etc.).
Most of WED's source lives in `src/WED*` directories; everything else is shared infrastructure
or unrelated tools.

---

## 2. Repository Top Level

```
xptools/
├── Building.md            Build instructions (read this first to get compiling)
├── README.md              Project overview & licensing
├── CMakeLists.txt         Top-level CMake build
├── cmake/                 CMake helpers
├── conanfile.py           Conan dependency manifest
├── SDK/                   X-Plane SDK headers
├── scripts/               Packaging / release scripts
├── test/                  Regression test fixtures for WED
└── src/                   All source. WED-specific dirs are prefixed WED*.
```

Inside `src/` (only WED-relevant entries shown here; see `src/README.txt` for the full list):

| Directory          | Role                                                              |
|--------------------|-------------------------------------------------------------------|
| `WEDCore/`         | Document, archive, undo, validation, library/resource/texture mgrs |
| `WEDEntities/`     | Concrete entity classes (airport, runway, taxiway, placements…)   |
| `WEDMap/`          | 2D map view, layers, interactive tools                            |
| `WEDProperties/`   | Property table UI                                                  |
| `WEDWindows/`      | Top-level windows and dialogs                                      |
| `WEDTCE/`          | Texture coordinate editor (UV editor for orthophotos)             |
| `WEDLibrary/`      | Library / asset browser pane                                       |
| `WEDImportExport/` | apt.dat, DSF, gateway, scenery-pack import/export                  |
| `WEDFileCache/`    | Disk cache for downloaded assets                                   |
| `WEDNetwork/`      | Gateway client / live-collab server                                |
| `WEDResources/`    | Icons, fonts, splash, line/pavement art                            |
| `WEDDocs/`         | Source for the user manual (markdown → HTML)                       |
| `Interfaces/`      | Abstract interfaces (`I*.h`) — the contracts everything talks to   |
| `GUI/`             | Cross-platform widget framework                                    |
| `Utils/`           | Geometry, math, file utilities                                     |
| `XESCore/`         | GIS engine — used by RenderFarm; little overlap with WED            |
| `DSF/`, `Obj/`     | DSF and OBJ format read/write libraries                             |

> **Naming gotcha** — the directory called `WorldEditor` (if you find references to it) and
> classes named `GISTool_*` are usually for *RenderFarm*, not WED. Old vocabulary.
> See `src/README.txt` for context.

---

## 3. Architectural Overview — The Five Big Ideas

If you understand these five patterns you can navigate the rest of WED:

### 3.1 Persistent object hierarchy: `WED_Thing`

Everything in a WED document is a `WED_Thing` — runways, taxiways, even the document
root and selection marker. `WED_Thing` provides:

- **Hierarchy** — parent + indexed children. The document is a tree of `WED_Thing`s.
- **Properties** — typed, named, introspectable, editable. Drives the property UI.
- **Sources / viewers** — observer pattern for change notification.
- **Persistence** — every property change is captured by the archive for undo/redo.

`WED_Entity : WED_Thing` adds spatial concerns: cached bounds, locked/hidden flags, and
implements the GIS interfaces.

### 3.2 The GIS abstraction: `IGISEntity`

The map view, selection, validation, geometric tools, etc. don't know about `WED_Runway`
or `WED_Taxiway` — they talk to `IGISEntity`, `IGISPoint`, `IGISPolygon`, etc.
`GISClass_t` (in `Interfaces/IGIS.h`) enumerates the kinds: point, point with heading,
line, ring, polygon, composite, …

This is *the* layer that decouples spatial algorithms from the airport-domain model.
Whenever you see code iterating `GetGISClass()` and switching, you're using it.

### 3.3 Properties + reflection: `IPropertyObject` / `WED_PropertyHelper`

Every editable object exposes its fields by name and type through `IPropertyObject`.
Concrete entities use `WED_PropertyHelper` and `WED_DEFINE_PROP_*` macros to declare
properties; the same machinery serves UI editing, multi-select merging, XML I/O, and
undo capture. **You don't write custom serialization for new entities — declare the
properties and the framework does the rest.**

### 3.4 Undo: archive + `WED_UndoMgr`

All mutations happen inside a command:

```cpp
doc->StartCommand("Move runway");
runway->SetLocation(...);          // captured automatically
doc->CommitCommand();               // or AbortCommand() to roll back
```

`WED_UndoMgr` records the diff via `WED_UndoLayer`. There is no manual "remember the old
value" code in normal entity logic — the archive snapshots property changes for you.
Forgetting to wrap a mutation in `StartCommand/Commit` will not crash, but the change
will not be undoable and may not persist.

### 3.5 The resolver pattern: paths instead of pointers

UI code that survives across undo/redo can't hold raw `WED_Thing*` pointers (they may
be deleted and re-created). Instead, panes ask the document (an `IResolver`) to
resolve a string path like `"world.airport[2].runway[0]"`. This is also how the
property pane and selection survive document edits.

---

## 4. The `Interfaces/` Directory — Read This First

Almost every cross-cutting boundary in WED runs through one of these abstract interfaces.
Knowing them is non-negotiable.

| Header               | Purpose                                                                            |
|----------------------|------------------------------------------------------------------------------------|
| `IBase.h`            | Root interface; ref-counting (`AddRef`/`Release`).                                  |
| `IGIS.h`             | `GISClass_t`, `IGISEntity`, `IGISPoint`, `IGISPoint_Bezier`, `IGISPoint_Heading`, `IGISQuad`. The spatial abstraction. |
| `IPropertyObject.h`  | `PropertyInfo_t`, `PropertyVal_t`, named/typed property access.                     |
| `IResolver.h`        | Resolve a path string to an `IBase*`. Implemented by `WED_Document`.               |
| `IArray.h`           | `Count`/`GetNth` — generic indexed iteration over children.                         |
| `IDirectory.h`       | Lookup-child-by-name.                                                               |
| `ISelection.h`       | Selection state: `IsSelected`, `Iterate`, etc.                                      |
| `IDocPrefs.h`        | Read/write doc-scoped or global prefs (int/double/string/int-set).                  |
| `ILibrarian.h`       | Library asset path/type/status lookup. Implemented via `WED_LibraryMgr`.            |
| `ITexMgr.h`          | OpenGL texture caching: `GetTexture`, `ReleaseTexture`.                             |
| `IHasResource.h`     | Marker — "this object references a library resource".                               |
| `IOperation.h`       | Nested undo: `__StartOperation` / `CommitOperation` / `AbortOperation`.             |
| `IControlHandles.h`  | Bezier control handle accessors for curved geometry.                                |

If you're touching the map, validation, or property pane, expect to be working through
`IGISEntity` and `IPropertyObject`, not concrete classes.

---

## 5. WEDCore — Document, Archive, Undo, Managers

Where the document model lives. Read `WED_Thing.h` and `WED_Document.h` first.

| Class / File           | What it does                                                            |
|------------------------|-------------------------------------------------------------------------|
| `WED_Thing`            | Root persistent object. Hierarchy, properties, observers, XML I/O.       |
| `WED_Entity`           | Spatial subclass; bounds caching, lock/hide, GIS interface plumbing.     |
| `WED_Persistent`       | Abstract `ReadFrom` / `WriteTo` / `ToXML` / `FromXML` contract.          |
| `WED_PropertyHelper`   | Mixin that implements `IPropertyObject` from declared properties.        |
| `WED_Document`         | The document. Owns archive, undo mgr, library mgr, resource mgr, prefs. Implements `IResolver`, `ILibrarian`, `IDocPrefs`. |
| `WED_Archive`          | Persistent storage layer. SQLite-backed; serializes things via `IOReader`/`IOWriter`. |
| `WED_UndoMgr`          | Undo/redo stack. `StartCommand` / `CommitCommand` / `AbortCommand`.      |
| `WED_UndoLayer`        | One undoable transaction's worth of property/hierarchy diffs.            |
| `WED_XMLReader/Writer` | Streaming XML I/O for documents.                                         |
| `WED_Application`      | App singleton; main window, doc lifecycle, file open/save.               |
| `WED_LibraryMgr`       | Resolves virtual library paths (objects, facades, forests, lines…) to disk and tracks asset status (public / deprecated / private). |
| `WED_ResourceMgr`      | Caches loaded `.obj` / `.fac` / `.for` / `.pol` definitions.             |
| `WED_TexMgr`           | Caches OpenGL textures (PNG/JPG via `BitmapUtils`).                      |
| `WED_Validate`         | Validation entry point; produces `WED_ValidateList` of issues.           |

**Where to start:** `WED_Thing.h` → `WED_Entity.h` → `WED_Document.h` → `WED_UndoMgr.h`.

---

## 6. WEDEntities — The Class Hierarchy

This is the domain model. Each class corresponds to something a user can place
on the map and that maps onto an apt.dat or DSF concept.

### 6.1 GIS shape bases (used as parents by concrete entities)

| Class                              | Shape                                                |
|------------------------------------|------------------------------------------------------|
| `WED_GISPoint`                     | Single position.                                     |
| `WED_GISPoint_Heading`             | Position + heading (towers, signs, windsocks).       |
| `WED_GISPoint_HeadingWidthLength`  | Position + heading + width + length (runway endpoints). |
| `WED_GISChain`                     | Sequenced polyline of child points.                  |
| `WED_GISLine_Width`                | Polyline with variable pavement width.               |
| `WED_GISEdge`                      | Edge between two endpoints (taxi routes, ATC nets).  |
| `WED_GISRing`                      | Closed ring (polygon boundary).                      |
| `WED_GISPolygon`                   | Polygon = outer ring + holes.                         |
| `WED_GISComposite`                 | Container of arbitrary `IGISEntity` children.        |

### 6.2 Airport furniture & geometry

| Class                       | Represents                                            |
|-----------------------------|-------------------------------------------------------|
| `WED_Airport`               | Top-level airport. ICAO, type, scenery ID, metadata kv pairs. Holds runways, taxiways, ATC, signs, etc. |
| `WED_Runway` + `WED_RunwayNode` | Runway (two endpoint nodes).                       |
| `WED_Taxiway`               | Pavement polygon with surface/markings/lighting.      |
| `WED_AirportChain`          | Linear features (markings, lights). Base for chained airport features. |
| `WED_AirportSign`           | Taxiway sign (position, heading, label string).       |
| `WED_AirportBeacon`         | Rotating beacon.                                      |
| `WED_Windsock`              | Windsock (lit flag).                                   |
| `WED_Helipad`               | Helipad.                                               |
| `WED_LightFixture`          | Light fixture (PAPI, VASI, etc.).                      |
| `WED_TowerViewpoint`        | ATC tower viewpoint.                                   |
| `WED_TruckDestination`, `WED_TruckParkingLocation` | Ground vehicle furniture.       |
| `WED_RampPosition`          | Aircraft parking spot.                                 |

### 6.3 ATC

| Class                                                                    | Role                          |
|--------------------------------------------------------------------------|-------------------------------|
| `WED_ATCFlow`                                                            | Named flow — composite of rules and runway uses. |
| `WED_ATCRunwayUse`                                                       | Per-runway use rule within a flow. |
| `WED_ATCWindRule`, `WED_ATCTimeRule`                                     | Wind / time predicates for flows. |
| `WED_ATCFrequency`                                                       | Frequency assignment.          |
| `WED_TaxiRouteNode`, `WED_TaxiRoute`                                     | Taxi route graph (nodes + edges). |

### 6.4 Overlay placements

| Class                       | Represents                                            |
|-----------------------------|-------------------------------------------------------|
| `WED_FacadePlacement`       | Building / facade. Resource path + ring of points.    |
| `WED_ForestPlacement`       | Forest area.                                          |
| `WED_StringPlacement`       | Linear string (fence, lights along a path).           |
| `WED_LinePlacement`         | Painted line.                                          |
| `WED_PolygonPlacement`      | Generic polygon overlay.                               |
| `WED_DrapedOrthophoto`      | Image overlay with explicit UVs (edited via TCE).      |
| `WED_AutogenPlacement`      | Autogen pack placement.                                |
| `WED_ObjPlacement`          | Single OBJ instance.                                   |
| `WED_ExclusionZone`         | Suppresses autogen / specific resource types in a box. |

### 6.5 Other

| Class                       | Role                                                  |
|-----------------------------|-------------------------------------------------------|
| `WED_Group`                 | User-created grouping container.                       |
| `WED_Select`                | Current selection. Lives in the document tree like everything else; implements `ISelection`. |
| `WED_Root`                  | Document root container.                               |

**Where to start:** open `WED_Airport.h` and trace down to `WED_Runway.h` to see how a
domain class is composed from GIS bases plus declared properties.

---

## 7. WEDMap — 2D Map View, Layers, Tools

Two parallel hierarchies: **layers** render, **tools** edit. Both derive from `WED_MapLayer`
(tools are layers that also handle input).

### 7.1 Core dispatch

| Class                | Role                                                                |
|----------------------|---------------------------------------------------------------------|
| `WED_Map`            | The map pane. Owns layers + active tool, dispatches draw and input. |
| `WED_MapPane`        | Wraps `WED_Map` with toolbar, tool buttons, preview pane.            |
| `WED_MapZoomerNew`   | Zoom/pan state and screen↔world coordinate transforms.               |
| `WED_MapBkgnd`       | Background layer (orthophoto / elevation tiles).                     |

### 7.2 Layers (rendering)

`WED_MapLayer::GetCaps` advertises whether a layer draws structure (handles, outlines),
visualization (filled shapes, icons), or selection. `WED_Map` walks the entity tree
and calls `DrawEntityVisualization` / `DrawEntityStructure` per layer per entity.

| Class                | Renders                                |
|----------------------|----------------------------------------|
| `WED_StructureLayer` | Vertices, edges, control handles.       |
| `WED_PreviewLayer`   | Photoreal preview of placements.        |
| `WED_ATCLayer`       | ATC taxi routes / flow visualization.   |
| `WED_BoundaryLayer`  | Airport boundaries.                     |
| `WED_DebugLayer`     | Bounding boxes, etc.                    |

### 7.3 Tools (input)

| Class                       | What it does                                  |
|-----------------------------|-----------------------------------------------|
| `WED_MapToolNew`            | Abstract base. Click/drag/key handlers, status text, undo. |
| `WED_HandleToolBase`        | Base for tools that drag existing vertices/handles.        |
| `WED_CreatePointTool`       | Single-click point creation.                  |
| `WED_CreateLineTool`        | Multi-click polyline.                          |
| `WED_CreatePolygonTool`     | Closed polygon.                                |
| `WED_CreateBoxTool`         | Click-drag rectangle / quad.                   |
| `WED_CreateEdgeTool`        | Edge in a graph (taxi network).                |
| `WED_VertexTool`            | Drag vertices / Bezier handles.                |
| `WED_MarqueeTool`           | Rectangle multi-select.                        |

### 7.4 Drawing utilities

`WED_DrawUtils.h` (icons, lines, text) and `WED_Colors.h` (palette).

**Where to start:** `WED_Map.h` → `WED_MapLayer.h` → `WED_MapToolNew.h`. To learn the
*rendering* pipeline, read `WED_StructureLayer`. To learn *editing*, read
`WED_VertexTool` and `WED_HandleToolBase`.

---

## 8. WEDProperties — Property Table UI

| Class                | Role                                                                 |
|----------------------|----------------------------------------------------------------------|
| `WED_PropertyPane`   | Pane wrapping table + filter; modes: hierarchical / filtered / by-selection. |
| `WED_PropertyTable`  | Adapter from `IPropertyObject(s)` to a generic `GUI_TextTable`. Multi-select merge logic lives here. |

The pane reads selection from the document and walks `IPropertyObject` to populate cells.
Edits are written back through the same interface, capturing undo automatically.

---

## 9. WEDWindows — Top-Level Windows & Dialogs

| Class                  | Role                                                              |
|------------------------|-------------------------------------------------------------------|
| `WED_DocumentWindow`   | Main editing window. Splits map / properties / library / TCE.     |
| `WED_StartWindow`      | Launcher: recent files, new/open buttons.                          |
| `WED_AboutBox`         | About dialog.                                                      |
| `WED_Sign_Editor`      | Edits taxi sign text + style.                                      |
| `WED_Line_Selector`    | Picks a taxi/road line type.                                       |
| `WED_Road_Selector`    | Picks a road type (when road editing is on).                       |
| `WED_Menus`            | Menu / command definitions and dispatch.                           |

**Where to start:** `WED_DocumentWindow.h` is the layout assembly point — reading it
shows how the panes wire together.

---

## 10. WEDTCE — Texture Coordinate Editor

A second 2D canvas, mirroring `WED_Map`'s design, but used for editing UVs on
draped orthophotos.

| Class                | Role                                                  |
|----------------------|-------------------------------------------------------|
| `WED_TCEPane`        | Pane container; analogue of `WED_MapPane`.            |
| `WED_TCE`            | Canvas; analogue of `WED_Map`.                        |
| `WED_TCELayer`       | Layer base.                                            |
| `WED_TCEToolNew`     | Tool base.                                             |
| `WED_TCEVertexTool`  | Drag corner UVs.                                       |
| `WED_TCEMarqueeTool` | Marquee select UVs.                                    |
| `WED_TCEToolAdapter` | Bridges TCE selection to the property pane.            |

If you understood `WEDMap`, you've understood `WEDTCE`.

---

## 11. WEDImportExport — apt.dat, DSF, Gateway

This is where WED talks to the outside world.

| File                       | Role                                                    |
|----------------------------|---------------------------------------------------------|
| `WED_AptIE.{h,cpp}`        | apt.dat import/export. `WED_AptImport`, `WED_AptExport`, plus UI entry points. |
| `WED_DSFImport.{h,cpp}`    | Reads a DSF and creates placement objects.               |
| `WED_DSFExport.{h,cpp}`    | Writes WED objects out as DSF.                           |
| `WED_GatewayImport.{h,cpp}`| Downloads scenery from the X-Plane Gateway.              |
| `WED_GatewayExport.{h,cpp}`| Uploads / submits scenery; conflict detection.            |
| `WED_SceneryImport.{h,cpp}`| Generic X-Plane scenery package import.                  |
| `WED_SceneryPackExport.{h,cpp}` | Standard scenery pack export.                       |
| `WED_OrthoExport.{h,cpp}`  | Orthophoto-specific export.                              |
| `WED_AptTable`             | Airport-list table model used by the import dialog.       |
| `WED_MetaDataKeys`, `WED_MetaDataDefaults` | Airport metadata field definitions and defaults. |
| `WED_ICAOTable`            | ICAO airport code lookup.                                |

Apt.dat parsing relies on the `AptDefs.h` structures (in `Utils/`); WED translates
between those and the `WED_Airport` tree.

---

## 12. WEDLibrary, WEDFileCache, WEDNetwork

### WEDLibrary

| Class                       | Role                                                |
|-----------------------------|-----------------------------------------------------|
| `WED_LibraryPane`           | Library browser pane embedded in the document window.|
| `WED_LibraryListAdapter`    | Tree/list table adapter over `WED_LibraryMgr`.       |
| `WED_LibraryPreviewPane`    | Thumbnail / 3D preview.                              |
| `WED_LibraryFilterBar`      | Search input.                                         |

The actual asset discovery / path resolution lives in `WED_LibraryMgr` (in WEDCore).

### WEDFileCache

| Class                  | Role                                                   |
|------------------------|--------------------------------------------------------|
| `WED_FileCache`        | Public API. `request_file()` returns a status/path response. |
| `CACHE_CacheObject`    | Per-file cache entry (download state, error cool-down). |
| `CACHE_DomainPolicy`   | Per-domain age, cool-down, and bandwidth rules.         |

### WEDNetwork

| Class                  | Role                                                   |
|------------------------|--------------------------------------------------------|
| `WED_Server`           | TCP server lifecycle + send/receive.                    |
| `WED_Connection`       | Per-client connection state.                            |
| `WED_NWLinkAdapter`    | Bridges document changes to network sync.               |
| `WED_NWInfoLayer`      | Map layer for showing network status / conflicts.       |
| `WED_NWDefs`           | Protocol message definitions.                           |

---

## 13. WEDResources

Static art and metadata: airport icons, parking-spot icons, line-marking textures,
pavement textures, vertex handle graphics, scrollbar/splitter/tab assets, the WED
icon, the Mac menu nib, fonts, the splash worldmap. Loaded via `GUI_Resources.h`.

---

## 14. The GUI Framework — Just Enough to Get Going

WED's UI is its own portable widget toolkit. All panes derive from `GUI_Pane`. Read
the comments at the top of `GUI_Pane.h` for the full theory of operation.

| Class                  | Role                                                   |
|------------------------|--------------------------------------------------------|
| `GUI_Pane`             | Base widget. Drawing, mouse/keyboard, sticky-edge layout. |
| `GUI_Window`           | Top-level window.                                       |
| `GUI_Application`      | App singleton, event loop, modal dialogs.               |
| `GUI_Broadcaster` / `GUI_Listener` | Pub/sub. Widgets `BroadcastMessage(msg, param)`; listeners override `ReceiveMessage`. |
| `GUI_Commander`        | Menu / keyboard command routing along the pane chain.    |
| `GUI_Control`          | Abstract value-bearing control (slider-style).           |
| `GUI_Button`, `GUI_TextField`, `GUI_Label`, `GUI_ScrollBar`, `GUI_PopupButton`, `GUI_TabControl`, `GUI_FilterBar` | Standard controls. |
| `GUI_Table`, `GUI_TextTable`, `GUI_Header` | Generic grid / two-column tables. |
| `GUI_ScrollerPane`, `GUI_Splitter`, `GUI_Packer` | Layout containers.    |
| `GUI_Destroyable`      | Mixin: defer `delete` until safe (e.g. event handler exit). |
| `GUI_DrawUtils`, `GUI_GraphState`, `GUI_Fonts` | Drawing primitives.  |

The two messaging patterns — **Broadcaster/Listener** (data changes) and **Commander**
(menu/keys) — are everywhere. Understanding them is the price of admission.

---

## 15. How-To Recipes

### 15.1 Add a new entity type

1. Create `WED_NewThing.{h,cpp}` in `WEDEntities/`, deriving from the appropriate GIS
   base (`WED_GISPoint`, `WED_GISPolygon`, …).
2. Declare persistent properties using `WED_PROPERTY_*` macros (see existing entities
   like `WED_AirportSign` for the pattern). The framework handles XML I/O, undo, and
   the property pane automatically.
3. Override `GetGISClass()` if needed.
4. Register the class in `WED_Entity.cpp`'s factory table so the archive can construct it.
5. Add import/export handling — usually a new clause in `WED_AptIE.cpp` or
   `WED_DSFImport/Export.cpp`.
6. Optional: add icon assets in `WEDResources/` and a render branch in the appropriate
   map layer if it needs custom drawing.

### 15.2 Add a map tool

1. Create `WED_MyTool.{h,cpp}` in `WEDMap/`, deriving from `WED_MapToolNew` (or
   `WED_HandleToolBase` if it manipulates existing handles).
2. Implement `HandleClickDown/Drag/Up`, `HandleToolKeyPress`, `DrawVisualization`,
   and the `GetStatusText` / `GetCaps` overrides.
3. Wrap mutations in `StartCommand` / `CommitCommand` (or `__StartOperation` for
   nested operations).
4. Register the tool in `WED_MapPane` or wherever the tool palette is built.

### 15.3 Add a property to an existing entity

1. Add a `WED_PROPERTY_*` declaration in the entity's header.
2. Initialize it in the constructor.
3. The property table, undo, and XML I/O pick it up automatically.

### 15.4 Add a validation check

Open `WED_Validate.cpp` and follow the existing pattern of walking the document and
emitting `WED_ValidateError` records into the issue list.

### 15.5 Trace a bug starting from a UI symptom

- **Map misbehavior:** start in the active layer's `DrawEntity*` or the active tool's
  click handlers.
- **Property pane wrong:** `WED_PropertyTable` — confirm what `IPropertyObject` is
  reporting for the selected object(s).
- **Save / load wrong:** `WED_XMLReader/Writer` for XML; `WED_Archive` for the
  legacy SQLite path.
- **Apt.dat wrong:** `WED_AptIE.cpp`.
- **Undo wrong:** confirm the mutation is wrapped in `StartCommand`/`CommitCommand`
  *and* goes through the property system rather than back-door pointer mutations.

---

## 16. Reading Order Recommendation

If you have an afternoon to spend reading code before touching anything, this is the
order with the highest payoff:

1. `src/README.txt` (legacy — context, naming history)
2. `Building.md` (get a build going)
3. `Interfaces/IGIS.h`, `Interfaces/IPropertyObject.h`, `Interfaces/IResolver.h`
4. `WEDCore/WED_Thing.h`, `WED_Entity.h`, `WED_Document.h`, `WED_UndoMgr.h`
5. `WEDEntities/WED_Airport.h`, `WED_Runway.h` — concrete examples of the patterns
6. `WEDMap/WED_Map.h`, `WED_MapLayer.h`, `WED_MapToolNew.h`
7. `WEDProperties/WED_PropertyPane.h`
8. `WEDWindows/WED_DocumentWindow.h` — see how it all snaps together
9. `WEDImportExport/WED_AptIE.h` — the most-touched I/O surface

Once those are familiar, the rest of the tree reads itself.

---

## 17. Conventions & Gotchas

- **Coordinate systems:** WED uses lat/lon in degrees for storage. The map converts to
  screen pixels via `WED_MapZoomerNew`. Beware of code that assumes meters.
- **Bounds caching:** `WED_Entity` caches its bounding box. If you mutate geometry
  outside the property system, you may need to call the appropriate dirty-flag method.
- **Pointers vs. paths:** Don't cache `WED_Thing*` across operations that might delete
  and re-create the object (undo, reload). Use the resolver path.
- **Undo wrapping:** Every user-visible mutation must live inside a command block.
  Forgetting this produces silent loss-of-undo, not a crash.
- **GUI threading:** the GUI framework is single-threaded. File downloads (cache,
  gateway) marshal back to the main thread for UI updates.
- **Two persistence formats:** modern documents are XML; older code paths still touch
  SQLite via `WED_Archive`. Don't rip out the SQLite path without checking what
  still uses it.

---

## 18. Where This Doc Stops

- The `XESCore` / RenderFarm / DSF GIS engine is barely covered — WED uses very little
  of it directly.
- The `GUI` framework section is intentionally shallow; if you're doing serious widget
  work, read `GUI_Pane.h` end-to-end.
- Format-level details of apt.dat, DSF, OBJ are out of scope — see `Utils/AptDefs.h`,
  `DSF/`, and `Obj/` respectively.

When something here goes stale, fix it — this file is meant to be edited as you learn.
