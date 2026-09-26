# -*- coding: utf-8 -*-
# tools/editlist/TrayForm.py -- gen_editlist.py 的結構設定（一個 C 形狀結構一個檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only TrayForm
#
# Steven 團隊 20260925：TrayForm（<recipe>\Tray.Data 的 Tray Assignment 那一半）—— golden TfTrayAssignment
# （HT9011UC_Code_V3.33.912.0_20260908_Jimmy cTrayAssignment.cpp，1817 行）。頁面 web/page/Setup.TrayAssignment.html。
# 沒有 HTEditList：golden 存檔鈕 spbSaveClick（:1337）→ SaveSetupFile（:1409）自己逐鍵 WriteIniData，
# 存完 ReadFile（:1629）。同 ArmSpeed_File 的做法：替身＋直接跑 golden 存檔鈕。
# 前一版 A 形狀已退役（tools/formbridge/_retired/TfTrayAssignment.py）；C 形狀直接轉 golden 912 的 ReadFile，
# 那一版回報的「移植樹讀檔器落後 912（AMR Loader Re-Test、Top&Bottom AOI）」在這條路上不存在
# （開頁／存後重讀跑的就是 golden 912 的 ReadFile＋FixCanUse）。
#
# 不 adopt 移植樹 fTrayAssignment（forms/fTrayAssignment.h）的真元件，理由見 FileRW/TrayForm.cpp 檔頭。
#
# TScrollBar（sbNormalTest／sbNormalTest_RT，圖像模式 IniConfig.bTrayAssignUseGraphic）：產生器把 TScrollBar
# 退成 TControl（沒有 Position），所以用到它們的 golden 行一律 replace 成 filerw::ELTrackBar 替身
# （VCL TScrollBar 的 Position 也是夾在 Min..Max、值有變才觸發 OnChange —— 同 ELTrackBar 語意）。
# 替身由 FileRW/TrayForm.cpp 開機最先以 ELTrackBar 型別建立（DFM Max=15、OnChange＝sbNormalTest*Change）。
# 同一行上的 TRadioGroup 設 ItemIndex 用 TA_SetRadioIndex（VCL TCustomRadioGroup.SetItemIndex 的夾值：
# golden GraphicToRadio 的 Position&0x02／&0x04／&0x08 會給 2／4／8，VCL 夾成 Count-1；vclcompat 不夾）。
_F = 'TfTrayAssignment'


def _tb(name):
    return 'EL<filerw::ELTrackBar>("%s", "%s")' % (_F, name)


def _rg(name):
    return 'EL<TRadioGroup>("%s", "%s")' % (_F, name)


def _g2r(dst, src, mask):
    # golden: dst->ItemIndex=src->Position&mask;
    return 'TA_SetRadioIndex(%s, %s->Position&0x%02X);' % (_rg(dst), _tb(src), mask)


_GRAPH = '圖像模式（bTrayAssignUseGraphic）：TScrollBar 在產生器是 TControl，改 ELTrackBar 替身；RadioGroup 照 VCL 夾值'
_PIC = '圖片檔名（純畫面，HTML 端處理）'


STRUCT = {
    'struct': 'TrayForm',
    'prefix': 'TA',
    'class': 'TfTrayAssignment',
    'cpp': 'cTrayAssignment.cpp',
    'h': 'cTrayAssignment.h',
    'files': ['Tray.Data'],
    'lists': [],
    'methods': ['TfTrayAssignment', 'FixCanUse', 'ReadFile', 'DoIniDataToForm', 'FormShow', 'ShowCompnet',
                'ShowTrayDirectIMG', 'GraphicToRadio', 'RadioToGraphic', 'GraphicToRadio_RT', 'RadioToGraphic_RT',
                'sbNormalTestChange', 'sbNormalTest_RTChange', 'FormClose', 'spbSaveClick', 'SaveSetupFile'],
    # GraphicToRadio／_RT 在存檔鈕 :1379-1380 跑（圖像模式時 RadioGroup 由 Position 決定）→ 它們讀的 Position 也是存檔輸入
    'save_methods': ['spbSaveClick', 'SaveSetupFile', 'GraphicToRadio', 'GraphicToRadio_RT'],
    'params': {'TfTrayAssignment': '', 'FormShow': '', 'FormClose': '', 'spbSaveClick': '',
               'sbNormalTestChange': '', 'sbNormalTest_RTChange': ''},
    'members': ['int iTrayDirect[eTrayCount];            // golden cTrayAssignment.h:285（private）方向圖索引 0=Loader 1..6=Auto 7..12=Fix',
                'bool fShow=false;                       // golden cTrayAssignment.h:302',
                'TCheckBox *chkICSort[MAX_AUTO_TRAY];    // golden cTrayAssignment.h:303（建構子 :47-48 填）',
                'TEdit *edtICSort[MAX_AUTO_TRAY][6];     // golden cTrayAssignment.h:304（建構子 :49-61 填）',
                'AnsiString Caption;                     // golden TForm::Caption（FormShow :750，HTML 不用）',
                'int Left=0;                             // golden TForm::Left（FormShow :756）',
                'int Top=0;                              // golden TForm::Top（FormShow :757）',
                'int Width=0;                            // golden TForm::Width（ShowCompnet :1089-1147）',
                'bool TA_rgLoaderTrayModeItems=true;     // FileRW：rgLoaderTrayMode->Controls[0..2]->Enabled（golden ReadFile :341-376，三顆一起設）'],
    'replace': [
        # ---- ReadFile ----
        ('ReadFile', 341, 343, 'rgLoaderTrayMode->Controls[0..2]->Enabled=false：vclcompat TRadioGroup 沒有 Controls[]；golden 三顆一起設、DFM 只有三項 ⇒ 等同「這組不能改」，記旗標，FormShow 結尾併進 Enabled',
         'TA_rgLoaderTrayModeItems=false;'),
        ('ReadFile', 350, 352, '同 :341-343（KLT Auto SKIP）', 'TA_rgLoaderTrayModeItems=false;'),
        ('ReadFile', 358, 360, '同 :341-343（KLT 非 Auto SKIP：三顆打開）', 'TA_rgLoaderTrayModeItems=true;'),
        ('ReadFile', 374, 376, '同 :341-343（CC_KYEC_LEE）', 'TA_rgLoaderTrayModeItems=false;'),
        # ---- FormShow ----
        ('FormShow', 745, 745, 'LoadImage()：方向示意圖（HTML 自己有）', ';'),
        ('FormShow', 966, 966, 'golden 每顆 RadioButton 的 Enabled（ReadFile 記的旗標）與群組 Enabled 取 AND ＝ 頁面上這組可不可以改',
         '{ if(!TA_rgLoaderTrayModeItems) EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Enabled=false; } fShow=true;'),
        # ---- ShowTrayDirectIMG：只留 iTrayDirect 夾值（<0 或 >8 → 0，存檔寫的就是它）----
        ('ShowTrayDirectIMG', 1212, 1213, 'TImage *MyImage[]＝13 張方向圖（TImage 在 vclcompat 沒有；圖片 HTML 端處理）',
         ';   // golden MyImage[]={imgLoader, imgAuto1..6, imgFix1..6}'),
        ('ShowTrayDirectIMG', 1216, 1216, 'sizeof(MyImage)/4 ＝ 13（BCB6 32 位元指標）', 'for(int i=0; i<13; i++)'),
        # ---- 圖像模式 ----
        ('GraphicToRadio', 1647, 1648, _PIC + '。自己換成空敘述：產生器的 UI-only 改寫會把 ImgNormalTest 留在同一行註解裡，'
         '被存檔讀替身的掃描當成「讀」→ mustSend 出現沒有值種類的 TImage，頁面永遠存不了', ';'),
        ('GraphicToRadio', 1650, 1653, _GRAPH,
         _g2r('RGAuto3', 'sbNormalTest', 1) + ' ' + _g2r('RGAuto2', 'sbNormalTest', 2) + ' ' +
         _g2r('RGAuto1', 'sbNormalTest', 4) + ' ' + _g2r('RGLoader', 'sbNormalTest', 8)),
        ('RadioToGraphic', 1668, 1668, _GRAPH, _tb('sbNormalTest') + '->Position=j;'),
        ('RadioToGraphic', 1670, 1670, _PIC, ';'),
        ('GraphicToRadio_RT', 1682, 1683, _PIC + '（同 :1647-1648，ImgReTest）', ';'),
        ('GraphicToRadio_RT', 1687, 1687, 'pgRunMode->ActivePage==tsNormalTestGraph：vclcompat TPageControl 只有 ActivePageIndex（tsNormalTestGraph＝第 2 頁，DFM 順序）',
         'if(TA_RunModePageIndex()==2)'),
        ('GraphicToRadio_RT', 1689, 1689, _GRAPH, _tb('sbNormalTest_RT') + '->Position=' + _tb('sbNormalTest') + '->Position;'),
        ('GraphicToRadio_RT', 1693, 1696, _GRAPH,
         _g2r('rgAuto3_RT', 'sbNormalTest_RT', 1) + ' ' + _g2r('rgAuto2_RT', 'sbNormalTest_RT', 2) + ' ' +
         _g2r('rgAuto1_RT', 'sbNormalTest_RT', 4) + ' ' + _g2r('rgLoad_RT', 'sbNormalTest_RT', 8)),
        ('RadioToGraphic_RT', 1711, 1711, _GRAPH, _tb('sbNormalTest_RT') + '->Position=j;'),
        ('RadioToGraphic_RT', 1713, 1713, _PIC, ';'),
        # ---- spbSaveClick ----
        ('spbSaveClick', 1343, 1343, 'Close()：golden 權限不足時關表單 → 伺服器端記 closed（ack.trace），頁面自己關',
         'filerw::ELMark("closed");'),
        ('spbSaveClick', 1355, 1356, 'pgRunMode->ActivePage==tsNormalTestGroup||tsNormalTestGraph：頁面不送目前分頁，伺服器保留 DFM 的 tsReTestGroup（第 1 頁）⇒ 不複製 FT→RT',
         'filerw::ELTodo("golden cTrayAssignment.cpp:1355 bFTBin2RTBin copies FT->RT only when an FT tab is active; the page does not send the pgRunMode tab -- server keeps the DFM tab tsReTestGroup => no copy, RT saved as sent"); '
         'if(TA_RunModePageIndex()==0 || TA_RunModePageIndex()==2)'),
        ('spbSaveClick', 1361, 1361, _GRAPH, _tb('sbNormalTest_RT') + '->Position=' + _tb('sbNormalTest') + '->Position;'),
    ],
    # Steven 20260925（Steven：「P46 是特殊功能，可以接受 by config 或是 by 工作檔，需要根據 BCB 的流程做決定」）：
    # 照 golden cTrayAssignment.cpp:1419-1426——bP46 開時這一鍵寫 config.ini [Flag] Skip Manual Remove Tray（:1421），
    # 關時寫 Tray.Data（:1425），都由 TfTrayAssignment 的存檔寫。原本這裡擋掉 :1421（「config.ini 歸 IniConfig」），
    # 結果兩頁都存不到。擁有者閘擋的是 B 路整檔鏡像；兩個 golden 存檔流程各寫 config.ini 的不同鍵是 golden 本來的設計
    # （IniConfig 的 SaveEditTextToFile 逐鍵寫，不會蓋掉這一鍵），所以照 golden 放行。
    'blocks': [],
    'includes': ['cprod.h', 'Config.h', 'LastSet.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'forms/fMain.h', 'forms/fSecurity.h', 'forms/fLotInfo.h',
                 'forms/fShowBinSelect.h', 'csystem.h', 'Motor/mymotor.h',
                 'SECSGEM/SecsEventType.h', 'SECSGEM/SecsEventReport.h'],
    # cAuthority.h 帶進 language.h（TWinControl 與 Public/HTEdit.h 重複）→ 只前置宣告要用的（同 IniConfig）
    'decls': ['extern bool bAuthCriticalPara[26];   // cAuthority.h:64（golden cAuthority.h）',
              'static int  TA_RunModePageIndex();                   // FileRW/TrayForm.cpp：pgRunMode->ActivePageIndex（不算存檔讀的替身）',
              'static void TA_SetRadioIndex(TRadioGroup* g, int v);  // FileRW/TrayForm.cpp：VCL TCustomRadioGroup.SetItemIndex 夾值',
              'extern void MyDBIProcess(AnsiString S1, AnsiString S2);   // aHotPlateSubstrate.h:979（本體 aHotPlateSubstrate.cpp:1244；aHotPlateSubstrate.h 與 HTEditList.h 衝突）'],
    'overrides': [],
}

# SaveSetupFile 的 PtrCombBox[RGAutoN->ItemIndex]：golden 下標沒檢查（VCL 的 RadioGroup 會把 ItemIndex 夾在 -1..Count-1，
# 但 -1 仍會越界）。vclcompat 不夾，頁面沒選時送 -1 ⇒ 伺服器端讀野指標。越界時不寫這一鍵、ELTodo（其餘照 golden）。
_PTR = {1544: ('eAuto1', 'RGAuto1'), 1562: ('eAuto3', 'RGAuto3'), 1580: ('eAuto4', 'RGAuto4'),
        1587: ('eAuto5', 'RGAuto5'), 1594: ('eAuto6', 'RGAuto6')}
for _ln, (_t, _w) in sorted(_PTR.items()):
    STRUCT['replace'].append(
        ('SaveSetupFile', _ln, _ln, 'PtrCombBox[%s->ItemIndex] 下標越界時 golden 是未定義行為 → 不寫這一鍵、ELTodo' % _w,
         '{ const int k_=%s->ItemIndex; if(k_<0 || k_>1) filerw::ELTodo("golden cTrayAssignment.cpp:%d PtrCombBox[%s->ItemIndex] out of range (golden UB) -- key not written"); '
         'else WriteIniData(szDir, s6TrayName[%s], "Tray Type", PtrCombBox[k_]->ItemIndex); }' % (_rg(_w), _ln, _w, _t)))
STRUCT['replace'].append(
    ('SaveSetupFile', 1555, 1555, 'PtrCombBox[ibuffer] 下標越界時 golden 是未定義行為 → 不寫這一鍵、ELTodo',
     'if(ibuffer<0 || ibuffer>1) filerw::ELTodo("golden cTrayAssignment.cpp:1555 PtrCombBox[ibuffer] out of range (golden UB) -- key not written"); '
     'else WriteIniData(szDir, s6TrayName[eAuto2], "Tray Type", PtrCombBox[ibuffer]->ItemIndex);'))

# ShowCompnet :1036-1041／:1045-1050 `?cbEmpty->Text:cbColor->Text`：產生器的元件樣式排除前一個字元是「:」的
# （為了不動 `X::`），三元運算子的 `:cbColor` 因此沒改成替身（編譯擋下）→ 逐行等價取代（只是把 cbColor 換成替身）
for _i, _ln in enumerate(range(1036, 1042)):
    STRUCT['replace'].append(
        ('ShowCompnet', _ln, _ln, '三元運算子 :cbColor 產生器沒改寫（前一字元是「:」）—— 等價取代',
         'EL<TEdit>("%s", "edAuto%dType")->Text=(%s->ItemIndex==0)?EL<TComboBox>("%s", "cbEmpty")->Text:EL<TComboBox>("%s", "cbColor")->Text;'
         % (_F, _i + 1, _rg('RGAuto%d' % (_i + 1)), _F, _F)))
for _i, _ln in enumerate(range(1045, 1051)):
    STRUCT['replace'].append(
        ('ShowCompnet', _ln, _ln, '三元運算子 :cbColor 產生器沒改寫（前一字元是「:」）—— 等價取代',
         'EL<TEdit>("%s", "edAuto%dType")->Text=(%s->ItemIndex==0)?EL<TComboBox>("%s", "cbEmpty")->Text:EL<TComboBox>("%s", "cbColor")->Text;'
         % (_F, _i + 1, _rg('rgAuto%d_RT' % (_i + 1)), _F, _F)))
