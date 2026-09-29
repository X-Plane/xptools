/* 
 * Copyright (c) 2014, Laminar Research.
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

#ifndef WED_SceneryPackExport_H
#define WED_SceneryPackExport_H

class	WED_Thing;
class	WED_Group;
class	IResolver;
class 	WED_MapPane;
class	WED_Document;
struct	validation_error_t;

void	WED_ExportPackToPath(WED_Thing * root, IResolver * resolver, const string& in_path, set<WED_Thing *>& problem_children);

// Top level commands for WED.
int		WED_CanExportPack(IResolver * resolver, string& ioname);
// Returns false if validation errors stopped the export.  skipErrorDialog and out_msgs are passed to WED_ValidateApt
// (for automation: no results window, messages returned instead).
bool	WED_DoExportPack(WED_Document * resolver, WED_MapPane * pane, bool skipErrorDialog = false, vector<validation_error_t> * out_msgs = NULL);

#endif /* WED_SceneryPackExport_H */