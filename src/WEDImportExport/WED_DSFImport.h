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

class	WED_Thing;
class	IResolver;
class	ILibrarian;

// The parent object for DSF_Import really really really should be some kind of composite
// like WED_Group or WED_Airport; however this API lets you specify any base and hopes
// you know what you're doing. This is because WED_GISComposite is sort of an implementation
// intermediate and thus kind of weird to have in a public API.
int DSF_Import(const char * file, WED_Thing * base);

int		WED_CanImportDSF(IResolver * resolver);
void	WED_DoImportDSF(IResolver * resolver);
int		WED_CanImportRoads(IResolver * resolver);
void	WED_DoImportRoads(IResolver * resolver);
void	WED_DoImportText(const char * path, WED_Thing * base);
#if GATEWAY_IMPORT_FEATURES
void	WED_DoImportDSFText(IResolver * resolver);
#endif

#endif /* WED_DSFImport_H */
