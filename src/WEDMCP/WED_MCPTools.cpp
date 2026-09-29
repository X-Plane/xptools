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

#include "WED_MCPTools.h"
#include "WED_StartWindow.h"
#include "WED_DocumentWindow.h"

bool	WED_MCP_FindDocument(WED_MCPCall& call, const Json::Value& args, WED_Document *& out_doc, WED_DocumentWindow *& out_window, string& out_package)
{
	int count = WED_StartWindow::CountOpenDocuments();
	Json::Value open(Json::arrayValue);
	for(int n = 0; n < count; ++n)
	{
		WED_StartWindow::GetNthOpenDocument(n, out_doc, out_window, out_package);
		open.append(out_package);
	}
	Json::Value extra;
	extra["open_documents"] = open;

	if (args.isMember("doc"))
	{
		string want = args["doc"].asString();
		for(int n = 0; n < count; ++n)
		{
			WED_StartWindow::GetNthOpenDocument(n, out_doc, out_window, out_package);
			if (out_package == want)
				return true;
		}
		call.Error("unknown_document", "No open document for package '" + want + "'.", extra);
		return false;
	}

	if (count == 0)
	{
		call.Error("no_document", "No document is open. Use open_package first.", extra);
		return false;
	}
	for(int n = 0; n < count; ++n)
	{
		WED_StartWindow::GetNthOpenDocument(n, out_doc, out_window, out_package);
		if (out_window->IsActiveNow())
			return true;
	}
	if (count == 1)
	{
		WED_StartWindow::GetNthOpenDocument(0, out_doc, out_window, out_package);
		return true;
	}
	call.Error("ambiguous_document", "Several documents are open and none is active. Pass 'doc'.", extra);
	return false;
}

bool	WED_MCP_ContainsNoCase(const string& haystack, const string& needle)
{
	return std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(),
		[](char a, char b) { return tolower((unsigned char) a) == tolower((unsigned char) b); }) != haystack.end();
}

bool	WED_MCP_GetString(WED_MCPCall& call, const Json::Value& args, const char * key, string& out, bool required)
{
	if (!args.isMember(key))
	{
		if (required)
			call.Error("invalid_argument", string("missing required argument '") + key + "'");
		return !required;
	}
	if (!args[key].isString())
	{
		call.Error("invalid_argument", string("argument '") + key + "' must be a string");
		return false;
	}
	out = args[key].asString();
	return true;
}

bool	WED_MCP_GetInt(WED_MCPCall& call, const Json::Value& args, const char * key, int& out, bool required)
{
	if (!args.isMember(key))
	{
		if (required)
			call.Error("invalid_argument", string("missing required argument '") + key + "'");
		return !required;
	}
	if (!args[key].isIntegral() || args[key].isBool())
	{
		call.Error("invalid_argument", string("argument '") + key + "' must be an integer");
		return false;
	}
	out = args[key].asInt();
	return true;
}
