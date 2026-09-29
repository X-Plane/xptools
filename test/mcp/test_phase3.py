#!/usr/bin/env python3
"""End-to-end check of WED's MCP validation and export tools.

usage: test_phase3.py <path to the WED executable>
"""
import json, os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from wed_mcp import call, launch, quit, ToolError

HERE = os.path.dirname(os.path.abspath(__file__))
failures = 0
def check(label, ok):
    global failures
    print(("PASS " if ok else "FAIL ") + label)
    failures += 0 if ok else 1

def main(wed):
    proc, scratch = launch(wed)
    call("new_package", name="MCP Export")
    call("execute_command", name="wed_CreateApt")
    fix = json.load(open(os.path.join(HERE, "fixture_runway_taxiroute.json")))
    call("inject_fixture", **fix)

    v = call("validate")
    check("fixture validates with warnings only", v["result"] == "warnings_only" and v["error_count"] == 0
          and all(m["code"].startswith("warn_") and m["object_ids"] for m in v["messages"]))
    before = call("get_state")["export_target"]
    g = call("validate", target="gateway")
    check("gateway target is stricter and is restored afterwards",
          g["error_count"] > 0 and g["export_target"] == "gateway" and call("get_state")["export_target"] == before)

    apt = os.path.join(scratch, "apt.dat")
    r = call("export_apt", path=apt)
    text = open(apt).read()
    check("export_apt writes the runway", r["bytes"] == os.path.getsize(apt) and " 09 " in text and " 27 " in text)
    r = call("export_pack")
    check("export_pack writes the package apt.dat", "Earth nav data/apt.dat" in r["written"] or
          "Earth nav data\\apt.dat" in r["written"])

    base = call("dump_document", ids=False)
    call("inject_fixture", objects=[fix["objects"][0]])      # a second 09/27 runway
    v = call("validate")
    dup = [m for m in v["messages"] if m["code"] == "err_duplicate_name"]
    check("duplicate runway is an error naming both runways", v["result"] == "errors" and dup and len(dup[0]["object_ids"]) == 2)
    bad = os.path.join(scratch, "bad_apt.dat")
    for tool, args in [("export_apt", {"path": bad}), ("export_pack", {})]:
        try:
            call(tool, **args); check(tool + " refuses on validation errors", False)
        except ToolError as e:
            check(tool + " refuses on validation errors", e.args[0]["error"] == "validation_failed")
    check("nothing written on failure", not os.path.exists(bad))
    call("execute_command", name="gui_Undo")
    check("undo removes the duplicate", call("dump_document", ids=False) == base)

    call("close_document", if_dirty="discard")
    check("WED exits on gui_Quit", quit(proc))
    print("%d failure(s); scratch folder %s" % (failures, scratch))
    sys.exit(1 if failures else 0)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    main(sys.argv[1])
