# -*- coding: utf-8 -*-
# tools/editlist/HSys.py -- gen_editlist.py 的結構設定（一個 C 形狀結構一個檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only HSys
#
# Steven 團隊 20260925：HSys（system\Gerneral.ini ＋ D:\RS232Standard\System\Setup.ini ＋ D:\GPIB9045\system\general.ini）
# —— golden THandlerSystem（HT9011UC_Code_V3.33.912.0_20260908_Jimmy HandlerSys.cpp，1542 行）。頁面 web/page/HW.HandlerSys.html。
#
# Steven 20260925 裁決：「這部分要參考 BCB 原本的作法：void SYSTEM_MODULAR::ReadGeneralIni()、
# void __fastcall THandlerSystem::FormShow(TObject *Sender)」。
#   開頁＝golden FormShow（:132，SortItemToMap／LoaderSystemSet :177／LoaderSafeDoorSet :1132／GPIB 型號）；
#   golden 開表單時 CheckAndReadIniDataGeneral 補寫缺鍵、MyForceDirectories("D:\\RS232Standard\\System")、
#   CheckAndReadIniData 補寫 D:\GPIB9045\system\general.ini 與 Setup.ini 的缺鍵 —— 照 golden 保留（BCB 開表單本來就做）。
#   存檔鈕＝SaveBtnClick（:1121 → SaveSystemSet :575，YES/NO 題目 golden 原字串）。
#   存檔後照 golden 離開鈕 ExitBtnClick（:1179）的資料那一半跑 HSys.ReadGeneralIni()＋SaveSafeDoorSet()（理由見 FileRW/HSys_C.cpp）。
#   golden TfMain::FormShow（main.cpp:9589-9594）的 bHandlerModel==false 停機 → 網頁版不結束程式：ELMessage（golden 原字串）
#   ＋ELMark("model_read_error")，並拒絕存檔（FileRW/HSys_C.cpp 的包裝函式，不在產生碼裡）。
#
# 不 adopt 移植樹 HandlerSystem（forms/fHandlerSys.h）：移植樹沒有任何活著的程式讀它的元件
# （grep `HandlerSystem->` 20260925：只有 forms/fTemperFrom.h:85 一行註解），而且它寫的是 `new TComboBox()`
# （不帶 vclcompat::），產生器的 adopt 認不到。
#
# 入口檔名注意：FileRW/HSys.cpp 已被 A 形狀（tools/formbridge/THandlerSystem.py）佔用，本結構的手寫入口是
# FileRW/HSys_C.cpp；gen_editlist.py 的 _editlist_sources.cmake 會列 FileRW/<struct>.cpp ＝ FileRW/HSys.cpp ——
# 整合者退役 A 形狀時要把 HSys_C.cpp 改名成 HSys.cpp（或調整清單），否則會編到 A 形狀那支。
_F = 'THandlerSystem'


def _el(t, name):
    return 'EL<%s>("%s", "%s")' % (t, _F, name)


# Steven 20260925（9050GPIB 改善 A＋B，見 replace FormShow :166、SaveSystemSet :1032／:1034）
CB = _el('TComboBox', 'cbHandlerModel')
KNOWN = ' || '.join('Str2=="%s"' % m for m in
                    ['9045GPIB', '9046GPIB', '9045GPIB_12Site', '9046_32GPIB', '502GPIB', '1032GPIB', '7080GPIB', '9050GPIB'])


_HEATER_TODO_LOAD = (
    'golden HandlerSys.cpp:70-112 + :262-278 EN_HEATER_SHEET=1 (golden 912 MachineType.h:658) branch not in the port '
    '(no EN_HEATER_SHEET, no g_tHeaterInsInfo/HeaterInsOpt_Read): per-station heater brand ComboBoxes are not built/shown, '
    'per-station [TempCtrl] keys are not read (golden HeaterInsOpt_Read also back-fills them into Gerneral.ini), '
    'rgHeaterType uses the :271 fallback HEATER_CTRL_TYPE (golden :269 uses the common per-station value when all stations agree)')
_HEATER_TODO_SAVE = (
    'golden HandlerSys.cpp:779-794 EN_HEATER_SHEET=1 branch not in the port -- per-station [TempCtrl] keys '
    '(g_tHeaterInsInfo[].m_asSaveName) are NOT written; HEATER_CTRL_TYPE is written as rgHeaterType (golden :793; '
    'golden :791 writes the common per-station value when all stations agree)')

STRUCT = {
    'struct': 'HSys',
    'prefix': 'HS',
    'class': 'THandlerSystem',
    'cpp': 'HandlerSys.cpp',
    'h': 'HandlerSys.h',
    'files': ['system\\Gerneral.ini', 'D:\\RS232Standard\\System\\Setup.ini', 'D:\\GPIB9045\\system\\general.ini'],
    'lists': [],
    # 建構子 :63（初始化串列 : TForm(Owner)）＝slCustomerCode（客戶碼清單）；
    # rgTTLCardClick：LoaderSystemSet :307 設 rgTTLCard->ItemIndex，VCL 在值有變時觸發 OnClick（dfm :442）→ 見 replace；
    # SaveSafeDoorSet／ExitBtnClick：存檔後的「離開鈕」資料半段（FileRW/HSys_C.cpp 呼叫）。
    'methods': ['THandlerSystem', 'FormShow', 'LoaderSystemSet', 'LoaderSafeDoorSet', 'rgTTLCardClick',
                'SaveSystemSet', 'SaveBtnClick', 'SaveSafeDoorSet', 'ExitBtnClick'],
    # SaveSafeDoorSet 不列：它讀的 SafeDoor1..10／HeaterDoor1..2 在 GroupBox1／GroupBox2 裡，golden FormShow :139-140
    # 把兩個 GroupBox 設成 Visible=false（使用者改不到）⇒ 頁面送的值一律丟掉（ELEditable），值只可能是 LoaderSafeDoorSet
    # 讀進來的 Sen[].Enable；列成 mustSend 只會逼頁面送一堆被丟掉的值。
    'save_methods': ['SaveBtnClick', 'SaveSystemSet'],
    'params': {'THandlerSystem': '', 'FormShow': '', 'rgTTLCardClick': '', 'SaveBtnClick': '', 'ExitBtnClick': ''},
    'members': ['TStringList *slCustomerCode=nullptr;   // golden HandlerSys.h:513（建構子 :66 new；只給客戶碼搜尋 edtSearchCodeChange 用）',
                # Steven 20260925（9050GPIB 改善 A）：開頁時 GPIB Model 能不能對到 cbHandlerModel、對到哪一項
                'bool bW906ModelOpenKnown=true;   // 開頁讀到的 general.ini Model 在 cbHandlerModel 的 8 項之內',
                'int iW906ModelOpenIndex=-1;       // 開頁時 cbHandlerModel->ItemIndex（使用者沒動＝存檔時仍是這個值）'],
    'replace': [
        # ---- Steven 20260925：9050GPIB 改善 A＋B（移植樹 ReadGeneralIni 白名單依使用者裁決多了 9050GPIB，database.cpp:345；
        #      golden cbHandlerModel 只有 7 項 → 9050 機台開頁停在 DFM 初值 3，一存就把 Model 改成 9046_32GPIB） ----
        ('FormShow', 166, 166,
         '9050GPIB 改善 B：cbHandlerModel 第 8 項 HT9050（FileRW/HSys.cpp Boot 補 Items）對應 9050GPIB，並同步 Text；'
         '改善 A：記下開頁時型號是否對得到、對到哪一項（存檔守衛用）',
         'else if(Str2=="7080GPIB") filerw::ELComboIndex(%s, 6); '
         'else if(Str2=="9050GPIB") filerw::ELComboIndex(%s, 7); '
         'bW906ModelOpenKnown=(%s); iW906ModelOpenIndex=%s->ItemIndex;' % (CB, CB, KNOWN, CB)),
        ('SaveSystemSet', 1032, 1032, '9050GPIB 改善 B：第 8 項寫回 9050GPIB',
         'case 6: Str2="7080GPIB"; break; case 7: Str2="9050GPIB"; break;'),
        ('SaveSystemSet', 1034, 1034,
         '9050GPIB 改善 A：開頁讀到的 Model 不在 cbHandlerModel 之內、使用者也沒改選 → 不改寫 general.ini Model（保留原值），'
         '避免 golden switch 把未知型號寫成預設值',
         'if(!bW906ModelOpenKnown && %s->ItemIndex==iW906ModelOpenIndex) '
         'filerw::ELMessage("GPIB general.ini Model is not in the model list and was not changed -- Model kept as is.", '
         '"GPIB general.ini 的 Model 不在機型清單內、也沒有改選，保留原值不改寫。"); '
         'else WriteIniData(Str, "Version", "Model", Str2);' % CB),
        # ---- 建構子 ----
        ('THandlerSystem', 66, 68,
         'slCustomerCode 是 TStringList 成員（不是 widget），但 golden header 的宣告形狀與 widget 相同，產生器會把它改成替身 —— 等價取代成 static 成員',
         'slCustomerCode=new TStringList(); for(int i=0; i<%s->Items->Count; i++) slCustomerCode->Add(%s->Items->Strings[i]);'
         % (_el('TRadioGroup', 'rgCustomerList'), _el('TRadioGroup', 'rgCustomerList'))),
        ('THandlerSystem', 70, 112,
         'EN_HEATER_SHEET=1（golden 912 MachineType.h:658）的逐站溫控 ComboBox 動態建立：移植樹沒有 EN_HEATER_SHEET 也沒有 g_tHeaterInsInfo，'
         '照抄的話 `#if EN_HEATER_SHEET` 在移植樹默默變成空；開機沒有 session，待辦改在 LoaderSystemSet :262 回報',
         ';'),
        # ---- FormShow ----
        ('FormShow', 134, 134, 'SortItemToMap()：功能搜尋框的元件索引（TempComp，edtSearchFunctionChange 搬 Parent），純畫面', ';'),
        ('FormShow', 174, 174, 'myLog.Do_Log(Sender, asUser, asLogPath)：TMyLog 逐一比對表單控制項記錄使用者改了什麼（伺服器端沒有表單物件）',
         'filerw::ELTodo("golden HandlerSys.cpp:174 myLog.Do_Log(Sender, asUser, asLogPath) (per-control change log) not done");'),
        # ---- LoaderSystemSet ----
        ('LoaderSystemSet', 262, 278, '#if !EN_HEATER_SHEET：移植樹沒定義 EN_HEATER_SHEET ⇒ 照抄會默默走 golden 912 沒在跑的舊分支；改成明寫 golden :271 的回退值',
         'filerw::ELTodo("%s"); %s->ItemIndex=CheckAndReadIniDataGeneral("TempCtrl","HEATER_CTRL_TYPE",KT4H);'
         % (_HEATER_TODO_LOAD, _el('TRadioGroup', 'rgHeaterType'))),
        ('LoaderSystemSet', 307, 307,
         'VCL 程式設定 TRadioGroup::ItemIndex（值有變）會觸發 OnClick（dfm :442 OnClick=rgTTLCardClick）；vclcompat 替身不觸發事件 ⇒ 值有變時明呼叫',
         '{ const int v_=CheckAndReadIniDataGeneral("System", "TTL_CARD_TYPE",      0); const bool ch_=(%s->ItemIndex!=v_); '
         '%s->ItemIndex=v_; if(ch_) HS_rgTTLCardClick(); }' % (_el('TRadioGroup', 'rgTTLCard'), _el('TRadioGroup', 'rgTTLCard'))),
        ('LoaderSystemSet', 563, 563,
         '預設值 eSafePLCIOType_Uninstall（golden cmydef.h ESafePLCIOType，＝0）移植樹沒有這個列舉 → 字面值 0（值相同；移植樹 database.cpp 讀同鍵也是 0）',
         '%s->ItemIndex=CheckAndReadIniDataGeneral("System", "SafePlcIO", 0 /* golden: eSafePLCIOType_Uninstall (=0) */);'
         % _el('TRadioGroup', 'rgSafePlcIO')),
        # ---- SaveSystemSet ----
        ('SaveSystemSet', 577, 577,
         'Application->MessageBox YES/NO → ELAsk（題目＝golden 英文原字串，頁面帶答案 {"Do you want to store the setting?":1}）；'
         '答 YES 才記 SaveSystemSet:write（PageDesc.savedMark）',
         'if(filerw::ELAsk("Do you want to store the setting?", "是否儲存設定？")==1) filerw::ELMark("SaveSystemSet:write"); else return;'),
        ('SaveSystemSet', 612, 612, 'Application->MessageBox(MB_OK｜MB_ICONWARNING) → ELMessage（之後照 golden 強制關掉 chkAuto1Y／chkAuto2Y）',
         'filerw::ELMessage("Cassette mode (Boat Carrier) already uses MAuto1Y/MAuto2Y. AUTO1_Y_USE_MOTOR / AUTO2_Y_USE_MOTOR will be force-disabled.", '
         '"Cassette 模式（Boat Carrier）已使用 MAuto1Y／MAuto2Y，AUTO1_Y_USE_MOTOR／AUTO2_Y_USE_MOTOR 將強制關閉。");'),
        ('SaveSystemSet', 688, 688,
         'fMain->InitialSuperVisorPassword(CUSTOMER_CODE)：golden 本體 main.cpp:26458；移植樹只在 WebLogin.cpp:64 以 static 翻譯（開機 WebLogin_Boot 呼叫），本 TU 呼叫不到',
         'filerw::ELTodo("golden HandlerSys.cpp:688 fMain->InitialSuperVisorPassword(CUSTOMER_CODE) not called -- the port has it only as a static in WebLogin.cpp:64 (boot time); the supervisor password is not re-initialised for the new CUSTOMER_CODE");'),
        ('SaveSystemSet', 779, 794, '#if !EN_HEATER_SHEET（同 LoaderSystemSet :262-278）',
         'filerw::ELTodo("%s"); WriteIniDataGeneral("TempCtrl","HEATER_CTRL_TYPE",%s->ItemIndex);'
         % (_HEATER_TODO_SAVE, _el('TRadioGroup', 'rgHeaterType'))),
        # ---- SaveBtnClick ----
        ('SaveBtnClick', 1124, 1124, 'Application->MessageBox(MB_OK) → ELMessage（golden 答 NO 也會跳這一則）',
         'filerw::ELMessage("Please restart the program to active new parameters.", "請重新啟動程式以套用新參數。");'),
        # ---- ExitBtnClick ----
        ('ExitBtnClick', 1184, 1184,
         'Close()：網頁的存檔不是離開鈕 —— FileRW/HSys_C.cpp 只在存檔成功後借用離開鈕的資料半段（ReadGeneralIni＋SaveSafeDoorSet），不關頁',
         ';'),
    ],
    'blocks': [],
    'includes': ['cprod.h', 'Config.h', 'LastSet.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'database.h', 'mysensor.h',
                 # COMMSPEED_20M（LoaderSystemSet :305 的預設值）：golden Motor/mn200.h，移植樹在 Motor/vendor/mn200.h:14
                 'Motor/vendor/mn200.h'],
    'decls': [],
    'overrides': [],
}
