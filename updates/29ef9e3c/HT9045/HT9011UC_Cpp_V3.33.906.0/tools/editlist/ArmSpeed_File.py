# -*- coding: utf-8 -*-
# tools/editlist/ArmSpeed_File.py -- gen_editlist.py 的結構設定（一個 C 形狀結構一個檔；Steven 20260924 拆檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py [--only ArmSpeed_File]
# Steven 20260924：ArmSpeed_File 族（ArmCondition.Data）—— golden TfSpeed（cSpeed.cpp，2517 行）。
# 沒有 HTEditList：golden 存檔鈕 spbSaveClick（:1433）自己逐鍵 WriteIniData（139 處）＋ReadWriteFile(false)，
# 不呼叫 SaveSetupFile —— A 形狀產生器（FormState 空跑 save）處理不了，改走 C 路（替身＋直接跑 golden 存檔鈕）。
# TTrackBar／TUpDown 的 Position／OnChange 照 VCL 語意（filerw::ELTrackBar）。
# //AI(W906-E031) 20261003 [W906] (St01)：E-031 全面切換第 1 批 —— golden 換成 906 0618（STRUCT['golden']＝'906'，tools/golden_root.py；
#   Jimmy RULINGS_20261003 第 2、4 條）。cSpeed.cpp／.h 0618 與 V912 逐位元組相同（2518／557 行），cSpeed.dfm 只差換行字元（0618 有幾行
#   是 LF，去掉行尾空白後兩棵一樣，產生器讀的屬性一樣）⇒ 本檔的行號兩棵都對、產生的程式不變（只有檔頭路徑變）。
import golden_root as _GR   # AI(W906-E031) 20261003 [W906]：golden 一律經過 tools/golden_root.py

_TREE = _GR.tree_of('ArmSpeed_File', '906')   # AI(W906-E031)：E-031 全面切換第 1 批 —— 906 0618；STRUCT['golden'] 同一個值
STRUCT = {
    'struct': 'ArmSpeed_File',
    'golden': _TREE,   # AI(W906-E031) 20261003 [W906]：906 0618（tools/golden_root.py）
    'prefix': 'SP',
    'class': 'TfSpeed',
    'cpp': 'cSpeed.cpp',
    'h': 'cSpeed.h',
    'files': ['ArmCondition.Data'],
    'lists': [],
    'methods': ['TfSpeed', 'FormShow', 'ReadFile', 'ReadWriteFile', 'DoIniDataToForm',
                'tbAllSpeedChange', 'tbAccSpeedChange', 'tbEPControlChange', 'FormClose', 'spbSaveClick',
                # AI(W906-FRW-S158) 20260927 [W906]：Q41 盤點（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md §3.5）
                #   SP-2／SP-3／SP-4 的 golden 處理器 —— WS form.event（下面 'events'）＋FileRW/ArmSpeed_File.cpp BeforeApply 的
                #   cbIndexArmClick（存檔前重播）。加在最後：既有方法的產生碼不動。
                'spbSpeedAddClick', 'spbSpeedDecClick', 'cbIndexArmClick', 'spbSelectAllClick', 'spbSetToDefClick'],
    'save_methods': ['spbSaveClick', 'ReadWriteFile'],
    'params': {'TfSpeed': '', 'FormShow': '', 'FormClose': '', 'spbSaveClick': '',
               'tbAllSpeedChange': '', 'tbAccSpeedChange': '', 'tbEPControlChange': '',
               # AI(W906-FRW-S158) 20260927 [W906]：這五支 golden 都不讀 Sender
               'spbSpeedAddClick': '', 'spbSpeedDecClick': '', 'cbIndexArmClick': '', 'spbSelectAllClick': '',
               'spbSetToDefClick': ''},
    'members': ['AnsiString LastFileName;   // golden cSpeed.h: TfSpeed::LastFileName',
                'bool fShow=false;          // golden cSpeed.h: TfSpeed::fShow'],
    'replace': [
        ('TfSpeed', 38, 38, 'edtHPVacuumDelay->Hint：Default Recipe ChangeLog 用的提示字（HTML 不用）', ';'),
        ('FormShow', 48, 48, 'Caption：視窗標題', ';'),
        ('FormShow', 51, 52, 'Left／Top：視窗位置', ';'),
        ('FormShow', 208, 208, 'rbTemp->SetFocus()：焦點', ';'),
        ('FormShow', 146, 147, 'InArmSuck／OutArmSuck：aHotPlateSubstrate.h 與 HTEditList.h 的 TList 重複定義，本 TU 看不到 → FileRW/_KitSuck.cpp 轉接（同一個判斷）',
         'if(FileRW_InOutArmSuckHasIC() || ShuttleHasIC() || IndexHasIC())'),
        ('ReadWriteFile', 797, 879,
         'LoaderUnload_StepMotor：Tray Y 步進速度全經 dmTrayMotor（移植樹沒有 dmTrayMotor，GATE S5）',
         'if(LoaderUnload_StepMotor) filerw::ELTodo("golden cSpeed.cpp:797-879 tray-Y step motor speeds via dmTrayMotor not ported (GATE S5) -- not shown, not saved");'),
        ('spbSaveClick', 1523, 1526, 'MyMessageBox->Close()：訊息框（伺服器端沒有）', ';'),
        ('spbSaveClick', 1730, 1730, 'fShowMessage->sgdSpeedView->Refresh()：主畫面速度表重畫（HTML 端）', ';'),
        ('spbSaveClick', 1739, 1761, 'Recipe Parameter Default 比對（fRPDefault／SearchRecipeParameter 未移植）',
         'if(CosFunction.bRecipeParameterDefault) filerw::ELTodo("golden cSpeed.cpp:1739-1761 Recipe Parameter Default compare not ported");'),
        ('spbSaveClick', 1784, 1784, 'dmTrayMotor->StartSetSpeed()：步進速度推到驅動器（移植樹沒有 dmTrayMotor）',
         'if(LoaderUnload_StepMotor) filerw::ELTodo("golden cSpeed.cpp:1784 dmTrayMotor->StartSetSpeed not ported");'),
        # AI(W906-FRW-S158) 20260927 [W906]：BCB 的 `tbAllSpeed->Position+=10` 是屬性：讀 Position、加、寫回（寫入走 SetPosition，
        #   夾 Min..Max、值有變就觸發 OnChange＝tbAllSpeedChange）。filerw::ELTrackBar::Pos 只有 operator=／operator int
        #   （FileRW/_EditList.h，不是這次能改的檔）→ 逐行展開成同一件事：讀→算→寫回。整數除法同 BCB（int 屬性）。
        ('spbSpeedAddClick', 1416, 1416, 'Position+=10：屬性複合指派 → 讀、加、寫回（ELTrackBar::Pos 沒有 +=）',
         'EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position = EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position + 10;'),
        ('spbSpeedAddClick', 1417, 1417, 'Position/=10：同上（int 除法）',
         'EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position = EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position / 10;'),
        ('spbSpeedAddClick', 1418, 1418, 'Position*=10：同上',
         'EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position = EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position * 10;'),
        ('spbSpeedDecClick', 1423, 1423, 'Position-=10：同上',
         'EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position = EL<filerw::ELTrackBar>("TfSpeed", "tbAllSpeed")->Position - 10;'),
        # golden 直接呼叫三支處理器並傳 this（Sender 沒有人讀）；產生的 static 函式沒有 this、參數是 ''
        ('spbSetToDefClick', 1818, 1820, 'spbSelectAllClick(this)／spbSpeedAddClick(this)／tbAccSpeedChange(this)：直接呼叫處理器（Sender 不用）',
         'SP_spbSelectAllClick(); SP_spbSpeedAddClick(); SP_tbAccSpeedChange();'),
    ],
    'includes': ['cprod.h', 'Config.h', 'LastSet.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'forms/fMain.h', 'forms/fSecurity.h', 'forms/fObserver.h', 'csystem.h',
                 'SECSGEM/SecsEventType.h', 'SECSGEM/SecsEventReport.h'],
    # cAuthority.h 帶進 language.h（TWinControl 與 Public/HTEdit.h 重複）→ 只前置宣告要用的
    'decls': ['bool FileRW_InOutArmSuckHasIC();         // FileRW/_KitSuck.cpp（aHotPlateSubstrate.h 與 HTEditList.h 的 TList 重複定義）',
              'extern bool authMainForm[12];        // cAuthority.h:56（golden cAuthority.h）',
              'extern bool bAuthCriticalPara[26];   // cAuthority.h:64（golden cAuthority.h）'],
    'blocks': [],
    'overrides': [],
    # AI(W906-FRW-S158) 20260927 [W906]：WS form.event 事件表（Steven ★ Q40＝A，RULINGS_20260926 S157；Q41 SP-2／SP-3／SP-4）。
    #   golden cSpeed.dfm:533-655 九個勾選框 OnClick＝cbIndexArmClick（:1426）、:504 spbSelectAll、:518 spbSetToDef、:252 spbSpeedAdd、
    #   :365 spbSpeedDec。產生器在 .gen.inc 檔尾產生 kSP_Events，FileRW/ArmSpeed_File.cpp 用 filerw::PageEventsRegistrar 註冊。
    #   三條滑桿的 OnChange（tbAllSpeedChange :1282／tbAccSpeedChange :1345／tbEPControlChange :1408，DFM :69／:124／:707）**不列**：
    #   form.event 的請求沒有 position 欄位（FileRW/_FormEvent.h Request），FileRW/_EditPage.cpp RunPageEvent 第 4 步把控制項自己
    #   從 state 拿掉、第 5 步不套 TTrackBar 的值 ⇒ 處理器只看得到伺服器端的舊 Position。滑桿改由存檔前重播
    #   （FileRW/ArmSpeed_File.cpp BeforeApply）。
    # AI(W906-EVB1) 20260928 [W906] X-2 ⛔ 更正：上面「不列」已過期 —— form.event 多了 "position"（FileRW/_FormEvent.h；
    #   RunPageEvent 先照 VCL 設 Position 再跑處理器），三條滑桿的 OnChange 接在表尾（既有 13 列的產生碼不動）：
    #   tbAllSpeed → tbAllSpeedChange（golden cSpeed.cpp:1282（0618＝V912），DFM :69）、tbAccSpeed → tbAccSpeedChange（:1345，DFM :124）、
    #   tbEPControl → tbEPControlChange（:1408，DFM :707）。存檔前重播（BeforeApply）照留：頁面沒送事件就存時還是靠它
    #   （送過事件的滑桿值已同步，BeforeApply 看到一樣就跳過）。
    'events': [('cbIndexArm', 'click', 'cbIndexArmClick'), ('cbInArm', 'click', 'cbIndexArmClick'),
               ('cbOutArm', 'click', 'cbIndexArmClick'), ('cbShuttle', 'click', 'cbIndexArmClick'),
               ('cbTrayArm', 'click', 'cbIndexArmClick'), ('cbInArmZ', 'click', 'cbIndexArmClick'),
               ('cbOutArmZ', 'click', 'cbIndexArmClick'), ('cbInRotate', 'click', 'cbIndexArmClick'),
               ('cbOutRotate', 'click', 'cbIndexArmClick'),
               ('spbSelectAll', 'click', 'spbSelectAllClick'), ('spbSetToDef', 'click', 'spbSetToDefClick'),
               ('spbSpeedAdd', 'click', 'spbSpeedAddClick'), ('spbSpeedDec', 'click', 'spbSpeedDecClick'),
               ('tbAllSpeed', 'change', 'tbAllSpeedChange'), ('tbAccSpeed', 'change', 'tbAccSpeedChange'),   # AI(W906-EVB1) 20260928 [W906] X-2
               ('tbEPControl', 'change', 'tbEPControlChange')],
}
