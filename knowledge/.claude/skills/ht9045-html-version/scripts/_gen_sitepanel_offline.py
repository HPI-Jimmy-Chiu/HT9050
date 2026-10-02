"""Generate the C++ offline SitePanel snapshot from Setup-current.json only."""
import json
import os
from datetime import datetime

ROOT = r"D:\HT9045"
SETUP = os.path.join(ROOT, "JSON", "Setup-current.json")
OUTPUT = os.path.join(ROOT, "JSON", "offline", "SitePanel-runtime.offline.json")
ROWS = 4
COLS = 8
COLOR_MAP = ["#00ff00", "#00ffff", "#c0c0c0", "#ffffff", "#ff0000"]


def value(section, key, default=0):
    entry = section.get(key, {})
    return entry.get("value", default)


def site_name(row, col):
    return chr(ord("A") + row) + chr(ord("a") + col)


def main():
    with open(SETUP, "r", encoding="utf-8") as source:
        setup = json.load(source)
    documents = setup.get("documents", {})
    configuration = documents.get("handlerCondition", {}).get("sections", {}).get("Configuration", {})
    dut_on_off = documents.get("testMode", {}).get("sections", {}).get("DutOnOff", {})
    cells = []
    for row in range(ROWS):
        for col in range(COLS):
            name = site_name(row, col)
            number = int(value(configuration, "Site " + name, 0) or 0)
            enabled_by_arm = [bool(value(dut_on_off, "Dut  " + name, 0)), bool(value(dut_on_off, "Dut  " + name + "2", 0))]
            state = "enabled" if number > 0 and any(enabled_by_arm) else ("disabled" if number > 0 else "unmapped")
            cells.append({"x": col, "y": row, "text": str(number) if number > 0 else "", "siteNumber": number,
                          "enabledByArm": enabled_by_arm, "state": state})
    standard = all(not cell["siteNumber"] or cell["siteNumber"] == cell["y"] + 1 + cell["x"] * ROWS for cell in cells)
    for cell in cells:
        cell["colorIndex"] = 0 if cell["state"] == "enabled" and standard else (1 if cell["state"] == "enabled" else 2)
    snapshot = {
        "schemaVersion": "1.0.0",
        "generatedAt": datetime.now().astimezone().isoformat(timespec="seconds"),
        "source": {"toolchain": "BCB6", "mode": "cpp-offline", "input": "JSON/Setup-current.json",
                   "functions": ["TfMain::DrawTestSitePanel()", "TfMain::ShowTestHeadComp1()", "TfMain::CheckSiteMapIsStander()"]},
        "recipeName": setup.get("recipeName"),
        "layout": {"columns": COLS, "rows": ROWS},
        "colorMap": COLOR_MAP,
        "standardSiteMap": standard,
        "cells": cells,
        "note": "Lime/Aqua=enabled standard/nonstandard map; Silver=disabled or unmapped. White/Red require C++ runtime state.",
    }
    os.makedirs(os.path.dirname(OUTPUT), exist_ok=True)
    with open(OUTPUT, "w", encoding="utf-8", newline="\n") as target:
        json.dump(snapshot, target, ensure_ascii=False, indent=2)
        target.write("\n")
    print("ok:", OUTPUT, "recipe:", snapshot["recipeName"], "cells:", len(cells))


if __name__ == "__main__":
    main()
