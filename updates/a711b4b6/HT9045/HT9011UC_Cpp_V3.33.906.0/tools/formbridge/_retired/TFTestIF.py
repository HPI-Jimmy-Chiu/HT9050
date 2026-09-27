# -*- coding: utf-8 -*-
# tools/formbridge/TFTestIF.py -- gen_formbridge.py 的表單設定（一個 BCB 表單一個檔）。
# 欄位說明見 tools/formbridge/README.md。改完跑：python tools/gen_formbridge.py [--only TFTestIF]
FORM = {
    'class': 'TFTestIF',
    'cpp': 'cTesterIF.cpp',
    'h': 'cTesterIF.h',
    'page': 'Setup.TesterIF.html',
    # 這個表單寫檔的主要結構（決定放進 FileRW/<struct>.cpp）；golden 函式不拆，跨結構的另列 also
    'struct': 'TestIF_File',
    'files': ['Tester.Data'],
    # 要轉的 golden 方法。顯示入口與存檔入口另外指定。
    'methods': ['InitcbDIOType', 'DoIniDataToForm', 'rgInterfaceTypeClick', 'ShowPageControl2',
                'cbRs232TypeChange', 'SaveSetupFile', 'spbSaveClick'],
    'members': ['fShow', 'bflag'],
    # 顯示入口 ＝ golden 開表單的順序：建構子 :42 InitcbDIOType(false) → DoIniDataToForm()
    'display': ['B_InitcbDIOType(J, false);', 'B_DoIniDataToForm(J);'],
    'save': 'B_SaveSetupFile',
    # 存檔的完整流程 ＝ golden 存檔鈕 spbSaveClick（守衛 → SaveSetupFile → SECS → 重讀 → 備份 → SetWorkParameter）
    'saveFlow': 'B_spbSaveClick',
    # 讀檔端缺口（使用者 20260924：「讀檔的部分，如果沒有實作的，就列入待辦」）
    'sourceGap': 'TestIF_File (Tester.Data) is filled by golden TFTestIF::ReadTestIFFile() '
                 '(cTesterIF.cpp:563), which is still GATE (F-5) in the port -- display values '
                 'are struct init values, not the recipe',
    'includes': ['cprod.h', 'Config.h', 'cmydef.h', 'common.h', 'MachineType.h', 'BarcodeReader.h',
                 'CosFunction.h', 'vclcompat/SysUtils.h', '<windows.h>',
                 # spbSaveClick：SECS 事件（真的 EventReport，不是 acatchtray_shims.h 那個）、fMain、SetWorkParameter
                 'SECSGEM/SecsEventType.h', 'SECSGEM/SecsEventReport.h', 'forms/fMain.h', 'cinitial.h'],
    # 整段覆寫：(方法, golden 起始行, golden 結束行, 起始行必須含的文字, 取代文字)。
    # 起始行文字核對不過就中止 —— golden 換版行號漂了，要人重新確認，不能蓋錯地方。
    'blocks': [
        # golden :1348-1371 Recipe Parameter Default 的 ChangeLog（fRPDefault／fSpeed->SearchRecipeParameter／
        # fCleaning／fYieldMonitoring->SearchRecipeParameter）—— 這幾個 SearchRecipeParameter 在移植樹都沒有。
        ('spbSaveClick', 1348, 1371, 'if(CosFunction.bRecipeParameterDefault)',
         'if(CosFunction.bRecipeParameterDefault)                                     //Sam 20201209 : Default Recipe ChangeLog\n'
         '    {\n'
         '        J.Todo("golden spbSaveClick :1348-1371 Recipe Parameter Default ChangeLog (fRPDefault / SearchRecipeParameter) not ported");\n'
         '    }'),
    ],
    # 無法機械改寫的語句：(方法, 原文片段) -> 取代文字。每一條都要說為什麼。
    'overrides': [
        # golden :1325-1326 權限不足：ShowMyMessage 後 Close() 關表單。bridge 改進 messages，並標記 closed。
        ('spbSaveClick', 'ShowMyMessage("[A01_2]目前已切換到Operator權限，\\r\\n請重新登入再做設定!");',
         'J.Message("[A01_2] switched to Operator level, please log in again", "[A01_2]目前已切換到Operator權限，\\r\\n請重新登入再做設定!");'),
        ('spbSaveClick', 'Close();', 'J.M("closed")=1;   // golden: Close()'),
        # golden :1345 存檔後重讀。讀檔端還沒實作（GATE F-5），使用者 20260924：列入待辦。
        ('spbSaveClick', 'ReadTestIFFile();',
         'J.Todo("golden spbSaveClick :1345 ReadTestIFFile() not ported (GATE F-5) -- TestIF_File is NOT reloaded after save");'),
        # golden :98-101 找不到 DIO 設定檔時：開機路徑 MessageBox＋Terminate，其餘 ShowMyMessage。
        # bridge 在 GET 裡不彈窗、更不能結束程式 —— 改成 JSON 的 messages，由頁面顯示。
        ('InitcbDIOType', 'Application->MessageBox("DIO data has been lossed, please check!!", "DIO Data loss", MB_OK|MB_TOPMOST);',
         'J.Message("DIO data has been lossed, please check!!", "DIO Data loss");   // golden: Application->MessageBox(... MB_OK|MB_TOPMOST)'),
        ('InitcbDIOType', 'Application->Terminate();',
         'J.Todo("golden InitcbDIOType: Application->Terminate() when DIO data is missing -- not done by the bridge");'),
        ('InitcbDIOType', 'ShowMyMessage("DIO data has been lossed, please check!!", "DIO資料遺失，請檢查！！");',
         'J.Message("DIO data has been lossed, please check!!", "DIO資料遺失，請檢查！！");'),
        # golden :1165 ShowMyMessage 只在 fShow==true 時觸發；bridge 不彈窗，改進 messages。
        ('rgInterfaceTypeClick', 'ShowMyMessage("TTL only support less then 4 site, please select GPIB or RS-232.", "4 Site 以上不支援 TTL，請選擇GPIB 或 RS-232");',
         'J.Message("TTL only support less then 4 site, please select GPIB or RS-232.", "4 Site 以上不支援 TTL，請選擇GPIB 或 RS-232");'),
        # golden :1194 `TTabSheet *tsTemp[]={tsDio, tsGpib, tsRs232, tsTCPIP};` 是 widget 指標陣列，
        # 改成名字陣列；:1196-1199 的 ->TabVisible／ActivePage／ActivePageIndex 跟著改。
        ('ShowPageControl2', 'TTabSheet *tsTemp[]={tsDio, tsGpib, tsRs232, tsTCPIP};',
         'const char* tsTemp[]={"tsDio", "tsGpib", "tsRs232", "tsTCPIP"};'),
        ('ShowPageControl2', 'tsTemp[i]->TabVisible=false;', 'J.SetTabVisible(tsTemp[i], false);'),
        ('ShowPageControl2', 'tsTemp[Index]->TabVisible=true;',
         'if(Index<0 || Index>3) { J.Todo("ShowPageControl2: Index out of 0..3 (golden indexes tsTemp[] unchecked)"); return; }\n'
         '    J.SetTabVisible(tsTemp[Index], true);'),
        ('ShowPageControl2', 'PageControl2->ActivePage=tsTemp[Index];',
         '/* golden 這行設 PageControl2 的 ActivePage 為 tsTemp[Index]，與下一行的 ActivePageIndex 同義 */'),
    ],
}
