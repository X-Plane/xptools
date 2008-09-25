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

#ifndef WED_AptIE_H
#define WED_AptIE_H

class	WED_Thing;
class	WED_Archive;
class	IResolver;

void	WED_AptImport(
				WED_Archive *	archive,
				WED_Thing *		container,
				const char *	file_path);

void	WED_AptExport(
				WED_Thing *		container,
				const char *	file_path);

int		WED_CanExportApt(IResolver * resolver);
void	WED_DoExportApt(IResolver * resolver);

int		WED_CanImportApt(IResolver * resolver);
void	WED_DoImportApt(IResolver * resolver, WED_Archive * archive);

bool	WED_ValidateApt(IResolver * resolver);

#endif /* WED_AptIE_H */
