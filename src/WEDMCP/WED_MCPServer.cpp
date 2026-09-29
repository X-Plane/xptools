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

#include <httplib.h>

#include "WED_MCPServer.h"
#include "WED_MCPTools.h"
#include "WED_MCPHeadless.h"
#include "WED_MCPCommandNames.h"
#include "WED_Menus.h"
#include "WED_Document.h"
#include "WED_StartWindow.h"
#include "WED_Version.h"
#include "GUI_Application.h"
#include "GUI_Timer.h"
#include "AssertUtils.h"
#include "PlatformUtils.h"

#include <condition_variable>
#include <deque>
#include <mutex>

/*
	THE MAIN-THREAD HANDOFF AND THE SAFE POINT

	HTTP worker threads never touch WED state.  A tools/call becomes a WED_MCPJob, queued and waited on.  A GUI_Timer on
	the main thread runs queued jobs one at a time, and only when:

	- no job is already running (async tools like screenshots stay "running" across timer ticks, and so hold the queue),
	- we are not inside a job handler (timers fire inside nested event loops, so a handler that pumps events could
	  otherwise re-enter us),
	- the root commander isn't deferring (a mouse button is down - a tool gesture is half done), and
	- no document has an undo command open (we'd be in the middle of someone else's edit).

	A job that can't start within JOB_START_TIMEOUT is cancelled and the agent gets "busy" - it never runs late, after
	the agent has given up on it.  Once started a job always runs to the end.
*/

#define JOB_START_TIMEOUT	10		// seconds a job may wait for a safe point
#define JOB_RUN_TIMEOUT		300		// seconds we wait for a started job before giving the agent a timeout
#define TIMER_INTERVAL		0.01f

static const char * kProtocolVersions[] = { "2025-11-25", "2025-06-18", "2025-03-26", nullptr };

static const char * kInstructions =
	"This server drives WorldEditor (WED), X-Plane's airport and scenery editor, for automated testing. "
	"Start with get_state. Objects are addressed by numeric ID; IDs are only stable within one WED session. "
	"Every tool that changes a document does so as a single undoable command, so gui_Undo (execute_command) reverts it. "
	"OS dialogs are never shown: they are answered automatically and reported in each result's 'alerts' - "
	"use set_dialog_answers beforehand to choose the answers. If a tool returns error 'busy', WED is in the middle of "
	"something (mouse down, command open): wait and retry with the same arguments.";

enum { job_pending, job_running, job_done, job_cancelled };

struct	WED_MCPJob {
	const WED_MCPTool *			tool;
	Json::Value					args;

	std::mutex					lock;
	std::condition_variable		cv;
	int							state = job_pending;
	Json::Value					result;			// MCP CallToolResult

	// Main-thread only
	int							alert_seq = 0;
	vector<long long>			cache_keys;		// per open document, for the read-only check
};

typedef std::shared_ptr<WED_MCPJob>	job_ptr;

class	WED_MCPServer : public GUI_Timer {
public:
			WED_MCPServer(WED_StartWindow * start_window);

	bool	Start(int port);
	void	Stop(void);

	void	FinishJob(WED_MCPJob * job, const Json::Value& result);
	void	RunLater(std::function<bool()> step) { mContinuations.push_back(step); }

	virtual	void	TimerFired(void);

	WED_StartWindow *	mStartWindow;

private:

	void	HandlePost(const httplib::Request& req, httplib::Response& res);
	Json::Value	CallTool(const Json::Value& params, const Json::Value& id);
	bool	IsSafePoint(void) const;
	void	SnapshotCacheKeys(vector<long long>& keys) const;

	httplib::Server				mHTTP;
	std::thread					mListenThread;

	vector<WED_MCPTool>			mTools;			// immutable after Start, so HTTP threads may read it
	vector<Json::Value>			mSchemas;

	std::mutex					mQueueLock;
	std::deque<job_ptr>			mQueue;
	bool						mStopping = false;

	job_ptr						mCurrent;		// main thread only
	vector<std::function<bool()> >	mContinuations;	// main thread only
	bool						mInHandler = false;
};

static WED_MCPServer *	sServer = nullptr;

static string	to_json(const Json::Value& v)
{
	Json::StreamWriterBuilder b;
	b["indentation"] = "";
	b["emitUTF8"] = true;
	b["precision"] = 17;
	return Json::writeString(b, v);
}

static bool	from_json(const string& s, Json::Value& out, string& err)
{
	Json::CharReaderBuilder b;
	b["collectComments"] = false;
	std::unique_ptr<Json::CharReader> r(b.newCharReader());
	return r->parse(s.data(), s.data() + s.size(), &out, &err);
}

static Json::Value	error_result(const string& code, const string& msg, const Json::Value& extra = Json::Value())
{
	Json::Value data(Json::objectValue);
	if (extra.isObject())
		data = extra;
	data["error"] = code;
	data["message"] = msg;
	Json::Value r;
	r["content"][0]["type"] = "text";
	r["content"][0]["text"] = to_json(data);
	r["isError"] = true;
	return r;
}

static Json::Value	rpc_result(const Json::Value& id, const Json::Value& result)
{
	Json::Value r;
	r["jsonrpc"] = "2.0";
	r["id"] = id;
	r["result"] = result;
	return r;
}

static Json::Value	rpc_error(const Json::Value& id, int code, const string& msg)
{
	Json::Value r;
	r["jsonrpc"] = "2.0";
	r["id"] = id;
	r["error"]["code"] = code;
	r["error"]["message"] = msg;
	return r;
}

// Browsers send Origin on cross-site requests; a page on another site that resolves to 127.0.0.1 (DNS rebinding)
// still carries its own origin, so allowing only localhost origins blocks it.  Non-browser clients send no Origin.
static bool	origin_ok(const httplib::Request& req)
{
	if (!req.has_header("Origin"))
		return true;
	string o = req.get_header_value("Origin");
	const char * ok[] = { "http://localhost", "http://127.0.0.1", "http://[::1]", nullptr };
	for(const char ** p = ok; *p; ++p)
	{
		size_t n = strlen(*p);
		if (o.compare(0, n, *p) == 0 && (o.size() == n || o[n] == ':'))
			return true;
	}
	return false;
}

// Minimal JSON Schema check - enough for our flat tool schemas: object args, known properties only, required
// properties present, top-level types and string enums.  Returns an error message, or empty if OK.
static string	check_args(const Json::Value& schema, const Json::Value& args)
{
	if (!args.isObject())
		return "arguments must be an object";
	const Json::Value& props = schema["properties"];
	for(Json::Value::const_iterator a = args.begin(); a != args.end(); ++a)
	{
		string key = a.name();
		if (!props.isMember(key))
		{
			string known;
			for(Json::Value::const_iterator p = props.begin(); p != props.end(); ++p)
				known += (known.empty() ? "" : ", ") + p.name();
			return "unknown argument '" + key + "' (arguments are: " + (known.empty() ? "none" : known) + ")";
		}
		const Json::Value& ps = props[key];
		const Json::Value& v = *a;
		if (ps.isMember("type"))
		{
			Json::Value types = ps["type"];
			if (!types.isArray()) { Json::Value t(Json::arrayValue); t.append(types); types = t; }
			bool match = false;
			for(Json::ArrayIndex t = 0; t < types.size(); ++t)
			{
				string ty = types[t].asString();
				if ((ty == "string"  && v.isString()) ||
					(ty == "integer" && v.isIntegral() && !v.isBool()) ||
					(ty == "number"  && v.isNumeric() && !v.isBool()) ||
					(ty == "boolean" && v.isBool()) ||
					(ty == "array"   && v.isArray()) ||
					(ty == "object"  && v.isObject()) ||
					(ty == "null"    && v.isNull()))
					match = true;
			}
			if (!match)
				return "argument '" + key + "' must be of type " + to_json(ps["type"]);
		}
		if (ps.isMember("enum") && v.isString())
		{
			bool found = false;
			for(Json::ArrayIndex e = 0; e < ps["enum"].size(); ++e)
				if (ps["enum"][e].asString() == v.asString())
					found = true;
			if (!found)
				return "argument '" + key + "' must be one of " + to_json(ps["enum"]);
		}
	}
	const Json::Value& req = schema["required"];
	for(Json::ArrayIndex r = 0; r < req.size(); ++r)
		if (!args.isMember(req[r].asString()))
			return "missing required argument '" + req[r].asString() + "'";
	return string();
}

WED_MCPServer::WED_MCPServer(WED_StartWindow * start_window) : mStartWindow(start_window)
{
	WED_MCP_RegisterAppTools(mTools);
	WED_MCP_RegisterDocTools(mTools);
	WED_MCP_RegisterMapTools(mTools);

	for(int t = 0; t < mTools.size(); ++t)
	{
		Json::Value schema;
		string err;
		if (!from_json(mTools[t].input_schema, schema, err))
			AssertPrintf("MCP tool %s has a bad input schema: %s", mTools[t].name, err.c_str());
		mSchemas.push_back(schema);
	}

#if DEV
	// The name table is hand-synced with WED_Menus.h - catch a missing or out-of-order entry.
	int prev = 0;
	for(const WED_MCPCommandName * c = WED_MCP_GetCommandNames(); c->name; ++c)
	{
		int expect = prev + 1;
		if (expect == wed_AddMetaDataBegin || expect == wed_AddMetaDataEnd)	// markers, deliberately not in the table
			++expect;
		if (prev >= GUI_APP_MENUS && c->cmd != expect)
			AssertPrintf("WED_MCPCommandNames.cpp is out of sync with WED_Menus.h near %s", c->name);
		prev = c->cmd;
	}
	if (prev != wed_ESRIUses)
		AssertPrintf("WED_MCPCommandNames.cpp is out of sync with WED_Menus.h - last command is not wed_ESRIUses");
#endif
}

bool	WED_MCPServer::Start(int port)
{
	mHTTP.set_payload_max_length(256 * 1024 * 1024);

	// httplib defaults to SO_REUSEPORT, which lets a second WED bind the same port and silently share requests with
	// the first.  We want the second one to fail with "port in use" instead.
	mHTTP.set_socket_options([](socket_t sock) {
		int yes = 1;
#if IBM
		setsockopt(sock, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, reinterpret_cast<const char *>(&yes), sizeof(yes));
#else
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));	// only allows rebinding over TIME_WAIT
#endif
	});

	mHTTP.Post("/mcp", [this](const httplib::Request& req, httplib::Response& res) { HandlePost(req, res); });

	// No sessions, so there's nothing to delete.  No SSE stream to GET.
	mHTTP.Delete("/mcp", [](const httplib::Request& req, httplib::Response& res) {
		res.status = origin_ok(req) ? 200 : 403;
	});
	mHTTP.Get("/mcp", [](const httplib::Request& req, httplib::Response& res) {
		res.status = origin_ok(req) ? 405 : 403;
	});

	if (!mHTTP.bind_to_port("127.0.0.1", port))
		return false;

	mListenThread = std::thread([this]() { mHTTP.listen_after_bind(); });
	GUI_Timer::Start(TIMER_INTERVAL);
	LOG_MSG("I/MCP server listening on http://127.0.0.1:%d/mcp\n", port);
	LOG_FLUSH();
	return true;
}

void	WED_MCPServer::Stop(void)
{
	GUI_Timer::Stop();

	// Release every HTTP thread waiting on a job first - httplib's stop() joins its workers.
	{
		std::lock_guard<std::mutex> l(mQueueLock);
		mStopping = true;
		for(std::deque<job_ptr>::iterator j = mQueue.begin(); j != mQueue.end(); ++j)
		{
			std::lock_guard<std::mutex> jl((*j)->lock);
			(*j)->state = job_cancelled;
			(*j)->cv.notify_all();
		}
		mQueue.clear();
	}
	if (mCurrent)
	{
		std::lock_guard<std::mutex> jl(mCurrent->lock);
		mCurrent->result = error_result("shutting_down", "WED is quitting.");
		mCurrent->state = job_done;
		mCurrent->cv.notify_all();
		mCurrent.reset();
	}

	mHTTP.stop();
	if (mListenThread.joinable())
		mListenThread.join();
}

void	WED_MCPServer::HandlePost(const httplib::Request& req, httplib::Response& res)
{
	if (!origin_ok(req))
	{
		res.status = 403;
		res.set_content("Forbidden origin", "text/plain");
		return;
	}

	Json::Value msg;
	string err;
	if (!from_json(req.body, msg, err))
	{
		res.set_content(to_json(rpc_error(Json::Value(), -32700, "Parse error: " + err)), "application/json");
		return;
	}
	if (!msg.isObject() || msg.get("jsonrpc", "").asString() != "2.0")
	{
		res.set_content(to_json(rpc_error(Json::Value(), -32600, "Invalid request: expected a single JSON-RPC 2.0 message")), "application/json");
		return;
	}

	// Notifications and responses to us get 202 and no body.
	if (!msg.isMember("id") || !msg.isMember("method"))
	{
		res.status = 202;
		return;
	}

	const Json::Value& id = msg["id"];
	string method = msg["method"].asString();
	const Json::Value& params = msg["params"];
	Json::Value reply;

	if (method == "initialize")
	{
		string asked = params.get("protocolVersion", "").asString();
		string version = kProtocolVersions[0];
		for(const char ** v = kProtocolVersions; *v; ++v)
			if (asked == *v)
				version = asked;

		Json::Value r;
		r["protocolVersion"] = version;
		r["capabilities"]["tools"]["listChanged"] = false;
		r["serverInfo"]["name"] = "WED";
		r["serverInfo"]["title"] = "WorldEditor";
		r["serverInfo"]["version"] = WED_VERSION_STRING;
		r["instructions"] = kInstructions;
		reply = rpc_result(id, r);
	}
	else if (method == "ping")
	{
		reply = rpc_result(id, Json::Value(Json::objectValue));
	}
	else if (method == "tools/list")
	{
		Json::Value r;
		r["tools"] = Json::Value(Json::arrayValue);
		for(int t = 0; t < mTools.size(); ++t)
		{
			Json::Value tool;
			tool["name"] = mTools[t].name;
			tool["description"] = mTools[t].description;
			tool["inputSchema"] = mSchemas[t];
			tool["annotations"]["readOnlyHint"] = mTools[t].read_only;
			tool["annotations"]["destructiveHint"] = mTools[t].destructive;
			tool["annotations"]["openWorldHint"] = false;
			r["tools"].append(tool);
		}
		reply = rpc_result(id, r);
	}
	else if (method == "tools/call")
	{
		reply = CallTool(params, id);
	}
	else
	{
		reply = rpc_error(id, -32601, "Method not found: " + method);
	}

	res.set_content(to_json(reply), "application/json");
}

Json::Value	WED_MCPServer::CallTool(const Json::Value& params, const Json::Value& id)
{
	string name = params.get("name", "").asString();
	int t;
	for(t = 0; t < mTools.size(); ++t)
		if (name == mTools[t].name)
			break;
	if (t == mTools.size())
		return rpc_error(id, -32602, "Unknown tool: " + name);

	Json::Value args = params.get("arguments", Json::Value(Json::objectValue));
	string bad = check_args(mSchemas[t], args);
	if (!bad.empty())
		return rpc_result(id, error_result("invalid_argument", bad));

	job_ptr job = std::make_shared<WED_MCPJob>();
	job->tool = &mTools[t];
	job->args = args;
	{
		std::lock_guard<std::mutex> l(mQueueLock);
		if (mStopping)
			return rpc_result(id, error_result("shutting_down", "WED is quitting."));
		mQueue.push_back(job);
	}

	std::unique_lock<std::mutex> jl(job->lock);
	if (!job->cv.wait_for(jl, std::chrono::seconds(JOB_START_TIMEOUT), [&job]() { return job->state != job_pending; }))
	{
		// Still pending: withdraw it so it can never run after we've told the agent it didn't.
		job->state = job_cancelled;
		return rpc_result(id, error_result("busy",
			"WED did not reach a safe point to run this tool (a mouse button is down, an edit is in progress, or another request is running). "
			"Nothing was changed. Wait and retry with the same arguments."));
	}
	if (job->state == job_cancelled)
		return rpc_result(id, error_result("shutting_down", "WED is quitting."));

	if (!job->cv.wait_for(jl, std::chrono::seconds(JOB_RUN_TIMEOUT), [&job]() { return job->state == job_done; }))
		return rpc_result(id, error_result("timeout", "The tool started but did not finish in time. It may still complete; check get_state."));

	return rpc_result(id, job->result);
}

bool	WED_MCPServer::IsSafePoint(void) const
{
	if (gApplication->IsDeferring())
		return false;
	for(int n = 0; n < WED_StartWindow::CountOpenDocuments(); ++n)
	{
		WED_Document * d; WED_DocumentWindow * w; string p;
		WED_StartWindow::GetNthOpenDocument(n, d, w, p);
		if (d->GetUndoMgr()->IsCommandOpen())
			return false;
	}
	return true;
}

void	WED_MCPServer::SnapshotCacheKeys(vector<long long>& keys) const
{
	keys.clear();
	for(int n = 0; n < WED_StartWindow::CountOpenDocuments(); ++n)
	{
		WED_Document * d; WED_DocumentWindow * w; string p;
		WED_StartWindow::GetNthOpenDocument(n, d, w, p);
		keys.push_back(d->GetArchive()->CacheKey());
	}
}

void	WED_MCPServer::TimerFired(void)
{
	if (mInHandler)
		return;

	if (!mContinuations.empty())
	{
		vector<std::function<bool()> > run;
		run.swap(mContinuations);
		mInHandler = true;
		for(int n = 0; n < run.size(); ++n)
			if (!run[n]())
				mContinuations.push_back(run[n]);
		mInHandler = false;
	}

	if (mCurrent)
		return;

	job_ptr job;
	{
		std::lock_guard<std::mutex> l(mQueueLock);
		while (!mQueue.empty() && !job)
		{
			job_ptr j = mQueue.front();
			std::lock_guard<std::mutex> jl(j->lock);
			if (j->state == job_cancelled)
				mQueue.pop_front();
			else if (IsSafePoint())
			{
				mQueue.pop_front();
				j->state = job_running;
				j->cv.notify_all();
				job = j;
			}
			else
				return;
		}
	}
	if (!job)
		return;

	mCurrent = job;
	job->alert_seq = WED_MCPHeadless_NextSeq();
	if (job->tool->read_only)
		SnapshotCacheKeys(job->cache_keys);

	WED_MCPCall call(job);
	mInHandler = true;
	job->tool->handler(call, job->args);
	mInHandler = false;
}

void	WED_MCPServer::FinishJob(WED_MCPJob * job, const Json::Value& in_result)
{
	Json::Value result(in_result);

	// Report every dialog that was auto-answered while this tool ran.
	Json::Value alerts = WED_MCPHeadless_GetAlerts(job->alert_seq);
	if (alerts.size() > 0)
	{
		Json::Value item;
		item["type"] = "text";
		item["text"] = "alerts: " + to_json(alerts);
		result["content"].append(item);
	}

	// Postconditions - these are WED bugs, not agent errors.
	for(int n = 0; n < WED_StartWindow::CountOpenDocuments(); ++n)
	{
		WED_Document * d; WED_DocumentWindow * w; string p;
		WED_StartWindow::GetNthOpenDocument(n, d, w, p);
		if (d->GetUndoMgr()->IsCommandOpen())
		{
			LOG_MSG("E/MCP tool %s left an undo command open in %s\n", job->tool->name, p.c_str());
			DebugAssert(!"MCP tool left an undo command open");
		}
	}
	if (job->tool->read_only)
	{
		vector<long long> keys;
		SnapshotCacheKeys(keys);
		if (keys != job->cache_keys)
		{
			LOG_MSG("E/MCP read-only tool %s changed a document\n", job->tool->name);
			DebugAssert(!"MCP read-only tool changed a document");
		}
	}

	std::lock_guard<std::mutex> jl(job->lock);
	job->result = result;
	job->state = job_done;
	job->cv.notify_all();
	if (mCurrent.get() == job)
		mCurrent.reset();
}

#pragma mark -

WED_MCPCall::WED_MCPCall(std::shared_ptr<WED_MCPJob> job) : mJob(job)
{
}

void	WED_MCPCall::Reply(const Json::Value& data)
{
	Json::Value r;
	r["content"][0]["type"] = "text";
	r["content"][0]["text"] = to_json(data);
	ReplyContent(r["content"]);
}

void	WED_MCPCall::ReplyContent(const Json::Value& content)
{
	DebugAssert(!IsDone());
	if (IsDone() || !sServer) return;
	Json::Value r;
	r["content"] = content;
	sServer->FinishJob(mJob.get(), r);
}

void	WED_MCPCall::Error(const string& code, const string& msg, const Json::Value& extra)
{
	DebugAssert(!IsDone());
	if (IsDone() || !sServer) return;
	sServer->FinishJob(mJob.get(), error_result(code, msg, extra));
}

bool	WED_MCPCall::IsDone(void) const
{
	std::lock_guard<std::mutex> jl(mJob->lock);
	return mJob->state == job_done;
}

#pragma mark -

bool	WED_MCP_Start(int port, WED_StartWindow * start_window)
{
	DebugAssert(sServer == nullptr);
	sServer = new WED_MCPServer(start_window);
	if (!sServer->Start(port))
	{
		delete sServer;
		sServer = nullptr;
		// No alert: whoever asked for --mcp is probably a script, and a modal would just hang it.
		string msg = "WED could not start its MCP server: port " + to_string(port) + " on 127.0.0.1 is in use. Use --mcp_port=N to pick another.";
		LOG_MSG("E/MCP %s\n", msg.c_str());
		LOG_FLUSH();
		fprintf(stderr, "%s\n", msg.c_str());
		return false;
	}
	WED_MCPHeadless_Install();
	return true;
}

void	WED_MCP_Stop(void)
{
	if (!sServer)
		return;
	WED_MCPHeadless_Remove();
	sServer->Stop();
	delete sServer;
	sServer = nullptr;
}

void	WED_MCP_RunLater(std::function<bool()> step)
{
	if (sServer)
		sServer->RunLater(step);
}

WED_StartWindow *	WED_MCP_GetStartWindow(void)
{
	return sServer ? sServer->mStartWindow : nullptr;
}
