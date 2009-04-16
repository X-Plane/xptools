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
#ifndef MAPIO_H
#define MAPIO_H

#include "IODefs.h"
#include "MapDefs.h"
#include "ProgressUtils.h"
#include "EnumSystem.h"

/*
	MAP STORAGE FORMAT:

	The map is stored as an atom of atoms.  The subatoms are one for the map itself,
	and a series of other atoms containing the data attached to elements.

	The map has an inherent order, the order the items appear in the file, that is
	also used for the other atoms of data.  (Each atom covers one type of entity.)

	The map contains 3 ints, for the vertex, face and halfedge count, then
	for each vertex:
	  two doubles - x and y
	for each halfedge
	  int - vertex index of target
	  four doubles - curve
	for each face
	  int number of outer ccb halfedges, or 0 if unbounded
	     for each outer halfedge, write index
	  int number of holes
	  for each hole
	     int number of half edges on each hole
	     for each inner halfedge, write index
 */

 struct	XAtomContainer;

void	WriteMap(FILE * fi, const 	Pmwx& inMap, ProgressFunc inProgress, int atomID);
void	ReadMap(XAtomContainer& container, Pmwx& inMap, ProgressFunc inProgress, int atomID, const TokenConversionMap& c);

#endif
