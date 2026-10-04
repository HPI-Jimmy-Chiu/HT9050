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
                 'BarcodeReader.h', 'FileRW/MainClickTail.h'],   # AI(W906-FRW-S157) 20260927 [W906]：cbSelectHPFromDBChange :417 Barcode_Reader(bcPlateForm)；AI(W906-FRW-S158) 20260927 [W906]：＋FileRW/MainClickTail.h（W906_Main_sbPlateFormClickTail，見檔尾 saveFlowAfter；同一行改寫）
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
        # AI(W906-E031) 20261003 [W906] (St01)：Q78 第 20 支（ST01-M 7cfd6ff8 補登記；human-review C24；另一個驗證用 session V-2 覆核）——A02 分支
        #   Close() 後面那句 `return;` 照 V912 留著，這一列只加三段註解：取代碼＝golden 原文 `return;`，產生的程式不變。
        #   本結構目前讀 V912（golden_root DEFAULT_TREE），行號是 V912 的；'return;' 釘住 V912 :447——TfHotPlate 換到 906 0618 時這一列會
        #   中止產生器（0618 :447 是 `}`），到時改成接在 0618 :446 Close() 後面的插入（同 tools/editlist/TrayForm.py 的 0618 :1260 那一列）。
        ('spbSaveClick', 447, 447, 'return;',
         'return;   //AI(W906-E031) 20261003 [W906] (St01): Q78 handler #20, golden text kept (comment only). '
         '(1) golden 906 0618 cHotPlate.cpp:446: the A02 branch (RogerYang 20260305 [A01_2], :442-447) only calls Close() -- no return, so it '
         'closes and goes on to the HP check and SaveSetupFile (:449-460); fHotPlate is ShowModal (0618 main.cpp:27431), Close() only sets '
         'ModalResult => the values the operator changed are still written. (2) golden V912 cHotPlate.cpp:447 `return;` kept => closed, nothing saved. '
         '(3) #20 exception (Steven 1003 standing rule; Q78)'),
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
    # AI(W906-FRW-S158) 20260927 [W906]：Q41 第 3 項 HP-3 關窗尾段（decisions-pending R85＝A：RULINGS_20260926 S107-1「存檔後就跑，
    #   不改成等 Exit」的延伸；方案 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_CLOSETAIL_PLAN_20260927.md §5.7）。
    #   saveFlowAfter＝tools/gen_formbridge.py 的可選鍵：每條一行 C++，照原樣接在產生的 SaveFlow 裡 B_spbSaveClick(J); 之後。
    #   golden 的順序：spbSaveClick（:440）→ 視窗關掉時 FormClose（:401-410：ReadFile、DoIniDataToForm、fShow=false）→ ShowModal 回來
    #   → TfMain::sbPlateFormClick 尾段（V912 main.cpp:28422-28425：DoStructUnitConvert、SetWorkParameter、fCleaning->LoadAutoCleanData、
    #   GetHotPlateYHalfPos；本體 FileRW/MainClick.cpp W906_Main_sbPlateFormClickTail，運轉中不跑 R86）。
    #     (1) A02 權限不足：golden :446 Close()（產生器轉成 J.M("closed")=1）→ FormClose → 尾段。FormClose 只跑 fHotPlate->ReadFile：
    #         DoIniDataToForm 與 fShow=false 改的是這次請求的 J（A 形狀每次請求一份 FormState，請求結束就丟），沒有伺服器端替身要還原。
    #     (2) 有進 golden SaveSetupFile（J.M("saved")，B_SaveSetupFile 一進來就設）：golden 存完視窗還開著、等 Exit 才跑尾段；網頁 Exit
    #         不送伺服器 → 存完就跑（S107-1）。「兩個加熱盤都沒勾」（:450-455）在 SaveSetupFile 之前 return ＝ golden 視窗還開著 → 不跑。
    #   沒跑的原因（運轉中）交給 J.Todo（ack.todo）；呼叫要寫 ::（產生碼在 namespace ht9045::formbridge 裡，宣告在全域標頭 FileRW/MainClickTail.h）。
    #   form.save 本身運轉中已擋（R87，JsonBridge/FormJson.cpp FormSave），尾段自己的 R86 是第二道。
    'saveFlowAfter': [
        'if (J.M("closed")) fHotPlate->ReadFile();   //AI(W906-FRW-S158) 20260927 [W906]: golden FormClose（cHotPlate.cpp:401）的 :404 ReadFile（A02 Close() 之後；DoIniDataToForm／fShow 是這次請求的 J，不跑）',
        'if (J.M("closed") || J.M("saved")) { if (const char* w = ::W906_Main_sbPlateFormClickTail()) J.Todo(w); const char* e_ = 0; const char* z_ = 0; const char* t_ = 0; while (::W906_Main_Hp3TailTakeMessage(&e_, &z_)) J.Message(AnsiString(e_), AnsiString(z_)); while (::W906_Main_Hp3TailTakeTodo(&t_)) J.Todo(t_); }   //AI(W906-FRW-S158) 20260927 [W906]: golden TfMain::sbPlateFormClick 關窗尾段 V912 main.cpp:28422-28425（FileRW/MainClick.cpp；R85／S107-1、R86）  AI(W906-EVB6) 20260928 [W906]: R107 尾段裡 golden LoadAutoCleanData 的 ShowMyMessage／todo 放進存檔回覆（ack.messages／ack.todo）',
    ],
}
