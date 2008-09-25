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

#ifndef APTIO_H
#define APTIO_H

struct XAtomContainer;

#include "AptDefs.h"
#include <set>

//void	WriteApts(FILE * fi, const AptVector& inApts);
bool	ReadApts(XAtomContainer& container, AptVector& outApts);

string	ReadAptFile(const char * inFileName, AptVector& outApts);
string	ReadAptFileMem(const char * inBegin, const char * inEnd, AptVector& outApts);
bool	WriteAptFile(const char * inFileName, const AptVector& outApts);
bool	WriteAptFileOpen(FILE * inFile, const AptVector& outApts);

void	IndexAirports(const AptVector& apts, AptIndex& index);
void	FindAirports(const Bbox2& bounds, const AptIndex& index, set<int>& apts);

void	ConvertForward(AptInfo_t& io_apt);

#if OPENGL_MAP
void	GenerateOGL(AptInfo_t * apt);
#endif

#endif
