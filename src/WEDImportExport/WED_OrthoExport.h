/*
 * Copyright (c) 2007, Laminar Research.
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

#ifndef WED_OrthoExport_H
#define WED_OrthoExport_H

class	WED_DrapedOrthophoto;
class	WED_TerPlacement;
class	WED_Thing;
class	IResolver;
class 	WED_Document;
typedef struct DEMGeo dem_info_t;

#include "BitmapUtils.h"

class DSF_export_info_t
{
public:
	ImageInfo	orthoImg;      // in case an orthoimage is to be converted/exported, store its info, so it does not need to be loaded it repeatedly
	string		orthoFile;     // path to last orthoImage - so we know if there is a 2nd one to deal with - in which case we drop the first

	bool		DockingJetways;
	bool		resourcesAdded;
private:
	set<string> previous_dsfs;
	string		new_dsfs;
	WED_Document* inDoc;
public:
	DSF_export_info_t(IResolver* resolver = nullptr);
	~DSF_export_info_t(void);
	void mark_written(const string& file);
};

int WED_ExportOrtho(WED_DrapedOrthophoto* orth, IResolver* resolver, const string& pkg, DSF_export_info_t* export_info, string &r);

int WED_ExportTerrObj(WED_TerPlacement* ter, IResolver* resolver, const string& pkg, string &resource);

bool WED_ExtractGeoTiff(dem_info_t& inMap, const char* inFileName, int post_style);

#endif /* WED_OrthoExport_H */
