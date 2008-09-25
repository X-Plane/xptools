/*
 * Copyright (c) 2005, Laminar Research.
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
#ifndef OBJCONVERT_H
#define OBJCONVERT_H

struct XObj;
struct XObj8;

// OBJECT CONVERSION
// These convert from one obj to the other.  This can take overloaded
// tris/quads/lines in the OBJ7 but generates separate tris/quads/lines
// on convert-back.
void	Obj7ToObj8(const XObj& obj7, XObj8& obj8);
void	Obj8ToObj7(const XObj8& obj8, XObj& obj7);

// This merges consecutive index commands in an OBJ8 for you.
void	Obj8_ConsolidateIndexCommands(XObj8& obj8);
// This calculates OBJ8 normals frmo tris, editing the point pool.
void	Obj8_CalcNormals(XObj8& obj8);

bool	Obj8_Optimize(XObj8& obj8);

#endif