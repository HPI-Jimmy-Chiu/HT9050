# 由 BCB6 cinitial.cpp 的 SetSimuScreenPara() 抽出模擬畫面對照，產生 Sim-scale.json
#   SetScreenScale(screenA, screenB, machineA, machineB) → 螢幕像素 ↔ 機構座標線性對應
#   SetPanel(element, vertical)                          → 該馬達要移動的畫面元件
#   XXX.SetMyLed(row, col, led)                          → 吸嘴/Kit LED 矩陣
import json
import os
import re
from datetime import datetime

SRC = r"D:\HT9045\HT9011UC_Code_V3.33.910.0_20260716_NN Mode 2D + AutoClean V2"
OUT = r"D:\HT9045\JSON\Sim-scale.json"

s = open(os.path.join(SRC, "cinitial.cpp"), encoding="cp950", errors="replace").read()

# 同一顆馬達可能被多次設定（不同機種分支）→ 取最後一次
scale = {}
for m in re.finditer(r"MOT\[(\w+)\]\.SetScreenScale\(([^;]*)\);", s):
    mid = m.group(1)
    args = [a.strip() for a in re.split(r",(?![^\[]*\])", m.group(2))]
    if len(args) < 4:
        continue
    def num(x):
        try:
            return int(x)
        except ValueError:
            return None
    scale[mid] = {
        "screen": [num(args[0]), num(args[1])],
        "machineExpr": [args[2], args[3]],
        "machine": [num(args[2]), num(args[3])],
    }

panel = {}
for m in re.finditer(r"MOT\[(\w+)\]\.SetPanel\(([^;]*)\);", s):
    mid = m.group(1)
    a = m.group(2)
    el = re.search(r"fMain->(\w+)", a)
    vert = a.rstrip().endswith("true")
    panel[mid] = {"target": el.group(1) if el else None, "vertical": vert}

motors = []
for mid in sorted(set(list(scale.keys()) + list(panel.keys()))):
    row = {"motorId": mid}
    row.update(scale.get(mid, {"screen": [None, None], "machineExpr": [None, None],
                               "machine": [None, None]}))
    row.update(panel.get(mid, {"target": None, "vertical": False}))
    motors.append(row)

leds = {}
for m in re.finditer(r"(\w+)\.SetMyLed\(\s*(\d+)\s*,\s*(\d+)\s*,\s*\(TMyLed \*\)fMain->(\w+)\)", s):
    leds.setdefault(m.group(1), []).append(
        {"row": int(m.group(2)), "col": int(m.group(3)), "led": m.group(4)})

doc = {
    "schemaVersion": "1.0.0",
    "generatedAt": datetime.now().astimezone().isoformat(timespec="seconds"),
    "source": {"toolchain": "BCB6", "cpp": "cinitial.cpp :: SetSimuScreenPara()",
               "base": SRC.replace("\\", "/")},
    "note": ("screen=[a,b] 為畫面像素範圍；machine 為機構座標範圍，"
             "machineExpr 保留 BCB6 原式（Prod.* 為 Recipe 值，HTML 端無此 JSON 時以 "
             "Motor-config 的 softLimitN/P 或預設區間替代）"),
    "summary": {"motors": len(motors), "ledGroups": len(leds)},
    "motors": motors,
    "ledGroups": leds,
}
json.dump(doc, open(OUT, "w", encoding="utf-8"), ensure_ascii=False, indent=1)
print("ok:", OUT, "motors=%d ledGroups=%d" % (len(motors), len(leds)))
