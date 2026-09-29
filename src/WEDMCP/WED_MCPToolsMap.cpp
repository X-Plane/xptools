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

// Map and UI MCP tools: screenshots, the map viewport, map tools, synthetic mouse and keyboard input.
//
// COORDINATES: agents see window pixels with the origin at the TOP left, matching screenshots.  WED panes (and
// WED_Map's pixel space) use GL coordinates with the origin at the BOTTOM left.  Convert with img_y = win_h - gl_y.
// Map points can also be given as lat/lon, which is what fixtures are written in.

#include "WED_MCPTools.h"
#include "WED_MCPDocJson.h"
#include "WED_Document.h"
#include "WED_DocumentWindow.h"
#include "WED_StartWindow.h"
#include "WED_MapPane.h"
#include "WED_Map.h"
#include "WED_MapToolNew.h"
#include "WED_Thing.h"
#include "GUI_Window.h"
#include "GUI_Defs.h"
#include "IGIS.h"

#include <png.h>

#define CAPTURE_TIMEOUT		5.0		// seconds to wait for the window to draw
#define DEFAULT_MAX_WIDTH	1280	// screenshots are big in tokens; agents can ask for more

Json::Value		WED_MCP_DocSummary(WED_Document * doc, WED_DocumentWindow * win, const string& package);

static double	now_seconds(void)
{
	return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

static void	window_size(GUI_Window * w, int& width, int& height)
{
	int b[4];
	static_cast<GUI_Pane *>(w)->GetBounds(b);		// pane bounds: GL coordinates, what events and the map use
	width = b[2] - b[0];
	height = b[3] - b[1];
}

// A pane's bounds as [x1, y1, x2, y2] in top-left window pixels.
static Json::Value	pane_rect(GUI_Pane * p, int win_h)
{
	int b[4];
	p->GetBounds(b);
	Json::Value r(Json::arrayValue);
	r.append(b[0]); r.append(win_h - b[3]); r.append(b[2]); r.append(win_h - b[1]);
	return r;
}

static Json::Value	viewport(WED_DocumentWindow * win)
{
	WED_Map * map = win->GetMapPane()->GetMap();
	int ww, wh;
	window_size(win, ww, wh);
	double w, s, e, n;
	map->GetMapVisibleBounds(w, s, e, n);
	Json::Value r;
	r["bounds"].append(w); r["bounds"].append(s); r["bounds"].append(e); r["bounds"].append(n);
	r["map_rect"] = pane_rect(map, wh);
	r["window_size"].append(ww); r["window_size"].append(wh);
	r["pixels_per_meter"] = map->GetPPM();
	return r;
}

#pragma mark -

struct	png_buf { vector<unsigned char> data; };

static void	png_write_cb(png_structp png, png_bytep bytes, png_size_t len)
{
	png_buf * b = (png_buf *) png_get_io_ptr(png);
	b->data.insert(b->data.end(), bytes, bytes + len);
}

static bool	encode_png(int w, int h, const vector<unsigned char>& rgb_top_down, vector<unsigned char>& out)
{
	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png) return false;
	png_infop info = png_create_info_struct(png);
	if (!info || setjmp(png_jmpbuf(png)))
	{
		png_destroy_write_struct(&png, info ? &info : nullptr);
		return false;
	}
	png_buf buf;
	png_set_write_fn(png, &buf, png_write_cb, nullptr);
	png_set_IHDR(png, info, w, h, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png, info);
	for(int y = 0; y < h; ++y)
		png_write_row(png, (png_bytep) &rgb_top_down[(size_t) y * w * 3]);
	png_write_end(png, info);
	png_destroy_write_struct(&png, &info);
	out.swap(buf.data);
	return true;
}

static string	base64(const vector<unsigned char>& in)
{
	static const char * k = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	string out;
	out.reserve((in.size() + 2) / 3 * 4);
	for(size_t i = 0; i < in.size(); i += 3)
	{
		unsigned v = in[i] << 16;
		if (i + 1 < in.size()) v |= in[i+1] << 8;
		if (i + 2 < in.size()) v |= in[i+2];
		out += k[(v >> 18) & 63];
		out += k[(v >> 12) & 63];
		out += i + 1 < in.size() ? k[(v >> 6) & 63] : '=';
		out += i + 2 < in.size() ? k[v & 63] : '=';
	}
	return out;
}

// Crop (top-left window pixel rect), flip to top-down, drop alpha, and box-filter down by an integer factor.
static void	make_image(int w, int h, const vector<unsigned char>& rgba_bottom_up, int cx1, int cy1, int cx2, int cy2,
						int factor, int& out_w, int& out_h, vector<unsigned char>& rgb)
{
	out_w = (cx2 - cx1) / factor;
	out_h = (cy2 - cy1) / factor;
	rgb.assign((size_t) out_w * out_h * 3, 0);
	for(int y = 0; y < out_h; ++y)
	for(int x = 0; x < out_w; ++x)
	{
		unsigned sum[3] = { 0, 0, 0 };
		for(int dy = 0; dy < factor; ++dy)
		for(int dx = 0; dx < factor; ++dx)
		{
			int sx = cx1 + x * factor + dx;
			int sy = h - 1 - (cy1 + y * factor + dy);		// GL rows are bottom-up
			const unsigned char * p = &rgba_bottom_up[((size_t) sy * w + sx) * 4];
			sum[0] += p[0]; sum[1] += p[1]; sum[2] += p[2];
		}
		unsigned char * o = &rgb[((size_t) y * out_w + x) * 3];
		for(int c = 0; c < 3; ++c)
			o[c] = sum[c] / (factor * factor);
	}
}

static void	capture_screenshot(WED_MCPCall& call, const Json::Value& args)
{
	string window = args.get("window", "document").asString();
	string region = args.get("region", "window").asString();
	int max_width = DEFAULT_MAX_WIDTH;
	string path;
	if (!WED_MCP_GetInt(call, args, "max_width", max_width, false)) return;
	if (!WED_MCP_GetString(call, args, "path", path, false)) return;

	GUI_Window * target = nullptr;
	GUI_Pane * crop = nullptr;
	if (window == "start")
		target = WED_MCP_GetStartWindow();
	else
	{
		WED_Document * doc; WED_DocumentWindow * win; string package;
		if (!WED_MCP_FindDocument(call, args, doc, win, package)) return;
		target = win;
		if (region == "map")
			crop = win->GetMapPane()->GetMap();
	}
	if (region == "map" && !crop)
	{
		call.Error("invalid_argument", "region 'map' needs a document window");
		return;
	}
	if (!target->IsVisibleNow())
	{
		call.Error("window_hidden", "That window is not visible, so it will not draw.");
		return;
	}

	struct capture_state {
		bool done = false;
		int w = 0, h = 0;
		vector<unsigned char> rgba;
	};
	std::shared_ptr<capture_state> st = std::make_shared<capture_state>();
	target->RequestCapture([st](int w, int h, const vector<unsigned char>& rgba) {
		st->w = w; st->h = h; st->rgba = rgba; st->done = true;
	});

	// The capture happens inside the window's next draw; wait for it without blocking the main thread.
	double deadline = now_seconds() + CAPTURE_TIMEOUT;
	WED_MCPCall c(call);
	int crop_rect[4] = { 0, 0, 0, 0 };
	if (crop)
		crop->GetBounds(crop_rect);
	bool has_crop = crop != nullptr;
	WED_MCP_RunLater([c, st, deadline, max_width, path, has_crop, crop_rect, target]() mutable {
		if (!st->done)
		{
			if (now_seconds() < deadline) return false;
			target->RequestCapture(GUI_Window::CaptureFunc());		// cancel
			c.Error("capture_timeout", "The window did not draw within the timeout (minimized or offscreen?).");
			return true;
		}
		int x1 = 0, y1 = 0, x2 = st->w, y2 = st->h;
		if (has_crop)
		{
			// GL bounds -> top-left pixels, clamped to the framebuffer
			x1 = max(0, crop_rect[0]); x2 = min(st->w, crop_rect[2]);
			y1 = max(0, st->h - crop_rect[3]); y2 = min(st->h, st->h - crop_rect[1]);
		}
		int factor = 1;
		if (max_width > 0)
			while ((x2 - x1) / factor > max_width) ++factor;
		int ow, oh;
		vector<unsigned char> rgb, png;
		make_image(st->w, st->h, st->rgba, x1, y1, x2, y2, factor, ow, oh, rgb);
		if (!encode_png(ow, oh, rgb, png))
		{
			c.Error("encode_failed", "PNG encoding failed.");
			return true;
		}

		Json::Value info;
		info["width"] = ow;
		info["height"] = oh;
		info["scale"] = factor;
		info["origin_in_window"].append(x1); info["origin_in_window"].append(y1);
		info["window_size"].append(st->w); info["window_size"].append(st->h);
		info["note"] = "window pixel = origin_in_window + image pixel * scale (top-left origin); pass window pixels to mouse";

		if (!path.empty())
		{
			FILE * f = fopen(path.c_str(), "wb");
			if (!f || fwrite(png.data(), 1, png.size(), f) != png.size())
			{
				if (f) fclose(f);
				c.Error("write_failed", "Could not write " + path);
				return true;
			}
			fclose(f);
			info["path"] = path;
			info["bytes"] = (Json::UInt64) png.size();
			c.Reply(info);
			return true;
		}

		Json::StreamWriterBuilder b;
		b["indentation"] = "";
		Json::Value content(Json::arrayValue);
		content[0]["type"] = "image";
		content[0]["mimeType"] = "image/png";
		content[0]["data"] = base64(png);
		content[1]["type"] = "text";
		content[1]["text"] = Json::writeString(b, info);
		c.ReplyContent(content);
		return true;
	});
}

#pragma mark -

static void	get_viewport(WED_MCPCall& call, const Json::Value& args)
{
	WED_Document * doc; WED_DocumentWindow * win; string package;
	if (!WED_MCP_FindDocument(call, args, doc, win, package)) return;
	call.Reply(viewport(win));
}

static void	set_viewport(WED_MCPCall& call, const Json::Value& args)
{
	WED_Document * doc; WED_DocumentWindow * win; string package;
	if (!WED_MCP_FindDocument(call, args, doc, win, package)) return;
	WED_MapPane * pane = win->GetMapPane();
	WED_Map * map = pane->GetMap();
	double margin = args.get("margin", 0.1).asDouble();

	int modes = args.isMember("bounds") + args.isMember("ids") + args.isMember("center") + args.get("all", false).asBool();
	if (modes != 1)
	{
		call.Error("invalid_argument", "Pass exactly one of bounds, ids, center or all:true.");
		return;
	}

	if (args.get("all", false).asBool())
		pane->ZoomShowAll();
	else if (args.isMember("center"))
	{
		const Json::Value& c = args["center"];
		if (!c.isMember("lat") || !c.isMember("lon"))
		{
			call.Error("invalid_argument", "center is {lat, lon}");
			return;
		}
		pane->CenterOnPoint(Point2(c["lon"].asDouble(), c["lat"].asDouble()));
	}
	else
	{
		Bbox2 box;
		if (args.isMember("bounds"))
		{
			const Json::Value& b = args["bounds"];
			if (b.size() != 4)
			{
				call.Error("invalid_argument", "bounds is [west, south, east, north]");
				return;
			}
			box = Bbox2(b[0].asDouble(), b[1].asDouble(), b[2].asDouble(), b[3].asDouble());
		}
		else
		{
			const Json::Value& ids = args["ids"];
			for(Json::ArrayIndex i = 0; i < ids.size(); ++i)
			{
				IGISEntity * e = dynamic_cast<IGISEntity *>(doc->GetArchive()->Fetch(ids[i].asInt()));
				if (!e)
				{
					call.Error("unknown_object", "Object " + ids[i].toStyledString() + " doesn't exist or has no geometry.");
					return;
				}
				Bbox2 b;
				e->GetBounds(gis_Geo, b);
				box += b;
			}
			if (box.is_null())
			{
				call.Error("invalid_argument", "ids is empty");
				return;
			}
		}
		// Pad, and keep a minimum size so a single point doesn't zoom to infinity.
		double pad = max(max(box.xspan(), box.yspan()) * margin, 0.0005);
		box.expand(pad);
		map->ZoomShowArea(box.p1.x(), box.p1.y(), box.p2.x(), box.p2.y());
		map->Refresh();
	}
	call.Reply(viewport(win));
}

#pragma mark -

static int	find_tool(WED_MapPane * pane, const string& name)
{
	for(int n = 0; n < pane->CountTools(); ++n)
		if (pane->GetNthTool(n) && name == pane->GetNthTool(n)->GetToolName())
			return n;
	return -1;
}

static Json::Value	tool_json(WED_MapPane * pane, int n)
{
	WED_MapToolNew * t = pane->GetNthTool(n);
	Json::Value j;
	j["name"] = t->GetToolName();
	j["current"] = pane->GetCurrentTool() == n;
	j["props"] = WED_MCP_DumpDisplayProperties(t);
	const char * status = t->GetStatusText();
	if (status && *status)
		j["status"] = status;
	return j;
}

static void	list_tools(WED_MCPCall& call, const Json::Value& args)
{
	WED_Document * doc; WED_DocumentWindow * win; string package;
	if (!WED_MCP_FindDocument(call, args, doc, win, package)) return;
	WED_MapPane * pane = win->GetMapPane();
	Json::Value r(Json::arrayValue);
	for(int n = 0; n < pane->CountTools(); ++n)
		if (pane->GetNthTool(n))
			r.append(tool_json(pane, n));
	Json::Value ret;
	ret["tools"] = r;
	call.Reply(ret);
}

static void	set_tool(WED_MCPCall& call, const Json::Value& args)
{
	WED_Document * doc; WED_DocumentWindow * win; string package;
	if (!WED_MCP_FindDocument(call, args, doc, win, package)) return;
	WED_MapPane * pane = win->GetMapPane();
	string name;
	if (!WED_MCP_GetString(call, args, "name", name)) return;
	int n = find_tool(pane, name);
	if (n < 0)
	{
		Json::Value extra;
		for(int t = 0; t < pane->CountTools(); ++t)
			if (pane->GetNthTool(t))
				extra["valid"].append(pane->GetNthTool(t)->GetToolName());
		call.Error("unknown_tool", "No map tool named '" + name + "'.", extra);
		return;
	}
	if (pane->GetCurrentTool() != n)
		pane->SetCurrentTool(n);
	if (args.isMember("props"))
	{
		// Tool settings aren't document state, so no undo command.
		WED_MCPError err;
		if (!WED_MCP_SetDisplayProperties(pane->GetNthTool(n), name.c_str(), args["props"], err))
		{
			call.Error(err.code, err.message, err.extra);
			return;
		}
	}
	call.Reply(tool_json(pane, n));
}

#pragma mark -

struct	synth_step {
	int kind;
	int x, y;		// window GL coordinates
};

static bool	parse_modifiers(WED_MCPCall& call, const Json::Value& args, GUI_KeyFlags& flags)
{
	flags = 0;
	const Json::Value& m = args["modifiers"];
	for(Json::ArrayIndex i = 0; i < m.size(); ++i)
	{
		string s = m[i].asString();
		if (s == "shift")						flags |= gui_ShiftFlag;
		else if (s == "ctrl" || s == "cmd")		flags |= gui_ControlFlag;		// WED maps Cmd on Mac to its control flag
		else if (s == "alt" || s == "option")	flags |= gui_OptionAltFlag;
		else
		{
			call.Error("invalid_argument", "modifiers are shift, ctrl (cmd on Mac) and alt (option)");
			return false;
		}
	}
	return true;
}

// Resolve one point to window GL coordinates.  Lat/lon points must be inside the visible map.
static bool	resolve_point(WED_MCPCall& call, WED_DocumentWindow * win, const Json::Value& p, int& gx, int& gy)
{
	int ww, wh;
	window_size(win, ww, wh);
	if (p.isMember("lat") && p.isMember("lon"))
	{
		WED_Map * map = win->GetMapPane()->GetMap();
		Point2 px = map->LLToPixel(Point2(p["lon"].asDouble(), p["lat"].asDouble()));
		gx = (int) lround(px.x());
		gy = (int) lround(px.y());
		int b[4];
		map->GetBounds(b);
		if (gx < b[0] || gx >= b[2] || gy < b[1] || gy >= b[3])
		{
			Json::Value extra;
			extra["viewport"] = viewport(win);
			call.Error("offscreen", "Point " + p.toStyledString() + " is outside the visible map. Use set_viewport first.", extra);
			return false;
		}
		return true;
	}
	if (p.isMember("x") && p.isMember("y"))
	{
		gx = p["x"].asInt();
		gy = wh - p["y"].asInt();
		if (gx < 0 || gx >= ww || gy <= 0 || gy > wh)
		{
			call.Error("offscreen", "Point " + p.toStyledString() + " is outside the window.");
			return false;
		}
		return true;
	}
	call.Error("invalid_argument", "Each point is {lat, lon} (map) or {x, y} (window pixels, top-left origin).");
	return false;
}

static void	mouse(WED_MCPCall& call, const Json::Value& args)
{
	WED_Document * doc; WED_DocumentWindow * win; string package;
	if (!WED_MCP_FindDocument(call, args, doc, win, package)) return;
	string action = args.get("action", "click").asString();
	int button = args.get("button", 0).asInt();
	int steps = max(1, min(args.get("steps", 4).asInt(), 50));
	GUI_KeyFlags flags;
	if (!parse_modifiers(call, args, flags)) return;
	if (button < 0 || button > 1)
	{
		call.Error("invalid_argument", "button is 0 (left) or 1 (right)");
		return;
	}

	const Json::Value& pts = args["points"];
	vector<pair<int,int> > gl;
	for(Json::ArrayIndex i = 0; i < pts.size(); ++i)
	{
		int x, y;
		if (!resolve_point(call, win, pts[i], x, y)) return;
		gl.push_back(make_pair(x, y));
	}
	if (gl.empty() || (action == "click" && gl.size() != 1) || (action == "drag" && gl.size() < 2))
	{
		call.Error("invalid_argument", "click takes 1 point, drag 2 or more (start, ..., end), move 1 or more");
		return;
	}

	vector<synth_step> script;
	if (action == "move")
	{
		for(int i = 0; i < gl.size(); ++i)
			script.push_back({ GUI_Window::synth_move, gl[i].first, gl[i].second });
	}
	else
	{
		script.push_back({ GUI_Window::synth_down, gl[0].first, gl[0].second });
		// Drags are fed as several intermediate events per segment, like a real hand, so tools see real motion.
		for(int i = 1; i < gl.size(); ++i)
			for(int s = 1; s <= steps; ++s)
				script.push_back({ GUI_Window::synth_drag,
					gl[i-1].first + (gl[i].first - gl[i-1].first) * s / steps,
					gl[i-1].second + (gl[i].second - gl[i-1].second) * s / steps });
		script.push_back({ GUI_Window::synth_up, gl.back().first, gl.back().second });
	}

	// One event per server tick, so each one is fully handled (and the map redraws) before the next - and the job
	// holds the queue for the whole gesture, so nothing else runs mid-drag.
	win->FocusChain(1);
	GUI_Pane::SetModifiersOverride(true, flags);
	WED_MCPCall c(call);
	std::shared_ptr<int> next = std::make_shared<int>(0);
	WED_MCP_RunLater([c, script, next, win, doc, package, button, flags]() mutable {
		const synth_step& s = script[(*next)++];
		GUI_Pane::SetModifiersOverride(true, flags);
		win->SynthMouse(s.kind, s.x, s.y, button);
		if (*next < script.size())
			return false;
		GUI_Pane::SetModifiersOverride(false);
		Json::Value r = WED_MCP_DocSummary(doc, win, package);
		r["events"] = (int) script.size();
		c.Reply(r);
		return true;
	});
}

struct	named_key { const char * name; uint32_t key; int vk; };
static const named_key kKeys[] = {
	{ "return",		GUI_KEY_RETURN,	GUI_VK_RETURN },
	{ "enter",		GUI_KEY_RETURN,	GUI_VK_RETURN },
	{ "escape",		GUI_KEY_ESCAPE,	GUI_VK_ESCAPE },
	{ "tab",		GUI_KEY_TAB,	GUI_VK_TAB },
	{ "backspace",	GUI_KEY_BACK,	GUI_VK_BACK },
	{ "delete",		GUI_KEY_DELETE,	GUI_VK_DELETE },
	{ "left",		GUI_KEY_LEFT,	GUI_VK_LEFT },
	{ "right",		GUI_KEY_RIGHT,	GUI_VK_RIGHT },
	{ "up",			GUI_KEY_UP,		GUI_VK_UP },
	{ "down",		GUI_KEY_DOWN,	GUI_VK_DOWN },
	{ "space",		' ',			GUI_VK_SPACE },
	{ "home",		0,				GUI_VK_HOME },
	{ "end",		0,				GUI_VK_END },
	{ "pageup",		0,				GUI_VK_PRIOR },
	{ "pagedown",	0,				GUI_VK_NEXT },
	{ ",",			',',			GUI_VK_COMMA },
	{ ".",			'.',			GUI_VK_PERIOD },
	{ nullptr, 0, 0 }
};

static void	key(WED_MCPCall& call, const Json::Value& args)
{
	WED_Document * doc; WED_DocumentWindow * win; string package;
	if (!WED_MCP_FindDocument(call, args, doc, win, package)) return;
	string name;
	if (!WED_MCP_GetString(call, args, "key", name)) return;
	GUI_KeyFlags flags;
	if (!parse_modifiers(call, args, flags)) return;

	uint32_t k = 0;
	int vk = 0;
	for(const named_key * n = kKeys; n->name; ++n)
		if (name == n->name) { k = n->key; vk = n->vk; }
	if (!vk && name.size() == 1 && isalnum((unsigned char) name[0]))
	{
		k = (flags & gui_ShiftFlag) ? toupper(name[0]) : tolower(name[0]);
		vk = toupper(name[0]);		// GUI_VK_A..Z and 0..9 are ASCII
	}
	if (!vk)
	{
		Json::Value extra;
		for(const named_key * n = kKeys; n->name; ++n)
			extra["valid"].append(n->name);
		call.Error("unknown_key", "Keys are a single letter or digit, or one of the listed names.", extra);
		return;
	}

	win->FocusChain(1);
	GUI_Pane::SetModifiersOverride(true, flags);
	// A real key press is a down event then an up event; handlers (e.g. the create tools) act on the down.
	int handled = win->DispatchKeyPress(k, vk, flags | gui_DownFlag);
	win->DispatchKeyPress(k, vk, flags | gui_UpFlag);
	GUI_Pane::SetModifiersOverride(false);

	Json::Value r = WED_MCP_DocSummary(doc, win, package);
	r["handled"] = handled != 0;
	call.Reply(r);
}

#pragma mark -

void	WED_MCP_RegisterMapTools(vector<WED_MCPTool>& tools)
{
	tools.push_back({ "capture_screenshot",
		"PNG of a document window (default) or the start window, as rendered by WED itself - works whether or not WED is "
		"frontmost, but not while minimized. region 'map' crops to the map. Downscaled to max_width (default 1280); the "
		"result says how image pixels map back to window pixels for the mouse tool. Native menus and OS dialogs are not included.",
		R"({"type":"object","properties":{"doc":{"type":"string","description":"Package name; default: the active or only document."},)"
		R"("window":{"type":"string","enum":["document","start"]},"region":{"type":"string","enum":["window","map"]},)"
		R"("max_width":{"type":"integer","description":"Downscale (by whole factors) until at most this wide; 0 = full size."},)"
		R"("path":{"type":"string","description":"Write the PNG here instead of returning it."}},"additionalProperties":false})",
		true, false, capture_screenshot });

	tools.push_back({ "get_viewport",
		"The map's visible area [west, south, east, north], where the map sits in the window (top-left pixels), window size and zoom.",
		R"({"type":"object","properties":{"doc":{"type":"string","description":"Package name; default: the active or only document."}},"additionalProperties":false})",
		true, false, get_viewport });

	tools.push_back({ "set_viewport",
		"Pan/zoom the map: to bounds [west, south, east, north], to fit objects (ids), centered on {lat, lon}, or all:true. "
		"The view isn't document state, so this isn't undoable.",
		R"({"type":"object","properties":{"doc":{"type":"string","description":"Package name; default: the active or only document."},)"
		R"("bounds":{"type":"array","items":{"type":"number"}},"ids":{"type":"array","items":{"type":"integer"}},)"
		R"("center":{"type":"object","properties":{"lat":{"type":"number"},"lon":{"type":"number"}}},"all":{"type":"boolean"},)"
		R"("margin":{"type":"number","description":"Extra space around bounds/ids as a fraction of their size (default 0.1)."}},"additionalProperties":false})",
		false, false, set_viewport });

	tools.push_back({ "list_tools",
		"The map toolbar's tools (Vertex, Marquee, Runway, Taxiway, Taxi Routes, Objects...), which is current, and each tool's settings.",
		R"({"type":"object","properties":{"doc":{"type":"string","description":"Package name; default: the active or only document."}},"additionalProperties":false})",
		true, false, list_tools });

	tools.push_back({ "set_tool",
		"Pick a map tool as if its toolbar button was clicked, and optionally change its settings (keys as in list_tools).",
		R"({"type":"object","properties":{"doc":{"type":"string","description":"Package name; default: the active or only document."},)"
		R"("name":{"type":"string"},"props":{"type":"object"}},"required":["name"],"additionalProperties":false})",
		false, false, set_tool });

	tools.push_back({ "mouse",
		"Synthetic mouse gesture in a document window, through the same code path as real events: click (1 point), drag "
		"(start, ..., end) or move (hover). Points are {lat, lon} on the map (must be visible - see set_viewport) or {x, y} in "
		"window pixels, top-left origin, as in capture_screenshot. The current map tool handles map clicks and owns any undo "
		"step. Modifiers: shift, ctrl (= cmd on Mac), alt.",
		R"({"type":"object","properties":{"doc":{"type":"string","description":"Package name; default: the active or only document."},)"
		R"("action":{"type":"string","enum":["click","drag","move"]},"points":{"type":"array","items":{"type":"object"}},)"
		R"("button":{"type":"integer","description":"0 left (default), 1 right."},)"
		R"("modifiers":{"type":"array","items":{"type":"string","enum":["shift","ctrl","cmd","alt","option"]}},)"
		R"("steps":{"type":"integer","description":"Drag events per segment (default 4)."}},"required":["points"],"additionalProperties":false})",
		false, false, mouse });

	tools.push_back({ "key",
		"Synthetic key press in a document window, as if typed with that window in front: goes to the focused field, else the "
		"map (tool shortcuts, Return to finish a shape, Escape, Delete...). Key: a letter or digit, or a name (return, escape, "
		"tab, backspace, delete, arrows, space, home, end, pageup, pagedown).",
		R"({"type":"object","properties":{"doc":{"type":"string","description":"Package name; default: the active or only document."},)"
		R"("key":{"type":"string"},"modifiers":{"type":"array","items":{"type":"string","enum":["shift","ctrl","cmd","alt","option"]}}},)"
		R"("required":["key"],"additionalProperties":false})",
		false, false, key });
}
