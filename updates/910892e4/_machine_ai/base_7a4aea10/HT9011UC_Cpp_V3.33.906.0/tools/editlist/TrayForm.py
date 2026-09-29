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

# AI(W906-FRW-S158) 20260927 [W906]：WS form.event（Steven ★ Q40＝A，RULINGS_20260926 S157；Q41 盤點 TA-1～TA-4，
#   docs/Q41_INVENTORY_20260927.md §3.4）用到的 golden 行一律用 _expect 釘住原文（golden 那一行變了產生器就停，不會蓋錯行）。
_GOLDEN_CPP = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp'
_GL = open(_GOLDEN_CPP, 'rb').read().decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')


def _expect(ln, text):
    if text not in _GL[ln - 1]:
        raise SystemExit('TrayForm.py: golden cTrayAssignment.cpp:%d 不再是 %r（實際 %r）' % (ln, text, _GL[ln - 1].strip()))
    return ln


# TA-1 方向圖：golden 點 TImage → iTrayDirect[Ptr->Tag] 循環 0..7、Picture 換成 type<dir>.bmp（:971-991）。
#   替身的 Tag 在這一頁的意思改成「目前載入的圖號」（＝type<n>.bmp 的 n；頁面引擎 web/page/ht9045_wire_engine.js:1166-1175
#   對 TImage 替身的慣例：tag＝圖號、顯示 img/dfm_type<tag>.png）。golden 拿 DFM Tag 當 iTrayDirect[] 的索引
#   （imgLoader 0、imgAuto1..6 1..6、imgFix1..6 7..12；golden 執行期從不改圖片的 Tag）→ 改由 TA_ImgIndex() 查同一張表
#   （FileRW/TrayForm.cpp kTA_ImgName）。不這樣做的話，產生器替 form.event 控制項帶入的 DFM Tag 1..12 會被 editlist.get 當成
#   圖號送出（PutProxyValue：Tag 非 0 就送），引擎顯示 dfm_type12.png、點一下在本地把 Tag 改掉、存檔送回 → 索引被頁面改寫。
_IMG = 'TA-1：替身 Tag＝目前載入的圖號（golden Picture＝type<n>.bmp），iTrayDirect[] 的索引＝golden DFM Tag 由 TA_ImgIndex() 查表（見設定檔 _IMG 註解）'
# VCL TCustomRadioGroup.SetItemIndex：程式設 ItemIndex（夾在 -1..Count-1）值有變 → TGroupButton.Checked → Click → OnClick
#   （同 tools/editlist/TestIF_File_Cleaning.py 的 (2) CL_RadioIndex、HSys.py:116）。vclcompat 替身不觸發 → TA_RadioIndex
#   （FileRW/TrayForm.cpp，DFM OnClick 對照表）。只用在本波新轉的處理器裡；既有方法（DoIniDataToForm :570-626 等）照舊不觸發，見交件。
_VCLRG = 'VCL 程式設 TRadioGroup::ItemIndex（值有變）會觸發 DFM OnClick；vclcompat 替身不觸發 → TA_RadioIndex（FileRW/TrayForm.cpp）'
_EV_REPLACE = [
    # ---- TA-1 imgLoaderClick（:971）----
    ('imgLoaderClick', _expect(973, 'TImage *Ptr;'), 973, 'TImage 在 vclcompat 沒有 → 替身是 TControl（產生器 KNOWN 以外的型別退回 TControl）',
     'TControl *Ptr;'),
    ('imgLoaderClick', _expect(975, 'Ptr=(TImage *)Sender;'), 975, '同 :973', 'Ptr=(TControl *)Sender;'),
    ('imgLoaderClick', _expect(976, 'int dir=iTrayDirect[Ptr->Tag];'), 976, _IMG + '；不是這 13 張圖的替身（不會發生：事件表只列這 13 個）→ 記 todo 不動',
     'const int iTag_=TA_ImgIndex(Ptr); if(iTag_<0) { filerw::ELTodo("golden cTrayAssignment.cpp:976 imgLoaderClick Sender is not one of the 13 direction images -- nothing changed"); return; } int dir=iTrayDirect[iTag_];'),
    ('imgLoaderClick', _expect(980, 'iTrayDirect[Ptr->Tag]=dir;'), 980, _IMG, 'iTrayDirect[iTag_]=dir;'),
    ('imgLoaderClick', _expect(981, 'sprintf(Dir, "%stype%d.bmp", BmpPath, dir);'), 981,
     'C 的 sprintf 可變參數不能收 vclcompat AnsiString（BCB6 的 AnsiString 恰好只有一個 char* 成員，才過得去）→ .c_str()，同一個字串',
     'sprintf(Dir, "%stype%d.bmp", BmpPath.c_str(), dir);'),
    ('imgLoaderClick', _expect(984, 'Ptr->Picture->LoadFromFile(Dir);'), 984, _IMG + '：換圖 ＝ 替身 Tag 換成新圖號（form.event 的 changed 就帶這個 tag）',
     'Ptr->Tag=dir;'),
    # ---- TA-1 ShowTrayDirectIMG 的換圖（:1223；:1212-1213 見 replace 主表）----
    ('ShowTrayDirectIMG', _expect(1223, 'MyImage[i]->Picture->LoadFromFile(Dir);'), 1223, _IMG + '：開頁把檔案的方向顯示出來',
     'MyImage[i]->Tag=iTrayDirect[i];'),
    # ---- TA-2 rgLoaderTypeClick（:1153）----
    ('rgLoaderTypeClick', _expect(1158, 'rgLoaderType->ItemIndex=TrayForm.LodareType;'), _expect(1159, 'return;'),
     'KYEC 條碼登入 Barcode_Reader(bcTrayAssign)（只有 CC_KYEC_LEE／CC_KYEC_XILINX 且 USE_BARCODE_AS_KEYBOARD 時問操作員 ID，其他機台回 2）回 0 → '
     'golden 改回 TrayForm.LodareType（' + _VCLRG + '：golden 這裡會再進一次 rgLoaderTypeClick，值相等 → cbLoaderChange＋ShowCompnet）；'
     '另記 ELTodo 讓頁面看得到為什麼被改回（輸入框是離線空殼，同 tools/editlist/UserDefForm_File.py cbTrayType1Change）',
     'TA_RadioIndex(EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType"), TrayForm.LodareType); '
     'filerw::ELTodo("golden cTrayAssignment.cpp:1156 Barcode_Reader(bcTrayAssign) returned 0 (KYEC operator-ID prompt; the port input box is an offline shell) -- golden puts rgLoaderType back"); return;'),
    ('rgLoaderTypeClick', _expect(1162, 'cbLoaderChange(this);'), 1162, '事件以 (this) 呼叫（參數已拿掉）', 'TA_cbLoaderChange();'),
    # ---- TA-3 rgFixTrayModeClick（:1166）：Fix 盤有 IC 不准切（:1182-1194 照 golden）----
    ('rgFixTrayModeClick', _expect(1193, 'rgFixTrayMode->ItemIndex=TrayForm.iFixTrayMode;'), 1193,
     _VCLRG + '（golden 改回原值會再進一次 rgFixTrayModeClick，用原值重算 ckUseFix1..6 的 Visible）',
     'TA_RadioIndex(EL<TRadioGroup>("TfTrayAssignment", "rgFixTrayMode"), TrayForm.iFixTrayMode);'),
    # ---- TA-2 RGLoaderClick（:1233）／rgLoad_RTClick（:1275）的圖像模式捲軸 ----
    ('RGLoaderClick', _expect(1258, 'sbNormalTest->Position=0;'), 1258, _GRAPH, _tb('sbNormalTest') + '->Position=0;'),
    ('RGLoaderClick', _expect(1262, 'sbNormalTest->Position=15;'), 1262, _GRAPH, _tb('sbNormalTest') + '->Position=15;'),
    ('rgLoad_RTClick', _expect(1300, 'sbNormalTest_RT->Position=0;'), 1300, _GRAPH, _tb('sbNormalTest_RT') + '->Position=0;'),
    ('rgLoad_RTClick', _expect(1304, 'sbNormalTest_RT->Position=15;'), 1304, _GRAPH, _tb('sbNormalTest_RT') + '->Position=15;'),
    # ---- TA-2 cbLoaderChange（:1317）----
    ('cbLoaderChange', _expect(1321, 'rgLoad_RTClick(this);'), 1321, '事件以 (this) 呼叫（參數已拿掉）', 'TA_rgLoad_RTClick();'),
    ('cbLoaderChange', _expect(1322, 'RGLoaderClick(this);'), 1322, '事件以 (this) 呼叫（參數已拿掉）', 'TA_RGLoaderClick();'),
    # ---- TA-4 BinTrayDetect（:1719）的改回（:1756-1762）：VCL 會觸發被改那一組的 OnClick ----
    ('BinTrayDetect', _expect(1756, 'RGLoader->ItemIndex=0;'), 1756, _VCLRG + '（RGLoader OnClick＝RGLoaderClick，dfm:407）',
     'TA_RadioIndex(%s, 0);' % _rg('RGLoader')),
    ('BinTrayDetect', _expect(1758, 'RGAuto2->ItemIndex=0;'), 1758, _VCLRG + '（RGAuto2 OnClick＝RGAuto2Click，dfm:316）',
     'TA_RadioIndex(%s, 0);' % _rg('RGAuto2')),
    ('BinTrayDetect', _expect(1760, 'rgLoad_RT->ItemIndex=0;'), 1760, _VCLRG + '（rgLoad_RT OnClick＝rgLoad_RTClick，dfm:555）',
     'TA_RadioIndex(%s, 0);' % _rg('rgLoad_RT')),
    ('BinTrayDetect', _expect(1762, 'rgAuto2_RT->ItemIndex=0;'), 1762, _VCLRG + '（rgAuto2_RT OnClick＝cbEmptyChange，dfm:464）',
     'TA_RadioIndex(%s, 0);' % _rg('rgAuto2_RT')),
]


STRUCT = {
    'struct': 'TrayForm',
    'prefix': 'TA',
    'class': 'TfTrayAssignment',
    'cpp': 'cTrayAssignment.cpp',
    'h': 'cTrayAssignment.h',
    'files': ['Tray.Data'],
    'lists': [],
    # AI(W906-FRW-S158) 20260927 [W906]：＋第二行 9 個 —— WS form.event 的 golden 處理器（TA-1～TA-4，見下面 'events'）
    #   與它們呼叫的 BinTrayDetect（:1719）。cbLoaderDropDown（:1781，cbLoader OnDropDown，KYEC 條碼、回 0 也只是 return）、
    #   cbEnableAMR_KYECClick（:1805，KYEC AMR）、sbNormalTestChange／_RT 的頁面觸發（TA-5）、sbtExitClick 不在本波。
    'methods': ['TfTrayAssignment', 'FixCanUse', 'ReadFile', 'DoIniDataToForm', 'FormShow', 'ShowCompnet',
                'ShowTrayDirectIMG', 'GraphicToRadio', 'RadioToGraphic', 'GraphicToRadio_RT', 'RadioToGraphic_RT',
                'sbNormalTestChange', 'sbNormalTest_RTChange', 'FormClose', 'spbSaveClick', 'SaveSetupFile',
                'imgLoaderClick', 'rgLoaderTypeClick', 'rgFixTrayModeClick', 'cbEmptyChange', 'RGLoaderClick',
                'rgLoad_RTClick', 'cbLoaderChange', 'RGAuto2Click', 'BinTrayDetect'],
    # GraphicToRadio／_RT 在存檔鈕 :1379-1380 跑（圖像模式時 RadioGroup 由 Position 決定）→ 它們讀的 Position 也是存檔輸入
    'save_methods': ['spbSaveClick', 'SaveSetupFile', 'GraphicToRadio', 'GraphicToRadio_RT'],
    # AI(W906-FRW-S158)：imgLoaderClick 保留 golden 的 TObject *Sender（它讀 Sender 決定是哪一張圖）；其餘處理器不讀 Sender → ''
    'params': {'TfTrayAssignment': '', 'FormShow': '', 'FormClose': '', 'spbSaveClick': '',
               'sbNormalTestChange': '', 'sbNormalTest_RTChange': '',
               'rgLoaderTypeClick': '', 'rgFixTrayModeClick': '', 'cbEmptyChange': '', 'RGLoaderClick': '',
               'rgLoad_RTClick': '', 'cbLoaderChange': '', 'RGAuto2Click': ''},
    'rettype': {'BinTrayDetect': 'int'},
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
        # AI(W906-FRW-S158) 20260927 [W906]：⛔ 更正 —— 原本換成空敘述（「HTML 自己有」）。13 張方向圖現在是替身（Tag＝圖號，TA-1），
        #   golden LoadImage（:723-739）＝全部載入 type0.bmp ⇒ 13 個替身 Tag＝0（:751 ShowTrayDirectIMG 再換成檔案的方向；頁面看到的結果不變）
        ('FormShow', 745, 745, 'LoadImage()：13 張方向圖全部載入 type0.bmp（:723-739）＝ ' + _IMG,
         'for(int k_=0; k_<13; k_++) TA_Img(k_)->Tag=0;'),
        ('FormShow', 966, 966, 'golden 每顆 RadioButton 的 Enabled（ReadFile 記的旗標）與群組 Enabled 取 AND ＝ 頁面上這組可不可以改',
         '{ if(!TA_rgLoaderTrayModeItems) EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Enabled=false; } fShow=true;'),
        # ---- ShowTrayDirectIMG：只留 iTrayDirect 夾值（<0 或 >8 → 0，存檔寫的就是它）----
        # AI(W906-FRW-S158) 20260927 [W906]：⛔ 更正 —— 原本換成空敘述（「圖片 HTML 端處理」）；現在 13 張圖是替身（TA-1，_IMG），:1223 換圖寫替身 Tag
        ('ShowTrayDirectIMG', 1212, 1213, 'TImage *MyImage[]＝13 張方向圖（TImage 在 vclcompat 沒有 → TControl 替身，順序同 golden：imgLoader, imgAuto1..6, imgFix1..6）',
         'TControl *MyImage[13]; for(int k_=0; k_<13; k_++) MyImage[k_]=TA_Img(k_);'),
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
                 'SECSGEM/SecsEventType.h', 'SECSGEM/SecsEventReport.h',
                 'BarcodeReader.h'],   # AI(W906-FRW-S158) 20260927 [W906]：rgLoaderTypeClick :1156 Barcode_Reader（同 UserDefForm_File）
    # cAuthority.h 帶進 language.h（TWinControl 與 Public/HTEdit.h 重複）→ 只前置宣告要用的（同 IniConfig）
    'decls': ['extern bool bAuthCriticalPara[26];   // cAuthority.h:64（golden cAuthority.h）',
              'static int  TA_RunModePageIndex();                   // FileRW/TrayForm.cpp：pgRunMode->ActivePageIndex（不算存檔讀的替身）',
              'static void TA_SetRadioIndex(TRadioGroup* g, int v);  // FileRW/TrayForm.cpp：VCL TCustomRadioGroup.SetItemIndex 夾值',
              'extern void MyDBIProcess(AnsiString S1, AnsiString S2);   // aHotPlateSubstrate.h:979（本體 aHotPlateSubstrate.cpp:1244；aHotPlateSubstrate.h 與 HTEditList.h 衝突）',
              # AI(W906-FRW-S158) 20260927 [W906]：WS form.event（TA-1～TA-4）
              'static TControl* TA_Img(int i);                      // FileRW/TrayForm.cpp：13 張方向圖替身（golden MyImage[] 順序 imgLoader, imgAuto1..6, imgFix1..6）',
              'static int  TA_ImgIndex(TObject* Sender);            // FileRW/TrayForm.cpp：golden DFM Tag（iTrayDirect[] 索引）；不是這 13 張圖回 -1',
              'static void TA_RadioIndex(TRadioGroup* g, int v);    // FileRW/TrayForm.cpp：VCL SetItemIndex（夾值、值有變 → DFM OnClick）'],
    'overrides': [],
    # AI(W906-FRW-S158) 20260927 [W906]：WS form.event 事件表（Steven ★ Q40＝A，RULINGS_20260926 S157；格式 FROM_STEVEN 20260927 10:15；
    #   Q41 盤點 TA-1～TA-4）。控制項↔處理器照 golden cTrayAssignment.dfm 的 OnClick／OnChange（行號見各列），產生器在
    #   .gen.inc 檔尾產生 kTA_Events，FileRW/TrayForm.cpp 用 filerw::PageEventsRegistrar 註冊。golden 怪處照翻：
    #   * RGAuto5 的 OnClick 是 RGAuto2Click（dfm:352）——點 Auto5 跑的是「Auto2 設成 bin 盤時停用 Auto3」那段；
    #   * RGAuto1／3／4／6（FT 頁）與 cbEmpty／cbColor 的處理器 cbEmptyChange 看的是 RT 頁的 rgAuto2_RT、停用的是 rgAuto3_RT；
    #   * BinTrayDetect（:1753-1762）改回的是「第一個」值為 2 的群組（RGLoader > RGAuto2 > rgLoad_RT > rgAuto2_RT），不一定是剛點的那組。
    'events': [('imgLoader', 'click', 'imgLoaderClick'),                      # dfm:230（DFM Tag 0）
               ('imgAuto1', 'click', 'imgLoaderClick'),                       # dfm:135（Tag 1）
               ('imgAuto2', 'click', 'imgLoaderClick'),                       # dfm:95（Tag 2）
               ('imgAuto3', 'click', 'imgLoaderClick'),                       # dfm:55（Tag 3）
               ('imgAuto4', 'click', 'imgLoaderClick'),                       # dfm:1167（Tag 4）
               ('imgAuto5', 'click', 'imgLoaderClick'),                       # dfm:1127（Tag 5）
               ('imgAuto6', 'click', 'imgLoaderClick'),                       # dfm:1087（Tag 6）
               ('imgFix1', 'click', 'imgLoaderClick'),                        # dfm:654（Tag 7）
               ('imgFix2', 'click', 'imgLoaderClick'),                        # dfm:715（Tag 8）
               ('imgFix3', 'click', 'imgLoaderClick'),                        # dfm:777（Tag 9）
               ('imgFix4', 'click', 'imgLoaderClick'),                        # dfm:901（Tag 10）
               ('imgFix5', 'click', 'imgLoaderClick'),                        # dfm:962（Tag 11）
               ('imgFix6', 'click', 'imgLoaderClick'),                        # dfm:1024（Tag 12）
               ('rgLoaderType', 'click', 'rgLoaderTypeClick'),                # dfm:632（TA-2）
               ('cbLoader', 'change', 'cbLoaderChange'),                      # dfm:240（TA-2）
               ('RGLoader', 'click', 'RGLoaderClick'),                        # dfm:407（TA-2）
               ('rgLoad_RT', 'click', 'rgLoad_RTClick'),                      # dfm:555（TA-2）
               ('rgFixTrayMode', 'click', 'rgFixTrayModeClick'),              # dfm:830（TA-3）
               ('cbColor', 'change', 'cbEmptyChange'),                        # dfm:176（TA-4）
               ('cbEmpty', 'change', 'cbEmptyChange'),                        # dfm:204（TA-4）
               ('RGAuto1', 'click', 'cbEmptyChange'),                         # dfm:304
               ('RGAuto2', 'click', 'RGAuto2Click'),                          # dfm:316
               ('RGAuto3', 'click', 'cbEmptyChange'),                         # dfm:328
               ('RGAuto4', 'click', 'cbEmptyChange'),                         # dfm:340
               ('RGAuto5', 'click', 'RGAuto2Click'),                          # dfm:352（golden 怪處，見上）
               ('RGAuto6', 'click', 'cbEmptyChange'),                         # dfm:364
               ('rgAuto1_RT', 'click', 'cbEmptyChange'),                      # dfm:452
               ('rgAuto2_RT', 'click', 'cbEmptyChange'),                      # dfm:464
               ('rgAuto3_RT', 'click', 'cbEmptyChange'),                      # dfm:476
               ('rgAuto4_RT', 'click', 'cbEmptyChange'),                      # dfm:488
               ('rgAuto5_RT', 'click', 'cbEmptyChange'),                      # dfm:500
               ('rgAuto6_RT', 'click', 'cbEmptyChange')],                     # dfm:512
}
STRUCT['replace'] += _EV_REPLACE   # AI(W906-FRW-S158) 20260927 [W906]：form.event 處理器用到的等價取代（見檔頭 _EV_REPLACE）

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
