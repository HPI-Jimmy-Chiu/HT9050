# 產生 JSON/View-rules.json：background 各視窗／各頁區塊「是否顯示」的規則（依 General-config / Config 判斷）
#   path 語法（settings.js HTSettings.get）：general.quick.<key> | general.sections.<Section>.<Key> | config.sections.<Section>.<Key>
#   op：truthy | falsy | eq | ne | gt | lt | in | exists ；onFalse：hide（不建立視窗、選單標灰）| disable（建立但選單不可點）
#   對照 BCB6：各 Click 事件前的 if(USE_xxx) / IniConfig.bXXX 判斷；未列出的視窗一律顯示
import json, os
from datetime import datetime

OUT = r"D:\HT9045\JSON\View-rules.json"
NOW = datetime.now().astimezone().isoformat(timespec="seconds")

W = {}
def rule(win, when, note, on_false="hide", any_=False):
    r = {"when": when, "onFalse": on_false, "note": note}
    if any_:
        r["any"] = True
    W[win] = r
def c(path, op="truthy", value=None):
    d = {"path": path, "op": op}
    if value is not None:
        d["value"] = value
    return d

# ---- Gerneral.ini（機台選配；HandlerSys）----
rule("sckart",    [c("general.quick.useAutoRetest")],                        "USE_AUTO_RETEST（Auto Retest 視窗）")
rule("barcode",   [c("general.sections.2D_BarCode.BAR_CODE_INSTALL")],       "BAR_CODE_INSTALL>0（Bar Code 視窗）")
rule("groundman", [c("general.sections.Ground_Man.USE_GROUND_MAN")],         "USE_GROUND_MAN")
rule("cclink",    [c("general.sections.SHUTTLE_SENSOR.NCUL1_NO", "gt", 0)],  "Shuttle Sensor Utility：NCUL1_NO>0")
rule("omron",     [c("general.sections.TempCtrl.TEMPCTRL_TYPE", "exists")],  "TEMPCTRL_TYPE（Omron 溫控）", on_false="disable")
rule("trayassign",[c("general.quick.useCatchTrayModel", "exists")],           "USE_CATCH_TRAY_MODEL（Tray Assignment）", on_false="disable")
rule("hotplate",  [c("general.sections.System.USE_HOTPLATE_TYPE", "exists")], "USE_HOTPLATE_TYPE", on_false="disable")
rule("motionview",[c("general.quick.motionCardType", "exists")],              "MOTION_CARD_TYPE 存在才有動作畫面", on_false="disable")
rule("motorview", [c("general.quick.motionCardType", "exists")],              "MOTION_CARD_TYPE", on_false="disable")
rule("home",      [c("general.quick.motionCardType", "exists")],              "Home Monitor 需馬達卡", on_false="disable")
rule("teach",     [c("general.quick.motionCardType", "exists")],              "Teaching 需馬達卡", on_false="disable")
rule("motortest", [c("general.quick.motionCardType", "exists")],              "Motor Test 需馬達卡", on_false="disable")
rule("io",        [c("general.quick.ioCardType", "exists")],                  "IO_CARD_TYPE 存在才有 IO 畫面", on_false="disable")
rule("aoainfo",   [c("general.sections.System.MACHINE_HAS_AUTO_ALIGNMENT_CCD")], "MACHINE_HAS_AUTO_ALIGNMENT_CCD（AOA Info）")
rule("shot_FrmRotate", [c("general.sections.ROTATE_KIT.USE_ROTATE_KIT")],    "USE_ROTATE_KIT（Rotate 截圖頁）")

# ---- config.ini（功能開關；cConfiguration）----
rule("qamode",    [c("config.sections.QA Mode.Enable QA Mode")],              "config [QA Mode] Enable QA Mode", on_false="disable")
rule("cleaning",  [c("general.sections.System.USE_AutoCleanIonFan"), c("config.sections.Auto Clean.bAutoCleanShuttleDisable", "exists")],
                  "Auto Clean：任一相關鍵存在即顯示", on_false="disable", any_=True)
rule("yieldmon",  [c("config.sections.Function.bEnableYieldRecord", "exists")], "Yield Monitoring（config 有 bEnableYieldRecord 鍵即可開）", on_false="disable")

# ---- 頁面內區塊（給各頁自行呼叫 HTSettings.decide('<page>:<block>')）----
BLOCKS = {
    "motionview:OutSort":   {"when": [c("general.sections.OutSortArm.USE_OUT_SORT_ARM")], "onFalse": "hide",
                             "note": "pnlSortArmX / pnlSortArmY / palOutSht / ledSortArm*：USE_OUT_SORT_ARM"},
    "motionview:Fix3":      {"when": [c("general.quick.fix3Install")], "onFalse": "hide", "note": "Fix3 托盤區：FIX3_INSTALL"},
    "motionview:OutShuttle":{"when": [c("general.quick.useOutShtMot")], "onFalse": "dim", "note": "Out Shuttle 馬達：USE_OUT_SHT_MOT"},
    "teach:tsXPitch40":     {"when": [c("general.sections.System.USE_IN_OUT_ARM_X_PITCH")], "onFalse": "hide", "note": "X Pitch 頁：USE_IN_OUT_ARM_X_PITCH"},
    "teach:tsXPitch120":    {"when": [c("general.sections.System.USE_IN_OUT_ARM_X_PITCH")], "onFalse": "hide", "note": "X Pitch 頁"},
    "teach:tsYPitch15":     {"when": [c("general.quick.useInOutArmYPitch")], "onFalse": "hide", "note": "Y Pitch 頁：USE_IN_OUT_ARM_Y_PITCH"},
    "teach:tsYPitch60":     {"when": [c("general.quick.useInOutArmYPitch")], "onFalse": "hide", "note": "Y Pitch 頁"},
    "teach:tsTrayArm":      {"when": [c("general.quick.useCatchTrayModel", "exists")], "onFalse": "hide", "note": "Tray Arm 頁：USE_CATCH_TRAY_MODEL"},
    "teach:tsHinge":        {"when": [c("general.sections.System.USE_LOADER_HINGE")], "onFalse": "hide", "note": "Hinge 頁：USE_LOADER_HINGE"},
    "ioview:Fix3":          {"when": [c("general.quick.fix3Install")], "onFalse": "dim", "note": "IO 表 Fix3 群組"},
    "ioview:OutSort":       {"when": [c("general.sections.OutSortArm.USE_OUT_SORT_ARM")], "onFalse": "dim", "note": "IO 表 OutSort 群組"},
}

doc = {
    "schemaVersion": "1.0.0", "generatedAt": NOW,
    "source": {"toolchain": "BCB6", "note": "規則對照 main.cpp / cinitial.cpp 各 USE_xxx 與 IniConfig 判斷；由 _gen_view_rules.py 產生，改規則請改產生器"},
    "pathSyntax": "general.quick.<k> | general.sections.<Sec>.<Key> | config.sections.<Sec>.<Key>（sections 下自動取 .value）",
    "ops": ["truthy", "falsy", "eq", "ne", "gt", "lt", "in", "exists"],
    "onFalse": {"hide": "視窗不顯示、工作列/選單項標灰不可開", "disable": "視窗可建立但選單項不可點（顯示原因）", "dim": "頁內區塊變淡（保留版面）"},
    "windows": W,
    "blocks": BLOCKS,
}
json.dump(doc, open(OUT, "w", encoding="utf-8"), ensure_ascii=False, indent=1)
print("ok:", OUT, "windows=%d blocks=%d" % (len(W), len(BLOCKS)))
