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
#include "GeoTIFFWrite.h"

#include <stdio.h>

#if USE_TIF
#include <xtiffio.h>
#include <geotiffio.h>
#include <geo_normalize.h>
#endif

#if IBM
#include "GUI_Unicode.h"
#endif

namespace {

#if USE_TIF
TIFF* open_tiff_w(const char* path)
{
#if SUPPORT_UNICODE
    XTIFFInitialize();
    return TIFFOpenW(convert_str_to_utf16(path).c_str(), "w");
#else
    return XTIFFOpen(path, "w");
#endif
}

bool write_geo_keys(TIFF* tif, const GeoTIFFBounds& bb, int width, int height)
{
    // ModelTiepointTag: pixel (0,0) at (west, north).
    double tiepoint[6] = { 0.0, 0.0, 0.0, bb.west, bb.north, 0.0 };
    TIFFSetField(tif, TIFFTAG_GEOTIEPOINTS, 6, tiepoint);

    // ModelPixelScaleTag: (lon-per-px, lat-per-px, 0).
    double pxscale[3] = {
        (bb.east  - bb.west)  / static_cast<double>(width),
        (bb.north - bb.south) / static_cast<double>(height),
        0.0
    };
    TIFFSetField(tif, TIFFTAG_GEOPIXELSCALE, 3, pxscale);

    GTIF* gtif = GTIFNew(tif);
    if (!gtif) return false;
    GTIFKeySet(gtif, GTModelTypeGeoKey,     TYPE_SHORT, 1, ModelTypeGeographic);
    GTIFKeySet(gtif, GTRasterTypeGeoKey,    TYPE_SHORT, 1, RasterPixelIsArea);
    GTIFKeySet(gtif, GeographicTypeGeoKey,  TYPE_SHORT, 1, GCS_WGS_84);
    GTIFKeySet(gtif, GeogAngularUnitsGeoKey,TYPE_SHORT, 1, Angular_Degree);
    GTIFWriteKeys(gtif);
    GTIFFree(gtif);
    return true;
}

bool write_strip_image(const char* path, int width, int height, int channels,
                       int bits_per_sample, int sample_format, int photometric,
                       const GeoTIFFBounds& bb, const void* data,
                       const int16_t* nodata)
{
    TIFF* tif = open_tiff_w(path);
    if (!tif) return false;

    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH,      static_cast<uint32_t>(width));
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH,     static_cast<uint32_t>(height));
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE,   static_cast<uint16_t>(bits_per_sample));
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, static_cast<uint16_t>(channels));
    TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT,    static_cast<uint16_t>(sample_format));
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG,    PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC,     static_cast<uint16_t>(photometric));
    TIFFSetField(tif, TIFFTAG_ORIENTATION,     ORIENTATION_TOPLEFT);
    TIFFSetField(tif, TIFFTAG_COMPRESSION,     COMPRESSION_LZW);
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP,    TIFFDefaultStripSize(tif, 0));

    if (channels == 4 && bits_per_sample == 8)
    {
        // Tell readers the 4th band is alpha (associated).
        uint16_t extra[1] = { EXTRASAMPLE_ASSOCALPHA };
        TIFFSetField(tif, TIFFTAG_EXTRASAMPLES, 1, extra);
    }
    if (nodata)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "%d", static_cast<int>(*nodata));
        TIFFSetField(tif, TIFFTAG_GDAL_NODATA, buf);
    }

    if (!write_geo_keys(tif, bb, width, height))
    {
        XTIFFClose(tif);
        return false;
    }

    const size_t row_bytes = static_cast<size_t>(width) * channels * (bits_per_sample / 8);
    const uint8_t* p = static_cast<const uint8_t*>(data);
    for (int y = 0; y < height; ++y)
    {
        if (TIFFWriteScanline(tif, const_cast<uint8_t*>(p + y * row_bytes), y, 0) < 0)
        {
            XTIFFClose(tif);
            return false;
        }
    }

    XTIFFClose(tif);
    return true;
}
#endif // USE_TIF

} // namespace

bool WriteGeoTIFF_UInt8(const char* path, int width, int height, int channels,
                        const GeoTIFFBounds& bb, const uint8_t* data)
{
#if USE_TIF
    if (!path || !data || width <= 0 || height <= 0) return false;
    if (channels != 1 && channels != 3 && channels != 4) return false;
    const int photometric = (channels == 1) ? PHOTOMETRIC_MINISBLACK
                                            : PHOTOMETRIC_RGB;
    return write_strip_image(path, width, height, channels,
                             8, SAMPLEFORMAT_UINT, photometric,
                             bb, data, nullptr);
#else
    (void)path; (void)width; (void)height; (void)channels; (void)bb; (void)data;
    return false;
#endif
}

bool WriteGeoTIFF_RGBA(const char* path, int width, int height, int channels,
                       const GeoTIFFBounds& bb, const uint8_t* data)
{
    if (channels != 3 && channels != 4) return false;
    return WriteGeoTIFF_UInt8(path, width, height, channels, bb, data);
}

bool WriteGeoTIFF_Int16(const char* path, int width, int height,
                        const GeoTIFFBounds& bb, const int16_t* data,
                        int16_t nodata)
{
#if USE_TIF
    if (!path || !data || width <= 0 || height <= 0) return false;
    return write_strip_image(path, width, height, /*channels*/ 1,
                             16, SAMPLEFORMAT_INT, PHOTOMETRIC_MINISBLACK,
                             bb, data, &nodata);
#else
    (void)path; (void)width; (void)height; (void)bb; (void)data; (void)nodata;
    return false;
#endif
}
