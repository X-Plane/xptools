# WED Entities, Properties and the GIS Model

> Source: `src/WEDEntities/`, `src/WEDCore/WED_PropertyHelper.*`, `src/WEDCore/WED_EnumSystem.*`,
> `src/WEDCore/WED_Enums.h`, `src/Interfaces/{IGIS,IPropertyObject,IArray,IDirectory,ISelection,IHasResource,IControlHandles}.h`
>
> The archive, `WED_Persistent`, undo layers and the ID/peer model are in [wed-object-model.md](wed-object-model.md).
> This article covers what sits on top: how entities are built, what their properties do, and what
> structure they assume.

## Things That Will Bite You

### What is persisted, and so must never change

A `.earth.wed.xml` file stores each object as `<object class=… id=… parent_id=…>` plus one XML
element/attribute pair per property. What each piece of a declaration does:

| Piece of a declaration | Persisted? | Consequence of changing it |
|---|---|---|
| C++ class name (`sClass` is `#__Class` in `DEFINE_PERSISTENT`) | Yes, as `class="WED_Foo"` | `WED_Archive` calls `CreateByClass`. An unknown class fails the **whole load** ("Create obj failed."). So renaming a class breaks old files, and a new class cannot be opened by older WEDs. |
| `XML_Name("element","attr")` | Yes | On load, `WED_PropertyHelper::StartElement` silently ignores attributes nobody claims. A rename loses the value with no error. The misspelled `XML_Name("excluzions",…)` in `WED_ExclusionZone` has been kept since 2011 for this reason. |
| Enum **description** string (2nd arg of `ENUM(...)`) | Yes, for `WED_PropIntEnum` and `WED_PropIntEnumSet` | XML stores `ENUM_Desc(value)` and reloads with `ENUM_LookupDesc(domain, text)`. Changing the UI text breaks old files. See the `ObjElevationType` trick below. |
| Enum **export value** (3rd arg) | Yes, for `WED_PropIntEnumBitfield` (XML holds an OR'd int) and for apt.dat/DSF export everywhere | Must stay the same forever. |
| `decimals` argument of `WED_PropDoubleText` | Yes, as the XML precision | `ToXML` calls `add_attr_double(…, mDecimals)`, so it controls saved precision, not only display. Commits `d148a9c65` (polygon heading) and `fe3e95943` (tower height) raised it to stop data being rounded on save. |
| Constructor default value | Indirectly | Used for any file that lacks the attribute, meaning every file written before the property existed. |
| WED name (1st arg of `PROP_Name`) | No | But it is an in-memory API. See "WED names are keys" below. |
| Declaration order | No (binary undo only) | Sets property-pane order and binary undo stream order. Undo buffers never reach disk, so reordering is safe for files. |

Handling unknown enum text on load:
- For `WED_PropIntEnumSet` it is ignored on purpose (see the comment in `StartElement`).
- For a single `WED_PropIntEnum`, `WantsAttribute` stores **-1**. `ENUM_Desc(-1)` returns NULL, and the
  next save passes that NULL into `add_attr_c_str` → `str_escape(const string&)`. `FIX_EMPTY` is 0,
  so nothing guards it. A file from a newer WED that uses a new enum value can therefore crash an
  older WED when it saves. On [the punch list](../bug-punch-list.md), high priority (Ben, 2026-09-25).

**Changing a property's type while keeping files readable.** Follow `a52ca602f`. `has_msl` in
`WED_ObjPlacement` went from `WED_PropBoolText` to `WED_PropIntEnum`, and the new enums were given the
descriptions `"0"` and `"1"`, so old `custom_msl="0|1"` attributes still parse. Friendly UI text is then
supplied by overriding `GetNthPropertyDict` / `GetNthPropertyDictItem`. Do not rename the descriptions.

**Compile-time-gated properties** (`#if ROWCODE_105` in `WED_Runway`, `#if HAS_BDY_TYPES` in
`WED_AirportBoundary`; flags live in `Obj/XDefs.h`) disappear from the object when the flag is off. A
build with the flag off drops those attributes the next time it saves a file written by a build with it on.

### Writing `.value` directly bypasses undo

Every `WED_Prop*` has two write paths:
- `operator=` and `SetProperty` call `PropEditCallback(1)` → `WED_Thing::PropEditCallback` →
  `StateChanged(wed_Change_Properties)`, then write, then call `PropEditCallback(0)`. This path is undo-safe.
- `prop.value = x` does nothing else. Code that writes `.value` must call `StateChanged()` **before** the
  write, as `WED_GISPoint::SetLocation` and `WED_GISPoint_Bezier::SetControlHandle*` do.

`WED_UndoLayer::ObjectChanged` snapshots the object (`WriteTo`) on the *first* `StateChanged` in a
command. If you mutate first and call `StateChanged` later, the "before" image already contains the new
data, and undo quietly restores the wrong state.

Live example: `WED_Airport::AddMetaDataKey` / `EditMetaDataKey` never call `StateChanged`, and several
callers (`WED_SceneryPackExport` "Gateway upgrade heuristics", `WED_MetaDataDefaults`) call them inside
a command. Metadata changes made there are not reliably undoable. The property-pane path
(`WED_Airport::SetNthProperty`) does it correctly. Confirmed bug (Ben, 2026-09-25) — on [the punch list](../bug-punch-list.md), high priority.

The `change_kind` you pass matters too. `WED_PropertyTable` only rebuilds on
`wed_Change_CreateDestroy | wed_Change_Topology`, so a structural change reported as
`wed_Change_Properties` leaves panes stale. With no argument, `StateChanged()` means `wed_Change_Any`.

### Non-property state needs five hand-written hooks

Any member that is not a `WED_Prop*` gets nothing for free: no undo, save, clone or pane. Examples are
`WED_AirportChain::closed`, `WED_Airport::meta_data_vec_map`, `WED_Select::mSelected` and
`WED_KeyObjects::choices`. Each of these classes implements all of:
1. `ReadFrom` / `WriteTo` (undo), chaining to the base first.
2. `AddExtraXML` (save) and `StartElement` (load), chaining to the base for unknown elements.
3. A custom `CopyFrom` instead of `TRIVIAL_COPY`, with `StateChanged()` before copying.
4. `StateChanged()` in every setter.

Leaving any one out gives silent data loss on undo, save or copy/paste. Prefer a real `WED_Prop*`
whenever the value fits one.

### Declaration order in the header, not initializer order, defines the property list

`WED_PropertyItem`'s constructor appends itself to the parent's `mItems`. C++ runs member constructors
in *header declaration* order, whatever order the initializer list uses. `WED_TaxiRoute` initializes
`width` last but declares it fourth, so "Size" appears before "Departures". The header decides pane
order and the binary undo layout.

### `PROP_Name` bit-packing limits fail silently in release builds

`PROP_PTR_OPT` (on) packs three things into one `uintptr_t`: the title pointer, the offsets of the XML
names inside the concatenated literal `wed "\0" elem "\0" attr`, and an 8-bit relative offset back to
the parent. The only checks are `DebugAssert`s, which compile to nothing when `DEV` is 0. A violation in
release writes the wrong XML names or corrupts memory. The constraints:
- At most **30 characters** of WED name (the element offset must be < 32).
- WED name + element name + 2 must be < 64.
- At most **32 properties per object**, counting the hidden "Class", "Name", "Locked" and "Hidden".
  `relPtr::mItemsOffs[32]` overflows past that. `WED_Runway` is the heaviest; with `ROWCODE_105` on it
  sits at the limit the header comment mentions.
- Every property member must lie within 2 KB after the `WED_PropertyHelper` sub-object, 8-byte aligned.
  Don't put a large inline array before property members.
- Titles must be **string literals**. `PROP_Name` uses `sizeof`, and `PTR_FIX` restores address bits 45
  and 46 from `.rodata` (`s_rodata_hi_bits`, fixed for ASLR in `9ef2fcdcc`). A `c_str()` or buffer
  pointer breaks both.
- A `WED_PropertyItem` built with a NULL parent (tools do this for inactive options, e.g.
  `WED_CreatePolygonTool`) never sets `mTitle`. `GetParent()` then returns garbage, so `operator=`
  would call `PropEditCallback` on garbage. Only touch `.value` on parentless items.

### Enum-system invariants

- `WED_Enums.h` is expanded twice: once into a C++ `enum {}` in `WED_EnumSystem.h`, and once as
  `ENUM_Create` calls in `ENUM_Init`. The constants are only right if every entry appends exactly one
  slot. `ENUM_Create` de-duplicates on (domain, description), so **two identical descriptions in one
  domain shift every later constant by one**. The same happens with a repeated domain name. There are
  no duplicates today; keep it that way.
- Domains take enum slots too. Index 0 is the domain `Airport_Type`, so **0 is never a valid enum
  member**. UI and union code rely on this: `WED_PropertyTable` treats `int_val 0` on an exclusive set
  as "none", and `WED_PropIntEnumSetUnion::GetProperty` inserts 0 to mean "some child has none".
- In-memory enum ints move whenever entries are inserted. Only descriptions (XML) and export values
  (bitfields, apt.dat) are stable. Never write raw enum ints anywhere except the undo stream.
- Not every enum comes from `WED_Enums.h`. The library scan adds `LinearFeature` entries for
  library-defined lines and lights at runtime, so those ints depend on scan order and a file that
  uses one loads as -1 when the library is missing. See
  [wed-core-services.md](wed-core-services.md#the-library-scan-mutates-the-global-enum-system).
- `WED_PropIntEnum::SetProperty` and `WED_PropIntEnumSet::SetProperty` silently ignore values from
  another domain (the `ENUM_Domain(…) != domain` check). A "set did nothing" bug is often a
  wrong-domain constant.

### WED names are keys

The first `PROP_Name` argument is looked up by string in several places:
- `FindProperty`. Examples: `WED_ConvertCommands` copies "Heading" / "Texture Heading" between types,
  and `WED_PropertyTable` looks up every cell by its column title.
- The hierarchy pane's fixed columns `{"Locked","Hidden","Name"}` and the selection pane's `"Name"`
  (in `WED_DocumentWindow`).
- Virtual items find their host by name:
  - `WED_AirportNode`'s "Line Attributes" / "Light Attributes" (`WED_PropIntEnumSetFilterVal`) filter
    the real set ".Attributes".
  - `WED_AirportChain`, `WED_Taxiway` and `WED_AirportBoundary` declare `WED_PropIntEnumSetUnion`
    items that union the child property **with the same name**.
  - The editable taxiway line markings therefore depend on three classes spelling "Line Attributes"
    identically. Taxiway → chain → node is a union of a union.
- A WED name starting with `.` is hidden from the pane (`WED_PropertyTable::RecalculateColumns`,
  `WED_ToolInfoAdapter`). `GetNthPropertyInfo` overrides use `prop_name = "."` to hide a property
  conditionally (`WED_TruckParkingLocation`, `WED_ObjPlacement`, `WED_TaxiRoute` for trucks).
- Columns are unioned by name across the selection. Two classes whose properties share a WED name
  share a column, even when the types differ.

### Property indices are not member indices

`WED_GISLine_Width` puts 8 synthetic properties (Length, Heading, Lat/Lon 1/Ctr/2) **in front of** the
inherited ones. `WED_Airport` appends one fake property per metadata entry. `WED_PolygonPlacement`
appends "= Taxi Surface". In an override that must recognize "my" property, compare
`n == PropertyItemNumber(&member)` (virtual; remapped by `WED_GISLine_Width`), never a literal index or
`mItems` position. Code outside the entity should use `FindProperty(name)`.

### `WED_PropIntEnumSet` needs its own XML element

On load, `WED_PropertyHelper::StartElement` first asks every item `WantsElement(name)`. A set claims
its element, pushes itself as handler and `return`s, so **no attributes on that element are parsed**.
If a set shared its element name with scalar properties, those scalars would load as defaults. Scalar
properties may share elements freely: `WED_GISPoint`, `WED_GISPoint_Bezier` and `WED_ShapeNode` all
use `"point"`, through `add_or_find_sub_element`. Attribute names must still be unique within an
element across the whole class chain, because the first match wins.

### `PropEditCallback` overrides

- An override must still call `StateChanged` when `before` is true, either directly (`WED_Runway`,
  `WED_TaxiRoute`) or by chaining to the base (`WED_GISPoint`).
- `WED_Runway` and `WED_TaxiRoute` keep before-and-after state in **function-local statics**. That
  isn't re-entrant, and it assumes before/after pairs are never interleaved across objects.
- `WED_Runway::PropEditCallback` ("Smart Runway Rename") runs on *any* runway property edit. When the
  name's runway enum changes, it calls `apt->CommitCommand(); apt->StartCommand(...)`, which splits the
  edit into two undo steps, and it may show modal dialogs. Keep this in mind before calling
  `SetName`/`SetNthProperty` on runways from batch code.
- `WED_TaxiRoute`'s callback renames the route whenever `runway` changes.

### `CopyFrom` goes through the property interface

`WED_Thing::CopyFrom`, used by `Clone`, copy/paste, `SplitSide` and `SplitEdge`, copies
`GetNthProperty` → `SetNthProperty` for every item with `can_edit && !synthetic`. Two consequences:
- If `GetNthProperty` returns something other than the stored value, the clone stores that. For a
  runway, `WED_TaxiRoute` reports extra "virtual" hot zones and its runway name. That is why
  `WED_TaxiRoute::CopyFrom` re-copies `hot_depart/arrive/ils` by hand afterwards.
- Virtual/filter items must say `synthetic = 1`; the helper classes do. Otherwise they are copied twice.

## Architecture

### How an entity is put together

```
WED_Persistent ─┐
                ├─ WED_Thing (WEDEntities)   hierarchy, sources/viewers, "Class"+"Name" props, XML/undo streaming
WED_PropertyHelper (WEDCore) ┘
   └─ WED_Entity         "Locked"/"Hidden" props, bounds-cache protocol
        └─ WED_GIS*      intermediate "spatial brains" (DECLARE_INTERMEDIATE, not registered)
             └─ concrete (DECLARE_PERSISTENT + DEFINE_PERSISTENT, registered)
```

Non-spatial Things derive straight from `WED_Thing`: `WED_ATCFlow`, `WED_ATCFrequency`,
`WED_ATCRunwayUse`, `WED_ATCTimeRule`, `WED_ATCWindRule`, `WED_Root`, `WED_Select` and
`WED_KeyObjects`. So the airport, a `WED_GISComposite`, has children that are *not* `IGISEntity`, which
contradicts the `WED_Entity.h` comment that "all children of WED_Entities are WED_Entities".
`WED_GISComposite::RebuildCache` skips non-entities. As a result **`GetNthEntity(n)` and
`GetNthChild(n)` index different lists**. The same holds for `WED_GISChain::GetNthPoint`, which skips
non-point children.

Optional mix-ins: `IHasResource` (library path; used by validation, hierarchy search and the
gateway/pack export) and `IHasAttr` (a fake "resource" built from line/light enum descriptions joined
with `$^`, so the hierarchy search filter in `WED_PropertyTable` can match "Red Line"). `IControlHandles`
is implemented by map tools, not entities (see [wed-map-and-tce.md](wed-map-and-tce.md)).

### The four consumers of a property declaration

1. **Property pane**: `IPropertyObject`, implemented by `WED_PropertyHelper`. `WED_PropertyTable`
   wraps each edit in `StartCommand("Change <name>")`, so `PropEditCallback` always runs inside a
   command when the edit comes from the UI.
2. **XML**: `WED_Thing::ToXML` → `PropsToXML` → per-item `ToXML`. Load goes through
   `StartElement` → `WantsElement` / `WantsAttribute`. Attribute matching starts at item 1 because item
   0 is always the `WED_TypeField` "Class" pseudo-property.
3. **Undo**: `WED_Thing::ReadFrom/WriteTo` stream hierarchy IDs, then `ReadPropsFrom/WritePropsTo` in
   `mItems` order, with no names or tags.
4. **Clone**: `CopyFrom` by index, as above.

Virtual items (`WED_PropIntEnumSetFilter`, `…FilterVal`, `…Union`) have empty
`ReadFrom/WriteTo/ToXML`. They only change the pane and must use `XML_Name("","")`. `…Filter` ranges
over enum *ints*, while `…FilterVal` ranges over **export values**: lines are 1–99 and lights 101–199 in
`LinearFeature`.

`WED_PropDoubleTextMeters` stores meters and converts on Get/Set using the global `gIsFeet`. Anything
reading it through `GetNthProperty` sees feet or meters depending on the user's setting. Entity code
should use `.value`. `WED_PropFrequencyText` rounds to the 25 kHz / 8.33 kHz raster on every set. On
load it only nudges .x20 and .x70 values up by 5 kHz.

### GIS classes and the child structure each one assumes

`GetGISClass` returns the most specific class and is **computed live**, so it can change when a
property changes:
- `WED_GISChain`: `gis_Ring` if `IsClosed()`, else `gis_Chain`, or `gis_Composite` if `IsJustPoints()`.
  `WED_ForestRing` derives both from its parent `WED_ForestPlacement`'s fill mode.
- `WED_GISPolygon`: `gis_Polygon` only if `IsInteriorFilled()`. `WED_AirportBoundary`,
  `WED_ExclusionPoly`, a non-area `WED_ForestPlacement` and a `WED_FacadePlacement` with topo mode ≠ 0
  report **`gis_Composite`**. Code that switches on `gis_Polygon` misses them.
- `WED_GISChain::PtWithin` returns false: rings have no area. Area tests are only in `WED_GISPolygon`.

| Base | Children / sources | Invariant (who relies on it) |
|---|---|---|
| `WED_GISPoint` (+`_Bezier`, `_Heading`, `_HeadingWidthLength`) | none | Location is lat/lon properties. HWL (helipads) adds width/length in meters. |
| `WED_GISChain` → `WED_Ring`, `WED_AirportChain`, `WED_FacadeRing`, `WED_ForestRing`, `WED_LinePlacement`, `WED_StringPlacement`, `WED_ShapePlacement` | children = points, in order | `Reverse`/`Shuffle` `Assert` (active in release) that the points are **all bezier or all non-bezier**. `SplitSide` clones `GetNthChild(best)`, so a chain whose children aren't all points misindexes. `WED_AirportChain::closed` is a raw member (see the five hooks). |
| `WED_GISPolygon` | child 0 = outer ring, children 1..n = holes; every child an `IGISPointSequence` | `GetOuterRing` `DebugAssert`s child 0. `AddHole` asserts `gis_Ring`. Taxiway/boundary rings are `WED_AirportChain`s with `closed=1` (set by `WED_CreatePolygonTool`, `WED_AptIE`, convert commands). `DeleteHole(n)` deletes child **n**, not n+1, so `DeleteHole(0)` would delete the outer ring. It has no callers. |
| `WED_GISLine` / `WED_GISLine_Width` (runway, sealane) | exactly 2 children: child 0 = source = end "1", child 1 = target = end "2" | `GetNthPoint` `Assert`s n∈{0,1}. Runway property suffixes 1/2 and the name halves "09L/27R" map to source/target. |
| `WED_GISEdge` (taxi route, road) | **sources**: exactly 2 (start, end), which are sibling nodes elsewhere in the tree; children = interior shape points; no viewers | Edges share end nodes through source/viewer links, so a node knows its edges as viewers. End-point bezier handles live **on the edge** (`ctrl_lat_lo` etc., stored as deltas) because the node is shared. `Validate` checks 2 sources and 0 viewers. |
| `WED_GISComposite` (airport, group) | any children | Bounds are the union over the children that are `IGISEntity`. |
| `WED_GISBoundingBox` (exclusion zone) | min and max points | |

**Bezier handles** (`WED_GISPoint_Bezier`, `WED_GISEdge`) are stored as lat/lon **deltas** from the
point. A handle "exists" exactly when its delta is non-zero. When `is_split` is false, setting one handle
mirrors the other (`hi = -lo`). `IGIS.h` warns that un-splitting without then moving a handle leaves
things ambiguous. `GetSide` returns true (bezier) if either end has a handle.

**Airport line/light attributes belong to the segment that starts at the node.** `ExportLinearPath`
(`WED_AptIE`) writes each node's attributes on the segment leaving it, and drops them for the last node
of an open chain. That's why `WED_GISChain::Reverse` swaps `WED_AirportNode` attributes between n and
np−n−2 instead of reversing them, and why `Shuffle` rotates facade wall types.

### Bounds-cache protocol (`WED_Entity`)

- `CacheInval(flags)` only walks up to the parent and out to viewers for bits that are *currently
  valid*. Once invalid, an object stops re-notifying until someone calls `CacheBuild` on it.
- Each class must follow one of the three behaviours documented in `WED_Entity.h`. Non-caching leaves
  like `WED_GISPoint` call `CacheInval(cache_Spatial); CacheBuild(cache_Spatial)` together on every
  mutation (`SetLocation`, `Rotate`, `PropEditCallback`). Otherwise the next change never reaches
  the parent.
- Child add/remove and viewer add/remove invalidate automatically (`WED_Entity` overrides
  `AddChild`, `RemoveChild`, `AddViewer`, `RemoveViewer`). Geometry mutated any other way needs an
  explicit `CacheInval`. Undo restores invalidate through `PostChangeNotify`, and tools that cache
  derived data key on `WED_Archive::CacheKey()` instead (both in
  [wed-object-model.md](wed-object-model.md#entity-caches-invalidation-is-edge-triggered)).
- `WED_GISChain::RebuildCache` calls `GetBounds` on every child on purpose, to re-arm them (WED-828).
  If you add a caching container, do the same, or the children's next moves won't reach you.
- **`WED_GISEdge` is fragile here.** Its cache was added in `e3e14fee7` and patched twice (`2b7102509`,
  and `356c5b791` "bandaid"). Changing sources through `ReplaceSource` or a node merge doesn't reliably
  invalidate the edge, so `GetNthPoint(0/last)` and `GetSide` now read end points straight from
  `GetNthSource()` and bypass `mCachePts`. Keep it that way.
- In its spatial pass, `WED_GISEdge::RebuildCache` loops `mm < GetNumPoints()` (children + 2) but calls
  `GetNthChild(mm)`. For an edge *with* shape points that reads past `child_id`. `GetNthChild` only
  guards the empty case. Confirmed bug (Ben, 2026-09-25) — on [the punch list](../bug-punch-list.md), high priority.

### Document skeleton, selection and groups

- `WED_Document` creates a new document as `WED_Root` (ID 1) with three named children: `"selection"`
  (`WED_Select`), `"choices"` (`WED_KeyObjects`) and `"world"` (`WED_Group`).
- `Resolver_Find` starts at object 1 and walks names through `IDirectory::Directory_Find` (a
  `GetNamedChild` string compare) and `[n]` through `IArray`. `WED_GetWorld` and `WED_GetSelect` depend
  on those **names**.
- `WED_KeyObjects` overrides `Directory_Find` to return its key→ID map, e.g. the current airport.
- `WED_Select` is a persistent Thing, so selection changes are undoable and **need an open command**
  (`StateChanged` → `DebugAssert` "object changed outside of a command").
- It stores a `set<int>` of IDs, so `GetSelectionVector` is in **ID order**, not click or hierarchy
  order. Use `WED_GetSelectionInOrder` for hierarchy order.
- `GetNthSelection(n)` walks the set, so `for(i<count) GetNthSelection(i)` is O(n²).
- `WED_Group` adds nothing to `WED_GISComposite`. Its identity as a folder is **hardcoded** by `sClass`
  pointer in `WED_IsFolder` and in `WED_PropertyTable`'s search filter, as the list `WED_Group`,
  `WED_Airport`, `WED_ATCFlow`. A new container class must be added to both.
- Class identity checks compare **pointers**: `GetClass() == WED_Runway::sClass`, and
  `GetGISSubtype()` returns `GetClass()`. Comparing against a string literal needs `strcmp`, never `==`.

### Airport and ATC structure

- Airport-only classes carry a required ancestor in `WED_GetParentForClass` (`WED_ToolUtils`): an
  airport for runways, taxiways, signs, routes and so on, and an `WED_ATCFlow` for runway use and
  time/wind rules. The rule is enforced when reorganizing (`Iterate_CollectRequiredParents` in
  `WED_GroupCommands`), and it can be satisfied by any ancestor, not only the parent.
- apt.dat export (`WED_AptIE`) **walks the hierarchy** and appends to `apts.back()` and
  `apts.back().flows.back()`. A runway use reached before any flow would index an empty vector.
  Hierarchy order becomes apt.dat order for flows and their rules. X-Plane evaluates flows in file
  order and **the first flow whose rules match wins** — confirmed in X-Plane source 2026-09-25
  (`ATCControllerCab::GetBestFlowByTimeAndWeather`, `engine/ATC/ATC_controller_cab.cpp` in the
  design repo: no scoring; time rules OR'ed, wind rules OR'ed, ceiling and visibility must pass;
  no match → no flow). So reordering flows in WED's hierarchy changes sim behavior.
- Runway identity across ATC objects is **by name or enum, not by pointer**:
  - `WED_Runway::GetRunwayEnumsOneway/Twoway` parse the runway's Name ("9L" is retried as "09L").
  - `WED_TaxiRoute::runway` is an `ATCRunwayTwoway` enum. Hot zones and `WED_ATCRunwayUse::rwy` are
    `ATCRunwayOneway`.
  - Renaming a runway only keeps them consistent through "Smart Runway Rename" in
    `WED_Runway::PropEditCallback`.
  - For two-way runway enums, export value /10 ≥ 37 (see `get_runway_parts`).
- A taxi route that is a runway (`runway != atc_rwy_None`) reports its Name as the runway enum's text,
  hides the name from editing, and adds the runway's own one-way ends to the hot-zone sets it shows.
  `HasHot*` deliberately ignores those implicit parts.
- `WED_TaxiRouteNode` is orphaned, and deleted by `WED_NoLongerViable` / `WED_Repair`, when it has no
  viewers. Other viability rules (`WED_GroupCommands`): a point sequence needs ≥2 points (≥3 inside a
  polygon under the strict delete-key rule, 4 under an overlay image, 3 for a closed facade ring); a
  polygon with no children is dead; an edge with a NULL source is dead. `WED_Repair` applies the loose
  rules at file load. See [wed-validation.md](wed-validation.md) for the export-time checks.
- `ROAD_EDITING` (on in `Obj/XDefs.h`) selects between **two copies** of `REGISTER_LIST_ATC` in
  `WED_AppMain.cpp`. An ATC-side class has to be added to both.

## Recipes

### Add a new entity type

Checked against `7e1b7a141` (Shape: `WED_ShapePlacement` + `WED_ShapeNode`, 2023) and `52d265e8c` /
`a91021b14` / `b688441b8` (`WED_ExclusionPoly`, 2023). The build files named in those commits (msvc,
makerules, xcode) were deleted in `86bb9a0a6`. Today the build lives only in `cmake/WED.cmake`.

1. **Header** (`src/WEDEntities/WED_Foo.h`): derive from the matching `WED_GIS*` base. Put
   `DECLARE_PERSISTENT(WED_Foo)` in the class and implement `HumanReadableType()` (pure virtual in
   `WED_Thing`). Also implement the base's pure virtuals: `IsClosed()` / `IsJustPoints()` for a chain,
   `IsInteriorFilled()` for a polygon. Declare property members in the order the pane should show them.
2. **Source**: `DEFINE_PERSISTENT(WED_Foo)`, plus `TRIVIAL_COPY(WED_Foo, Base)`, or a real `CopyFrom`
   if there is non-property state. Then the constructor `(WED_Archive*, int) : Base(a,i),
   prop(this, PROP_Name("UI Name", XML_Name("foo","attr")), default…)` and an empty destructor.
3. **Register**: add `_R(WED_Foo)` to `REGISTER_LIST` in `src/WEDCore/WED_AppMain.cpp`, or to *both*
   `REGISTER_LIST_ATC` copies. Without this, any file containing the class fails to load. The
   macro and registration rules (and what breaks when one is missing) are in
   [wed-object-model.md](wed-object-model.md#registering-a-new-persistent-class-three-places-or-loadundo-breaks).
4. **Build**: add the .cpp and .h to `cmake/WED.cmake`.
5. **Enums** (if needed): append a new `ENUM_DOMAIN` block to `WED_Enums.h`. Shape added `ShapeType`
   at the end. Descriptions must be unique within the domain.
6. **Creation**:
   - Add a `create_*` mode to `WED_CreatePolygonTool` or `WED_CreatePointTool`, choosing the ring and
     node classes there; that is where child-class invariants actually come from.
   - Add a `mTools` slot in `WED_MapPane`, with its icon in `WEDResources/map_tools.png`.
   - Optionally add a "Convert To" entry: a `wed_ConvertToFoo` enum in `WED_Menus.h`, the item in
     `WED_Menus.cpp`, a case in `WED_DocumentWindow::HandleCommand` / `CanHandleCommand`, and logic in
     `WED_ConvertCommands.cpp`.
7. **Map display**: add a `WED_IsIconic` exclusion in `WED_ToolUtils.cpp` for plain vertex node types
   (Shape added `WED_ShapeNode`). Add drawing branches in `WED_StructureLayer` / `WED_PreviewLayer`,
   and `WED_MapPane` hide/unhide lists where relevant.
8. **Airport-bound?** Add it to `WED_GetParentForClass`. **A container?** Add it to `WED_IsFolder` and
   the `WED_PropertyTable` filter list.
9. **Export/import and validation**: `WED_AptIE` (a clause in the hierarchy walker plus import),
   `WED_DSFExport` / `WED_DSFImport` (ExclusionPoly), and `WED_Validate.cpp` (ExclusionPoly
   `b688441b8`). See [wed-import-export.md](wed-import-export.md).

### Add a property to an existing entity

Checked against `104a4b2cf` / `beaa5771a` (custom service truck on `WED_TruckParkingLocation`, 2022),
`d148a9c65` (precision change), `a52ca602f` (type change) and `7688cd055` (flag-gated boundary type).

1. Declare `WED_PropXxx foo;` in the header at the position it should take in the pane.
2. Initialize it in the constructor with `PROP_Name("≤30-char UI name", XML_Name("elem","attr"))`.
   Reuse the class's existing element (`"truck_parking_spot"`) with a **new** attribute name. Pick the
   default with care: every existing file loads with it.
3. Access it from entity code through `operator=` (undo-safe) or `.value` for reads.
   `WED_TruckParkingLocation::GetTruckCustom` is the pattern.
4. If it reaches apt.dat or DSF, extend the entity's `Import` / `Export`, the `XESCore/AptDefs.h`
   struct, and `XESCore/AptIO.cpp` read/write, including version gating
   (`if (vers < 1200) … "no custom trucks"`). WED depends on these XESCore files for apt.dat.
5. Hide it conditionally, or trim its enum list, in `GetNthPropertyInfo` / `GetNthPropertyDict` using
   `n == PropertyItemNumber(&foo)`.
6. Add validation to `WED_Validate.cpp` (a new `err_*` enum in `WED_Validate.h`, as in `104a4b2cf`).
7. If the feature is not yet released, you may still rename or drop it freely. `beaa5771a` removed
   `custom_driver` two days after adding it. After a release the XML names are frozen.
8. Record user-visible changes in `src/WEDCore/README.WorldEditor`. Both example commits did.

## Connections to Other Systems

- **Design rules** ([wed-design-principles.md](wed-design-principles.md)): the stated rules for new code and reviews that this subsystem's conventions should follow — command ownership, pointer vs ID, class vs interface casts, error handling, layering, per-doc prefs, platform reference.
- [wed-object-model.md](wed-object-model.md): `WED_Persistent`, `WED_Archive`, the undo layers,
  `StateChanged` / command discipline and XML document I/O, all of which this article builds on.
- [wed-core-services.md](wed-core-services.md): the library/resource managers behind `IHasResource`
  lookups, e.g. `WED_PolygonPlacement`'s "= Taxi Surface" dictionary.
- [wed-validation.md](wed-validation.md): `WED_Validate` walks these classes; viability and repair
  rules sit alongside it.
- [wed-map-and-tce.md](wed-map-and-tce.md): tools implement `IControlHandles` over `IGIS*`. Create
  tools decide child classes. Layers switch on `GetGISClass` / `GetGISSubtype`.
- [wed-ui-panes.md](wed-ui-panes.md): `WED_PropertyTable` / `WED_PropertyPane` consume
  `IPropertyObject` by WED name.
- [wed-import-export.md](wed-import-export.md): `WED_AptIE` (hierarchy walk), DSF import/export, and
  entity `Import` / `Export` methods.
- [utils-geometry.md](utils-geometry.md): `Bezier2`, `Bbox2`, `Point2` and `GISUtils` heading/meters
  conversions used by `Rotate` and `Rescale`.
