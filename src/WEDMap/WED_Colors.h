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

#ifndef WED_Colors_H
#define WED_Colors_H

enum WED_Color {
	wed_Map_Matte,
	wed_Map_Gridlines,
	wed_Map_Bkgnd,

	wed_Structure,
	wed_StructureSelected,
	wed_StructureLocked,
	wed_StructureLockedSelected,

	wed_ControlHandle,
	wed_Link,
	wed_ControlLink,
	wed_GhostLink,
	wed_Marquee,

	wed_Surface_Asphalt,
	wed_Surface_Concrete,
	wed_Surface_Grass,
	wed_Surface_Dirt,
	wed_Surface_Gravel,
	wed_Surface_DryLake,
	wed_Surface_Water,
	wed_Surface_Snow,
	wed_Surface_Transparent,

	wed_Table_Gridlines,
	wed_Table_Text,
	wed_Table_Select,
	wed_Table_SelectText,
	wed_Table_Drag_Insert,
	wed_Table_Drag_Into,
	wed_Header_Text,
	wed_Tabs_Text,
	wed_PropertyBar_Text,

	wed_TextField_Bkgnd,
	wed_TextField_Text,
	wed_TextField_Hilite,
	wed_TextField_FocusRing,

	wed_Last
};

float *		WED_Color_RGBA		(WED_Color c);
float *		WED_Color_RGBA_Alpha(WED_Color c, float alpha, float storage[4]);
float *		WED_Color_Surface	(int surface, float alpha, float storage[4]);


#endif /* WED_Colors_H */
