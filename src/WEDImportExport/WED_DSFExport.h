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

#ifndef WED_DSFExport_H
#define WED_DSFExport_H

class	IResolver;
class	WED_Thing;
struct	DSF_export_info_t;

// You will need the IResolver in case you're handling a orthophoto
int DSF_Export(WED_Thing * base, IResolver * resolver, const string& in_package, set<WED_Thing *>& problem_items);
int DSF_ExportTile(WED_Thing * base, IResolver * resolver, const string& pkg, int x, int y, set <WED_Thing *>& problem_children, DSF_export_info_t * export_info = nullptr);

// 
int DSF_ExportText(IResolver * resolver, WED_Thing * who, const string& package, set<WED_Thing *>& problem_children);


#endif /* WED_DSFExport_H */
