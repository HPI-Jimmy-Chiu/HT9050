# -*- coding: utf-8 -*-
# tools/editlist/UserDefForm_File.py -- gen_editlist.py 的結構設定（一個 C 形狀結構一個檔；Steven 20260924 拆檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py [--only UserDefForm_File]
# Steven 20260924：UserDefForm_File（Tray.Data，elTrayForm）—— golden TfTrayForm（cTrayForm.cpp，1131 行）。
# 移植樹其他程式還在讀 fTrayForm 的真元件（uPAT_Function 讀 TrayName1->Text、csystem／fBuilder 呼叫
# fTrayForm->SaveSetupFile、TrayEdit[][]）→ 'adopt'：golden 名稱與型別都對得上的，替身就是移植樹那個物件
# （ELKeep 登記），兩邊共用同一份值；其餘才是新替身。
import os

# //AI(W906-EVB3) 20260928 [W906]：釘住 golden 原文（本檔其他條目用寫死的行號；新加的 TF-4 用 _expect，golden 改了就中止）
_G = open(os.path.join(r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy', 'cTrayForm.cpp'), 'rb').read().decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')


def _expect(gl, text):
    if _G[gl - 1].strip() != text:
        raise SystemExit('UserDefForm_File.py: golden cTrayForm.cpp:%d is %r, expected %r' % (gl, _G[gl - 1].strip(), text))
    return gl


# TF-4 btnBinBoxResetClick（golden cTrayForm.cpp:729-734，dfm:11660）
_expect(729, 'void __fastcall TfTrayForm::btnBinBoxResetClick(TObject *Sender)')
_expect(731, 'edtBinBoxNow->Text=0;')
_expect(732, 'LastSet.iBinBoxCount=0;')
_expect(733, 'MOT[MManualTray3].ClearTray(__FUNC__);')

STRUCT = {
    'struct': 'UserDefForm_File',
    'prefix': 'TF',
    'class': 'TfTrayForm',
    'cpp': 'cTrayForm.cpp',
    'h': 'cTrayForm.h',
    'files': ['Tray.Data'],
    'lists': ['elTrayForm'],
    'adopt': {'object': 'fTrayForm', 'header': 'forms/fTrayForm.h'},
    # AI(W906-FRW-S157) 20260927 [W906]：＋cbTrayType1Change（:570，cbTrayType1／2／3 共用的 OnChange，cTrayForm.dfm:545／:4321／:8047）
    #   與它呼叫的 ShowTypePage（:580）—— WS form.event（Steven ★ Q40＝A，RULINGS_20260926 S157），見下面 'events'。
    #   cbTrayType1Change 保留 golden 的 TObject *Sender（它讀 ComboBox->Tag 決定填哪一型：DFM Tag 0／1／2，產生器的 DfmState 帶入）。
    'methods': ['TfTrayForm', 'FormShow', 'DoIniDataToForm', 'ReadFile', 'FormClose', 'spbSaveClick', 'SaveSetupFile',
                'cbTrayType1Change', 'ShowTypePage',
                # //AI(W906-EVB3) 20260928 [W906]：TF-4（事件移植批次 B3，docs/EVENT_PORT_BATCH_20260928.md）—— Bin Box 分頁的「Reset」鈕
                #   btnBinBoxResetClick（:729，dfm:11653／:11660）：已裝數顯示歸零、LastSet.iBinBoxCount=0、MOT[MManualTray3].ClearTray（Fix3 盤的格子清空）。
                #   加在最後：既有方法的產生碼不動。
                'btnBinBoxResetClick'],
    'events': [('cbTrayType1', 'change', 'cbTrayType1Change'),
               ('cbTrayType2', 'change', 'cbTrayType1Change'),
               ('cbTrayType3', 'change', 'cbTrayType1Change'),
               # //AI(W906-EVB3) 20260928 [W906]：TF-4（Steven 20260928「任何畫面的事件, 都是我們做」）。按鈕在 pnlBixBox（Visible＝IniConfig.bBinBox，
               #   FormShow :180）→ tsBinBox（TabVisible＝bBinBox||bShowTrayAndDeviceDir，:177）裡；按鈕替身不在產生器的 names_used
               #   → FileRW/UserDefForm_File.cpp 開機補建並接上父層（同 d7fa099a）。按下即生效、不寫檔（golden 同：LastSet 由 lastdata.dat 的
               #   週期寫檔帶走）；存檔補點做不到（按鈕沒有值）。頁面 Setup.TrayForm.html 是 B2 的，送出點寫法見交件。
               ('btnBinBoxReset', 'click', 'btnBinBoxResetClick')],
    'save_methods': ['spbSaveClick', 'SaveSetupFile'],
    'params': {'TfTrayForm': '', 'FormShow': '', 'FormClose': '', 'spbSaveClick': '',
               'btnBinBoxResetClick': ''},   # //AI(W906-EVB3) 20260928 [W906]：TF-4，golden 不讀 Sender
    'members': ['bool fShow=false;   // golden cTrayForm.h:274 TfTrayForm::fShow',
                '#define TrayEdit (fTrayForm->TrayEdit)       // golden cTrayForm.h:249 —— 移植樹 fTrayForm 那一份（它的 SaveSetupFile 也用）',
                '#define asErrorMsg (fTrayForm->asErrorMsg)   // golden cTrayForm.h:279 —— asendic_Loader.cpp 讀 fTrayForm->asErrorMsg',
                '#define fConfiguration (W906_CfgTrayPlate())   // AI(W906-FRW-S98) 20260926: golden fConfiguration 的 Tray／Plate 表（FileRW/CfgTrayPlate.cpp；FormShow :162-175 用）。移植樹叫 fConfiguration 的全域是 SCK_ART 替身（Automation/SCK_ART_Remainder.cpp:352），本 TU 不用它'],
    'replace': [
        ('TfTrayForm', 127, 138,
         '色感 MU-N GUI（uColorSensorInfo，移植樹沒有這個類別）：資料註冊照 golden，掛到移植樹的替代 TEdit（同 cTrayForm.cpp Init()）',
         'if(USE_COLORSENSOR_MUN==eCSMUN_Install_Loader) { '
         'elTrayForm->Add(EL<TEdit>("TfTrayForm", "edColorSensorFTEnable"), &TestIF_File.sLoaderColorSenFTEnable, ECText, "Color Sensor", "sLoaderColorSenFTEnable", bNoShow, bEnable, bReadFromFile, "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"); '
         'elTrayForm->Add(EL<TEdit>("TfTrayForm", "edColorSensorRTEnable"), &TestIF_File.sLoaderColorSenRTEnable, ECText, "Color Sensor", "sLoaderColorSenRTEnable", bNoShow, bEnable, bReadFromFile, "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"); '
         'filerw::ELTodo("golden cTrayForm.cpp:129/:132-133 color sensor GUI (uColorSensorInfo) + LoadColorSensorMapping not ported"); }'),
        ('FormShow', 144, 144, 'LoadImage()：示意圖（HTML 自己有）', ';'),
        ('FormShow', 148, 148, 'Caption：視窗標題', ';'),
        ('FormShow', 150, 150, 'UpDateType()：「從 Type? 複製」下拉的選項（依目前分頁，頁面端的複製功能）', ';'),
        # AI(W906-FRW-S98) 20260926: golden FormShow :162-175（bHasTrayCSV → fConfiguration->sbtReloadTray->Click()＋strngrdTray 第 0 欄
        #   填 cbTrayType1..3 的選項）解閘 —— 表在 FileRW/CfgTrayPlate.cpp，本 TU 的 fConfiguration 接過去（members 的 #define）。
        ('FormShow', 271, 281, '色感分頁顯示＋LoadColorSensorMapping（缺鍵會寫 ColorSensorType.ini）＋視窗位置',
         'if(USE_COLORSENSOR_MUN==eCSMUN_Install_Loader) { EL<TTabSheet>("TfTrayForm", "tsColorSensor")->TabVisible=true; filerw::ELTodo("golden cTrayForm.cpp:274 LoadColorSensorMapping not ported"); } '
         'else { EL<TTabSheet>("TfTrayForm", "tsColorSensor")->TabVisible=false; }'),
        ('DoIniDataToForm', 330, 335, 'TMyTray1..3（TTMyTray 盤面示意圖）的 XItem／YItem：純畫面', ';'),
        ('ReadFile', 449, 450, 'rgDeviceDirClick／rgTrayDirClick：只切換示意圖與 fShowMessage 的圖（純畫面）', ';'),
        ('ReadFile', 452, 452, 'LoadColorSensorEnable()：色感 GUI 勾選框（uColorSensorInfo 未移植；值在 TestIF_File 字串，已由 elTrayForm 讀）', ';'),
        ('spbSaveClick', 616, 616, 'Close()：golden 權限不足時關表單 → 伺服器端記 closed（ack.trace）', 'filerw::ELMark("closed");'),
        ('SaveSetupFile', 646, 646, 'SaveColorSensorEnable()：色感 GUI 勾選框 → TestIF_File 字串（uColorSensorInfo 未移植）',
         'if(USE_COLORSENSOR_MUN==eCSMUN_Install_Loader) filerw::ELTodo("golden cTrayForm.cpp:646 SaveColorSensorEnable (uColorSensorInfo GUI) not ported");'),
        # ---- AI(W906-FRW-S157) 20260927 [W906]：WS form.event（cbTrayType1Change :570／ShowTypePage :580）----
        ('cbTrayType1Change', 572, 575,
         'KYEC 條碼登入 Barcode_Reader(bcTrayForm)（只有 CC_KYEC_LEE／CC_KYEC_XILINX 且 USE_BARCODE_AS_KEYBOARD 時問操作員 ID，其他機台回 2）：'
         '照叫移植樹真本體（BarcodeReader.cpp:445），回 0 時照 golden return，另記 ELTodo 讓頁面看得到為什麼沒填（輸入框是離線空殼，同 FileRW/MainClose.cpp:377）',
         'if(Barcode_Reader(bcTrayForm)==0) { filerw::ELTodo("golden cTrayForm.cpp:572 Barcode_Reader(bcTrayForm) returned 0 (KYEC operator-ID prompt; the port input box is an offline shell) -- golden returns without filling"); return; }'),
        ('ShowTypePage', 582, 582, 'TTMyTray *TMyTray[3]：盤面示意圖元件（TTMyTray 在 vclcompat 沒有；同 DoIniDataToForm :330-335 的理由，純畫面）',
         ';   // golden TMyTray[3]={TMyTray1, TMyTray2, TMyTray3}'),
        # //AI(W906-EVB3) 20260928 [W906]：TF-4 btnBinBoxResetClick :733（_expect 釘住，見檔頭）
        ('btnBinBoxResetClick', 733, 733,
         '__FUNC__（BCB6 內建巨集，只進 MOT[].ClearTray 的 MNetLog 字串「Clear tray [..] by function ..」）：移植樹慣例 canary_support.h:44-46 '
         '→ __func__，本 TU 沒 include 它；產生的函式名又多了 TF_ 前綴 → 直接寫處理器名（與移植樹其他 ClearTray(__FUNC__) 呼叫點印出來的字同一種）',
         'MOT[MManualTray3].ClearTray("btnBinBoxResetClick");'),
        ('ShowTypePage', 599, 600, 'TMyTray[iTag]->XItem／YItem：盤面示意圖的格數（純畫面；同 DoIniDataToForm :330-335）。'
         '存檔的格數是 TrayEdit[iTag][6]／[7]（XCT／YCT，:595-596 已填）', ';'),
    ],
    'includes': ['cprod.h', 'Config.h', 'LastSet.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'forms/fMain.h', 'forms/fSecurity.h', 'forms/fTrayForm.h', 'csystem.h',
                 'BarcodeReader.h', 'SECSGEM/SecsEventType.h', 'SECSGEM/SecsEventReport.h',
                 'FileRW/CfgTrayPlate.h',   # AI(W906-FRW-S98) 20260926: W906_CfgTrayPlate()（golden fConfiguration 的 Tray 表）
                 'Motor/mymotor.h'],   # //AI(W906-EVB3) 20260928 [W906]：TF-4 MOT[MManualTray3].ClearTray（Motor/mymotor.h:385 MOT[]、:349 ClearTray；同 FileRW/TrayForm.gen.inc:23）
    # cAuthority.h 帶進 language.h（TWinControl 與 Public/HTEdit.h 重複）→ 只前置宣告要用的（同 IniConfig）
    'decls': ['extern bool bAuthCriticalPara[26];   // cAuthority.h:64（golden cAuthority.h）'],
    'blocks': [],
    'overrides': [],
}
