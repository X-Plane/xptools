# WED Map View and Texture Coordinate Editor

> Source: `src/WEDMap/`, `src/WEDTCE/` (plus `src/WEDCore/WED_TCEDebugLayer.*`, which is built as part of the TCE)

## Things That Will Bite You

**Undo and the tool state machine**

- **Every model mutation from a tool must sit inside an open command.** `WED_Archive::ChangedObject`/`AddObject`/`RemoveObject` hit `DebugAssert("object changed outside of a command")` when there is no undo layer. `WED_UndoMgr::__StartCommand` asserts (and calls the panic handler, if one is set) when a command is started while another is still open. So a tool has to keep its Start/Commit calls balanced on every path: mouse-up, tool switch (`KillOperation`) and menu command mid-drag (`PreCommandNotification`).
- **Selection is persistent state.** `WED_Select` is a `WED_Persistent`, so `sel->Clear()/Insert()/Toggle()` are undoable edits and need an open command. Tools start that command through the selection's `IOperation` (`SAFE_CAST(IOperation, WED_GetSelect(resolver))->StartOperation(...)`). This works because `WED_Thing::__StartOperation` forwards to the whole archive.
- **Two undo styles are in use, and they must not be mixed within one gesture.**
  - Editing tools (`WED_VertexTool`, `WED_MarqueeTool`, the TCE tools) start an operation in `IControlHandles::BeginEdit()` and commit it in `EndEdit()`. `WED_HandleToolBase` then calls `ControlsHandlesBy`/`ControlsLinksBy`/`ControlsMoveBy` once per mouse-drag event, all inside that one operation. A drag is one undo step.
  - Create tools (`WED_CreateToolBase` subclasses) keep the half-built shape in tool-local vectors (`mPts`, `mControlLo/Hi`, …) and touch no model state until `AcceptPath()`. `AcceptPath()` does `GetArchive()->StartCommand` … `CommitCommand` in one piece. `WED_CreateToolBase::BeginEdit/EndEdit` are deliberately empty.
- **Switching tools commits a pending shape rather than discarding it.** `WED_Map::SetTool` calls the old tool's `KillOperation`. `WED_CreateToolBase::KillOperation` calls `DoEmit` when at least `mMinPts` points exist. The `KillOperation` comment says "ABORT!", but the code does not do that for create tools.
- **Known gaps in `WED_HandleToolBase`.** `KillOperation` and `PreCommandNotification` close only `drag_Sel` (the selection op) and `drag_Handles/Links/Ent/PreEnt` (`EndEdit`). `drag_PreMove`/`drag_Move` open a "Drag" or "Copy" op in `HandleClickDown`, and neither function closes it. `mClickLayer` in `WED_Map` still points at the old tool, but that tool's `mDragType` is now `drag_None`, so the later mouse-up does nothing and the op stays open. The normal key and menu paths can't trigger this: while a button is down the app-root commander defers every command and key until after `MouseUp` ([gui-framework.md](gui-framework.md#commands-and-keys-are-deferred-while-any-mouse-button-is-down)). [Needs Runtime: whether any path calls `SetTool`/`KillOperation` or dispatches a command without going through that defer during a drag-move. If one does, the next command will assert.] The history shows this state machine has been patched for unbalanced ops before (`498487808` added the missing `AbortOperation` for `drag_PreMove`, and `789662738` added `PreCommandNotification`).
- **Alt-drag duplicates inside an already-open op.** `HandleClickDown` calls `StartOperation("Copy")` and then `WED_DoDuplicate(resolver, false)`. The `false` is `wrap_in_cmd`, so the duplicate runs inside the op that is already open. It then rebuilds `mSelManip` from the new selection. Moving an airport pops a modal `ConfirmMessage` from inside the mouse-up handler (in `HandleToolBase::HandleClickUp` and in `WED_MarqueeTool::EndEdit`), and the choice there commits or aborts the op.

**Walking the entity tree and filters**

- **Hide/lock filter specs are compared by `const char *` pointer, not by string.** `matches_filter` in `WED_MapLayer.cpp` compares `GetClass()` against `FilterSpec` entries with `==`. Always build specs from `WED_Xxx::sClass`, never from a string literal. A multi-element spec matches the entity, then its parent, then its grandparent, and so on. `WED_Airport` is rewritten to `WED_Group` during matching, so specs must say `WED_Group::sClass` to match things under an airport (see the comment above `k_show_taxiline_chain` in `WED_MapPane.cpp`). A spec whose last element would need to match a root with no parent never matches.
- **Filters are stored by pointer.** `WED_Map::AddLayer` gives every layer pointers to the map's own `mHideFilter`/`mLockFilter` ("client MUST retain storage"). A layer that is not added to a `WED_Map` has no filters at all. The 3D preview's private `WED_PreviewLayer` is one example: tab filter modes do not apply in the 3D window.
- **The walk skips any `IGISEntity` that is not a `WED_Entity`** (`DrawVisFor`/`DrawStrFor` do a `dynamic_cast<WED_Entity*>` and return on failure). Hidden is checked per node with `GetHidden()`. Pruning at a hidden parent is what makes hiding "recursive". Lock accumulates down the structure pass (`what_locked |= IsLocked(ent)`). Hit-testing and handle building use `IsLockedNow`, which calls `GetLockedRecursive()` instead.
- **A layer's `DrawEntity*` return value means "descend into my children."** Descent only happens for `gis_Composite`, and only when the composite is bigger than `TOO_SMALL_TO_GO_IN` pixels, is a single point, or sits at depth 0. `WED_StructureLayer` returns `false` for polygons and recurses into their rings itself, so the map's filter checks do not run on rings and nodes it draws that way.
- **Children are visited last-to-first.** Both walks run `for (n = t-1; n >= 0; --n)`, so child 0 (top of the hierarchy list) is drawn last and ends up on top. `WED_PreviewLayer` relies on this: its per-draw counters `mRunwayLayer++`/`mTaxiLayer++` turn visit order into layer order. It then uses `std::sort`, which is not stable, so items with equal layer numbers (everything in `group_Objects`) have no defined order.
- **Tab filter modes are tied to tab indices.** `WED_MapPane::ReceiveMessage` treats every message not sent by its toolbar as a tab change and passes `inParam` straight to `SetTabFilterMode`. The `tab_*` enum in `WED_MapPane.cpp` ("Must be kept in sync with TabPane") must match the `prop_tabs->AddPane` order in `WED_DocumentWindow`. Adding or reordering a property tab silently breaks the filter modes.

**Tool palette registration uses parallel arrays**

- **`mTools` index, `map_tools.png` cell, and `kToolKeys` must stay in step.** `GUI_ToolBar` numbers cell `n` as `x + y*2`, with row 0 at the bottom. `map_tools.png` is two copies of the grid side by side: the left half is normal and the right half is selected. `TOOLICON_ROWS`, the `Assert(mTools.size() == 2 * TOOLICON_ROWS)`, and the `kToolKeys[2*TOOLICON_ROWS]` table (two keys per row) all have to agree. The mapping was broken once already (`55835ef6a`, "fix tools shortcut key assignment"). A `NULL` entry makes a disabled placeholder cell.
- **Tool names are preference keys.** `WED_MapPane::ToPrefs/FromPrefs` save every tool property as `map_<ToolName>_<prop_name>`. Renaming a tool or a tool property drops users' saved settings. `FromPrefs` parses `prop_Double` with `atoi`, so saved fractional tool values come back truncated.
- **`WED_Map::Draw` dereferences `mTool` unconditionally** when it draws the scale bar (`cur->mZoomer->GetPPM()`). `WED_MapPane` passes `SetTool(mTools[0])` (NULL), and the following `mToolbar->SetValue(mTools.size()-2)` broadcasts and selects the Vertex tool before the first draw. Do not leave the map without a tool.

**Coordinates and precision**

- **`XPixelToLon`/`YPixelToLat`/`LonToXPixel`/`LatToYPixel` are linear and ignore the projection.** Use them only for small deltas and slop radii, which is how `WED_HandleToolBase` and `WED_CreateEdgeTool` use them. For absolute positions, use `PixelToLL`/`LLToPixel`. When zoomed in, the map is gnomonic; far out it blends into a Wagner-like projection. The header warns about this. `WED_NWInfoLayer` (compiled only with `WITHNWLINK`, off by default) breaks the rule.
- **Anything drawn in meter space must add `zoomer->GetRotation(loc)` to its heading.** In gnomonic mode, north is not straight up away from the map centre. `WED_PreviewLayer` adds it for objects and `.agp`s (`9c7119c4f`, `ed27e8f69`).
- **`WED_MapZoomerNew` doubles as the transform stack for the 3D preview.** Code shared between the map and the 3D window (preview items, `WED_FacadePreview`, `draw_agp_at_ll`) must use `zoomer->PushMatrix/Translatef/Rotatef/Scalef/PopMatrix` for transforms made relative to the zoomer. Those calls forward to `WED_PerspectiveCamera`, which keeps its own copy of the matrix for culling and `PixelSize`. A raw `glRotatef` in between makes the copies disagree, and DEV builds assert in `ModelViewMatrixConsistent` (`06cf1acd6` fixed one such bug). Raw GL is safe only fully nested inside a raw `glPushMatrix/glPopMatrix`, as `draw_obj_at_xyz` does.
- **Texture-generation origins must stay near the view.** Draped polygon UVs are produced with `GL_TEXTURE_GEN` in pixel space. `some_nearby_fixed_loc()` rounds a nearby lat/lon to whole degrees so that the 32-bit GPU floats stay accurate while the pattern stays fixed to the earth (`f9ea5cc83`).

**OpenGL state**

- **The GUI framework leaves back-face culling on with `glFrontFace(GL_CW)`** (`GUI_GraphState::Init`). `glPolygon2` output has to be wrapped in `glFrontFace(GL_CCW) … glFrontFace(GL_CW)`, as the structure and preview layers do. Otherwise filled polygons vanish.
- **`GUI_Window::GLDraw` sets `glOrtho(0,w,0,h,-1000,0.01)`.** Anything with z > 0.01 is clipped. This is how parts of objects below ground get cut away in the tilted 2D map (`5cd048f4c`). Map drawing uses window pixel coordinates with an identity modelview.
- **`GUI_GraphState` caches nothing.** Every `SetState(lit, tex_units, fog, alpha_test, blend, depth_read, depth_write)` call issues GL directly, and nothing restores state for you. Each draw routine must set everything it depends on. `WED_Map::Draw` clears depth once at the start of every frame (`ace04eb01` moved the clear there from `WED_PreviewLayer`). Layers drawn before the preview must leave depth writes off.
- **`WED_PreviewLayer` must only queue work in `DrawEntityVisualization`.** It fills `mPreviewItems`, and `DrawVisualization` sorts, draws and deletes them. The 3D pane calls the entity walk before it clears colour and depth, so anything drawn directly during the walk is wiped.

## Architecture

### Ownership and wiring

`WED_DocumentWindow` creates `WED_MapPane` and hands it the document's resolver, archive and library adapter. It also creates `WED_TCEPane`, which becomes the "Texture" property tab, and `WED_MapPreviewWindow`, the separate 3D window. `WED_MapPane` owns:

- `WED_Map`: the canvas. It is a `GUI_Pane`, a `WED_MapZoomerNew` and a `GUI_ScrollerPaneContent` all at once. It sits inside a `GUI_ScrollerPane`.
- Every layer and every tool (it deletes them in its destructor).
- `GUI_ToolBar`, and a `WED_ToolInfoAdapter`/`GUI_TextTable` pair. That pair is the tool-property strip across the top of the window. `DocumentWindow` reparents it through `GetTopBar()`.

`WED_MapPane` is deliberately not in the focus chain for keys and menus (see the class comment in `WED_MapPane.h`). `WED_DocumentWindow` calls `Map_KeyPress`, `Map_HandleCommand` and `Map_CanHandleCommand` itself, so map menu items keep working while the property pane has focus. `Map_KeyPress` gives the key to the tool first (`HandleToolKeyPress`), then checks the tool hot-keys, which only fire with no Shift/Alt/Ctrl.

`archive->AddListener(mMap)` is how the map learns about changes: it refreshes on `msg_ArchiveChanged` (sent on commit, undo and redo). Mid-drag redraws happen only because every mouse event calls `Refresh()`. The 3D preview pane also listens for `msg_ArchiveChangedEphemerally`, which `WED_UndoLayer` sends on every in-command change. That lets the separate window follow along during a drag.

### Layers, caps and draw order

`WED_MapPane` adds layers in this order: `WED_MapBkgnd` (flat background colour only, no imagery), `WED_WorldMapLayer`, `WED_SlippyMap` (OSM/ESRI/custom tiles), `WED_StructureLayer`, `WED_ATCLayer`, `WED_PreviewLayer`, `WED_NavaidLayer`, `WED_TerrainLayer` (DEM previews), `WED_BoundaryLayer`, `WED_DebugLayer`, and then every non-NULL tool. `WED_TerraserverLayer` is still in the tree but is never instantiated and is not listed in `cmake/WED.cmake`.

`WED_Map::Draw` makes three passes over that list, skipping invisible layers:

1. **Visualization.** If `draw_ent_v`, walk the world and call `DrawEntityVisualization`, then `DrawVisualization`. The tilt matrix from the four tilt buttons is multiplied into the projection only around `DrawVisualization`, and only for layers with `draw_ent_v`. In practice that means the preview and structure layers' queued or batched drawing.
2. **Structure.** If `draw_ent_s`, walk the world and call `DrawEntityStructure`, then `DrawStructure`.
3. **`DrawSelected`** on every layer.

The overlay text (tool name, current airport, filter name, status, cursor position and scale bar) comes last.

`GetCaps(draw_ent_v, draw_ent_s, cares_about_sel, wants_clicks)` exists because every per-entity walk costs a full tree traversal. `cares_about_sel` decides whether the walk computes `sel->IsSelected()` per entity or passes false. Tools report all four as false. They draw from `DrawStructure`/`DrawSelected`, and **every** tool's draw methods run every frame with `inCurrent` false for inactive tools. `WED_HandleToolBase::DrawStructure` returns early unless the tool is current or `SetDrawAlways(1)` was called. A new tool that draws from `DrawVisualization` has to check `inCurrent` itself.

Layer visibility is not saved per layer. `ToPrefs/FromPrefs` save only the world map, navaids, slippy mode, preview, pavement alpha, object density and structure-layer toggles. The ATC and boundary layers are forced off at construction and switched only by `SetTabFilterMode`.

### Mouse dispatch

Only button 0 goes to layers and tools; button 1 pans and the wheel zooms by 1.2× steps. On button-0 down, layers that report `wants_clicks` get first refusal in list order; the first one to return nonzero becomes `mClickLayer`. If none claims the click, the current tool gets it. Drag and up events go to `mClickLayer` only. Tools never receive clicks through `wants_clicks`; they are called as `mTool`. Map tools have no pane of their own, so `WED_Map` calls `Refresh()` after every mouse event on their behalf.

### Handles: `IControlHandles` and `WED_HandleToolBase`

`IControlHandles` (`src/Interfaces/IControlHandles.h`) describes editable geometry as entities identified by opaque `intptr_t` IDs. Each entity has indexed handles (a position, a `HandleType_t` that controls how it is drawn, a direction and a hit radius) and links (source and target handle indices, plus optional Bézier control-handle indices, with -1 meaning "none"). The same interface serves three providers:

- `WED_VertexTool`: the IDs are raw `IGISEntity *`. Handles exist only for selected entities that are visible, unlocked and on screen. They are rebuilt whenever `WED_Archive::CacheKey()` or `WED_MapZoomerNew::CacheKey()` changes. Any model change bumps the archive key, so handle indices are valid only within one gesture.
- `WED_CreateToolBase`: a single fake entity (ID 0) holding the path under construction, with three handles per point (point, low control, high control).
- The TCE tools (below), which report handles in `gis_UV` rather than `gis_Geo`.

`WED_HandleToolBase::HandleClickDown` tries, in priority order:

1. The nearest active handle within its radius (`drag_Handles`).
2. A link within `LINE_DIST` pixels (`drag_Links`).
3. `PointOnStructure` (`drag_PreEnt`, which needs `DRAG_START_DIST` of travel before it moves).
4. `CreationDown` (`drag_Create`).
5. A click on an already-selected entity, which starts a move (`drag_PreMove`).
6. Otherwise a click or marquee selection (`drag_Sel`).

Ctrl-click on single-point handles is ignored so that Ctrl can toggle selection instead. Single-click selection collects every candidate, then keeps one by a fixed priority: airport boundary, then the nearest point, then forests/facades/exclusion polys, then lines, then textured and draped polygons, then runways, then other polygons, then exclusion boxes. Subclasses shape the result through `TraverseEntity` (skip, atomic, container, or atomic-or-container). While a drag is active the tool registers as a `GUI_Commander_Notifiable` (unregistering on mouse-up and in `KillOperation`). A menu command or key pressed mid-drag is normally deferred by the GUI until after mouse-up, so `PreCommandNotification` closing the gesture is a backstop for commands that bypass the defer.

### Coordinate systems

- **Model coordinates** are lon/lat degrees (`gis_Geo`). Each entity also carries UVs (`gis_UV`) and `gis_Param`.
- **Map "pixels"** are window coordinates. `WED_Map::SetBounds` passes the pane's window-space bounds to `SetPixelBounds`, so no pane-local offset exists.
- **Projection in `WED_MapZoomerNew`.** A `cos(lat)` equirectangular base carries a GRS80 correction: separate meridional and prime-vertical radii, and a fixed 300 m altitude (the comment explains the WYSIWYG target of about 1 in 10,000). Once `Pix2DegLat` drops below `THR_GNOMONIC`, it blends into a gnomonic projection, and above `THR_WAGNER` of world width it blends toward Wagner. Gnomonic mode is disabled whenever a camera is attached (`cam != nullptr`), which is the 3D preview.
- **Zoom** is capped at about 1 mm per pixel in `ZoomAround`. `ZoomShowArea` clamps spans to at least 0.00001°. `WED_MapPane` pads point-only selections by `PAD_POINTS_FOR_ZOOM_MTR` so "zoom to selection" never divides by zero.
- **Drag deltas** are computed each drag event as `PixelToLL(new) - PixelToLL(old)` and applied incrementally. `drag_Move` applies them by `Rescale`-ing each entity's bounding box, which is linear in lat/lon.
- **3D preview.** `WED_MapPreviewPane` is itself a `WED_MapZoomerNew` with `SetPPM(1.0)` and the pixel centre pinned at (0,0). In that pane "pixels" are metres from the look-at lat/lon, the ground plane is z = 0, and the zoomer's visible bounds are set from the frustum footprint on the ground.

### Preview rendering

`WED_PreviewLayer` turns entities into `WED_PreviewItem`s, each with a layer number taken from the `group_*` enum in `WED_PreviewLayer.h`. That enum mirrors X-Plane's layer groups with gaps left for ±offsets. `layer_group_for_string` maps `.pol`/`.lin` `LAYER_GROUP` names onto it. The same function and enum are used by validation (`WED_ValidateATCRunwayChecks.cpp`), and `draw_obj_at_xyz`/`draw_agp_at_xyz` are reused by `WED_LibraryPreviewPane` and `WED_FacadePreview`. Changes here affect more than the map. (`kGroupNames` maps "unpaved_runways" onto the unpaved taxiway range — a bug (Ben, 2026-09-25), on [the punch list](../bug-punch-list.md).)

`WED_MapPreviewPane` keeps its own copy of the entity walk (`DrawVisFor` in that file), which uses `zoomer.PixelSize()` for camera-aware culling of small composites. It has no hide or lock filtering. All `GUI_Window`s share one GL context with the first window created (`GUI_Window` constructor), so `ITexMgr` texture IDs work in both windows.

### Background and tile layers

`WED_SlippyMap` works out the tiles in view (zoom level from ppm and latitude) and keeps **one** outstanding `WED_file_cache_request` at a time. It polls `gFileCache` from `DrawVisualization` (`finish_loading_tile`) and re-arms a 50 ms `GUI_Timer` that only calls `Refresh()`. Textures are created on the draw path, where the GL context is current. They go into `m_cache`, keyed by the cache path, and are never deleted. A tile that fails to load, or an ESRI "no data" grey image (`is_ESRI_blank`), is stored as id 0 so it is not requested again. Tile colours are darkened and desaturated on load. `WED_NavaidLayer` parses the X-Plane nav, ATC and apt data synchronously on its first draw, and again on every draw for as long as it ends up empty.

## WEDTCE: where it really differs from WEDMap

The TCE copies the map's structure (pane, canvas, layers, tools, toolbar and info strip) but is a separate implementation with real differences:

- **Only the selection is drawn; the world is never walked.** `WED_TCE::Draw` collects selected entities with `Iterate_CollectEntities` and hands them to the layers. It applies no hide/lock filters, no culling and no small-entity cutoff.
- **Nothing runs without a background texture.** `CalcBgknd` finds a `WED_DrapedOrthophoto` in the selection, resolves its `.pol` through `WED_ResourceMgr`, and uses `base_tex` from the last such orthophoto. If none is found (`mTex == NULL`), no layer or tool draws and no mouse input reaches the tool.
- **UV space is fed through the geographic zoomer.** Logical bounds are [0,1] or, for wrapping textures, [-32,32]. The UVs are treated as degrees by `WED_MapZoomerNew`, so the GRS80 aspect correction (about 0.7% at these "latitudes") and the gnomonic blend (below `THR_GNOMONIC` UV per pixel) apply to UV space too. The texture quad and grid are drawn through `LLToPixel` from their corners only. [Needs Runtime: is the resulting misregistration between texture and handles ever visible?]
- **Layer interface.** `WED_TCELayer` is its own class, not a `WED_MapLayer`. `GetCaps` takes two flags, and the entity callbacks return `void` and take no selected/locked arguments. In the `draw_ent_s` pass, `WED_TCE::Draw` calls `DrawEntityVisualization` where it should call `DrawEntityStructure`, a copy-paste bug. It is dormant because no current TCE layer sets the flags.
- **Tools are adapters around map-tool "brains."** `WED_TCEToolAdapter` implements `WED_TCEToolNew` by forwarding to a `WED_MapToolNew` it owns and deletes. The brains, `WED_TCEVertexTool` and `WED_TCEMarqueeTool`, are ordinary `WED_HandleToolBase` + `IControlHandles` subclasses. They call `SetCanSelect(0)` because the base class's selection and move logic work in `gis_Geo`. Their handles are read and written in `gis_UV`, and `BeginEdit/EndEdit` wrap each drag in a "UVmap Modification" or "UVbounds Modification" operation. The brains are `WED_MapLayer`s that never get `SetFilter`, so their `IsVisibleNow`/`IsLockedNow` check only the entities' own hidden and locked flags.
- **Dead wiring.** `WED_TCEPane::TCE_KeyPress`, `TCE_HandleCommand`, `TCE_CanHandleCommand`, `FromPrefs` and `ToPrefs` have no callers anywhere, so the 'v'/'e' hot-keys, TCE menu commands and view persistence do nothing. To make one work, forward it from `WED_DocumentWindow` the way `Map_KeyPress`/`Map_HandleCommand` are forwarded. `FromPrefs` also reads `"tce/top"` for the west edge, where `ToPrefs` writes `"tce/left"`. Decision (Ben, 2026-09-25): wire these up — on [the punch list](../bug-punch-list.md), low priority.

## Recipe: add a map tool

The steps below were checked against the current code but have not been built.

1. **Pick a base class.**
   - To create geometry, derive from `WED_CreateToolBase`, passing min/max point counts and can/must curve/close flags. Implement `AcceptPath` (one `GetArchive()->StartCommand` … `CommitCommand`; find the parent with `WED_GetCreateHost`, which inserts right after a lone selected sibling; leave the new object selected), `CanCreateNow` (return false when no valid host exists, e.g. no current airport) and `GetStatusText`.
   - To edit existing geometry, derive from `WED_HandleToolBase` and `IControlHandles`, call `SetControlProvider(this)`, and start and commit an operation on the selection's `IOperation` in `BeginEdit/EndEdit`.
   - To derive from `WED_MapToolNew` directly, you must implement `HandleToolKeyPress`, `KillOperation`, `GetStatusText` and `GetCaps`, and balance every operation yourself.
   - To serve several entity types from one class, follow `WED_CreatePointTool`/`WED_CreatePolygonTool`: a `create_*` enum indexes the parallel arrays `kCreateCmds`, `kIsAirport`, `kIsToolDirectional`, and so on.
2. **Tool options** go in `WED_Prop*` members constructed with `XML_Name("","")`. They are not archived and are saved only through the `map_<ToolName>_<prop>` prefs. To show an option only for some types, pass `this` as its parent conditionally (`tool==create_X ? this : NULL`).
3. **Add the `.cpp`/`.h` to `cmake/WED.cmake`**, which lists every WED source explicitly.
4. **Register the tool in `WED_MapPane`'s constructor.** Place it in `mTools` at the index matching its icon cell (row 0 is at the bottom; there are two columns). Either replace a `NULL` placeholder, or increase `TOOLICON_ROWS` and add a row to `kToolKeys` (0 means no hot-key). Edit `src/WEDResources/map_tools.png` in both halves: normal on the left, selected on the right. Check that the constructor's `Assert` holds and that `mToolbar->SetValue(mTools.size()-2)` still selects the Vertex tool.
5. **For library-driven tools**, add a `res_*` case to `WED_MapPane::SetResource` so that clicking in the library pane selects the tool and sets its resource.
6. **If the tool creates a new entity class**, decide whether each `SetTabFilterMode` tab should hide, lock or show it, and add it to `hide_all_persistents` if it should be hidden outside the Selection tab.
7. **Test.** Undo and redo after one gesture, switch tools mid-gesture, fire a menu command mid-drag, and try Esc/Return/Backspace for create tools. In a DEV build, watch for the "changed outside of a command" and "started while command … still active" asserts.

## Connections to Other Systems

- **Design rules** ([wed-design-principles.md](wed-design-principles.md)): the stated rules for new code and reviews that this subsystem's conventions should follow — command ownership, pointer vs ID, class vs interface casts, error handling, layering, per-doc prefs, platform reference.
- **Undo and archive** ([wed-object-model.md](wed-object-model.md)): `WED_Archive::StartCommand/CommitCommand`, `CacheKey()`, and the `msg_ArchiveChanged`/`msg_ArchiveChangedEphemerally` broadcasts. The per-document managers the layers draw from (`WED_ResourceMgr`, `WED_TexMgr`) are in [wed-core-services.md](wed-core-services.md). `WED_ToolUtils` (`WED_GetSelect`, `WED_GetWorld`, `WED_GetCurrentAirport`, `WED_GetCreateHost`, the `Iterate_*` helpers) lives here but is included from WEDEntities, WEDImportExport, WEDWindows, WEDCore and WEDProperties. It is a general resolver-lookup utility, not something specific to the map.
- **Entity model** ([wed-object-model.md](wed-object-model.md), [wed-entities.md](wed-entities.md)): drawing and hit-testing go only through `IGIS*` (`Cull`, `GetBounds`, `PtWithin`, `PtOnFrame`, `WithinBox`, `Rescale`, `Rotate`). Per-type code in the preview and vertex tool switches on `GetGISSubtype()`/`GetClass()` compared with `sClass` pointers. Entity `Cull` implementations add their own slop, e.g. `WED_ObjPlacement` uses its visible radius.
- **GUI framework** ([gui-framework.md](gui-framework.md)): `GUI_Pane`, `GUI_ScrollerPane(Content)`, `GUI_ToolBar`, `GUI_GraphState`, `GUI_Commander_Notifiable`, `GUI_Timer`, and GL context sharing between `GUI_Window`s.
- **UI panes** ([wed-ui-panes.md](wed-ui-panes.md)): `WED_DocumentWindow` hosts all three canvases and forwards keys and menu commands. Tab order in the property tab pane drives `SetTabFilterMode`. The library pane calls `WED_MapPane::SetResource`.
- **Validation** ([wed-validation.md](wed-validation.md)): uses `layer_group_for_string` and the `group_*` enum from `WED_PreviewLayer.h`, and `WED_ValidateApt` takes the `WED_MapPane *` and passes it on to `WED_ValidateDialog`.
- **Import/export** ([wed-import-export.md](wed-import-export.md)): the apt.dat and Gateway import dialogs call `WED_MapPane::ZoomShowSel` after an import.
- **Network and file cache** ([wed-network-and-filecache.md](wed-network-and-filecache.md)): `WED_SlippyMap` fetches through `gFileCache`.
- **Textures and resources** ([bitmap-texture-and-ddstool.md](bitmap-texture-and-ddstool.md)): `ITexMgr`/`WED_ResourceMgr` supply the `.pol`/`.lin`/`.obj`/`.agp`/`.fac` data drawn by `WED_PreviewLayer` and the TCE background.
- **DSF library** ([dsf-library-and-dsftool.md](dsf-library-and-dsftool.md)): `WED_TerrainLayer` reads X-Plane's (usually 7z-compressed) global-scenery DSFs with `DSFReadFile` and a positionally initialized `DSFCallbacks_t`, so a field inserted into that struct breaks it silently. The raster callback must copy the DEM data, which DSFLib frees on return.
- **OBJ library** ([utils-platform-and-files.md](utils-platform-and-files.md)): `ObjDraw` creates an OBJ's VBOs lazily on first draw and never deletes them, so `WED_ResourceMgr::Purge` leaks them. They work in the 3D window only because every `GUI_Window` shares one GL context group.
- **Geometry** ([utils-geometry.md](utils-geometry.md)): `Bbox2`, `Bezier2`, `Point2`, and `LonLatDistMeters`/`VectorMeters2NorthHeading` for the status-line readouts. Tessellation uses libtess2 (`LIBTESS 1` in `XDefs.h`), with a GLU fallback.
