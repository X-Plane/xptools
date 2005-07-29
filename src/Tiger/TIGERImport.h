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
#ifndef TIGERIMPORT_H
#define TIGERIMPORT_H

#include "TigerTypes.h"
#include "MapDefs.h"
#include "ProgressUtils.h"

/*
 * TIGERImport
 *
 * Given a set of tiger chains, polygons and landmarks that have been
 * topologically sorted, this routine converts them to a single topological
 * map, importing as much metadata as possible.
 *
 */
void	TIGERImport(
			const	ChainInfoMap&		chains,
			const	LandmarkInfoMap&	landmarks,
			const	PolygonInfoMap&		polygons,
			Pmwx&						ioMap,
			ProgressFunc				inProgress);

#endif