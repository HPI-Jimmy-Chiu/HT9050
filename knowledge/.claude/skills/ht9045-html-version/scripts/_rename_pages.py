# 頁面檔名重新命名（2026-09-02；可重跑，依目前檔名重新分類）
#   規則：① 去掉 dfm 表單前綴字母（c/u/f）：fMain.X → Main.X、cBinSel → BinSel、uhome → home
#         ② 分類前綴（以「去前綴後的基底名」查表）：
#            Setup.  工作檔範疇（存取 D:\HT9045\IniData\Data，參 _scan_datapath.py）與機台設定
#            Data.   生產數據顯示；  Status. 狀態顯示；  HW. 硬體直接操作（馬達/IO/溫控/感測器）
#            IDE.    開發輔助工具頁（元件對照/多語系編輯/CSS 規範/元件模板）
#            不在任一集合 → 無前綴
#   同時替換所有引用（html/js/py/md），用法：python _rename_pages.py [--apply]
import os, re, sys, shutil

PAGE = r"D:\HT9045\page"
PREFIXES = ("Setup.", "Data.", "Status.", "HW.", "IDE.")
# 以「基底名」（去 dfm 前綴字母、去分類前綴）分類；2026-09-02 下午依使用者重新分類
SETUP = {"OffSet", "Speed", "Configuration", "QAMode", "BarCode", "Cleaning",
         "Contact", "TesterIF", "Ld_ULd", "TrayForm", "SCK_ART", "YieldMonitoring", "HotPlate", "SetUp",
         "Temp_Set", "BinSel", "TrayAssignment", "BinSelNormal", "DIOInterFaceCFG"}
DATA = {"SortCT", "ContactCT", "TestCategory", "StartCondition", "LotInfo", "Observer", "SmartDiagnostic",
        "CounterClear", "Builder"}
STATUS = {"ShowMessage", "LtcSensor", "ShowBinSelect", "GroundMan", "TowerLight", "TemperFrom",
          "CounterSel", "Security"}
HW = {"OmronEJ1N", "MyCCLinkSensor", "MotorTest", "home", "IoSetView", "teach", "HandlerSys"}
IDE = {"ComponentMap", "I18nEditor", "StyleGuide", "WidgetTemplates"}
KEEP = {"ScreenShots"}

def strip_prefix(b):
    for p in PREFIXES:                       # 先去現有分類前綴（可重跑）
        if b.startswith(p):
            b = b[len(p):]
    if b == "main":                          # 主畫面：只大寫化，不加分類前綴
        return "Main"
    if b.startswith("fMain."):
        return "Main." + b[6:]
    if b in ("uhome", "uteach"):
        return b[1:]
    if re.match(r"^[cuf][A-Z]", b):
        return b[1:]
    return b

RENAME = {}
for fn in sorted(os.listdir(PAGE)):
    if not fn.endswith(".html"):
        continue
    b = fn[:-5]
    if b in KEEP:
        continue
    nb = strip_prefix(b)
    if nb in SETUP: nb = "Setup." + nb
    elif nb in DATA: nb = "Data." + nb
    elif nb in STATUS: nb = "Status." + nb
    elif nb in HW: nb = "HW." + nb
    elif nb in IDE: nb = "IDE." + nb
    if nb != b:
        RENAME[b + ".html"] = nb + ".html"

# 引用替換範圍
TARGETS = []
for d, pats in [
    (r"D:\HT9045", ("*.html",)),
    (r"D:\HT9045\page", ("*.html", "*.js")),
    (r"D:\HT9045\page\shot", ("*.html",)),
    (r"D:\AI_TempFile", ("_gen_*.py", "_scan_*.py", "_apply_theme_links.py")),
    (r"D:\HT9045\.github\skills\ht9045-html-version", ("SKILL.md",)),
    (r"D:\HT9045\.github\skills\ht9045-html-version\references", ("*.md",)),
    (r"D:\HT9045\.github\skills\ht9045-html-version\scripts", ("*.py",)),
    (r"D:\HT9045\docs\ops\daily", ("*.md",)),
    (r"D:\docs\ops\daily", ("20260902.md",)),
]:
    if not os.path.isdir(d):
        continue
    import fnmatch
    for fn in os.listdir(d):
        if any(fnmatch.fnmatch(fn, p) for p in pats):
            TARGETS.append(os.path.join(d, fn))

# 一次性 alternation，避免鏈式替換（cSetUp→Setup.SetUp 後再被別的規則吃掉）
keys = sorted(RENAME, key=len, reverse=True)
RX = re.compile(r"(?<![\w.])(" + "|".join(re.escape(k) for k in keys) + r")")

apply = "--apply" in sys.argv
print("RENAME MAP (%d):" % len(RENAME))
for k in keys:
    print("  %-28s -> %s" % (k, RENAME[k]))

total = 0
for p in TARGETS:
    try:
        s = open(p, encoding="utf-8").read()
    except UnicodeDecodeError:
        s = open(p, encoding="cp950", errors="replace").read()
    n = len(RX.findall(s))
    if not n:
        continue
    total += n
    print("  %4d  %s" % (n, p))
    if apply:
        s2 = RX.sub(lambda m: RENAME[m.group(1)], s)
        open(p, "w", encoding="utf-8").write(s2)
print("total refs:", total)

if apply:
    for old, new in RENAME.items():
        a, b = os.path.join(PAGE, old), os.path.join(PAGE, new)
        if os.path.exists(a):
            shutil.move(a, b)
            print("moved", old, "->", new)
    print("done")
else:
    print("(dry run; add --apply)")
