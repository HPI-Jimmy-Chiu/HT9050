# INI → JSON 轉換（BCB6 設定檔 → HTML 唯一資料來源）
#   D:\HT9045\system\Gerneral.ini → JSON\General-config.json
#   D:\HT9045\config\config.ini    → JSON\Config.json（含 Config.Configuration.html 元件對照 uiMap）
#   uiMap 來源：BCB6 cConfiguration.cpp  elConfig->Add(元件, &IniConfig.欄位, EC型態, "Section", "Key", Visible, Enable, ReadFromFile, Default, ...)
#   跑完要再跑 _gen_json_shim.py
import json, os, re
from datetime import datetime

BASE = r"D:\HT9045\HT9011UC_Code_V3.33.910.0_20260716_NN Mode 2D + AutoClean V2"
OUT = r"D:\HT9045\JSON"
NOW = datetime.now().astimezone().isoformat(timespec="seconds")


def typed(v):
    s = v.strip()
    if re.fullmatch(r"[-+]?\d+", s):
        return int(s), "int"
    if re.fullmatch(r"[-+]?\d*\.\d+(e[-+]?\d+)?", s, re.I):
        return float(s), "float"
    if s.lower() in ("true", "false"):
        return s.lower() == "true", "bool"
    return s, "string"


def parse_ini(path):
    sections = {}          # 保序：dict 在 3.7+ 有序
    order = []
    cur = None
    raw = open(path, encoding="cp950", errors="replace").read().splitlines()
    for ln in raw:
        t = ln.strip()
        if not t or t.startswith(";") or t.startswith("#"):
            continue
        m = re.match(r"^\[(.+?)\]\s*$", t)
        if m:
            cur = m.group(1)
            if cur not in sections:
                sections[cur] = {}
                order.append(cur)
            continue
        if "=" in t and cur is not None:
            k, v = t.split("=", 1)
            k = k.strip()
            val, ty = typed(v)
            sections[cur][k] = {"value": val, "type": ty, "raw": v.strip()}
    return sections, order


def ini_doc(path, reader):
    sections, order = parse_ini(path)
    nkeys = sum(len(s) for s in sections.values())
    return {
        "schemaVersion": "1.0.0", "generatedAt": NOW,
        "source": {"toolchain": "BCB6", "file": path, "encoding": "cp950", "reader": reader},
        "note": "HTML 端只讀本 JSON，不得直接解析 .ini；value 為推定型態，raw 為原字串。",
        "summary": {"sections": len(sections), "keys": nkeys},
        "sectionOrder": order,
        "sections": sections,
    }


# ---------- 1. Gerneral.ini ----------
gen = ini_doc(r"D:\HT9045\system\Gerneral.ini", "cinitial.cpp / cmydef.cpp ReadGeneralIni（機台選配、CUSTOMER_CODE、Model、驅動卡、ATC、SECS_GEM…）")
# 給 background 啟動判斷用的關鍵欄位（依 [System]/[VENDER]/[Version] 常見鍵）
sys_sec = gen["sections"].get("System", {})
def pick(sec, *names):
    for n in names:
        if n in sec:
            return sec[n]["value"]
    return None
gen["quick"] = {
    "customerCode": pick(sys_sec, "CUSTOMER_CODE"),
    "model": pick(gen["sections"].get("Version", {}), "Model"),
    "subModel": pick(gen["sections"].get("Version", {}), "SubModel"),
    "serialNo": pick(gen["sections"].get("Version", {}), "Serial No"),
    "machineId": pick(gen["sections"].get("Version", {}), "Machine ID"),
    "factory": pick(gen["sections"].get("Version", {}), "Factory"),
    "version": pick(gen["sections"].get("Version", {}), "Ver"),
    "useATC": pick(gen["sections"].get("ATC", {}), "USE_ATC_MODE"),
    "secsGemSystem": pick(gen["sections"].get("SECS_GEM", {}), "SECS_GEM_SYSTEM"),
    "motionCardType": pick(sys_sec, "MOTION_CARD_TYPE"),
    "indexMotionCard": pick(sys_sec, "INDEX_MOTION_CARD"),
    "ioCardType": pick(sys_sec, "IO_CARD_TYPE"),
    "indexDriverType": pick(gen["sections"].get("IndexDriver", {}), "INDEX_DRIVER_TYPE"),
    "trayArmMode": pick(sys_sec, "TRAY_ARM_MODE"),
    "useTrayMapping": pick(sys_sec, "USE_TRAY_MAPPING"),
    "useSocketSensor": pick(sys_sec, "USE_SOCKET_SENSOR"),
    "useAutoRetest": pick(sys_sec, "USE_AUTO_RETEST"),
    "realTimeCCD": pick(sys_sec, "REAL_TIME_CCD"),
    "installOCR": pick(sys_sec, "INSTALL_OCR"),
    "useInOutArmYPitch": pick(sys_sec, "USE_IN_OUT_ARM_Y_PITCH"),
    "use2x4": pick(sys_sec, "bHT9045S_USE2x4"),
    "useOutShtMot": pick(sys_sec, "USE_OUT_SHT_MOT"),
    "fix3Install": pick(sys_sec, "FIX3_INSTALL"),
    "useCatchTrayModel": pick(sys_sec, "USE_CATCH_TRAY_MODEL"),
}

# HandlerSys.dfm（THandlerSystem，機台系統設定）元件 ↔ Gerneral.ini：
#   BCB6 HandlerSys.cpp SaveSystemSet()  WriteIniDataGeneral("Section", "Key", <元件>->ItemIndex|Checked|Text…)
hs_cpp = open(os.path.join(BASE, "HandlerSys.cpp"), encoding="cp950", errors="replace").read()
RE_W = re.compile(r'WriteIniDataGeneral\(\s*"([^"]+)"\s*,\s*"([^"]+)"\s*,\s*([^;]+?)\)\s*;')
RE_COMP = re.compile(r"\b(\w+)->(ItemIndex|Checked|Text|Value|Position|Down)\b")
hs_ui = {}
hs_by_key = {}
for sec, key, expr in RE_W.findall(hs_cpp):
    cm = RE_COMP.search(expr)
    kk = sec + "/" + key
    ent = hs_by_key.setdefault(kk, {"section": sec, "key": key, "components": [], "exprs": []})
    ent["exprs"].append(expr.strip())
    if not cm:
        continue
    comp, prop = cm.groups()
    if comp not in ent["components"]:
        ent["components"].append(comp)
    it = hs_ui.setdefault(comp, {"component": comp, "prop": prop, "bindings": []})
    if not any(b["section"] == sec and b["key"] == key for b in it["bindings"]):
        it["bindings"].append({"section": sec, "key": key, "expr": expr.strip()})
# 補：LoaderSystemSet() 直接由 ini 讀進元件者（如 edtCustomerCode->Text=CheckAndReadIniDataGeneral("System","CUSTOMER_CODE",0)；Save 端寫的是全域變數）
RE_R = re.compile(r'\b(\w+)->(ItemIndex|Checked|Text)\s*=\s*(?:\([^)]*\))?\s*CheckAndReadIniDataGeneral\(\s*"([^"]+)"\s*,\s*"([^"]+)"')
for comp, prop, sec, key in RE_R.findall(hs_cpp):
    kk = sec + "/" + key
    ent = hs_by_key.setdefault(kk, {"section": sec, "key": key, "components": [], "exprs": []})
    if comp not in ent["components"]:
        ent["components"].append(comp)
    it = hs_ui.setdefault(comp, {"component": comp, "prop": prop, "bindings": []})
    if not any(b["section"] == sec and b["key"] == key for b in it["bindings"]):
        it["bindings"].append({"section": sec, "key": key, "expr": "CheckAndReadIniDataGeneral (LoaderSystemSet)"})
hs_html = open(r"D:\HT9045\page\HW.HandlerSys.html", encoding="utf-8").read() if os.path.exists(r"D:\HT9045\page\HW.HandlerSys.html") else ""
hs_ids = set(re.findall(r'id="([^"]+)"', hs_html))
for comp, it in hs_ui.items():
    it["inHtml"] = comp in hs_ids
    b0 = it["bindings"][0]
    sec = gen["sections"].get(b0["section"], {})
    it["inIni"] = b0["key"] in sec
    it["iniValue"] = sec[b0["key"]]["value"] if it["inIni"] else None
gen["uiMap"] = {
    "note": "HW.HandlerSys.html 元件 → Gerneral.ini Section/Key（prop=ItemIndex→radio/select 索引、Checked→checkbox、Text→edit）。"
            "多鍵綁同一元件（如 MachineTrack→AUTO_EMPTY_COLOR+SUPPORT_2_EMPTY_EMPTY）以 bindings[0] 為顯示來源。",
    "source": "BCB6 HandlerSys.cpp SaveSystemSet()/LoaderSystemSet()",
    "byComponent": hs_ui,
    "byKey": hs_by_key,
    "summary": {"components": len(hs_ui), "keys": len(hs_by_key),
                "inIni": sum(1 for v in hs_ui.values() if v["inIni"]),
                "inHtml": sum(1 for v in hs_ui.values() if v["inHtml"]),
                "missingInHtml": sorted(c for c, v in hs_ui.items() if not v["inHtml"])},
}
p = os.path.join(OUT, "General-config.json")
json.dump(gen, open(p, "w", encoding="utf-8"), ensure_ascii=False, indent=1)
print("ok:", p, gen["summary"])

# ---------- 2. config.ini + cConfiguration uiMap ----------
cfg = ini_doc(r"D:\HT9045\config\config.ini", "cConfiguration.cpp InitConfigEdtList_Item*（HTEditList elConfig->Add）/ ReadConfigStandard")

cpp = open(os.path.join(BASE, "cConfiguration.cpp"), encoding="cp950", errors="replace").read()
# elConfig->Add(cbA01, &IniConfig.bA01AutoSwitchToOperatorMode, ECBool, "Function", "bAutoSwitchToOperatorMode", bShow, bDisable, bFixedValue, 1);
RE_ADD = re.compile(
    r"elConfig->Add\(\s*(\w+)\s*,\s*&?([\w\.\[\]]+)\s*,\s*(EC\w+)\s*,\s*\"([^\"]*)\"\s*,\s*\"([^\"]*)\"\s*,"
    r"\s*(\w+)\s*,\s*(\w+)\s*,\s*(\w+)\s*(?:,\s*([^,\)]+))?", re.S)
ui = {}
for m in RE_ADD.finditer(cpp):
    comp, field, ectype, sec, key, vis, en, rff, dflt = m.groups()
    sec = sec or "fConfiguration"
    key = key or comp
    item = ui.setdefault(comp, {"component": comp, "field": field, "type": ectype,
                                "section": sec, "key": key, "variants": []})
    item["variants"].append({"visible": vis in ("bShow", "true"), "enabled": en in ("bEnable", "true"),
                             "readFromFile": rff in ("bReadFromFile", "true"),
                             "default": (dflt or "").strip()})
# 反查：Section/Key → 元件；並標記 config.ini 內是否存在該鍵
by_key = {}
for comp, it in ui.items():
    by_key.setdefault(it["section"] + "/" + it["key"], []).append(comp)
    sec = cfg["sections"].get(it["section"], {})
    it["inIni"] = it["key"] in sec
    it["iniValue"] = sec[it["key"]]["value"] if it["inIni"] else None

# Config.Configuration.html 內是否有此元件 id
html = open(r"D:\HT9045\page\Config.Configuration.html", encoding="utf-8").read()
ids = set(re.findall(r'id="([^"]+)"', html))
for comp, it in ui.items():
    it["inHtml"] = comp in ids

cfg["uiMap"] = {
    "note": "component → IniConfig 欄位 / EC 型態 / ini Section+Key；variants 為 cpp 內依 CUSTOMER_CODE 分支的多組 Add（HTML 取第一組）",
    "source": "BCB6 cConfiguration.cpp InitConfigEdtList_Item*",
    "byComponent": ui,
    "byKey": by_key,
    "summary": {
        "components": len(ui),
        "inIni": sum(1 for v in ui.values() if v["inIni"]),
        "inHtml": sum(1 for v in ui.values() if v["inHtml"]),
        "missingInHtml": sorted(c for c, v in ui.items() if not v["inHtml"]),
    },
}
p = os.path.join(OUT, "Config.json")
json.dump(cfg, open(p, "w", encoding="utf-8"), ensure_ascii=False, indent=1)
print("ok:", p, cfg["summary"], cfg["uiMap"]["summary"]["components"],
      "inIni", cfg["uiMap"]["summary"]["inIni"], "inHtml", cfg["uiMap"]["summary"]["inHtml"],
      "missingInHtml", len(cfg["uiMap"]["summary"]["missingInHtml"]))
