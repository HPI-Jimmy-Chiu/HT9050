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
                'TimerEPTimer'],        # AI(W906-B8-CTL2) 20260930 [W906]：B8 CT-L2 golden TimerEPTimer（:17177，見上面 _EP_REPLACE）再加在最後
    'save_methods': ['spbSaveClick', 'CheckContactSettingChange', 'SaveSetupFile'],
    'params': {'TfContact': '', 'FormShow': '', 'spbSaveClick': '', 'scrbSLKChange': '', 'edAirForceChange': '',
               'edPinCountChange': '', 'edForcePerPinNChange': '', 'edDieForcePerPinGChange': '', 'rgKitDiameterClick': '',
               'rgOutKitDiameterClick': '', 'rgDieForceKitDiameterClick': '', 'chkUseAddWeightClick': '', 'cbEnableUKClick': '',
               'cbContactModeChange': '', 'coD41Change': '', 'edContactHeight1Change': '', 'edForcePerDeviceKGClick': '',
               'pnlSensorAdjClick': '',    # AI(W906-EVB3) 20260928 [W906]：golden :14171 不讀 Sender
               'FormClose': '',            # AI(W906-EVB10A) 20260929 [W906]：CT-3b，golden FormClose(TObject*, TCloseAction&) 兩個參數都不讀
               'TimerEPTimer': ''},        # AI(W906-B8-CTL2) 20260930 [W906]：B8 CT-L2，golden :17177 不讀 Sender
    'rettype': {'CalcDeviceForce': 'double', 'CheckContactSettingChange': 'bool',
                'CalculateTotalAirForce': 'double', 'GetMaxIndexForceLimit': 'double', 'GetMinForce': 'double'},
    'members': [
        '#define CarlibrationTask (fContactForm->CarlibrationTask)   // golden cContact.h:557 —— 移植樹 fContactForm 那一份（public；存檔守衛讀它）',
        '#define eTeachSHLtcNomal 0                   // golden cContact.h:633 enum eInSHLtcStatus',
        'bool fShow=false;                  // golden cContact.h:549（不接 fContact／fContactForm，見檔頭）',
        'bool bSetupStart=false;            // golden cContact.h:545',
        'bool bSetupStep=false;             // golden cContact.h:551',
        'bool bOneCycleFinish=false;        // golden cContact.h:552',
        'double dDutCount=2;                // golden cContact.h:553',
        'bool bSetHasIC=false;              // golden cContact.h:569',
        'bool bAutoHighFinish=false;        // golden cContact.h:400',
        'int iSpeedZ=30, iSpeed=30, TorqueData=0;   // golden cContact.h:401/403/404',
        'bool MotorStatus=false, bContinueContact=false, bOldRTCAutoTuning=false, brecordmsgLock=false;   // golden cContact.h',
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
        ('FormShow', 1633, 1633, 'ledOneCycle->Value：TMyLed 燈號（替身是 TControl，沒有 Value；燈號是 Step Contact Test 狀態）', ';'),
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
        ('spbSaveClick', 14266, 14266, 'fOffSet->ClearIndexOffset()：移植樹 GATE (O-6)，沒有本體（forms/fOffSet.h:390）',
         'filerw::ELTodo("golden cContact.cpp:14266 fOffSet->ClearIndexOffset() is GATE (O-6) in the port (no body) -- index offset not cleared after auto height");'),
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
    ] + _FC_REPLACE + _FS_BACKUP + _EP_REPLACE,   # AI(W906-EVB10A) 20260929 [W906]：CT-3b FormClose 七條＋開頁備份一條（見上）  # AI(W906-B8-CTL2) 20260930 [W906]：B8 CT-L2 TimerEPTimer 兩條（見上 _EP_REPLACE）
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
                 'EtherCAT/MyEtherCAT.h', 'W906FormShowing.h'],
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
              'extern bool W906_COM2_bRTCVerSupportAutoTurnning;     // golden rs232.h:156 TCOM2 成員（開機值 false，rs232.cpp:119）；TCOM2Shim 沒有 → FileRW/DeviceForm_File.cpp 檔尾  AI(W906-B8-CTL2) 20260930 [W906]'],
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
               ('pnlSensorAdj', 'click', 'pnlSensorAdjClick')],
}
