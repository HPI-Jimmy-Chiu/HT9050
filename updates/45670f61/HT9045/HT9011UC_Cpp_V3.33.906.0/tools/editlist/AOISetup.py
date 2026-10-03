# -*- coding: utf-8 -*-
# tools/editlist/AOISetup.py -- gen_editlist.py 的結構設定（C 形狀：具名替身，一個結構一個檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only AOISetup
#
# //AI(W906-FRW-S69) 20260926: 新檔（Steven 團隊，S69「AOI.Data：TFrmAOI 整檔不在移植樹」）。
#   golden V912 TFrmAOI（fAOI.cpp，10179 行，cp950）的讀寫段：<配方>\AOI.Data 的 [SETTING]／[DutOnOff_BGAView]／[DutOnOff_PADView]／
#   [RS232]／[AOITRAY] → 全域 tAOISetup（cprod.h:3124）＋ ScannerAOIIF（cprod.h:141）＋ bVitroxBGA/PADViewUse/Map（golden fAOI.cpp:63-74
#   檔案層級全域 → 移植樹 forms/fAOI.cpp）＋ MOT[MMScanAOI].Tray.Data（AOI Tray Edit）。
#   網頁沒有這一頁（web/page/ 沒有 AOI 頁；HW.HandlerSys 的 AOI／Scanner_AOI 是安裝選項，不是這個表單）→ 只翻、不登記 PageDesc
#   （同 c675594d Rotate／AutoAlignment、c79ee4e9 FixAICCD／Magazine 的做法）。
#   結構名 AOISetup：write-inventory.md 的落點名（「未移植 → AOISetup.cpp」），同 Rotate 的慣例（全域 tAOISetup 去掉前綴）。
#   一律照 V912（RULINGS 第 37 條）；客戶專屬條件（S25）：讀檔尾巴 CC_KYEC_LEE 的鎖定段要 ttbInsp（Top&Bottom，未移植）才會跑 → 擋，註記。
#   AOI 的相機／雷射掃描通訊與運動不接（S48，Jimmy）：存檔尾巴的 spbStopCom／spbStartCom（RS232 起停）、ttbInsp->DoCCDLightDown。
#
# 轉的 golden 方法：
#   fAOI_ReadFile（:3369）        ＝ 讀 AOI.Data → tAOISetup／ScannerAOIIF／Vitrox 兩組 Dut on/off／MOT[MMScanAOI].Tray.Data，
#                                   接著 fAOI_DoIniDataToForm＋UpDataTrayData。只用 ReadIniData（不會補寫缺鍵，不改任何檔）。
#                                   golden 呼叫點：DoReadLastData main.cpp:9374（開機 :9993、換配方 :25722／:25770）、FormShow :3776、
#                                   spbSaveClick :3338、uteach.cpp:1822（TfTeach，Top&Bottom 才跑）。
#   fAOI_DoIniDataToForm（:3611） ＝ 結構 → 元件（替身）。golden 只從 fAOI_ReadFile 呼叫。
#   UpdateAOIFailBinTypetoForm（:4653）＝ rgAOIFailBinType → lblAOIBinSel1/2、cbAOIFialAndTestPass 顯示（DoIniDataToForm 呼叫）。
#   UpDataTrayData（:4630）       ＝ InitialOK 之後才做：MOT[MMScanAOI].Tray.X/YItem ← MOT[MManualTray2]（資料，照 golden 保留）；
#                                   mtAOIBuffer（TTMyTray 盤面元件）的三行是純畫面 → replace 成空敘述。
#   CheckFailBin（:3899）         ＝ 存檔前「AOI Fail Bin 不可是 Pass Bin」的檢查（只提示，不擋存檔）。移植樹門面 forms/fAOI.cpp 也有一份
#                                   （讀門面自己的元件）；這裡轉的是讀替身的那份。門面那 14 個元件由 FileRW/AOISetup.cpp 開機時直接當替身
#                                   （ELKeep），兩份讀到同一組物件。
#   spbSaveClick（:3120）         ＝ 存檔鈕：A02 → CheckFailBin 提示 → WriteIniData 逐鍵寫 AOI.Data → (Top&Bottom 的 elParameter) →
#                                   fAOI_ReadFile → fMain->BackupSetupFile → RS232 起停 → CCD 燈。
#                                   ⚠ 目前沒有觸發點（沒有頁面；FileRW/AOISetup.cpp 的 FileRW_AOISetup_spbSaveClick() 沒有呼叫者）。
#
# 不轉：
#   建構子（:160）：非 Top&Bottom 只設 iQuotient／iRemainder=0（本 TU 的 static，零初值同）、elParameter／tsTBAOIFailStop=NULL、
#     fShow=false、mtDut* 顏色表（TTMyTray 畫面）、scroAOIFailCountLinkLotRunMode 捲軸（畫面）→ 沒有讀寫檔。
#     Top&Bottom（USE_Scanner_AOI_Inspection==eBtnAOI_TopBottomInstall）才 new TTopBottomInspect＋IntialParameter（elParameter 註冊
#     ttbInsp 的 70 多個欄位，含 AOIFailList 20 組動態元件、cbTBAOIFailStop[15]）→ TTopBottomInspect 類別整個不在移植樹（socket 通訊＋
#     運動狀態機，Jimmy）→ 不轉；讀檔／存檔裡 elParameter 與 ttbInsp 的段落一律擋（Top&Bottom 機台才回報，見 REPLACE）。
#   FormShow :3769（開頁＝fAOI_ReadFile＋分頁顯示）、FormClose :9037、sbtExitClick、*Click 小鍵盤、mtDutOnOff_* 點格子（改
#     bVitrox*ViewUse）、DrawSitePanelVitrox1/2、mtAOIBufferMouseDown（EditTray）→ 頁面接上時再加進 METHODS（見交件報告「建頁備忘」）。
#   其餘 AOI 執行期（DoAOIFunction、Scanner／Top Scanner 流程、TTopBottomInspect、RS232／socket）→ Jimmy。
#
# ⚠ golden 看起來錯（(3)～(5) 照翻，不修；(1)(2) 已修，見下面 S152）：
#   (1) 存檔 :3300 把 edt_SDelayTopScanAOI（Top 的延遲）寫到 "StartDelayTimeScanAOIView"——那是 :3236 剛寫過的 Bottom 鍵，被蓋掉；
#       讀檔 :3494 讀 Top 用的是 "StartDelayTimeTopScanAOIView"（沒有人寫）。⇒ 存一次，Bottom 的 Start Delay 變成 Top 欄位的值
#       （Bottom-only 機台 Top 欄位顯示讀不到鍵的預設 10）。
#   (2) 存檔 :3301 寫 "TimeOutTopScanAOIView"，讀檔 :3495 讀 "TimeOutScanTopAOIView" ⇒ Top Timeout 存了讀不回來（永遠預設 3）。
# //AI(W906-FRW-S152) 20260927 [W906]: (1)(2) 偏離 golden（Steven Q31＝A'，RULINGS_20260926 S152：906 C++ 單邊修、接受 AOI.Data 格式跟
#   BCB6 機台不一致；V912 不改，只通報 Jimmy）。兩個都改「寫」的那一行（REPLACE 的 _SV_TOPDLY／_SV_TOPTO），讀檔 fAOI_ReadFile 照 golden 不動：
#     (1) Top 延遲改寫 "StartDelayTimeTopScanAOIView"（讀檔 :3494 讀的鍵）；Bottom 的 "StartDelayTimeScanAOIView" 只剩 :3236 寫 Bottom 的值。
#     (2) 鍵名統一成讀檔 :3495 的 "TimeOutScanTopAOIView"（不是寫檔的 "TimeOutTopScanAOIView"）。理由：讀的一邊不動 ⇒
#         (a) BCB6 機台寫的舊檔，906 讀到的值跟 BCB6 自己讀到的一樣（Top Timeout 仍是預設 3、Bottom Start Delay 仍是 Top 欄位的值）——
#             同一份配方在兩種機台上跑的值相同，不會因為換到 906 就突然變；
#         (b) 906 存過的檔拿回 BCB6 機台，BCB6 的讀檔也讀到正確的 Top 延遲／Top Timeout（BCB6 讀的就是這兩個鍵）。
#         代價：BCB6 存檔寫的 "TimeOutTopScanAOIView"（操作員在 BCB6 畫面打的 Top Timeout）906 不讀，檔裡的舊值留著、沒有人讀
#         （WriteIniData 不刪鍵）；906 也不再寫它。另一個選項（統一成寫檔的鍵名）→ 交件報告 R 題，Steven 可推翻。
#   (3) Top Scan AOI 的 5 個警報設定（bEnabledTopScanAOIBySiteAlarm／ByArmAlarm、iTopScanAOIAlarmCountBySite／ByArm、
#       bEnabledTopScanAOIUnUseFailBin）存檔 :3296-3303 有寫，fAOI_ReadFile 沒讀 ⇒ tAOISetup 裡恆 0，畫面顯示 0，再存就寫 0。
#   (4) iBallDamageType：讀檔 :3453 讀、DoIniDataToForm :3664 顯示，存檔沒寫。
#   (5) fScannerICGain 讀兩次（:3480、:3503，結果相同，無害）。
import os
import re

_G = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
_cpp = open(os.path.join(_G, 'fAOI.cpp'), 'rb').read().decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')
_F = 'TFrmAOI'

METHODS = ['fAOI_ReadFile', 'fAOI_DoIniDataToForm', 'UpdateAOIFailBinTypetoForm', 'UpDataTrayData', 'CheckFailBin', 'spbSaveClick']


def _span(meth):
    for i, l in enumerate(_cpp):
        if re.match(r'^[^\n/]*\bTFrmAOI::' + meth + r'\s*\(', l):
            d, seen = 0, False
            for j in range(i, len(_cpp)):
                s = _cpp[j].split('//')[0]
                d += s.count('{') - s.count('}')
                seen = seen or '{' in s
                if seen and d == 0:
                    return i + 1, j + 1
    raise SystemExit('AOISetup.py: span %s' % meth)


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
    raise SystemExit('AOISetup.py: L(%s, %r) not found' % (meth, text))


def _expect(gl, text):
    if _cpp[gl - 1].strip() != text:
        raise SystemExit('AOISetup.py: golden :%d is %r, expected %r' % (gl, _cpp[gl - 1].strip(), text))
    return gl


def _block_end(meth, start):
    """start 行（if/for 那一行）之後第一個 `{` 對應的 `}` 的行號（golden 原文，註解不算）。"""
    d, seen = 0, False
    a, b = SPANS[meth]
    for gl in range(start, b + 1):
        s = _cpp[gl - 1].split('//')[0]
        d += s.count('{') - s.count('}')
        seen = seen or '{' in s
        if seen and d == 0:
            return gl
    raise SystemExit('AOISetup.py: block end %s:%d' % (meth, start))


_TB = 'USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall'

# ---- spbSaveClick（golden :3120）
_SV_CLOSE = L('spbSaveClick', 'Close();')
_SV_WR1 = L('spbSaveClick', 'WriteIniData(szDir, "SETTING", "EnabledTopView", cbEnabledTopView->Checked);')
_SV_POS = L('spbSaveClick', 'WriteIniData(szDir, "SETTING", "bEnabledPositionByAOI",')
# AI(W906-FRW-S152) 20260927: Q31＝A' 的兩行（golden :3300／:3301；_expect 釘住原文，golden 改了就中止）
_SV_TOPDLY = _expect(L('spbSaveClick', 'edt_SDelayTopScanAOI->Text'),
                     'WriteIniData(szDir, "SETTING", "StartDelayTimeScanAOIView",     edt_SDelayTopScanAOI->Text);')
_SV_TOPTO = _expect(_SV_TOPDLY + 1, 'WriteIniData(szDir, "SETTING", "TimeOutTopScanAOIView",         edt_TimeoutTopScanAOI->Text);')
_SV_TBGRID = L('spbSaveClick', 'if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall)')
_SV_TBGRID_E = _block_end('spbSaveClick', _SV_TBGRID)
_SV_EL = L('spbSaveClick', 'if(elParameter)')
_SV_EL_E = _block_end('spbSaveClick', _SV_EL)
_SV_COM1 = L('spbSaveClick', 'if(USE_Scanner_AOI_Inspection==true)')
_SV_COM1_E = _block_end('spbSaveClick', _SV_COM1)
_SV_COM2 = L('spbSaveClick', 'if(USE_Top_Scanner_AOI_Inspection==true)')
_SV_COM2_E = _block_end('spbSaveClick', _SV_COM2)
_SV_LIGHT = L('spbSaveClick', 'if(FrmAOI->RunTopBottomInspect())')
_SV_LIGHT_E = _expect(_SV_LIGHT + 1, 'FrmAOI->ttbInsp->DoCCDLightDown(false, true);')

# ---- fAOI_ReadFile（golden :3369）
_RD_POS = L('fAOI_ReadFile', 'ScannerAOIIF.bEnabledPositionByAOI')
_RD_EL = L('fAOI_ReadFile', 'if(elParameter)')
_RD_TB = L('fAOI_ReadFile', 'if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall && ttbInsp)')
_RD_TB_E = _block_end('fAOI_ReadFile', _RD_TB)
_RD_LAB = L('fAOI_ReadFile', 'if(USE_Scanner_AOI_Inspection>(int)eBtnAOI_Uninstall && fMain)')
_RD_LAB_E = _block_end('fAOI_ReadFile', _RD_LAB)
_RD_KYEC = L('fAOI_ReadFile', '//Eastsun 20260519 KYEC')
if not _cpp[_RD_KYEC - 1].lstrip().startswith('if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall)'):
    raise SystemExit('AOISetup.py: golden :%d 不是 KYEC 那一段的 if（golden 改了，重新核對）' % _RD_KYEC)
_RD_KYEC_E = _block_end('fAOI_ReadFile', _RD_KYEC)
if _RD_TB != _block_end('fAOI_ReadFile', _RD_EL) + 2:
    raise SystemExit('AOISetup.py: elParameter／ttbInsp 兩段之間不是只隔一個空行（golden 改了，重新核對）')

# ---- fAOI_DoIniDataToForm（golden :3611）
_DF_POS = L('fAOI_DoIniDataToForm', 'cb_EnabledPositionByAOI->Checked')

# ---- UpDataTrayData（golden :4630）
_UT_X = L('UpDataTrayData', 'mtAOIBuffer->XItem=')
_UT_Y = _expect(_UT_X + 1, 'mtAOIBuffer->YItem=MOT[MManualTray2].Tray.YItem;')
_UT_CELL = L('UpDataTrayData', 'mtAOIBuffer->SetCellColorIndex(')

_POSWHY = ('ScannerAOIIF.bEnabledPositionByAOI（golden cprod.h:140，Ifor 20251031 Move Position Provided By AOI）：移植樹 cprod.h 的 '
           'SYSTEM_SCANNER_AOI_IF 沒有這一欄（V912 新增在結構最後）。cprod.h 是全樹共用標頭，本工作不改 —— 讀、顯示、寫三處一起擋，'
           '存檔時這一鍵保持檔案原值（同 Rotate 的 ColCount／RowCount 做法）。補欄位片段見交件報告')

REPLACE = [
    # ---- spbSaveClick
    ('spbSaveClick', _SV_CLOSE, _SV_CLOSE,
     'Close()：golden A02（Operator 權限）時關表單（下一行 return）；網頁端記 closed，沒寫檔',
     'filerw::ELMark("closed");'),
    ('spbSaveClick', _SV_WR1, _SV_WR1,
     '存檔標記：golden 第一個 WriteIniData 就落地 —— 前面的 A02 會 return（沒寫檔），CheckFailBin 只提示、不 return',
     'filerw::ELMark("AO_WriteIniData"); WriteIniData(szDir, "SETTING", "EnabledTopView", EL<TCheckBox>("TFrmAOI", "cbEnabledTopView")->Checked);'),
    ('spbSaveClick', _SV_TOPDLY, _SV_TOPDLY,
     "AI(W906-FRW-S152) 20260927 [W906] 偏離 golden（Steven Q31＝A'，RULINGS_20260926 S152）：golden 把 Top 的延遲 edt_SDelayTopScanAOI "
     '寫進 Bottom 的鍵 "StartDelayTimeScanAOIView"（蓋掉 :%d 剛寫的 Bottom 值），讀檔 :%d 讀 Top 用 "StartDelayTimeTopScanAOIView" '
     '（golden 沒有人寫）→ 改寫讀檔用的鍵；讀檔照 golden。AOI.Data 格式從此跟 BCB6 機台不一致（BCB6 的舊檔讀起來跟 BCB6 相同），V912 不改'
     % (L('spbSaveClick', 'edt_SDelayScanAOI->Text'), L('fAOI_ReadFile', '"StartDelayTimeTopScanAOIView"')),
     'WriteIniData(szDir, "SETTING", "StartDelayTimeTopScanAOIView",  EL<TEdit>("TFrmAOI", "edt_SDelayTopScanAOI")->Text);'),
    ('spbSaveClick', _SV_TOPTO, _SV_TOPTO,
     "AI(W906-FRW-S152) 20260927 [W906] 偏離 golden（Steven Q31＝A'，RULINGS_20260926 S152）：golden 寫 \"TimeOutTopScanAOIView\"、"
     '讀檔 :%d 讀 "TimeOutScanTopAOIView" → Top Timeout 存了讀不回來。鍵名統一成讀檔的 "TimeOutScanTopAOIView"（改寫這一行；讀檔照 golden）'
     '：BCB6 舊檔讀起來跟 BCB6 相同（預設 3），906 存的檔 BCB6 也讀得到；BCB6 寫的 "TimeOutTopScanAOIView" 從此沒有人讀也沒有人寫。'
     '選哪個鍵名是 R 題（Steven 可推翻，tools/editlist/AOISetup.py 檔頭），V912 不改'
     % L('fAOI_ReadFile', '"TimeOutScanTopAOIView"'),
     'WriteIniData(szDir, "SETTING", "TimeOutScanTopAOIView",         EL<TEdit>("TFrmAOI", "edt_TimeoutTopScanAOI")->Text);'),
    ('spbSaveClick', _SV_TBGRID, _SV_TBGRID_E,
     'Top&Bottom：sgICSmall／sgICLarge 格子 → edlsSamll／edlsLarge（建構子 InitialEdList 建的 TEdit* 陣列，elParameter 註冊的元件）。'
     'TTopBottomInspect 與 elParameter 都不在移植樹 → Top&Bottom 機台回報 todo，其餘機台 golden 本來就不跑',
     'if(' + _TB + ') filerw::ELTodo("golden fAOI.cpp:%d-%d Top&Bottom AOI grid -> edlsSamll/edlsLarge not ported '
     '(TTopBottomInspect / elParameter, Jimmy)");' % (_SV_TBGRID, _SV_TBGRID_E)),
    ('spbSaveClick', _SV_EL, _SV_EL_E,
     'elParameter->SaveEditTextToFile(<配方>\\, "AOI.Data")：elParameter 只有 Top&Bottom 建構子才 new（建構子 :182 設 NULL）→ 其餘機台 golden 跳過。'
     'Top&Bottom 的 ttbInsp 欄位（[TopBottomInspect]／[ZPickOffset]／[Function Setting] 共 70 多鍵）不在移植樹 → 回報 todo，那些鍵保持檔案原值',
     'if(' + _TB + ') filerw::ELTodo("golden fAOI.cpp:%d-%d elParameter->SaveEditTextToFile(AOI.Data) not ported: '
     'Top&Bottom keys ([TopBottomInspect]/[ZPickOffset]/[Function Setting]) kept as on disk (TTopBottomInspect, Jimmy)");' % (_SV_EL, _SV_EL_E)),
    ('spbSaveClick', _SV_COM1, _SV_COM1_E,
     'Scanner AOI 的 RS232 起停（spbStopCom／spbStartCom → AOIComm->StopComm／RS232Init）：外部設備通訊，S48 歸 Jimmy。條件照 golden',
     'if(USE_Scanner_AOI_Inspection==true) filerw::ELTodo("golden fAOI.cpp:%d-%d Scanner AOI RS232 stop/start after save '
     '(spbStopCom/spbStartCom) not ported (device comm, Jimmy)");' % (_SV_COM1, _SV_COM1_E)),
    ('spbSaveClick', _SV_COM2, _SV_COM2_E,
     'Top Scanner AOI 的 RS232 起停（同上；golden 按的也是 spbStopCom／spbStartCom）：S48 歸 Jimmy。條件照 golden',
     'if(USE_Top_Scanner_AOI_Inspection==true) filerw::ELTodo("golden fAOI.cpp:%d-%d Top Scanner AOI RS232 stop/start after save '
     'not ported (device comm, Jimmy)");' % (_SV_COM2, _SV_COM2_E)),
    ('spbSaveClick', _SV_LIGHT, _SV_LIGHT_E,
     'RunTopBottomInspect()（ttbInsp && Top&Bottom && ttbInsp->iEnable==1）→ ttbInsp->DoCCDLightDown：CCD 燈（外部設備），S48 歸 Jimmy；'
     'ttbInsp 不在移植樹 → Top&Bottom 機台回報 todo',
     'if(' + _TB + ') filerw::ELTodo("golden fAOI.cpp:%d-%d ttbInsp->DoCCDLightDown after save not ported (TTopBottomInspect, Jimmy)");'
     % (_SV_LIGHT, _SV_LIGHT_E)),
    # ---- fAOI_ReadFile
    ('fAOI_ReadFile', _RD_EL, _RD_TB_E,
     'elParameter->ReadEditTextFromFile（Top&Bottom 才有 elParameter）＋ ttbInsp->SetCommParameter／UpdateAOIRecipeUI／sgICSmall／sgICLarge：'
     'TTopBottomInspect 不在移植樹（ttbInsp 恆無）→ 其餘機台與 golden 相同（elParameter NULL、ttbInsp NULL 都不跑）；'
     'Top&Bottom 機台開機印一行提醒：[TopBottomInspect] 等鍵沒有讀進任何結構',
     'if(' + _TB + ') std::printf("AOISetup: USE_Scanner_AOI_Inspection=Top&Bottom but TTopBottomInspect is not ported -- '
     'golden fAOI.cpp:%d-%d (elParameter->ReadEditTextFromFile AOI.Data, ttbInsp) skipped (Jimmy)\\n");' % (_RD_EL, _RD_TB_E)),
    ('fAOI_ReadFile', _RD_LAB, _RD_LAB_E,
     '純畫面（主畫面 fMain->labScanAOI「Scan AOI ON/OFF」字樣與字色；移植樹門面 forms/fMain.h 沒有這個元件、網頁也沒有對應 id）。'
     '判斷式裡的 FrmAOI->RunTopBottomInspect() 在移植樹恆 false（ttbInsp 不存在，golden :4782-4785 同樣回 false）',
     ';'),
    ('fAOI_ReadFile', _RD_KYEC, _RD_KYEC_E,
     'CC_KYEC_LEE 且 ttbInsp!=NULL 才跑（Eastsun 20260519 KYEC 鎖 rgActionMode、ttbInsp->iAction=1）：客戶專屬（S25）而且要 TTopBottomInspect —— '
     '移植樹 ttbInsp 恆無，golden 在同條件下也不跑',
     ';'),
    # ---- UpDataTrayData
    ('UpDataTrayData', _UT_X, _UT_Y,
     '純畫面：mtAOIBuffer（TTMyTray 盤面元件）的格數；下面兩行 MOT[MMScanAOI].Tray.X/YItem 是資料，照 golden 保留',
     ';'),
    ('UpDataTrayData', _UT_CELL, _UT_CELL,
     '純畫面：mtAOIBuffer 逐格上色（外層兩個 for 照 golden 留著，只剩空敘述）',
     ';'),
]

BLOCKS = [
    # AI(W906-FRW-S69) 20260926: 拿掉 —— cprod.h 已照 golden 補 bEnabledPositionByAOI（Steven 13:5x「補」）：('spbSaveClick', _SV_POS, _SV_POS, _POSWHY),
    # AI(W906-FRW-S69) 20260926: 拿掉 —— cprod.h 已照 golden 補 bEnabledPositionByAOI（Steven 13:5x「補」）：('fAOI_ReadFile', _RD_POS, _RD_POS, _POSWHY),
    # AI(W906-FRW-S69) 20260926: 拿掉 —— cprod.h 已照 golden 補 bEnabledPositionByAOI（Steven 13:5x「補」）：('fAOI_DoIniDataToForm', _DF_POS, _DF_POS, _POSWHY),
]

STRUCT = {
    'struct': 'AOISetup',
    'prefix': 'AO',
    'class': _F,
    'cpp': 'fAOI.cpp',
    'h': 'fAOI.h',
    'files': ['<DataPath>\\<配方>\\AOI.Data [SETTING]／[DutOnOff_BGAView]／[DutOnOff_PADView]／[RS232]／[AOITRAY]'
              '（讀 fAOI_ReadFile：只有 ReadIniData；寫 spbSaveClick：WriteIniData）'],
    'lists': [],
    'methods': METHODS,
    'save_methods': ['spbSaveClick'],
    'params': {'spbSaveClick': ''},
    'rettype': {'CheckFailBin': 'bool'},
    'members': [
        'int iQuotient;    // golden fAOI.h:428（private；建構子 :180 設 0 —— 本 TU static 零初值相同；golden 只有讀寫檔兩支用）',
        'int iRemainder;   // golden fAOI.h:429（同上，:181）',
    ],
    'replace': REPLACE,
    'blocks': BLOCKS,
    'includes': ['cmydef.h', 'cprod.h', 'Config.h', 'common.h', 'MachineType.h', 'Motor/mymotor.h', 'forms/fAOI.h', 'forms/fMain.h'],
    # cmydef.h：USE_AOI_Inspection／USE_Scanner_AOI_Inspection／USE_Top_Scanner_AOI_Inspection／AccessLevel／InitialOK／IndexSuckName／
    #   MMScanAOI／MManualTray2／MAX_ARM_Row／MAX_ARM_Col；cprod.h：tAOISetup／ScannerAOIIF／Prod（bIsPassBin）；Config.h：IniConfig（A02）；
    # common.h：DataPath／GetLastOpenFN／ReadIniData／WriteIniData；MachineType.h：eBtnAOI_*、CheckRange；Motor/mymotor.h：MOT[]（Tray.Data／XItem／YItem，
    #   mytray.h _MAX_COL_ITEM／_MAX_ROW_ITEM）；forms/fAOI.h：bVitroxBGA/PADViewUse/Map（golden fAOI.cpp:63-74 → 移植樹 forms/fAOI.cpp）；
    # forms/fMain.h：fMain->BackupSetupFile（移植樹目前是 offline no-op，forms/fMain.h:270）
    'decls': ['#include <cstdio>   // std::printf（fAOI_ReadFile 的 Top&Bottom 提醒）'],
    'overrides': [],
}

# AI(W906-E030-Q78) 20261003 [W906] (St01)：Steven 1003 05:3x Q78 裁決「Q78 Q79, 可以按照912，但是註解同時提供906的行號位置」，
#   加上 05:4x 慣例「如果是912比較好，就是註記906的行號跟做法　然後增加註記912已修正或更新的行號」⇒ A02 分支
#   （RogerYang 20260305 [A01_2]，906／V912 都有）裡 Close() 後面那句 `return;` 照 V912 留著，記成第 20 條的例外
#   （D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md Q78；human-review C24）。
#   下面這一列是「等價取代」：取代碼＝golden 原文 `return;`，產生的程式不變，只是讓產生檔那一行帶 906／V912 對照註解
#   （V912 原文留在 #if 0 // GATE 裡）。_e030q78_expect 釘住 V912 的 if／Close()／return 三行：V912 變了，產生器停下來。
#   906＝D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618 [AI(W906-E032) 20261003]（cp950，不在 git，唯讀）；行號＝grep -n／iconv 行號，
#   V912＝產生器的 golden 根目錄（tools/gen_editlist.py GOLDEN）。不改既有的列。
import os   # noqa: E402
_E030Q78_G = open(os.path.join(r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy', 'fAOI.cpp'), 'rb').read() \
    .decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')


def _e030q78_expect(gl, text):
    """golden V912 fAOI.cpp 第 gl 行（去掉 // 註解與頭尾空白）要等於 text，否則中止。"""
    s = _E030Q78_G[gl - 1].split('//')[0].strip()
    if s != text:
        raise SystemExit('AOISetup.py (E030-Q78): golden V912 fAOI.cpp:%d is %r, expected %r' % (gl, s, text))
    return gl


_e030q78_expect(3122, 'if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&')
_e030q78_expect(3126, 'Close();')
_E030Q78_WHY = ('AI(W906-E030-Q78) 原文照留（取代碼＝golden 原文 return;，產生的程式不變，只加這段註解）。'
                '906 fAOI.cpp:2950-2955：A02 分支只有 Close()（:2954），後面沒有 return，存檔照跑；主選單 main.cpp:28585 用 Show 開（非對話框），fAOI.dfm 沒綁 OnClose ⇒ Close() 只把表單藏起來，操作員改的值照樣寫進檔。'
                'V912 fAOI.cpp:3126-3127：Close() 後多一句 `return;` ⇒ 頁面關掉、不寫檔（A02 分支本身兩版都有，RogerYang 20260305 [A01_2]）。'
                'Kept: #20 exception (Steven 1003 Q78, keep V912)')
STRUCT.setdefault('replace', []).append(
    ('spbSaveClick', _e030q78_expect(3127, 'return;'), 3127, _E030Q78_WHY, 'return;'))
