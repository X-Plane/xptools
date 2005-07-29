/* 
 * Copyright (c) 2004, Laminar Research.
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
#ifndef WED_MSGS_H
#define WED_MSGS_H

enum {

	wed_Cat_File = 0,
	wed_Cat_Selection
	
};

/* wed_Cat_File */

enum {

	wed_Msg_FileLoaded = 0,		// The whole file was loaded - everything's blown away.
	wed_Msg_RasterChange,		// At least one raster layer has changed - msg is the DEM layer or 0 for unknown
	wed_Msg_VectorChange,		// Vector layer has changed, moving stuff spatially or adding/removing objects.
	wed_Msg_VectorMetaChange,	// Vector map is constant but the meta data per segment may have changed.
	wed_Msg_TriangleHiChange,	// Triangle Mesh is changed in some way.
	wed_Msg_AirportsLoaded		// Airport data has changed.	

};

/* wed_Cat_Selection */

enum {

	wed_Msg_SelectionModeChanged = 0,	// Param is int, true if sel changed too
	wed_Msg_SelectionChanged

};

#endif
