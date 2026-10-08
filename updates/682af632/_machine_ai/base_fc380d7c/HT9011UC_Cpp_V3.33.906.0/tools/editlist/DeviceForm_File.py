# -*- coding: utf-8 -*-
# tools/editlist/DeviceForm_File.py -- gen_editlist.py 的結構設定（一個 C 形狀結構一個檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only DeviceForm_File
#
# Steven 20260925：DeviceForm_File（<recipe>\Contact.Data、Position Offset.Data；CosFunction.bContactHeightSaveToContactIni
# 時高度寫 system\Contact.ini）—— golden TfContact（cContact.cpp，23062 行）。A 形狀（tools/formbridge/_retired/TfContact.py）退役。
#
# golden 開頁＝FormShow（:1094，內含 ReadFile＋DoIniDataToForm＋SetContactMode＋ShowArmAndDeviceForce＋scrbSLKChange）；
# 存檔鈕＝spbSaveClick（:14179）→ CheckContactSettingChange（SECS）→ SaveSetupFile（:14313）→ ReadFile＋DoIniDataToForm。
# elContact（HTEditList）golden 只有 new（main.cpp:1534），沒有任何 Add() —— 清單是空的，呼叫照留（no-op）。
#
# 不收養（adopt）fContactForm：forms/fContact.h 的元件是 `new TPanel()` 形式（不是 `new vclcompat::T()`），
# 多個還是門面自訂型別（TfContactRadioGroup／TfContactGroupBox…），而且全樹除了 wb_serve 呼叫 ReadFile 外沒有人讀
# fContactForm 的元件（grep 20260925）。只有 CarlibrationTask（Auto Height／Contact Test 狀態機 task，public）
# 接到移植樹那一份。fShow 刻意不接：移植樹執行期讀的是 fContact（TfContactShim）->fShow，網頁開頁沒有關頁事件，
# 接過去會讓執行期以為 Contact 表單一直開著。
#
# TScrollBar（scrbSLK）：產生器 MAPPED 沒有 TScrollBar → 退回 TControl（沒有 Position／Max）。
# 這裡用 replace 明寫 EL<filerw::ELTrackBar>（VCL 語意：夾 Min..Max、值變觸發 OnChange），
# FileRW/DeviceForm_File.cpp 開機時第一個建立它（DFM Min=2 Max=5 Position=2，OnChange=scrbSLKChange）。
# 整合者若在 MAPPED 加 'TScrollBar': 'filerw::ELTrackBar'，這些 replace 仍然正確。
#
# Steven 20260925（第二輪，Steven 指示「不要拒存，把公式移植過來」）：
#   golden 存的 "Torque Control/Torque"＝edAirForce＝ShowArmAndDeviceForce（:1925）的 iTotalGf＝CalculateTotalAirForce（:18966）。
#   CalculateTotalAirForce／GetMaxIndexForceLimit／GetMinForce／CountDieForceKg 與 DFM 綁的 OnChange／OnClick 全部照 golden 轉；
#   fContactForce->SLKClass／DieForceSLKClass 經 decls 的薄轉接接到 ContactForceTables()（原文照抄）。
#   頁面只送畫面最後狀態 → 存檔前由 FileRW/DeviceForm_File.cpp 依 golden 事件順序從主輸入重算衍生欄位（見該檔）。
KIT = 'EL<TRadioGroup>("TfContact", "rgKitDiameter")'
OUTKIT = 'EL<TRadioGroup>("TfContact", "rgOutKitDiameter")'
DIEKIT = 'EL<TRadioGroup>("TfContact", "rgDieForceKitDiameter")'
SLK = 'EL<filerw::ELTrackBar>("TfContact", "scrbSLK")'

# AI(W906-EVB3) 20260928 [W906]：批次 B3 CT-1／CT-L1（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md）——
#   golden 原文讀進來，給新加的 replace 找行（_line）與釘住原文（_expect）；golden 改了，產生器停下來。
import os   # noqa: E402

_DF_CPP = open(os.path.join(r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy', 'cContact.cpp'), 'rb').read() \
    .decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')


def _expect(gl, text):
    """golden cContact.cpp 第 gl 行（去掉 // 註解與頭尾空白）要等於 text，否則中止。"""
    s = _DF_CPP[gl - 1].split('//')[0].strip()
    if s != text:
        raise SystemExit('DeviceForm_File.py: golden cContact.cpp:%d is %r, expected %r' % (gl, s, text))
    return gl


def _head(meth):
    for i, ln in enumerate(_DF_CPP):
        if 'TfContact::' + meth + '(' in ln.split('//')[0]:
            return i + 1
    raise SystemExit('DeviceForm_File.py: golden TfContact::%s not found' % meth)


_SA = _head('pnlSensorAdjClick')                                                 # golden V912 :14171
_expect(_SA + 2, 'if(fSecurity->Insufficient(109)==false)')                     # :14173 等級 109（CT-L1）
_expect(_SA + 3, 'return;')
_SA_CC1 = _expect(_SA + 4, 'fCCLink->ShowUseSensor(TestIF.iTestMode, TestIF.dSiteXPitch);')   # :14175
_SA_CC2 = _expect(_SA + 5, 'fCCLink->Show();')                                 # :14176
_expect(_SA + 6, '}')

# AI(W906-EVB10A) 20260929 [W906]：CT-3b（事件批次 B10 part a，D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md B10 表；
#   Steven 20260928「如果沒有移植的, 我們直接實作」、20260929「照 BCB 的邏輯」）—— golden TfContact::FormClose（V912 cContact.cpp:1842-1923）
#   照產生器轉；呼叫端 FileRW/DeviceForm_File.cpp 檔尾（頁面表的關窗邊緣；golden Contact 沒有 ✕，cContact.cpp:112 BorderIcons=TBorderIcons()，
#   只能按 sbtExit → sbtExitClick :14586 → Close()）。下面釘住原文、只換伺服器端接不上的幾行（其餘照 golden：SystemStart=false 停機、
#   iGPIBIndexStatus、燈、CarlibrationTask!=1 ⇒ fAllMotorHome=false、iContactMode 回 NORMAL、SetContactMode…）。
_FC = _head('FormClose')                                                          # golden V912 :1842
_expect(_FC + 8, 'SystemStart=false;')                                           # :1850（Form Close時,機台要停住!!）
_FC_RESTORE = (_expect(_FC + 24, 'if(iContactMode!=CONTACT_NORMAL)'),            # :1866
               _expect(_FC + 37, '}'))                                           # :1879（if 的收尾）
_expect(_FC + 33, 'FTestSuck.SetItemData(i, j, iFTestBackItem[i][j]);')          # :1875
_FC_TRAYHAS = _expect(_FC + 42, 'if(MOT[MMTrayY].HasIC())')                    # :1884
_FC_CLEARTRAY = _expect(_FC + 44, 'MOT[MMTrayY].ClearTray(__FUNC__);')           # :1886
_FC_RECORD = _expect(_FC + 53, 'NewRecordProcess("MES2170", "Exit Contact");')  # :1895
_FC_KIT = (_expect(_FC + 62, 'FLCarryKit.SetAll(NULL_IC);'),                     # :1904
           _expect(_FC + 63, 'BLCarryKit.SetAll(NULL_IC);'))                     # :1905
_FC_CCLINK = _expect(_FC + 70, 'fCCLink->bShow==false)')                         # :1912
_FC_PAUSE = _expect(_FC + 71, 'MyEtherCAT->Pause();')                            # :1913
# 開頁 FormShow :1388-1404 的備份：原本在 blocks（TList 衝突、「還原那一半 FormClose 網頁不跑」）；FormClose 接上之後兩半一起經 FileRW/_KitSuck.cpp 做。
#   golden 每次 Show() 才跑 FormShow；網頁存檔後引擎會重讀（也是 FormShow）⇒ 只在「這一次開窗第一次讀」備份（filerw::PageShownNow 還是 false），
#   不然存完再關窗會還原成存檔當下（Contact 模式跑到一半）的狀態。
_FS_BACKUP = [('FormShow', _expect(1388, 'for(int i=0; i<FTestSuck.iMaxRow; i++)'), _expect(1404, '}'),
               'iFTestBackItem/iBTestBackItem 備份 FTestSuck/BTestSuck.Item（aHotPlateSubstrate.h 與 HTEditList.h 的 TList 衝突 → FileRW/_KitSuck.cpp 轉接，'
               '原文照抄；還原在 FormClose :1866-1879，CT-3b）；同一次開窗的重讀不再備份',
               'if(!filerw::PageShownNow("DeviceForm_File")) FileRW_Contact_TestSuckBackup();')]
_FC_REPLACE = [
    ('FormClose', _FC_RESTORE[0], _FC_RESTORE[1],
     'iContactMode!=CONTACT_NORMAL 的收尾：fLtcSensor->GetLtcSensor(1／0) 清 Latch＋FTestSuck／BTestSuck.SetItemData 還原開頁備份 —— '
     'TfLtcSensor（acarry_shims.h）與 TMyKitSuck（aHotPlateSubstrate.h）和 HTEditList.h 衝突 → FileRW/_KitSuck.cpp 轉接（同一段原文、備份陣列也在那裡；'
     '開頁的備份 :1388-1404 同一支轉接）',
     'if(iContactMode!=CONTACT_NORMAL) FileRW_Contact_TestSuckRestore();'),
    ('FormClose', _FC_TRAYHAS, _FC_TRAYHAS, 'MOT[MMTrayY].HasIC()：Motor/mymotor.h 會多帶 43 個 HTMotor.h 警告進這個 TU → FileRW/_KitSuck.cpp 轉接',
     'if(FileRW_MotTrayHasIC(MMTrayY))'),
    ('FormClose', _FC_CLEARTRAY, _FC_CLEARTRAY, '__FUNC__（BCB6 巨集＝函式名字串）→ 同一個字串；MOT[] 經 FileRW/_KitSuck.cpp 轉接（同上）',
     'FileRW_MotClearTray(MMTrayY, "TfContact::FormClose");'),
    ('FormClose', _FC_RECORD, _FC_RECORD, 'NewRecordProcess 的第三參數預設值（cMyDB.h:129 Debug=" "）：本 TU 只前置宣告（不 include cMyDB.h），寫明',
     'NewRecordProcess("MES2170", "Exit Contact", " ");'),
    ('FormClose', _FC_KIT[0], _FC_KIT[1],
     'FLCarryKit／BLCarryKit.SetAll(NULL_IC)：TMyKitSuck（aHotPlateSubstrate.h 與 HTEditList.h 衝突）→ FileRW/_KitSuck.cpp 轉接（原文照抄）',
     'FileRW_CarryKitsSetAllNull();'),
    ('FormClose', _FC_CCLINK, _FC_CCLINK,
     'fCCLink->bShow（Shuttle Sensor Utility 開著沒）：移植樹沒有 TfCCLink → 問頁面表（W906_FormShowing，golden 表單名 fCCLink ＝ 網頁視窗 cclink）',
     'W906_FormShowing("fCCLink", false)==false)'),
    ('FormClose', _FC_PAUSE, _FC_PAUSE,
     'MyEtherCAT->Pause()：移植樹的 MyEtherCAT（EtherCAT/MyEtherCAT.cpp:147）沒有人 new（golden 開機依 SHUTTLE_SENSOR_TYPE 建）→ NULL 時記 todo，不解參考',
     '{ if(MyEtherCAT!=NULL) MyEtherCAT->Pause(); else filerw::ELTodo("golden cContact.cpp:1913 MyEtherCAT->Pause() not done -- MyEtherCAT is never created in the port (EtherCAT/MyEtherCAT.cpp:147)"); }'),
]

# AI(W906-B8-CTL2) 20260930 [W906]：B8 CT-L2（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「CT-L2」；
#   Steven 20260928「任何畫面的事件, 都是我們做」、20260929「請按照bcb的邏輯處理」）—— golden TfContact::TimerEPTimer（V912 cContact.cpp:17177-17232，
#   DFM cContact.dfm:18634 TimerEP：Enabled=False、沒寫 Interval＝VCL 預設 1000 ms；FormShow 設 Enabled=(EP_Install==3 || EP_Install==5)，FormClose 清）。
#   照產生器轉（方法加在 methods 最後，既有方法的產生碼不動）；呼叫端 FileRW/DeviceForm_File.cpp 檔尾 FileRW_Contact_TimerEPTick（TimerEP->Enabled 才跑一拍）。
#   三段：
#     :17180-17189 EP 讀值（labEPValue／labEPValue_1032 ← ADAM_ReadPA）：ADAM-6024 回讀在移植樹沒有本體（Command.cpp:2934 GATE(FW3-WA) 同一件，
#                  B8 CT-3 列）→ replace 成只留 (void)dValue。labEPValue 開頁 Visible=false（FormShow :1649-1650），只有 labMPaDblClick 會打開 ⇒ 畫面不變。
#     :17191-17217 RTC Auto Tuning 顯示（CT-L2 本體）：照 golden。只換一個成員的家：golden COM2->bRTCVerSupportAutoTurnning（rs232.h:156 TCOM2 成員）
#                  在移植樹的 COM2（TCOM2Shim，atester_shims.h:373，jimmychiu 的檔）沒有 ⇒ 放在 St01 的 FileRW/DeviceForm_File.cpp（W906_COM2_bRTCVerSupportAutoTurnning，
#                  照 golden 開機值 false，golden rs232.cpp:119）。條件原樣：!COM2->bCCDDummyRum && IniConfig.bD74RTCAutoTuning && fSecurity->Insufficient(176, false)
#                  && 這個成員 —— 不是寫死隱藏。golden 事實：V912 全樹只有 rs232.cpp:119 設它（false），沒有人設 true（20260930 11:39 grep -rl：cContact.cpp、
#                  rs232.cpp、rs232.h 三個檔）⇒ 計時器有跑的機台（EP_Install 3／5）這一格一拍之後一定隱藏。看起來是 golden 沒做完（rs232.cpp:119 的註解
#                  「啟動時傳送 AutoTeach 功能，若沒有回應代表不支援就不要顯示」＝原意是有回應要設 true）—— 照翻，不修。
#                  VCL：程式設 cbRTCAutoTuning->Checked 值有變會觸發 DFM OnClick＝cbRTCAutoTuningClick（:20658，CT-3 列）。DeviceForm_File 沒開 vcl_clicks
#                  （開了會改 DoIniDataToForm 的 rgKitDiameter 等既有產生碼），這三個 Checked 指派照原樣；移植樹到不了「值有變」：Checked 只會被
#                  SIGURD 那一支設 true（要 Visible＝true ⇒ 要這個成員 true，沒有人設）或被頁面值設 true（要可改＝Visible，開頁 Visible=!bCCDDummyRum，
#                  TCOM2Shim 建構子設 bCCDDummyRum=true，移植樹 rs232.cpp:162）⇒ 一律 false→false，golden 也不會觸發。
#     :17218-17231 KYEC_LEE ATC 等待倒數字（lblATCTempWait，Eastsun 20260526 #026-1.5L）：不在 CT-L2（B8 CT-3 列的 TimerEP 那一項）。golden 每秒更新秒數；
#                  網頁只在開頁／事件回覆時拿到替身快照，照抄會顯示一個停住的秒數 ⇒ 擋著（原文留 #if 0），等 P-1 計時器拍子＋推送（CT-3）。
_EP = _head('TimerEPTimer')                                                     # golden V912 :17177
_EP_READ = (_expect(_EP + 3, 'if(EP_Install==3 && fShow==true)'),               # :17180
            _expect(_EP + 12, '}'))                                             # :17189（EP_Install==5 那一段的收尾）
_expect(_EP + 8, 'if(EP_Install==5 && fShow==true)')                            # :17185
_expect(_EP + 14, 'if(REAL_TIME_CCD)')                                          # :17191
_EP_VIS = (_expect(_EP + 16, 'cbRTCAutoTuning->Visible=(!COM2->bCCDDummyRum                  &&'),   # :17193
           _expect(_EP + 19, 'COM2->bRTCVerSupportAutoTurnning);'))             # :17196
_expect(_EP + 17, 'IniConfig.bD74RTCAutoTuning         &&')                     # :17194
_expect(_EP + 18, 'fSecurity->Insufficient(176, false) &&')                     # :17195
_expect(_EP + 21, 'if(bOldRTCAutoTuning!=cbRTCAutoTuning->Visible)')            # :17198
_EP_KYEC = (_expect(_EP + 42, 'if(CUSTOMER_CODE==CC_KYEC_LEE && bEnable_KLT_Function==false && bCheckATCTemp==true &&'),   # :17219
            _expect(_EP + 53, '}'))                                             # :17230
_expect(_EP + 55, '}')                                                          # :17232（方法的收尾）
_EP_REPLACE = [
    ('TimerEPTimer', _EP_READ[0], _EP_READ[1],
     'EP 讀值 labEPValue／labEPValue_1032＝ADAM_ReadPA（ADAM-6024 EP 回讀）：移植樹沒有本體（Command.cpp:2934 GATE(FW3-WA) 同一件，B8 CT-3 列）；'
     'labEPValue 開頁 Visible=false（FormShow :1649-1650，只有 labMPaDblClick 打開）⇒ 畫面不變。dValue 只給它用',
     '(void)dValue;'),
    ('TimerEPTimer', _EP_VIS[0], _EP_VIS[1],
     'B8 CT-L2：條件照 golden；COM2->bRTCVerSupportAutoTurnning（golden rs232.h:156 TCOM2 成員，rs232.cpp:119 開機設 false、V912 沒有人設 true）'
     '在移植樹的 TCOM2Shim 沒有 → FileRW/DeviceForm_File.cpp 的 W906_COM2_bRTCVerSupportAutoTurnning（同開機值）',
     'EL<TCheckBox>("TfContact", "cbRTCAutoTuning")->Visible=(!COM2->bCCDDummyRum && IniConfig.bD74RTCAutoTuning && '
     'fSecurity->Insufficient(176, false) && W906_COM2_bRTCVerSupportAutoTurnning);'),
]
_EP_BLOCKS = [
    ('TimerEPTimer', _EP_KYEC[0], _EP_KYEC[1],
     'KYEC_LEE ATC Temp Wait countdown label (lblATCTempWait, Eastsun 20260526 #026-1.5L): golden refreshes the seconds every 1000 ms; '
     'the page only gets a proxy snapshot at editlist.get / form.event, a copied snapshot would show a frozen count -- waits for the P-1 timer beat + push (B8 CT-3)'),
]

# AI(W906-B8-CT3A) 20261001 [W906] (St01)：B8 CT-3a（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「CT-3」第 8 點第一段；
#   Jimmy RULINGS_20261001 第 0 條「照 golden 翻、會動的也接上」；Steven 20261001 09:4x「需要上機驗證的, 都是請Eastsun處理」）——
#   Contact 頁切模式＋T.Start／T.Step／One Cycle（只設旗標，START 之後流程才照著做）。golden V912 cContact.cpp（cp950；產生器的 golden 根目錄是 V912，本段行號是 V912；906 對照寫在 FileRW/DeviceForm_File.cpp 檔尾同一段的雙列，AI(W906-E030-CITE) 20261003）：
#     rbModeNormalClick :15555-15564（11 顆模式單選共用，cContact.dfm:18125-18318 OnClick=rbModeNormalClick，父層 rgHandlerMode :18102）：
#       ASE 高雄選 Auto Height ⇒ fAllMotorHome=false（下一次 START 全部回原點）→ SetContactMode :15566-15696（設全域 iContactMode；KYEC_CHEN＋A16 另叫 ChangeContactMode）
#     btnTStartClick :2280-2283 bSetupStart=true；btnTStepClick :2285-2288 bSetupStep=true（流程 ckernel.cpp:96-131／:54-94 在等）
#     spbOneCycleClick :17152-17156 → OneCycleProcess :11770-11775（bContinueContact 反相；iContactMode<CONTACT_TEST 一律 false）＋ledOneCycle 燈
#   旗標寫到哪一個物件：bSetupStart／bSetupStep 改指 fContact（TfContactShim，atester_shims.h:222-223、:251，jimmychiu）—— ckernel.cpp:255／:324 讀的那一份
#     （見 members）。AI(W906-W156) 20261007 (St02-E)：bContinueContact 只有一份——members 的 #define 指到 fContactForm->bContinueContact（W-156 (a)）；golden 讀它的是 Contact 狀態機（Do_Z1_AutoGetHeight :7106 等、DoTestContactFunction :11804／:13394），
#     移植樹 E-042 已翻（forms/fContact_AutoHeight.cpp／fContact_ContactSM.cpp），W-152 起 MainProc 在 HT9050 PCI1203 Index Z1 上呼叫（csystem.cpp:31468）；forms/fContact.h:1340 同一行改成 public、
#     TfContactShim 沒有這個成員 ⇒ 頁面 One Cycle／面板 ONE CYCLE 改的就是狀態機讀的那一份。
#   VCL 語意（vclcompat TRadioButton 只是欄位）：程式或使用者把單選設成 Checked=true ⇒ TRadioButton.SetChecked(True)：值有變才 TurnSiblingsOff（同一個父層
#     rgHandlerMode 的其他顆取消）＋Click→OnClick。FormShow :1178、FormClose :1898 的 rbModeNormal->Checked=true 與網頁點選都經 FileRW_Contact_RbClickTrue
#     （FileRW/DeviceForm_File.cpp 檔尾；「哪一顆勾著」記在那裡，不信頁面帶的 checked）。golden 開頁 :1178＋:1179 ⇒ 每次開 Contact 都回 Normal（照 golden）。
#   存檔後的重讀（引擎在存檔後一定 editlist.get＝再跑一次 FormShow；filerw::PageShownNow 還是 true）不是 golden 的 FormShow：golden 存檔 spbSaveClick
#     只 ReadFile＋DoIniDataToForm（:14262-14263），不碰模式單選、iContactMode、bSetupStart／bSetupStep、bContinueContact ⇒ 重讀時 :1178、:1180、:1187、:1235、:1360
#     不做（同上面 _FS_BACKUP 的「同一次開窗的重讀」規則）；:1179 SetContactMode 照跑（從還原的單選算出同一個 iContactMode）。
#   燈號 ledOneCycle（TMyLed → 替身 TControl，沒有 Value）：值放在替身的 Tag（ProxyStateJson 對 TControl 一律帶 tag），頁面 ht9045_contact_ev.js 依 tag 點燈。
_CT3A_RB = ['rbModeNormal', 'rbAutoHeight', 'rbManualHeight', 'rbContactTest', 'rbAutoContactTest', 'rbStepContactTest',
            'rbDeviceMapping', 'rbLoadCellAutoHigh', 'rbKTempIndexMove', 'rbDeviceLoopTest', 'rbVisualDetectionTest']   # cContact.dfm:18125-18318（rgHandlerMode 底下全部，順序＝DFM）
_CT3A_EVENTS = [(n, 'click', 'rbModeNormalClick') for n in _CT3A_RB] + \
               [('btnTStart', 'click', 'btnTStartClick'),       # cContact.dfm:16136 OnClick=btnTStartClick
                ('btnTStep', 'click', 'btnTStepClick'),         # cContact.dfm:16146 OnClick=btnTStepClick
                ('spbOneCycle', 'click', 'spbOneCycleClick')]   # cContact.dfm:15887 OnClick=spbOneCycleClick
_RB = _head('rbModeNormalClick')                                                 # golden V912 :15555（906 :15330）
_expect(_RB + 2, 'TRadioButton *Ptr;')                                           # :15557
_expect(_RB + 3, 'Ptr=(TRadioButton *)Sender;')                                  # :15558
_CT3A_ASE = _expect(_RB + 4, 'if(CUSTOMER_CODE==CC_ASE_KaohSiung && Ptr->Name=="rbAutoHeight")')   # :15559
_expect(_RB + 6, 'fAllMotorHome=false;')                                         # :15561
_expect(_RB + 8, 'SetContactMode();')                                            # :15563
_TS = _head('btnTStartClick')                                                    # :2280
_expect(_TS + 2, 'bSetupStart=true;')                                            # :2282
_TP = _head('btnTStepClick')                                                     # :2285
_expect(_TP + 2, 'bSetupStep=true;')                                             # :2287
_OC = _head('OneCycleProcess')                                                   # :11770
_expect(_OC + 2, 'bContinueContact=!bContinueContact;')                         # :11772
_expect(_OC + 3, 'if(iContactMode<CONTACT_TEST)')                               # :11773
_expect(_OC + 4, 'bContinueContact=false;')                                      # :11774
_SO = _head('spbOneCycleClick')                                                  # :17152
_expect(_SO + 2, 'OneCycleProcess();')                                           # :17154
_CT3A_LED = _expect(_SO + 3, 'ledOneCycle->Value=bContinueContact;')             # :17155
_CT3A_FS_RB = _expect(1178, 'rbModeNormal->Checked=true;')                       # FormShow :1178
_expect(1179, 'SetContactMode();')                                               # :1179
_CT3A_FS_START = _expect(1180, 'bSetupStart=false;')                             # :1180
_CT3A_FS_STEP = _expect(1187, 'bSetupStep=false;')                               # :1187
_CT3A_FS_CONT = _expect(1235, 'bContinueContact=false;')                         # :1235
_CT3A_FS_MODE = _expect(1360, 'iContactMode=CONTACT_NORMAL;')                    # :1360（FormShow 第二次回 Normal，在 :1179 之後）
_expect(1633, 'ledOneCycle->Value=false;')                                       # :1633（replace 在 STRUCT 裡那一列）
_CT3A_FC_RB = _expect(_FC + 56, 'rbModeNormal->Checked=true;')                   # FormClose :1898
_expect(_FC + 55, 'iContactMode=CONTACT_NORMAL;')                                # :1897
_expect(_FC + 57, 'SetContactMode();')                                           # :1899
_CT3A_REREAD = 'filerw::PageShownNow("DeviceForm_File")'
_CT3A_LED_SHOW = 'EL<TControl>("TfContact", "ledOneCycle")->Tag=(%s ? (int)bContinueContact : 0);' % _CT3A_REREAD
_CT3A_REPLACE = [
    ('rbModeNormalClick', _CT3A_ASE, _CT3A_ASE,
     'Ptr->Name：vclcompat TControl 沒有 Name；替身的身分就是名字（EL<>(表單, 名稱) 永遠同一個物件）⇒ 比指標，條件照 golden',
     'if(CUSTOMER_CODE==CC_ASE_KaohSiung && Ptr==EL<TRadioButton>("TfContact", "rbAutoHeight"))'),
    ('FormShow', _CT3A_FS_RB, _CT3A_FS_RB,
     'rbModeNormal->Checked=true：VCL SetChecked(True)（值有變 ⇒ TurnSiblingsOff＋OnClick rbModeNormalClick）；存檔後的重讀不做（golden 存檔不碰模式單選，'
     ':14262-14263），只把這一次開窗最後點的那顆放回替身',
     'if(!%s) FileRW_Contact_RbClickTrue(EL<TRadioButton>("TfContact", "rbModeNormal")); else FileRW_Contact_RbReimpose();' % _CT3A_REREAD),
    ('FormShow', _CT3A_FS_START, _CT3A_FS_START, 'bSetupStart=false（fContact，見 members）：存檔後的重讀不做（golden 存檔不碰）',
     'if(!%s) bSetupStart=false;' % _CT3A_REREAD),
    ('FormShow', _CT3A_FS_STEP, _CT3A_FS_STEP, 'bSetupStep=false（fContact，見 members）：存檔後的重讀不做（golden 存檔不碰）',
     'if(!%s) bSetupStep=false;' % _CT3A_REREAD),
    ('FormShow', _CT3A_FS_CONT, _CT3A_FS_CONT, 'bContinueContact=false：存檔後的重讀不做（golden 存檔不碰）',
     'if(!%s) bContinueContact=false;' % _CT3A_REREAD),
    ('FormShow', _CT3A_FS_MODE, _CT3A_FS_MODE, 'iContactMode=CONTACT_NORMAL（FormShow 第二次回 Normal）：存檔後的重讀不做（golden 存檔不碰 iContactMode）',
     'if(!%s) iContactMode=CONTACT_NORMAL;' % _CT3A_REREAD),
    ('FormClose', _CT3A_FC_RB, _CT3A_FC_RB, 'rbModeNormal->Checked=true：VCL SetChecked(True)（值有變 ⇒ TurnSiblingsOff＋OnClick rbModeNormalClick）',
     'FileRW_Contact_RbClickTrue(EL<TRadioButton>("TfContact", "rbModeNormal"));'),
    ('spbOneCycleClick', _CT3A_LED, _CT3A_LED, 'ledOneCycle->Value：TMyLed 燈號 → 替身 TControl 的 Tag（頁面依 tag 點燈）',
     'EL<TControl>("TfContact", "ledOneCycle")->Tag=bContinueContact;'),
]

# AI(W906-B8-CT3B) 20261001 [W906] (St01)：B8 CT-3b'（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「CT-3」第 8 點第二段；
#   Jimmy RULINGS_20261001 第 0 條「照 golden 翻、會動的也接上」；Steven 20261001 09:4x「需要上機驗證的, 都是請Eastsun處理」）——
#   Contact 頁的 START／PAUSE。golden V912 cContact.cpp（cp950；產生器的 golden 根目錄是 V912，本段行號是 V912；906 對照寫在 FileRW/DeviceForm_File.cpp 檔尾同一段的雙列，AI(W906-E030-CITE) 20261003）：
#     btnStartClick :14072-14077 → fMain->BtnStartClick(fMain)（主畫面 START 鈕的 OnClick，V912 main.cpp:6529-6593）＋fMain->edTorue0／edTorue1->Text="10"
#     btnPauseClick :14079-14082 → fMain->BtnPauseClick(fMain)（main.cpp:7387-7393 → Pause("BtnPauseClick") main.cpp:6595）
#   兩支都不在處理器裡（持 FormLock）做：BtnStartClick → TfMain::Start 會開 YES／NO 框等瀏覽器回答；Pause 經 SetRunStartMode(rsmAutoSiteMap)
#     （[I50] Pause 觸發 ASM）在 SCK ART 機台會走到 ShowMyMessage("No Run ART Mode")（移植樹 RunStartMode.cpp:526，golden 同）——也是等瀏覽器。
#     ⇒ 處理器只登記 form.event after-ack 動作（FileRW/_FormEvent.cpp 檔尾，同 B8 OS-1b），wb_serve 送出回覆之後、鎖外、同一條主迴圈執行緒照 golden 順序跑
#     （本體 FileRW/DeviceForm_File.cpp 檔尾：W906_Contact_B8BtnStartClick／W906_Contact_B8BtnPauseClick）。中間沒有 MainProc 拍子。
#   顯示照 golden：[D16] Step Contact Test（FormShow :1628-1629）、SOFT_SIMULTE 一律看得見（:1662-1663）——產生檔本來就照翻，這裡不動。
_CT3B_EVENTS = [('btnStart', 'click', 'btnStartClick'),           # cContact.dfm:236-245 OnClick=btnStartClick（pnlBottom，Visible=False）
                ('btnPause', 'click', 'btnPauseClick')]           # cContact.dfm:246-255 OnClick=btnPauseClick
_SC = _head('btnStartClick')                                                     # golden V912 :14072（906 :13965）
_CT3B_SC1 = _expect(_SC + 2, 'fMain->BtnStartClick(fMain);')                    # :14074
_expect(_SC + 3, 'fMain->edTorue0->Text="10";')                                  # :14075
_CT3B_SC2 = _expect(_SC + 4, 'fMain->edTorue1->Text="10";')                     # :14076
_expect(_SC + 5, '}')                                                            # :14077
_PC = _head('btnPauseClick')                                                     # :14079
_CT3B_PC = _expect(_PC + 2, 'fMain->BtnPauseClick(fMain);')                     # :14081
_expect(_PC + 3, '}')                                                            # :14082
_CT3B_REPLACE = [
    ('btnStartClick', _CT3B_SC1, _CT3B_SC2,
     'fMain->BtnStartClick(fMain)＋edTorue0／edTorue1->Text="10"：移植樹 TfMain 沒有 BtnStartClick（golden main.cpp:6529-6593 的翻譯在 FileRW/DeviceForm_File.cpp 檔尾），'
     '而且它會走到 TfMain::Start 的 YES／NO 框 ⇒ 三行照 golden 順序在回覆之後跑（form.event after-ack；edTorue0／edTorue1 是 forms/fMain.h:1262-1263 的真成員）',
     'W906_Contact_B8BtnStartClick();'),
    ('btnPauseClick', _CT3B_PC, _CT3B_PC,
     'fMain->BtnPauseClick(fMain)：移植樹 TfMain::BtnPauseClick（forms/fMain.cpp:497）→ Pause("BtnPauseClick")（:246）→ W906_RemoteRunPause → TfMainWeb::PauseFromWeb；'
     'Pause 在 SCK ART 機台會走到 ShowMyMessage（RunStartMode.cpp:526）⇒ 同一行在回覆之後跑（form.event after-ack，本體 FileRW/DeviceForm_File.cpp 檔尾）',
     'W906_Contact_B8BtnPauseClick();'),
]

# AI(W906-B8-CT3D) 20261001 [W906] (St01)：B8 CT-3d（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「CT-3」第 8 點第四段；
#   Jimmy RULINGS_20261001 第 0 條「照 golden 翻、會動的也接上」；Steven 20261001 09:4x「需要上機驗證的, 都是請Eastsun處理」）——
#   Contact 頁 OTD（One Touch Docking，測試頭 Dock 鎖）兩顆面板＋OTDTimer。golden V912 cContact.cpp（cp950；產生器的 golden 根目錄是 V912，本段行號是 V912；906 對照寫在 FileRW/DeviceForm_File.cpp 檔尾同一段的雙列，AI(W906-E030-CITE) 20261003）：
#     palOTD_4Click :15395-15418（"OTD Under 240KG"，cContact.dfm:16329-16346 OnClick=palOTD_4Click）、
#     palOTD_6Click :15420-15443（"OTD Over 360KG"，cContact.dfm:16355-16372 OnClick=palOTD_6Click）
#       → Cylinder[C_DockYAxisOn／C_DockYAxisOff／C_DockXAxisOn／C_DockXAxisOff].On()／Off()。golden 沒有任何檢查（不查 SystemStart、Dock 感測器、
#       門）；兩支各有自己的 static bDown（按一下 On 組、再按一下回 Off 組），照抄。
#     OTDTimerTimer :15445-15507（cContact.dfm:18627 OTDTimer：Enabled=False、Interval=100；FormShow :1646 Enabled=(USE_OTD==1)，FormClose :1844 關）
#       → ledOTD 燈＋兩個輸出點 SW[SwUnDock]／SW[SwDockError]。bBusy 或 InitialOK==false 不做；IO 畫面開著不做（:15457）。
#     顯示：palOTD_4／palOTD_6 Visible=(USE_OTD==1)（FormShow :1329-1330，產生檔本來就照翻）—— 開發機 D:\HT9045\system\Gerneral.ini:54 USE_OTD=2
#       ⇒ 面板看不到、點不到（RunPageEvent 看得見才點得到），計時器也不開。
#   只換伺服器端接不上的幾行（其餘照 golden，Cylinder／SW 是移植樹的真物件 mycylin.h:176／myswitch.h:43）：
#     BevelOuter（面板按下去的樣子）：vclcompat TPanel 沒有 → 替身的 Tag＝VCL TBevelCut 的值（DF_bvLowered=1／DF_bvRaised=2；0＝還沒點過＝DFM 預設 bvRaised）。
#       用 1／2 不用 0：TPanel 的 tag 只在非 0 時進替身快照（FileRW/_EditList.cpp PutProxyValue），回到 0 頁面收不到。
#     ledOTD（TMyLed → 替身 TControl，沒有 TrueColor／Value）→ Tag（DF_ledOTDOff=0 滅、DF_ledOTDLime=1 綠、DF_ledOTDRed=2 紅）。
#       golden 每一支亮燈都是 TrueColor＋Value=true 一起設、滅燈只設 Value=false（顏色看不到）⇒ 一個 Tag 就夠。
#     fiosetview->fShow → W906_FormShowing("fiosetview", fiosetview->fShow)（同移植樹 adam6024.cpp:941；IO 頁是網頁，TfiosetviewShim::fShow 永遠 false）。
#   ⚠ golden 疑點（照翻、不修、寫明）：
#     (a) :15478 第二個 OnSensor 是 C_DockYAxisOn（看起來應該是 C_DockXAxisOn）——第一支 if（:15465）已經排除兩顆 On 感測器為 true，所以這個筆誤不改結果。
#     (b) :15483-15485 要 C_DockYAxisOff.OnSensor() 同時 ==false 又 ==true ⇒ 永遠不成立；而且同一組 Status（YOn true／XOn false）:15471 先接走＝死碼。
#   計時器拍子（B8 P-1）：同 CT-L2 的 TimerEP，開頁（golden FormShow 之後）與每個 form.event 之後各補一拍（FileRW/DeviceForm_File.cpp 檔尾
#     FileRW_Contact_OTDTimerTick，OTDTimer->Enabled 才跑）；golden 是每 100 ms ⇒ 兩個輸出點只在頁面有動作時更新（交件寫明，P-1 定時拍子接上才同 golden）。
#   運轉中：palOTD_4／palOTD_6 不加 runexc 列（照舊 running）——golden 運轉中按得到（處理器不查、fContact 非模態），流程不等它們 ⇒ 比 golden 嚴；
#     要不要照 golden 放行（2 列例外）是 Steven 的決定（動 Dock 氣缸＝安全判斷）。
_CT3D_EVENTS = [('palOTD_4', 'click', 'palOTD_4Click'),         # cContact.dfm:16346 OnClick=palOTD_4Click（TPanel）
                ('palOTD_6', 'click', 'palOTD_6Click')]         # cContact.dfm:16372 OnClick=palOTD_6Click
_P4 = _head('palOTD_4Click')                                                    # golden V912 :15395（906 :15170）
_expect(_P4 + 2, 'static bool bDown=false;')                                    # :15397
_CT3D_P4 = (_expect(_P4 + 3, 'palOTD_6->BevelOuter=bvRaised;'),                 # :15398
            _expect(_P4 + 8, 'palOTD_4->BevelOuter=bvLowered;'),                # :15403
            _expect(_P4 + 17, 'palOTD_4->BevelOuter=bvRaised;'))                # :15412
for _o, _s in ((9, 'Cylinder[C_DockYAxisOn].On();'), (10, 'Cylinder[C_DockYAxisOff].Off();'), (11, 'Cylinder[C_DockXAxisOn].Off();'),
               (12, 'Cylinder[C_DockXAxisOff].On();'), (18, 'Cylinder[C_DockYAxisOn].Off();'), (19, 'Cylinder[C_DockYAxisOff].On();'),
               (20, 'Cylinder[C_DockXAxisOn].Off();'), (21, 'Cylinder[C_DockXAxisOff].On();')):
    _expect(_P4 + _o, _s)                                                       # :15404-15407／:15413-15416（照抄的氣缸命令）
_expect(_P4 + 23, '}')                                                          # :15418
_P6 = _head('palOTD_6Click')                                                    # :15420
_expect(_P6 + 2, 'static bool bDown=false;')                                    # :15422
_CT3D_P6 = (_expect(_P6 + 3, 'palOTD_4->BevelOuter=bvRaised;'),                 # :15423
            _expect(_P6 + 8, 'palOTD_6->BevelOuter=bvLowered;'),                # :15428
            _expect(_P6 + 17, 'palOTD_6->BevelOuter=bvRaised;'))                # :15437
for _o, _s in ((9, 'Cylinder[C_DockYAxisOn].On();'), (10, 'Cylinder[C_DockYAxisOff].Off();'), (11, 'Cylinder[C_DockXAxisOn].On();'),
               (12, 'Cylinder[C_DockXAxisOff].Off();'), (18, 'Cylinder[C_DockYAxisOn].Off();'), (19, 'Cylinder[C_DockYAxisOff].On();'),
               (20, 'Cylinder[C_DockXAxisOn].Off();'), (21, 'Cylinder[C_DockXAxisOff].On();')):
    _expect(_P6 + _o, _s)                                                       # :15429-15432／:15438-15441
_expect(_P6 + 23, '}')                                                          # :15443
_OT = _head('OTDTimerTimer')                                                    # :15445
_expect(_OT + 2, 'static bool bBusy=false;')                                    # :15447
_expect(_OT + 6, 'if(bBusy || InitialOK==false)')                               # :15451
_CT3D_IOS = _expect(_OT + 12, 'if(fiosetview->fShow)')                          # :15457
_expect(_OT + 20, 'if(Cylinder[C_DockYAxisOn].OnSensor()==true || Cylinder[C_DockXAxisOn].OnSensor()==true)')   # :15465
_expect(_OT + 33, 'Cylinder[C_DockXAxisOn].Status==true && Cylinder[C_DockYAxisOn].OnSensor()==false)')        # :15478 疑點 (a)
_expect(_OT + 39, 'Cylinder[C_DockYAxisOff].OnSensor()==false &&')                                              # :15484 疑點 (b)
_expect(_OT + 40, 'Cylinder[C_DockXAxisOn].Status==false && Cylinder[C_DockYAxisOff].OnSensor()==true)')       # :15485 疑點 (b)
_CT3D_LED = [((_expect(_OT + 22, 'ledOTD->TrueColor=clRed;'), _expect(_OT + 23, 'ledOTD->Value=true;')), 'DF_ledOTDRed', 'clRed＋Value=true（:15467-15468，沒插好）'),
             ((_expect(_OT + 29, 'ledOTD->TrueColor=clLime;'), _expect(_OT + 30, 'ledOTD->Value=true;')), 'DF_ledOTDLime', 'clLime＋Value=true（:15474-15475）'),
             ((_expect(_OT + 35, 'ledOTD->TrueColor=clLime;'), _expect(_OT + 36, 'ledOTD->Value=true;')), 'DF_ledOTDLime', 'clLime＋Value=true（:15480-15481）'),
             ((_expect(_OT + 42, 'ledOTD->TrueColor=clLime;'), _expect(_OT + 43, 'ledOTD->Value=true;')), 'DF_ledOTDLime', 'clLime＋Value=true（:15487-15488，死碼那一支）'),
             ((_expect(_OT + 48, 'ledOTD->Value=false;'), _OT + 48), 'DF_ledOTDOff', 'Value=false（:15493，Dock 全開＝UnDock）'),
             ((_expect(_OT + 53, 'ledOTD->TrueColor=clRed;'), _expect(_OT + 54, 'ledOTD->Value=true;')), 'DF_ledOTDRed', 'clRed＋Value=true（:15498-15499，其他＝錯誤）')]
_expect(_OT + 58, 'SW[SwUnDock].OnOff(bUnDock);')                               # :15503
_expect(_OT + 59, 'SW[SwDockError].OnOff(bError);')                             # :15504
_expect(_OT + 62, '}')                                                          # :15507
_CT3D_PNL = 'EL<TPanel>("TfContact", "%s")->Tag=%s;'
_CT3D_BEVEL = 'BevelOuter：vclcompat TPanel 沒有 → 替身的 Tag＝VCL TBevelCut 值（頁面依 tag 畫凸／凹；見上面 _CT3D_ 說明）'
_CT3D_REPLACE = [
    ('palOTD_4Click', _CT3D_P4[0], _CT3D_P4[0], 'palOTD_6->BevelOuter=bvRaised：' + _CT3D_BEVEL, _CT3D_PNL % ('palOTD_6', 'DF_bvRaised')),
    ('palOTD_4Click', _CT3D_P4[1], _CT3D_P4[1], 'palOTD_4->BevelOuter=bvLowered：' + _CT3D_BEVEL, _CT3D_PNL % ('palOTD_4', 'DF_bvLowered')),
    ('palOTD_4Click', _CT3D_P4[2], _CT3D_P4[2], 'palOTD_4->BevelOuter=bvRaised：' + _CT3D_BEVEL, _CT3D_PNL % ('palOTD_4', 'DF_bvRaised')),
    ('palOTD_6Click', _CT3D_P6[0], _CT3D_P6[0], 'palOTD_4->BevelOuter=bvRaised：' + _CT3D_BEVEL, _CT3D_PNL % ('palOTD_4', 'DF_bvRaised')),
    ('palOTD_6Click', _CT3D_P6[1], _CT3D_P6[1], 'palOTD_6->BevelOuter=bvLowered：' + _CT3D_BEVEL, _CT3D_PNL % ('palOTD_6', 'DF_bvLowered')),
    ('palOTD_6Click', _CT3D_P6[2], _CT3D_P6[2], 'palOTD_6->BevelOuter=bvRaised：' + _CT3D_BEVEL, _CT3D_PNL % ('palOTD_6', 'DF_bvRaised')),
    ('OTDTimerTimer', _CT3D_IOS, _CT3D_IOS,
     'fiosetview->fShow：IO 頁是網頁，TfiosetviewShim::fShow 永遠 false（atester_shims.h:359）→ 問頁面表（W906_FormShowing，同移植樹 adam6024.cpp:941）',
     'if(W906_FormShowing("fiosetview", fiosetview->fShow))'),
] + [('OTDTimerTimer', _r[0], _r[1], 'ledOTD ' + _w + '：TMyLed → 替身 TControl 的 Tag（0 滅／1 綠／2 紅；頁面依 tag 點燈）',
      'EL<TControl>("TfContact", "ledOTD")->Tag=%s;' % _v) for (_r, _v, _w) in _CT3D_LED]

STRUCT = {
    'struct': 'DeviceForm_File',
    'prefix': 'DF',
    'class': 'TfContact',
    'cpp': 'cContact.cpp',
    'h': 'cContact.h',
    'files': ['Contact.Data', 'Position Offset.Data', 'system/Contact.ini (CosFunction.bContactHeightSaveToContactIni)'],
    'lists': ['elContact'],
    'methods': ['TfContact', 'InitContactEdtList', 'FormShow', 'ReadFile', 'SetIndexDownPos', 'CalcDeviceForce',
                'DoIniDataToForm', 'SetContactMode', 'ChangeContactMode', 'ShowArmAndDeviceForce', 'DutCount',
                'scrbSLKChange', 'edAirForceChange', 'InitCarlibrationTask', 'UpdateContactRelative',
                'spbSaveClick', 'CheckContactSettingChange', 'SaveSetupFile',
                # 力量公式（Steven 20260925 追加）：golden 存的 Torque＝edAirForce＝CalculateTotalAirForce
                'CalculateTotalAirForce', 'GetMaxIndexForceLimit', 'GetMinForce', 'CountDieForceKg',
                # golden DFM 綁的 OnChange／OnClick（FileRW/DeviceForm_File.cpp 的存檔前推導照 VCL 事件重跑）
                'edPinCountChange', 'edForcePerPinNChange', 'edDieForcePerPinGChange', 'rgKitDiameterClick',
                'rgOutKitDiameterClick', 'rgDieForceKitDiameterClick', 'chkUseAddWeightClick', 'cbEnableUKClick',
                'cbContactModeChange', 'coD41Change', 'edContactHeight1Change', 'edForcePerDeviceKGClick',
                'pnlSensorAdjClick',    # AI(W906-EVB3) 20260928 [W906]：CT-L1（見 STRUCT 'events'）
                'FormClose',            # AI(W906-EVB10A) 20260929 [W906]：CT-3b golden FormClose（:1842，見上面 _FC_REPLACE）加在最後
                'TimerEPTimer',         # AI(W906-B8-CTL2) 20260930 [W906]：B8 CT-L2 golden TimerEPTimer（:17177，見上面 _EP_REPLACE）再加在最後
                'rbModeNormalClick', 'btnTStartClick', 'btnTStepClick', 'OneCycleProcess', 'spbOneCycleClick', 'btnStartClick', 'btnPauseClick', 'palOTD_4Click', 'palOTD_6Click', 'OTDTimerTimer'],   # AI(W906-B8-CT3A) 20261001 [W906] (St01)：B8 CT-3a（:15555／:2280／:2285／:11770／:17152，見上面 _CT3A_REPLACE）再加在最後   # AI(W906-B8-CT3B) 20261001 [W906] (St01)：+ B8 CT-3b' btnStartClick／btnPauseClick（:14072／:14079，見上 _CT3B_REPLACE），同一行附加   # AI(W906-B8-CT3D) 20261001 [W906] (St01)：+ B8 CT-3d palOTD_4Click／palOTD_6Click／OTDTimerTimer（:15395／:15420／:15445，見上 _CT3D_REPLACE），同一行附加
    'save_methods': ['spbSaveClick', 'CheckContactSettingChange', 'SaveSetupFile'],
    'params': {'TfContact': '', 'FormShow': '', 'spbSaveClick': '', 'scrbSLKChange': '', 'edAirForceChange': '',
               'edPinCountChange': '', 'edForcePerPinNChange': '', 'edDieForcePerPinGChange': '', 'rgKitDiameterClick': '',
               'rgOutKitDiameterClick': '', 'rgDieForceKitDiameterClick': '', 'chkUseAddWeightClick': '', 'cbEnableUKClick': '',
               'cbContactModeChange': '', 'coD41Change': '', 'edContactHeight1Change': '', 'edForcePerDeviceKGClick': '',
               'pnlSensorAdjClick': '',    # AI(W906-EVB3) 20260928 [W906]：golden :14171 不讀 Sender
               'FormClose': '',            # AI(W906-EVB10A) 20260929 [W906]：CT-3b，golden FormClose(TObject*, TCloseAction&) 兩個參數都不讀
               'TimerEPTimer': '',         # AI(W906-B8-CTL2) 20260930 [W906]：B8 CT-L2，golden :17177 不讀 Sender
               'btnTStartClick': '', 'btnTStepClick': '', 'spbOneCycleClick': '', 'btnStartClick': '', 'btnPauseClick': '', 'palOTD_4Click': '', 'palOTD_6Click': '', 'OTDTimerTimer': ''},   # AI(W906-B8-CT3A) 20261001 [W906] (St01)：golden :2280／:2285／:17152 不讀 Sender（rbModeNormalClick 保留 TObject *Sender：:15558 要知道點的是哪一顆）   # AI(W906-B8-CT3B) 20261001 [W906] (St01)：golden :14072／:14079 不讀 Sender，同一行附加   # AI(W906-B8-CT3D) 20261001 [W906] (St01)：+ B8 CT-3d golden :15395／:15420／:15445 不讀 Sender，同一行附加
    'rettype': {'CalcDeviceForce': 'double', 'CheckContactSettingChange': 'bool',
                'CalculateTotalAirForce': 'double', 'GetMaxIndexForceLimit': 'double', 'GetMinForce': 'double'},
    'members': [
        '#define CarlibrationTask (fContactForm->CarlibrationTask)   // golden cContact.h:557 —— 移植樹 fContactForm 那一份（public；存檔守衛讀它）',
        '#define eTeachSHLtcNomal 0                   // golden cContact.h:633 enum eInSHLtcStatus',
        'bool fShow=false;                  // golden cContact.h:549（不接 fContact／fContactForm，見檔頭）',
        '#define bSetupStart (fContact->bSetupStart)   // golden V912 cContact.h:548 —— AI(W906-B8-CT3A) 20261001 [W906] (St01)：流程讀的是 fContact（TfContactShim，atester_shims.h:222／:251，jimmychiu）：ckernel.cpp:324 WaitManualStartKey；本檔的 C 路副本沒有人讀 ⇒ 改指那一份（同 CarlibrationTask 的做法）',
        '#define bSetupStep (fContact->bSetupStep)     // golden V912 cContact.h:555 —— AI(W906-B8-CT3A) 20261001 [W906] (St01)：同上，ckernel.cpp:255 WaitManualStepKey（atester_shims.h:223）',
        'bool bOneCycleFinish=false;        // golden cContact.h:552',
        'double dDutCount=2;                // golden cContact.h:553',
        '#define bSetHasIC (fContactForm->bSetHasIC)   // golden 0618 cContact.h:582（V912 :584）—— AI(W906-W156) 20261007 (St02-E) (b)：只有一份＝fContactForm 那份（取料狀態機 fContact_IndexPickPlace.cpp:340／:365 設、本檔 FormClose 讀；FormShow 的清除見檔尾 W-156 列）',
        'bool bAutoHighFinish=false, MotorStatus=false, bOldRTCAutoTuning=false, brecordmsgLock=false;   // golden cContact.h:400 ／ cContact.h（AI(W906-W156) 20261007 (St02-E)：後三個從下一列搬來，產生檔行數不變）',
        'int iSpeedZ=30, iSpeed=30, TorqueData=0;   // golden cContact.h:401/403/404',
        '#define bContinueContact (fContactForm->bContinueContact)   // golden 0618 cContact.h:406 —— AI(W906-W156) 20261007 (St02-E) (a)：只有一份＝fContactForm 那份（Contact 狀態機讀；頁面 One Cycle／面板 ONE CYCLE 經 OneCycleProcess 改它；forms/fContact.h:1340 同一行改 public）',
        'int RELEASE_UP_BIG=0, RELEASE_UP_SMALL=0;   // golden cContact.h:409-410',
        'int ROILearningTimes=0, iDoArm1PlaceToShuttleTask=0, iDoArm2PlaceToShuttleTask=0, iAutoTeachInSHLtcStatus=0;   // golden cContact.h',
        'double dMinForce=0.0;              // golden cContact.h:627（GetMinForce 寫）',
        'TStringList *sOutDiameter=nullptr; // golden cContact.h:583',
        'AnsiString asContactHeight[2];     // golden cContact.cpp:103 檔案層全域',
        # golden cContact.cpp:73-93 檔案層常數（cContact.h 只有 extern；移植樹沒有 header 帶全套）
        'const int CONTACT_NORMAL=0, CONTACT_AUTO_GET_HEIGHT=1, CONTACT_MANUAL_GET_HEIGHT=2, CONTACT_TEST=3, AUTO_CONTACT_TEST=4, '
        'STEP_CONTACT_TEST=5, CONTACT_LoadCell_AUTO_GET_HEIGHT=8, CONTACT_DEVICE_MAP_CHECK=9, CONTACT_DEVICE_LOOP_TEST=10, '
        'K_TEMP_INDEX_MOVE=11, VISUAL_DETECTION_TEST=12;   // golden cContact.cpp:74-86',
        'const double fIndexDownPos_for9045=-135.0, fIndexDownPos_forATC=-146.0, fIndexDownPos_for9046LS=-148.0;   // golden cContact.cpp:91-93',
    ],
    'globals': ['dOldDieForceKitDiameter'],
    'replace': [
        # ---- 建構子 ----
        ('TfContact', 112, 112, 'BorderIcons：視窗標題列按鈕（HTML 不用）', ';'),
        ('TfContact', 121, 121, 'rgKitDiameter->Caption：群組標題字（vclcompat TRadioGroup 沒有 Caption；HTML 標籤）', ';'),
        ('TfContact', 137, 137, 'rgKitDiameter->Caption：同上', ';'),
        ('TfContact', 172, 183,
         'sOutDiameter：golden cContact.h:583 的 TStringList* 成員（產生器依 header 把它當元件名改寫，改回成員本身；程式照 golden）',
         'sOutDiameter = new TStringList(); sOutDiameter->Clear(); sOutDiameter->CommaText=EL<TEdit>("TfContact", "edOutDiameter")->Text; '
         'for(int i=0; i<sOutDiameter->Count; i++) { sKitOutDiameter[i]=""; '
         'if(sOutDiameter->Strings[i]!="" && atof(sOutDiameter->Strings[i].c_str())>15.0) sKitOutDiameter[i]=sOutDiameter->Strings[i]; } '
         'sOutDiameter->Clear(); delete sOutDiameter;'),
        ('TfContact', 250, 250, 'InitRTCAutoTuning()：RTC Auto Tuning 狀態機 task（執行期狀態，在 fContactForm；它的 Init() 已做）', ';'),
        ('TfContact', 252, 252, 'scrbSLK->Max（TScrollBar → ELTrackBar，見檔頭）', SLK + '->Max=6;'),
        ('TfContact', 254, 254, 'scrbSLK->Max', SLK + '->Max=6;'),
        ('TfContact', 256, 256, 'scrbSLK->Max', SLK + '->Max=5;'),
        ('TfContact', 261, 261, 'gbContactParameter->Height：版面高度（HTML 不用）', ';'),
        ('TfContact', 263, 263, 'gbContactParameter->Height：版面高度', ';'),
        ('TfContact', 267, 267, 'gbContactParameter->Height：版面高度', ';'),
        # ---- ReadFile ----
        ('ReadFile', 529, 529, 'scrbSLK->Position（ELTrackBar）', SLK + '->Position=DeviceForm_File.iHeadDeviceCT;'),
        # ---- DoIniDataToForm ----
        ('DoIniDataToForm', 827, 827, 'scrbSLK->Position（ELTrackBar）', SLK + '->Position=DeviceForm_File.iHeadDeviceCT;'),
        ('DoIniDataToForm', 854, 854, 'rgKitDiameter->Height：版面高度（HTML 不用）', ';'),
        # ---- FormShow ----
        ('FormShow', 1104, 1104, 'rbTemp->SetFocus()：焦點', ';'),
        ('FormShow', 1106, 1106, 'LoadImage()：示意圖（HTML 自己有）', ';'),
        ('FormShow', 1123, 1123, 'rgKitDiameter->Caption：群組標題字', ';'),
        ('FormShow', 1152, 1152, 'rgDieForceKitDiameter->Caption：群組標題字', ';'),
        ('FormShow', 1177, 1177, 'Caption=S：視窗標題', ';'),
        ('FormShow', 1185, 1186, 'scrbSLK->Position（ELTrackBar）＋scrbSLKChange(this)（事件處理器參數已拿掉）',
         SLK + '->Position=DeviceForm_File.iHeadDeviceCT; DF_scrbSLKChange();'),
        ('FormShow', 1208, 1211, 'InArmSuck／OutArmSuck：aHotPlateSubstrate.h 與 HTEditList.h 的 TList 重複定義 → FileRW/_KitSuck.cpp 轉接（同一個判斷）',
         'if(IndexHasIC() || FileRW_InOutArmSuckHasIC() || ShuttleHasIC())'),
        ('FormShow', 1230, 1231, 'Left／Top：視窗位置', ';'),
        ('FormShow', 1518, 1519, 'btTempOffset／btAt->Parent：版面父容器（HTML 不用）', ';'),
        ('FormShow', 1523, 1524, 'btTempOffset／btAt->Parent：版面父容器', ';'),
        ('FormShow', 1633, 1633, 'ledOneCycle->Value：TMyLed 燈號（替身是 TControl，沒有 Value；燈號是 Step Contact Test 狀態）', _CT3A_LED_SHOW),   # AI(W906-B8-CT3A) 20261001 [W906] (St01)：';' → 燈號值放在替身的 Tag（見上面 _CT3A_LED_SHOW）；同一行改寫
        ('FormShow', 1782, 1782, 'lblPerGf->Width：版面寬度', ';'),
        # ---- DutCount ----
        ('DutCount', 2090, 2098, 'scrbSLK->Position（ELTrackBar；設 Position 照 VCL 觸發 OnChange=scrbSLKChange）',
         'if(TestIF_File.iTestMode==SingleSite) { if(' + SLK + '->Position==3 || ' + SLK + '->Position==6) ' + SLK + '->Position=5; '
         'else if(' + SLK + '->Position==4) ' + SLK + '->Position=2; } '
         'DeviceForm_File.iHeadDeviceCT=' + SLK + '->Position; '
         'asStr.sprintf("%sContact%d.bmp", BmpPath.c_str(), (int)' + SLK + '->Position);'),
        # ---- scrbSLKChange ----
        ('scrbSLKChange', 2109, 2109, 'edAirForceChange(this)：事件處理器參數已拿掉', 'DF_edAirForceChange();'),
        # ---- edAirForceChange ----
        # ---- edAirForceChange ----
        ('edAirForceChange', 2301, 2301, 'ADAM_WriteVoltage：EP 電壓輸出（硬體；移植樹 atester_shims 是空殼）—— DeviceForm.dPress／DeviceForm_File.dPress 兩行照 golden',
         'filerw::ELTodo("golden cContact.cpp:2301 ADAM_WriteVoltage(DeviceForm.dPress) not done (EP hardware output; ADAM_* are empty stubs in the port)");'),
        # ---- CalculateTotalAirForce ----
        ('CalculateTotalAirForce', 18984, 18984, 'scrbSLK->Position（ELTrackBar）。golden iconv 行號 :18985；產生器 cp950 解碼在 :18744-18966 間少算一行，行號以產生器為準、內容已核對', 'switch((int)' + SLK + '->Position)'),
        # ---- 事件處理器互呼：參數已拿掉 ----
        ('rgKitDiameterClick', 15543, 15543, 'edAirForceChange(this)：事件處理器參數已拿掉', 'DF_edAirForceChange();'),
        ('chkUseAddWeightClick', 17390, 17390, 'edAirForceChange(this)：事件處理器參數已拿掉', 'DF_edAirForceChange();'),
        ('edForcePerDeviceKGClick', 18701, 18701, 'fQwertyKey->ShowQwertyKey：螢幕小鍵盤（輸入在頁面）', ';'),
        ('edContactHeight1Change', 2294, 2294, 'fOffSet->bEnterSpecialOffset：移植樹 TfOffSet 沒有這個成員（forms/fContact.h X-18；Offset 頁的 C 形狀用 J.M 自帶）',
         'filerw::ELTodo("golden cContact.cpp:2294 fOffSet->bEnterSpecialOffset=true not done (TfOffSet has no such member in the port)");'),
        # ---- spbSaveClick ----
        ('spbSaveClick', 14190, 14190, 'Close()：golden 權限不足時關表單 → 伺服器端記 closed（ack.trace）', 'filerw::ELMark("closed");'),
        ('spbSaveClick', 14266, 14266, 'fOffSet->ClearIndexOffset()：GATE (O-6) 已退役（AI(W906-W152) 20261007 St02-E：本體照 golden 0618 cOffSet.cpp:1817-1853 翻在 cOffSet.cpp 檔尾）；這一列只為了讓產出檔行號不動，取代句就是 golden 原句',
         'fOffSet->ClearIndexOffset();'),
        # ---- Get0_01MMType(char*)：移植樹 cpublic.h:19 不收 const char*（golden BCB6 c_str() 回 char*）→ const_cast，其餘照 golden ----
        ('UpdateContactRelative', 22611, 22611, 'Get0_01MMType(char*) const', 'Pos=Get0_01MMType(const_cast<char*>(EL<TEdit>("TfContact", "edContactHeight1")->Text.c_str()));'),
        ('UpdateContactRelative', 22614, 22614, 'Get0_01MMType(char*) const', 'Pos=Get0_01MMType(const_cast<char*>(EL<TEdit>("TfContact", "edContactHeight2")->Text.c_str()));'),
        ('spbSaveClick', 14221, 14222, 'Get0_01MMType(char*) const',
         'position1=atof(ConvertTouMType(Get0_01MMType(const_cast<char*>(EL<TEdit>("TfContact", "edContactHeight1")->Text.c_str())))); position2=atof(ConvertTouMType(Get0_01MMType(const_cast<char*>(asContactHeight[0].c_str()))));'),
        ('spbSaveClick', 14240, 14241, 'Get0_01MMType(char*) const',
         'position1=atof(ConvertTouMType(Get0_01MMType(const_cast<char*>(EL<TEdit>("TfContact", "edContactHeight2")->Text.c_str())))); position2=atof(ConvertTouMType(Get0_01MMType(const_cast<char*>(asContactHeight[1].c_str()))));'),
        # ---- SaveSetupFile ----
        ('SaveSetupFile', 14404, 14404, 'scrbSLK->Position（ELTrackBar）',
         'WriteIniData(szDir, "Mode", "Head Device Mode", (int)' + SLK + '->Position);'),
        # ---- CheckContactSettingChange ----
        ('CheckContactSettingChange', 17365, 17365, 'scrbSLK->Position（ELTrackBar）',
         'else if((int)' + SLK + '->Position!=DeviceForm.iHeadDeviceCT)'),
        # AI(W906-NUMCMP) 20260927 (Steven 團隊)：Jimmy／NB2 R89（TO_STEVEN §4 05:5x）—— golden `int!=AnsiString`（數字在左），
        #   BCB6 bcc32 5.6.4 編成 Variant 數值比較（NB2 實測）；vclcompat 照字面是字串比較 ⇒ Dimension 0 時 golden 判「沒變」、
        #   移植樹判「變了」，SECS/GEM 機台存 Contact 頁會多要一次 Run check。改成明講的數值比較（FormatFloat 輸出一定是數字，ToDouble 不會丟）。
        ('CheckContactSettingChange', 17379, 17379, 'int!=AnsiString：照 BCB6 Variant 數值比較（NB2 R89）',
         'else if(double(iUnitMultiply100(atoi(EL<TEdit>("TfContact", "edXDimension")->Text.c_str())))!=FormatFloat("0.00", DeviceForm.XDimension).ToDouble())'),
        ('CheckContactSettingChange', 17381, 17381, 'int!=AnsiString：照 BCB6 Variant 數值比較（NB2 R89）',
         'else if(double(iUnitMultiply100(atoi(EL<TEdit>("TfContact", "edYDimension")->Text.c_str())))!=FormatFloat("0.00", DeviceForm.YDimension).ToDouble())'),
        # AI(W906-EVB3) 20260928 [W906]：CT-L1 pnlSensorAdjClick（golden :14171-14177）—— 等級 109 照 golden 在 C++ 查（fSecurity->Insufficient(109)，
        #   不足時 golden 跳 WAR1676）；過了之後的 fCCLink（Shuttle Sensor Utility，golden CCLink\MyCCLinkSensor）移植樹沒有表單 →
        #   ack.todo 記一筆 "open:cclink …"，頁面（web/page/ht9045_contact_ev.js）看到它才叫 background.html 開 'cclink' 視窗。
        ('pnlSensorAdjClick', _SA_CC1, _SA_CC2,
         'fCCLink->ShowUseSensor／Show()：TfCCLink（golden CCLink/MyCCLinkSensor.h）移植樹沒有（CCLink/MyCCLink.h 註明 W7-UI deferred）→ '
         '頁面開 HW.MyCCLinkSensor.html；ShowUseSensor 依 iTestMode／dSiteXPitch 顯示哪幾個感測器，網頁那頁沒有對應（列給 Jimmy）',
         'filerw::ELTodo("open:cclink golden cContact.cpp:14175-14176 fCCLink->ShowUseSensor(TestIF.iTestMode, TestIF.dSiteXPitch) + fCCLink->Show(): TfCCLink is not ported -- the page opens window cclink (HW.MyCCLinkSensor.html); ShowUseSensor sensor selection is not applied there");'),
    ] + _FC_REPLACE + _FS_BACKUP + _EP_REPLACE + _CT3A_REPLACE + _CT3B_REPLACE + _CT3D_REPLACE,   # AI(W906-EVB10A) 20260929 [W906]：CT-3b FormClose 七條＋開頁備份一條（見上）  # AI(W906-B8-CTL2) 20260930 [W906]：B8 CT-L2 TimerEPTimer 兩條（見上 _EP_REPLACE）  # AI(W906-B8-CT3A) 20261001 [W906] (St01)：B8 CT-3a 八條（見上 _CT3A_REPLACE）   # AI(W906-B8-CT3B) 20261001 [W906] (St01)：+ B8 CT-3b' 兩條（見上 _CT3B_REPLACE），同一行附加   # AI(W906-B8-CT3D) 20261001 [W906] (St01)：+ B8 CT-3d 十三條（見上 _CT3D_REPLACE），同一行附加
    'blocks': [
        ('ReadFile', 556, 561, 'REAL_TIME_CCD ROI size serial send via COM2 (TCOM2Shim has no sRealTimeCom_Send/SendCommToVision) -- hardware write, same GATE as forms/fContact.cpp G-CT-RTCROI'),
        ('spbSaveClick', 14271, 14278, 'KYEC_LEE fMain->LabZ1_Test/LabZ2_Test caption (not in the port TfMain)'),
        ('spbSaveClick', 14281, 14294, 'Double/Multi/Independent EP output after save (IsMultiEPPressureRouteActive/IsIndependentEPPressureRouteActive/APAX_WriteData not in the port; ADAM_* are empty stubs)'),
        ('spbSaveClick', 14305, 14308, 'USE_VibrationCommunication dmTrayMotor->bNeedSetVibrateMotSpeed (dmTrayMotor not in the port)'),
    ] + _EP_BLOCKS,   # AI(W906-B8-CTL2) 20260930 [W906]：B8 CT-L2 TimerEPTimer 的 KYEC ATC 倒數字（見上 _EP_BLOCKS）
    'includes': ['cprod.h', 'Config.h', 'LastSet.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'forms/fMain.h', 'forms/fSecurity.h', 'forms/fContact.h', 'forms/fOffSet.h',
                 'forms/fProductionInfo.h', 'csystem.h', 'cpublic.h', 'cUnitConvert.h', 'ContactForce.h',
                 'CCLink/MyCCLinkSensor_predicates.h', 'atester_shims.h',
                 'SECSGEM/SecsEventType.h', 'SECSGEM/SecsEventReport.h',
                 # AI(W906-EVB10A) 20260929 [W906]：CT-3b FormClose —— MyEtherCAT（:1913）、W906_FormShowing（:1912 fCCLink）；
                 #   MOT[MMTrayY] 經 FileRW/_KitSuck.cpp 轉接（Motor/mymotor.h 會多帶 43 個 Motor/HTMotor.h 的警告進這個 TU）
                 'EtherCAT/MyEtherCAT.h', 'W906FormShowing.h', 'mycylin.h', 'myswitch.h'],   # AI(W906-B8-CT3D) 20261001 [W906] (St01)：+ B8 CT-3d golden OTD 的 Cylinder[]（mycylin.h:176）、SW[]（myswitch.h:43），同一行附加
    # cAuthority.h（language.h）、aHotPlateSubstrate.h 與 HTEditList.h 衝突 → 只前置宣告要用的
    'decls': ['bool FileRW_InOutArmSuckHasIC();         // FileRW/_KitSuck.cpp（aHotPlateSubstrate.h 與 HTEditList.h 的 TList 重複定義）',
              'extern int iCloseSiteModeFor2x8;       // aHotPlateSubstrate.h:903（golden ainarm9045_2x8_8.h:28）',
              'void GetLimitAuth();                   // cAuthority.h:77（golden cAuthority.h）',
              'extern bool bAuthCriticalPara[26];   // cAuthority.h:64（golden cAuthority.h）',
              # golden fContactForce->SLKClass[i]->欄位 / ->DieForceSLKClass：移植樹的家是 ContactForceTables()（ContactForce.h，
              # wb_serve 開機 LoadContactForceTables 載入）。這個薄轉接讓 golden 原文 `fContactForce->SLKClass[i]->dDiameter` 照抄成立
              # （size()＋operator[] 回指標，同 golden vector<THTSLKClass*>）。
              'struct DF_SlkView { SlkForceTable* t; size_t size() const { return t->size(); } SlkForceData* operator[](size_t i) const { return &t->items[i]; } };',
              'struct DF_ContactForceView { DF_SlkView SLKClass; DF_SlkView DieForceSLKClass; };',
              'static DF_ContactForceView* DF_ContactForce() { static DF_ContactForceView v; v.SLKClass.t=&ContactForceTables().SLKClass; v.DieForceSLKClass.t=&ContactForceTables().DieForceSLKClass; return &v; }',
              '#define fContactForce (DF_ContactForce())   // golden ContactForce.h:361 extern TfContactForce *fContactForce',
              'void NewRecordProcess(AnsiString AlarmCode, AnsiString S, AnsiString Debug);   // cMyDB.h:129（不 include cMyDB.h，同 FileRW/_EditPage.cpp:968）  AI(W906-EVB10A) 20260929 [W906]：CT-3b FormClose 用',
              '// AI(W906-EVB10A) 20260929 [W906]：CT-3b —— golden 碰 TMyKitSuck／MOT 的幾行經 FileRW/_KitSuck.cpp 轉接（aHotPlateSubstrate.h 與 HTEditList.h 衝突）',
              'void FileRW_Contact_TestSuckBackup();                 // golden FormShow :1388-1404',
              'void FileRW_Contact_TestSuckRestore();                // golden FormClose :1868-1878',
              'bool FileRW_MotTrayHasIC(int iMot);                   // golden FormClose :1884 MOT[].HasIC()',
              'void FileRW_MotClearTray(int iMot, const char* func); // golden FormClose :1886 MOT[].ClearTray(__FUNC__)',
              'void FileRW_CarryKitsSetAllNull();                    // golden FormClose :1904-1905',
              'extern bool W906_COM2_bRTCVerSupportAutoTurnning;     // golden rs232.h:156 TCOM2 成員（開機值 false，rs232.cpp:119）；TCOM2Shim 沒有 → FileRW/DeviceForm_File.cpp 檔尾  AI(W906-B8-CTL2) 20260930 [W906]',
              'void FileRW_Contact_RbClickTrue(TRadioButton* self);  // VCL TRadioButton.SetChecked(True)（TurnSiblingsOff＋OnClick＝rbModeNormalClick）；FileRW/DeviceForm_File.cpp 檔尾  AI(W906-B8-CT3A) 20261001 [W906] (St01)',
              'void FileRW_Contact_RbReimpose();                     // 存檔後重讀：模式單選照這一次開窗最後點的那顆（同上檔尾）  AI(W906-B8-CT3A) 20261001 [W906] (St01)', 'void W906_Contact_B8BtnStartClick();  void W906_Contact_B8BtnPauseClick();   // golden btnStartClick :14074-14076／btnPauseClick :14081 的 after-ack 登記；FileRW/DeviceForm_File.cpp 檔尾  AI(W906-B8-CT3B) 20261001 [W906] (St01)', 'const int DF_bvLowered=1, DF_bvRaised=2;   // VCL ExtCtrls TBevelCut（bvNone, bvLowered, bvRaised, bvSpace）：palOTD_4／palOTD_6 的 BevelOuter 放在替身 Tag  AI(W906-B8-CT3D) 20261001 [W906] (St01)', 'const int DF_ledOTDOff=0, DF_ledOTDLime=1, DF_ledOTDRed=2;   // ledOTD（TMyLed）的 Value＋TrueColor 放在替身 Tag  AI(W906-B8-CT3D) 20261001 [W906] (St01)'],   # AI(W906-B8-CT3D) 20261001 [W906] (St01)：+ B8 CT-3d 兩行常數（見上 _CT3D_ 說明），同一行附加
    'overrides': [],
    # AI(W906-EVB3) 20260928 [W906]：WS form.event 事件表（Steven ★ Q40＝A；批次 B3 CT-1／CT-L1）。golden cContact.dfm 的綁定：
    #   :16178 rgKitDiameter OnClick、:16272 rgOutKitDiameter OnClick、:16502 rgDieForceKitDiameter OnClick、:16288 chkUseAddWeight OnClick、
    #   :17931 cbEnableUK OnClick、:17428 cbContactMode OnChange、:18606 coD41 OnChange（CT-1：原本只在存檔時由 DF_DeriveBeforeSave 第 1、2 步重跑）；
    #   :16391 pnlSensorAdj OnClick（CT-L1）。打字欄位（edPinCount／N／G…）與 scrbSLK 不在表裡：頁面 ht9045_contact_slk.js 自己照 golden 算，
    #   存檔前 DF_DeriveBeforeSave 第 3～5 步重算（滑桿的 position 要 B1 的共用層）。
    #   FileRW/DeviceForm_File.cpp 檔尾把產生的 kDF_Events 抄一份、處理器換成跳板（機台記憶體等存檔才改；見該段）。
    'events': [('cbContactMode', 'change', 'cbContactModeChange'),
               ('rgKitDiameter', 'click', 'rgKitDiameterClick'),
               ('rgOutKitDiameter', 'click', 'rgOutKitDiameterClick'),
               ('rgDieForceKitDiameter', 'click', 'rgDieForceKitDiameterClick'),
               ('chkUseAddWeight', 'click', 'chkUseAddWeightClick'),
               ('cbEnableUK', 'click', 'cbEnableUKClick'),
               ('coD41', 'change', 'coD41Change'),
               ('pnlSensorAdj', 'click', 'pnlSensorAdjClick')]
              + _CT3A_EVENTS + _CT3B_EVENTS + _CT3D_EVENTS,   # AI(W906-B8-CT3A) 20261001 [W906] (St01)：B8 CT-3a 14 列（11 顆模式單選＋T.Start／T.Step／One Cycle，見上 _CT3A_EVENTS）；FileRW/DeviceForm_File.cpp 檔尾的跳板 Ct3aRun（不走 CT-1 的 session）   # AI(W906-B8-CT3B) 20261001 [W906] (St01)：+ B8 CT-3b' btnStart／btnPause 兩列（跳板 Ct3bRun，FileRW/DeviceForm_File.cpp 檔尾），同一行附加   # AI(W906-B8-CT3D) 20261001 [W906] (St01)：+ B8 CT-3d palOTD_4／palOTD_6 兩列（跳板 Ct3dRun，同檔尾），同一行附加
}

# AI(W906-E030A) 20261003 [W906] (St01)：todo E-030 part A —— Jimmy RULINGS_20261002 第 20 條（golden＝906，912 只拿來看 906 漏了什麼）、
#   第 23 條第 6 項（#62＝A：已在 main、內容跟 906 不同的改回 906；第 20a／20b／20c 條與 Q-A 例外不動）；Steven 1002 19:4x「雙重註解」⇒ 906＋V912 行號都列。
#   本產生器的 golden 根目錄還是 V912（tools/gen_editlist.py:38 GOLDEN；換根要 Jimmy 決定，906 樹不在 git），所以這裡用 replace 把 V912 才有的段落
#   換回 906 的樣子（V912 原文留在 #if 0 // GATE 裡，_expect 釘住；V912 那幾行哪天變了，產生器停下來）。
#   906＝D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\cContact.cpp [AI(W906-E032) 20261003]；下面「906 :N」是 iconv 行號，「V912 :M」是產生器（cp950 decode）行號。
#   (1) ⛔ E-037 改回 V912（見檔尾 AI(W906-E037)；Steven 1003 Q86＝A）——以下是 E-030 當時的做法，這 7 列已從 _E030_REPLACE 移到 _E037_REPLACE：
#       ASE 中壢 Contact 高度反灰（Chrischen 20260316，CC_ASE_CL）：906 一處都沒有 ⇒ 7 處換成空敘述 `;`（edContactHeight1／2 照 906 不反灰）：
#       DoIniDataToForm V912 :1021-1028（906 :1012 與 :1013 之間）、FormShow V912 :1306-1307（906 :1288 之後）、:1478-1485（906 :1458 之後）、
#       SetContactMode V912 :15577-15584／:15595-15602／:15608-15615／:15626-15633（906 :15341-15435 沒有）。
#       rbVisualDetectionTest（SetContactMode V912 :15661-15665、FormShow V912 :1234）不刪：RULINGS_20261002 第 23 條第 5 項 Q-C 先留，併入第 62 項（FormShow :1234 的顯示條件由下面 AI(W906-E035) 那一列改）。
#   (2) FormShow 的 AMD 條件：V912 `IniConfig.bAMDFunction`（Ifor 20260716「AMD group function flag, replace CUSTOMER_CODE==CC_AMD_SG」）
#       ⇒ 906 `CUSTOMER_CODE==CC_AMD_M`：V912 :1437（906 :1418）、:1614（906 :1587）、:1736（906 :1709）。跟 Q-C 不是同一件：
#       rbVisualDetectionTest 本身照 Q-C 留著；它的 Visible=(IniConfig.bAMDFunction)（V912 :1234，Ifor 20260204）在移植樹永遠是 false（沒有人寫這個旗標）⇒ E-035 改成 CUSTOMER_CODE==CC_AMD_M（見下面 AI(W906-E035) 那一列）。旗標欄位（Config.h:1501）留著。
#   ⓘ 沒有改的（ST01-E 20261003 01:2x 決定 HOLD，問 Jimmy）：spbSaveClick 的 A02（RogerYang 20260305）V912 :14191 在 Close() 後面的 `return;`
#       （906 :14072-14084 沒有）。V912 在 19 支產生的存檔處理器都加了這一句、906 全都沒有；它是擋存檔的保護（同 Q-A 那一類），照舊留 V912，
#       等 Jimmy 決定（St01 建議記成第 20 條的例外）。
def _e030_rows(meth, a, b, first, last, why):
    _expect(a, first)
    _expect(b, last)
    return (meth, a, b, why, ';')


_E030_ASECL = 'ASE 中壢 Contact 高度反灰（Chrischen 20260316）：V912 才有、906 沒有 ⇒ 照 906 不做（E-030；RULINGS_20261002 第 23 條第 6 項）'
_E030_REPLACE = [
    # AI(W906-E037) 20261004 [W906] (St01)：原本這裡是 ASE 中壢反灰的 7 列（_e030_rows → `;`），E-037 移到檔尾 _E037_REPLACE（改回 V912）。
    ('FormShow', _expect(1437, 'if(IniConfig.bAMDFunction)'), 1437,
     'V912 IniConfig.bAMDFunction（Ifor 20260716 AMD 群組旗標）⇒ 906 :1418 原文 CUSTOMER_CODE==CC_AMD_M（E-030；RULINGS_20261002 第 23 條第 6 項）',
     'if(CUSTOMER_CODE==CC_AMD_M)'),
    ('FormShow', _expect(1614, 'if(IniConfig.bAMDFunction)'), 1614,
     'V912 IniConfig.bAMDFunction ⇒ 906 :1587 原文 CUSTOMER_CODE==CC_AMD_M（E-030）',
     'if(CUSTOMER_CODE==CC_AMD_M)'),
    ('FormShow', _expect(1736, 'if(CosFunction.bHiSiliconFunction==true && IniConfig.bAMDFunction)'), 1736,
     'V912 IniConfig.bAMDFunction ⇒ 906 :1709 原文 CUSTOMER_CODE==CC_AMD_M（E-030）',
     'if(CosFunction.bHiSiliconFunction==true && CUSTOMER_CODE==CC_AMD_M)'),
]
STRUCT['replace'] += _E030_REPLACE

# AI(W906-E035) 20261003 [W906] (St01 ST01-E2)：todo E-035 —— Contact 頁第 11 個模式 Visual Detection Test（rbVisualDetectionTest）永遠不顯示。
#   這是 V912 才有的客戶功能（Ifor 20260204「TFAMD 要求新增檢測流程」）：照 Q-C 先留（RULINGS_20261002 第 23 條第 5 項，併入第 62 項），
#   也符合 Q81 的預設（912 才有的客戶專屬功能留 912，Steven 1003；D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md Q81）。
#   三段對照：
#   - 906 0618（D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618）：沒有這個元件（cContact.cpp／.h／.dfm 0 筆；rgHandlerMode 只有 10 個模式，.dfm:18125-18303）。
#   - V912 cContact.cpp:1234（FormShow）`rbVisualDetectionTest->Visible=(IniConfig.bAMDFunction);`；這個旗標只在 FUNC_CC_AMD_SG（CosFunction.cpp:2991，
#     代碼 982）設 true、InitialCosFunction（:4378）設 false；跟著 AMD_SG 的 TFAMD_SUZHOU／TFAMD_M／AMD_US 三個代碼 906 沒有。
#   - 906 的 982＝CC_AMD_M（MachineType.h:354；FUNC_CC_AMD_M CosFunction.cpp:2942）⇒ 條件寫 CUSTOMER_CODE==CC_AMD_M（同上面 E-030 三列 AMD 條件的換法）。
#     不用 CosFunction.bAMDFunction：906 SPILFunction（CosFunction.cpp:3082）也設它（V912 :3162 已註解掉）——用它的話 SPIL 機台也會出現這個模式。
#   移植樹沒有人寫 IniConfig.bAMDFunction（只寫 CosFunction.bAMDFunction：CosFunction.cpp:3134／:3265／:4457），所以 V912 原文在這裡永遠是 false。
#   取代碼直接寫 EL<> 形式（gen_editlist.py 照抄 replace 的手寫碼，不再轉換）。測試：tests/test_b8_ct3a_contactflags.cpp [3c]（ctest B8_Ct3a_ContactFlags）。
#   移植樹其他 12 處讀 IniConfig.bAMDFunction 的地方也一樣永遠是 false：不在這一項，ST01-M 記在 E-035 列當第 62 項的後續。
STRUCT['replace'].append(('FormShow', _expect(1234, 'rbVisualDetectionTest->Visible=(IniConfig.bAMDFunction);'), 1234,
    'V912 IniConfig.bAMDFunction（只有 FUNC_CC_AMD_SG＝982 設）⇒ 906 的 982＝CC_AMD_M（MachineType.h:354）；不用 CosFunction.bAMDFunction'
    '（906 SPILFunction :3082 也設）（E-035；906 0618 沒有這個元件＝V912 才有的客戶功能，Q-C 先留＋Q81 預設留 912）',
    'EL<TRadioButton>("TfContact", "rbVisualDetectionTest")->Visible=(CUSTOMER_CODE==CC_AMD_M);'))

# AI(W906-E030-Q78) 20261003 [W906] (St01)：Steven 1003 05:3x Q78 裁決「Q78 Q79, 可以按照912，但是註解同時提供906的行號位置」，
#   加上 05:4x 慣例「如果是912比較好，就是註記906的行號跟做法　然後增加註記912已修正或更新的行號」⇒ 上面 E030A 段 ⓘ 的 HOLD
#   （spbSaveClick A02 Close() 後的 `return;`）已決定：照 V912 留著，記成第 20 條的例外
#   （D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md Q78；human-review C24）。
#   這一列是「等價取代」：取代碼＝golden 原文 `return;`，產生的程式不變，只是讓產生檔那一行帶 906／V912 對照註解
#   （V912 原文留在 #if 0 // GATE 裡）；_expect 釘住 V912 的 if／Close()／return。不改既有的列。
_expect(14181, 'if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&')
_expect(14190, 'Close();')
_E030Q78_WHY = ('AI(W906-E030-Q78) 原文照留（取代碼＝golden 原文 return;，產生的程式不變，只加這段註解）。'
                '906 cContact.cpp:14074-14084：A02 分支（裡面 CarlibrationTask!=1 那句 return 兩版都有）最後只有 Close()（:14083），後面沒有 return，存檔照跑；主選單 main.cpp:27324 用 Show 開（Jimmychiu 20240731 ShowModal --> Show），Close() 當場跑 FormClose（:1815-1896）的 ReadFile()（:1821）＋DoIniDataToForm()（:1822）⇒ 改的值被丟掉，存檔把檔案原值寫回（存檔的其他動作照跑）；uLotInfo.cpp:2616 用 ShowModal 開時，改的值照樣寫進檔。'
                'V912 cContact.cpp:14190-14191：Close() 後多一句 `return;` ⇒ 頁面關掉、不寫檔（A02 分支本身兩版都有，RogerYang 20260305 [A01_2]）。'
                'Kept: #20 exception (Steven 1003 Q78, keep V912)')
STRUCT['replace'].append(('spbSaveClick', _expect(14191, 'return;'), 14191, _E030Q78_WHY, 'return;'))

# AI(W906-E030-Q79) 20261003 [W906] (St01)：Steven 1003 05:3x Q79 裁決（同上原話）⇒ Contact 讀檔算 GPIB 接觸力字串 asArmForce1／2、
#   畫面的 ShowArmAndDeviceForce 都照 V912 的 CalcDeviceForce（RogerYang 20260624），記成第 20 條的例外
#   （decisions-decided.md Q79；human-review C25）。三列都是「等價取代」：取代碼＝golden 原文（CalcDeviceForce 已換成產生器的
#   DF_CalcDeviceForce，跟產生器自己轉的一字不差），產生的程式不變，只在那一行帶 906／V912 對照註解；_expect 釘住 V912 原文。
#   ST01-M 的說明「G == N x 1000 / 9.8 時兩版的值相同（兩版的預設與換算都這樣）」核對兩棵樹後：換算對（改 N 算 G、改 G 算 N，
#   兩版一樣），但 N、G 各自四捨五入到 "0.0000" 存檔，殘差乘上腳數看得出來（4509 pin 約 0.018 Kgf）；預設只有 CC_JCET／CC_AMKOR_China
#   是 N*1000/9.8，其他客戶檔裡沒有 G 鍵時預設 30.0（N 預設 0.0）——寫進下面的註解。
if _head('CalcDeviceForce') != 23055:
    raise SystemExit('DeviceForm_File.py (E030-Q79): golden V912 TfContact::CalcDeviceForce moved from :23055')
_expect(611, 'double dGpibForce = CalcDeviceForce(DeviceForm_File.iPinCT,')
_expect(614, 'true);')
_expect(616, 'asArmForce2=FormatFloat("0.0000", dGpibForce);')
_expect(1941, 'double dDeviceGf = CalcDeviceForce(dBallCount, dSingleN, dSingleGf, true);')
_expect(23058, 'return dPinCount * dForcePerPinGf * 0.001;')
_expect(23060, 'return dPinCount * dForcePerPinN;')
_E030Q79_RF = ('AI(W906-E030-Q79) 原文照留（取代碼＝golden 原文，產生的程式不變，只加這段註解）。'
               '906 cContact.cpp:607-608：asArmForce1／2＝iPinCT*ForcePerPinN/9.8（double 直接給 AnsiString）。'
               'V912 cContact.cpp:609-616：舊式註解掉（:609-610），改 dGpibForce＝CalcDeviceForce(iPinCT, N, G, true)＝iPinCT*ForcePerPinG*0.001（:611-614、:23055-23061；RogerYang 20260624「★這裡決定送Kgf(true)還是N(false)」），asArmForce1／2＝FormatFloat("0.0000", dGpibForce)（:615-616）。'
               '兩版的值只有在檔裡 Force Per Pin G 剛好＝N*1000/9.8 時相同。兩版 Contact 頁的換算一樣（改 N 時 G＝N*1000/9.8：906 :1927-1938／V912 :1956-1967；改 G 時 N＝G*9.8/1000：906 :1941-1957／V912 :1970-1986，cContact.dfm:16036 edForcePerPinG OnChange），'
               '但 N、G 各自以畫面文字 "0.0000" 存檔（906 :14379-14385／V912 :14487-14493），四捨五入的殘差乘上腳數就看得出來（skill ht9045-contact-force §2 的例子：4509 pin、N 0.2323、G 23.7 ⇒ 906 送 106.8817…、V912 送 106.8633，差約 0.018 Kgf；V912 這個寫法就是該節的建議解法）；'
               '檔裡沒有 G 鍵時，只有 CC_JCET／CC_AMKOR_China 的預設是 N*1000/9.8（906 :575／583／591＝V912 :577／585／593），其他客戶預設 30.0（906 :577／585／593＝V912 :579／587／595）、N 預設 0.0（906 :566／569／589＝V912 :568／571／591）⇒ 兩版差更多。'
               '送給測試機的字串格式也不同：V912 "0.0000"，906 AnsiString(double)。'
               'Kept: #20 exception (Steven 1003 Q79, keep V912)')
_E030Q79_SA = ('AI(W906-E030-Q79) 原文照留（取代碼＝golden 原文，CalcDeviceForce 換成 DF_CalcDeviceForce 跟產生器自己轉的一樣，產生的程式不變）。'
               '906 cContact.cpp:1911-1912：dDeviceN＝dBallCount*dSingleN、dDeviceGf＝dBallCount*dSingleGf*0.001（直接算）。'
               'V912 cContact.cpp:1938-1941：舊式註解掉（:1938-1939），改呼叫 CalcDeviceForce(…, false)／(…, true)（RogerYang 20260624「統一公式」）；CalcDeviceForce :23058／:23060 是同一個算式、同樣的運算順序 ⇒ 畫面的 edForcePerDeviceKG／edForcePerDeviceN 跟 906 一樣，只是改走共用函式（GPIB 字串見 ReadFile :615）。'
               'Kept: #20 exception (Steven 1003 Q79, keep V912)')
_E030Q79_CF = ('AI(W906-E030-Q79) 原文照留（取代碼＝golden 原文，產生的程式不變）。'
               'V912-only method; not in 906 0618（906 cContact.cpp／cContact.h 都沒有 CalcDeviceForce；V912 cContact.cpp:23055-23061、cContact.h:667，RogerYang 20260624）。'
               '906 在兩個呼叫點直接算：ReadFile 906 :607-608（V912 :611-614 呼叫，true＝Kgf）、ShowArmAndDeviceForce 906 :1911-1912（V912 :1940-1941）。'
               'Kept: #20 exception (Steven 1003 Q79, keep V912)')
STRUCT['replace'] += [
    ('ReadFile', _expect(615, 'asArmForce1=FormatFloat("0.0000", dGpibForce);'), 615, _E030Q79_RF,
     'asArmForce1=FormatFloat("0.0000", dGpibForce);'),
    ('ShowArmAndDeviceForce', _expect(1940, 'double dDeviceN  = CalcDeviceForce(dBallCount, dSingleN, dSingleGf, false);'), 1940, _E030Q79_SA,
     'double dDeviceN  = DF_CalcDeviceForce(dBallCount, dSingleN, dSingleGf, false);'),
    ('CalcDeviceForce', _expect(23057, 'if(bReturnKgf)'), 23057, _E030Q79_CF, 'if(bReturnKgf)'),
]

# AI(W906-E037) 20261004 [W906] (St01)：todo E-037 B71 —— Steven 1003 21:3x Q86＝A（「目前不急，慢慢做就好了，答案A」）：ASE 中壢（CC_ASE_CL＝933）
#   Contact 頁 edContactHeight1／2 反灰（Chrischen 20260316）是 V912 才有的客戶功能 ⇒ 照 Q81 預設「912 才有的客戶專屬功能留 912」改回 V912，
#   記成第 20 條的例外（Steven 1003 常設：程式照 912，註解寫 906 行號＋做法與 912 的行號）。E-030（8f589831）照 906 換成 `;` 的 7 列從上面
#   _E030_REPLACE 移到這裡，改成「等價取代」（同 Q78／Q79 的寫法）：取代碼＝V912 原文轉成替身（EL<TEdit>），產生的程式＝產生器自己轉 V912 的結果，
#   V912 原文照舊留在 #if 0 // GATE 裡、_expect 釘住頭尾；每一列的說明是三段註解：(1) 906 0618 那裡做什麼 (2) 留的 V912 行號 (3) 第 20 條例外。
#   906＝D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\cContact.cpp（iconv 行號）；V912＝產生器 golden（cp950 decode 行號，這一段跟 iconv 一樣）。
#   只在 CUSTOMER_CODE==CC_ASE_CL 才走到（後 6 處另要 CosFunction.bContactShowOffset）：HT9050 機台是 957（CC_PTI，machines/HT9050/README.md）⇒ 行為不變。
#   測試：tests/test_b8_ct3a_contactflags.cpp [3b]（ASE-CL 開頁＋4 個模式都灰、HT9050 的 957 不灰）、[8]（產生檔 7 處活的 V912 碼）。
_E037_X = '#20 exception (Steven 1003 standing rule; Q81 / Q86 = A)'
_E037_H2 = 'EL<TEdit>("TfContact", "edContactHeight1")->Enabled = false; EL<TEdit>("TfContact", "edContactHeight2")->Enabled = false;'
_E037_IF_OFS = 'if(CUSTOMER_CODE==CC_ASE_CL && CosFunction.bContactShowOffset)'


def _e037_rows(meth, a, b, first, last, s906, code):
    _expect(a, first)
    _expect(b, last)
    why = ('AI(W906-E037) 原文照留（取代碼＝V912 原文轉成替身，產生的程式＝V912）。(1) 906 0618 cContact.cpp:%s。'
           '(2) 留 V912 cContact.cpp:%d-%d（Chrischen 20260316 ASE 中壢 Contact 高度反灰）。(3) %s' % (s906, a, b, _E037_X))
    return (meth, a, b, why, code)


_E037_REPLACE = [
    _e037_rows('DoIniDataToForm', 1021, 1028, 'if(CUSTOMER_CODE==CC_ASE_CL)', '}',
               '1005-1013 的 else if(CosFunction.bContactShowOffset) 分支只讀 Offset Arm1／2 並顯示 4 個元件，不灰高度（V912 把這一段插在 906 :1012 與 :1013 之間）',
               'if(CUSTOMER_CODE==CC_ASE_CL) { ' + _E037_H2 + ' }'),
    _e037_rows('FormShow', 1306, 1307, 'edContactHeight1->Enabled = false;', 'edContactHeight2->Enabled = false;',
               '1286-1289 的 else if(CUSTOMER_CODE==CC_ASE_CL) 分支只有 chkTeachInOutArmZ->Visible=true，不灰高度',
               _E037_H2),
    _e037_rows('FormShow', 1478, 1485, _E037_IF_OFS, '}',
               '1450-1458 權限段（等級 171／173 鎖 Contact／Release height）到 :1458 為止，:1459 的 } 之前沒有 ASE-CL 這一段',
               _E037_IF_OFS + ' { ' + _E037_H2 + ' }'),
    _e037_rows('SetContactMode', 15577, 15584, _E037_IF_OFS, '}',
               '15343-15352 Normal 分支：iContactMode＋KYEC_CHEN A16 ChangeContactMode(false)＋Memo1，:15351 之後沒有反灰',
               _E037_IF_OFS + ' { ' + _E037_H2 + ' }'),
    _e037_rows('SetContactMode', 15595, 15602, _E037_IF_OFS, '}',
               '15358-15362 Auto Height 分支：iContactMode＋Memo1，:15361 之後沒有反灰',
               _E037_IF_OFS + ' { ' + _E037_H2 + ' }'),
    _e037_rows('SetContactMode', 15608, 15615, _E037_IF_OFS, '}',
               '15363-15367 Manual Height 分支：iContactMode＋Memo1，:15366 之後沒有反灰',
               _E037_IF_OFS + ' { ' + _E037_H2 + ' }'),
    _e037_rows('SetContactMode', 15626, 15633, _E037_IF_OFS, '}',
               '15368-15377 Contact Test 分支：iContactMode＋Memo1＋KYEC_CHEN A16 ChangeContactMode(true)，:15376 之後沒有反灰',
               _E037_IF_OFS + ' { ' + _E037_H2 + ' }'),
]
STRUCT['replace'] += _E037_REPLACE

# AI(W906-W156) 20261007 (St02-E)，W-156 (b)：golden FormShow（V912 :1678，0618 :1651）`bSetHasIC=false;` —— golden 每次 Show() 才跑；網頁每次 editlist.get 都重跑
#   FormShow（W-152 的重讀也是）⇒ 只在「這一次開窗第一次讀」清（filerw::PageShownNow 還是 false，同 _FS_BACKUP），不然取料狀態機
#   （fContact_IndexPickPlace.cpp:340／:365）設的旗標會在 FormClose（V912 :1902-1906 清 FLCarryKit／BLCarryKit）之前被重讀清掉。  ⚠ NOT GOLDEN 接合碼（人工審查 B）。
STRUCT['replace'] += [('FormShow', _expect(1678, 'bSetHasIC=false;'), 1678,
                       'NOT GOLDEN（移植樹接合碼）：golden FormShow 每次開窗才跑一次，網頁每次 editlist.get 都重跑（W-152 的重讀也是）⇒ 只在這一次開窗第一次讀時照 golden 清 bSetHasIC，同一次開窗的重讀不清（不然取料狀態機設的旗標在 FormClose 之前就被清掉）；只有一份＝fContactForm->bSetHasIC（members 的 #define，W-156 (b)）',
                       'if(!filerw::PageShownNow("DeviceForm_File")) bSetHasIC=false;')]
