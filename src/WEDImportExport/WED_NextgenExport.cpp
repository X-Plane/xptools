/*
 * Copyright (c) 2026, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#include "WED_NextgenExport.h"

#include "WED_DrapedOrthophoto.h"
#include "WED_TerPlacement.h"
#include "WED_ResourceMgr.h"
#include "WED_LibraryMgr.h"
#include "WED_HierarchyUtils.h"
#include "WED_GISUtils.h"
#include "WED_ToolUtils.h"
#include "WED_Version.h"

#include "IGIS.h"
#include "IResolver.h"
#include "ILibrarian.h"

#include "GeoTIFFWrite.h"
#include "ProcessUtils.h"
#include "PolyRasterUtils.h"
#include "BitmapUtils.h"
#include "FileUtils.h"
#include "PlatformUtils.h"
#include "CompGeomDefs2.h"
#include "DEMDefs.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <string>
#include <vector>

// Diagnostic switch. When non-zero, the exporter builds the dataset and
// commits every layer but stops short of `tm_package` / `tm_export` and
// leaves the staging dir in place, so the raw per-tree build can be
// inspected. Default off; set to 1 locally when debugging tile output.
#define WED_NEXTGEN_SKIP_PACKAGE 0

using std::string;
using std::vector;

namespace {

// --- tilemanager pyramid params -------------------------------------------
// Albedo ships at 513 samples/side; elevation at 129. Each mask tree must
// match the content layer it backs (per-pixel alpha).
const int kAlbedoResolution    = 513;
const int kElevationResolution = 129;
// archive_size = tiles-per-side bundled into a single .xta. `package`
// refuses to run if this isn't recorded on the tree at layer_add time
// ("Tree has no package size, will not package").
const int kArchiveSize = 16;

// Spherical-Earth meters-per-sample for a lon/lat-aligned raster. Used to
// pick the densest of several staged inputs so layer_add's --pixel_density
// matches the highest-resolution data we ship.
double approx_meters_per_sample(const GeoTIFFBounds& bb, int w, int h)
{
    const double kDegToRad = 3.14159265358979323846 / 180.0;
    const double kMPerDeg  = 111320.0;
    const double mid_lat   = 0.5 * (bb.north + bb.south);
    const double m_lat = (bb.north - bb.south) * kMPerDeg;
    const double m_lon = (bb.east  - bb.west)  * kMPerDeg *
                         std::cos(mid_lat * kDegToRad);
    const double per_y = m_lat / std::max(1, h);
    const double per_x = m_lon / std::max(1, w);
    return std::min(per_x, per_y);
}

// --- path helpers ---------------------------------------------------------

string join_path(const string& a, const string& b)
{
    if (a.empty()) return b;
    if (a.back() == DIR_CHAR || a.back() == '/') return a + b;
    return a + DIR_STR + b;
}

// Sanitize an entity name into a filename-safe slug.
string safe_name(const string& raw)
{
    string out;
    out.reserve(raw.size());
    for (char c : raw)
    {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.')
            out.push_back(c);
        else
            out.push_back('_');
    }
    if (out.empty()) out = "unnamed";
    return out;
}

// Resolve the tilemanager binary: look next to the WED executable first,
// then fall through to a bare name (PATH lookup happens in the spawn).
// `found_on_disk` is set true iff we located the binary by absolute path
// (so no PATH lookup is needed and the caller can skip the launch probe).
string find_tilemanager(bool& found_on_disk)
{
    found_on_disk = false;
    string app = GetApplicationPath();
    string dir = FILE_get_dir_name(app);
#if IBM
    string candidate = join_path(dir, "tilemanager.exe");
#else
    string candidate = join_path(dir, "tilemanager");
#endif
    if (FILE_exists(candidate.c_str()))
    {
        found_on_disk = true;
        return candidate;
    }
#if IBM
    return "tilemanager.exe";
#else
    return "tilemanager";
#endif
}

// Verify that `bin` can actually be launched. Runs `bin help` and treats
// exit code 0 as "tilemanager is alive". Any other outcome (spawn failure,
// shell "command not found" 127, etc.) is reported via `diag_out`, which
// receives a short human-readable description suitable for an alert.
bool probe_tilemanager(const string& bin, string& diag_out)
{
    vector<string> captured;
    int rc = run_subprocess(bin, { "help" }, [&](const string& line) {
        if (captured.size() < 12) captured.push_back(line);
    });

    if (rc == 0)
        return true;

    diag_out = "Could not run \"" + bin + "\" (exit ";
    char buf[32]; snprintf(buf, sizeof(buf), "%d", rc);
    diag_out += buf;
    diag_out += ").";
    if (!captured.empty())
    {
        diag_out += " Last output:\n";
        for (const auto& l : captured)
        {
            diag_out += "  ";
            diag_out += l;
            diag_out += "\n";
        }
    }
    return false;
}

// Build a log-friendly preview of a command line.
string preview_cmd(const string& bin, const vector<string>& argv)
{
    string out = bin;
    for (const auto& a : argv) { out.push_back(' '); out += a; }
    return out;
}

// Run tilemanager with `argv` (no argv[0]); pipe its log through LOG_MSG and
// flush gLogFile after every line so a tailing user (or a post-mortem on a
// still-running WED) sees live progress instead of a stdio-buffered tail.
int run_tm(const string& bin, const vector<string>& argv)
{
    LOG_MSG("I/nextgen $ %s\n", preview_cmd(bin, argv).c_str());
    LOG_FLUSH();
    int rc = run_subprocess(bin, argv, [](const string& line) {
        LOG_MSG("I/nextgen   %s\n", line.c_str());
        LOG_FLUSH();
    });
    LOG_MSG("I/nextgen   (exit %d)\n", rc);
    LOG_FLUSH();
    return rc;
}

// --- polygon rasterization ------------------------------------------------

// Fill the mask raster (row-major, top-to-bottom, value 0 outside, 255 inside)
// from a polygon described in geographic coordinates.
void rasterize_polygon_to_mask(
    const Polygon2& outer,
    const GeoTIFFBounds& bb,
    int width, int height,
    vector<uint8_t>& out_mask)
{
    out_mask.assign(static_cast<size_t>(width) * height, 0);

    const double dlon = bb.east  - bb.west;
    const double dlat = bb.north - bb.south;
    if (width <= 0 || height <= 0 || dlon <= 0.0 || dlat <= 0.0)
        return;

    PolyRasterizer<double> r;
    auto add_ring = [&](const Polygon2& ring) {
        for (size_t i = 0, n = ring.size(); i < n; ++i)
        {
            const auto& a = ring[i];
            const auto& b = ring[(i + 1) % n];
            // pixel-space: x increases east, y increases south
            double ax = (a.x() - bb.west)  / dlon * width;
            double ay = (bb.north - a.y()) / dlat * height;
            double bx = (b.x() - bb.west)  / dlon * width;
            double by = (bb.north - b.y()) / dlat * height;
            r.AddEdge(ax, ay, bx, by);
        }
    };
    add_ring(outer);
    r.SortMasters();

    int y = 0;
    r.StartScanline(y);
    while (!r.DoneScan())
    {
        int x1, x2;
        while (r.GetRange(x1, x2))
        {
            if (x1 < 0) x1 = 0;
            if (x2 > width) x2 = width;
            if (y >= 0 && y < height && x1 < x2)
                std::memset(&out_mask[static_cast<size_t>(y) * width + x1],
                            255, static_cast<size_t>(x2 - x1));
        }
        ++y;
        if (y >= height) break;
        r.AdvanceScanline(y);
    }
}

// --- per-entity exporters -------------------------------------------------

struct StagePaths
{
    string root;             // <pkg>/tiles  (final export destination)
    string staging;          // <pkg>/tiles/_staging
    string dataset;          // <pkg>/tiles/_staging/dataset  (tilemanager build root)
    string ortho_dir;        // ortho RGB(A) GeoTIFFs
    string dem_dir;          // DEM elevation GeoTIFFs
    string albedo_mask_dir;  // ortho polygon mask GeoTIFFs
    string dem_mask_dir;     // ter polygon mask GeoTIFFs
};

bool make_staging(const string& pkg, StagePaths& out)
{
    out.root            = join_path(pkg, "tiles");
    out.staging         = join_path(out.root, "_staging");
    out.dataset         = join_path(out.staging, "dataset");
    out.ortho_dir       = join_path(out.staging, "ortho");
    out.dem_dir         = join_path(out.staging, "dem");
    out.albedo_mask_dir = join_path(out.staging, "albedo_mask");
    out.dem_mask_dir    = join_path(out.staging, "dem_mask");
    if (FILE_make_dir_exist(out.root.c_str())            != 0) return false;
    if (FILE_make_dir_exist(out.staging.c_str())         != 0) return false;
    if (FILE_make_dir_exist(out.dataset.c_str())         != 0) return false;
    if (FILE_make_dir_exist(out.ortho_dir.c_str())       != 0) return false;
    if (FILE_make_dir_exist(out.dem_dir.c_str())         != 0) return false;
    if (FILE_make_dir_exist(out.albedo_mask_dir.c_str()) != 0) return false;
    if (FILE_make_dir_exist(out.dem_mask_dir.c_str())    != 0) return false;
    return true;
}

// Gather all visible non-library orthos under `root`.
void collect_orthos(WED_Thing* root, IResolver* resolver,
                    vector<WED_DrapedOrthophoto*>& out)
{
    vector<WED_DrapedOrthophoto*> all;
    CollectRecursive(root, std::back_inserter(all),
                     WED_DrapedOrthophoto::sClass);

    WED_LibraryMgr* lmgr = WED_GetLibraryMgr(resolver);
    for (auto* o : all)
    {
        string res;
        o->GetResource(res);
        // Skip library orthos -- they are pre-baked .pol references and
        // there is nothing to tile here.
        if (lmgr && lmgr->IsResourceLibrary(res))
            continue;
        if (!o->IsNew())
            continue;
        out.push_back(o);
    }
}

void collect_terrains(WED_Thing* root, vector<WED_TerPlacement*>& out)
{
    CollectRecursive(root, std::back_inserter(out),
                     WED_TerPlacement::sClass);
}

// Resolve an entity resource path to an absolute file path. WED uses the
// document's librarian to anchor relative paths at the package root.
string resolve_resource_path(IResolver* resolver, const string& rel)
{
    string out = rel;
    if (ILibrarian* lib = WED_GetLibrarian(resolver))
        lib->LookupPath(out);
    return out;
}

bool write_ortho(WED_DrapedOrthophoto* o, IResolver* resolver,
                 const StagePaths& s, int idx,
                 vector<string>& src_files, vector<string>& mask_files,
                 double& out_mpp)
{
    Bbox2 box;
    o->GetBounds(gis_Geo, box);
    GeoTIFFBounds bb {
        box.bottom_left().x(), box.bottom_left().y(),
        box.top_right().x(),   box.top_right().y()
    };

    string res;
    o->GetResource(res);
    string abs = resolve_resource_path(resolver, res);
    if (!FILE_exists(abs.c_str()))
    {
        LOG_MSG("W/nextgen ortho resource not found: %s\n", abs.c_str());
        return false;
    }

    ImageInfo img;
    std::memset(&img, 0, sizeof(img));
    if (LoadBitmapFromAnyFile(abs.c_str(), &img) != 0)
    {
        LOG_MSG("W/nextgen failed to load ortho source: %s\n", abs.c_str());
        return false;
    }

    string entity_name; o->GetName(entity_name);
    string base = safe_name(entity_name);
    char suffix[16]; snprintf(suffix, sizeof(suffix), "_%04d", idx);
    base += suffix;

    string src_path  = join_path(s.ortho_dir,       base + "_rgba.tif");
    string mask_path = join_path(s.albedo_mask_dir, base + "_mask.tif");

    // When WED auto-tiles a large ortho at import, it produces N entities
    // that all point at the same source image, each carving out its own
    // sub-rectangle via the (top, bottom, left, right) properties. UV (0,0)
    // is the source image's bottom-left; (1,1) is its top-right. We must
    // crop to that sub-rect or every tile ends up containing the whole
    // image positioned at a different geo bbox.
    Bbox2 uv;
    o->GetSubTexture(uv);
    double uv_l = std::clamp(uv.p1.x(), 0.0, 1.0);
    double uv_r = std::clamp(uv.p2.x(), 0.0, 1.0);
    double uv_b = std::clamp(uv.p1.y(), 0.0, 1.0);
    double uv_t = std::clamp(uv.p2.y(), 0.0, 1.0);
    if (uv_r <= uv_l) uv_r = uv_l + 1e-9;
    if (uv_t <= uv_b) uv_t = uv_b + 1e-9;

    int src_w = static_cast<int>(img.width);
    int src_h = static_cast<int>(img.height);
    int ch    = img.channels;

    // Source-pixel sub-rectangle. `px_top` is the *higher* image row index
    // because ImageInfo stores y=0 at the bottom.
    auto iround = [](double v) { return static_cast<int>(v + 0.5); };
    int px_l = std::clamp(iround(uv_l * src_w), 0, src_w);
    int px_r = std::clamp(iround(uv_r * src_w), 0, src_w);
    int px_b = std::clamp(iround(uv_b * src_h), 0, src_h);
    int px_t = std::clamp(iround(uv_t * src_h), 0, src_h);
    if (px_r <= px_l || px_t <= px_b)
    {
        LOG_MSG("W/nextgen ortho '%s' has empty UV sub-texture (%g..%g, %g..%g); skipping.\n",
                entity_name.c_str(), uv_l, uv_r, uv_b, uv_t);
        DestroyBitmap(&img);
        return false;
    }
    const int w = px_r - px_l;
    const int h = px_t - px_b;
    const size_t row_bytes_src = static_cast<size_t>(src_w) * ch + img.pad;
    const size_t row_bytes_out = static_cast<size_t>(w)     * ch;

    // Pack the sub-rect into a top-down TIFF buffer. Source row (px_t - 1)
    // is the topmost pixel of the crop; that becomes output row 0.
    vector<uint8_t> packed(row_bytes_out * h);
    for (int y = 0; y < h; ++y)
    {
        int src_y = (px_t - 1) - y;
        const uint8_t* src_row = img.data
            + static_cast<size_t>(src_y) * row_bytes_src
            + static_cast<size_t>(px_l)  * ch;
        std::memcpy(packed.data() + static_cast<size_t>(y) * row_bytes_out,
                    src_row, row_bytes_out);
    }

    // ImageInfo stores BGRA/BGR; WriteGeoTIFF_RGBA declares PHOTOMETRIC_RGB
    // and writes bytes verbatim, so swizzle channels 0 and 2 before the
    // write. Works for both ch==3 and ch==4 (alpha at index 3 is untouched).
    for (size_t i = 0; i + 2 < packed.size(); i += ch)
        std::swap(packed[i], packed[i + 2]);

    bool ok_src = WriteGeoTIFF_RGBA(src_path.c_str(), w, h,
                                    (ch == 4 ? 4 : 3), bb,
                                    packed.data());
    DestroyBitmap(&img);
    if (!ok_src)
    {
        LOG_MSG("E/nextgen failed to write ortho GeoTIFF: %s\n",
                src_path.c_str());
        return false;
    }

    Polygon2 outer;
    if (IGISPointSequence* ps = o->GetOuterRing())
        WED_PolygonForPointSequence(ps, outer, COUNTERCLOCKWISE);

    // Mask is at the cropped sub-rect resolution, anchored at the same geo
    // bbox as the source GeoTIFF.
    vector<uint8_t> mask;
    rasterize_polygon_to_mask(outer, bb, w, h, mask);

    if (!WriteGeoTIFF_UInt8(mask_path.c_str(), w, h, 1, bb, mask.data()))
    {
        LOG_MSG("E/nextgen failed to write ortho mask: %s\n",
                mask_path.c_str());
        return false;
    }

    src_files.push_back(src_path);
    mask_files.push_back(mask_path);
    out_mpp = approx_meters_per_sample(bb, w, h);
    return true;
}

bool write_terrain(WED_TerPlacement* t, IResolver* resolver,
                   const StagePaths& s, int idx,
                   vector<string>& dem_files, vector<string>& mask_files,
                   double& out_mpp)
{
    string dem_res;
    t->GetResource(dem_res);
    WED_ResourceMgr* rmgr = WED_GetResourceMgr(resolver);
    const dem_info_t* dem_info = nullptr;
    if (!rmgr || !rmgr->GetDem(dem_res, dem_info) || !dem_info)
    {
        LOG_MSG("W/nextgen ter placement: DEM not loadable: %s\n",
                dem_res.c_str());
        return false;
    }

    const DEMGeo& dem = *dem_info;
    GeoTIFFBounds dem_bb {
        dem.mWest, dem.mSouth, dem.mEast, dem.mNorth
    };

    string entity_name; t->GetName(entity_name);
    string base = safe_name(entity_name);
    char suffix[16]; snprintf(suffix, sizeof(suffix), "_%04d", idx);
    base += suffix;

    string dem_path  = join_path(s.dem_dir,      base + "_elev.tif");
    string mask_path = join_path(s.dem_mask_dir, base + "_mask.tif");

    int w = dem.mWidth, h = dem.mHeight;

    // Rasterize the ter polygon onto the DEM grid. The same mask is reused
    // for (a) clipping the DEM via nodata as a safety net inside the GeoTIFF
    // and (b) feeding the shared tilemanager mask tree as a separate file so
    // the elevation layer's mask binding will skip clear tiles.
    Polygon2 outer;
    if (IGISPointSequence* ps = t->GetOuterRing())
        WED_PolygonForPointSequence(ps, outer, COUNTERCLOCKWISE);
    vector<uint8_t> mask;
    rasterize_polygon_to_mask(outer, dem_bb, w, h, mask);

    const int16_t kNoData = INT16_MIN;
    const float clip = static_cast<float>(t->MSLClip());
    vector<int16_t> elev(static_cast<size_t>(w) * h);
    for (int row = 0; row < h; ++row)
    {
        // DEMGeo: y=0 is south, y=mHeight-1 is north. TIFF row 0 = north.
        int dy = h - 1 - row;
        for (int x = 0; x < w; ++x)
        {
            size_t mi = static_cast<size_t>(row) * w + x;
            if (mask[mi] == 0)
            {
                elev[mi] = kNoData;
                continue;
            }
            float v = dem.mData[x + dy * w];
            if (v == DEM_NO_DATA || v <= clip)
            {
                elev[mi] = kNoData;
                mask[mi] = 0;
            }
            else
            {
                if (v > 32767.f)  v =  32767.f;
                if (v < -32767.f) v = -32767.f;
                elev[mi] = static_cast<int16_t>(v);
            }
        }
    }
    if (!WriteGeoTIFF_Int16(dem_path.c_str(), w, h, dem_bb,
                            elev.data(), kNoData))
    {
        LOG_MSG("E/nextgen failed to write DEM GeoTIFF: %s\n", dem_path.c_str());
        return false;
    }
    if (!WriteGeoTIFF_UInt8(mask_path.c_str(), w, h, 1, dem_bb, mask.data()))
    {
        LOG_MSG("E/nextgen failed to write DEM mask: %s\n", mask_path.c_str());
        return false;
    }

    dem_files.push_back(dem_path);
    mask_files.push_back(mask_path);
    out_mpp = approx_meters_per_sample(dem_bb, w, h);
    return true;
}

// --- tilemanager driver --------------------------------------------------

string res_str(int n)
{
    char b[16];
    snprintf(b, sizeof(b), "%d", n);
    return b;
}
string archive_size_str()
{
    char b[16];
    snprintf(b, sizeof(b), "%d", kArchiveSize);
    return b;
}

int tm_create(const string& bin, const string& tiles, const string& pack_name)
{
    return run_tm(bin, {
        "create",
        "--name",      pack_name,
        "--copyright", "WED",
        "--author",    "WED",
        tiles
    });
}

int tm_layer_add_mask(const string& bin, const string& tiles, const string& uri,
                      const string& density_ref_path, int resolution)
{
    return run_tm(bin, {
        "layer_add",
        "--usage",         "mask",
        "--uri",           uri,
        "--resolution",    res_str(resolution),
        "--data_type",     "int8u",
        "--pixel_density", density_ref_path,
        "--archive_size",  archive_size_str(),
        tiles
    });
}

int tm_add_uri(const string& bin, const string& tiles, const string& uri,
               const vector<string>& inputs, bool with_alpha)
{
    vector<string> argv = { "add", "--uri", uri };
    if (with_alpha) { argv.push_back("--alpha"); argv.push_back("channel"); }
    argv.push_back(tiles);
    for (const auto& f : inputs) argv.push_back(f);
    return run_tm(bin, argv);
}

int tm_commit_uri(const string& bin, const string& tiles, const string& uri)
{
    vector<string> argv = { "commit", "--uri", uri };
#if DEV
    argv.push_back("--debug_keep_sources");
#endif
    argv.push_back(tiles);
    return run_tm(bin, argv);
}

int tm_layer_add_albedo(const string& bin, const string& tiles,
                        const string& mask_uri,
                        const string& density_ref_path)
{
    return run_tm(bin, {
        "layer_add",
        "--usage",          "albedo",
        "--resolution",     res_str(kAlbedoResolution),
        "--data_type",      "int8u",
        "--pixel_density",  density_ref_path,
        "--channels",       "4",
        "--archive_size",   archive_size_str(),
        "--use_alpha_mask", mask_uri,
        tiles
    });
}

int tm_layer_add_elevation(const string& bin, const string& tiles,
                           const string& mask_uri,
                           const string& density_ref_path)
{
    return run_tm(bin, {
        "layer_add",
        "--usage",          "elevation",
        "--resolution",     res_str(kElevationResolution),
        "--data_type",      "int16s",
        "--pixel_density",  density_ref_path,
        "--archive_size",   archive_size_str(),
        "--use_alpha_mask", mask_uri,
        tiles
    });
}

int tm_add_usage(const string& bin, const string& tiles, const string& usage,
                 const vector<string>& inputs, bool with_alpha)
{
    vector<string> argv = { "add", "--usage", usage };
    if (with_alpha) { argv.push_back("--alpha"); argv.push_back("channel"); }
    argv.push_back(tiles);
    for (const auto& f : inputs) argv.push_back(f);
    return run_tm(bin, argv);
}

int tm_commit_usage(const string& bin, const string& tiles, const string& usage)
{
    vector<string> argv = { "commit", "--usage", usage };
#if DEV
    argv.push_back("--debug_keep_sources");
#endif
    argv.push_back(tiles);
    return run_tm(bin, argv);
}

int tm_package(const string& bin, const string& tiles)
{
    return run_tm(bin, { "package", tiles });
}

// Copy the read-side files (index.json, subtrees, archives) of a packaged
// dataset into `out`. Refuses if any tree still has pending tiles or has not
// been packaged -- which gives us a free invariant check at the boundary.
int tm_export(const string& bin, const string& dataset, const string& out)
{
    return run_tm(bin, { "export", dataset, out });
}

} // namespace

int WED_ExportNextgenTiles(WED_Thing* world, IResolver* resolver,
                           const string& pkg)
{
    if (!world || !resolver) return -1;

    // First: figure out whether there is anything to tile at all. If not,
    // do nothing -- including no tilemanager probe and no staging dir.
    vector<WED_DrapedOrthophoto*> orthos;
    collect_orthos(world, resolver, orthos);
    vector<WED_TerPlacement*> terrains;
    collect_terrains(world, terrains);

    if (orthos.empty() && terrains.empty())
    {
        LOG_MSG("I/nextgen no orthos or ter placements to tile; skipping.\n");
        LOG_FLUSH();
        return 0;
    }

    // Pre-flight: tilemanager must be reachable before we stage anything.
    // If the binary lives right next to WED we trust the on-disk check; if
    // we are relying on a PATH lookup, actually launch it once to confirm.
    bool found_on_disk = false;
    string bin = find_tilemanager(found_on_disk);
    LOG_MSG("I/nextgen using tilemanager: %s%s\n",
            bin.c_str(),
            found_on_disk ? " (next to WED)" : " (PATH lookup)");
    LOG_FLUSH();

    if (!found_on_disk)
    {
        string diag;
        if (!probe_tilemanager(bin, diag))
        {
            LOG_MSG("E/nextgen tilemanager probe failed: %s\n", diag.c_str());
            LOG_FLUSH();
            string msg =
                "Next-gen export requires the \"tilemanager\" command-line "
                "tool, but it could not be launched.\n\n"
                "Install tilemanager next to WED, or add it to your PATH, "
                "then try again.\n\n";
            msg += diag;
            DoUserAlert(msg.c_str());
            return -1;
        }
    }

    StagePaths s;
    if (!make_staging(pkg, s))
    {
        DoUserAlert("Failed to create tiles/ staging directories.");
        return -1;
    }

    // Staging produces albedo and DEM polygon masks separately so each
    // content layer can supply only its own footprint for in-tile alpha
    // composition. The shared mask tree (see below) is the union of both.
    vector<string> ortho_src, albedo_mask_files;
    vector<string> dem_src,   dem_mask_files;
    // Track the densest (smallest mpp) staged input per layer; we hand its
    // path to layer_add --pixel_density so the pyramid is deep enough to
    // preserve the finest-resolution input.
    string ortho_density_ref;
    double ortho_density_mpp = 0.0;
    string dem_density_ref;
    double dem_density_mpp = 0.0;
    int idx = 0;
    for (auto* o : orthos)
    {
        double mpp = 0.0;
        if (write_ortho(o, resolver, s, idx++, ortho_src, albedo_mask_files, mpp))
        {
            if (ortho_density_ref.empty() || mpp < ortho_density_mpp)
            {
                ortho_density_ref = ortho_src.back();
                ortho_density_mpp = mpp;
            }
        }
    }
    idx = 0;
    for (auto* t : terrains)
    {
        double mpp = 0.0;
        if (write_terrain(t, resolver, s, idx++, dem_src, dem_mask_files, mpp))
        {
            if (dem_density_ref.empty() || mpp < dem_density_mpp)
            {
                dem_density_ref = dem_src.back();
                dem_density_mpp = mpp;
            }
        }
    }

    if (ortho_src.empty() && dem_src.empty())
    {
        DoUserAlert("Nextgen export: no usable orthos or ter placements were "
                    "staged (see log for details).");
        return -1;
    }

    string pack_name = FILE_get_file_name(pkg);
    if (pack_name.empty()) pack_name = "nextgen_pack";

    if (tm_create(bin, s.dataset, pack_name) != 0) goto fail;

    // Two independent mask trees, one per content layer. Each is built only
    // from its own polygon footprints, so albedo and elevation can be
    // clipped to different shapes without leaking into each other.
    if (!albedo_mask_files.empty())
    {
        if (tm_layer_add_mask(bin, s.dataset, "albedo_mask", ortho_density_ref, kAlbedoResolution) != 0) goto fail;
        if (tm_add_uri(bin, s.dataset, "albedo_mask", albedo_mask_files, true) != 0) goto fail;
        if (tm_commit_uri(bin, s.dataset, "albedo_mask") != 0) goto fail;
    }
    if (!dem_mask_files.empty())
    {
        if (tm_layer_add_mask(bin, s.dataset, "dem_mask", dem_density_ref, kElevationResolution) != 0) goto fail;
        if (tm_add_uri(bin, s.dataset, "dem_mask", dem_mask_files, true) != 0) goto fail;
        if (tm_commit_uri(bin, s.dataset, "dem_mask") != 0) goto fail;
    }
    if (!ortho_src.empty())
    {
        if (tm_layer_add_albedo(bin, s.dataset, "albedo_mask", ortho_density_ref) != 0) goto fail;
        if (tm_add_usage(bin, s.dataset, "albedo", ortho_src, false) != 0) goto fail;
        if (tm_add_usage(bin, s.dataset, "albedo", albedo_mask_files, true) != 0) goto fail;
        if (tm_commit_usage(bin, s.dataset, "albedo") != 0) goto fail;
    }
    if (!dem_src.empty())
    {
        if (tm_layer_add_elevation(bin, s.dataset, "dem_mask", dem_density_ref) != 0) goto fail;
        if (tm_add_usage(bin, s.dataset, "elevation", dem_src, false) != 0) goto fail;
        if (tm_add_usage(bin, s.dataset, "elevation", dem_mask_files, true) != 0) goto fail;
        if (tm_commit_usage(bin, s.dataset, "elevation") != 0) goto fail;
    }
#if WED_NEXTGEN_SKIP_PACKAGE
    // Diagnostic build: skip `package` + `export` and keep the staging dir
    // so the per-tree build state can be inspected on disk.
    LOG_MSG("I/nextgen WED_NEXTGEN_SKIP_PACKAGE set: skipping package/export, "
            "keeping staging at %s\n", s.staging.c_str());
#else
    if (tm_package(bin, s.dataset) != 0) goto fail;

    // Copy the shippable read-side files out of the build dataset into the
    // final <pkg>/tiles/ destination. `export` strips the per-tree source/,
    // mask/, and raw .xtd staging that the build accumulated.
    if (tm_export(bin, s.dataset, s.root) != 0) goto fail;

    // Success: clean up staging (which contains the dataset build root).
    // In DEV builds, keep staging so the WED-written GeoTIFFs and the
    // tilemanager dataset build dir remain available for inspection.
#if DEV
    LOG_MSG("I/nextgen DEV build: keeping staging at %s\n", s.staging.c_str());
#else
    FILE_delete_dir_recursive(s.staging);
#endif
#endif
    LOG_MSG("I/nextgen export complete.\n");
    LOG_FLUSH();
    return 0;

fail:
    LOG_FLUSH();
    DoUserAlert("tilemanager invocation failed during nextgen export. "
                "Inspect the log for details; staging files have been kept "
                "in <pack>/tiles/_staging/ (including the dataset build under "
                "_staging/dataset/) for diagnostics.");
    return -1;
}
