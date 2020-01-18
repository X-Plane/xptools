/*
 * Copyright (c) 2009, Laminar Research.
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

#ifndef WED_DSFImport_H
#define WED_DSFImport_H

#include "CompGeomDefs2.h"

class	WED_Thing;
class	IResolver;

// The parent object for DSF_Import really really really should be some kind of composite
// like WED_Group or WED_Airport; however this API lets you specify any base and hopes
// you know what you're doing. This is because WED_GISComposite is sort of an implementation
// intermediate and thus kind of weird to have in a public API.

int		WED_CanImportDSF(IResolver * resolver);
void	WED_DoImportDSF(IResolver * resolver);
int		WED_CanImportRoads(IResolver * resolver);
void	WED_DoImportRoads(IResolver * resolver);

int 	DSF_Import(const char * file, WED_Thing * base);
void	WED_ImportText(const char * path, WED_Thing * base);


enum dsf_filter_category {
	dsf_filter_exclusion = 1,
	dsf_filter_objects = 2,
	dsf_filter_facades = 4,
	dsf_filter_forests = 8,
	dsf_filter_lines = 16,
	dsf_filter_strings = 32,
	dsf_filter_orthophoto = 64,
	dsf_filter_draped_poly = 128,
	dsf_filter_roads = 256,
	dsf_filter_terrain = 512,
	dsf_filter_all = 1023
};

int		DSF_Import_Partial(const char * path, WED_Thing * base, int inFilter, const Bbox2&  cull_bound = Bbox2(-180,-90,180,90));

#endif /* WED_DSFImport_H */
