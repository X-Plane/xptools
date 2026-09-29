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

// Document MCP tools: dump the document as JSON, inject fixtures, edit properties and the selection.
// Every edit here is exactly one undoable command, opened and committed (or aborted) by the tool.

#include "WED_MCPTools.h"
#include "WED_MCPDocJson.h"
#include "WED_Document.h"
#include "WED_DocumentWindow.h"
#include "WED_Thing.h"
#include "WED_Airport.h"
#include "WED_Globals.h"
#include "WED_ToolUtils.h"
#include "WED_LibraryMgr.h"
#include "ISelection.h"

const char *	WED_MCP_ExportTargetName(int target);
Json::Value		WED_MCP_DocSummary(WED_Document * doc, WED_DocumentWindow * win, const string& package);

static WED_Thing *	fetch_thing(WED_MCPCall& call, WED_Document * doc, int id)
{
	WED_Thing * t = dynamic_cast<WED_Thing *>(doc->GetArchive()->Fetch(id));
	if (!t)
		call.Error("unknown_object", "No object with ID " + to_string(id) + " in this document.");
	return t;
}

static void	dump_document(WED_MCPCall& call, const Json::Value& args)
{
	WED_Document * doc; WED_DocumentWindow * win; string package;
	if (!WED_MCP_FindDocument(call, args, doc, win, package)) return;

	WED_MCPDumpOptions opts;
	int root_id = 0;
	string path;
	if (!WED_MCP_GetInt(call, args, "root_id", root_id, false)) return;
	if (!WED_MCP_GetInt(call, args, "max_depth", opts.max_depth, false)) return;
	if (!WED_MCP_GetString(call, args, "path", path, false)) return;
	opts.ids = args.get("ids", true).asBool();
	opts.geo = args.get("geo", false).asBool();
	opts.file_precision = args.get("file_precision", false).asBool();

	WED_Thing * root = root_id ? fetch_thing(call, doc, root_id) : WED_GetWorld(doc);
	if (!root) return;

	map<int, Json::Value> names;
	Json::Value r;
	r["package"] = package;
	r["dirty"] = doc->IsDirty();
	r["export_target"] = WED_MCP_ExportTargetName(gExportTarget);
	r["root"] = WED_MCP_DumpTree(root, opts, names);
	r["object_count"] = (int) names.size();

	Json::Value sel_names(Json::arrayValue);
	ISelection * sel = WED_GetSelect(doc);
	vector<ISelectable *> sv;
	sel->GetSelectionVector(sv);
	for(int n = 0; n < sv.size(); ++n)
	{
		map<int, Json::Value>::iterator nm = names.find(sv[n]->GetSelectionID());
		sel_names.append(nm != names.end() ? nm->second : Json::Value(sv[n]->GetSelectionID()));
	}
	r["selection"] = sel_names;

	if (path.empty())
	{
		call.Reply(r);
		return;
	}

	// One property per line, so the file diffs well.
	Json::StreamWriterBuilder b;
	b["indentation"] = " ";
	b["emitUTF8"] = true;
	b["precision"] = 17;
	string text = Json::writeString(b, r) + "\n";
	FILE * f = fopen(path.c_str(), "wb");
	if (!f || fwrite(text.data(), 1, text.size(), f) != text.size())
	{
		if (f) fclose(f);
		call.Error("write_failed", "Could not write " + path);
		return;
	}
	fclose(f);
	Json::Value ret;
	ret["path"] = path;
	ret["bytes"] = (Json::UInt64) text.size();
	ret["object_count"] = (int) names.size();
	call.Reply(ret);
}

static void	inject_fixture(WED_MCPCall& call, const Json::Value& args)
{
	WED_Document * doc; WED_DocumentWindow * win; string package;
	if (!WED_MCP_FindDocument(call, args, doc, win, package)) return;

	int parent_id = 0, position = -1;
	if (!WED_MCP_GetInt(call, args, "parent_id", parent_id, false)) return;
	if (!WED_MCP_GetInt(call, args, "position", position, false)) return;
	bool select = args.get("select", true).asBool();

	WED_Thing * parent = nullptr;
	if (parent_id)
		parent = fetch_thing(call, doc, parent_id);
	else
	{
		parent = WED_GetCurrentAirport(doc);
		if (!parent)
			parent = WED_GetWorld(doc);
	}
	if (!parent) return;
	if (position < 0 || position > parent->CountChildren())
		position = parent->CountChildren();

	map<string,int> refs;
	vector<int> ids;
	WED_MCPError err;

	WED_Archive * archive = doc->GetArchive();
	archive->StartCommand("MCP: Inject Fixture");
	bool ok = WED_MCP_InjectObjects(parent, position, args["objects"], refs, ids, err);
	if (ok && select)
	{
		ISelection * sel = WED_GetSelect(doc);
		sel->Clear();
		for(int n = 0; n < ids.size(); ++n)
			sel->Insert(dynamic_cast<ISelectable *>(archive->Fetch(ids[n])));
	}
	if (!ok)
	{
		archive->AbortCommand();
		call.Error(err.code, err.message + " - nothing was created.", err.extra);
		return;
	}
	archive->CommitCommand();

	Json::Value r = WED_MCP_DocSummary(doc, win, package);
	r["parent_id"] = parent->GetID();
	r["ids"] = Json::Value(Json::arrayValue);
	for(int n = 0; n < ids.size(); ++n)
		r["ids"].append(ids[n]);
	r["refs"] = Json::Value(Json::objectValue);
	for(map<string,int>::iterator i = refs.begin(); i != refs.end(); ++i)
		r["refs"][i->first] = i->second;
	call.Reply(r);
}

static void	set_properties(WED_MCPCall& call, const Json::Value& args)
{
	WED_Document * doc; WED_DocumentWindow * win; string package;
	if (!WED_MCP_FindDocument(call, args, doc, win, package)) return;
	int id;
	if (!WED_MCP_GetInt(call, args, "id", id)) return;
	WED_Thing * t = fetch_thing(call, doc, id);
	if (!t) return;

	WED_MCPError err;
	WED_Archive * archive = doc->GetArchive();
	archive->StartCommand("MCP: Set Properties");
	if (!WED_MCP_SetProperties(t, args["props"], err))
	{
		archive->AbortCommand();
		call.Error(err.code, err.message + " - nothing was changed.", err.extra);
		return;
	}
	archive->CommitCommand();

	Json::Value r = WED_MCP_DocSummary(doc, win, package);
	map<int, Json::Value> names;
	WED_MCPDumpOptions opts;
	opts.max_depth = 0;
	r["object"] = WED_MCP_DumpTree(t, opts, names);
	call.Reply(r);
}

static void	set_selection(WED_MCPCall& call, const Json::Value& args)
{
	WED_Document * doc; WED_DocumentWindow * win; string package;
	if (!WED_MCP_FindDocument(call, args, doc, win, package)) return;
	string mode = args.get("mode", "replace").asString();

	const Json::Value& ids = args["ids"];
	vector<ISelectable *> who;
	for(Json::ArrayIndex n = 0; n < ids.size(); ++n)
	{
		if (!ids[n].isIntegral())
		{
			call.Error("invalid_argument", "ids must be integers");
			return;
		}
		WED_Thing * t = fetch_thing(call, doc, ids[n].asInt());
		if (!t) return;
		who.push_back(t);
	}

	ISelection * sel = WED_GetSelect(doc);
	WED_Archive * archive = doc->GetArchive();
	archive->StartCommand("MCP: Set Selection");
	if (mode == "replace")
		sel->Clear();
	for(int n = 0; n < who.size(); ++n)
	{
		if (mode == "remove")
			sel->Erase(who[n]);
		else
			sel->Insert(who[n]);
	}
	archive->CommitCommand();

	Json::Value r = WED_MCP_DocSummary(doc, win, package);
	vector<ISelectable *> sv;
	sel->GetSelectionVector(sv);
	r["selection"] = Json::Value(Json::arrayValue);
	for(int n = 0; n < sv.size(); ++n)
		r["selection"].append(sv[n]->GetSelectionID());
	call.Reply(r);
}

static const struct { res_type type; const char * name; } kResTypes[] = {
	{ res_Object, "object" }, { res_Facade, "facade" }, { res_Forest, "forest" }, { res_String, "string" },
	{ res_Line, "line" }, { res_Autogen, "autogen" }, { res_Polygon, "polygon" },
#if ROAD_EDITING
	{ res_Road, "road" },
#endif
	{ res_None, nullptr }
};

static void	search_library(WED_MCPCall& call, const Json::Value& args)
{
	WED_Document * doc; WED_DocumentWindow * win; string package;
	if (!WED_MCP_FindDocument(call, args, doc, win, package)) return;
	string pattern, type;
	int limit = 100;
	if (!WED_MCP_GetString(call, args, "pattern", pattern, false)) return;
	if (!WED_MCP_GetString(call, args, "type", type, false)) return;
	if (!WED_MCP_GetInt(call, args, "limit", limit, false)) return;
	limit = max(1, min(limit, 1000));

	WED_LibraryMgr * lib = doc->GetLibrary();
	Json::Value found(Json::arrayValue);
	int total = 0;
	vector<string> todo(1, string());		// "" is the root
	while (!todo.empty())
	{
		string dir = todo.back();
		todo.pop_back();
		vector<string> kids;
		lib->GetResourceChildren(dir, pack_All, kids);
		for(int n = 0; n < kids.size(); ++n)
		{
			res_type rt = lib->GetResourceType(kids[n]);
			if (rt == res_Directory)
			{
				todo.push_back(kids[n]);
				continue;
			}
			const char * tname = "other";
			for(int t = 0; kResTypes[t].name; ++t)
				if (kResTypes[t].type == rt)
					tname = kResTypes[t].name;
			if (!type.empty() && type != tname) continue;
			if (!WED_MCP_ContainsNoCase(kids[n], pattern)) continue;
			if (++total > limit) continue;
			Json::Value r;
			r["vpath"] = kids[n];
			r["type"] = tname;
			r["local"] = lib->IsResourceLocal(kids[n]);
			found.append(r);
		}
	}
	Json::Value ret;
	ret["resources"] = found;
	ret["total_matches"] = total;
	call.Reply(ret);
}

void	WED_MCP_RegisterDocTools(vector<WED_MCPTool>& tools)
{
	tools.push_back({ "dump_document",
		"The document (default: the whole 'world' tree) as JSON, for diffing state. Each object: id, class, props keyed by their "
		"persisted earth.wed.xml names (e.g. \"hierarchy.name\"; enums as strings; lengths in meters), extra (airport meta_data, "
		"chain closed), sources, children. With ids:false, IDs are replaced by refs (o1, o2... in tree order) so dumps from "
		"different sessions compare equal and the output can be fed to inject_fixture. A real airport dumps to MBs, far over "
		"MCP client output limits: pass 'path' to write the dump to a file (then query it with jq/grep), or use root_id / max_depth.",
		R"({"type":"object","properties":{"doc":{"type":"string","description":"Package name; default: the active or only document."},)"
		R"("root_id":{"type":"integer","description":"Dump only this object's subtree."},)"
		R"("max_depth":{"type":"integer","description":"Levels of children to include; deeper objects show child_count."},)"
		R"("ids":{"type":"boolean","description":"Include object IDs (default true)."},)"
		R"("geo":{"type":"boolean","description":"Add derived GIS class and lon/lat bounds per object."},)"
		R"("file_precision":{"type":"boolean","description":"Round numbers to the precision earth.wed.xml stores, so dumps compare equal across save and reopen."},)"
		R"("path":{"type":"string","description":"Write the dump to this file and return only its size."}},"additionalProperties":false})",
		true, false, dump_document });

	tools.push_back({ "inject_fixture",
		"Create objects from JSON in the dump_document format (without ids) as ONE undoable command, to set up a test. "
		"Give an object a 'ref' and use that string in another's 'sources' to wire them (taxi route edges to nodes); sources "
		"may also be existing object IDs. On any error nothing is created. Default parent: the current airport, else the world. "
		"Returns the new top-level IDs and every ref's ID; by default the new objects become the selection (in the same command).",
		R"({"type":"object","properties":{"doc":{"type":"string","description":"Package name; default: the active or only document."},)"
		R"("objects":{"type":"array","items":{"type":"object"},"description":"Objects in dump_document format."},)"
		R"("parent_id":{"type":"integer"},"position":{"type":"integer","description":"Child index to insert at; default: last."},)"
		R"("select":{"type":"boolean","description":"Select the new objects (default true)."}},"required":["objects"],"additionalProperties":false})",
		false, false, inject_fixture });

	tools.push_back({ "set_properties",
		"Set properties of one object, by their dump_document keys, as ONE undoable command. Returns the object afterwards. "
		"Unknown keys or bad enum strings fail with the valid choices listed, and change nothing.",
		R"({"type":"object","properties":{"doc":{"type":"string","description":"Package name; default: the active or only document."},)"
		R"("id":{"type":"integer"},"props":{"type":"object","description":"{key: value} as in dump_document."}},"required":["id","props"],"additionalProperties":false})",
		false, false, set_properties });

	tools.push_back({ "set_selection",
		"Change the selection as ONE undoable command (selection is part of the document).",
		R"({"type":"object","properties":{"doc":{"type":"string","description":"Package name; default: the active or only document."},)"
		R"("ids":{"type":"array","items":{"type":"integer"}},)"
		R"("mode":{"type":"string","enum":["replace","add","remove"],"description":"Default replace; replace with no ids clears."}},"required":["ids"],"additionalProperties":false})",
		false, false, set_selection });

	tools.push_back({ "search_library",
		"Search the library resources (objects, facades, forests, lines, polygons...) visible to a document, for use as "
		"resource paths in fixtures. Public items only, like WED's library pane.",
		R"({"type":"object","properties":{"doc":{"type":"string","description":"Package name; default: the active or only document."},)"
		R"("pattern":{"type":"string","description":"Case-insensitive substring of the virtual path."},)"
		R"("type":{"type":"string","enum":["object","facade","forest","string","line","autogen","polygon","road"]},)"
		R"("limit":{"type":"integer","description":"Max results, default 100, max 1000."}},"additionalProperties":false})",
		true, false, search_library });
}
