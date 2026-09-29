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

// Validation and export MCP tools.  All of them run WED's own validation with the results window suppressed and
// return the messages instead.  An explicit export target is applied only for the call and then restored (the
// target belongs to the document - see wed-design-principles.md rule 9).

#include "WED_MCPTools.h"
#include "WED_Document.h"
#include "WED_DocumentWindow.h"
#include "WED_Validate.h"
#include "WED_AptIE.h"
#include "WED_SceneryPackExport.h"
#include "WED_Airport.h"
#include "WED_Globals.h"
#include "WED_ToolUtils.h"
#include "WED_PackageMgr.h"
#include "WED_LibraryMgr.h"
#include "FileUtils.h"

#include <sys/stat.h>

#define MAX_MESSAGES	500

const char *	WED_MCP_ExportTargetName(int target);
const char *	WED_MCP_ValidateErrorName(int code);
Json::Value		WED_MCP_DocSummary(WED_Document * doc, WED_DocumentWindow * win, const string& package);

static const int kTargets[] = { wet_xplane_900, wet_xplane_1000, wet_xplane_1021, wet_xplane_1050, wet_xplane_1100,
								wet_xplane_1130, wet_xplane_1200, wet_xplane_1212, wet_gateway };

class	target_override {
public:
	target_override() : mSaved(gExportTarget) { }
	~target_override() { gExportTarget = mSaved; }
private:
	WED_Export_Target mSaved;
};

// Applies the "target" argument, if any.  The caller holds a target_override to restore it.
static bool	apply_target(WED_MCPCall& call, const Json::Value& args)
{
	if (!args.isMember("target"))
		return true;
	string want = args["target"].asString();
	for(int t : kTargets)
		if (want == WED_MCP_ExportTargetName(t))
		{
			gExportTarget = (WED_Export_Target) t;
			return true;
		}
	Json::Value extra;
	for(int t : kTargets)
		extra["valid"].append(WED_MCP_ExportTargetName(t));
	call.Error("invalid_argument", "Unknown export target '" + want + "'.", extra);
	return false;
}

static Json::Value	messages_json(const validation_error_vector& msgs, int& errors, int& warnings)
{
	errors = warnings = 0;
	Json::Value r(Json::arrayValue);
	for(const validation_error_t& v : msgs)
	{
		bool warn = v.err_code > warnings_start_here;
		(warn ? warnings : errors)++;
		if (r.size() >= MAX_MESSAGES)
			continue;
		Json::Value m;
		m["code"] = WED_MCP_ValidateErrorName(v.err_code);
		m["severity"] = warn ? "warning" : "error";
		m["message"] = v.msg;
		if (v.airport)
		{
			string icao;
			v.airport->GetICAO(icao);
			m["airport_id"] = v.airport->GetID();
			m["airport_icao"] = icao;
		}
		m["object_ids"] = Json::Value(Json::arrayValue);
		for(WED_Thing * t : v.bad_objects)
			if (t) m["object_ids"].append(t->GetID());
		r.append(m);
	}
	return r;
}

static Json::Value	validation_json(validation_result_t res, const validation_error_vector& msgs)
{
	int errors, warnings;
	Json::Value r;
	r["result"] = res == validation_clean ? "clean" : res == validation_warnings_only ? "warnings_only" : "errors";
	r["export_target"] = WED_MCP_ExportTargetName(gExportTarget);
	r["messages"] = messages_json(msgs, errors, warnings);
	r["error_count"] = errors;
	r["warning_count"] = warnings;
	if (r["messages"].size() < msgs.size())
		r["truncated"] = true;
	return r;
}

static WED_Thing *	find_root(WED_MCPCall& call, WED_Document * doc, const Json::Value& args)
{
	if (!args.isMember("root_id"))
		return WED_GetWorld(doc);
	WED_Thing * t = dynamic_cast<WED_Thing *>(doc->GetArchive()->Fetch(args["root_id"].asInt()));
	if (!t)
		call.Error("unknown_object", "No object with ID " + args["root_id"].toStyledString());
	return t;
}

static void	validate(WED_MCPCall& call, const Json::Value& args)
{
	WED_Document * doc; WED_DocumentWindow * win; string package;
	if (!WED_MCP_FindDocument(call, args, doc, win, package)) return;
	target_override keep_target;
	if (!apply_target(call, args)) return;
	WED_Thing * root = find_root(call, doc, args);
	if (!root) return;

	validation_error_vector msgs;
	validation_result_t res = WED_ValidateApt(doc, NULL, root, true, "Dismiss", &msgs);
	call.Reply(validation_json(res, msgs));
}

static void	export_apt(WED_MCPCall& call, const Json::Value& args)
{
	WED_Document * doc; WED_DocumentWindow * win; string package;
	if (!WED_MCP_FindDocument(call, args, doc, win, package)) return;
	string path;
	if (!WED_MCP_GetString(call, args, "path", path)) return;
	target_override keep_target;
	if (!apply_target(call, args)) return;
	WED_Thing * root = find_root(call, doc, args);
	if (!root) return;

	// Same order as WED_DoExportApt: validate, and only errors block.
	validation_error_vector msgs;
	validation_result_t res = WED_ValidateApt(doc, NULL, root, true, "Dismiss", &msgs);
	Json::Value r = validation_json(res, msgs);
	if (res == validation_errors)
	{
		call.Error("validation_failed", "Validation errors - nothing was exported.", r);
		return;
	}
	WED_AptExport(root, path.c_str());

	struct stat st;
	if (FILE_get_file_meta_data(path, st) != 0)
	{
		call.Error("export_failed", "No file was written to " + path + " (see alerts).", r);
		return;
	}
	r["path"] = path;
	r["bytes"] = (Json::UInt64) st.st_size;
	call.Reply(r);
}

typedef map<string, pair<long long, long long> >	dir_snapshot;		// relative path -> (mtime, size)

static void	snapshot(const string& dir, dir_snapshot& out)
{
	out.clear();
	vector<string> files, dirs;
	FILE_get_directory_recursive(dir, files, dirs);
	for(const string& f : files)
	{
		struct stat st;
		if (FILE_get_file_meta_data(f, st) == 0)
			out[f.substr(dir.size() + 1)] = make_pair((long long) st.st_mtime, (long long) st.st_size);
	}
}

static void	export_pack(WED_MCPCall& call, const Json::Value& args)
{
	WED_Document * doc; WED_DocumentWindow * win; string package;
	if (!WED_MCP_FindDocument(call, args, doc, win, package)) return;
	target_override keep_target;
	if (!apply_target(call, args)) return;

	string dir = gPackageMgr->ComputePath(package, "");
	while (!dir.empty() && (dir.back() == '/' || dir.back() == '\\'))
		dir.pop_back();
	dir_snapshot before, after;
	snapshot(dir, before);
	time_t start = time(NULL);

	// The menu command's own flow (validation, Gateway heuristics and their roll-back, stale DSF deletion,
	// selecting objects that failed) - just without the results window.
	validation_error_vector msgs;
	bool exported = WED_DoExportPack(doc, win->GetMapPane(), true, &msgs);
	Json::Value r = validation_json(exported ? (msgs.empty() ? validation_clean : validation_warnings_only) : validation_errors, msgs);
	if (!exported)
	{
		call.Error("validation_failed", "Validation errors - nothing was exported.", r);
		return;
	}

	snapshot(dir, after);
	r["written"] = Json::Value(Json::arrayValue);
	r["deleted"] = Json::Value(Json::arrayValue);
	for(dir_snapshot::iterator f = after.begin(); f != after.end(); ++f)
	{
		dir_snapshot::iterator b = before.find(f->first);
		if (b == before.end() || b->second != f->second || f->second.first >= start)
			r["written"].append(f->first);
	}
	for(dir_snapshot::iterator b = before.begin(); b != before.end(); ++b)
		if (!after.count(b->first))
			r["deleted"].append(b->first);
	r["package_path"] = dir;
	r["document"] = WED_MCP_DocSummary(doc, win, package);
	call.Reply(r);
}

void	WED_MCP_RegisterExportTools(vector<WED_MCPTool>& tools)
{
	tools.push_back({ "validate",
		"Run WED's validation (as the Validate menu command, without the results window) and return every message: stable "
		"code name, severity (errors block export, warnings don't), text, airport and the offending object IDs. Rules depend on "
		"the export target; 'target' applies one for this call only. The gateway target also checks runways against CIFP data, "
		"which may download it. Also writes validation_report.txt in the package, as WED always does.",
		R"({"type":"object","properties":{"doc":{"type":"string","description":"Package name; default: the active or only document."},)"
		R"("root_id":{"type":"integer","description":"Validate only this subtree (e.g. one airport)."},)"
		R"("target":{"type":"string","enum":["xp900","xp1000","xp1021","xp1050","xp1100","xp1130","xp1200","xp1212","gateway"]}},"additionalProperties":false})",
		true, false, validate });

	tools.push_back({ "export_apt",
		"Validate, then write the airports to an apt.dat at 'path' (like File > Export apt.dat, without dialogs). Fails with "
		"validation_failed and the messages if there are errors; warnings are returned alongside the result.",
		R"({"type":"object","properties":{"doc":{"type":"string","description":"Package name; default: the active or only document."},)"
		R"("path":{"type":"string","description":"Output file; overwritten."},"root_id":{"type":"integer"},)"
		R"("target":{"type":"string","enum":["xp900","xp1000","xp1021","xp1050","xp1100","xp1130","xp1200","xp1212","gateway"]}},"required":["path"],"additionalProperties":false})",
		true, true, export_apt });

	tools.push_back({ "export_pack",
		"Export Scenery Pack into the document's package, exactly as the menu command (validation, DSF tiles, apt.dat, Gateway "
		"upgrade heuristics rolled back afterwards, stale DSFs removed) but returning the validation messages instead of showing "
		"them. Reports the package files written and deleted. Objects that failed to export become the selection.",
		R"({"type":"object","properties":{"doc":{"type":"string","description":"Package name; default: the active or only document."},)"
		R"("target":{"type":"string","enum":["xp900","xp1000","xp1021","xp1050","xp1100","xp1130","xp1200","xp1212","gateway"]}},"additionalProperties":false})",
		false, true, export_pack });
}
