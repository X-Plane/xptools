#!/usr/bin/env python3
"""End-to-end check of WED's MCP map/UI tools: viewport, screenshots, map tools, mouse gestures, keys.

usage: test_phase2.py <path to the WED executable>
"""
import base64, json, os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from wed_mcp import call, rpc, launch, quit, ToolError

HERE = os.path.dirname(os.path.abspath(__file__))
failures = 0
def check(label, ok):
    global failures
    print(("PASS " if ok else "FAIL ") + label)
    failures += 0 if ok else 1

def children(cls=None, name=None):
    d = call("dump_document")
    return [c for c in d["root"]["children"][0]["children"]
            if (cls is None or c["class"] == cls) and (name is None or c["props"].get("hierarchy.name") == name)]

def main(wed):
    proc, scratch = launch(wed)
    call("new_package", name="MCP Map")
    call("execute_command", name="wed_CreateApt")
    fix = call("inject_fixture", **json.load(open(os.path.join(HERE, "fixture_runway_taxiroute.json"))))

    v = call("set_viewport", ids=fix["ids"])
    w, s, e, n = v["bounds"]
    check("set_viewport fits the fixture", w < -71.01 and e > -70.99 and s < 42.0 and n > 42.002)

    r = rpc("tools/call", {"name": "capture_screenshot", "arguments": {"region": "map", "max_width": 400}})["result"]
    img = [c for c in r["content"] if c["type"] == "image"]
    info = json.loads([c for c in r["content"] if c["type"] == "text"][0]["text"])
    check("screenshot returns a PNG of the requested size",
          img and base64.b64decode(img[0]["data"])[:4] == b"\x89PNG" and info["width"] <= 400)
    p = call("capture_screenshot", path=os.path.join(scratch, "shot.png"))
    check("screenshot to file", os.path.getsize(p["path"]) == p["bytes"])

    base = call("dump_document", ids=False)
    call("set_tool", name="Vertex")
    r = call("mouse", action="drag", points=[{"lat": 42.002, "lon": -71.0}, {"lat": 42.003, "lon": -70.998}])
    a2 = children(name="A2")[0]["props"]
    check("vertex drag moves the node (to within a pixel)",
          abs(a2["point.latitude"] - 42.003) < 2e-5 and abs(a2["point.longitude"] + 70.998) < 2e-5 and r["undo"][0] == "Vertex Modification")
    call("execute_command", name="gui_Undo")
    check("undo of the drag restores", call("dump_document", ids=False) == base)

    rwy = children(cls="WED_Runway")[0]["id"]
    a1 = children(name="A1")[0]["id"]
    call("mouse", action="click", points=[{"lat": 42.0, "lon": -71.005}])
    check("click selects the runway", call("dump_document", max_depth=0)["selection"] == [rwy])
    call("mouse", action="click", points=[{"lat": 42.001, "lon": -71.0}], modifiers=["shift"])
    check("shift-click adds to the selection", sorted(call("dump_document", max_depth=0)["selection"]) == sorted([rwy, a1]))

    try:
        call("mouse", action="click", points=[{"lat": 45.0, "lon": -71.0}]); check("offscreen point rejected", False)
    except ToolError as ex:
        check("offscreen point rejected", ex.args[0]["error"] == "offscreen")

    base = call("dump_document", ids=False)
    call("set_tool", name="Taxi Routes")
    for lat, lon in [(42.004, -71.006), (42.004, -71.002), (42.006, -71.002)]:
        call("mouse", action="click", points=[{"lat": lat, "lon": lon}])
    r = call("key", key="return")
    check("clicks + Return create a taxi route", r["handled"] and r["undo"][0] == "Create Taxiway Route Line"
          and len(children(cls="WED_TaxiRoute")) == 3)
    call("execute_command", name="gui_Undo")
    check("undo of the created route restores", call("dump_document", ids=False) == base)

    call("key", key="m")
    check("tool shortcut key switches tools", [t["name"] for t in call("list_tools")["tools"] if t["current"]] == ["Marquee"])

    call("close_document", if_dirty="discard")
    check("WED exits on gui_Quit", quit(proc))
    print("%d failure(s); scratch folder %s" % (failures, scratch))
    sys.exit(1 if failures else 0)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    main(sys.argv[1])
