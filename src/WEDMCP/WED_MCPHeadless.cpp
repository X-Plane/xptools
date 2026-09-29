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

#include "WED_MCPHeadless.h"
#include "PlatformUtils.h"

#include <deque>

// Keep the log bounded - an agent that never reads it shouldn't grow WED forever.
#define MAX_ALERTS 500

struct	preset_answer_t {
	string			kind;		// "any" or a dialog kind
	string			answer;
	vector<string>	paths;
};

static	std::deque<Json::Value>			sAlerts;
static	int							sNextSeq = 1;
static	std::deque<preset_answer_t>		sPresets;

static bool	take_preset(const char * kind, preset_answer_t& out)
{
	for(std::deque<preset_answer_t>::iterator p = sPresets.begin(); p != sPresets.end(); ++p)
	if (p->kind == "any" || p->kind == kind)
	{
		out = *p;
		sPresets.erase(p);
		return true;
	}
	return false;
}

static void	record(const char * kind, const char * text, const Json::Value& buttons, const string& answer, bool preset)
{
	Json::Value a;
	a["seq"] = sNextSeq++;
	a["kind"] = kind;
	a["text"] = text ? text : "";
	if (!buttons.isNull())
		a["buttons"] = buttons;
	a["answer"] = answer;
	a["preset"] = preset;
	sAlerts.push_back(a);
	while (sAlerts.size() > MAX_ALERTS)
		sAlerts.pop_front();
	LOG_MSG("I/MCP %s '%s' answered '%s'%s\n", kind, text ? text : "", answer.c_str(), preset ? " (preset)" : "");
}

static int	hook_get_file_path(int inType, const char * inPrompt, const char * inAction, char * outFileName, int inBufSize, const char * initialPath)
{
	preset_answer_t p;
	bool has = take_preset("file", p) && !p.paths.empty();
	if (has)
	{
		strncpy(outFileName, p.paths[0].c_str(), inBufSize);
		outFileName[inBufSize-1] = 0;
	}
	record("file", inPrompt, Json::Value(), has ? p.paths[0] : "cancel", has);
	return has ? 1 : 0;
}

static char *	hook_get_multi_file_path(const char * inPrompt, const char * inAction, const char * initialPath)
{
	preset_answer_t p;
	bool has = take_preset("file", p) && !p.paths.empty();
	record("file", inPrompt, Json::Value(), has ? p.paths[0] : "cancel", has);
	if (!has)
		return NULL;

	// Same format as the OS version: 0-terminated strings, then an empty string; caller frees.
	int buf_size = 1;
	for(int i = 0; i < p.paths.size(); ++i)
		buf_size += p.paths[i].size() + 1;
	char * ret = (char *) malloc(buf_size);
	char * w = ret;
	for(int i = 0; i < p.paths.size(); ++i)
	{
		strcpy(w, p.paths[i].c_str());
		w += p.paths[i].size() + 1;
	}
	*w = 0;
	return ret;
}

static void	hook_user_alert(const char * inMsg)
{
	preset_answer_t p;
	bool has = take_preset("alert", p);
	record("alert", inMsg, Json::Value(), "ok", has);
}

static int	hook_confirm_message(const char * inMsg, const char * proceedBtn, const char * cancelBtn, const char * optionBtn)
{
	Json::Value buttons(Json::arrayValue);
	buttons.append(proceedBtn);
	if (optionBtn) buttons.append(optionBtn);
	buttons.append(cancelBtn);

	int result = 0;
	string answer("cancel");
	preset_answer_t p;
	bool has = take_preset("confirm", p);
	if (has)
	{
		// Accept either the role or the button's label.
		if (p.answer == "proceed" || p.answer == proceedBtn)						{ result = 1; answer = "proceed"; }
		else if (optionBtn && (p.answer == "option" || p.answer == optionBtn))	{ result = 2; answer = "option"; }
	}
	record("confirm", inMsg, buttons, answer, has);
	return result;
}

static int	hook_save_discard(const char * inMessage1, const char * inMessage2)
{
	Json::Value buttons(Json::arrayValue);
	buttons.append("save");
	buttons.append("discard");
	buttons.append("cancel");

	int result = close_Cancel;
	string answer("cancel");
	preset_answer_t p;
	bool has = take_preset("save_discard", p);
	if (has)
	{
		if (p.answer == "save")			{ result = close_Save;    answer = "save"; }
		else if (p.answer == "discard")	{ result = close_Discard; answer = "discard"; }
	}
	string text(inMessage1);
	if (inMessage2 && *inMessage2)
		text = text + "\n" + inMessage2;
	record("save_discard", text.c_str(), buttons, answer, has);
	return result;
}

static const PlatformModalHooks	kHooks = {
	hook_get_file_path,
	hook_get_multi_file_path,
	hook_user_alert,
	hook_confirm_message,
	hook_save_discard
};

void	WED_MCPHeadless_Install(void)
{
	gPlatformModalHooks = &kHooks;
}

void	WED_MCPHeadless_Remove(void)
{
	gPlatformModalHooks = nullptr;
}

int		WED_MCPHeadless_NextSeq(void)
{
	return sNextSeq;
}

Json::Value	WED_MCPHeadless_GetAlerts(int since)
{
	Json::Value ret(Json::arrayValue);
	for(std::deque<Json::Value>::iterator a = sAlerts.begin(); a != sAlerts.end(); ++a)
		if ((*a)["seq"].asInt() >= since)
			ret.append(*a);
	return ret;
}

string	WED_MCPHeadless_AddAnswers(const Json::Value& answers, bool replace, bool at_front)
{
	if (!answers.isArray())
		return "answers must be an array";

	std::deque<preset_answer_t> adds;
	for(Json::ArrayIndex i = 0; i < answers.size(); ++i)
	{
		const Json::Value& a = answers[i];
		if (!a.isObject())
			return "each answer must be an object";
		preset_answer_t p;
		p.kind = a.get("kind", "any").asString();
		if (p.kind != "any" && p.kind != "alert" && p.kind != "confirm" && p.kind != "save_discard" && p.kind != "file")
			return "unknown dialog kind '" + p.kind + "' - use any, alert, confirm, save_discard or file";
		p.answer = a.get("answer", "").asString();
		if (a.isMember("path"))
			p.paths.push_back(a["path"].asString());
		if (a.isMember("paths"))
			for(Json::ArrayIndex n = 0; n < a["paths"].size(); ++n)
				p.paths.push_back(a["paths"][n].asString());
		adds.push_back(p);
	}
	if (replace)
		sPresets.clear();
	sPresets.insert(at_front ? sPresets.begin() : sPresets.end(), adds.begin(), adds.end());
	return string();
}

Json::Value	WED_MCPHeadless_GetPendingAnswers(void)
{
	Json::Value ret(Json::arrayValue);
	for(std::deque<preset_answer_t>::iterator p = sPresets.begin(); p != sPresets.end(); ++p)
	{
		Json::Value a;
		a["kind"] = p->kind;
		if (!p->answer.empty())
			a["answer"] = p->answer;
		for(int n = 0; n < p->paths.size(); ++n)
			a["paths"].append(p->paths[n]);
		ret.append(a);
	}
	return ret;
}
