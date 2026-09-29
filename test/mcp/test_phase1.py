#!/usr/bin/env python3
"""End-to-end check of WED's MCP server: commands, dump/inject round trip, undo of every edit, modals, save/reopen, quit.

usage: test_phase1.py <path to the WED executable>
   e.g. test_phase1.py build_Debug/Debug/WED.app/Contents/MacOS/WED

Runs WED against a throwaway X-Plane folder and prefs file, so your own setup is never touched.
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

    call("new_package", name="MCP E2E")
    r = call("execute_command", name="wed_CreateApt")
    check("create airport is one undo step", r["documents"][0]["undo"] == ["Create Airport"])
    r = call("inject_fixture", **json.load(open(os.path.join(HERE, "fixture_runway_taxiroute.json"))))
    check("inject is one undo step, refs resolved, selected",
          r["undo"][0] == "MCP: Inject Fixture" and set(r["refs"]) == {"a", "b"} and r["selection_count"] == 4)
    d = call("dump_document")
    apt = d["root"]["children"][0]
    route = [c for c in apt["children"] if c["class"] == "WED_TaxiRoute"][0]
    check("taxi route sources are the two nodes", route["sources"] == [r["refs"]["a"], r["refs"]["b"]])
    rwy = [c for c in apt["children"] if c["class"] == "WED_Runway"][0]

    base = call("dump_document", ids=False)
    a = call("dump_document", root_id=apt["id"], ids=False)["root"]
    n = call("inject_fixture", parent_id=d["root"]["id"], objects=[a], select=False)
    check("dump -> inject -> dump is identical", call("dump_document", root_id=n["ids"][0], ids=False)["root"] == a)
    call("execute_command", name="gui_Undo")
    check("undo of inject restores document exactly", call("dump_document", ids=False) == base)

    call("set_properties", id=rwy["id"], props={"runway.surface": "Concrete", "line.width": 60.5})
    call("execute_command", name="gui_Undo")
    check("undo of set_properties restores", call("dump_document", ids=False) == base)
    call("set_selection", ids=[rwy["id"]])
    call("execute_command", name="gui_Undo")
    check("undo of set_selection restores", call("dump_document", ids=False) == base)

    # A runway rename may add WED's own "Smart Runway Rename" step, so undo back to the depth, not by count.
    depth = len(call("get_state")["documents"][0]["undo"])
    call("set_properties", id=rwy["id"], props={"hierarchy.name": "10/28"})
    while len(call("get_state")["documents"][0]["undo"]) > depth:
        call("execute_command", name="gui_Undo")
    check("undoing a rename back to the prior depth restores", call("dump_document", ids=False) == base)

    try:
        call("set_properties", id=rwy["id"], props={"runway.surface": "Chocolate"}); check("bad enum rejected", False)
    except ToolError as e:
        check("bad enum rejected with valid list, nothing changed",
              e.args[0]["error"] == "invalid_enum_value" and "valid" in e.args[0] and call("dump_document", ids=False) == base)

    path = os.path.join(scratch, "dump.json")
    p = call("dump_document", path=path)
    check("dump to file", p["bytes"] > 0 and json.load(open(path))["object_count"] == p["object_count"])

    r = call("execute_command", name="gui_Close")
    check("dirty close auto-cancelled and reported",
          [(x["kind"], x["answer"]) for x in r.get("_alerts", [])] == [("save_discard", "cancel")] and len(r["documents"]) == 1)
    saved = call("dump_document", ids=False, file_precision=True)["root"]
    r = call("execute_command", name="gui_Save")
    check("save clears dirty", r["documents"][0]["dirty"] is False)
    call("close_document")
    call("open_package", name="MCP E2E")
    check("reopened document matches saved state (file precision)",
          call("dump_document", ids=False, file_precision=True)["root"] == saved)
    call("close_document")

    check("WED exits on gui_Quit", quit(proc))
    check("test prefs were written (not the user's)", os.path.exists(os.path.join(scratch, "test.prefs")))
    print("%d failure(s); scratch folder %s" % (failures, scratch))
    sys.exit(1 if failures else 0)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    main(sys.argv[1])
