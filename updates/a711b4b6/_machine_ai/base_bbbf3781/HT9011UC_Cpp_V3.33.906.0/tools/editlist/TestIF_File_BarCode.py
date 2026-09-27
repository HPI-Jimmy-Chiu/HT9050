# -*- coding: utf-8 -*-
# tools/editlist/TestIF_File_BarCode.py -- gen_editlist.py 的結構設定（C 形狀：具名替身）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only TestIF_File_BarCode
#
# Steven 團隊 20260925：TestIF_File 的 TfBarCode 半邊（2DID／Bar Code／Shuttle Float Check 設定）——
# golden V912 TfBarCode（BarCode\BarCode.cpp，12326 行，cp950）。頁面 web/page/Setup.BarCode.html。
# 一律照 V912（RULINGS_20260925.md 第 37 條）；客戶專屬條件（第 25 條）：編得過就照 golden 留著，需要額外移植的才擋掉並註記。
#
# 結構名 TestIF_File_BarCode：tag "TestIF_File" 已被 A 形狀 FileRW/TestIF_File.cpp 佔用（同 TestIF_File_QAMode 的理由）。
#
#   讀：ReadFile（:673）＝ <recipe>\HandlerCondition.Data [Configuration]＋[Lot Verification]
#       （CC_KYEC_XILINX：D:\HT9045\system\Barcode.ini [Configuration_Barcode(XILINX)]）→ TestIF_File.*
#   開頁：FormShow（:332）＝ DoIniDataToForm（:1055，TestIF_File → 元件）→ 顯示／權限（含 SetVisible :628）
#   存檔：spbSaveClick（:1268）＝ A02 → Multi2D 檢查 → 86 個 WriteIniData（:1303-:1456）→ ReadFile → spbStartCom->Click()
#         → fMain->BackupSetupFile → （bReadClipCodeFromUnloader）Write／ReadUnloaderClipIni → （CC_JCET）JCETWhite2DIDShow
#   關頁：FormClose（:531）＝ DoIniDataToForm（JerryYang 20250411：離開頁面刷新一次，避免誤存檔）＝ PageDesc::reload
#   開機／換配方：golden TfMain::DoReadLastData main.cpp:9361 fBarCode->ReadFile()、:9405 fBarCode->DoIniDataToForm()
#         → FileRW_BarCode_ReadFile()／FileRW_BarCode_DoIniDataToForm()（FileRW/TestIF_File_BarCode.cpp）。
#
# 客戶專屬（Steven 20260925：先跳過，只註記；每一條見 replace 的原因）：
#   elUnloaderClip（CosFunction.bReadClipCodeFromUnloader：FUNC_CC_HONPREC_QC、FUNC_CC_JSSI_Semiconductor）——
#     golden 在建構子 new HTEditList＋InitUnloaderClipEdtList（:11537，12 筆註冊到 ccdUnloader[].sIP／sPort）。
#     移植樹沒有 ccdUnloader[]（uCCDUnloaderClip 物件，aoutarm9045.cpp:2322 的 gate 同一個缺口）→ 不 new，elUnloaderClip 保持 NULL；
#     golden Read／WriteUnloaderClipIni 本身有 `if(elUnloaderClip!=NULL)` 守衛 → 照轉，非 clip 客戶的行為與 golden 相同。
#   CC_JCET JCETWhite2DIDShow、cbUsePinInspectionClick（KYEC Pin1 切換 SendCCDCommand）、CC_KYEC_XILINX 寫 Barcode.ini（照轉，會寫）。
import os
import re

G = r'D:\HT9045_ref\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
_cpp = open(os.path.join(G, 'BarCode', 'BarCode.cpp'), 'rb').read().decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')
F = '"TfBarCode"'

METHODS = ['TfBarCode', 'FormShow', 'FormClose', 'SetVisible', 'ReadFile', 'DoIniDataToForm', 'spbSaveClick',
           'SetMulti2DMap', 'rgMulti2DTypeClick', 'chkMulti2DIDClick', 'CheckMulti2DMap',
           'GetBarcodeByServerData', 'JCETUseMakeWhite2DIDList', 'spbStartComClick', 'InitSht2DCodeComPort',
           'ReadUnloaderClipIni', 'WriteUnloaderClipIni']


def _span(meth):
    for i, l in enumerate(_cpp):
        if re.match(r'^[^\n/]*\bTfBarCode::' + meth + r'\s*\(', l):
            d, seen = 0, False
            for j in range(i, len(_cpp)):
                s = _cpp[j].split('//')[0]
                d += s.count('{') - s.count('}')
                seen = seen or '{' in s
                if seen and d == 0:
                    return i + 1, j + 1
    raise SystemExit('span %s' % meth)


SPANS = {m: _span(m) for m in METHODS}


def L(meth, text, nth=1):
    """golden 行號（1 起）：meth 本體裡第 nth 個含 text 的行。行號漂了就中止（不蓋錯地方）。"""
    a, b = SPANS[meth]
    k = 0
    for gl in range(a, b + 1):
        if text in _cpp[gl - 1]:
            k += 1
            if k == nth:
                return gl
    raise SystemExit('L(%s, %r) not found' % (meth, text))


def RANGE(meth, t1, t2, nth=1):
    a = L(meth, t1, nth)
    for gl in range(a, SPANS[meth][1] + 1):
        if t2 in _cpp[gl - 1]:
            return a, gl
    raise SystemExit('RANGE(%s,%r,%r)' % (meth, t1, t2))


def CB(n):
    return 'EL<TComboBox>(%s, "%s")' % (F, n)


_ctor = RANGE('TfBarCode', 'BarcodeCOM[0]=Barcode_1;', 'ShowBinMappingAuto_Index_Ptr[i]->Tag        =i;')
_ctor = (_ctor[0], _ctor[1] + 1)          # 含收尾的 }
_clip_new = RANGE('TfBarCode', 'elUnloaderClip    =new HTEditList;', 'InitUnloaderClipEdtList();')
_fc_test = RANGE('FormClose', 'if(bStartTest[3]==true)', 'tmr1->Enabled=false;', 1)
_fc_test = (_fc_test[0], L('FormClose', 'tmr1->Enabled=false;', 2) + 1)   # 含收尾的 }
_summary = RANGE('ReadFile', '#ifdef SOFT_SIMULTE', '#endif', 1)
_summary = (_summary[0], L('ReadFile', '#endif', 2))                        # :948-959 整段（兩個 #endif 的第二個）
_com = RANGE('InitSht2DCodeComPort', '#ifndef SOFT_SIMULTE', '#endif')
_clip_loop = RANGE('ReadUnloaderClipIni', 'for(int i=eUnloaderClipAuto1;', 'ccdUnloader[i].SetCommParameter')
_clip_loop = (_clip_loop[0], _clip_loop[1] + 1)

REPLACE = [
    # ---- 建構子（golden :131，CreateForm HT9045.cpp）
    ('TfBarCode', _ctor[0], _ctor[1],
     '建構子前段：BarcodeCOM[]（RS232 讀碼器 TComm）、2D socket 收資料清單、2DID 名單／排序清單、CSV 快取、iBarCodeNo 計數、'
     'Bottom 8CCD 通道、TMyTray 顏色表、GPIB 測試旗標、tmr1、mtBarcodeSetDefaultView、edCCDAddress／Port 指標、TimerProcess2DData、'
     'LastBarcodeLog.ini（執行期 log 檔名）、dVisionVer、Barcode Tray record 指標 —— 全是執行期物件，不是表單讀寫檔狀態'
     '（移植樹的執行期部分在 BarCode/BarCode_*.cpp）。表單橋只留 bShow=false（:229）',
     'bShow=false;'),
    ('TfBarCode', _clip_new[0], _clip_new[1],
     '客戶專屬（bReadClipCodeFromUnloader：HONPREC_QC／JSSI）：new HTEditList＋InitUnloaderClipEdtList（:11537）註冊到 ccdUnloader[].sIP／sPort，'
     '移植樹沒有 ccdUnloader[]（uCCDUnloaderClip）→ 不建，elUnloaderClip 保持 NULL（golden Read／WriteUnloaderClipIni 自帶 NULL 守衛）',
     'filerw::ELTodo("golden BarCode.cpp:322-323 CosFunction.bReadClipCodeFromUnloader: elUnloaderClip=new HTEditList + InitUnloaderClipEdtList() '
     '(UnloaderClipSetting.ini -> ccdUnloader[].sIP/sPort) not ported -- port has no ccdUnloader[] (uCCDUnloaderClip); elUnloaderClip stays NULL");'),
    # ---- FormShow（golden :332）
    ('FormShow', L('FormShow', 'Left=10;'), L('FormShow', 'Top =0;'), 'Left／Top：視窗位置（HTML 不用）', ';'),
    ('FormShow', L('FormShow', 'mtBarcodeSetDefaultView();'), L('FormShow', 'mtBarcodeSetDefaultView();'),
     'mtBarcodeSetDefaultView（:1475）：純畫面（fLotInfo->sgBarcode 表頭字串、本表單 TMyTray 格子 mtBarcodeInSh／myInShuttleLabel 的格數與顏色），'
     '不讀寫檔、不改 TestIF_File；移植樹 TfLotInfo 門面沒有 sgBarcode', ';'),
    ('FormShow', L('FormShow', 'fBarCode->Caption='), L('FormShow', 'fBarCode->Caption='), 'fBarCode->Caption：視窗標題（HTML 自己有）', ';'),
    # ---- FormClose（golden :531）
    ('FormClose', _fc_test[0], _fc_test[1],
     'bStartTest[3]：只有 btnGPIBReader4Click（手動 GPIB 讀碼測試，未轉）會設 true；FormShow :348 已設 false → 這個分支在表單橋永遠不成立'
     '（分支內 SetNoiseDelay／TestISTimeOut／TestSocket.ClearAll 是執行期狀態）', ';'),
    # ---- ReadFile（golden :673）
    ('ReadFile', _summary[0], _summary[1],
     'fMain->pnlSaveSummary->Visible：主畫面「手動存 Summary」面板的顯示（純畫面）；移植樹 TfMain 門面沒有 pnlSaveSummary（同 BarCode/BarCode.cpp GATE(G-BC-SAVESUMMARY)）', ';'),
    ('ReadFile', L('ReadFile', 'fMain->ShowOCRState(0);'), L('ReadFile', 'fMain->ShowOCRState(0);'),
     'fMain->ShowOCRState(0)：主畫面 OCR 狀態燈（純畫面）；移植樹 TfMain 門面沒有（同 GATE(G-BC-OCRSTATE)）。上面四個 SendMSG_CMD 照 golden 送', ';'),
    ('ReadFile', L('ReadFile', 'Change2DSetupFile();'), L('ReadFile', 'Change2DSetupFile();'),
     'Change2DSetupFile（:5548）：設 iConntectionOkTask／iBottomConntectionOkTask1/2=1，讓 TimerCCDInitialTimer 重新對 2D CCD 下換配方指令；'
     '移植樹沒有這三個 task 與計時器（同 GATE(G-BC-TAIL)、forms/fLotInfo.cpp 的 Change2DSetupFile gate）',
     'if(BAR_CODE_INSTALL!=ebctUninstall) filerw::ELTodo("golden BarCode.cpp:1050 Change2DSetupFile(): iConntectionOkTask/iBottomConntectionOkTask=1 '
     '(2D CCD re-init with the new recipe via TimerCCDInitialTimer) not ported -- the CCD is NOT told about this recipe");'),
    ('ReadFile', L('ReadFile', 'SetSFCCheckStepCount();'), L('ReadFile', 'SetSFCCheckStepCount();'),
     'SetSFCCheckStepCount（:3150）：iSFCTotalMoveStep=InArmSuck.iShtCol(*2) —— TfBarCode 成員，移植樹沒有這個成員也沒有讀者'
     '（grep iSFCTotalMoveStep 0 筆）；Shuttle Float Check 本機 SHT_FLOATING_CHK=0', ';'),
    ('ReadFile', L('ReadFile', 'mtBarcodeSetDefaultView();'), L('ReadFile', 'mtBarcodeSetDefaultView();'),
     'mtBarcodeSetDefaultView：純畫面（見 FormShow 同一條）', ';'),
    ('ReadFile', L('ReadFile', 'bCSVCacheLoaded = false;'), L('ReadFile', 'bCSVCacheLoaded = false;'),
     'bCSVCacheLoaded：CSV 比對快取旗標（TfBarCode 成員，LoadBarcodeCSVCache :11995 未轉）→ 本 TU 的 static 照設', 'bCSVCacheLoaded = false;'),
    # ('ReadFile', L('ReadFile', 'TestIF_File.bEnableBarcodeCSVCompare='), L('ReadFile', 'TestIF_File.bEnableBarcodeCSVCompare='), # AI(W906-TIF912-R10) 20260926: cprod.h 已補這一欄（RULINGS_20260926 第 10 條），照原因欄說的「補進 cprod.h 後拿掉這條」
     # 'TestIF_File.bEnableBarcodeCSVCompare（golden 912 cprod.h，Ifor 20260511）：移植樹 SYSTEM_TEST_IF 沒有這欄 → 讀進本 TU 的 static '
     # 'bEnableBarcodeCSVCompare（DoIniDataToForm／存檔來回照 golden，原值不會被存檔蓋掉）；執行期讀者 DoBarcodeCSVCompare 未轉。'
     # '要補欄位：Jimmy／JerryYang 補進 cprod.h 後拿掉這條與 DoIniDataToForm 那條',
     # 'bEnableBarcodeCSVCompare=ReadIniData(szDir, sGroup, "bEnableBarcodeCSVCompare", false); //Ifor 20260511 add: Barcode CSV Compare'),
    # ---- DoIniDataToForm（golden :1055）
    ('DoIniDataToForm', L('DoIniDataToForm', 'Multi2DSiteCH[i][j]->ItemIndex=0;', 1), L('DoIniDataToForm', 'Multi2DSiteCH[i][j]->ItemIndex=0;', 1),
     'TComboBox ItemIndex 的 VCL 連動（帶出 Text）：vclcompat 只有欄位 → filerw::ELComboIndex', 'filerw::ELComboIndex(Multi2DSiteCH[i][j], 0);'),
    ('DoIniDataToForm', L('DoIniDataToForm', 'Multi2DSiteCH[i][j]->ItemIndex=0;', 2), L('DoIniDataToForm', 'Multi2DSiteCH[i][j]->ItemIndex=0;', 2),
     '同上', 'filerw::ELComboIndex(Multi2DSiteCH[i][j], 0);'),
    ('DoIniDataToForm', L('DoIniDataToForm', 'Multi2DSiteCH[i][j]->ItemIndex=TestIF_File.iMulti2DMap[i][j];'),
     L('DoIniDataToForm', 'Multi2DSiteCH[i][j]->ItemIndex=TestIF_File.iMulti2DMap[i][j];'),
     '同上', 'filerw::ELComboIndex(Multi2DSiteCH[i][j], TestIF_File.iMulti2DMap[i][j]);'),
    # ('DoIniDataToForm', L('DoIniDataToForm', 'cbBarcodeCSVCompare->Checked=TestIF_File.bEnableBarcodeCSVCompare;'), # AI(W906-TIF912-R10) 20260926: cprod.h 已補這一欄（RULINGS_20260926 第 10 條），照原因欄說的「補進 cprod.h 後拿掉這條」
     # L('DoIniDataToForm', 'cbBarcodeCSVCompare->Checked=TestIF_File.bEnableBarcodeCSVCompare;'),
     # 'TestIF_File.bEnableBarcodeCSVCompare 移植樹沒有 → 本 TU 的 static（見 ReadFile 同一條）',
     # 'EL<TCheckBox>(%s, "cbBarcodeCSVCompare")->Checked=bEnableBarcodeCSVCompare;          //Ifor 20260511 add: Barcode CSV Compare' % F),
    # ---- spbSaveClick（golden :1268）
    ('spbSaveClick', L('spbSaveClick', 'Close();'), L('spbSaveClick', 'Close();'),
     'Close()：golden A02 權限不足時關表單 → 伺服器端記 closed，頁面自己關；沒寫檔 → PageDesc::reload（golden FormClose 的 DoIniDataToForm）還原替身',
     'filerw::ELMark("closed");'),
    ('spbSaveClick', L('spbSaveClick', 'if(BOTTOM_2DID)'), L('spbSaveClick', 'if(BOTTOM_2DID)'),
     '存檔標記：這一行之後第一個 WriteIniData（"Bottom 2D"）就落地 —— golden 在它之後仍可能 return（:1309 Multi2D 2CCD 檢查），'
     '但檔案已經寫了一鍵，所以 saved 以這裡為準（savedMark="BC_WriteIniData"）',
     'filerw::ELMark("BC_WriteIniData"); if(BOTTOM_2DID)'),
    ('spbSaveClick', L('spbSaveClick', 'rgMulti2DType->ItemIndex=0;'), L('spbSaveClick', 'rgMulti2DType->ItemIndex=0;'),
     '產生器的元件改寫不接在 "->" 後面：同一行等價改寫（Multi2D 2CCD 不合法時 golden 把選項改回 0）',
     'EL<TRadioGroup>(%s, "rgMulti2DType")->ItemIndex=0;' % F),
    ('spbSaveClick', L('spbSaveClick', 'spbStartCom->Click();'), L('spbSaveClick', 'spbStartCom->Click();'),
     'spbStartCom->Click()：VCL Click() 觸發 OnClick＝spbStartComClick（:2520，DFM 綁定）', 'BC_spbStartComClick();'),
    ('spbSaveClick', L('spbSaveClick', 'spbSave->Down=false;'), L('spbSaveClick', 'spbSave->Down=false;'), 'spbSave->Down：按鈕外觀', ';'),
    # ---- SetMulti2DMap（golden :8830）
    ('SetMulti2DMap', L('SetMulti2DMap', 'Multi2DSiteCH[i][j]->Clear();'), L('SetMulti2DMap', 'Multi2DSiteCH[i][j]->Clear();'),
     'TComboBox::Clear 的 VCL 語意（Items 清空＋ItemIndex=-1＋Text=""；vclcompat Clear 只清 Items）',
     'BC_CbClear(Multi2DSiteCH[i][j]);'),
    ('SetMulti2DMap', L('SetMulti2DMap', 'Multi2DSiteCH[i][j]->ItemIndex=0;'), L('SetMulti2DMap', 'Multi2DSiteCH[i][j]->ItemIndex=0;'),
     'TComboBox ItemIndex 的 VCL 連動（帶出 Text）', 'filerw::ELComboIndex(Multi2DSiteCH[i][j], 0);'),
    # ---- GetBarcodeByServerData（golden :11348）
    ('GetBarcodeByServerData', L('GetBarcodeByServerData', 'return AnsiString().sprintf('), L('GetBarcodeByServerData', 'return AnsiString().sprintf('),
     'AnsiString 經 "..." 傳給 sprintf：BCB 可以（AnsiString 就是一個 char*），C++17／g++ 不行 → 同一個格式、傳 c_str()',
     'return AnsiString().sprintf("%s%s",TestIF_File.asMes2DID_URL.c_str(),fLotInfo->edtSysLotID->Text.c_str());'),
    # ---- InitSht2DCodeComPort（golden :2528）
    ('InitSht2DCodeComPort', _com[0], _com[1],
     'RS232 讀碼器（BAR_CODE_INSTALL=OutShtAMD／InShtIntel）重開 COM：Barcode_1..4 是 TComm 元件、InitBarCodeRS232（:1715）開 COM port，'
     '移植樹沒有。上面的 return 條件照 golden（CCD／EtherNet／OCR／未安裝直接 return），SOFT_SIMULTE 下 golden 本來就不做',
     'if(!kBC_SOFT_SIMULTE) filerw::ELTodo("golden BarCode.cpp:2536-2568 InitSht2DCodeComPort: RS232 2D reader COM restart (Barcode_1..4 StopComm/StartComm, '
     'InitBarCodeRS232) not ported");'),
    # ---- ReadUnloaderClipIni（golden :11562）
    ('ReadUnloaderClipIni', _clip_loop[0], _clip_loop[1],
     'ccdUnloader[i].SetCommParameter：移植樹沒有 ccdUnloader[]（見建構子那條）；elUnloaderClip 在本 TU 永遠 NULL，走不到這裡',
     'filerw::ELTodo("golden BarCode.cpp:11568-11571 ccdUnloader[].SetCommParameter not ported (no uCCDUnloaderClip in the port)");'),
]

STRUCT = {
    'struct': 'TestIF_File_BarCode',
    'prefix': 'BC',
    'class': 'TfBarCode',
    'cpp': 'BarCode\\BarCode.cpp',
    'h': 'BarCode\\BarCode.h',
    'files': ['<recipe>\\HandlerCondition.Data [Configuration]／[Lot Verification]',
              'D:\\HT9045\\system\\Barcode.ini [Configuration_Barcode(XILINX)]（CC_KYEC_XILINX）',
              'AuthPath\\UnloaderClipSetting.ini（elUnloaderClip，bReadClipCodeFromUnloader，未移植）'],
    'lists': [],
    'methods': METHODS,
    'save_methods': ['spbSaveClick'],
    'params': {'TfBarCode': '', 'FormShow': '', 'FormClose': '', 'spbSaveClick': '', 'rgMulti2DTypeClick': '',
               'chkMulti2DIDClick': '', 'spbStartComClick': ''},
    'rettype': {'CheckMulti2DMap': 'bool', 'GetBarcodeByServerData': 'AnsiString', 'JCETUseMakeWhite2DIDList': 'bool'},
    'members': [
        'bool bShow=false;                        // golden BarCode.h:777（本 TU 自己的；移植樹 fBarCode 門面沒有這個成員）',
        'bool bStartTest[4]={false,false,false,false};   // golden BarCode.h:717',
        'bool bCSVCacheLoaded=false;              // golden BarCode.h:993',
        # 'bool bEnableBarcodeCSVCompare=false;     // golden 912 TestIF_File.bEnableBarcodeCSVCompare（移植樹 SYSTEM_TEST_IF 沒有，見 ReadFile 的 replace）', # AI(W906-TIF912-R10) 20260926: cprod.h 已補這一欄（RULINGS_20260926 第 10 條），照原因欄說的「補進 cprod.h 後拿掉這條」
        'HTEditList* elUnloaderClip=NULL;         // golden BarCode.h:982（客戶專屬，本 TU 不 new，見建構子的 replace）',
        'AnsiString GetUnloaderClipFileName(){return AnsiString("UnloaderClipSetting.ini");}   // golden BarCode.h:980',
        'TComboBox* Multi2DSiteCH[2][2]={{NULL,NULL},{NULL,NULL}};   // golden BarCode.cpp:49（檔案層級全域；建構子 :316-319 指到 cbAa／cbAb／cbBa／cbBb）',
        # VCL TCustomComboBox::Clear（CB_RESETCONTENT）：Items 清空、沒有選取、編輯框清空（vclcompat Clear 只清 Items）
        'void BC_CbClear(TComboBox* cb) { cb->Clear(); cb->ItemIndex=-1; cb->Text=""; }',
    ],
    'replace': REPLACE,
    'blocks': [],
    'includes': ['cprod.h', 'Config.h', 'LastSet.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'MessageDef.h', 'forms/fMain.h', 'forms/fLotInfo.h', 'csystem.h', 'cinitial.h',
                 'database.h'],   # database.h：HSys（DoIniDataToForm :1140-1154 的 COM／CCD IP／Port 顯示）
    'decls': ['#ifdef SOFT_SIMULTE',
              'static const bool kBC_SOFT_SIMULTE=true;    // golden InitSht2DCodeComPort 的 #ifndef SOFT_SIMULTE（見 replace）',
              '#else',
              'static const bool kBC_SOFT_SIMULTE=false;',
              '#endif',
              '// golden BarCode.h:83-91 enum eMulti2DType（移植樹在 BarCode/BarCode_Shuttle2_CCDScan.h:197，該標頭帶 aHotPlateSubstrate.h，'
              '// 與 HTEditList.h 重複定義 TList／uPlateInfo → 不 include，這裡照 golden 值宣告）',
              'enum eMulti2DType { e1x2In1CCD=0, e2x1In1CCD=1, e2x1In2CCD=2, e2x2In1CCD=3, e2x2In2CCD=4, eMulti2DTypeTotal };'],
    'overrides': [],
}
