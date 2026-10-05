"""
Extract SVID, ECID, CEID, RCMD reference tables from:
  1. Excel: SECS_20250717_Ifor.xlsx
  2. Source: uHGemHT9045_SV.cpp, uHGemHT9045_EC.cpp, uHGemHT9045.h, uHGemHT9045.cpp
Output: 4 Markdown reference files for ht9045-secs-sem SKILL
"""
import openpyxl
import re
import os

XLSX = r'D:\HT9045\.github\skills\ht9045-secs-sem\references\SECS_20250717_Ifor.xlsx'
SRC  = r'D:\HT9045\HT9011UC_Code_V3.33.899.0_20260325_9045AU_Backup_BeforeJimmy_20260330\SECSGEM'
OUT  = r'D:\HT9045\.github\skills\ht9045-secs-sem\references'

wb = openpyxl.load_workbook(XLSX, data_only=True)

# ============================================================================
# 1. SVID & ECID from Excel + Source
# ============================================================================
def extract_sv_ec_from_excel():
    ws = wb['SV & EC']
    svid_list = []
    ecid_list = []
    for row in ws.iter_rows(min_row=3, max_row=ws.max_row, values_only=False):
        id_val   = row[1].value   # col B = ID
        is_sv    = row[2].value   # col C = SV marker
        is_ec    = row[3].value   # col D = EC marker
        is_s7f23 = row[4].value   # col E = S7F23
        func     = row[5].value   # col F = Function name
        dtype    = row[6].value   # col G = Type
        unit     = row[7].value   # col H = Unit
        vmax     = row[8].value   # col I = Max
        vmin     = row[9].value   # col J = Min
        default  = row[10].value  # col K = default
        desc     = row[11].value  # col L = Description
        ht9045   = row[14].value  # col O = HT9045/HT9046 support

        if id_val is None:
            continue
        try:
            id_num = int(id_val)
        except (ValueError, TypeError):
            continue

        if ht9045 != 'V':
            continue

        func_s  = str(func).strip() if func else ''
        dtype_s = str(dtype).strip() if dtype else ''
        unit_s  = str(unit).strip() if unit else ''
        desc_s  = str(desc).strip().replace('\n', ' ') if desc else ''
        vmax_s  = str(vmax).strip() if vmax else ''
        vmin_s  = str(vmin).strip() if vmin else ''
        def_s   = str(default).strip() if default else ''
        s7f23_s = str(is_s7f23).strip() if is_s7f23 else ''

        if is_sv == 'V':
            svid_list.append({
                'id': id_num, 'name': func_s, 'type': dtype_s,
                'unit': unit_s, 'desc': desc_s
            })
        if is_ec == 'V':
            ecid_list.append({
                'id': id_num, 'name': func_s, 'type': dtype_s,
                'unit': unit_s, 'max': vmax_s, 'min': vmin_s,
                'default': def_s, 'desc': desc_s, 's7f23': s7f23_s
            })

    return svid_list, ecid_list


# ============================================================================
# 2. SVID from source code (AddSV)
# ============================================================================
def extract_sv_from_source():
    path = os.path.join(SRC, 'uHGemHT9045_SV.cpp')
    with open(path, 'r', encoding='cp950', errors='replace') as f:
        content = f.read()

    pattern = re.compile(
        r'SetSVDataPointer\(\s*(\d+)\s*,\s*HType\.(\w+)\s*,\s*"([^"]*)"',
        re.DOTALL
    )
    results = []
    for m in pattern.finditer(content):
        results.append({
            'id': int(m.group(1)),
            'type': m.group(2).replace('_TYPE', ''),
            'name': m.group(3)
        })
    return results


# ============================================================================
# 3. ECID from source code (AddEC)
# ============================================================================
def extract_ec_from_source():
    path = os.path.join(SRC, 'uHGemHT9045_EC.cpp')
    with open(path, 'r', encoding='cp950', errors='replace') as f:
        content = f.read()

    pattern = re.compile(
        r'SetECDataPointer\(\s*(\d+)\s*,\s*HType\.(\w+)\s*,\s*"([^"]*)"',
        re.DOTALL
    )
    results = []
    for m in pattern.finditer(content):
        results.append({
            'id': int(m.group(1)),
            'type': m.group(2).replace('_TYPE', ''),
            'name': m.group(3)
        })
    return results


# ============================================================================
# 4. CEID from Excel
# ============================================================================
def extract_ceid_from_excel():
    ws = wb['CEID Report']
    ceid_list = []
    for row in ws.iter_rows(min_row=3, max_row=ws.max_row, values_only=False):
        ceid_val  = row[1].value   # col B = CEID
        rptid_val = row[2].value   # col C = Report ID
        svid_val  = row[3].value   # col D = SV ID
        remark    = row[4].value   # col E = Remark
        ht9045    = row[7].value   # col H = HT9045/HT9046

        if ceid_val is None:
            continue
        try:
            ceid_num = int(ceid_val)
        except (ValueError, TypeError):
            continue

        if ht9045 != 'V':
            continue

        rptid_s  = str(rptid_val).strip() if rptid_val else ''
        svid_s   = str(svid_val).strip() if svid_val else ''
        remark_s = str(remark).strip().replace('\n', ' ').replace('|', '/') if remark else ''

        ceid_list.append({
            'ceid': ceid_num, 'rptid': rptid_s,
            'svid': svid_s, 'remark': remark_s
        })
    return ceid_list


# ============================================================================
# 5. CEID from source code (uHGemHT9045.h enum)
# ============================================================================
def extract_ceid_from_source():
    path = os.path.join(SRC, 'uHGemHT9045.h')
    with open(path, 'r', encoding='cp950', errors='replace') as f:
        content = f.read()

    # Match enum entries like: DoStart=1,  // 1  ...
    pattern = re.compile(r'(\w+)\s*(?:=\s*\d+)?\s*,\s*//\s*(\d+)\s*(.*)', re.MULTILINE)
    results = []
    for m in pattern.finditer(content):
        name = m.group(1)
        ceid = int(m.group(2))
        comment = m.group(3).strip()
        # Clean Big5 garbled
        results.append({'ceid': ceid, 'name': name, 'comment': comment})
    return results


# ============================================================================
# 6. RCMD from Excel
# ============================================================================
def extract_rcmd_from_excel():
    ws = wb['Remote Command']
    rcmd_list = []
    for row in ws.iter_rows(min_row=3, max_row=ws.max_row, values_only=False):
        cmd     = row[1].value   # col B = Command
        remark  = row[2].value   # col C = Remark
        state   = row[3].value   # col D = Valid Machine State
        ht9045  = row[6].value   # col G = HT9045/HT9046

        if cmd is None:
            continue

        cmd_s    = str(cmd).strip()
        if not cmd_s or cmd_s == 'Command':
            continue

        remark_s = str(remark).strip().replace('\n', ' ').replace('|', '/') if remark else ''
        state_s  = str(state).strip().replace('\n', ' ').replace('|', '/') if state else ''
        ht9045_s = str(ht9045).strip() if ht9045 else ''

        rcmd_list.append({
            'cmd': cmd_s, 'remark': remark_s,
            'state': state_s, 'ht9045': ht9045_s
        })
    return rcmd_list


# ============================================================================
# 7. RCMD from source code (S2F42)
# ============================================================================
def extract_rcmd_from_source():
    path = os.path.join(SRC, 'uHGemHT9045.cpp')
    with open(path, 'r', encoding='cp950', errors='replace') as f:
        content = f.read()

    # Find all S.AnsiPos("XXXX")==1 patterns
    pattern = re.compile(r'S\.AnsiPos\("([^"]+)"\)\s*==\s*1')
    cmds = []
    seen = set()
    for m in pattern.finditer(content):
        cmd = m.group(1)
        if cmd not in seen:
            seen.add(cmd)
            cmds.append(cmd)
    return cmds


# ============================================================================
# Generate reference files
# ============================================================================

print("Extracting from Excel...")
excel_sv, excel_ec = extract_sv_ec_from_excel()
excel_ceid = extract_ceid_from_excel()
excel_rcmd = extract_rcmd_from_excel()

print(f"  SV & EC sheet: {len(excel_sv)} SVIDs, {len(excel_ec)} ECIDs")
print(f"  CEID Report:   {len(excel_ceid)} CEIDs")
print(f"  Remote Command:{len(excel_rcmd)} RCMDs")

print("Extracting from source...")
src_sv   = extract_sv_from_source()
src_ec   = extract_ec_from_source()
src_ceid = extract_ceid_from_source()
src_rcmd = extract_rcmd_from_source()

print(f"  Source SV:   {len(src_sv)} entries")
print(f"  Source EC:   {len(src_ec)} entries")
print(f"  Source CEID: {len(src_ceid)} entries")
print(f"  Source RCMD: {len(src_rcmd)} commands")

# Build lookup dicts from source
src_sv_dict = {item['id']: item for item in src_sv}
src_ec_dict = {item['id']: item for item in src_ec}
src_ceid_dict = {item['ceid']: item for item in src_ceid}

# ===== SVID Reference =====
with open(os.path.join(OUT, 'SVID-Reference.md'), 'w', encoding='utf-8') as f:
    f.write("# SVID 狀態變量參照表\n\n")
    f.write("> 資料來源：`SECS_20250717_Ifor.xlsx` + `uHGemHT9045_SV.cpp`\n")
    f.write(f"> 程式碼版本：`HT9011UC_Code_V3.33.899.0_20260325`\n\n")
    f.write(f"HT9045/HT9046 共註冊 **{len(excel_sv)}** 個 SVID（Excel 定義），程式碼實作 **{len(src_sv)}** 個。\n\n")

    f.write("## 目錄\n\n")
    # Group by range
    ranges = [
        (0, 99, "GEM 系統變量 (0-99)"),
        (1000, 1099, "機台基本資訊 (1000-1099)"),
        (1100, 1199, "計數器與良率 (1100-1199)"),
        (1200, 1299, "ATC 溫度 (1200-1299)"),
        (1300, 1499, "測試參數 (1300-1499)"),
        (1500, 1999, "擴充參數 (1500-1999)"),
        (2000, 2999, "下壓/Force 參數 (2000-2999)"),
        (3000, 9999, "進階參數 (3000-9999)"),
        (10000, 19999, "Site 統計 (10000-19999)"),
        (20000, 39999, "Port/擴充 (20000-39999)"),
        (40000, 99999, "Head/觸碰 (40000+)"),
    ]
    for lo, hi, label in ranges:
        items = [s for s in excel_sv if lo <= s['id'] <= hi]
        if items:
            anchor = label.split('(')[0].strip().replace(' ', '-').replace('/', '-')
            f.write(f"- [{label}](#{anchor}) ({len(items)} 項)\n")
    f.write("\n")

    for lo, hi, label in ranges:
        items = [s for s in excel_sv if lo <= s['id'] <= hi]
        if not items:
            continue
        f.write(f"## {label}\n\n")
        f.write("| SVID | Name | Type | Unit | Description | 程式碼 |\n")
        f.write("|-----:|------|------|------|-------------|:------:|\n")
        for item in sorted(items, key=lambda x: x['id']):
            src_match = src_sv_dict.get(item['id'])
            code_mark = '✓' if src_match else ''
            src_name = src_match['name'] if src_match else ''
            disp_name = item['name'] if item['name'] else src_name
            desc_short = item['desc'][:80] if item['desc'] else ''
            f.write(f"| {item['id']} | {disp_name} | {item['type']} | {item['unit']} | {desc_short} | {code_mark} |\n")
        f.write("\n")

    # Source-only SVIDs
    excel_sv_ids = {s['id'] for s in excel_sv}
    src_only_sv = [s for s in src_sv if s['id'] not in excel_sv_ids]
    if src_only_sv:
        f.write("## 僅程式碼存在 (未列於 Excel)\n\n")
        f.write("| SVID | Name | Type |\n")
        f.write("|-----:|------|------|\n")
        for item in sorted(src_only_sv, key=lambda x: x['id']):
            f.write(f"| {item['id']} | {item['name']} | {item['type']} |\n")
        f.write("\n")

print(f"Written: SVID-Reference.md")

# ===== ECID Reference =====
with open(os.path.join(OUT, 'ECID-Reference.md'), 'w', encoding='utf-8') as f:
    f.write("# ECID 設備常數參照表\n\n")
    f.write("> 資料來源：`SECS_20250717_Ifor.xlsx` + `uHGemHT9045_EC.cpp`\n")
    f.write(f"> 程式碼版本：`HT9011UC_Code_V3.33.899.0_20260325`\n\n")
    f.write(f"HT9045/HT9046 共定義 **{len(excel_ec)}** 個 ECID（Excel），程式碼實作 **{len(src_ec)}** 個。\n\n")

    ec_ranges = [
        (0, 999, "GEM 系統常數 (0-999)"),
        (1000, 1099, "Lot/基本資訊 (1000-1099)"),
        (1100, 1499, "Contact/Arm (1100-1499)"),
        (1500, 1599, "配方/溫度/Site (1500-1599)"),
        (1600, 1699, "個別溫度限制 (1600-1699)"),
        (1700, 1999, "2DID/擴充 (1700-1999)"),
        (2000, 2099, "IC 參數 (2000-2099)"),
        (2100, 2299, "下壓/Auto Site Off (2100-2299)"),
        (2300, 2599, "Speed/Arm 參數 (2300-2599)"),
        (2600, 2999, "下壓/Tray (2600-2999)"),
        (3000, 3999, "測試介面/Bin (3000-3999)"),
        (4000, 9999, "進階功能 (4000+)"),
    ]

    f.write("## 目錄\n\n")
    for lo, hi, label in ec_ranges:
        items = [e for e in excel_ec if lo <= e['id'] <= hi]
        if items:
            anchor = label.split('(')[0].strip().replace(' ', '-').replace('/', '-')
            f.write(f"- [{label}](#{anchor}) ({len(items)} 項)\n")
    f.write("\n")

    for lo, hi, label in ec_ranges:
        items = [e for e in excel_ec if lo <= e['id'] <= hi]
        if not items:
            continue
        f.write(f"## {label}\n\n")
        f.write("| ECID | Name | Type | Max | Min | Default | Description | 程式碼 |\n")
        f.write("|-----:|------|------|-----|-----|---------|-------------|:------:|\n")
        for item in sorted(items, key=lambda x: x['id']):
            src_match = src_ec_dict.get(item['id'])
            code_mark = '✓' if src_match else ''
            src_name = src_match['name'] if src_match else ''
            disp_name = item['name'] if item['name'] else src_name
            desc_short = item['desc'][:80] if item['desc'] else ''
            f.write(f"| {item['id']} | {disp_name} | {item['type']} | {item['max']} | {item['min']} | {item['default']} | {desc_short} | {code_mark} |\n")
        f.write("\n")

    # Source-only ECIDs
    excel_ec_ids = {e['id'] for e in excel_ec}
    src_only_ec = [e for e in src_ec if e['id'] not in excel_ec_ids]
    if src_only_ec:
        f.write("## 僅程式碼存在 (未列於 Excel)\n\n")
        f.write("| ECID | Name | Type |\n")
        f.write("|-----:|------|------|\n")
        for item in sorted(src_only_ec, key=lambda x: x['id']):
            f.write(f"| {item['id']} | {item['name']} | {item['type']} |\n")
        f.write("\n")

print(f"Written: ECID-Reference.md")

# ===== CEID Reference =====
with open(os.path.join(OUT, 'CEID-Reference.md'), 'w', encoding='utf-8') as f:
    f.write("# CEID 收集事件參照表\n\n")
    f.write("> 資料來源：`SECS_20250717_Ifor.xlsx` + `uHGemHT9045.h` ETypeStruct enum\n")
    f.write(f"> 程式碼版本：`HT9011UC_Code_V3.33.899.0_20260325`\n\n")
    f.write(f"Excel 定義 **{len(excel_ceid)}** 個 CEID，程式碼列舉 **{len(src_ceid)}** 個。\n\n")

    f.write("## 完整 CEID 對照表\n\n")
    f.write("| CEID | 程式碼名稱 | Report ID | SV ID | 說明 | 程式碼 |\n")
    f.write("|-----:|-----------|-----------|-------|------|:------:|\n")

    # Merge excel + source
    all_ceids = set()
    for item in excel_ceid:
        all_ceids.add(item['ceid'])
    for item in src_ceid:
        all_ceids.add(item['ceid'])

    for ceid in sorted(all_ceids):
        excel_item = next((c for c in excel_ceid if c['ceid'] == ceid), None)
        src_item = src_ceid_dict.get(ceid)

        name = src_item['name'] if src_item else ''
        rptid = excel_item['rptid'] if excel_item else ''
        svid = excel_item['svid'] if excel_item else ''
        remark = excel_item['remark'] if excel_item else (src_item['comment'][:60] if src_item else '')
        code_mark = '✓' if src_item else ''

        f.write(f"| {ceid} | {name} | {rptid} | {svid} | {remark[:80]} | {code_mark} |\n")

    f.write("\n")

    # Summary by category
    f.write("## CEID 分類摘要\n\n")
    categories = [
        (1, 34, "機台操作事件"),
        (35, 70, "Tray / ART 事件"),
        (71, 90, "擴充操作事件"),
        (91, 93, "SECS 連線狀態"),
        (94, 135, "OHT / Cassette 事件"),
        (136, 211, "Tray / Port 擴充事件"),
        (212, 216, "省電模式"),
        (217, 233, "Port 狀態變更"),
        (234, 270, "SafetyDoor / Bundle 事件"),
        (271, 288, "進階事件 (AGV/RFID/Material)"),
    ]
    f.write("| 範圍 | 類別 | 數量 |\n")
    f.write("|:----:|------|:----:|\n")
    for lo, hi, label in categories:
        count = len([c for c in all_ceids if lo <= c <= hi])
        if count:
            f.write(f"| {lo}-{hi} | {label} | {count} |\n")
    f.write("\n")

print(f"Written: CEID-Reference.md")

# ===== RCMD Reference =====
with open(os.path.join(OUT, 'RCMD-Reference.md'), 'w', encoding='utf-8') as f:
    f.write("# RCMD 遠端控制指令參照表\n\n")
    f.write("> 資料來源：`SECS_20250717_Ifor.xlsx` + `uHGemHT9045.cpp` S2F42 處理\n")
    f.write(f"> 程式碼版本：`HT9011UC_Code_V3.33.899.0_20260325`\n\n")

    f.write("## Excel 規格定義\n\n")
    f.write("| Command | 說明 | Valid Machine State | HT9045 |\n")
    f.write("|---------|------|---------------------|:------:|\n")
    for item in excel_rcmd:
        f.write(f"| `{item['cmd']}` | {item['remark'][:60]} | {item['state'][:60]} | {item['ht9045']} |\n")
    f.write("\n")

    f.write("## 程式碼實作 (S2F42 Handler)\n\n")
    f.write("以下為 `uHGemHT9045.cpp` 中 `S2F42_Host_Command_Acknowledge()` 實際處理的 RCMD：\n\n")
    f.write("| # | Command | Excel 定義 |\n")
    f.write("|:-:|---------|:----------:|\n")
    excel_cmds = {r['cmd'].upper() for r in excel_rcmd}
    for i, cmd in enumerate(src_rcmd, 1):
        in_excel = '✓' if cmd.upper() in excel_cmds else ''
        f.write(f"| {i} | `{cmd}` | {in_excel} |\n")
    f.write("\n")

    # Source-only commands
    src_cmd_set = {c.upper() for c in src_rcmd}
    excel_only = [r for r in excel_rcmd if r['cmd'].upper() not in src_cmd_set]
    if excel_only:
        f.write("## 僅 Excel 定義（程式碼未實作）\n\n")
        f.write("| Command | 說明 |\n")
        f.write("|---------|------|\n")
        for item in excel_only:
            f.write(f"| `{item['cmd']}` | {item['remark'][:60]} |\n")
        f.write("\n")

    f.write("## HCACK 回傳值\n\n")
    f.write("| HCACK | 說明 | 備註 |\n")
    f.write("|:-----:|------|------|\n")
    f.write("| 0 | Acknowledge (成功) | 指令執行成功 |\n")
    f.write("| 1 | Denied, invalid command | 無效指令或前置條件不符 |\n")
    f.write("| 2 | Denied, cannot perform now | 機台有 IC / 執行中 |\n")
    f.write("| 3 | Denied, parameter error | 參數錯誤 |\n")
    f.write("| 4 | Acknowledge, will complete later | 非同步完成 |\n")
    f.write("| **7** | FTP 控制中 | `DOWNLOAD_RECIPE_BY_FTP` 執行中 (HT9045 擴充) |\n")
    f.write("| **8** | LIST 結構錯誤 | S2F41 LIST 格式不正確 (HT9045 擴充) |\n")
    f.write("| **9** | 參數名稱錯誤 | CPNAME 非 \"Setup_File\" (HT9045 擴充) |\n")
    f.write("| **10** | LIST 類型錯誤 | 資料類型不符 (HT9045 擴充) |\n")
    f.write("\n> HCACK 7-10 為 HT9045 自定義擴充碼 (Steven 20240923)\n")

print(f"Written: RCMD-Reference.md")
print("\nDone! All 4 reference files generated.")
