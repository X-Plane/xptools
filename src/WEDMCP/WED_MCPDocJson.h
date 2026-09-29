/*
 * Copyright (c) 2026, Laminar Research.
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

#ifndef WED_MCPDocJson_H
#define WED_MCPDocJson_H

#include <json/json.h>

class	WED_Thing;
class	WED_Archive;

/*
	WED_MCPDocJson - the document as JSON, and back

	One schema serves both dump_document and inject_fixture, so an agent can dump, edit and re-inject:

	{ "id": 12, "class": "WED_Runway",
	  "props":    { "<xml element>.<xml attribute>": value, ... },
	  "extra":    { ... },        state that isn't a property: airport "meta_data", airport chain "closed"
	  "sources":  [ id | "ref" ], objects this one watches (taxi route edges -> their nodes, etc.)
	  "ref":      "o7",           name other objects in the same dump/fixture use in "sources"
	  "children": [ ... ] }

	Property keys are the persisted earth.wed.xml names, which never change (they're a file format).  Values are raw
	stored values: enums as their persisted description strings, lengths always in meters whatever the UI units.

	The dump reads each WED_PropertyItem directly, not through IPropertyObject, because entities override the
	IPropertyObject view to show synthetic, derived or unit-converted values for the property pane.  Inject writes
	them back the same way (like loading earth.wed.xml does), through SetProperty so every change is undo-captured.
*/

struct	WED_MCPDumpOptions {
	int				max_depth = -1;		// -1: unlimited.  Objects cut off report child_count instead of children.
	bool			ids = true;			// false: omit IDs; sources inside the dump become refs ("o<n>")
	bool			geo = false;		// add derived GIS info (class, bounds) - informational, ignored by inject
	bool			file_precision = false;	// round doubles to the decimals earth.wed.xml keeps, so a dump
											// compares equal across save + reopen (reading XML isn't bit-exact)
};

// Dump root and its subtree.  out_names maps the ID of every dumped object to how the dump names it: its id, or
// its ref when opts.ids is false - so callers can report selections etc. the same way.
Json::Value	WED_MCP_DumpTree(WED_Thing * root, const WED_MCPDumpOptions& opts, map<int, Json::Value>& out_names);

struct	WED_MCPError {
	string			code;
	string			message;
	Json::Value		extra;
};

// Create the objects (an array in dump format, no ids) as children of parent, starting at child index position.
// MUST be called inside an open undo command; on failure the caller aborts it, so nothing partial survives.
// Fills out_refs with ref -> new ID, and out_ids with the IDs of the top-level objects created.
bool	WED_MCP_InjectObjects(WED_Thing * parent, int position, const Json::Value& objects,
								map<string,int>& out_refs, vector<int>& out_ids, WED_MCPError& err);

// Set properties ({key: value}) on one object.  MUST be called inside an open undo command.
bool	WED_MCP_SetProperties(WED_Thing * thing, const Json::Value& props, WED_MCPError& err);

// Non-persisted property helpers (map tool settings), keyed by their display names.  No undo involved.
class	WED_PropertyHelper;
Json::Value	WED_MCP_DumpDisplayProperties(WED_PropertyHelper * obj);
bool	WED_MCP_SetDisplayProperties(WED_PropertyHelper * obj, const char * what, const Json::Value& props, WED_MCPError& err);

#endif /* WED_MCPDocJson_H */
