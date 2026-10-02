#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
從 HT9045 StateRecord 目錄抽出 Motion View HTML UI 模板所需的參數，
直接輸出可貼進 assets/motionview-template.html 參數區的 JavaScript 片段。

用法:
    python extract_params.py "<StateRecord 目錄>"            # 印出 JS 片段
    python extract_params.py "<StateRecord 目錄>" --json     # 印出 JSON（給其他工具用）
    python extract_params.py "<StateRecord 目錄>" --report   # 印出人可讀的核對表

所有 .ini / .Data / .csv 一律以 cp950 讀取（專案強制 Big5，勿改 UTF-8）。
HP*_*.xls 是 BIFF 二進位，需要 xlrd>=2.0。
"""
import os, sys, re, json, glob

def rd(path):
    """cp950 讀檔，找不到回傳 None"""
    try:
        with open(path, "rb") as f:
            return f.read().decode("cp950", "replace")
    except Exception:
        return None

def ini(text):
    """解析 ini/Data → {section: {key: value}}；key 保留原樣（含空白）"""
    out, cur = {}, ""
    if not text:
        return out
    for line in text.splitlines():
        s = line.strip()
        if s.startswith("[") and s.endswith("]"):
            cur = s[1:-1]
            out.setdefault(cur, {})
        elif "=" in s:
            k, v = s.split("=", 1)
            out.setdefault(cur, {})[k.strip()] = v.strip()
    return out


def find_device_dir(base):
    """IniData\\Data\\<DeviceName> 只會有一個（就是當時的 recipe）"""
    p = os.path.join(base, "HT9045", "IniData", "Data")
    if not os.path.isdir(p):
        return None, None
    for d in sorted(os.listdir(p)):
        full = os.path.join(p, d)
        if os.path.isdir(full) and os.path.exists(os.path.join(full, "Tray.Data")):
            return d, full
    return None, None


def read_hp_xls(base, name):
    """HP1_*/HP2_*.xls → 16x8 矩陣（0 = 空）。回傳 None 表示讀不到或全空。"""
    try:
        import xlrd
    except ImportError:
        return None
    for f in ("HP2_%s.xls" % name, "HP1_%s.xls" % name):
        path = os.path.join(base, f)
        if not os.path.exists(path):
            continue
        try:
            wb = xlrd.open_workbook(path, encoding_override="cp950")
        except Exception:
            continue
        sh = wb.sheet_by_index(0)
        m, any_val = [], False
        for r in range(1, sh.nrows):                      # 第 0 列是欄標題
            row = []
            for c in range(1, sh.ncols):                  # 第 0 欄是列標題
                v = sh.cell_value(r, c)
                try:
                    iv = int(v)
                except (TypeError, ValueError):
                    iv = 0
                if iv:
                    any_val = True
                row.append(iv)
            m.append(row)
        if any_val:
            return m
    return None


def collect(base):
    P = {"src": base, "warn": []}

    dev, devdir = find_device_dir(base)
    P["device"] = dev or "?"
    if not devdir:
        P["warn"].append("找不到 IniData\\Data\\<Device> 目錄，Tray/HotPlate 參數缺失")
        devdir = base

    # ---- Version / 機台 ----
    g = ini(rd(os.path.join(base, "HT9045", "system", "Gerneral.ini")))
    ver = g.get("Version", {})
    sysg = g.get("System", {})
    P["model"] = ver.get("Model", "?")
    P["machineId"] = ver.get("Machine ID", "?")
    P["subModel"] = ver.get("SubModel", "?")

    # ---- Tray form（[Type0] 是 Loader 用的那一組）----
    t = ini(rd(os.path.join(devdir, "Tray.Data")))
    t0 = t.get("Type0", {})
    P["tray"] = {
        "name": t0.get("Name", "?"),
        "xs": float(t0.get("X Start", 0) or 0),
        "ys": float(t0.get("Y Start", 0) or 0),
        "xp": float(t0.get("X Pitch", 0) or 0),
        "yp": float(t0.get("Y Pitch", 0) or 0),
        "cols": int(float(t0.get("X Division", 0) or 0)),
        "rows": int(float(t0.get("Y Division", 0) or 0)),
        "thick": float(t0.get("Think", 0) or 0),
    }

    # ---- HotPlate form ----
    h = ini(rd(os.path.join(devdir, "HotPlate.Data")))
    hf = h.get("Hotplate Form", {})
    P["hp"] = {
        "name": hf.get("Name", "?"),
        "xs": float(hf.get("X Start", 0) or 0),
        "ys": float(hf.get("Y Start", 0) or 0),
        "xp": float(hf.get("X Pitch", 0) or 0),
        "yp": float(hf.get("Y Pitch", 0) or 0),
        "cols": int(float(hf.get("X Division", 0) or 0)),
        "rows": int(float(hf.get("Y Division", 0) or 0)),
        "usingFlag": hf.get("Using Flag", "?"),
        "wide": hf.get("Use Wide Hotplate", "?"),
    }

    # ---- 吸嘴組態（MainFormSnapshot 是 runtime 實況，優先）----
    snap = rd(os.path.join(base, "MainFormSnapshot.txt")) or ""
    m = re.search(r"iPickRow=(\d+)\s+iPickCol=(\d+)", snap)
    P["arm"] = {"rows": int(m.group(1)) if m else 0,
                "cols": int(m.group(2)) if m else 0}
    P["pickerCount"] = sysg.get("USE_PICKER_COUNT", "?")     # 0=ep4 1=ep8 2=ep2 4=ep1

    # ---- 變距軸 ----
    P["xPitchMode"] = sysg.get("USE_IN_OUT_ARM_X_PITCH", "?")   # 0=iXPitch40mm
    P["yPitchMode"] = sysg.get("USE_IN_OUT_ARM_Y_PITCH", "?")   # 0 = 未安裝
    P["xLimit"] = {"minX3": int(sysg.get("IN_OUT_ARM_X_PITCH_MIN", 4000)),
                   "maxX3": int(sysg.get("IN_OUT_ARM_X_PITCH_MAX", 12000))}
    P["yLimit"] = {"min": int(sysg.get("IN_OUT_ARM_Y_PITCH_MIN", 1500)),
                   "max": int(sysg.get("IN_OUT_ARM_Y_PITCH_MAX", 7500))}
    # 單一間距上下限（cmydef.cpp：iXPitch40mm 模式固定 1333 / 4000）
    P["xLimit"]["min1"] = P["xLimit"]["minX3"] // 3
    P["xLimit"]["max1"] = P["xLimit"]["maxX3"] // 3

    # ---- Pitch 模式 Open/Close vs Fixed ----
    a = ini(rd(os.path.join(devdir, "ArmCondition.Data")))
    P["pmodeIn"] = a.get("Input Arm", {}).get("One by one", "?")    # 0=Open/Close 1=Fixed
    P["pmodeOut"] = a.get("Output Arm", {}).get("One by one", "?")

    # ---- teach 教點 ----
    tc = ini(rd(os.path.join(base, "HT9045", "system", "teach.ini")))
    def T(sec, key, dflt=0):
        return int(float(tc.get(sec, {}).get(key, dflt) or dflt))
    P["teach"] = {
        "inX_loader": T("MInArmX", "setEditInLoader"),
        "inX_hp1":    T("MInArmX", "setEditInHotPlate1"),
        "inX_sht1":   T("MInArmX", "setEditInShuttle1"),
        "inY_loader": T("MInArmY", "setEditInLoader"),
        "inY_hp1":    T("MInArmY", "setEditInHotPlate1"),
        "inY_sht1":   T("MInArmY", "setEditInShuttle1"),
        "pitch40":    T("MInArmPitch", "setEditInXPitch40"),
        "pitch120":   T("MInArmPitch", "setEditInXPitch120"),
        "outPitch40": T("MOutArmPitch", "setEditOutXPitch40"),
        "outPitch120":T("MOutArmPitch", "setEditOutXPitch120"),
        "yPitch15":   T("MInArmPitchY", "setEditOutY15"),
        "yPitch60":   T("MInArmPitchY", "setEditOutY60"),
        "sht1Left":   T("MInShutte1", "setEditInLeft"),
        "sht1Right":  T("MInShutte1", "setEditInRight"),
    }
    # teach.ini 的鍵名各版本略有差異：抓不到就把整個 section 附上讓人工核對
    for sec in ("MInArmX", "MInArmY", "MInArmPitch", "MInArmPitchY", "MInShutte1"):
        if sec in tc:
            P.setdefault("teachRaw", {})[sec] = tc[sec]

    # ---- TestMode / DutOnOff → 幾 site ----
    tm = ini(rd(os.path.join(devdir, "TestMode.Data")))
    dut = tm.get("DutOnOff", {})
    on = [k for k, v in dut.items() if v == "1" and not k.rstrip().endswith("2")]
    P["siteOn"] = sorted(k.replace("Dut", "").strip() for k in on)
    P["siteCount"] = len(on)
    P["tempMode"] = tm.get("TestMode", {}).get("Temperature Mode", "?")

    # ---- HotPlate 實機現況 ----
    site = read_hp_xls(base, "Site")
    sht = read_hp_xls(base, "WhichShuttle")
    if site:
        P["hpSite"] = site
        P["hpSht"] = sht or [[0] * len(site[0]) for _ in site]
        P["hpRealIC"] = sum(1 for r in site for v in r if v)
    else:
        P["warn"].append("HP*_Site.xls 讀不到（需要 xlrd>=2.0）或全空")

    # ---- DecisionVariables 交叉核對 ----
    dv = rd(os.path.join(base, "DecisionVariables.csv")) or ""
    m = re.search(r"HP RealIC\(HowManyIC\) P0/P1,\s*(\d+)\s*/\s*(\d+)", dv)
    if m:
        P["dvRealIC"] = [int(m.group(1)), int(m.group(2))]
        if "hpRealIC" in P and P["hpRealIC"] not in P["dvRealIC"]:
            P["warn"].append("HP 逐格表顆數 %d 與 DecisionVariables %s 不符，請確認 P0/P1 對應"
                             % (P["hpRealIC"], P["dvRealIC"]))
    m = re.search(r"CUSTOMER_CODE,\s*(\d+)", dv)
    if m:
        P["customerCode"] = int(m.group(1))

    # ---- PickHPRec.json（實機取料群組，可當重播資料）----
    pj = os.path.join(base, "HT9045", "system", "PickHPRec.json")
    if os.path.exists(pj):
        try:
            d = json.load(open(pj, encoding="utf-8"))
            teams = []
            for k in sorted(d, key=lambda x: int(x.split("_")[1])):
                for t2 in d[k].get("HPSuckTeamList", []):
                    teams.append([t2["P"], t2["Col"], t2["Row"], t2["Sht"], t2["Kit"],
                                  t2["PlateRow"], t2["PlateCol"], t2["Site"], t2["Suck"]])
            P["pickRec"] = teams
        except Exception as e:
            P["warn"].append("PickHPRec.json 解析失敗: %s" % e)
    return P


# ---------- 推導（規則見 references/parameter-contract.md）----------
def derive(P):
    D = {}
    xl = P["xLimit"]
    n = max(1, P["arm"]["cols"] - 1)        # 幾個間距
    # X：由 k=1 往上找第一個落在 [min1, max1] 的倍數（同 ainarm9045.cpp）
    def steps(pitch_mm, lo, hi):
        out = []
        for k in range(1, 17):
            v = round(pitch_mm * 100 * k)
            if lo <= v <= hi:
                out.append(k)
        return out
    D["trayXK"] = steps(P["tray"]["xp"], xl["min1"], xl["max1"])
    D["trayYM"] = (steps(P["tray"]["yp"], P["yLimit"]["min"], P["yLimit"]["max"])
                   if str(P["yPitchMode"]) != "0" else [])
    # Site 節距：由 HotPlate 逐格表反推（同一 site 佔幾欄 / A-B 排差幾列）
    if "hpSite" in P:
        m = P["hpSite"]
        row0 = next((r for r in m if all(v for v in r)), None)
        if row0:
            reps = 1
            while reps < len(row0) and row0[reps] == row0[0]:
                reps += 1
            D["hpColStep"] = reps                     # 同一 site 連續佔幾欄 → 欄步進
        # 列步進(iYHalf)：統計「同一欄裡 A 排列 + d = B 排列」出現最多次的 d
        # （不可只取「第一個 A 列」與「第一個 B 列」之差 —— 盤上會有混排的列，會失準）
        half = P["arm"]["cols"]
        nr, nc = len(m), len(m[0])
        best_d, best_n = None, 0
        for d in range(1, nr):
            n = 0
            for c in range(nc):
                for r in range(nr - d):
                    va, vb = m[r][c], m[r + d][c]
                    if va and vb and va <= half < vb:
                        n += 1
            if n > best_n:
                best_d, best_n = d, n
        if best_d:
            D["hpRowStep"] = best_d                   # ＝ iYHalf
            D["hpRowStepHits"] = best_n
    D.setdefault("hpColStep", 2)
    D.setdefault("hpRowStep", 3)
    D["siteXP"] = P["hp"]["xp"] * D["hpColStep"]
    D["siteYP"] = P["hp"]["yp"] * D["hpRowStep"]
    D["armYGap"] = D["siteYP"]                        # 無 Y 變距時＝固定 Y 間距
    D["trayYRatio"] = (D["armYGap"] / P["tray"]["yp"]) if P["tray"]["yp"] else 0
    D["trayYIsInteger"] = abs(D["trayYRatio"] - round(D["trayYRatio"])) < 1e-6
    return D


def emit_js(P, D):
    t, h, a = P["tray"], P["hp"], P["arm"]
    L = []
    ad = L.append
    ad("/* ####  參數區：由 extract_params.py 從 StateRecord 抽出  ####")
    ad("   來源: %s" % P["src"])
    ad("   機台: %s / %s  SubModel %s   Device: %s" % (P["model"], P["machineId"], P["subModel"], P["device"]))
    ad("   #### */")
    ad('var MACHINE={model:"%s", id:"%s", device:"%s", cc:%s, tempMode:%s};'
       % (P["model"], P["machineId"], P["device"], P.get("customerCode", 0), P.get("tempMode", 0)))
    ad("var TRAY={name:%s, xp:%.3f, yp:%.3f, cols:%d, rows:%d};"
       % (json.dumps(t["name"], ensure_ascii=False), t["xp"], t["yp"], t["cols"], t["rows"]))
    ad("var HPF ={name:%s, xp:%.3f, yp:%.3f, cols:%d, rows:%d};"
       % (json.dumps(h["name"], ensure_ascii=False), h["xp"], h["yp"], h["cols"], h["rows"]))
    ad("var ARM ={rows:%d, cols:%d, yGapMM:%.2f};   /* yGap 由 Site Y 節距反推 */"
       % (a["rows"], a["cols"], D["armYGap"]))
    ad("var PIT ={min1:%d, max1:%d, minX3:%d, maxX3:%d, t40:%d, t120:%d};"
       % (P["xLimit"]["min1"], P["xLimit"]["max1"], P["xLimit"]["minX3"], P["xLimit"]["maxX3"],
          P["teach"]["pitch40"], P["teach"]["pitch120"]))
    ad("var YP  ={min:%d, max:%d, installed:%s};"
       % (P["yLimit"]["min"], P["yLimit"]["max"], "false" if str(P["yPitchMode"]) == "0" else "true"))
    ad("var SITE={xp:%.2f, yp:%.2f};        /* HP 欄步進 %d、列步進 %d(=iYHalf) 反推 */"
       % (D["siteXP"], D["siteYP"], D["hpColStep"], D["hpRowStep"]))
    ad("var PMODE={inArm:%s, outArm:%s};    /* 0=Open/Close 1=Fixed（One by one）；in 是保留字勿用 */"
       % (P["pmodeIn"], P["pmodeOut"]))
    if "hpSite" in P:
        ad("/* HotPlate 實機現況（%d 顆）：可當「實機重播」情境的起始狀態 */" % P["hpRealIC"])
        ad("var HP_SITE=[")
        for r in P["hpSite"]:
            ad("  [%s]," % ",".join(str(v) for v in r))
        ad("];")
        ad("var HP_SHT=[")
        for r in P["hpSht"]:
            ad("  [%s]," % ",".join(str(v) for v in r))
        ad("];")
    if "pickRec" in P:
        ad("/* PickHPRec.json 的 %d 個 team（實機取料群組記錄）*/" % len(P["pickRec"]))
        ad("var PICKREC=%s;" % json.dumps(P["pickRec"], separators=(",", ":")))
    return "\n".join(L)


def emit_report(P, D):
    t, h, a = P["tray"], P["hp"], P["arm"]
    L = []
    ad = L.append
    ad("=" * 72)
    ad("機台        %s / %s  SubModel %s" % (P["model"], P["machineId"], P["subModel"]))
    ad("Device      %s   CC=%s   Temperature Mode=%s"
       % (P["device"], P.get("customerCode", "?"), P.get("tempMode", "?")))
    ad("Site        %d site  %s" % (P["siteCount"], ",".join(P["siteOn"])[:60]))
    ad("-" * 72)
    ad("Tray form   %s  pitch %.2f/%.2f  %dx%d = %d 格"
       % (t["name"], t["xp"], t["yp"], t["cols"], t["rows"], t["cols"] * t["rows"]))
    ad("HotPlate    %s  pitch %.2f/%.2f  %dx%d = %d 格"
       % (h["name"], h["xp"], h["yp"], h["cols"], h["rows"], h["cols"] * h["rows"]))
    ad("吸嘴        %d 排 x %d 欄 = %d 支   USE_PICKER_COUNT=%s"
       % (a["rows"], a["cols"], a["rows"] * a["cols"], P["pickerCount"]))
    ad("-" * 72)
    ad("X 變距      模式=%s  單一間距 %.2f~%.2f mm  跨距 %.0f~%.0f mm"
       % (P["xPitchMode"], P["xLimit"]["min1"] / 100.0, P["xLimit"]["max1"] / 100.0,
          P["xLimit"]["minX3"] / 100.0, P["xLimit"]["maxX3"] / 100.0))
    ad("            教點 Pitch40=%d  Pitch120=%d" % (P["teach"]["pitch40"], P["teach"]["pitch120"]))
    ad("Y 變距      模式=%s（0=未安裝）  行程 %.2f~%.2f mm  教點 Y15=%d Y60=%d"
       % (P["yPitchMode"], P["yLimit"]["min"] / 100.0, P["yLimit"]["max"] / 100.0,
          P["teach"]["yPitch15"], P["teach"]["yPitch60"]))
    ad("Pitch 模式  InArm=%s  OutArm=%s   （0=Open/Close 可跳格, 1=Fixed）"
       % (P["pmodeIn"], P["pmodeOut"]))
    ad("-" * 72)
    ad("推導 Tray X 合法倍數 k = %s  → 吸嘴間距 %s mm"
       % (D["trayXK"], [round(k * t["xp"], 2) for k in D["trayXK"]]))
    ad("推導 Tray Y 合法倍數 m = %s%s"
       % (D["trayYM"], "" if D["trayYM"] else "（無 Y 變距軸）"))
    ad("推導 Site 節距 = %.2f / %.2f mm （HP 欄步進 %d、列步進 %d = iYHalf）"
       % (D["siteXP"], D["siteYP"], D["hpColStep"], D["hpRowStep"]))
    ad("推導 吸嘴 Y 間距 %.2f mm ÷ Tray Y pitch %.2f = %.3f → %s"
       % (D["armYGap"], t["yp"], D["trayYRatio"],
          "整數，兩排可同時取" if D["trayYIsInteger"] else "非整數，Tray 端一次只能下一排"))
    if "hpRealIC" in P:
        ad("HotPlate 現況 %d 顆（DecisionVariables: %s）" % (P["hpRealIC"], P.get("dvRealIC", "?")))
    if "pickRec" in P:
        ad("PickHPRec  %d 個 team" % len(P["pickRec"]))
    if P["warn"]:
        ad("-" * 72)
        for w in P["warn"]:
            ad("[警告] %s" % w)
    ad("=" * 72)
    return "\n".join(L)


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 1
    base = sys.argv[1].rstrip("\\/")
    if not os.path.isdir(base):
        print("找不到目錄: %s" % base)
        return 1
    try:
        sys.stdout.reconfigure(encoding="utf-8")
    except Exception:
        pass
    P = collect(base)
    D = derive(P)
    if "--json" in sys.argv:
        print(json.dumps({"params": P, "derived": D}, ensure_ascii=False, indent=1))
    elif "--report" in sys.argv:
        print(emit_report(P, D))
    else:
        print(emit_report(P, D))
        print()
        print(emit_js(P, D))
    return 0


if __name__ == "__main__":
    sys.exit(main())
