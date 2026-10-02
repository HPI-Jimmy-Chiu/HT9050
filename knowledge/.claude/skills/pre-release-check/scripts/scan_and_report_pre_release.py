import os
import re
import sys
import json
import argparse
import subprocess
from datetime import datetime

# ---------------------------------------------------------------------------
# 設定（F：不再寫死路徑，改吃命令列參數 / 可由版本資料夾自動推導）
#
#   python scan_and_report_pre_release.py <PROJECT_DIR> [options]
#
# 範例：
#   python scan_and_report_pre_release.py "D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618"
#
# 版本標籤（VERSION_TAG）會自動從資料夾名稱抽出 Vx.y.z；找不到時退回 "unknown"。
# REPORT_DIR / DEVELOPER 皆可用參數覆寫，未指定時用合理預設。
#
# 報告製作 follow make-report-skill（除錯 / 上線前掃描類）：
#   - 先產 MD（MD 不放 Logo），再用 honprec-blue-template 轉 HTML。
#   - HTML 一律透過 make-report-skill 的 md_to_html.py 產生（不在本腳本內嵌 base64）。
# ---------------------------------------------------------------------------

DEFAULT_REPORT_DIR = r"D:\docs\customers\HPI-TW\0000_HonPrec"
DEFAULT_DEVELOPER = "Steven"

# make-report-skill 的 MD→HTML 轉換器（除錯/風險報告用 blue 模板）
MD_TO_HTML = r"D:\.github\skills\make-report-skill\scripts\md_to_html.py"
REPORT_TEMPLATE = "blue"

SRC_EXT = {".cpp", ".h", ".c"}
DFM_EXT = {".dfm"}

# 由 main() 依參數填入
PROJECT_DIR = ""
REPORT_DIR = ""
DEVELOPER = ""
VERSION_TAG = ""
DATE_TAG = ""
REPORT_MD = ""
REPORT_HTML = ""
COMMIT_MSG = ""
SCAN_JSON = ""


def extract_version_tag(project_dir):
    """從版本資料夾名稱抽出 Vx.y.z[.w]（F：吃檔案版號）。"""
    base = os.path.basename(os.path.normpath(project_dir))
    m = re.search(r"V(\d+\.\d+(?:\.\d+){0,2})", base)
    return ("V" + m.group(1)) if m else "unknown"


def configure(project_dir, report_dir=None, developer=None):
    """填入全域設定（取代舊版寫死常數）。"""
    global PROJECT_DIR, REPORT_DIR, DEVELOPER, VERSION_TAG, DATE_TAG
    global REPORT_MD, REPORT_HTML, COMMIT_MSG, SCAN_JSON
    PROJECT_DIR = os.path.normpath(project_dir)
    REPORT_DIR = report_dir or DEFAULT_REPORT_DIR
    DEVELOPER = developer or DEFAULT_DEVELOPER
    VERSION_TAG = extract_version_tag(PROJECT_DIR)
    DATE_TAG = datetime.now().strftime("%Y%m%d")
    stem = f"PreReleaseRiskCheck_{VERSION_TAG}_{DATE_TAG}"
    REPORT_MD = os.path.join(REPORT_DIR, f"{stem}.md")
    REPORT_HTML = os.path.join(REPORT_DIR, f"{stem}.html")
    COMMIT_MSG = os.path.join(REPORT_DIR, f"SVN_PreCommit_Message_{VERSION_TAG}_{DATE_TAG}.txt")
    SCAN_JSON = os.path.join(REPORT_DIR, f"{stem}.json")


def walk_files(root):
    out = []
    for d, dirs, files in os.walk(root):
        dirs[:] = [x for x in dirs if x.lower() != ".svn"]
        for f in files:
            ext = os.path.splitext(f)[1].lower()
            if ext in SRC_EXT:
                out.append(os.path.join(d, f))
    return out


def walk_dfm_files(root):
    out = []
    for d, dirs, files in os.walk(root):
        dirs[:] = [x for x in dirs if x.lower() != ".svn"]
        for f in files:
            if os.path.splitext(f)[1].lower() in DFM_EXT:
                out.append(os.path.join(d, f))
    return out


def read_cp950(path):
    with open(path, "rb") as f:
        data = f.read()
    try:
        return data.decode("cp950")
    except Exception:
        return data.decode("cp950", errors="replace")


def rel(path):
    return os.path.relpath(path, PROJECT_DIR).replace("/", "\\")


def strip_inline_comment(line):
    # Keep it simple for C++: remove // trailing comment part
    p = line.find("//")
    if p >= 0:
        return line[:p]
    return line


def find_p1(lines, file_rel):
    findings = []
    for i, raw in enumerate(lines, 1):
        line = strip_inline_comment(raw)
        if "||" not in line and "&&" not in line:
            continue
        if "if" not in line and "while" not in line and "for" not in line and "return" not in line:
            continue
        terms = re.split(r"\|\||&&", line)
        if len(terms) < 2:
            continue

        has_compare = any(re.search(r"==|!=|<=|>=|<|>", t) for t in terms)
        if not has_compare:
            continue

        for t in terms:
            s = t.strip().strip("() ")
            if not s:
                continue
            if re.search(r"==|!=|<=|>=|<|>", s):
                continue
            if re.search(r"\btrue\b|\bfalse\b", s):
                continue
            if re.search(r"\w+\s*\(", s):
                continue
            if re.match(r"^[A-Za-z_][A-Za-z0-9_]*$", s) or re.match(r"^\d+$", s):
                findings.append({
                    "file": file_rel,
                    "line": i,
                    "pattern": "P1",
                    "severity": "Critical",
                    "description": f"鏈式條件可能存在裸值項目: {s}",
                    "snippet": raw.strip(),
                    "suggested_fix": "確認是否漏寫比較運算子（例如 var==X）。"
                })
                break
    return findings


def collect_enum_members(text):
    members = set()
    for m in re.finditer(r"enum\s+[A-Za-z_][A-Za-z0-9_]*?\s*\{([\s\S]*?)\}", text):
        body = m.group(1)
        body = re.sub(r"/\*.*?\*/", "", body, flags=re.S)
        for part in body.split(","):
            item = part.strip()
            if not item:
                continue
            name = item.split("=")[0].strip()
            if re.match(r"^[A-Za-z_][A-Za-z0-9_]*$", name):
                members.add(name)
    return members


def find_p2(lines, file_rel, enum_members):
    findings = []
    if not enum_members:
        return findings
    patt = re.compile(r"\b(if|while)\s*\(\s*!?\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)")
    for i, raw in enumerate(lines, 1):
        line = strip_inline_comment(raw)
        m = patt.search(line)
        if not m:
            continue
        token = m.group(2)
        if token in enum_members:
            findings.append({
                "file": file_rel,
                "line": i,
                "pattern": "P2",
                "severity": "High",
                "description": f"enum 成員 {token} 直接作為布林條件",
                "snippet": raw.strip(),
                "suggested_fix": "改為明確比較（例如 var==EnumMember）。"
            })
    return findings


def find_p4(lines, file_rel):
    findings = []
    for i, raw in enumerate(lines, 1):
        line = strip_inline_comment(raw)
        if re.search(r"\bSave[A-Za-z0-9_]*\s*\(", line) and re.search(r"\b(Read|Load)[A-Za-z0-9_]*\s*\(", line):
            findings.append({
                "file": file_rel,
                "line": i,
                "pattern": "P4",
                "severity": "High",
                "description": "同一行或鄰近邏輯出現 Save 後 Read/Load 呼叫，需確認回載對稱性",
                "snippet": raw.strip(),
                "suggested_fix": "核對 Save 與 Read/Load 欄位對稱。"
            })
    return findings


def find_p5(lines, file_rel):
    findings = []
    for i, raw in enumerate(lines, 1):
        line = strip_inline_comment(raw)
        if re.search(r"Out|Output", line, re.I) and re.search(r"\bInOfs[A-Za-z0-9_]*\b", line):
            findings.append({
                "file": file_rel,
                "line": i,
                "pattern": "P5",
                "severity": "Critical",
                "description": "Output 語境中出現 InOfs* 變數，疑似 copy-paste 誤用",
                "snippet": raw.strip(),
                "suggested_fix": "比對 Input/Output 區段，確認變數命名語義一致。"
            })
    return findings


def find_p6(lines, file_rel):
    findings = []
    safe_funcs = ("ChangeToFloat", "ChangeToPercentage")
    div_pat = re.compile(r"/(\s*)([A-Za-z_][A-Za-z0-9_\.\->\[\]]*)")

    for i, raw in enumerate(lines, 1):
        line = strip_inline_comment(raw)
        if not line or "/" not in line:
            continue
        if "//" in raw and raw.strip().startswith("//"):
            continue
        if any(fn in line for fn in safe_funcs):
            continue

        m = div_pat.search(line)
        if not m:
            continue
        divisor = m.group(2)
        if re.match(r"^[0-9]+(\.[0-9]+)?$", divisor):
            continue

        # local context guard check (20 lines up)
        guarded = False
        start = max(0, i - 21)
        for ctx in lines[start:i]:
            if re.search(rf"\bif\s*\(\s*{re.escape(divisor)}\s*(!=|>)\s*0", ctx):
                guarded = True
                break
            if re.search(rf"\bif\s*\(\s*0\s*<\s*{re.escape(divisor)}", ctx):
                guarded = True
                break
            if re.search(rf"\b{re.escape(divisor)}\s*\?", ctx):
                guarded = True
                break
        if guarded:
            continue

        findings.append({
            "file": file_rel,
            "line": i,
            "pattern": "P6",
            "severity": "Critical",
            "description": f"除法除數 {divisor} 可能未做 zero-guard",
            "snippet": raw.strip(),
            "suggested_fix": "以 ChangeToFloatNonPcnt(...) 或 if(divisor!=0) 防護。"
        })
    return findings


HELPER_PAT = re.compile(r"MyForceDirectories|EnsureDirectoriesExist")
WRITE_FOPEN_PAT = re.compile(r'fopen\s*\([^,]+,\s*"[wa]')
WRITE_API_PAT  = re.compile(
    r'SaveToFile\s*\(|'
    r'TFileStream\s*\([^,]+,\s*fmCreate|'
    r'CopyFile\s*\(|'
    r'WritePrivateProfileString\s*\('
)

def find_p9(lines, file_rel):
    """P9 — Folder Existence Before File Write."""
    findings = []
    for i, raw in enumerate(lines, 1):
        line = strip_inline_comment(raw)
        if not (WRITE_FOPEN_PAT.search(line) or WRITE_API_PAT.search(line)):
            continue
        # check upstream 30 lines for helper
        start = max(0, i - 31)
        ctx_block = "\n".join(lines[start:i - 1])
        if HELPER_PAT.search(ctx_block):
            continue
        api = "fopen(write/append)" if WRITE_FOPEN_PAT.search(line) else \
              ("SaveToFile" if "SaveToFile" in line else
               ("TFileStream(fmCreate)" if "fmCreate" in line else
                ("CopyFile" if "CopyFile" in line else "WritePrivateProfileString")))
        findings.append({
            "file": file_rel,
            "line": i,
            "pattern": "P9",
            "severity": "High",
            "description": f"{api} 前 30 行內未見 MyForceDirectories / EnsureDirectoriesExist",
            "snippet": raw.strip(),
            "suggested_fix": "寫檔前加 MyForceDirectories(path, \"FuncName\") 或 FileInfo().EnsureDirectoriesExist(path)。"
        })
    return findings


# P7 — Loop Hardcoded Upper-Bound vs Known Array Dimension Constants
# Flag for-loops where the loop condition uses a raw integer literal instead of
# a named dimension constant (MAX_ARM_Col, MAX_AUTO_TRAY, etc.).
KNOWN_DIM_CONSTS = {
    "MAX_ARM_Row": 2, "MAX_ARM_Col": 4,
    "MAX_Index_Row": 2, "MAX_Index_Col": 8,
    "MAX_SOCKET_ROW": 4, "MAX_SOCKET_COL": 8,
    "MAX_AUTO_TRAY": 6, "MAX_TRACK": 9,
    "MAX_FIX_TRAY": 6, "eTrayCount": 33,
    "TEST_MAX_BIN": 256, "iSnSocketCnt": 24,
}
# Match: for(... i < NUMBER ...) or for(... i <= NUMBER ...)
P7_FOR_PAT = re.compile(
    r'\bfor\s*\([^;]*[a-zA-Z_]\w*\s*[<>]=?\s*(\d+)\s*[;,)]'
)
# Literal numbers that are almost always safe to ignore (0, 1, 2 used as small counters)
P7_SAFE_LITERALS = {0, 1, 2}

def find_p7(lines, file_rel):
    """P7 — Array loop upper bound uses hardcoded integer instead of named constant."""
    findings = []
    for i, raw in enumerate(lines, 1):
        # skip pure comments
        stripped = raw.strip()
        if stripped.startswith("//"):
            continue
        line = strip_inline_comment(raw)
        m = P7_FOR_PAT.search(line)
        if not m:
            continue
        literal = int(m.group(1))
        if literal in P7_SAFE_LITERALS:
            continue
        # check if any known constant has the SAME numeric value — then it's a
        # potential magic-number substitution of that constant.
        matching = [k for k, v in KNOWN_DIM_CONSTS.items() if v == literal]
        hint = ""
        if matching:
            hint = f" (疑似應使用 {'/'.join(matching)}={literal})"
        findings.append({
            "file": file_rel,
            "line": i,
            "pattern": "P7",
            "severity": "High",
            "description": f"for 迴圈硬編碼上限 {literal}{hint}，應改用維度常數",
            "snippet": raw.strip(),
            "suggested_fix": f"將硬編碼數字換成對應常數（{', '.join(matching) if matching else '請確認陣列宣告大小'}），避免陣列宣告修改後迴圈未同步更新。"
        })
    return findings


# P11 — `==` used instead of `=` (statement-position comparison, no side effect)
# 偵測：非 if/while/for/return 開頭的陳述式，含 `==` 且以 `;` 結尾。
# B：必加 FP 過濾（括號平衡、前一行續行、三元）—— 原始命中誤報率約 82%。
P11_STMT_PAT = re.compile(r'^\s*([A-Za-z_][\w\.\[\]\->]*)\s*==\s*[^;]+;\s*$')
P11_PREV_CONT = ('|', '&', '(', ',', '?', ':', '+', '-', '*', '/', '<', '>', '=')

# G：嚴重度啟發式。控制流 / 資料持久化 = High/Critical；純 UI 顯示 = Low。
P11_CTRLFLOW_HINTS = ("ret", "result", "->Active", "->Port")
P11_UI_HINTS = ("->Checked", "->ActivePageIndex", "->Visible", "->Enabled",
                "->Caption", "->Text", "->Color", "->Font", "->ItemIndex")


def _p11_severity(lvalue, snippet):
    s = (lvalue + " " + snippet)
    if any(h in s for h in P11_UI_HINTS):
        return "Low"
    if any(h in s for h in P11_CTRLFLOW_HINTS):
        return "Critical"
    return "High"


def find_p11(lines, file_rel):
    """P11 — `==` 誤作 `=`。含 B 的 FP 過濾與 G 的嚴重度啟發式。"""
    findings = []
    for i, raw in enumerate(lines, 1):
        line = strip_inline_comment(raw)
        m = P11_STMT_PAT.match(line)
        if not m:
            continue
        lvalue = m.group(1)
        # FP-1：括號不平衡（`)` 多於 `(`）→ 多行條件的續行，跳過
        if line.count(")") > line.count("("):
            continue
        # FP-2：三元運算 `?` ... `:` → 非賦值語句
        if "?" in line and ":" in line:
            continue
        # FP-3：lvalue 側若含 `(` → 是函式呼叫片段，跳過
        if "(" in lvalue:
            continue
        # FP-4：前一個非空程式碼行以續行符號結尾 → 本行屬多行運算式中段
        j = i - 2
        while j >= 0:
            prev = strip_inline_comment(lines[j]).rstrip()
            if prev == "":
                j -= 1
                continue
            if prev.endswith(P11_PREV_CONT):
                lvalue = None  # mark skip
            break
        if lvalue is None:
            continue
        sev = _p11_severity(m.group(1), raw.strip())
        findings.append({
            "file": file_rel,
            "line": i,
            "pattern": "P11",
            "severity": sev,
            "description": f"`==` 疑似誤作 `=`（陳述式無 side effect，{m.group(1)} 實際未賦值）",
            "snippet": raw.strip(),
            "suggested_fix": "確認應為賦值；若是請改為單一 `=`。"
        })
    return findings


# P12 — malloc/calloc/realloc 配 delete，或 new[] 配 delete（缺 []）。同作用域配對。
P12_MALLOC_PAT = re.compile(r'(\w+)\s*=\s*\([^)]+\)\s*(?:malloc|calloc|realloc)\s*\(')
P12_NEW_ARR_PAT = re.compile(r'(\w+)\s*=\s*new\s+[\w:<>\s\*]+\[')


def find_p12(lines, file_rel):
    """P12 — malloc/delete 配對錯誤（同函式作用域啟發式）。"""
    findings = []
    depth = 0
    malloc_vars = {}   # var -> src line (malloc/calloc/realloc)
    newarr_vars = {}   # var -> src line (new[])
    for i, raw in enumerate(lines, 1):
        line = strip_inline_comment(raw)
        depth += line.count("{") - line.count("}")
        if depth <= 1:
            malloc_vars = {}
            newarr_vars = {}
        m = P12_MALLOC_PAT.search(line)
        if m:
            malloc_vars[m.group(1)] = i
        m2 = P12_NEW_ARR_PAT.search(line)
        if m2:
            newarr_vars[m2.group(1)] = i
        # delete X;  (無 [])
        d = re.search(r'\bdelete\s+(\w+)\s*;', line)
        if d:
            v = d.group(1)
            if v in malloc_vars:
                findings.append({
                    "file": file_rel, "line": i, "pattern": "P12", "severity": "Critical",
                    "description": f"{v} 由 malloc/calloc/realloc 配置（line {malloc_vars[v]}），卻用 delete 釋放 → UB",
                    "snippet": raw.strip(),
                    "suggested_fix": f"改用 free({v}); 以配對 C 標準庫配置。"
                })
            elif v in newarr_vars:
                findings.append({
                    "file": file_rel, "line": i, "pattern": "P12", "severity": "Critical",
                    "description": f"{v} 由 new[] 配置（line {newarr_vars[v]}），卻用 delete（缺 []）→ UB",
                    "snippet": raw.strip(),
                    "suggested_fix": f"改用 delete[] {v};。"
                })
    return findings


# DFM — Form 開在單螢幕可視範圍外（C：只看 Form 物件本身的 Left/Top，不看子元件）
DFM_FORM_PAT = re.compile(r'^\s*object\s+(\w+)\s*:\s*(T\w+)\s*$')
DFM_LEFT_PAT = re.compile(r'^\s*Left\s*=\s*(\d+)\s*$')
DFM_TOP_PAT = re.compile(r'^\s*Top\s*=\s*(\d+)\s*$')
DFM_MAX_LEFT = 1280
DFM_MAX_TOP = 1024
DFM_IDE_PLACEHOLDER = 60000  # 排除 IDE 佔位座標（如 65532）


def find_dfm_form_offscreen(lines, file_rel):
    """C：僅檢查每個 .dfm 的『第一個 object（Form 本身）』的 Left/Top。
    子元件 Left/Top 為相對父層，一律不檢查。"""
    findings = []
    # 只取檔案最上層 Form：第一個 object 行
    form_name = None
    form_line = 0
    for i, raw in enumerate(lines, 1):
        m = DFM_FORM_PAT.match(raw)
        if m:
            form_name, form_class, form_line = m.group(1), m.group(2), i
            break
    if not form_name:
        return findings
    # Form 的 Left/Top 在緊接其後的屬性區（第一個 end/object 之前）
    left = top = None
    for raw in lines[form_line:form_line + 30]:
        if DFM_FORM_PAT.match(raw):  # 進入子元件，停止
            break
        ml = DFM_LEFT_PAT.match(raw)
        if ml and left is None:
            left = int(ml.group(1))
        mt = DFM_TOP_PAT.match(raw)
        if mt and top is None:
            top = int(mt.group(1))
    bad = []
    if left is not None and DFM_MAX_LEFT < left < DFM_IDE_PLACEHOLDER:
        bad.append(f"Left={left}")
    if top is not None and DFM_MAX_TOP < top < DFM_IDE_PLACEHOLDER:
        bad.append(f"Top={top}")
    if bad:
        findings.append({
            "file": file_rel, "line": form_line, "pattern": "DFM", "severity": "Medium",
            "description": f"Form {form_name}({form_class}) 設計座標超出可視範圍：{', '.join(bad)}",
            "snippet": f"object {form_name}: {form_class}",
            "suggested_fix": "僅將此 Form 的 Left/Top 改為 10/10（子元件不可動）。"
        })
    return findings


# P13 — block-memory size mismatch (strcpy / strncpy / memcpy)
P13_BARE_STRCPY  = re.compile(r'\bstrcpy\s*\(')
P13_STRNCPY_LEN  = re.compile(r'\bstrncpy\s*\([^,]+,[^,]+,\s*\w+\.(?:Length|length|size)\s*\(\)')
P13_MEMCPY_PAT   = re.compile(r'\bmemcpy\s*\([^,]+,[^,]+,\s*sizeof\s*\(')

def find_p13(lines, file_rel):
    """P13 — block-memory functions with incorrect size argument."""
    findings = []
    for i, raw in enumerate(lines, 1):
        stripped = raw.strip()
        if stripped.startswith("//"):
            continue
        line = strip_inline_comment(raw)

        # (a) bare strcpy — no size limit at all
        if P13_BARE_STRCPY.search(line):
            findings.append({
                "file": file_rel,
                "line": i,
                "pattern": "P13",
                "severity": "High",
                "description": "strcpy 無上限，目標緩衝可能被來源字串溢出",
                "snippet": raw.strip(),
                "suggested_fix": "改為 strncpy(dst, src, sizeof(dst)-1); dst[sizeof(dst)-1]=0;"
            })
            continue  # don't double-report same line

        # (b) strncpy with source .Length() as size — should use sizeof(dst)
        if P13_STRNCPY_LEN.search(line):
            findings.append({
                "file": file_rel,
                "line": i,
                "pattern": "P13",
                "severity": "High",
                "description": "strncpy size 使用來源長度 .Length()，應改用 sizeof(目標緩衝)-1",
                "snippet": raw.strip(),
                "suggested_fix": "strncpy(dst, src.c_str(), sizeof(dst)-1); dst[sizeof(dst)-1]=0;"
            })
            continue

        # (c) memcpy with sizeof(source) — may overflow if dst smaller than src
        if P13_MEMCPY_PAT.search(line):
            findings.append({
                "file": file_rel,
                "line": i,
                "pattern": "P13",
                "severity": "Medium",
                "description": "memcpy size 來自 sizeof(某型別)，請確認目標緩衝 ≥ 來源大小",
                "snippet": raw.strip(),
                "suggested_fix": "確認 sizeof(dst) >= sizeof(src)；若不確定改用 memcpy(dst, src, sizeof(dst))。"
            })
    return findings


def scan_project():
    files = walk_files(PROJECT_DIR)
    all_findings = []
    enum_members = set()

    # pass 1: collect enum symbols
    for p in files:
        txt = read_cp950(p)
        enum_members.update(collect_enum_members(txt))

    # pass 2: scan source
    for p in files:
        txt = read_cp950(p)
        lines = txt.splitlines()
        r = rel(p)
        all_findings.extend(find_p1(lines, r))
        all_findings.extend(find_p2(lines, r, enum_members))
        all_findings.extend(find_p4(lines, r))
        all_findings.extend(find_p5(lines, r))
        all_findings.extend(find_p6(lines, r))
        all_findings.extend(find_p7(lines, r))
        all_findings.extend(find_p9(lines, r))
        all_findings.extend(find_p11(lines, r))   # A
        all_findings.extend(find_p12(lines, r))   # A
        all_findings.extend(find_p13(lines, r))

    # pass 3: scan .dfm（C：只看 Form 物件本身）
    dfm_files = walk_dfm_files(PROJECT_DIR)
    for p in dfm_files:
        lines = read_cp950(p).splitlines()
        all_findings.extend(find_dfm_form_offscreen(lines, rel(p)))

    return files + dfm_files, all_findings


def count_by_pattern(findings):
    out = {f"P{i}": 0 for i in range(1, 14)}
    out["DFM"] = 0
    for f in findings:
        out[f["pattern"]] = out.get(f["pattern"], 0) + 1
    return out


def count_critical(findings):
    return sum(1 for f in findings if f["severity"] == "Critical")


def convert_md_to_html(md_path, html_path):
    """follow make-report-skill：用 md_to_html.py（honprec-blue-template）把 MD 轉 HTML。
    AI/腳本不得自行內嵌 base64 logo；HTML 一律由轉換器產生。"""
    if not os.path.exists(MD_TO_HTML):
        print(f"WARN: 找不到 md_to_html.py（{MD_TO_HTML}），略過 HTML 轉換。請手動執行 make-report-skill 轉換。")
        return False
    try:
        subprocess.run(
            [sys.executable, MD_TO_HTML, md_path, "--template", REPORT_TEMPLATE, "--out", html_path],
            check=True,
        )
        return True
    except subprocess.CalledProcessError as e:
        print(f"WARN: md_to_html.py 轉換失敗（exit {e.returncode}）；MD 已產生，請手動轉 HTML。")
        return False


def write_reports(files, findings):
    os.makedirs(REPORT_DIR, exist_ok=True)

    summary = count_by_pattern(findings)
    critical = count_critical(findings)

    meta = {
        "project": os.path.basename(PROJECT_DIR),
        "project_path": PROJECT_DIR,
        "date": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        "developer": DEVELOPER,
        "scanned_files": len(files),
        "findings": len(findings),
        "critical": critical,
        "summary": summary,
    }

    with open(SCAN_JSON, "w", encoding="utf-8") as f:
        json.dump({"meta": meta, "findings": findings}, f, ensure_ascii=False, indent=2)

    # follow make-report-skill：MD 不放 Logo；含 YAML frontmatter 供 md_to_html.py 套模板
    lines = []
    lines.append("---")
    lines.append(f"title: {VERSION_TAG} 上線前風險掃描報告")
    lines.append(f"date: {meta['date']}")
    lines.append(f"author: {meta['developer']}")
    lines.append("category: 上線前掃描")
    lines.append("---")
    lines.append("")
    lines.append(f"# {VERSION_TAG} 上線前風險掃描報告")
    lines.append("")
    lines.append("## 基本資訊")
    lines.append("")
    lines.append("| 項目 | 內容 |")
    lines.append("|------|------|")
    lines.append(f"| 專案 | {meta['project']} |")
    lines.append(f"| 專案路徑 | {meta['project_path']} |")
    lines.append(f"| 掃描時間 | {meta['date']} |")
    lines.append(f"| 開發者 | {meta['developer']} |")
    lines.append(f"| 掃描檔案數 | {meta['scanned_files']} |")
    lines.append(f"| 風險筆數 | {meta['findings']} |")
    lines.append(f"| Critical 筆數 | {meta['critical']} |")
    lines.append("")
    lines.append("## 風險摘要（P1~P13 + DFM）")
    lines.append("")
    lines.append("| Pattern | Count |")
    lines.append("|---------|-------|")
    for k in ["P1", "P2", "P3", "P4", "P5", "P6", "P7", "P9", "P11", "P12", "P13", "DFM"]:
        lines.append(f"| {k} | {summary.get(k, 0)} |")

    lines.append("")
    lines.append("## 詳細結果")
    lines.append("")
    if not findings:
        lines.append("All scanned files — no risk patterns detected. ✔")
    else:
        lines.append("| # | File | Line | Pattern | Severity | Description | Suggested Fix |")
        lines.append("|---|------|------|---------|----------|-------------|---------------|")
        for i, f in enumerate(findings, 1):
            lines.append(
                f"| {i} | {f['file']} | {f['line']} | {f['pattern']} | {f['severity']} | {f['description']} | {f['suggested_fix']} |"
            )

    lines.append("")
    lines.append("## Build 驗證")
    lines.append("")
    lines.append("此報告僅包含風險掃描結果。若需 commit 前完整驗證，請附上 BCB6 full rebuild 結果（Exit Code / Errors / Warnings）。")

    with open(REPORT_MD, "w", encoding="utf-8") as f:
        f.write("\n".join(lines))

    # follow make-report-skill：HTML 一律用 md_to_html.py（honprec-blue-template）產生，
    # 不在本腳本內嵌 base64。失敗時不阻斷流程，僅警示。
    convert_md_to_html(REPORT_MD, REPORT_HTML)

    commit_lines = []
    commit_lines.append(f"{VERSION_TAG} Pre-Commit Risk Check")
    commit_lines.append("")
    commit_lines.append(f"- Scan scope: {meta['scanned_files']} source files (.cpp/.h/.c)")
    commit_lines.append(f"- Risk findings: {meta['findings']} (Critical: {meta['critical']})")
    commit_lines.append(
        f"- Pattern summary: P1={summary.get('P1',0)}, P2={summary.get('P2',0)}, P3={summary.get('P3',0)}, "
        f"P4={summary.get('P4',0)}, P5={summary.get('P5',0)}, P6={summary.get('P6',0)}, "
        f"P7={summary.get('P7',0)}, P9={summary.get('P9',0)}, P11={summary.get('P11',0)}, "
        f"P12={summary.get('P12',0)}, P13={summary.get('P13',0)}, DFM={summary.get('DFM',0)}"
    )
    if meta['findings'] == 0:
        commit_lines.append("- Pre-release risk check PASS (no findings)")
    else:
        commit_lines.append("- Pre-release risk check WARNING (findings present, review required before commit)")
    commit_lines.append(f"- Report: {REPORT_MD}")
    commit_lines.append(f"- HTML: {REPORT_HTML}")

    with open(COMMIT_MSG, "w", encoding="utf-8") as f:
        f.write("\n".join(commit_lines))


def main():
    parser = argparse.ArgumentParser(
        description="Pre-Release 風險掃描（P1/P2/P4/P5/P6/P7/P9/P11/P12/P13 + DFM Form）"
    )
    parser.add_argument("project_dir", help="原始碼版本資料夾（由資料夾名自動抽版號）")
    parser.add_argument("--report-dir", default=None, help=f"報告輸出資料夾（預設 {DEFAULT_REPORT_DIR}）")
    parser.add_argument("--developer", default=None, help=f"開發者名稱（預設 {DEFAULT_DEVELOPER}）")
    args = parser.parse_args()

    if not os.path.isdir(args.project_dir):
        print(f"ERROR: project_dir 不存在：{args.project_dir}")
        sys.exit(2)

    configure(args.project_dir, report_dir=args.report_dir, developer=args.developer)

    files, findings = scan_project()
    write_reports(files, findings)
    print(f"VERSION={VERSION_TAG}")
    print(f"SCAN_JSON={SCAN_JSON}")
    print(f"REPORT_MD={REPORT_MD}")
    print(f"REPORT_HTML={REPORT_HTML}")
    print(f"COMMIT_MSG={COMMIT_MSG}")
    print(f"FILES={len(files)} FINDINGS={len(findings)}")


if __name__ == "__main__":
    main()
