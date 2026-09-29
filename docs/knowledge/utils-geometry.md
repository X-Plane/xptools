# Utils Geometry (CompGeomDefs2, GISUtils, WED_GISUtils, libtess2)

> Source: `src/Utils/CompGeomDefs2.h`, `CompGeomDefs3.h`, `CompGeomUtils.*`, `GISUtils.*`, `MathUtils.h`,
> `MatrixUtils.*`, `Interpolation.*`; WED adapters in `src/WEDCore/WED_GISUtils.*`, `WED_Clipping.*`;
> vendored `SDK/libtess2/`. Test fixture: `test/wgs84/`.

## Things That Will Bite You

### CGAL is gone, but comments still describe it
CGAL was removed from WED in `2d1da404d` ("CGAL is dead!", Aug 2025). No exact or robust
kernel exists anywhere in WED. All geometry is plain `double`, and predicates like
`Vector2::left_turn` and `Segment2::intersect` are unfiltered. They compare with `==` / `>`
directly. Comments that still mention CGAL are stale:
- `CompGeomUtils.h` ("robust CGAL foundations")
- `WED_GISUtils.h` ("The CGAL variants will also fail…")
- `BezierToBezierPointMiddle` ("CGAL gives imprecise matching", with its assert disabled)
- `gentle_crop` in `WED_DSFExport.cpp` ("CGAL sometimes gives us a point… outside the DSF")

Don't reason from those comments. XESCore/RenderFarm still use CGAL, but WED/DSFTool/DDSTool don't.

### Point2 is (lon, lat) in degrees, and every predicate works in raw degree space
`Point2::x_` is longitude and `y_` is latitude, both in degrees. That holds for every WED
entity in `gis_Geo` and for everything in `Utils/CompGeomDefs2.h`. Distances, `is_near`,
`squared_distance`, bbox tests and simplification tolerances are all computed in
**anisotropic degree space**, where one degree of lon shrinks by cos(lat). Consequences:
- Tolerances are hard-coded in degrees. Their meaning in meters depends on direction and
  latitude:
  - `Polygon2cleaner` uses `1e-12` deg² (about 0.1 m N-S).
  - `PointSequenceToPolygon2` uses `3e-4` deg.
  - `SimplifyPolygonMaxMove(…, 1.8e-5, …)` in `WED_GroupCommands.cpp` is commented "about 1.5 meter".
  - `MIN_BEZ_HANDLE_DEG` is `1e-7`.
- `WED_MapZoomerNew::GetClickRadius` returns a radius in **latitude** degrees. Callers use it
  for both axes (e.g. `Bezier2::is_near`, which builds a square box in degrees), so the
  pick region is squashed E-W in meters at high latitude.
- Only a few places convert to meters first: `CoordTranslator2` via `CreateTranslatorForBounds`,
  `VectorLLToMeters`, and the `Quad_*` helpers. If an algorithm needs real angles or
  distances (insets, offsets, perpendiculars), convert first.

### Three different earth models are in use at once
| Where | Model |
|---|---|
| `GISUtils.cpp`: `LonLatDistMeters`, `VectorLLToMeters`/`VectorMetersToLL`, `MetersToLLE`, all `Quad_*` | Sphere, `EARTH_MEAN_RADIUS` = 6371008 m (`XESCore/XESConstants.h`); lon scale = cos(lat) |
| `WED_MapZoomerNew::mapScale::set` | GRS80 prime-vertical N and meridional M radii at a fixed 300 m MSL (`USE_GRS80`) |
| `CreateTranslatorForBounds` (`deg2mtr`) | WGS84-ish ellipsoid N/M at the bbox centroid latitude |

The sphere and GRS80 disagree by about 0.5% in N-S scale near the equator. Something sized in
meters with `Quad_1to4`/`MetersToLLE` can therefore measure differently on the map, or in X-Plane 12
(which uses an ellipsoid, per comments in `XESConstants.h` and `WED_MapZoomerNew.cpp`).
The long header comment in `WED_MapZoomerNew.cpp` explains the remaining known discrepancies
(3–5 ft for large meter-space objects, and so on). `test/wgs84/earth.wed.xml` plus `WED_grid_10m.obj`
is the manual fixture for checking object rotation and scale against X-Plane. Don't "fix" one
model in isolation. **Direction (Ben, 2026-09-25): everything should move to WGS84** — GISUtils,
the zoomer and the translators alike. Until that's done as one coordinated change (on [the punch list](../bug-punch-list.md)),
don't convert a single call site.

### Heading helpers: `…Degs…` and `…Meters…` are not interchangeable
In `GISUtils.cpp`:
- `VectorDegs2NorthHeading` / `NorthHeading2VectorDegs` take or return a vector in
  **lon/lat degrees**, and apply cos(ref.lat).
- `VectorMeters2NorthHeading` / `NorthHeading2VectorMeters` assume an isotropic
  (meters or pixels) vector.

Giving a raw `Vector2(ll_a, ll_b)` to a Meters variant gives a heading that is wrong in
proportion to latitude. `WED_CreateToolBase::RecalcHeadings` does exactly that with lat/lon `mPts`
(the status-bar heading while drawing). `WED_Map` computes its default readout with the Degs
variant. Treat the CreateToolBase call as a latent bug, not as a pattern to copy.

All four helpers also add a meridian-convergence term `lon_delta * sin(lat)` when `ref != p`.
Nearly every caller passes `ref == p`, so the term is zero. `Quad_4to1` (ctr vs. `ends1`) is
the notable exception.

### Winding convention: outer ring CCW, holes CW, enforced only in some places
The contract is outer ring CCW and holes CW. `WED_Clipping.h` states it, and
`WED_PolygonWithHolesForPolygon` / `WED_BezierPolygonWithHolesForPolygon` (for `Polygon2p`/
`uv`/`BezierPolygon2*`) normalize to it by reversing. For `Bezier2`, reversing also
swaps `c1`/`c2`. Other enforcement points:
- `WED_CreatePolygonTool` reverses points at creation.
- `ValidateOnePolygon` in `WED_Validate.cpp` raises `err_gis_poly_wound_clockwise`. Reference
  images are exempt, because WEDbing places CW nodes.

`clip_polygon` fails on wrong winding, and the validation comment warns DSF export may assert.
Traps:
- **The flattening overload** `WED_BezierPolygonWithHolesForPolygon(IGISPolygon*, vector<Polygon2>&)`
  is different from its siblings:
  - It does **not** normalize orientation.
  - It does **not** clear `out_pol`; it appends. The mow-grass code in `WED_GroupCommands.cpp`
    relies on appending to collect many polygons into one list.
  - It flattens beziers into 3–30 uniform-t points.
- **`PolygonUnion` / `PolygonIntersect` / `PolygonCut`** (`CompGeomUtils.cpp`, libtess2):
  - The header comment says "Outer contours clockwise, holes counter-clockwise", which is the
    opposite of the WED convention.
  - This works anyway because they call `tessTesselate(…, normal = 0)`. libtess2 then
    auto-computes the normal, and `CheckOrientation` flips it so that the total signed area is
    ≥ 0, which makes absolute orientation irrelevant.
  - What matters is that all outer rings share one orientation and holes are opposite.
  - `PolygonCut` reverses B itself.
  - Input rings with mixed orientation would cancel under `TESS_WINDING_POSITIVE` (inference;
    **[Needs Runtime]**).
- WED-1311 (`844b7f3a7`): "Convert To" left holes wound CCW. Any code that builds new rings
  must re-orient each ring by its index (outer vs. hole).

### Three CCW tests, all vertex-only
- `Polygon2::is_ccw` picks the highest-then-rightmost vertex. It has shortcuts for a flat top.
- `is_ccw_polygon_pt` picks the leftmost-then-lowest vertex.
- `is_ccw_polygon_seg` picks the side whose **p2** is lowest by x-then-y. The `lesser_*` /
  `greater_*` comparators' `Segment2`/`Bezier2` overloads compare `p2` only, so the turn tested
  is at that extreme vertex.

None of them look at bezier control handles, so a heavily curved ring can be misjudged. A
zero-length side at the extreme vertex makes `left_turn` see collinear and report CW. The
winding validation comment calls out zero-length segments as a source of false positives.

### Vendored libtess2 was changed to `double`, and callers depend on that layout
`SDK/libtess2/Include/tesselator.h` has `typedef double TESSreal`. Upstream uses `float`; it
was changed in `fa3ec053d`. Two call sites pass `Point2` arrays straight into `tessAddContour`
(stride `2*sizeof(TESSreal)`, or 4 with interleaved UV):
- `glPolygon2` in `WEDMap/WED_DrawUtils.cpp` (marked `#if 1 // 2 * sizeof(TESSReal) == sizeof(Point2)`)
- the boolean ops in `CompGeomUtils.cpp`

Re-vendoring upstream libtess2 would silently corrupt every polygon. `LIBTESS` is forced on in
`Obj/XDefs.h`, so the gluTess branch is dead code.

`glPolygon2` also refuses to fill a polygon when `tri_count - 2*n_holes + 2` differs from the
vertex count, i.e. when contours self-intersect or holes cross. That mimics X-Plane/gluTess
(`b0bed59aa`), so a polygon with no fill on the map is intentional.

### "No bezier handle" means exactly zero
- `WED_GISPoint_Bezier` stores handles as lon/lat **offsets from the node**.
  `GetControlHandleLo/Hi` report a handle only if the offset `!= 0.0`.
- `BezierPoint2::has_lo/has_hi` test `lo != pt`.
- `Bezier2::is_segment()` tests `p1==c1 && p2==c2`.
- `WED_GISChain::GetSide` returns `true` only if at least one handle exists; the
  "is it curved" flag that everything else branches on comes from here.

Any transform that round-trips through trig or meters leaves tiny non-zero offsets and turns
corners into bezier nodes. That was WED-1024 (`0aeb6b35a`), fixed by snapping offsets below
`MIN_BEZ_HANDLE_DEG` to 0 in `WED_GISPoint_Bezier::Rotate`. New transforms on bezier nodes
(see also `WED_TextureBezierNode`, `WED_RoadNode`) need the same snap. `Rescale`, via
`rescale_to_xv_projected`, keeps 0 exactly because it is purely multiplicative.

### Connectivity is decided by exact float equality
All of these compare with `==`, and none use an epsilon:
- In `WED_Clipping.cpp`: `validate_poly_closed` and `side_is_degenerate`.
- In `WED_DSFExport.cpp`: `one_winding` and `bad_match`.
- `BezierSeqIsRing` / `BezierPointSeqIsRing`.

Code that generates geometry must copy the shared endpoint `Point2` bit-for-bit (e.g.
`b.p1 = prev.p2`), not recompute it.

`Segment2::intersect` also has endpoint rules:
- It returns **true** with `p` set to the shared node when two segments share an endpoint.
  Callers in `WED_Validate.cpp` and `WED_GroupCommands.cpp` filter shared-node pairs before
  calling.
- It returns false for co-located, reversed-co-located and parallel pairs, so overlapping
  collinear edges are never reported as intersecting.

### DSF "triple" bezier notation collapses co-located vertices on import
DSF can only express mirrored handles. `BezierPointSeqToTriple` (`WED_GISUtils.h`) therefore
writes a split or one-sided node as up to three coincident points:
1. the point with `hi` mirrored from `lo`,
2. the point with no handles,
3. the point with the real `hi`.

On import, `BezierPointSeqFromTriple` merges **any** run of consecutive points with equal `pt`
into one node. Genuinely co-located consecutive vertices (zero-length sides) therefore
disappear on DSF import. The apt.dat importer had the same bug and got a guard (WED-787,
`022c6412e` in `WED_AptIE.cpp`). The DSF path has none. Not intended (Ben, 2026-09-25) — on [the punch list](../bug-punch-list.md), low priority.

### Bezier flattening differs by consumer
| Consumer | Method |
|---|---|
| Map drawing: `BezierPtsCount` in `WED_DrawUtils.cpp` | Control-polygon length in **pixels** / `BEZ_PIX_PER_SEG`, clamped; `BEZ_MIN_SEGS` if off-screen (`bounds_fast`); uniform t |
| `PointSequenceToPolygon2` (the flattening overload above) | Control-polygon length in degrees / 3e-4, clamped 3..30; uniform t |
| DSF export (`DSF_AccumPolygonBezier`, clip paths) | Keeps true cubic beziers (triple notation) |
| `approximate_bezier_epsi` / `_sequence_epsi` (`CompGeomDefs2.h`) | Adaptive, but unused by WED |

`approximate_bezier_epsi` only tests the curve point at `t_middle` against the chord. A
symmetric S-curve, whose midpoint lies on the chord, therefore comes out as a single straight
segment. Don't adopt it without fixing that.

### Bezier2 algorithms are approximate and recursion-bounded (or not)
- `Bezier2::intersect(rhs, depth)`:
  - Subdivides while the true bboxes interior-overlap, then, at depth < 0, tests the chords.
  - Sub-curves that share an endpoint are declared non-intersecting. This avoids false
    positives at acute node joins (`27c062113`) at the cost of missing crossings very close to
    a shared node.
  - Only the 3-argument overload returns a crossing point. At `WED_GroupCommands.cpp`'s
    "Cannot split through holes" check, `pt` stays at its default value when the side is a
    bezier.
- `Bezier2::is_near` has **no depth limit**. It stops only when the sub-curve bbox is smaller
  than `d` or misses the point box, so `d <= 0` on a point that lies on the curve recurses
  without bound.
- `Bezier2::bounds` is exact (monotone-region extrema). `bounds_fast` is the control hull.
  `WED_GISChain::WithinBox` relies on `bounds` touching the extremes.
- Root finders in `CompGeomDefs2.h`:
  - `linear_formula` returns **-1** for "all t" (a == b == 0).
  - `quadratic_formula` returns `0.0` (an int) for none.
  - `cubic_formula` is plain Cardano.
  - `t_at_x/t_at_y` bail out early when outside the control hull, because "the cubic formula
    runs out of internal precision".
  - `approx_t_for_xy` is a heuristic for off-curve points (`7422a204c` improved near-vertical
    cases).
  - On Windows, a `cbrt` shim handles negative input (`0ad35ecb8`).

### No antimeridian or pole handling anywhere
**Antimeridian-crossing scenery is unsupported (Ben, 2026-09-25).** Don't add piecemeal wrap
handling.
`Bbox2` is a plain min/max box, and lon is treated as an ordinary x axis. Nothing in Utils,
WED_GISUtils, WED_Clipping or the map zoomer wraps at ±180, and nothing special-cases the poles.
- `VectorLLToMeters`, `MetersToLLE` and `NorthHeading2VectorDegs` divide by cos(lat).
- The only guard found is validation rejecting runway ends outside ±180/±90.

Geometry that crosses ±180 would get a world-spanning bbox (inference; actual behavior is
**[Needs Runtime]**).

### Other sharp edges
- **`MathUtils.h` `interp()`** is `float`-only and **clamps** to [y1, y2]. `interp360` and
  `fltwrap` are float too. A float at lon 180 resolves only about 1e-5 deg (~1 m), so use
  `double_interp`/`double_extrap`/`dobwrap` for coordinates. `MagneticDeviation` in
  `WED_GISUtils.cpp` uses the float `interp`, which is fine for its coarse table.
- **`SimplifyPolygonMaxMove`** (`CompGeomUtils.cpp`):
  - It never removes a vertex whose lon **or** lat is integral (`IsIntegral`, a "HACK" to pin
    DSF tile edges).
  - In `calc_error`, the allow-in/allow-out test dots against `Vector2(seg.p1)`, the position
    vector, instead of the vector to the tested point. That restriction is effectively broken.
    WED only calls it with `(true, true)`.
- **`Polygon2cleaner`** (runs on every libtess boolean-op output) removes the middle point of a
  backtracking triple. When `n == size-1` the middle is index 0, but it pops the last element
  instead (suspected off-by-one; **[Needs Runtime]**).
- **`CoordTranslator2`** is a linear bbox→bbox map. Its `Forward`/`Reverse` guard zero-width
  axes (NaN fix `eed290fb9` for E-W or N-S line facades). `CreateTranslatorForBounds` also
  inherits the anisotropy limits described in its own comment.
- **`Bbox2::rescale_to_x_projected` / `rescale_to_xv_projected`** scale lon by the ratio of
  cos(**`p1.y`**), i.e. the south edge of each box, not its center.
- **Orphans.** Nothing in WED/DSFTool/DDSTool includes `QuadTree.h`, `RTree2.h`,
  `douglas_peuker.h`, `UTL_interval.h` or `PolyRasterUtils.h`. `BWImage.cpp` is compiled into
  WED (`cmake/WED.cmake`), but no WED source uses it. `GeoUtils` is only used by ObjView/ObjEdit.
  DSFTool and DDSTool link no geometry code; DDSTool only includes `MathUtils.h`.
- **Utils→XESCore header dependency.** `CompGeomUtils.cpp` and `GISUtils.cpp` include
  `XESConstants.h` (the source of `DEG_TO_MTR_LAT`, `EARTH_*`) and `DEMIO.h`. `DEG_TO_RAD`/
  `RAD_TO_DEG` are only defined if proj's header hasn't already defined them (`PROJ_API_H`).

## Architecture

```
WED entities (IGISPointSequence / IGISPolygon, gis_Geo | gis_UV | gis_Param layers)
     │  GetSide(layer, n, Bezier2&) -> bool "is curved"
     ▼
WEDCore/WED_GISUtils  — entity → Segment2/Bezier2 vectors, Polygon2*, BezierPolygon2*
     │                  (+ "p" = int param from gis_Param x, "uv" = parallel UV curve)
     │                  orientation normalization, BezierPoint2 <-> Bezier2 <-> DSF triple
     ▼
WEDCore/WED_Clipping  — AABB clip of segments / polygons-with-holes (4 half-plane passes)
Utils/CompGeomUtils   — CoordTranslator2, SimplifyPolygonMaxMove, libtess2 boolean ops
Utils/GISUtils        — degrees<->meters, headings, Quad_* rectangle helpers, GeoTIFF corners
Utils/CompGeomDefs2   — Point2/Vector2/Segment2/Line2/Bbox2/Polygon2/Bezier2 + predicates
```

**Bezier representations.** Four forms carry the same curve, and conversion between them is
where bugs cluster:
1. **Entity form.** `WED_GISPoint_Bezier` stores `ctrl_lat/lon_lo/hi` as degree offsets plus an
   `is_split` flag. When not split, `SetControlHandleLo` mirrors the opposite handle.
2. **Side form.** `Bezier2 {p1, c1, c2, p2}` in absolute coordinates, as returned by `GetSide`.
   Missing handles are filled with the endpoint, so a straight side `is_segment()`.
3. **Node form.** `BezierPoint2 {lo, pt, hi}`, with `lo == pt` / `hi == pt` meaning no handle.
   `BezierToBezierPointSeq` emits a closed ring with the first node **duplicated at the end**,
   and DSF export `pop_back()`s it.
4. **DSF triple form.** Coincident mirrored points (see above).

The `…2p` and `…2uv` variants (in `WED_GISUtils.h`) carry a per-side integer parameter or a
parallel UV curve. Their `subcurve`, reversal and clipping keep those in sync with the geometry.
The parameter comes from the `gis_Param` layer's `p1.x()`, truncated to `int`.

**Map projection** (details belong to [wed-map-and-tce.md](wed-map-and-tce.md)).
`WED_MapZoomerNew::LLToPixel` / `PixelToLL`:
- are an equirectangular-style projection at the view center with GRS80 scale;
- blend to gnomonic when zoomed in (`USE_GNOMONIC`, `THR_GNOMONIC`), so that great circles are
  straight;
- blend to Wagner when zoomed out to world scale.

The per-axis `XPixelToLon` / `LonToXPixel` family is exact only near the map center, per the
header warning. Code that projects lat/lon beziers for drawing projects the four control points
and then evaluates in pixel space (`PointSequenceToVector`), which is an approximation under the
non-linear projections.

**GeoTIFF / orthophotos.** `FetchTIFFCorners` returns corners in the order SW, SE, NW, NE
(lon before lat), adjusted between pixel-is-area and pixel-is-point via `post_pos`. For images
larger than 1536 px it also fills a `gcp_t` grid of up to 11×11 control points.
`WED_Orthophoto.cpp` interpolates that grid bilinearly (`Interpolation.h`) to warp imports.
`UTMToLonLat` uses the Clarke 1866 ellipsoid and is marked "completely untested… actually dead code".

## Connections to Other Systems

- [wed-entities.md](wed-entities.md): GIS entity classes (`WED_GISChain::GetSide`,
  `WED_GISPoint_Bezier` handle storage, `Rotate`/`Rescale`) are the producers of every
  `Bezier2` here.
- [wed-map-and-tce.md](wed-map-and-tce.md): `WED_MapZoomerNew` projection, `WED_DrawUtils`
  bezier flattening and libtess fill, tool heading readouts, click radius.
- [wed-validation.md](wed-validation.md): winding validation, bezier self-intersection
  (`Bezier2::self_intersect(10)`, `intersect(…, 10)`), runway heading vs `MagneticDeviation`.
- [wed-import-export.md](wed-import-export.md): DSF export clipping (`clip_polygon`,
  `gentle_crop` clamping to tile bounds), triple conversion, orthophoto GCP warping.
- [wed-core-services.md](wed-core-services.md): `WED_Clipping` and `WED_GISUtils` live in
  WEDCore. `WED_GroupCommands` (mow-grass/tyler heuristics) is the main user of the libtess2
  boolean ops and `SimplifyPolygonMaxMove`.
- [dsf-library-and-dsftool.md](dsf-library-and-dsftool.md): DSF's own point pools and
  quantization are separate. DSFTool uses none of this geometry code.
- [bitmap-texture-and-ddstool.md](bitmap-texture-and-ddstool.md): `BitmapUtils` uses
  `Interpolation.h` for resampling. DDSTool pulls in only `MathUtils.h`.
- [utils-platform-and-files.md](utils-platform-and-files.md): the rest of `src/Utils`.
