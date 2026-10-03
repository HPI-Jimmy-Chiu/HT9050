# -*- coding: utf-8 -*-
# tools/editlist/TTLCfg.py -- gen_editlist.py 的結構設定（C 形狀：具名替身）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only TTLCfg
#
# Steven 團隊 20260925：TTLCfg（<recipe>\<DIO>.ini 或 DIOCFGPath\<DIO>.ini）—— golden TfDIOFrom
# （DIOInterFaceCFG.cpp，912，269 行）。頁面 Config.DIOInterFaceCFG.html。
# 入口是 FileRW/TTLCfg_C.cpp（FileRW/TTLCfg.cpp 已被 A 形狀 gen_formbridge 產生檔佔用；整合時退役 A 形狀、改名）。
#
# DIO 檔名＝golden FTestIF->cbDIOType->Text（Steven 20260925 指示：照 golden，不再用 TestIF_File.sDioName 代替）：
#   golden TFTestIF::InitcbDIOType（cTesterIF.cpp:70-106）列 DIOCFGPath\*.ini 當 Items；
#   golden TFTestIF::DoIniDataToForm（cTesterIF.cpp:1089-1105）依 TestIF_File.sDioName／iDioMode 選 ItemIndex → Text。
#   兩段在 FileRW/TTLCfg_C.cpp 手寫（逐行照 golden），替身 EL<TComboBox>("TFTestIF", "cbDIOType")。
#   本檔只把 GetDIOFileName 的三處 FTestIF->cbDIOType->Text 換成那個替身。
#
# golden 開表單（main.cpp:28658 sbDioSetClick → fDIOFrom->ShowModal()）：FormShow :24 → InitData :148 把欄位清空、
#   DIOFileName=""；畫面空白，直到使用者按 Load（spbLoadClick :238 → OpenDialog 選檔 → LoadData :72 → DoIniDataToForm :133）。
#   存檔 spbSaveClick :191 要 DIOFileName!="" 才動，再開 SaveDialog 讓使用者確認檔名。
#   網頁開頁＝FormShow ＋「按 Load、選本配方目前的 DIO 檔」＝GetDIOFileName()（golden 開機 main.cpp:9415、關 DIO 表單
#   main.cpp:28668、TesterIF 換型態 cTesterIF.cpp:1302 交給 LoadData 的同一個檔）。這兩行換掉的理由與 A 形狀
#   tools/formbridge/TfDIOFrom.py 相同。
#
# LoadData :81 SystemStart=false（TTL 模式且 DIO 檔不存在）：照 golden 保留（不 ELTodo）。理由：
#   (1) 方向是安全側（清掉執行旗標＝擋住開始，不會讓機台動）；(2) golden 每次關 DIO 表單（main.cpp:28669）與
#   TesterIF 換 DIO 型態（cTesterIF.cpp:1303）都跑同一支 LoadData，開頁這條路徑與它等價；
#   (3) 移植樹 TestIF_File.iTestType 預設 0＝TTL_MODE（讀檔器 ReadTestIFFile 是 GATE F-5），所以檔案不存在時一定會清。
#   A 形狀選 ELTodo（「看畫面不改機台狀態」）；兩者差別列在 FileRW/TTLCfg_C.cpp 檔頭，Steven 可改判。
import os

G = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
_cpp = open(os.path.join(G, 'DIOInterFaceCFG.cpp'), 'rb').read().decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')


def _at(gl, text):
    """行號核對：golden 第 gl 行一定要含 text（golden 改版時中止，不會默默換錯行）。"""
    if text not in _cpp[gl - 1]:
        raise SystemExit('TTLCfg.py: golden DIOInterFaceCFG.cpp:%d does not contain %r' % (gl, text))
    return gl


CB = 'EL<TComboBox>("TFTestIF", "cbDIOType")->Text'
WHY_CB = ('FTestIF->cbDIOType->Text：golden TFTestIF 的下拉。替身 EL<TComboBox>("TFTestIF","cbDIOType")，'
          '開頁時由 FileRW/TTLCfg_C.cpp 的 TI_InitcbDIOType（golden cTesterIF.cpp:70-106）＋'
          'TI_DoIniDataToForm_DIO（golden cTesterIF.cpp:1089-1105）照 golden 填好')

STRUCT = {
    'struct': 'TTLCfg',
    'prefix': 'DI',
    'class': 'TfDIOFrom',
    'cpp': 'DIOInterFaceCFG.cpp',
    'h': 'DIOInterFaceCFG.h',
    'files': ['<recipe>\\<cbDIOType>.ini（IniConfig.bI16TTLSaveInSetupFile）或 DIOCFGPath\\<cbDIOType>.ini'],
    'lists': [],
    'methods': ['TfDIOFrom', 'FormShow', 'GetDIOFileName', 'LoadData', 'DoIniDataToForm', 'InitData',
                'spbSaveClick', 'spbLoadClick', 'FormClose',
                # AI(W906-FRW-S101) 20260926: RULINGS_20260926 S101 —— Delete 鈕（golden :249-257，刪使用者在開檔對話框選的 DIO ini）。
                #   入口 FileRW/TTLCfg.cpp FileRW_TTLCfg_DeleteOp（對話框換成網頁的檔案清單，見該處）。不列進 save_methods。
                'spbDeleteClick'],
    'save_methods': ['spbSaveClick'],
    'params': {'TfDIOFrom': '', 'FormShow': '', 'spbSaveClick': '', 'spbLoadClick': '', 'FormClose': '',
               'spbDeleteClick': ''},   # AI(W906-FRW-S101) 20260926
    'rettype': {'GetDIOFileName': 'AnsiString'},
    'members': ['AnsiString DIOFileName;     // golden DIOInterFaceCFG.h:64（spbLoadClick 設，spbSaveClick 讀）',
                'bool fShow=false;          // golden DIOInterFaceCFG.h:65（本 TU 自己的；golden Command.cpp:7353 讀它，移植樹沒有讀者）',
                # AI(W906-FRW-S101) 20260926: 刪除對話框（golden OpenDialog1）的兩個結果，由 FileRW_TTLCfg_DeleteOp 填好再呼叫 DI_spbDeleteClick
                'bool W906_DeleteDialogExecuted=false;   // [W906] golden OpenDialog1->Execute() 的回傳（spbDeleteClick :252）：網頁對話框選了檔按下去',
                'AnsiString W906_DeleteDialogFileName;   // [W906] golden OpenDialog1->FileName（:254）：網頁選的檔的完整路徑（DIOCFGPath＋檔名）'],
    'replace': [
        # ---- FormShow
        ('FormShow', _at(26, 'OpenDialog1->InitialDir=DIOCFGPath;'), 26,
         'OpenDialog1->InitialDir：開檔對話框的起始資料夾（伺服器端沒有對話框；Load 改成選本配方目前的 DIO 檔，見 spbLoadClick）', ';'),
        ('FormShow', _at(28, 'Top=30;'), _at(29, 'Left=200;'), 'Top／Left：視窗位置（HTML 不用）', ';'),
        # ---- GetDIOFileName：三處 FTestIF->cbDIOType->Text
        ('GetDIOFileName', _at(55, 'FTestIF->cbDIOType->Text'), 55, WHY_CB,
         r'S1.sprintf("%s%s\\%s.ini", DataPath, S, ' + CB + ');'),
        ('GetDIOFileName', _at(58, 'FTestIF->cbDIOType->Text'), 58, WHY_CB,
         r'szDir.sprintf("%s%s.ini", DIOCFGPath, ' + CB + ');'),
        ('GetDIOFileName', _at(67, 'FTestIF->cbDIOType->Text'), 67, WHY_CB,
         r'S1.sprintf("%s%s.ini", DIOCFGPath, ' + CB + ');'),
        # ---- spbSaveClick
        ('spbSaveClick', _at(197, 'Close();'), 197,
         'Close()：golden A02 權限不足時關表單 → 伺服器端記 closed（_EditPage 讓下次存檔前重新開頁），頁面自己關',
         'filerw::ELMark("closed");'),
        ('spbSaveClick', _at(204, 'SaveDialog1->InitialDir=DIOCFGPath;'), 204, 'SaveDialog1->InitialDir：存檔對話框的起始資料夾（伺服器端沒有對話框）', ';'),
        ('spbSaveClick', _at(205, 'SaveDialog1->FileName=DIOFileName;'), 205,
         'SaveDialog1->FileName：存檔對話框的預設檔名＝DIOFileName（:222 再從對話框取回，見下）', ';'),
        ('spbSaveClick', _at(207, 'if(SaveDialog1->Execute())'), 207,
         'SaveDialog1->Execute()：視同使用者接受預設檔名按「存檔」（另存別的檔名／位置沒有模擬 —— 一律寫開頁載入的那個 DIO 檔）',
         'if(true)'),
        ('spbSaveClick', _at(222, 'DIOFileName=SaveDialog1->FileName;'), 222,
         'DIOFileName=SaveDialog1->FileName：對話框預設檔名就是 DIOFileName（:205），值不變；這裡記 savedMark（下面 11 個 WriteIniData 一定會跑）',
         'filerw::ELMark("DIOFileWritten");'),
        # ---- spbDeleteClick   AI(W906-FRW-S101) 20260926
        ('spbDeleteClick', _at(251, 'OpenDialog1->Title='), 251,
         'OpenDialog1->Title：對話框標題（網頁的刪除清單用 golden 字樣 "Select file to delete"）', ';'),
        ('spbDeleteClick', _at(252, 'if(OpenDialog1->Execute())'), 252,
         'OpenDialog1->Execute()：網頁的刪除對話框（FileRW_TTLCfg_DeleteOp，列 DIOCFGPath\\*.ini）選了檔按下去',
         'if(W906_DeleteDialogExecuted)'),
        ('spbDeleteClick', _at(254, 'DeleteFile(OpenDialog1->FileName);'), 254,
         'OpenDialog1->FileName：網頁選的檔（入口只收 DIOCFGPath 底下存在的 *.ini —— golden 的對話框可以換資料夾，網頁版不開放，見 FileRW/TTLCfg.cpp）',
         'DeleteFile(W906_DeleteDialogFileName);'),
        # ---- spbLoadClick
        ('spbLoadClick', _at(240, 'OpenDialog1->Title='), 240, 'OpenDialog1->Title：開檔對話框標題', ';'),
        ('spbLoadClick', _at(241, 'if(OpenDialog1->Execute())'), 241,
         'OpenDialog1->Execute()：網頁開頁視同使用者按 Load 並在對話框選了檔（見下一條）', 'if(true)'),
        ('spbLoadClick', _at(243, 'DIOFileName=OpenDialog1->FileName;'), 243,
         'DIOFileName=OpenDialog1->FileName：選的檔＝本配方目前的 DIO 檔 GetDIOFileName()（golden main.cpp:9415／:28668、'
         'cTesterIF.cpp:1302 交給 LoadData 的同一個檔）',
         'DIOFileName=DI_GetDIOFileName();'),
    ],
    'blocks': [],
    'includes': ['cprod.h', 'Config.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h'],
    # 產生器的 includes 一律寫成 #include "…"；系統標頭放 decls（原樣輸出）
    'decls': ['#include <windows.h>   // CopyFile（golden GetDIOFileName :61）；FileRW/TTLCfg_C.cpp 的 FindFirstFile',
              '#include <cstring>     // strncpy（golden LoadData :110）',
              '#include <cstdlib>     // atoi（golden spbSaveClick :211）'],
    'overrides': [],
}
