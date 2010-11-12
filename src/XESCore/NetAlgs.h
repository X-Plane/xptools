/* 
 * Copyright (c) 2010, Laminar Research.
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

#ifndef NetAlgs_H
#define NetAlgs_H

#include "ProgressUtils.h"
#include "MapDefs.h"

class	DEMGeo;

void	CalcRoadTypes(Pmwx& ioMap, const DEMGeo& inElevation, const DEMGeo& inUrbanDensity, const DEMGeo& inTemp, const DEMGeo& inRain, ProgressFunc inProg);

#if OPENGL_MAP && DEV
void	debug_network(Pmwx& io_map);
#endif
void	repair_network(Pmwx& io_map);

int score_for_junction(Pmwx::Vertex_handle v);
int optimize_one_junction(Pmwx::Vertex_handle v);

#endif /* NetAlgs_H */
