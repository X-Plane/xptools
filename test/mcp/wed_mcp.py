"""Minimal client for WED's MCP server (see docs/knowledge/wed-mcp.md). Standard library only."""
import json, os, subprocess, sys, tempfile, time, urllib.request

URL = os.environ.get("WED_MCP_URL", "http://127.0.0.1:8087/mcp")

class ToolError(Exception):
    """A tool returned isError; args[0] is the decoded {error, message, ...} object."""

def rpc(method, params=None, timeout=400):
    body = json.dumps({"jsonrpc": "2.0", "id": 1, "method": method, "params": params or {}}).encode()
    req = urllib.request.Request(URL, body, {"Content-Type": "application/json"})
    return json.load(urllib.request.urlopen(req, timeout=timeout))

def call(tool, **args):
    """Call a tool; returns its decoded JSON result, with any auto-answered dialogs under '_alerts'."""
    r = rpc("tools/call", {"name": tool, "arguments": args})
    if "error" in r:
        raise RuntimeError(r["error"])
    res = r["result"]
    texts = [c["text"] for c in res["content"] if c["type"] == "text"]
    data = json.loads(texts[0])
    for t in texts[1:]:
        if t.startswith("alerts: "):
            data["_alerts"] = json.loads(t[len("alerts: "):])
    if res.get("isError"):
        raise ToolError(data)
    return data

def launch(wed):
    """Start WED with MCP against a throwaway X-Plane folder and prefs file.  Returns (process, scratch folder)."""
    scratch = tempfile.mkdtemp(prefix="wed_mcp_")
    xp = os.path.join(scratch, "xp")
    os.makedirs(os.path.join(xp, "Custom Scenery"))
    os.makedirs(os.path.join(xp, "Resources", "default scenery"))
    proc = subprocess.Popen([wed, "--mcp", "--prefs=" + os.path.join(scratch, "test.prefs"), "--xsystem=" + xp],
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    for _ in range(60):
        try:
            rpc("ping", timeout=2)
            return proc, scratch
        except Exception:
            if proc.poll() is not None:
                sys.exit("WED exited during startup (port 8087 in use?)")
            time.sleep(0.5)
    proc.kill()
    sys.exit("WED's MCP server never answered")

def quit(proc, timeout=20):
    """Quit WED via its menu command; returns True if it exited."""
    call("execute_command", name="gui_Quit")
    try:
        proc.wait(timeout=timeout)
        return True
    except subprocess.TimeoutExpired:
        proc.kill()
        return False
