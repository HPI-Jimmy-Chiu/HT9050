# 補充轉換：ht9045-html-json 表③「尚未轉換 JSON 的讀寫檔案」剩餘項目 → JSON
# 排除範圍（依使用者指示 2026-09-09）：
#   - Handler.db3（MDB Updater 專案管轄，忽略）
#   - Language.csv（併入 page/i18n.js，見 _merge_language_csv_i18n.py）
#   - ReleaseNote.txt（直接內嵌至 Data.Observer.html，見 _embed_release_note.py）
#   - D:\HT9045\IniData\Data\<Recipe>\...（Setup-index.json / Setup-current.json 已涵蓋，不重複）
# 跑完要再跑 _gen_json_shim.py
import json, os, re, csv
from datetime import datetime

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
    sections, order = {}, []
    cur = None
    for ln in open(path, encoding="cp950", errors="replace").read().splitlines():
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
            val, ty = typed(v)
            sections[cur][k.strip()] = {"value": val, "type": ty, "raw": v.strip()}
    return sections, order


def base_doc(file, reader, note):
    return {"schemaVersion": "1.0.0", "generatedAt": NOW,
            "source": {"toolchain": "BCB6", "file": file, "encoding": "cp950", "reader": reader},
            "note": note}


def write(name, doc):
    path = os.path.join(OUT, name)
    with open(path, "w", encoding="utf-8") as f:
        json.dump(doc, f, ensure_ascii=False, indent=2)
    print("wrote", name, len(json.dumps(doc)), "bytes")


def ini_job(outName, file, reader, note="HTML 端只讀本 JSON，不得直接解析 .ini；value 為推定型態，raw 為原字串。"):
    doc = base_doc(file, reader, note)
    if os.path.isfile(file):
        sections, order = parse_ini(file)
        doc["state"] = "ready"
        doc["summary"] = {"sections": len(sections), "keys": sum(len(s) for s in sections.values())}
        doc["sectionOrder"] = order
        doc["sections"] = sections
    else:
        doc["state"] = "missing"
        doc["summary"] = {"sections": 0, "keys": 0}
        doc["sectionOrder"] = []
        doc["sections"] = {}
        doc["note"] += " ⚠ 此開發環境未部署該檔，結構待實機驗證後補齊（schema-only）。"
    write(outName, doc)
    return doc


def csv_job(outName, file, reader):
    doc = base_doc(file, reader, "CSV 逐列轉 JSON 陣列，欄名取自標題列；HTML 端只讀本 JSON。")
    if os.path.isfile(file):
        with open(file, encoding="cp950", errors="replace", newline="") as f:
            rows = list(csv.reader(f))
        header = [h.strip() for h in rows[0] if h.strip() != ""] if rows else []
        items = []
        for r in rows[1:]:
            if not any(c.strip() for c in r):
                continue
            items.append({header[i]: r[i] for i in range(min(len(header), len(r)))})
        doc["state"] = "ready"
        doc["header"] = header
        doc["summary"] = {"rows": len(items)}
        doc["items"] = items
    else:
        doc["state"] = "missing"
        doc["header"] = []
        doc["summary"] = {"rows": 0}
        doc["items"] = []
    write(outName, doc)
    return doc


def dir_index_job(outName, folder, reader, note, pattern=None):
    doc = base_doc(folder, reader, note)
    if os.path.isdir(folder):
        names = sorted(os.listdir(folder))
        if pattern:
            names = [n for n in names if re.search(pattern, n, re.I)]
        doc["state"] = "ready"
        doc["summary"] = {"count": len(names)}
        doc["items"] = names
    else:
        doc["state"] = "missing"
        doc["summary"] = {"count": 0}
        doc["items"] = []
    write(outName, doc)
    return doc


# ---- 1. 硬體設定檔類 ------------------------------------------------------
ini_job("ARMS-config.json", r"D:\HT9045\system\ARMS.ini", "ARMS\\ARMS.cpp（_FILE_PTAH／asARSMParaPath）")
ini_job("ContactInfo-config.json", r"D:\HT9045\system\ContactInfo.ini", "cContact.cpp／ContactForce.cpp")
ini_job("ColorSensorType-config.json", r"D:\HT9045\system\ColorSensorType.ini", "cTrayForm.cpp")
ini_job("MVData-config.json", r"D:\HT9045\system\MVData.ini", "Monitor\\MonitorInterface.cpp")
ini_job("AutoTemperature-config.json", r"D:\HT9045\System\AutoTemperature.ini", "AutoTemperature.cpp（TMyATPanel 動態面板資料來源）")
ini_job("Barcode-config.json", r"D:\HT9045\system\Barcode.ini", "BarCode\\BarCode.cpp（2D Barcode 長度／機種設定，非 Recipe 內 Bar Code 選項）")
ini_job("PadInterfacePara-config.json", r"D:\HT9045\System\PadInterfacePara.ini", "uPadInterface.cpp")
ini_job("EventLogLevel-config.json", r"D:\HT9045\System\EvenLogLevel.ini", "Password.cpp")
ini_job("TrayStepSpeed-config.json", r"D:\HT9045\system\TrayStepSpeed.ini", "common.cpp asTrayStepSpeedByMachinePatch")
ini_job("SpecialErrNote-config.json", r"D:\HT9045\system\SpecialErrNote.ini", "note.cpp")
csv_job("TrayForm-config.json", r"D:\HT9045\System\TrayForm.csv", "cinitial.cpp／cTrayForm.cpp（TrayTablePath）")
csv_job("PlateForm-config.json", r"D:\HT9045\System\PlateForm.csv", "cinitial.cpp／cHotPlate.cpp（PlateTablePath）")

# ATC.ini 有兩份不同用途同名檔（system＝通道位址、config＝iCheckSameTempTime）合併一份 JSON
atc_doc = base_doc([r"D:\HT9045\system\ATC.ini", r"D:\HT9045\config\ATC.ini"],
                   "ATCInterface.cpp／main.cpp（system\\ATC.ini：通道位址／Port／Offset）；uTemp_Set.cpp（config\\ATC.ini：iCheckSameTempTime）",
                   "同名但不同資料夾、不同用途的兩份 ATC.ini，分開存放於 systemFile/configFile。")
for key, p in (("systemFile", r"D:\HT9045\system\ATC.ini"), ("configFile", r"D:\HT9045\config\ATC.ini")):
    if os.path.isfile(p):
        sections, order = parse_ini(p)
        atc_doc[key] = {"state": "ready", "path": p, "sectionOrder": order, "sections": sections}
    else:
        atc_doc[key] = {"state": "missing", "path": p, "sectionOrder": [], "sections": {}}
write("ATC-config.json", atc_doc)

dio_dir = r"D:\HT9045\IniData\DioCfg"
dio_doc = base_doc(dio_dir, "DIOInterFaceCFG.cpp／cTesterIF.cpp（FindFirstFile *.ini 動態列舉）",
                   "各 DIO 型態設定檔＋DIO.CFG 型態順序清單；HTML 端不得直接解析資料夾。")
if os.path.isdir(dio_dir):
    files = {}
    order = []
    if os.path.isfile(os.path.join(dio_dir, "DIO.CFG")):
        order = [l.strip() for l in open(os.path.join(dio_dir, "DIO.CFG"), encoding="cp950", errors="replace").read().splitlines() if l.strip()]
    for n in sorted(os.listdir(dio_dir)):
        if n.lower().endswith(".ini"):
            sections, sord = parse_ini(os.path.join(dio_dir, n))
            files[n] = {"sectionOrder": sord, "sections": sections}
    dio_doc["state"] = "ready"
    dio_doc["typeOrder"] = order
    dio_doc["summary"] = {"types": len(files)}
    dio_doc["files"] = files
else:
    dio_doc["state"] = "missing"
    dio_doc["typeOrder"] = []
    dio_doc["summary"] = {"types": 0}
    dio_doc["files"] = {}
write("DioCfg-index.json", dio_doc)

# ---- 2. Config 檔類 --------------------------------------------------------
ini_job("Security-def.json", r"D:\HT9045\config\Security_new.def", "cAuthority.cpp／uLotInfo.cpp（Enable/Disable 定義；非 levelset.dat 的權限等級）")
ini_job("CriticalParaControl-config.json", r"D:\HT9045\config\CriticalParaControl.ini", "cAuthority.cpp")
ini_job("ESDconfig-config.json", r"D:\HT9045\config\ESDconfig.ini", "ProductionInfo\\uESDControl.cpp")

# ---- 3. Setup 檔(Recipe) 類 ------------------------------------------------
ini_job("RPDefault-config.json", r"D:\HT9045\IniData\RPDefault.ini", "RPDefault.cpp／cSpeed.cpp／uCleaning.cpp／uYieldMonitoring.cpp")
ini_job("SocketCount-config.json", r"D:\HT9045\IniData\SocketCount.ini", "cStartCondition.cpp")
offset_dir = r"D:\HT9045\IniData\Offset"
offset_note = ("Arm Offset 備份資料夾清單（每個子資料夾＝一個 Recipe 名稱，內含 Position Offset.Data）；"
               + ("目前有 %d 個 Recipe 備份。" % len(os.listdir(offset_dir)) if os.path.isdir(offset_dir) else "此開發環境資料夾不存在。"))
dir_index_job("Offset-index.json", offset_dir, "cOffSet.cpp（common.cpp OffsetPath）", offset_note)
dir_index_job("SaveByMachine-index.json", r"D:\HT9045\IniData\SaveByMachine", "common.cpp sSaveByMachine", "依機台另存 Recipe 備份資料夾清單；目前為空資料夾（尚無備份）。")

dc_root = r"D:\HT9045\IniData\Data"
dc_doc = base_doc(r"D:\HT9045\IniData\Data\<Recipe>\DeviceCorrespond.ini", "uLotInfo.cpp",
                   "各 Recipe 內對應裝置設定；非 Setup-current.json 已涵蓋的 11 個核心 .Data 檔之一，需依目前工作檔另讀。")
found = {}
if os.path.isdir(dc_root):
    for name in os.listdir(dc_root):
        p = os.path.join(dc_root, name, "DeviceCorrespond.ini")
        if os.path.isfile(p):
            sections, order = parse_ini(p)
            found[name] = {"sectionOrder": order, "sections": sections}
dc_doc["state"] = "ready" if found else "missing"
dc_doc["summary"] = {"recipesChecked": len(os.listdir(dc_root)) if os.path.isdir(dc_root) else 0, "found": len(found)}
dc_doc["byRecipe"] = found
write("DeviceCorrespond-config.json", dc_doc)

# ---- 4. Alarm／資料庫類（Handler.db3 依使用者指示忽略，不產生 JSON）--------
alc_path = r"D:\HT9045\Error\AlarmCodeList.txt"
alc_doc = base_doc(alc_path, "cMyDB.cpp（fMain->AlarmCodeList->SaveToFile／LoadFromFile）",
                   "目前語系 Alarm 短描述快取，Code=Description 無 Section 之扁平清單；完整雙語建議詳述另見 AlarmCode_Manual 專案。")
if os.path.isfile(alc_path):
    codes = {}
    for ln in open(alc_path, encoding="cp950", errors="replace").read().splitlines():
        if "=" in ln:
            k, v = ln.split("=", 1)
            codes[k.strip()] = v.strip()
    alc_doc["state"] = "ready"
    alc_doc["summary"] = {"codes": len(codes)}
    alc_doc["codes"] = codes
else:
    alc_doc["state"] = "missing"
    alc_doc["summary"] = {"codes": 0}
    alc_doc["codes"] = {}
write("AlarmCodeList-index.json", alc_doc)

adesc_path = r"D:\HT9045\Error\AlarmDescription.ini"
adesc_doc = base_doc(adesc_path, "note.cpp（strList->LoadFromFile）",
                     "[CODE_語系] 區段標題後為自由格式描述文字（非 key=value），逐段整段存為文字。")
if os.path.isfile(adesc_path):
    sections = {}
    order = []
    cur = None
    buf = []
    for ln in open(adesc_path, encoding="cp950", errors="replace").read().splitlines():
        m = re.match(r"^\[(.+?)\]\s*$", ln.strip())
        if m:
            if cur is not None:
                sections[cur] = "\n".join(buf).strip("\n")
            cur = m.group(1)
            order.append(cur)
            buf = []
        else:
            buf.append(ln)
    if cur is not None:
        sections[cur] = "\n".join(buf).strip("\n")
    adesc_doc["state"] = "ready"
    adesc_doc["summary"] = {"sections": len(sections)}
    adesc_doc["sectionOrder"] = order
    adesc_doc["sections"] = sections
else:
    adesc_doc["state"] = "missing"
    adesc_doc["summary"] = {"sections": 0}
    adesc_doc["sectionOrder"] = []
    adesc_doc["sections"] = {}
write("AlarmDescription-config.json", adesc_doc)

err_root = r"D:\HT9045\Error"
lang_doc = base_doc(err_root, "cSecurity.cpp／KYECFTP\\FTPClient.cpp",
                    "各語系資料夾內每碼一個 .dat 描述檔的『索引』（僅列碼別，不含全文；完整雙語內容另見 AlarmCode_Manual 專案產出）。")
langs = {}
if os.path.isdir(err_root):
    for d in sorted(os.listdir(err_root)):
        p = os.path.join(err_root, d)
        if os.path.isdir(p):
            codes = sorted(os.path.splitext(n)[0] for n in os.listdir(p) if n.lower().endswith(".dat"))
            if codes:
                langs[d] = codes
    lang_doc["state"] = "ready"
    lang_doc["summary"] = {"languages": len(langs), "totalFiles": sum(len(v) for v in langs.values())}
    lang_doc["languages"] = langs
else:
    lang_doc["state"] = "missing"
    lang_doc["summary"] = {"languages": 0, "totalFiles": 0}
    lang_doc["languages"] = {}
write("AlarmDescriptionOverride-index.json", lang_doc)

# ---- 5. SECS／其他系統類 ----------------------------------------------------
ini_job("SecsGem-config.json", r"D:\HT9045\SECS\SECS\SYSTEM\Gerneral.ini", "common.cpp SecsGemPath（獨立於 system\\Gerneral.ini 的 SECS/GEM 系統設定）")

pm_files = {
    "PM_Month": r"D:\HT9045\PMAlarm\PM_Month.ini",
    "PM_Quarter": r"D:\HT9045\PMAlarm\PM_Quarter.ini",
    "PM_Year": r"D:\HT9045\PMAlarm\PM_Year.ini",
    "PM_Temperature": r"D:\HT9045\PMAlarm\PM_Temperature.ini",
    "PM_ESD": r"D:\HT9045\PMAlarm\PM_ESD.ini",
    "PM_IonFan": r"D:\HT9045\PMAlarm\PM_IonFan.ini",
    "PM_Setting": r"D:\HT9045\PMAlarm\PM_Setting.ini",
}
pm_doc = base_doc(list(pm_files.values()), "PMAlarm\\PMAlarmInterFace.cpp", "保養警報（PM Alarm）排程與門檻設定，7 檔合併一份 JSON。")
pm_doc["files"] = {}
for name, p in pm_files.items():
    if os.path.isfile(p):
        sections, order = parse_ini(p)
        pm_doc["files"][name] = {"state": "ready", "sectionOrder": order, "sections": sections}
    else:
        pm_doc["files"][name] = {"state": "missing", "sectionOrder": [], "sections": {}}
pm_doc["state"] = "ready"
pm_doc["summary"] = {"files": len(pm_files), "present": sum(1 for v in pm_doc["files"].values() if v["state"] == "ready")}
write("PMAlarm-config.json", pm_doc)

ini_job("ProductionInfo-config.json", r"D:\HT9045\system\ProductionInfo\PI_Setting.ini", "ProductionInfo\\ProductionInfo.cpp（GetProInfoFilePath()+\"\\PI_Setting.ini\"）")
ini_job("NSKit-flag.json", r"D:\HT9045\system\NSKit.txt", "CosFunction.cpp（CheckIniData 讀 [System] NSKit）")
ini_job("SitMap-config.json", r"C:\Windows\SitMap.ini", "cSetUp.cpp", note="Site Map 加密備份；路徑在 D:\\HT9045 範圍外，優先度最低，schema-only。")

# ---- 6. LastSet（lastdata.dat／login.dat）：schema-only contract，不做二進位解析 ----
lastset_doc = base_doc([r"D:\HT9045\system\lastdata.dat", r"D:\HT9045\system\lastdata_backup.dat",
                        r"D:\HT9045\system\lastdata_backup2.dat", r"D:\HT9045\system\login.dat"],
                       "cprod.cpp／LastSet.cpp（struct LAST_GENERAL_SET LastSet; ReadFile/WriteFile 二進位讀寫）",
                       "⚠ 依 skill ht9045-array-audit：LAST_GENERAL_SET 僅可由 C++ 端依實際 struct 逐欄位輸出 JSON "
                       "projection，Python 端不可臆測 binary layout（欄位增刪/對齊/版本相容性風險）。本檔僅為契約骨架，"
                       "實際欄位需由 C++ 工程師依 cprod.h LAST_GENERAL_SET 定義填入 fields，並在每次讀寫後原子更新。")
lastset_doc["state"] = "pending-cpp-projection"
for p in lastset_doc["source"]["file"]:
    lastset_doc.setdefault("filesOnDisk", {})[p] = {"exists": os.path.isfile(p), "bytes": (os.path.getsize(p) if os.path.isfile(p) else 0)}
lastset_doc["fields"] = []
write("LastSet-projection-contract.json", lastset_doc)

print("DONE")
