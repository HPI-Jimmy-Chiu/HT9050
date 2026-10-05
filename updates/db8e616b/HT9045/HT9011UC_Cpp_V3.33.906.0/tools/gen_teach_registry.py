# -*- coding: utf-8 -*-
# =============================================================================
#  tools/gen_teach_registry.py  --  AI(W906-TEACH-W1) 20260919
#
#  產生 forms/fTeachRegistry.cpp 的登錄表區段，來源是 golden
#  uteach.cpp:390-995（TfTeach ctor 裡的 TechPara / TechTwoPara /
#  TechMotorAxle 登錄）。
#
#  ⚠ 為什麼用產生的而不是手打：那是 440 個 push_back。手打一次就是 440 次
#    出錯機會，而且沒有人能逐行覆核。產生的版本是 golden 的**可重跑變換**：
#    golden 變了就重跑，diff 會說話。
#
#  ⚠ 這支**只讀** golden（`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618`），
#    絕不寫它。golden 是 SVN 工作副本（memory: golden-tree-is-an-svn-working-copy）。
#
#  做了什麼變換（每一項都刻意、都在輸出裡標註）
#  ---------------------------------------------------------------------------
#  T1  widget 引數 -> `/*原識別字*/0`
#      移植樹的 TfTeach **沒有 widget 實例**（forms/fTeach.h 的既定政策：
#      golden ctor :267-995 的 widget 佈局整段 DEFERRED）。golden 的 TECH_PARA
#      只在兩個地方碰 widget：ctor 的 `SetEdit->Visible=Visible` 與
#      ReadFromFile 尾端的 `SetEdit->Text=(int)(*Parameter)` —— **兩個都是純 UI**，
#      讀取路徑（Parameter / MotorSelect / Key）完全不經過它們。
#      ⇒ 引數位置保持一模一樣、原識別字保留在註解裡，所以這份輸出仍然可以
#         跟 golden 逐行機械比對。fTeachPara.cpp 的那兩個 deref 加 null 檢查，
#         並在那裡寫明這是本波**唯一**的一處偏離。
#      ⓘ 順帶：golden 的 Key 字串幾乎總是等於那個 widget 的識別字
#         （"setEditInZSafeHeight" vs setEditInZSafeHeight），所以 widget 指標
#         對讀取路徑不帶任何 Key 沒有的資訊。
#
#  T2  純 widget 敘述（`X->Caption=`、`X->Visible=`、`X->Left=` …）-> 整行註解掉，
#      前面加 `// [W906-TEACH-W1 T2 widget-only]`。
#
#  T3  控制流、註解、空白行、golden 的 /* */ 死碼 -> **原樣保留**。
#
#  輸出每 25 行插一個 `// golden :NNNN` 錨點，方便日後對帳。
#
#  E1  AI(W906-TEACH-3AXES) 20261001：W906 擴充列（**不是 golden**）—— EXT_ROWS。
#      EastSun 1001「三軸都幫我加 在合適的地方」：MOutShuttle1／MOutShuttle2／MCCDY 在 HT9050 是 Enable=1 的
#      PCIE-1203 軸，golden 沒有它們的教導點。這幾列插在 golden :993（`TECH_MAX_ITEM=TechPara.size();`）
#      之前、包在 `if(EXT_COND)` 裡：
#        * 放最後 ⇒ golden 每一列的 TechPara 索引（＝ golden FormShow 給按鈕的 Tag）都不動；
#        * 條件 ⇒ 只有 PCI1203_IO 機台登錄，別台機台的 teach.ini 不會被 CheckAndReadIniData 補鍵。
#      tools/gen_teach_editlist.py import 同一份 EXT_ROWS：golden 前綴照舊逐列比對 golden，尾端逐列比對 EXT_ROWS
#      （按鈕名必須不是 golden .dfm 的名字、處理函式明確指定）。改這裡就要重跑兩支產生器。
# =============================================================================
import io
import os
import re
import sys

GOLD = r"D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618\uteach.cpp"
START, END = 342, 994          # 1-based，含
#   ⚠ 止於 :994 不是 :995 —— golden :995 是 ctor 自己的收尾 `}`，
#     而產生的本體是要塞進 BuildTechRegistry() 的殼裡，殼自己有一個。
#   :342-388 是 TechSuckPara 的 2x8 迴圈填值（golden 用迴圈不是 push_back），
#   :390-995 是 265+52+121 個 push_back。兩段都屬於登錄表，一起產生。

# TECH_* 建構子裡哪幾個引數是 widget（0-based）
WIDGET_ARGS = {
    "TECH_PARA":       (2, 4, 5),          # EdtSet, SPBFun, SPBGo
    "TECH_PARA_DOUBLE": (2, 4, 5),
    "TECH_TWOPARA":    (4, 5, 8, 9),       # Edt1, Edt2, SPB1, SPB2
    "TECH_MotorAxle":  (1,),               # SPB
}

# E1：W906 擴充列（見檔頭 E1）。欄位：Tech 欄位、馬達列舉名、畫面欄位（＝ teach.ini 鍵）、Set 鈕、Go 鈕、處理函式、說明。
#   * 畫面欄位：OutShuttle 四個是 golden uteach.dfm Panel28 的保留隱藏欄位（:4349-4400），CCDY 兩個是新名字。
#   * 按鈕：新名字（golden .dfm 沒有），不可以用 Motor 開頭（網頁把 `Motor*` 的 TSpeedButton 當軸選取鈕）。
#   * 處理函式：golden SetButton140Click／GoButton140Click（單軸 TECH_PARA 的 Set／Go，讀 TechPara[Tag]）。
#   * teach.ini 區段＝MOT[馬達].Alias（TECH_PARA::ReadFromFile）⇒ [MOutShuttle1]／[MOutShuttle2]／[MCCDY]。
EXT_TAG = "AI(W906-TEACH-3AXES) 20261001"
EXT_COND = "IO_CARD_TYPE==PCI1203_IO"
EXT_ROWS = [
    # Tech 欄位              馬達            畫面欄位／鍵             Set 鈕                 Go 鈕                說明
    ("iOutShuttle1Left",  "MOutShuttle1", "setEditOutSht1Left",  "btnSetOutSht1Left",  "btnGoOutSht1Left",  "Out Shuttle 1 left  (Prod.OutSHT[0].iLeft,  cinitial.cpp:6224)"),
    ("iOutShuttle1Right", "MOutShuttle1", "setEditOutSht1Right", "btnSetOutSht1Right", "btnGoOutSht1Right", "Out Shuttle 1 right (Prod.OutSHT[0].iRight, cinitial.cpp:6222)"),
    ("iOutShuttle2Left",  "MOutShuttle2", "setEditOutSht2Left",  "btnSetOutSht2Left",  "btnGoOutSht2Left",  "Out Shuttle 2 left  (Prod.OutSHT[1].iLeft,  cinitial.cpp:6225)"),
    ("iOutShuttle2Right", "MOutShuttle2", "setEditOutSht2Right", "btnSetOutSht2Right", "btnGoOutSht2Right", "Out Shuttle 2 right (Prod.OutSHT[1].iRight, cinitial.cpp:6223)"),
    ("iCCDYSite1x1",      "MCCDY",        "setEditCCDYSite1x1",  "btnSetCCDYSite1x1",  "btnGoCCDYSite1x1",  "CCD Y inspection position (FinePitch Tech.iCCDYSite1x1, LastSet.h:978)"),
    ("iCalY1x1",          "MCCDY",        "setEditCCDYCal1x1",   "btnSetCCDYCal1x1",   "btnGoCCDYCal1x1",   "CCD Y calibration position (FinePitch Tech.iCalY1x1, LastSet.h:982)"),
    # AI(W906-TEACH-INDEXZ-OUTSHT) 20261005: EastSun「9050 inshuttle 和outshuttle 高度不一樣 所以index arm Z 高度需要有兩個」——
    #   Index Z1 放 IC 到 Out Shuttle 的高度；存在 golden 宣告但從沒用過的 Tech.iHT9040TestZ1_PlaceSH2（LastSet.h:708，TECH 大小不變）。
    ("iHT9040TestZ1_PlaceSH2", "MTestZ1", "setEditIndex1ToOutSht1Z", "btnSetIndex1OutShtZ", "btnGoIndex1OutShtZ", "Index Z1 place on Out Shuttle 1 (HT9050 Prod.TestZ1_Place, cinitial.cpp)"),
]
EXT_SET_HANDLER = "SetButton140Click"
EXT_GO_HANDLER = "GoButton140Click"
EXT_BEFORE_GOLDEN_LINE = 993           # golden :993 `TECH_MAX_ITEM=TechPara.size();` —— 擴充列插在它前面


def ext_block():
    """E1：擴充列的 C++ 原始碼（行的串列）。註解裡不可以出現 push_back 呼叫的字樣（gen_teach_editlist.py 用正則數它）。"""
    out = []
    out.append("    //%s: EastSun 1001「三軸都幫我加 在合適的地方」—— W906 擴充，不是 golden（tools/gen_teach_registry.py EXT_ROWS）。" % EXT_TAG)
    out.append("    //  MOutShuttle1／MOutShuttle2／MCCDY 在 HT9050 是 Enable=1 的 PCIE-1203 軸，golden 沒有它們的教導點：")
    out.append("    //  * golden 引擎讀 Tech.iOutShuttle1Left／1Right／2Left／2Right（cinitial.cpp:6222-6225 → Prod.OutSHT[0／1].iLeft／iRight），")
    out.append("    //    以前從沒從 teach.ini 讀過（＝0）；golden uteach.dfm Panel28 有四個保留的隱藏欄位 setEditOutSht*（:4349-4400），沒有 Set／Go 鈕。")
    out.append("    //  * MCCDY 沒有 golden 點位；用 FinePitch 已有的 Tech.iCCDYSite1x1（檢查位）／Tech.iCalY1x1（校正位）（LastSet.h:978／:982）。")
    out.append("    //  * 放在 golden 登錄表最後 ⇒ golden 每一列的 TechPara 索引（按鈕 Tag）都不動；只在 PCI1203_IO 機台登錄 ⇒ 別台的 teach.ini 不多鍵。")
    out.append("    //  * 第一次開機 ReadFromFile 的 CheckAndReadIniData 會把這 6 個鍵補寫進 teach.ini（值＝目前的 Tech 值，通常 0）—— golden 行為。")
    out.append("    if(%s)" % EXT_COND)
    out.append("    {")
    for fld, mot, edit, setb, gob, why in EXT_ROWS:
        out.append("        TechPara.push_back(new TECH_PARA(%s, %s, %s, %s, %s, %s));  // %s" % (
            ("&Tech." + fld).ljust(36), mot.ljust(14), ("/*%s*/0" % edit).ljust(26),
            ('"%s"' % edit).ljust(22), ("/*%s*/0" % setb).ljust(25), "/*%s*/0" % gob, why))
    out.append("    }")
    out.append("")
    return out


WIDGET_ONLY = re.compile(
    r"^\s*[A-Za-z_][A-Za-z0-9_\[\]]*\s*->\s*"
    r"(Caption|Visible|Left|Top|Width|Height|Enabled|Text|Picture|Color|Font)\b")

# T4：`X.SetEdit[..] = <widget>;` 這種**成員賦值**。TechSuckPara 的 2x8 迴圈用的是
# 賦值，不是建構子引數，所以 T1 攔不到。
# ⚠ 不可以整行註解掉 —— TECH_SUCKPARA 的 SetEdit 陣列沒有任何其他地方會寫它，
#   註解掉就變成未初始化指標，而 ReadFromFile 會去比它 `!=NULL`。
#   必須**賦 0**，原識別字留在註解裡。
SETEDIT_ASSIGN = re.compile(
    r"^(\s*[A-Za-z_][A-Za-z0-9_\[\]\.]*\.SetEdit\s*\[[^=]*\]\s*=)\s*([^;]+);(.*)$")


def split_args(s):
    """在頂層逗號切開引數（括號/方括號/字串內的逗號不算）。"""
    out, depth, cur, instr = [], 0, "", False
    i = 0
    while i < len(s):
        c = s[i]
        if instr:
            cur += c
            if c == "\\" and i + 1 < len(s):
                cur += s[i + 1]
                i += 2
                continue
            if c == '"':
                instr = False
        else:
            if c == '"':
                instr = True
                cur += c
            elif c in "([":
                depth += 1
                cur += c
            elif c in ")]":
                depth -= 1
                cur += c
            elif c == "," and depth == 0:
                out.append(cur)
                cur = ""
            else:
                cur += c
        i += 1
    out.append(cur)
    return out


def neutralise(line, collect=False):
    """T1：把 widget 引數換成 /*名字*/0，其餘一字不動。

    collect=True 時回傳 (line, hit, [被認定為 widget 的識別字...])，
    供第一趟建立 widget 名稱集合用。
    """
    names = []
    m = re.search(r"\bnew\s+(TECH_PARA_DOUBLE|TECH_PARA|TECH_TWOPARA|TECH_MotorAxle)\s*\(",
                  line)
    if not m:
        return (line, 0, names) if collect else (line, 0)
    kind = m.group(1)
    open_at = m.end() - 1
    depth, close_at = 0, None
    for i in range(open_at, len(line)):
        if line[i] == "(":
            depth += 1
        elif line[i] == ")":
            depth -= 1
            if depth == 0:
                close_at = i
                break
    # ⚠ v3：建構子呼叫可能**跨行**（golden 有一筆的可見性條件式佔了 3 行）。
    # 找不到結尾括號時不要整個放棄 —— 把這一行剩下的當引數串處理，
    # 把出現在這一行的 widget 位置中性化。v2 在這裡 return，結果
    # `btnLoaderY` 原封不動留下來，第二次建置就死在那一個符號上。
    partial = close_at is None
    if partial:
        inner = line[open_at + 1:]
        tail = ""
    else:
        inner = line[open_at + 1:close_at]
        tail = line[close_at:]
    args = split_args(inner)
    hit = 0
    for idx in WIDGET_ARGS[kind]:
        if idx >= len(args):
            continue
        raw = args[idx]
        name = raw.strip()
        if name in ("0", "NULL", "nullptr", "") or name.startswith("/*"):
            continue
        lead = raw[:len(raw) - len(raw.lstrip())]
        trail = raw[len(raw.rstrip()):]
        # 保留原本的欄寬，讓輸出跟 golden 對齊得起來
        repl = "/*%s*/0" % name
        pad = " " * max(0, len(raw) - len(lead) - len(repl) - len(trail))
        args[idx] = lead + repl + pad + trail
        names.append(name)
        hit += 1
    out = line[:open_at + 1] + ",".join(args) + tail
    return (out, hit, names) if collect else (out, hit)


def in_block_comment_map(seg):
    """回傳每一行「是否整行落在 golden 自己的 /* */ 區塊註解裡」。

    ⚠ 這是 v2 最重要的修正。v1 把 `/*名字*/0` 插進 golden 的註解區塊裡，
      巢狀 `/*` 不合法 -> 外層註解在第一個 `*/` 提早結束 -> 被註解掉的
      程式碼突然變活的 -> `MLoadRobotZ was not declared`、`#else without #if`。
      第一次建置就是被這個擋下來的。
    """
    out, depth = [], 0
    for line in seg:
        started_inside = depth > 0
        j, n = 0, len(line)
        while j < n:
            if depth == 0 and line.startswith("//", j):
                break                      # 行註解，後面不影響區塊狀態
            if depth == 0 and line.startswith("/*", j):
                depth += 1
                j += 2
                continue
            if depth > 0 and line.startswith("*/", j):
                depth -= 1
                j += 2
                continue
            j += 1
        # 只要這一行「開頭就在註解內」或「這一行自己開了一個沒關的註解」，
        # 都當成不可變換 —— 寧可少變換，也不要製造巢狀註解。
        out.append(started_inside or depth > 0)
    return out


def main():
    out_path = sys.argv[1] if len(sys.argv) > 1 else None
    d = io.open(GOLD, encoding="cp950", errors="replace").read().splitlines()
    seg = d[START - 1:END]
    incomment = in_block_comment_map(seg)

    # ---- 第一趟：收集所有被中性化的 widget 識別字 ------------------------
    widgets = set()
    for i, raw in enumerate(seg):
        if incomment[i]:
            continue
        line = raw.rstrip()
        m4 = SETEDIT_ASSIGN.match(line)
        if m4:
            widgets.add(m4.group(2).strip().split("[")[0])
        _, _, names = neutralise(line, collect=True)
        for nm in names:
            widgets.add(nm.split("[")[0])

    # ---- 第二趟：輸出 ------------------------------------------------------
    body = []
    n_widget_args = n_widget_only = n_push = n_setedit = n_incomment = 0
    n_ext = 0
    for i, raw in enumerate(seg):
        gl = START + i
        line = raw.rstrip()

        if gl == EXT_BEFORE_GOLDEN_LINE:                # E1：W906 擴充列（AI(W906-TEACH-3AXES) 20261001）
            assert line.strip() == "TECH_MAX_ITEM=TechPara.size();" and not incomment[i], ("E1 anchor moved", gl, line)
            body.extend(ext_block())
            n_ext = len(EXT_ROWS)

        if incomment[i]:
            # golden 自己註解掉的東西：原樣搬過來，一個字不動。
            n_incomment += 1
            if (i % 25) == 0:
                body.append("    // golden :%d" % gl)
            body.append(line)
            continue

        base = line.strip().split("->")[0].strip() if "->" in line else ""
        base = base.split("[")[0]
        if base and base in widgets and "=" in line and "push_back" not in line:
            body.append("    // [W906-TEACH-W1 T2 widget-only] golden :%d" % gl)
            body.append("    //%s" % line)
            n_widget_only += 1
            continue
        if WIDGET_ONLY.match(line):
            body.append("    // [W906-TEACH-W1 T2 widget-only] golden :%d" % gl)
            body.append("    //%s" % line)
            n_widget_only += 1
            continue

        m4 = SETEDIT_ASSIGN.match(line)
        if m4:
            lhs, rhs, tail = m4.group(1), m4.group(2).strip(), m4.group(3)
            pad = " " * max(0, len(m4.group(2)) - len(rhs))
            line = "%s/*%s*/0;%s%s" % (lhs, rhs, pad, tail)
            n_setedit += 1
        new, hit = neutralise(line)
        n_widget_args += hit
        if "push_back" in new:
            n_push += 1
        if (i % 25) == 0:
            body.append("    // golden :%d" % gl)
        body.append(new)

    text = "\n".join(body) + "\n"
    print("golden :%d-%d  共 %d 行" % (START, END, len(seg)))
    print("  push_back            %d" % n_push)
    print("  被中性化的 widget 引數 %d" % n_widget_args)
    print("  整行註解的純 widget 敘述 %d" % n_widget_only)
    print("  .SetEdit 成員賦值改成 0    %d" % n_setedit)
    print("  golden 自己的註解區塊（原樣搬） %d 行" % n_incomment)
    print("  收集到的 widget 識別字      %d" % len(widgets))
    print("  E1 W906 擴充列（不是 golden，golden :%d 之前） %d" % (EXT_BEFORE_GOLDEN_LINE, n_ext))
    assert n_ext == len(EXT_ROWS), "E1 anchor line not reached"
    if out_path:
        io.open(out_path, "w", encoding="utf-8", newline="\n").write(text)
        print("  寫到 %s（%d 行）" % (out_path, text.count("\n")))
    else:
        print("\n（沒給輸出路徑，只做統計）")


if __name__ == "__main__":
    main()
