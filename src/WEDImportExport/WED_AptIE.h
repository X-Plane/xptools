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

#include "AptDefs.h"

class	WED_Thing;
class	WED_Archive;
class	IResolver;
class	WED_Document;
class	WED_MapPane;

void	WED_AptImport(
				WED_Archive *			archive,
				WED_Thing *				container,
				const char *			file_path,
				AptVector&				apts,
				vector<WED_Thing *> *	out_airports);
				

void	WED_AptExport(
				WED_Thing *		container,
				const char *	file_path);

void	WED_AptExport(
				WED_Thing *		container,
				int (*			print_func)(void *, const char *, ...),
				void *			ref);

// Given a "WED_thing", add it to the apts as needed.
// A little bit dangerous but this can be a good way
// to convert a selection into an aptdefs.h construct.
void	AptExportRecursive(WED_Thing * what, 
				AptVector& apts);


int		WED_CanExportApt(IResolver * resolver);
void	WED_DoExportApt(IResolver * resolver);

int		WED_CanImportApt(IResolver * resolver);
void	WED_DoImportApt(WED_Document * resolver, WED_Archive * archive, WED_MapPane * pane);

// Given a WED_thing, put airports at file path into it - must be called inside an undo operation!
void	WED_ImportOneAptFile(
				const string&			in_path,
				WED_Thing *				in_parent);

#endif /* WED_AptIE_H */
