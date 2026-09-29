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

// App-level MCP tools: state, packages and documents, menu commands, dialogs, logs.

#include "WED_MCPTools.h"
#include "WED_MCPHeadless.h"
#include "WED_MCPCommandNames.h"
#include "WED_Document.h"
#include "WED_DocumentWindow.h"
#include "WED_StartWindow.h"
#include "WED_PackageMgr.h"
#include "WED_Globals.h"
#include "WED_Select.h"
#include "WED_ToolUtils.h"
#include "WED_Version.h"
#include "GUI_Application.h"
#include "FileUtils.h"
#include "PlatformUtils.h"

#include <fstream>

#define MAX_LIST_NAMES	25		// undo/redo names reported per document

const char *	WED_MCP_ExportTargetName(int target)
{
	switch(target) {
	case wet_xplane_900:	return "xp900";
	case wet_xplane_1000:	return "xp1000";
	case wet_xplane_1021:	return "xp1021";
	case wet_xplane_1050:	return "xp1050";
	case wet_xplane_1100:	return "xp1100";
	case wet_xplane_1130:	return "xp1130";
	case wet_xplane_1200:	return "xp1200";
	case wet_xplane_1212:	return "xp1212";
	case wet_gateway:		return "gateway";
	default:				return "unknown";
	}
}

// The short summary of a document every mutating tool returns, so an agent can see what it did without a dump.
Json::Value	WED_MCP_DocSummary(WED_Document * doc, WED_DocumentWindow * win, const string& package)
{
	Json::Value d;
	d["package"] = package;
	d["active"] = win->IsActiveNow();
	d["dirty"] = doc->IsDirty();
	WED_UndoMgr * um = doc->GetUndoMgr();
	vector<string> names;
	um->GetUndoNames(names);
	d["undo"] = Json::Value(Json::arrayValue);
	for(int n = 0; n < names.size() && n < MAX_LIST_NAMES; ++n)
		d["undo"].append(names[n]);
	um->GetRedoNames(names);
	d["redo"] = Json::Value(Json::arrayValue);
	for(int n = 0; n < names.size() && n < MAX_LIST_NAMES; ++n)
		d["redo"].append(names[n]);
	ISelection * sel = WED_GetSelect(doc);
	d["selection_count"] = sel ? sel->GetSelectionCount() : 0;
	return d;
}

#pragma mark -

static void	get_state(WED_MCPCall& call, const Json::Value& args)
{
	Json::Value r;
	r["wed_version"] = WED_VERSION_STRING;
#if APL
	r["platform"] = "mac";
#elif IBM
	r["platform"] = "windows";
#else
	r["platform"] = "linux";
#endif
	r["dev_build"] = DEV ? true : false;

	string xsys;
	r["xsystem_folder"] = gPackageMgr->GetXPlaneFolder(xsys) ? xsys : "";
	r["export_target"] = WED_MCP_ExportTargetName(gExportTarget);
	r["mouse_down"] = gApplication->IsDeferring();

	r["documents"] = Json::Value(Json::arrayValue);
	for(int n = 0; n < WED_StartWindow::CountOpenDocuments(); ++n)
	{
		WED_Document * d; WED_DocumentWindow * w; string p;
		WED_StartWindow::GetNthOpenDocument(n, d, w, p);
		r["documents"].append(WED_MCP_DocSummary(d, w, p));
	}

	r["next_alert_seq"] = WED_MCPHeadless_NextSeq();
	r["pending_dialog_answers"] = WED_MCPHeadless_GetPendingAnswers();
	call.Reply(r);
}

static void	list_packages(WED_MCPCall& call, const Json::Value& args)
{
	string filter;
	if (!WED_MCP_GetString(call, args, "filter", filter, false)) return;

	if (!gPackageMgr->HasSystemFolder())
	{
		call.Error("no_xsystem_folder", "No X-Plane folder is set. Launch WED with --xsystem=<path>.");
		return;
	}

	set<string> open;
	for(int n = 0; n < WED_StartWindow::CountOpenDocuments(); ++n)
	{
		WED_Document * d; WED_DocumentWindow * w; string p;
		WED_StartWindow::GetNthOpenDocument(n, d, w, p);
		open.insert(p);
	}

	Json::Value r(Json::arrayValue);
	for(int n = 0; n < gPackageMgr->CountCustomPackages(); ++n)
	{
		string name;
		gPackageMgr->GetNthPackageName(n, name);
		if (!WED_MCP_ContainsNoCase(name, filter))
			continue;
		Json::Value p;
		p["name"] = name;
		p["has_wed_xml"] = gPackageMgr->HasXML(n);
		p["has_apt_dat"] = gPackageMgr->HasAPT(n);
		p["open"] = open.count(name) > 0;
		r.append(p);
	}
	Json::Value ret;
	ret["packages"] = r;
	call.Reply(ret);
}

static void	open_package(WED_MCPCall& call, const Json::Value& args)
{
	string name;
	if (!WED_MCP_GetString(call, args, "name", name)) return;

	if (!gPackageMgr->HasSystemFolder())
	{
		call.Error("no_xsystem_folder", "No X-Plane folder is set. Launch WED with --xsystem=<path>.");
		return;
	}

	WED_Document * d; WED_DocumentWindow * w; string p;
	for(int n = 0; n < WED_StartWindow::CountOpenDocuments(); ++n)
	{
		WED_StartWindow::GetNthOpenDocument(n, d, w, p);
		if (p == name)
		{
			w->Show();
			Json::Value r = WED_MCP_DocSummary(d, w, p);
			r["already_open"] = true;
			call.Reply(r);
			return;
		}
	}

	gPackageMgr->Rescan();
	if (!WED_MCP_GetStartWindow()->OpenPackage(name))
	{
		call.Error("open_failed", "Could not open package '" + name + "'. It may not exist in Custom Scenery (see list_packages), or failed to load (see alerts).");
		return;
	}

	for(int n = 0; n < WED_StartWindow::CountOpenDocuments(); ++n)
	{
		WED_StartWindow::GetNthOpenDocument(n, d, w, p);
		if (p == name)
		{
			call.Reply(WED_MCP_DocSummary(d, w, p));
			return;
		}
	}
	call.Error("open_failed", "Package '" + name + "' did not open.");
}

static void	new_package(WED_MCPCall& call, const Json::Value& args)
{
	string name;
	if (!WED_MCP_GetString(call, args, "name", name, false)) return;

	if (!gPackageMgr->HasSystemFolder())
	{
		call.Error("no_xsystem_folder", "No X-Plane folder is set. Launch WED with --xsystem=<path>.");
		return;
	}

	gPackageMgr->Rescan();
	if (!name.empty())
	{
		if (name.find_first_of("/\\:") != string::npos || name == "." || name == "..")
		{
			call.Error("invalid_argument", "Package names cannot contain path separators.");
			return;
		}
		for(int n = 0; n < gPackageMgr->CountCustomPackages(); ++n)
		{
			string existing;
			gPackageMgr->GetNthPackageName(n, existing);
			if (strcasecmp(existing.c_str(), name.c_str()) == 0)
			{
				call.Error("package_exists", "A package named '" + existing + "' already exists.");
				return;
			}
		}
	}

	int idx = gPackageMgr->CreateNewCustomPackage();
	if (idx < 0)
	{
		call.Error("create_failed", "Could not create a package folder in Custom Scenery (see alerts).");
		return;
	}
	if (!name.empty())
		gPackageMgr->RenameCustomPackage(idx, name);
	gPackageMgr->GetNthPackageName(idx, name);

	if (!WED_MCP_GetStartWindow()->OpenPackage(name))
	{
		call.Error("open_failed", "Created package '" + name + "' but could not open it (see alerts).");
		return;
	}
	WED_Document * d; WED_DocumentWindow * w; string p;
	for(int n = 0; n < WED_StartWindow::CountOpenDocuments(); ++n)
	{
		WED_StartWindow::GetNthOpenDocument(n, d, w, p);
		if (p == name)
		{
			call.Reply(WED_MCP_DocSummary(d, w, p));
			return;
		}
	}
	call.Error("open_failed", "Package '" + name + "' did not open.");
}

static void	close_document(WED_MCPCall& call, const Json::Value& args)
{
	WED_Document * doc; WED_DocumentWindow * win; string package;
	if (!WED_MCP_FindDocument(call, args, doc, win, package)) return;

	string if_dirty = args.get("if_dirty", "fail").asString();
	if (doc->IsDirty())
	{
		if (if_dirty == "fail")
		{
			call.Error("dirty", "Document '" + package + "' has unsaved changes. Pass if_dirty: \"save\" or \"discard\".");
			return;
		}
		Json::Value a;
		a[0]["kind"] = "save_discard";
		a[0]["answer"] = if_dirty;
		WED_MCPHeadless_AddAnswers(a, false, true);
	}

	if (!doc->TryClose())
	{
		call.Error("close_cancelled", "The document did not close (see alerts).");
		return;
	}

	// The document is destroyed asynchronously on a later event-loop pass - reply once it's really gone, so the
	// next tool never sees a half-dead document.
	WED_MCPCall c(call);
	WED_MCP_RunLater([c, doc, package]() mutable {
		for(int n = 0; n < WED_StartWindow::CountOpenDocuments(); ++n)
		{
			WED_Document * d; WED_DocumentWindow * w; string p;
			WED_StartWindow::GetNthOpenDocument(n, d, w, p);
			if (d == doc)
				return false;
		}
		Json::Value r;
		r["closed"] = package;
		c.Reply(r);
		return true;
	});
}

#pragma mark -

// Focus the document's window so the command goes where the agent meant, exactly as if the user had clicked that
// window first and then picked the menu item.
static void	focus_for_command(const Json::Value& args, WED_MCPCall& call, bool& ok)
{
	ok = true;
	if (!args.isMember("doc") && WED_StartWindow::CountOpenDocuments() == 0)
		return;		// app-level commands (new/open package...) with no document
	WED_Document * doc; WED_DocumentWindow * win; string package;
	if (!WED_MCP_FindDocument(call, args, doc, win, package)) { ok = false; return; }
	win->FocusChain(1);
}

static void	list_commands(WED_MCPCall& call, const Json::Value& args)
{
	string filter;
	if (!WED_MCP_GetString(call, args, "filter", filter, false)) return;
	bool enabled_only = args.get("enabled_only", false).asBool();
	bool ok;
	focus_for_command(args, call, ok);
	if (!ok) return;

	Json::Value r(Json::arrayValue);
	for(const WED_MCPCommandName * c = WED_MCP_GetCommandNames(); c->name; ++c)
	{
		if (!WED_MCP_ContainsNoCase(c->name, filter))
			continue;
		string label;
		int check = 0;
		int enabled = gApplication->DispatchCanHandleCommand(c->cmd, label, check);
		if (enabled_only && !enabled)
			continue;
		Json::Value cmd;
		cmd["name"] = c->name;
		cmd["enabled"] = enabled != 0;
		if (check)
			cmd["checked"] = true;
		if (!label.empty())
			cmd["label"] = label;
		r.append(cmd);
	}
	Json::Value ret;
	ret["commands"] = r;
	call.Reply(ret);
}

static void	execute_command(WED_MCPCall& call, const Json::Value& args)
{
	string name;
	if (!WED_MCP_GetString(call, args, "name", name)) return;
	int cmd = WED_MCP_FindCommand(name);
	if (cmd == 0)
	{
		call.Error("unknown_command", "No command named '" + name + "'. Use list_commands to see them.");
		return;
	}
	bool ok;
	focus_for_command(args, call, ok);
	if (!ok) return;

	string label;
	int check = 0;
	if (!gApplication->DispatchCanHandleCommand(cmd, label, check))
	{
		call.Error("disabled", "Command " + name + " is disabled right now (e.g. nothing suitable is selected).");
		return;
	}

	int handled = gApplication->DispatchHandleCommand(cmd);

	Json::Value r;
	r["command"] = name;
	r["handled"] = handled != 0;
	// The command may have closed or opened documents, so look them up again.
	r["documents"] = Json::Value(Json::arrayValue);
	for(int n = 0; n < WED_StartWindow::CountOpenDocuments(); ++n)
	{
		WED_Document * d; WED_DocumentWindow * w; string p;
		WED_StartWindow::GetNthOpenDocument(n, d, w, p);
		r["documents"].append(WED_MCP_DocSummary(d, w, p));
	}
	call.Reply(r);
}

#pragma mark -

static void	get_alerts(WED_MCPCall& call, const Json::Value& args)
{
	int since = 0;
	if (!WED_MCP_GetInt(call, args, "since", since, false)) return;
	Json::Value r;
	r["alerts"] = WED_MCPHeadless_GetAlerts(since);
	r["next_alert_seq"] = WED_MCPHeadless_NextSeq();
	call.Reply(r);
}

static void	set_dialog_answers(WED_MCPCall& call, const Json::Value& args)
{
	string err = WED_MCPHeadless_AddAnswers(args["answers"], args.get("replace", false).asBool());
	if (!err.empty())
	{
		call.Error("invalid_argument", err);
		return;
	}
	Json::Value r;
	r["pending_dialog_answers"] = WED_MCPHeadless_GetPendingAnswers();
	call.Reply(r);
}

static void	get_logs(WED_MCPCall& call, const Json::Value& args)
{
	int lines = 100;
	string pattern;
	if (!WED_MCP_GetInt(call, args, "lines", lines, false)) return;
	if (!WED_MCP_GetString(call, args, "pattern", pattern, false)) return;
	lines = max(1, min(lines, 2000));

	LOG_FLUSH();
	string path = FILE_get_dir_name(GetApplicationPath()) + "WED_Log.txt";
	std::ifstream f(path);
	if (!f)
	{
		call.Error("no_log", "Could not read the log file " + path);
		return;
	}
	vector<string> all;
	string l;
	int total = 0;
	while (std::getline(f, l))
	{
		++total;
		if (WED_MCP_ContainsNoCase(l, pattern))
			all.push_back(l);
	}
	string text;
	for(int n = max(0, (int) all.size() - lines); n < all.size(); ++n)
		text += all[n] + "\n";

	Json::Value r;
	r["path"] = path;
	r["log"] = text;
	r["matched_lines"] = (int) all.size();
	r["total_lines"] = total;
	call.Reply(r);
}

#pragma mark -

void	WED_MCP_RegisterAppTools(vector<WED_MCPTool>& tools)
{
	tools.push_back({ "get_state",
		"Overview of WED: version, X-Plane folder, export target, whether the mouse is down, and each open document "
		"(package, active, dirty, undo/redo command names most recent first, selection count). Always safe; start here.",
		R"({"type":"object","properties":{},"additionalProperties":false})",
		true, false, get_state });

	tools.push_back({ "list_packages",
		"List the scenery packages in the X-Plane folder's Custom Scenery, with whether each has an earth.wed.xml or apt.dat and is open.",
		R"({"type":"object","properties":{"filter":{"type":"string","description":"Case-insensitive substring of the package name."}},"additionalProperties":false})",
		true, false, list_packages });

	tools.push_back({ "open_package",
		"Open a Custom Scenery package as a document, as if picked in the start window. Load problems are reported in 'alerts'.",
		R"({"type":"object","properties":{"name":{"type":"string","description":"Package (folder) name in Custom Scenery."}},"required":["name"],"additionalProperties":false})",
		false, false, open_package });

	tools.push_back({ "new_package",
		"Create a new empty package in Custom Scenery and open it. Without a name WED picks 'Untitled N'.",
		R"({"type":"object","properties":{"name":{"type":"string","description":"Folder name for the new package; must not exist yet."}},"additionalProperties":false})",
		false, false, new_package });

	tools.push_back({ "close_document",
		"Close a document window. Fails with 'dirty' if it has unsaved changes, unless if_dirty says what to do.",
		R"({"type":"object","properties":{"doc":{"type":"string","description":"Package name; default: the active or only document."},)"
		R"("if_dirty":{"type":"string","enum":["fail","save","discard"],"description":"What to do with unsaved changes (default fail)."}},"additionalProperties":false})",
		false, true, close_document });

	tools.push_back({ "list_commands",
		"List WED's menu commands by name (e.g. wed_Group, gui_Undo) with whether each is enabled right now for the given document. "
		"'label' is only present for commands whose menu text changes (e.g. Undo <name>).",
		R"({"type":"object","properties":{"doc":{"type":"string","description":"Package name; default: the active or only document."},)"
		R"("filter":{"type":"string","description":"Case-insensitive substring of the command name."},)"
		R"("enabled_only":{"type":"boolean"}},"additionalProperties":false})",
		true, false, list_commands });

	tools.push_back({ "execute_command",
		"Run a menu command exactly as if the user picked it with the document's window in front (gui_Undo, gui_Save, wed_Group, wed_ZoomAll, ...). "
		"The command owns its own undo step. Dialogs it raises are auto-answered (see set_dialog_answers) and reported in 'alerts'. "
		"Returns the state of every open document afterwards.",
		R"({"type":"object","properties":{"name":{"type":"string","description":"Command name from list_commands."},)"
		R"("doc":{"type":"string","description":"Package name; default: the active or only document."}},"required":["name"],"additionalProperties":false})",
		false, false, execute_command });

	tools.push_back({ "get_alerts",
		"The log of OS dialogs WED raised and how each was auto-answered, oldest first.",
		R"({"type":"object","properties":{"since":{"type":"integer","description":"Only alerts with seq >= since."}},"additionalProperties":false})",
		true, false, get_alerts });

	tools.push_back({ "set_dialog_answers",
		"Queue answers for the next dialogs WED raises, used in order, each once. Without a preset, alerts are OK'd and every other "
		"dialog is cancelled. kind: alert | confirm | save_discard | file | any. answer: for confirm 'proceed', 'option' or 'cancel' "
		"(or the button label); for save_discard 'save', 'discard' or 'cancel'. For file pickers give path (or paths).",
		R"({"type":"object","properties":{"answers":{"type":"array","items":{"type":"object","properties":{)"
		R"("kind":{"type":"string","enum":["any","alert","confirm","save_discard","file"]},"answer":{"type":"string"},)"
		R"("path":{"type":"string"},"paths":{"type":"array","items":{"type":"string"}}},"additionalProperties":false}},)"
		R"("replace":{"type":"boolean","description":"Drop previously queued answers first."}},"required":["answers"],"additionalProperties":false})",
		false, false, set_dialog_answers });

	tools.push_back({ "get_logs",
		"Tail of WED_Log.txt (WED's log, next to the application).",
		R"({"type":"object","properties":{"lines":{"type":"integer","description":"How many lines, default 100, max 2000."},)"
		R"("pattern":{"type":"string","description":"Only lines containing this, case-insensitive."}},"additionalProperties":false})",
		true, false, get_logs });
}
