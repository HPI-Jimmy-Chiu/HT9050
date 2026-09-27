# -*- coding: utf-8 -*-
# tools/formbridge/TfHotPlate.py -- gen_formbridge.py 的表單設定（一個 BCB 表單一個檔）。
# 欄位說明見 tools/formbridge/README.md。改完跑：python tools/gen_formbridge.py [--only TfHotPlate]
FORM = {
    'class': 'TfHotPlate',
    'cpp': 'cHotPlate.cpp',
    'h': 'cHotPlate.h',
    'page': 'Setup.HotPlate.html',
    'struct': 'HotPlateForm_File',
    'files': ['HotPlate.Data'],
    # golden 開表單＝FormShow（:35，內含 ReadFile＋DoIniDataToForm＋依條件改 Checked／Enabled）；
    # 存檔鈕＝spbSaveClick（:440，內含 SaveSetupFile＋存後 ReadFile）
    # AI(W906-FRW-S157) 20260927 [W906]：＋cbSelectHPFromDBChange（:412，cbSelectHPFromDB 的 OnChange cHotPlate.dfm:322）
    #   —— WS form.event（Steven ★ Q40＝A，RULINGS_20260926 S157），見下面 'events'
    'methods': ['FormShow', 'DoIniDataToForm', 'spbSaveClick', 'SaveSetupFile', 'cbSelectHPFromDBChange'],
    'members': ['fShow'],
    'display': ['B_FormShow(J);'],
    'save': 'B_SaveSetupFile',
    'saveFlow': 'B_spbSaveClick',
    # 讀檔器 TfHotPlate::ReadFile 已在移植樹（forms/fHotPlate.cpp），golden 呼叫它的地方接過去
    'port_calls': {'ReadFile': 'fHotPlate->ReadFile'},
    'sourceGap': '',
    'includes': ['cprod.h', 'Config.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'SECSGEM/SecsEventType.h', 'SECSGEM/SecsEventReport.h',
                 'forms/fMain.h', 'forms/fHotPlate.h', 'LastSet.h', 'csystem.h', 'cAuthority.h',
                 'forms/fSecurity.h', 'aHotPlateSubstrate.h', 'ainarm_SearchPlacePlate.h',
                 'FileRW/CfgTrayPlate.h',    # AI(W906-FRW-S98) 20260926: W906_CfgTrayPlate()（golden fConfiguration 的 Plate 表）
                 'BarcodeReader.h'],         # AI(W906-FRW-S157) 20260927 [W906]：cbSelectHPFromDBChange :417 Barcode_Reader(bcPlateForm)
    # AI(W906-FRW-S157) 20260927 [W906]：WS form.event 的事件表（Steven ★ Q40＝A，RULINGS_20260926 S157；格式 FROM_STEVEN
    #   20260927 10:15）。頁面選 cbSelectHPFromDB 一筆 → FileRW/_FormEvent.cpp → formbridge::RunEvent：先跑 display（golden
    #   FormShow，設 fShow=true、清單照 PlateForm.csv 重建）→ 套頁面送的 itemIndex／text → 跑下面轉出的 golden 處理器 →
    #   回處理器賦值的 7 個欄位（HotPlateName、XST1、YST1、XPitch1、YPitch1、XCT1、YCT1）。
    'events': [('cbSelectHPFromDB', 'change', 'cbSelectHPFromDBChange')],
    # AI(W906-FRW-S98) 20260926: 原本 golden :47-58 整段覆寫成 J.Todo（"cbSelectHPFromDB items ... not ported"）——
    #   Plate 表現在在 FileRW/CfgTrayPlate.cpp（golden sbtReloadHPClick cConfiguration.cpp:7148 逐字），所以拿掉整段覆寫，
    #   :47-58 交給產生器照 golden 機械轉換（:55 cbSelectHPFromDB->Items->Add(…) → J.ItemsAdd("cbSelectHPFromDB", …)），
    #   只在 :49 前面接上 fConfiguration（見 overrides）。
    'blocks': [
        # AI(W906-FRW-S157) 20260927 [W906]：golden :417-420 KYEC 條碼登入（Barcode_Reader，只有 CC_KYEC_LEE／CC_KYEC_XILINX 且
        #   USE_BARCODE_AS_KEYBOARD 時會問操作員 ID；其他機台回 2）。照叫移植樹的真本體（BarcodeReader.cpp:445）；回 0 時 golden
        #   直接 return（不填欄位），這裡照 return，另記 J.Todo 讓頁面看得到為什麼沒填（移植樹的輸入框是離線空殼，同 FileRW/MainClose.cpp:377）。
        ('cbSelectHPFromDBChange', 417, 420, 'if(Barcode_Reader(bcPlateForm)==0)',
         'if(Barcode_Reader(bcPlateForm)==0) { J.Todo("golden cHotPlate.cpp:417 Barcode_Reader(bcPlateForm) returned 0 (KYEC operator-ID prompt; the port input box is an offline shell) -- golden returns without filling"); return; }'),
    ],
    'overrides': [
        # AI(W906-FRW-S157) 20260927 [W906]：cbSelectHPFromDBChange（golden :412-438）。Sender＝cbSelectHPFromDB
        #   （這個處理器只掛在它的 OnChange，cHotPlate.dfm:322），所以 :421-422 的 ComboBox 就是它。
        ('cbSelectHPFromDBChange', 'TComboBox *ComboBox=(TComboBox *)Sender;',
         '/* golden :421 TComboBox *ComboBox=(TComboBox *)Sender;  —— Sender＝cbSelectHPFromDB（OnChange 只掛在它，cHotPlate.dfm:322） */'),
        ('cbSelectHPFromDBChange', 'int index=ComboBox->ItemIndex;',
         'int index=J.GetItemIndex("cbSelectHPFromDB");   // golden :422 ComboBox->ItemIndex'),
        # golden :429 同 FormShow :49 的接法（本方法自己的區域變數；和 FormShow 那一份指向同一張 Plate 表）
        ('cbSelectHPFromDBChange', 'fConfiguration->sbtReloadHP->Click();',
         'TfConfigurationTrayPlate *fConfiguration=W906_CfgTrayPlate();   //AI(W906-FRW-S157) 20260927 [W906]: golden 全域 fConfiguration 的 Plate 表（FileRW/CfgTrayPlate.cpp），同 FormShow :49 的接法\n'
         '    fConfiguration->sbtReloadHP->Click();'),
        # golden :38 LoadImage() 載入示意圖（純畫面）
        ('FormShow', 'LoadImage();', '/* golden: LoadImage() —— 示意圖，HTML 自己有 */'),
        # AI(W906-FRW-S98) 20260926: golden :49 fConfiguration->sbtReloadHP->Click()（讀 D:\HT9045\System\PlateForm.csv）。
        #   C 路（tools/editlist/UserDefForm_File.py、TestIF_File_Cleaning.py）在 TU 檔頭 `#define fConfiguration (W906_CfgTrayPlate())`；
        #   A 形狀產生器只有 includes、沒有檔案層原文插入點，所以同一個接法改成「這一行前面宣告同名區域變數」：
        #   golden :49-57 其餘每一行照原文，fConfiguration 解析到同一份表（全樹一份，C 路 IniConfig FormShow :4676-4684 也 Click 同一顆）。
        #   本 TU 原本看不到任何 fConfiguration 宣告（SCK_ART 替身 Automation/SCK_ART_Remainder.h:599 沒被引入）⇒ 區域變數不遮蔽任何全域。
        #   display 在 JsonBridge/FormJson.cpp:198 持 FormLock 跑，和主迴圈 editlist.get 同一把鎖，重讀表不會和 C 路撞。
        #   沒轉的另外兩處讀表（不在 methods；A 形狀 BridgeDesc 只有 display／save／saveFlow，沒有事件入口）：
        #     :424-437 cbSelectHPFromDBChange（cbSelectHPFromDB 的 OnChange，cHotPlate.dfm:322）——頁面選一筆不會填 7 個欄位；
        #       ⛔ 更正 AI(W906-FRW-S157) 20260927 [W906]：這一條已轉（methods／events，WS form.event；Steven ★ Q40＝A）。
        #     :747-759 ShowTypePage(int)——golden 唯一呼叫者 TfProductionInfo::SettingHotPlateFormFromServer（ProductionInfo.cpp:5471，
        #       MES 下載流程），移植樹那條在 ProductionInfo/uPAT_Function.cpp:1765-1796 的 #if 0（[G4]/[G5]），不可達。
        ('FormShow', 'fConfiguration->sbtReloadHP->Click();',
         'TfConfigurationTrayPlate *fConfiguration=W906_CfgTrayPlate();   //AI(W906-FRW-S98) 20260926: golden 全域 fConfiguration 的 Plate 表（FileRW/CfgTrayPlate.cpp）；同 C 路 #define 的接法，見 tools/formbridge/TfHotPlate.py\n'
         '    fConfiguration->sbtReloadHP->Click();'),
        # golden :478 fMain->ChangeATCSiteUse()：移植樹 TfMain 沒有本體（forms/fMain.cpp:921 GATE W906-HOME-W1-ATCSITE）
        ('spbSaveClick', 'fMain->ChangeATCSiteUse();',
         'J.Todo("golden spbSaveClick: fMain->ChangeATCSiteUse() has no body in the port (GATE W906-HOME-W1-ATCSITE)");'),
    ],
}
