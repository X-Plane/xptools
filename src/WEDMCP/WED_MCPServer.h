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

#ifndef WED_MCPServer_H
#define WED_MCPServer_H

class	WED_StartWindow;

/*
	WED_MCPServer - Model Context Protocol server for automated testing and development

	Started only when WED is launched with --mcp or --mcp_port=N.  Speaks MCP's Streamable HTTP transport (JSON-RPC 2.0,
	one JSON response per POST, no SSE, no sessions) on 127.0.0.1 only, and rejects requests whose Origin header is
	not localhost, so a web page can't drive WED through the user's browser.

	THREADING: the HTTP server parses requests on its own worker threads.  Every tools/call is queued as a job and run
	on the main thread by a GUI_Timer, one job at a time, and only at a safe point - see WED_MCPServer.cpp.  The HTTP
	thread blocks until the job finishes.

	While the server runs, OS modal dialogs are answered automatically - see WED_MCPHeadless.h.
*/

#define WED_MCP_DEFAULT_PORT 8087

// Returns false (after logging to stderr and WED_Log.txt) if the port could not be bound.
bool	WED_MCP_Start(int port, WED_StartWindow * start_window);

// Fails any pending requests and shuts the server down.  Safe to call if the server never started.
void	WED_MCP_Stop(void);

#endif /* WED_MCPServer_H */
