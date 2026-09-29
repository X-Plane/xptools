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

#ifndef WED_MCPTools_H
#define WED_MCPTools_H

#include <json/json.h>
#include <memory>

class	WED_Document;
class	WED_DocumentWindow;
class	WED_StartWindow;
struct	WED_MCPJob;

/*
	WED_MCPTools - the tool table and the helpers tools share

	Every tool handler runs on the main thread, at a safe point: no command is open in any document and the mouse
	is not down.  A handler gets a WED_MCPCall and must call Reply or Error on it exactly once - either before it
	returns, or later from the main thread (async tools like screenshots).

	Undo rule: any tool that changes a document does so inside exactly one undoable command it opens and commits (or
	aborts on error) itself - including selection changes.  Tools that hand off to existing WED handlers (menu
	commands, map tools) let that code own the command.
*/

class	WED_MCPCall {
public:
	explicit			WED_MCPCall(std::shared_ptr<WED_MCPJob> job);

	// Success: data is serialized as the text content of the result.
	void				Reply(const Json::Value& data);
	// Success with explicit MCP content items (e.g. images).
	void				ReplyContent(const Json::Value& content);
	// Failure: {error: code, message: msg, ...extra}.  The code is stable; agents branch on it.
	void				Error(const string& code, const string& msg, const Json::Value& extra = Json::Value());

	bool				IsDone(void) const;

private:
	std::shared_ptr<WED_MCPJob>	mJob;
};

typedef void (* WED_MCPHandler)(WED_MCPCall& call, const Json::Value& args);

struct	WED_MCPTool {
	const char *	name;
	const char *	description;
	const char *	input_schema;	// JSON Schema, as JSON text
	bool			read_only;		// read-only tools must not touch any document - DEV builds check this
	bool			destructive;	// may discard or overwrite user data (close without saving, overwrite files)
	WED_MCPHandler	handler;
};

// Tool groups - each appends its tools.
void	WED_MCP_RegisterAppTools(vector<WED_MCPTool>& tools);
void	WED_MCP_RegisterDocTools(vector<WED_MCPTool>& tools);

// Run step on the main thread on each server tick (~10 ms) until it returns true.  For async tools: the job stays
// running, and so holds the queue, until step calls Reply/Error on its WED_MCPCall.
void	WED_MCP_RunLater(std::function<bool()> step);

// The start window owns the list of open documents and the package list.
WED_StartWindow *	WED_MCP_GetStartWindow(void);

// Resolve the "doc" argument (a package name).  If it is absent, use the active document window, or the only open
// document.  On failure, sends the error on call and returns false.
bool	WED_MCP_FindDocument(WED_MCPCall& call, const Json::Value& args, WED_Document *& out_doc, WED_DocumentWindow *& out_window, string& out_package);

// Case-insensitive substring test; an empty needle matches everything.
bool	WED_MCP_ContainsNoCase(const string& haystack, const string& needle);

// Argument helpers.  They send an invalid_argument error on call and return false if the value is missing or of the wrong type.
bool	WED_MCP_GetString(WED_MCPCall& call, const Json::Value& args, const char * key, string& out, bool required = true);
bool	WED_MCP_GetInt(WED_MCPCall& call, const Json::Value& args, const char * key, int& out, bool required = true);

#endif /* WED_MCPTools_H */
