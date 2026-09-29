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

#ifndef WED_MCPHeadless_H
#define WED_MCPHeadless_H

#include <json/json.h>

/*
	WED_MCPHeadless - modal dialogs under automation

	While the MCP server runs, every OS modal (DoUserAlert, ConfirmMessage, DoSaveDiscardDialog, file pickers) is
	answered without showing anything: an agent driving WED can't click a dialog, and a modal loop would stall the
	request that caused it.  Each dialog is recorded in an alert log with the answer it got.

	Answers come from a FIFO of presets set by the agent, or a conservative default that never destroys work:
	alerts are OK'd, confirms and save/discard are cancelled, file pickers are cancelled.
*/

void	WED_MCPHeadless_Install(void);
void	WED_MCPHeadless_Remove(void);

// Sequence number that the next recorded dialog will get.
int		WED_MCPHeadless_NextSeq(void);

// All recorded dialogs with seq >= since, oldest first.
Json::Value	WED_MCPHeadless_GetAlerts(int since);

// Presets: [{kind?: "alert"|"confirm"|"save_discard"|"file"|"any", answer?: string, path?: string, paths?: [string]}]
// Returns an error message, or empty on success.
// at_front: these answers are used before any already queued (for tools that pre-answer the dialog they are about to cause).
string	WED_MCPHeadless_AddAnswers(const Json::Value& answers, bool replace, bool at_front = false);
Json::Value	WED_MCPHeadless_GetPendingAnswers(void);

#endif /* WED_MCPHeadless_H */
