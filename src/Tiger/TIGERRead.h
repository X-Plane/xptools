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
#ifndef TIGERREAD_H
#define TIGERREAD_H

#include "MapDefsCGAL.h"

#include "TIGERTypes.h"
#include "ProgressUtils.h"
extern	ChainInfoMap		gChains;
extern	LandmarkInfoMap		gLandmarks;
extern	PolygonInfoMap		gPolygons;

struct	MFFileSet;

// File loading routines.  Call them in the order they are listed in to
// get correct hashing!

void	TIGER_LoadRT1(MFFileSet * inSet, int inFileNumber);	// Chain Info
void	TIGER_LoadRT2(MFFileSet * inSet, int inFileNumber);	// Chain Shapes

void	TIGER_LoadRTP(MFFileSet * inSet, int inFileNumber);	// Polygon Water + Center Info
void	TIGER_LoadRTI(MFFileSet * inSet, int inFileNumber);	// Polygon Border Info

void	TIGER_LoadRT7(MFFileSet * inSet, int inFileNumber);	// Landmarks
void	TIGER_LoadRT8(MFFileSet * inSet, int inFileNumber);	// Polygon Landmarks

// Once you have loaded all of the TIGER/Line files you want, call this
// once.  This routine rebuilds the topological relationships between the
// files, so it should only be done once after all adjacent counties are read in.
void	TIGER_EliminateZeroLengthShapePoints(void);
void	TIGER_RoughCull(double inWest, double inSouth, double inEast, double inNorth);
void	TIGER_PostProcess(Pmwx&	outMap);

#endif
