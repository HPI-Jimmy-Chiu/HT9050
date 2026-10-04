# -*- coding: utf-8 -*-
# tools/editlist/TrayForm.py -- gen_editlist.py 的結構設定（一個 C 形狀結構一個檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only TrayForm
#
# //AI(W906-E031) 20261003 [W906] (St01)：E-031 全面切換第 1 批 —— golden 換成 906 0618（STRUCT['golden']＝'906'，tools/golden_root.py；
#   Jimmy RULINGS_20261003 第 2、4 條；舊側分支 725cde57 試行的重做）。本檔所有 cTrayAssignment.cpp／.dfm 行號都是 0618（裸的 :N＝0618，
#   括號「V912 :M」才是 V912）；由 `python tools/golden_root.py map cTrayAssignment.cpp <V912 行>` 逐一換算（difflib 相同段，61 列裡 59 列
#   機械換、2 列落在不同段由人處理，見檔尾 E030-Q78／E032 兩段）。0618 1728 行／V912 1817 行。
#   0618 與 V912 不同的程式段落（`python tools/golden_root.py blocks cTrayAssignment.cpp`）全部留 V912 ⇒ 產生的程式跟換樹前一樣：
#     Steven Q81＝A（1003 14:3x，decisions-decided Q81：V912 才有的客戶功能留 V912）—— B1 FixCanUse HT9046_LS 轉盤 kit（V912 :115-122）、
#     B2 FixCanUse Top&Bottom AOI（:139-160）、B3 ReadFile CYUEAN 停 Auto Tray Feed（:278-280）、B4 FormShow CYUEAN 鎖勾選（:860-865）、
#     B5 KYEC AMR 分頁（ReadFile :538-544、存檔鈕 :1399-1402）、B6 DFM GroupBox2_KYEC Visible=False（dfm:1310，'dfm_keep'）；
#     第 1 條修正 —— Fix3 =false（C27，V912 :212）、CASE-20260611-001 下半盤同步（:453-481）；Q78 A02 的 return;（C24，V912 :1344）。
#   每處三段註解在檔尾（E030-Q78、E032、E031 三段）。DFM 其餘不同（Left／Height／Width／TabOrder，產生器不讀）不影響產出。
# Steven 團隊 20260925：TrayForm（<recipe>\Tray.Data 的 Tray Assignment 那一半）—— golden TfTrayAssignment
# （golden 906 0618 cTrayAssignment.cpp，1728 行；20260925～E-030 是 V912，1817 行）。頁面 web/page/Setup.TrayAssignment.html。
# 沒有 HTEditList：golden 存檔鈕 spbSaveClick（:1254）→ SaveSetupFile（:1320）自己逐鍵 WriteIniData，
# 存完 ReadFile（:1540）。同 ArmSpeed_File 的做法：替身＋直接跑 golden 存檔鈕。
# 前一版 A 形狀已退役（tools/formbridge/_retired/TfTrayAssignment.py）；C 形狀直接轉 golden 的 ReadFile（E-031 起是 0618），
# 那一版回報的「移植樹讀檔器落後 912（AMR Loader Re-Test、Top&Bottom AOI）」在這條路上不存在
# （開頁／存後重讀跑的就是 golden 的 ReadFile＋FixCanUse；E-031 起讀 0618，Top&Bottom AOI 那段照 Steven Q81＝A 留 V912，見檔尾 B2）。
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
import golden_root as _GR   # AI(W906-E031) 20261003 [W906]：golden 一律經過 tools/golden_root.py（行號＝產生器的行號）

_TREE = _GR.tree_of('TrayForm', '906')   # AI(W906-E031)：E-031 試行 —— 906 0618；STRUCT['golden'] 同一個值
_GL = _GR.lines(_TREE, 'cTrayAssignment.cpp')


def _expect(ln, text):
    if text not in _GL[ln - 1]:
        raise SystemExit('TrayForm.py: golden cTrayAssignment.cpp:%d 不再是 %r（實際 %r）' % (ln, text, _GL[ln - 1].strip()))
    return ln


# TA-1 方向圖：golden 點 TImage → iTrayDirect[Ptr->Tag] 循環 0..7、Picture 換成 type<dir>.bmp（:888-908）。
#   替身的 Tag 在這一頁的意思改成「目前載入的圖號」（＝type<n>.bmp 的 n；頁面引擎 web/page/ht9045_wire_engine.js:1166-1175
#   對 TImage 替身的慣例：tag＝圖號、顯示 img/dfm_type<tag>.png）。golden 拿 DFM Tag 當 iTrayDirect[] 的索引
#   （imgLoader 0、imgAuto1..6 1..6、imgFix1..6 7..12；golden 執行期從不改圖片的 Tag）→ 改由 TA_ImgIndex() 查同一張表
#   （FileRW/TrayForm.cpp kTA_ImgName）。不這樣做的話，產生器替 form.event 控制項帶入的 DFM Tag 1..12 會被 editlist.get 當成
#   圖號送出（PutProxyValue：Tag 非 0 就送），引擎顯示 dfm_type12.png、點一下在本地把 Tag 改掉、存檔送回 → 索引被頁面改寫。
_IMG = 'TA-1：替身 Tag＝目前載入的圖號（golden Picture＝type<n>.bmp），iTrayDirect[] 的索引＝golden DFM Tag 由 TA_ImgIndex() 查表（見設定檔 _IMG 註解）'
# VCL TCustomRadioGroup.SetItemIndex：程式設 ItemIndex（夾在 -1..Count-1）值有變 → TGroupButton.Checked → Click → OnClick
#   （同 tools/editlist/TestIF_File_Cleaning.py 的 (2) CL_RadioIndex、HSys.py:116）。vclcompat 替身不觸發 → TA_RadioIndex
#   （FileRW/TrayForm.cpp）。
#   //AI(W906-EVB10B) 20260929 [W906] X-3（R100＝照 BCB）：⛔ 更正 —— 以前「只用在本波新轉的處理器裡；既有方法（DoIniDataToForm :494-550 等）
#   照舊不觸發」。現在 STRUCT 'vcl_clicks'＝True：產生器把所有轉出方法裡的 RadioGroup ItemIndex／CheckBox Checked 指派改寫成
#   filerw::ELClickIndex／ELClickChecked，DFM 的 OnClick 在 TA_DfmState 登記（開頁 DoIniDataToForm／FormShow、存檔鈕、關頁 FormClose
#   照 VCL 觸發）；TA_RadioIndex／TA_SetRadioIndex 也改走同一個登記表（FileRW/TrayForm.cpp）。
_VCLRG = 'VCL 程式設 TRadioGroup::ItemIndex（值有變）會觸發 DFM OnClick；vclcompat 替身不觸發 → TA_RadioIndex（FileRW/TrayForm.cpp）'
_EV_REPLACE = [
    # ---- TA-1 imgLoaderClick（:888）----
    ('imgLoaderClick', _expect(890, 'TImage *Ptr;'), 890, 'TImage 在 vclcompat 沒有 → 替身是 TControl（產生器 KNOWN 以外的型別退回 TControl）',
     'TControl *Ptr;'),
    ('imgLoaderClick', _expect(892, 'Ptr=(TImage *)Sender;'), 892, '同 :890', 'Ptr=(TControl *)Sender;'),
    ('imgLoaderClick', _expect(893, 'int dir=iTrayDirect[Ptr->Tag];'), 893, _IMG + '；不是這 13 張圖的替身（不會發生：事件表只列這 13 個）→ 記 todo 不動',
     'const int iTag_=TA_ImgIndex(Ptr); if(iTag_<0) { filerw::ELTodo("golden cTrayAssignment.cpp:893 imgLoaderClick Sender is not one of the 13 direction images -- nothing changed"); return; } int dir=iTrayDirect[iTag_];'),
    ('imgLoaderClick', _expect(897, 'iTrayDirect[Ptr->Tag]=dir;'), 897, _IMG, 'iTrayDirect[iTag_]=dir;'),
    ('imgLoaderClick', _expect(898, 'sprintf(Dir, "%stype%d.bmp", BmpPath, dir);'), 898,
     'C 的 sprintf 可變參數不能收 vclcompat AnsiString（BCB6 的 AnsiString 恰好只有一個 char* 成員，才過得去）→ .c_str()，同一個字串',
     'sprintf(Dir, "%stype%d.bmp", BmpPath.c_str(), dir);'),
    ('imgLoaderClick', _expect(901, 'Ptr->Picture->LoadFromFile(Dir);'), 901, _IMG + '：換圖 ＝ 替身 Tag 換成新圖號（form.event 的 changed 就帶這個 tag）',
     'Ptr->Tag=dir;'),
    # ---- TA-1 ShowTrayDirectIMG 的換圖（:1140；:1129-1130 見 replace 主表）----
    ('ShowTrayDirectIMG', _expect(1140, 'MyImage[i]->Picture->LoadFromFile(Dir);'), 1140, _IMG + '：開頁把檔案的方向顯示出來',
     'MyImage[i]->Tag=iTrayDirect[i];'),
    # ---- TA-2 rgLoaderTypeClick（:1070）----
    ('rgLoaderTypeClick', _expect(1075, 'rgLoaderType->ItemIndex=TrayForm.LodareType;'), _expect(1076, 'return;'),
     'KYEC 條碼登入 Barcode_Reader(bcTrayAssign)（只有 CC_KYEC_LEE／CC_KYEC_XILINX 且 USE_BARCODE_AS_KEYBOARD 時問操作員 ID，其他機台回 2）回 0 → '
     'golden 改回 TrayForm.LodareType（' + _VCLRG + '：golden 這裡會再進一次 rgLoaderTypeClick，值相等 → cbLoaderChange＋ShowCompnet）；'
     '另記 ELTodo 讓頁面看得到為什麼被改回（輸入框是離線空殼，同 tools/editlist/UserDefForm_File.py cbTrayType1Change）',
     'TA_RadioIndex(EL<TRadioGroup>("TfTrayAssignment", "rgLoaderType"), TrayForm.LodareType); '
     'filerw::ELTodo("golden cTrayAssignment.cpp:1073 Barcode_Reader(bcTrayAssign) returned 0 (KYEC operator-ID prompt; the port input box is an offline shell) -- golden puts rgLoaderType back"); return;'),
    ('rgLoaderTypeClick', _expect(1079, 'cbLoaderChange(this);'), 1079, '事件以 (this) 呼叫（參數已拿掉）', 'TA_cbLoaderChange();'),
    # ---- TA-3 rgFixTrayModeClick（:1083）：Fix 盤有 IC 不准切（:1099-1111 照 golden）----
    ('rgFixTrayModeClick', _expect(1110, 'rgFixTrayMode->ItemIndex=TrayForm.iFixTrayMode;'), 1110,
     _VCLRG + '（golden 改回原值會再進一次 rgFixTrayModeClick，用原值重算 ckUseFix1..6 的 Visible）',
     'TA_RadioIndex(EL<TRadioGroup>("TfTrayAssignment", "rgFixTrayMode"), TrayForm.iFixTrayMode);'),
    # ---- TA-2 RGLoaderClick（:1150）／rgLoad_RTClick（:1192）的圖像模式捲軸 ----
    ('RGLoaderClick', _expect(1175, 'sbNormalTest->Position=0;'), 1175, _GRAPH, _tb('sbNormalTest') + '->Position=0;'),
    ('RGLoaderClick', _expect(1179, 'sbNormalTest->Position=15;'), 1179, _GRAPH, _tb('sbNormalTest') + '->Position=15;'),
    ('rgLoad_RTClick', _expect(1217, 'sbNormalTest_RT->Position=0;'), 1217, _GRAPH, _tb('sbNormalTest_RT') + '->Position=0;'),
    ('rgLoad_RTClick', _expect(1221, 'sbNormalTest_RT->Position=15;'), 1221, _GRAPH, _tb('sbNormalTest_RT') + '->Position=15;'),
    # ---- TA-2 cbLoaderChange（:1234）----
    ('cbLoaderChange', _expect(1238, 'rgLoad_RTClick(this);'), 1238, '事件以 (this) 呼叫（參數已拿掉）', 'TA_rgLoad_RTClick();'),
    ('cbLoaderChange', _expect(1239, 'RGLoaderClick(this);'), 1239, '事件以 (this) 呼叫（參數已拿掉）', 'TA_RGLoaderClick();'),
    # ---- TA-4 BinTrayDetect（:1630）的改回（:1667-1673）：VCL 會觸發被改那一組的 OnClick ----
    ('BinTrayDetect', _expect(1667, 'RGLoader->ItemIndex=0;'), 1667, _VCLRG + '（RGLoader OnClick＝RGLoaderClick，dfm:407）',
     'TA_RadioIndex(%s, 0);' % _rg('RGLoader')),
    ('BinTrayDetect', _expect(1669, 'RGAuto2->ItemIndex=0;'), 1669, _VCLRG + '（RGAuto2 OnClick＝RGAuto2Click，dfm:316）',
     'TA_RadioIndex(%s, 0);' % _rg('RGAuto2')),
    ('BinTrayDetect', _expect(1671, 'rgLoad_RT->ItemIndex=0;'), 1671, _VCLRG + '（rgLoad_RT OnClick＝rgLoad_RTClick，dfm:555）',
     'TA_RadioIndex(%s, 0);' % _rg('rgLoad_RT')),
    ('BinTrayDetect', _expect(1673, 'rgAuto2_RT->ItemIndex=0;'), 1673, _VCLRG + '（rgAuto2_RT OnClick＝cbEmptyChange，dfm:464）',
     'TA_RadioIndex(%s, 0);' % _rg('rgAuto2_RT')),
]


STRUCT = {
    'struct': 'TrayForm',
    'golden': _TREE,   # AI(W906-E031) 20261003 [W906]：906 0618（tools/golden_root.py）
    'prefix': 'TA',
    'class': 'TfTrayAssignment',
    'cpp': 'cTrayAssignment.cpp',
    'h': 'cTrayAssignment.h',
    'files': ['Tray.Data'],
    'lists': [],
    # AI(W906-FRW-S158) 20260927 [W906]：＋第二行 9 個 —— WS form.event 的 golden 處理器（TA-1～TA-4，見下面 'events'）
    #   與它們呼叫的 BinTrayDetect（:1630）。cbLoaderDropDown（:1692，cbLoader OnDropDown，KYEC 條碼、回 0 也只是 return）、
    #   cbEnableAMR_KYECClick（:1716，KYEC AMR）、sbNormalTestChange／_RT 的頁面觸發（TA-5）、sbtExitClick 不在本波。（⛔ 20261001：TA-5 已接，見 'events' 檔尾兩列；cbEnableAMR_KYECClick 20260929 EVB10B 已接）
    'methods': ['TfTrayAssignment', 'FixCanUse', 'ReadFile', 'DoIniDataToForm', 'FormShow', 'ShowCompnet',
                'ShowTrayDirectIMG', 'GraphicToRadio', 'RadioToGraphic', 'GraphicToRadio_RT', 'RadioToGraphic_RT',
                'sbNormalTestChange', 'sbNormalTest_RTChange', 'FormClose', 'spbSaveClick', 'SaveSetupFile',
                'imgLoaderClick', 'rgLoaderTypeClick', 'rgFixTrayModeClick', 'cbEmptyChange', 'RGLoaderClick',
                'rgLoad_RTClick', 'cbLoaderChange', 'RGAuto2Click', 'BinTrayDetect',
                # //AI(W906-EVB10B) 20260929 [W906] X-3：cbEnableAMR_KYEC 的 OnClick（dfm:1456，golden :1716：勾 ⇒ rgLoaderTrayMode=0，
                #   取消 ⇒ =1）。DoIniDataToForm :508 設 Checked 時 VCL 就會跑它（排在 :491 設 rgLoaderTrayMode 之後 ⇒ 開頁時 AMR 勾著就會把
                #   Loader Tray Mode 改成 0）；也登記成 form.event（使用者點）。加在最後：既有方法的產生碼順序不動。
                'cbEnableAMR_KYECClick'],
    'vcl_clicks': True,   # //AI(W906-EVB10B) 20260929 [W906] X-3：見檔頭 _VCLRG 下的更正
    # GraphicToRadio／_RT 在存檔鈕 :1295-1296 跑（圖像模式時 RadioGroup 由 Position 決定）→ 它們讀的 Position 也是存檔輸入
    'save_methods': ['spbSaveClick', 'SaveSetupFile', 'GraphicToRadio', 'GraphicToRadio_RT'],
    # AI(W906-FRW-S158)：imgLoaderClick 保留 golden 的 TObject *Sender（它讀 Sender 決定是哪一張圖）；其餘處理器不讀 Sender → ''
    'params': {'TfTrayAssignment': '', 'FormShow': '', 'FormClose': '', 'spbSaveClick': '',
               'sbNormalTestChange': '', 'sbNormalTest_RTChange': '',
               'rgLoaderTypeClick': '', 'rgFixTrayModeClick': '', 'cbEmptyChange': '', 'RGLoaderClick': '',
               'rgLoad_RTClick': '', 'cbLoaderChange': '', 'RGAuto2Click': '',
               'cbEnableAMR_KYECClick': ''},   # //AI(W906-EVB10B) 20260929 [W906]：golden 不讀 Sender
    'rettype': {'BinTrayDetect': 'int'},
    'members': ['int iTrayDirect[eTrayCount];            // golden cTrayAssignment.h:285（private）方向圖索引 0=Loader 1..6=Auto 7..12=Fix',
                'bool fShow=false;                       // golden cTrayAssignment.h:302',
                'TCheckBox *chkICSort[MAX_AUTO_TRAY];    // golden cTrayAssignment.h:303（建構子 :46-47 填）',
                'TEdit *edtICSort[MAX_AUTO_TRAY][6];     // golden cTrayAssignment.h:304（建構子 :48-60 填）',
                'AnsiString Caption;                     // golden TForm::Caption（FormShow :674，HTML 不用）',
                'int Left=0;                             // golden TForm::Left（FormShow :680）',
                'int Top=0;                              // golden TForm::Top（FormShow :681）',
                'int Width=0;                            // golden TForm::Width（ShowCompnet :1006-1064）',
                'bool TA_rgLoaderTrayModeItems=true;     // FileRW：rgLoaderTrayMode->Controls[0..2]->Enabled（golden ReadFile :303-338，三顆一起設）'],
    'replace': [
        # ---- ReadFile ----
        ('ReadFile', 303, 305, 'rgLoaderTrayMode->Controls[0..2]->Enabled=false：vclcompat TRadioGroup 沒有 Controls[]；golden 三顆一起設、DFM 只有三項 ⇒ 等同「這組不能改」，記旗標，FormShow 結尾併進 Enabled',
         'TA_rgLoaderTrayModeItems=false;'),
        ('ReadFile', 312, 314, '同 :303-305（KLT Auto SKIP）', 'TA_rgLoaderTrayModeItems=false;'),
        ('ReadFile', 320, 322, '同 :303-305（KLT 非 Auto SKIP：三顆打開）', 'TA_rgLoaderTrayModeItems=true;'),
        ('ReadFile', 336, 338, '同 :303-305（CC_KYEC_LEE）', 'TA_rgLoaderTrayModeItems=false;'),
        # ---- FormShow ----
        # AI(W906-FRW-S158) 20260927 [W906]：⛔ 更正 —— 原本換成空敘述（「HTML 自己有」）。13 張方向圖現在是替身（Tag＝圖號，TA-1），
        #   golden LoadImage（:647-663）＝全部載入 type0.bmp ⇒ 13 個替身 Tag＝0（:675 ShowTrayDirectIMG 再換成檔案的方向；頁面看到的結果不變）
        ('FormShow', 669, 669, 'LoadImage()：13 張方向圖全部載入 type0.bmp（:647-663）＝ ' + _IMG,
         'for(int k_=0; k_<13; k_++) TA_Img(k_)->Tag=0;'),
        ('FormShow', 883, 883, 'golden 每顆 RadioButton 的 Enabled（ReadFile 記的旗標）與群組 Enabled 取 AND ＝ 頁面上這組可不可以改',
         '{ if(!TA_rgLoaderTrayModeItems) EL<TRadioGroup>("TfTrayAssignment", "rgLoaderTrayMode")->Enabled=false; } fShow=true;'),
        # ---- ShowTrayDirectIMG：只留 iTrayDirect 夾值（<0 或 >8 → 0，存檔寫的就是它）----
        # AI(W906-FRW-S158) 20260927 [W906]：⛔ 更正 —— 原本換成空敘述（「圖片 HTML 端處理」）；現在 13 張圖是替身（TA-1，_IMG），:1140 換圖寫替身 Tag
        ('ShowTrayDirectIMG', 1129, 1130, 'TImage *MyImage[]＝13 張方向圖（TImage 在 vclcompat 沒有 → TControl 替身，順序同 golden：imgLoader, imgAuto1..6, imgFix1..6）',
         'TControl *MyImage[13]; for(int k_=0; k_<13; k_++) MyImage[k_]=TA_Img(k_);'),
        ('ShowTrayDirectIMG', 1133, 1133, 'sizeof(MyImage)/4 ＝ 13（BCB6 32 位元指標）', 'for(int i=0; i<13; i++)'),
        # ---- 圖像模式 ----
        ('GraphicToRadio', 1558, 1559, _PIC + '。自己換成空敘述：產生器的 UI-only 改寫會把 ImgNormalTest 留在同一行註解裡，'
         '被存檔讀替身的掃描當成「讀」→ mustSend 出現沒有值種類的 TImage，頁面永遠存不了', ';'),
        ('GraphicToRadio', 1561, 1564, _GRAPH,
         _g2r('RGAuto3', 'sbNormalTest', 1) + ' ' + _g2r('RGAuto2', 'sbNormalTest', 2) + ' ' +
         _g2r('RGAuto1', 'sbNormalTest', 4) + ' ' + _g2r('RGLoader', 'sbNormalTest', 8)),
        ('RadioToGraphic', 1579, 1579, _GRAPH, _tb('sbNormalTest') + '->Position=j;'),
        ('RadioToGraphic', 1581, 1581, _PIC, ';'),
        ('GraphicToRadio_RT', 1593, 1594, _PIC + '（同 :1558-1559，ImgReTest）', ';'),
        ('GraphicToRadio_RT', 1598, 1598, 'pgRunMode->ActivePage==tsNormalTestGraph：vclcompat TPageControl 只有 ActivePageIndex（tsNormalTestGraph＝第 2 頁，DFM 順序）',
         'if(TA_RunModePageIndex()==2)'),
        ('GraphicToRadio_RT', 1600, 1600, _GRAPH, _tb('sbNormalTest_RT') + '->Position=' + _tb('sbNormalTest') + '->Position;'),
        ('GraphicToRadio_RT', 1604, 1607, _GRAPH,
         _g2r('rgAuto3_RT', 'sbNormalTest_RT', 1) + ' ' + _g2r('rgAuto2_RT', 'sbNormalTest_RT', 2) + ' ' +
         _g2r('rgAuto1_RT', 'sbNormalTest_RT', 4) + ' ' + _g2r('rgLoad_RT', 'sbNormalTest_RT', 8)),
        ('RadioToGraphic_RT', 1622, 1622, _GRAPH, _tb('sbNormalTest_RT') + '->Position=j;'),
        ('RadioToGraphic_RT', 1624, 1624, _PIC, ';'),
        # ---- spbSaveClick ----
        ('spbSaveClick', 1260, 1260, 'Close()：golden 權限不足時關表單 → 伺服器端記 closed（ack.trace），頁面自己關',
         'filerw::ELMark("closed");'),
        ('spbSaveClick', 1271, 1272, 'pgRunMode->ActivePage==tsNormalTestGroup||tsNormalTestGraph：頁面不送目前分頁，伺服器保留 DFM 的 tsReTestGroup（第 1 頁）⇒ 不複製 FT→RT',
         'filerw::ELTodo("golden cTrayAssignment.cpp:1271 bFTBin2RTBin copies FT->RT only when an FT tab is active; the page does not send the pgRunMode tab -- server keeps the DFM tab tsReTestGroup => no copy, RT saved as sent"); '
         'if(TA_RunModePageIndex()==0 || TA_RunModePageIndex()==2)'),
        ('spbSaveClick', 1277, 1277, _GRAPH, _tb('sbNormalTest_RT') + '->Position=' + _tb('sbNormalTest') + '->Position;'),
    ],
    # Steven 20260925（Steven：「P46 是特殊功能，可以接受 by config 或是 by 工作檔，需要根據 BCB 的流程做決定」）：
    # 照 golden cTrayAssignment.cpp:1330-1337——bP46 開時這一鍵寫 config.ini [Flag] Skip Manual Remove Tray（:1332），
    # 關時寫 Tray.Data（:1336），都由 TfTrayAssignment 的存檔寫。原本這裡擋掉 :1332（「config.ini 歸 IniConfig」），
    # 結果兩頁都存不到。擁有者閘擋的是 B 路整檔鏡像；兩個 golden 存檔流程各寫 config.ini 的不同鍵是 golden 本來的設計
    # （IniConfig 的 SaveEditTextToFile 逐鍵寫，不會蓋掉這一鍵），所以照 golden 放行。
    'blocks': [],
    'includes': ['cprod.h', 'Config.h', 'LastSet.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'forms/fMain.h', 'forms/fSecurity.h', 'forms/fLotInfo.h',
                 'forms/fShowBinSelect.h', 'csystem.h', 'Motor/mymotor.h',
                 'SECSGEM/SecsEventType.h', 'SECSGEM/SecsEventReport.h',
                 'BarcodeReader.h'],   # AI(W906-FRW-S158) 20260927 [W906]：rgLoaderTypeClick :1073 Barcode_Reader（同 UserDefForm_File）
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
    #   * BinTrayDetect（:1664-1673）改回的是「第一個」值為 2 的群組（RGLoader > RGAuto2 > rgLoad_RT > rgAuto2_RT），不一定是剛點的那組。
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
               ('rgAuto6_RT', 'click', 'cbEmptyChange'),                      # dfm:512
               ('cbEnableAMR_KYEC', 'click', 'cbEnableAMR_KYECClick'),        # dfm:1456（//AI(W906-EVB10B) 20260929 [W906]）
               # AI(W906-TA5) 20261001 [W906]：TA-5 圖像模式的兩條捲軸（TScrollBar，C 路替身 ELTrackBar；form.event 帶 "position"＝X-2，3.0g-9）。
               #   golden cTrayAssignment.cpp:1550（V912 :1639）sbNormalTestChange → GraphicToRadio（:1555-1565：RGAuto3／RGAuto2／RGAuto1／RGLoader
               #   ＝Position 的 bit0～bit3，VCL 夾值、值有變就跑那個群組的 OnClick）；:1585 sbNormalTest_RTChange → GraphicToRadio_RT（:1590-1608）。
               #   只在 IniConfig.bTrayAssignUseGraphic 且 AUTO_EMPTY_COLOR<3 且 rgLoaderType->ItemIndex!=0 時看得到（ShowCompnet :970-994 的
               #   tsNormalTestGraph／tsReTestGraph TabVisible）⇒ 其他時候 ELOperable 擋。分頁跳板見 FileRW/TrayForm.cpp kTA_OnTab。
               ('sbNormalTest', 'change', 'sbNormalTestChange'),              # dfm:579
               ('sbNormalTest_RT', 'change', 'sbNormalTest_RTChange')],       # dfm:602
}
STRUCT['replace'] += _EV_REPLACE   # AI(W906-FRW-S158) 20260927 [W906]：form.event 處理器用到的等價取代（見檔頭 _EV_REPLACE）

# SaveSetupFile 的 PtrCombBox[RGAutoN->ItemIndex]：golden 下標沒檢查（VCL 的 RadioGroup 會把 ItemIndex 夾在 -1..Count-1，
# 但 -1 仍會越界）。vclcompat 不夾，頁面沒選時送 -1 ⇒ 伺服器端讀野指標。越界時不寫這一鍵、ELTodo（其餘照 golden）。
_PTR = {1455: ('eAuto1', 'RGAuto1'), 1473: ('eAuto3', 'RGAuto3'), 1491: ('eAuto4', 'RGAuto4'),
        1498: ('eAuto5', 'RGAuto5'), 1505: ('eAuto6', 'RGAuto6')}
for _ln, (_t, _w) in sorted(_PTR.items()):
    STRUCT['replace'].append(
        ('SaveSetupFile', _ln, _ln, 'PtrCombBox[%s->ItemIndex] 下標越界時 golden 是未定義行為 → 不寫這一鍵、ELTodo' % _w,
         '{ const int k_=%s->ItemIndex; if(k_<0 || k_>1) filerw::ELTodo("golden cTrayAssignment.cpp:%d PtrCombBox[%s->ItemIndex] out of range (golden UB) -- key not written"); '
         'else WriteIniData(szDir, s6TrayName[%s], "Tray Type", PtrCombBox[k_]->ItemIndex); }' % (_rg(_w), _ln, _w, _t)))
STRUCT['replace'].append(
    ('SaveSetupFile', 1466, 1466, 'PtrCombBox[ibuffer] 下標越界時 golden 是未定義行為 → 不寫這一鍵、ELTodo',
     'if(ibuffer<0 || ibuffer>1) filerw::ELTodo("golden cTrayAssignment.cpp:1466 PtrCombBox[ibuffer] out of range (golden UB) -- key not written"); '
     'else WriteIniData(szDir, s6TrayName[eAuto2], "Tray Type", PtrCombBox[ibuffer]->ItemIndex);'))

# ShowCompnet :953-958／:962-967 `?cbEmpty->Text:cbColor->Text`：產生器的元件樣式排除前一個字元是「:」的
# （為了不動 `X::`），三元運算子的 `:cbColor` 因此沒改成替身（編譯擋下）→ 逐行等價取代（只是把 cbColor 換成替身）
for _i, _ln in enumerate(range(953, 959)):
    STRUCT['replace'].append(
        ('ShowCompnet', _ln, _ln, '三元運算子 :cbColor 產生器沒改寫（前一字元是「:」）—— 等價取代',
         'EL<TEdit>("%s", "edAuto%dType")->Text=(%s->ItemIndex==0)?EL<TComboBox>("%s", "cbEmpty")->Text:EL<TComboBox>("%s", "cbColor")->Text;'
         % (_F, _i + 1, _rg('RGAuto%d' % (_i + 1)), _F, _F)))
for _i, _ln in enumerate(range(962, 968)):
    STRUCT['replace'].append(
        ('ShowCompnet', _ln, _ln, '三元運算子 :cbColor 產生器沒改寫（前一字元是「:」）—— 等價取代',
         'EL<TEdit>("%s", "edAuto%dType")->Text=(%s->ItemIndex==0)?EL<TComboBox>("%s", "cbEmpty")->Text:EL<TComboBox>("%s", "cbColor")->Text;'
         % (_F, _i + 1, _rg('rgAuto%d_RT' % (_i + 1)), _F, _F)))

# AI(W906-E030-Q78) 20261003 [W906] (St01)：Steven 1003 05:3x Q78 裁決「Q78 Q79, 可以按照912，但是註解同時提供906的行號位置」，
#   加上 05:4x 慣例「如果是912比較好，就是註記906的行號跟做法　然後增加註記912已修正或更新的行號」⇒ A02 分支
#   （RogerYang 20260305 [A01_2]，906／V912 都有）裡 Close() 後面那句 `return;` 照 V912 留著，記成第 20 條的例外
#   （D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md Q78；human-review C24；RULINGS_20261003 第 6 條＝B）。
#   //AI(W906-E031) 20261003 [W906] (St01)：⛔ 更正 —— 產生器對本結構改讀 0618（E-031）之後，0618 的 A02 分支（:1256-1261）沒有
#   `return;`，原本那一列「等價取代」（V912 :1344 return; 換成自己）沒有行可以掛 ⇒ 改成把 `return;` 接在 0618 :1260 Close() 那一列
#   （主表的 spbSaveClick 列）的取代碼後面：'filerw::ELMark("closed");' → 'filerw::ELMark("closed"); return;'，原因接上下面三段。
#   這一列從此不是「等價取代」，是第 1 條的插入（產生的程式跟 V912 一樣：A02＋OP 按存檔 ⇒ 不寫檔）。
#   釘子：0618 的 if／Close()／} 三行（_expect）、V912 的 Close()／return 兩行（_v912_expect，經 golden_root 讀 V912）。
_E031_V912 = _GR.lines('v912', 'cTrayAssignment.cpp')   # AI(W906-E031)：只拿來釘第 1 條保留的 V912 原文（V912 變了就停）


def _v912_expect(gl, text):
    """golden V912 cTrayAssignment.cpp 第 gl 行（去掉 // 註解與頭尾空白）要等於 text，否則中止。"""
    s = _E031_V912[gl - 1].split('//')[0].strip()
    if s != text:
        raise SystemExit('TrayForm.py (E031): golden V912 cTrayAssignment.cpp:%d is %r, expected %r' % (gl, s, text))
    return gl


_expect(1256, 'if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&')
_expect(1260, 'Close();')
_expect(1261, '}')
_v912_expect(1343, 'Close();')
_v912_expect(1344, 'return;')
_E030Q78_WHY = ('AI(W906-E030-Q78)／AI(W906-E031) 插入 V912 的 return;（0618 沒有這一句）。'
                '(1) 906 cTrayAssignment.cpp:1256-1261：A02 分支只有 Close()（:1260），後面沒有 return，存檔照跑；主選單 main.cpp:27443 用 ShowModal 開（對話框），Close() 只設 ModalResult ⇒ 操作員改的值照樣寫進檔。'
                '(2) V912 cTrayAssignment.cpp:1343-1344：Close() 後多一句 `return;` ⇒ 頁面關掉、不寫檔（A02 分支本身兩版都有，RogerYang 20260305 [A01_2]）。'
                '(3) Kept: #20 exception (Steven 1003 Q78, keep V912; RULINGS_20261003 #6)')
_q78 = [k for k, r in enumerate(STRUCT['replace']) if r[0] == 'spbSaveClick' and r[1] == 1260]
assert len(_q78) == 1 and STRUCT['replace'][_q78[0]][4] == 'filerw::ELMark("closed");', _q78
STRUCT['replace'][_q78[0]] = ('spbSaveClick', 1260, 1260, STRUCT['replace'][_q78[0]][3] + '；' + _E030Q78_WHY,
                              'filerw::ELMark("closed"); return;')
# AI(W906-E032) 20261003 [W906] (St01)：todo E-032（golden 改回 906 0618，Jimmy RULINGS_20261003 第 2 條）——0618 與 0625 不同的
#   函式 TfTrayAssignment::ReadFile 只差一行。RULINGS_20261003 第 1 條（Steven 1003 常設規則）⇒ 留 V912（human-review C27）。
#   //AI(W906-E031) 20261003 [W906] (St01)：⛔ 更正 —— E-032 時產生器根目錄還是 V912，這一列是「等價取代」（釘 V912 :211-212）；
#   E-031 本結構改讀 0618 ⇒ 改釘 0618 :177-178（`==false` 那一行）、取代碼不變（V912 的 `=false`）⇒ 這一列從此真的把 0618 的
#   no-op 比較換成指定（產生的程式跟 V912 一樣）。V912 那一行另用 _v912_expect 釘住。
_expect(177, 'if(Prod.iTrayType[eFix3]==tNotUse)')
_v912_expect(212, 'TrayForm.bTrayUpDownSet[eFix3]=false;')
_E032_FIX3_WHY = ('AI(W906-E032)／AI(W906-E031) 第 1 條保留 V912（取代 0618 的 no-op 比較）。'
                  '(1) 906 0618 cTrayAssignment.cpp:178 `TrayForm.bTrayUpDownSet[eFix3]==false;` 是比較不是指定（code has no effect）'
                  '⇒ Fix Up/Down 模式、Fix3 盤型是 tNotUse 時，Fix3 的上下層旗標留著上一次的值（不清掉）。'
                  '(2) 0625 cTrayAssignment.cpp:178／V912 :212 改成 `=false`（Steven 0625 的 == 誤寫清理，V912 沿用）⇒ 不用的 Fix3 一定是 false；留這一版＝修正。'
                  '(3) #20 exception (Steven 1003 standing rule, RULINGS_20261003 #1)')
STRUCT.setdefault('replace', []).append(
    ('ReadFile', _expect(178, 'TrayForm.bTrayUpDownSet[eFix3]==false;'), 178, _E032_FIX3_WHY,
     'TrayForm.bTrayUpDownSet[eFix3]=false;'))
# AI(W906-E031) 20261003 [W906] (St01)：第 1 條保留 V912 —— ReadFile 的 CASE-20260611-001（V912 :453-481，AI(ht9045-v899) 20260611）。
#   (1) 906 0618 cTrayAssignment.cpp:378-413 讀 "Tray Type"／"Direction" 的迴圈只到 iFixRight，Fix Up/Down 模式（iFixTrayMode==1）的
#       下半盤（eFix4～6，AUTO_EMPTY_COLOR>=3 時 eFix7～12）Direction／iTrayType 從來沒設、停在 0 ⇒ 下半盤（例 Fix5）擺放方向跟上半盤不一樣。
#   (2) V912 cTrayAssignment.cpp:453-481 在迴圈之後把下半盤的 Direction／iTrayType 同步成對應的上半盤（同 bTrayUpDownSet 的做法）
#       ⇒ 客訴 CASE-20260611-001 的修正，留 V912。
#   (3) #20 exception (Steven 1003 standing rule, RULINGS_20261003 #1)
#   做法：0618 沒有這段 ⇒ 插在 0618 :415 `if(bUseAuto2Empty==true)` 前面（V912 也是：:481 收尾、:482 空行、:483 同一個 if）。
#   取代碼＝V912 :453-481 原文（golden_root 讀 V912，cp950→UTF-8，含原註解；沒有元件與表單方法，不必經產生器改寫）＋空行＋0618 :415 那個 if；
#   V912 頭尾與下一行用 _v912_expect 釘住。
_v912_expect(453, 'if(TrayForm.iFixTrayMode==1)')
_v912_expect(481, '}')
_v912_expect(483, 'if(bUseAuto2Empty==true)')
_E031_CASE_WHY = ('AI(W906-E031) 第 1 條保留 V912：插入 V912 :453-481（CASE-20260611-001）。'
                  '(1) 906 0618 cTrayAssignment.cpp:378-413 的讀檔迴圈只到 iFixRight，Fix Up/Down 模式下半盤的 Direction／iTrayType 停在 0。'
                  '(2) V912 cTrayAssignment.cpp:453-481 把下半盤同步成上半盤（客訴修正）。'
                  '(3) #20 exception (Steven 1003 standing rule, RULINGS_20261003 #1)')
STRUCT.setdefault('replace', []).append(
    ('ReadFile', _expect(415, 'if(bUseAuto2Empty==true)'), 415, _E031_CASE_WHY,
     '\n'.join(l.rstrip() for l in _E031_V912[452:481]).strip() + '\n\n    if(bUseAuto2Empty==true)'))
# AI(W906-E031) 20261003 [W906] (St01)：Steven Q81＝A（1003 14:3x，D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md
#   Q81；「Steven認可的就是直接放行」，Jimmy 只通知）——725cde57 試行照 906 拿掉的 V912 才有的 6 項客戶功能（B1～B6）留 V912。
#   0618 沒有那幾段 ⇒ 每段一列 replace，接在 0618 的一行上（同一個 golden 起行只能有一列）：取代碼＝V912 原文（golden_root 讀 V912、
#   cp950→UTF-8、含原註解；元件照產生器的規則改成具名替身——只有 B4 的 chkAutoTrayFeed）＋那一行 0618 自己的原文（空白行就沒有），
#   產生的程式跟換樹前（讀 V912）一樣。B6 是 DFM 設計期屬性 ⇒ STRUCT['dfm_keep']（tools/gen_editlist.py）。
#   釘子：0618 錨點行（_expect／_expect_line）、V912 每段頭尾（_v912_expect），而且抄的 V912 每一行都必須是 V912 才有的
#   （_v912_only：golden_root.map_line 落在不同段）——V912 或 0618 改了，產生器就停。
#   FileRW/TrayForm.cpp FileRW_TrayAssignment_InitProdTrayType() 手寫的 Top&Bottom AOI 段（照 V912 main.cpp:1455-1462；0618 main.cpp 沒有）
#   因為 B2 留 V912 而跟著留。
import re as _re   # noqa: E402


def _expect_line(ln, text):
    """golden 906 0618 cTrayAssignment.cpp 第 ln 行（去掉 // 註解與頭尾空白）要等於 text（_expect 只看「含有」；'}'／空白行要整行比）。"""
    s = _GL[ln - 1].split('//')[0].strip()
    if s != text:
        raise SystemExit('TrayForm.py (E031): golden 906 cTrayAssignment.cpp:%d is %r, expected %r' % (ln, s, text))
    return ln


def _v912_only(a, b):
    """V912 cTrayAssignment.cpp :a-:b 每一行都在 V912 才有的段落（golden_root.map_line 回 None），否則中止。"""
    bad = [n for n in range(a, b + 1) if _GR.map_line('cTrayAssignment.cpp', n) is not None]
    if bad:
        raise SystemExit('TrayForm.py (E031): golden V912 cTrayAssignment.cpp:%r are also in 0618 -- not a V912-only block' % bad)


def _v912_text(a, b, widgets=()):
    """V912 :a-:b 原文（每行去尾端空白、保留縮排）；widgets＝[(元件名, 型別)] 照產生器的規則（gen_editlist.py convert 的 wre，
    // 註解不動）改成具名替身 EL<型別>("TfTrayAssignment", "名")。"""
    _v912_only(a, b)
    out = []
    for l in _E031_V912[a - 1:b]:
        code, sep, com = l.rstrip().partition('//')
        for n, t in widgets:
            code = _re.sub(r'(?<![\w.>:])%s\b(?!\s*::)' % n, 'EL<%s>("%s", "%s")' % (t, _F, n), code)
        out.append(code + sep + com)
    return '\n'.join(out)


_Q81 = '(3) #20 exception (Steven 1003 standing rule, Q81 = A)'
# ---- B1 FixCanUse：HT9046_LS 轉盤 kit（V912 :115-122）→ 插在 0618 :113 fShowBinSelect->SetAutoVisible() 前面（V912 也是 :124 那一行接在後面）
_expect_line(112, '}')
_v912_expect(115, 'if(MachineTypeChoice==Type_HT9046_LS && USE_ROTATE_KIT==1 && AUTO3_IS_MAGAZINE==0)')
_v912_expect(122, '}')
_v912_expect(124, 'fShowBinSelect->SetAutoVisible();')
_E031_B1_WHY = ('AI(W906-E031) Q81＝A B1 保留 V912（插入）。'
                '(1) 906 0618 cTrayAssignment.cpp:106-113：HT9045 汽缸 FIX＋Rotate 鎖 Fix3 之後直接 fShowBinSelect->SetAutoVisible()，'
                '沒有 HT9046_LS 轉盤 kit 那一段 ⇒ 9046LS 裝轉盤 kit（USE_ROTATE_KIT==1、Auto3 不是 Magazine）、Auto 3 盤時 Fix1／Fix4 照 Tray.Data 可用。'
                '(2) V912 cTrayAssignment.cpp:115-122（JerryYang 20260716）：那個條件下 iAutoCnt==3 ⇒ Prod.iTrayType[eFix1]／[eFix4]＝tNotUse。' + _Q81)
STRUCT['replace'].append(
    ('FixCanUse', _expect_line(113, 'fShowBinSelect->SetAutoVisible();'), 113, _E031_B1_WHY,
     _v912_text(115, 122).lstrip() + '\n\n    fShowBinSelect->SetAutoVisible();'))
# ---- B2 FixCanUse：Top&Bottom AOI（V912 :139-160）→ 接在 0618 :126（BulkBox else 的 `}`）後面、方法結尾 :127 之前（V912 :137 → :139-160 → :161）
_expect_line(125, 'Prod.iTrayType[eBulkBox     ]=tNotUse;')
_expect_line(127, '}')
_v912_expect(140, 'if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall)')
_v912_expect(159, '}')
_v912_expect(161, '}')
_E031_B2_WHY = ('AI(W906-E031) Q81＝A B2 保留 V912（插入）。'
                '(1) 906 0618 cTrayAssignment.cpp:115-127：FixCanUse 到 BulkBox 判斷就結束，沒有 Top&Bottom AOI ⇒ 裝了上下 AOI 時 Fix2／Fix3 照 Tray.Data 可用。'
                '(2) V912 cTrayAssignment.cpp:139-160（Ifor 20260702）：USE_Scanner_AOI_Inspection==eBtnAOI_TopBottomInstall ⇒ Fix2／Fix3 tNotUse，'
                'Fix Up/Down 模式再清 bTrayUpDownSet[eFix2]／[eFix3] 與下半盤（AUTO_EMPTY_COLOR>=3：Fix8／9，否則 Fix5／6）；'
                'FileRW/TrayForm.cpp 手寫的 Top&Bottom AOI 段（V912 main.cpp:1455-1462）跟著留。' + _Q81)
STRUCT['replace'].append(
    ('FixCanUse', _expect_line(126, '}'), 126, _E031_B2_WHY, '}\n\n' + _v912_text(139, 160)))
# ---- B3 ReadFile：CYUEAN 停 Auto Tray Feed（V912 :278-280）→ 0618 :243（Hana ART 那兩行後面的空白行；V912 :277 空白、:281 空白）
_expect(241, 'if(fMain->hanaART->IsHanaArtAvailable())')
_expect_line(242, 'TrayForm.bAutoFeed          =true;')
_v912_expect(279, 'if(CosFunction.bDisableAutoTrayFeed)')
_v912_expect(280, 'TrayForm.bAutoFeed          =false;')
_E031_B3_WHY = ('AI(W906-E031) Q81＝A B3 保留 V912（插入）。'
                '(1) 906 0618 cTrayAssignment.cpp:236-243：bAutoFeed 照 Tray.Data，Hana ART 時強制 true，CYUEAN 沒有特別處理 ⇒ CC_CYUEAN 照工作檔可開 Auto Tray Feed。'
                '(2) V912 cTrayAssignment.cpp:278-280（AI(ht9045-v899) 20260817）：CosFunction.bDisableAutoTrayFeed ⇒ bAutoFeed=false（蓋過 Tray.Data 與 Hana ART）。' + _Q81)
STRUCT['replace'].append(
    ('ReadFile', _expect_line(243, ''), 243, _E031_B3_WHY, _v912_text(278, 280).lstrip()))
# ---- B5（ReadFile 那一處）KYEC AMR 分頁（V912 :538-544）→ 接在 0618 :469（CosFunction.bTraySortCntFunc 那個 if 的 `}`）後面、方法結尾 :470 之前
_expect(454, 'if(CosFunction.bTraySortCntFunc)')
_expect_line(470, '}')
_v912_expect(538, 'if(fLotInfo!=NULL)')
_v912_expect(544, '}')
_v912_expect(546, '}')
_E031_B5R_WHY = ('AI(W906-E031) Q81＝A B5 保留 V912（插入，ReadFile 那一處）。'
                 '(1) 906 0618 cTrayAssignment.cpp:454-470：ReadFile 讀完 TraySortCnt 就結束，不碰 LotInfo ⇒ KYEC AMR 分頁不跟著 bEnableAMR 顯示／隱藏。'
                 '(2) V912 cTrayAssignment.cpp:538-544（Eastsun 20260710 Merge）：fLotInfo!=NULL 時 fLotInfo->tsKYEC_AMR->TabVisible＝TrayForm.bEnableAMR。' + _Q81)
STRUCT['replace'].append(
    ('ReadFile', _expect_line(469, '}'), 469, _E031_B5R_WHY, '}\n' + _v912_text(538, 544)))
# ---- B4 FormShow：CYUEAN 鎖 chkAutoTrayFeed（V912 :860-865）→ 0618 :783（Hana ART 停用那兩行後面的空白行；V912 :859 空白、:866 空白）
_expect(781, 'if(fMain->hanaART->IsHanaArtAvailable())')
_expect(782, 'chkAutoTrayFeed->Enabled        =false;')
_expect(784, 'if(CUSTOMER_CODE==CC_SCC ||')
_v912_expect(861, 'if(CosFunction.bDisableAutoTrayFeed)')
_v912_expect(863, 'chkAutoTrayFeed->Checked        =false;')
_v912_expect(864, 'chkAutoTrayFeed->Enabled        =false;')
_v912_expect(865, '}')
_E031_B4_WHY = ('AI(W906-E031) Q81＝A B4 保留 V912（插入）。'
                '(1) 906 0618 cTrayAssignment.cpp:781-783：只有 Hana ART 時停用 chkAutoTrayFeed，CYUEAN 照常可勾。'
                '(2) V912 cTrayAssignment.cpp:860-865（AI(ht9045-v899) 20260817）：CosFunction.bDisableAutoTrayFeed ⇒ chkAutoTrayFeed 取消勾選並停用'
                '（只鎖這一顆）；元件照產生器規則改成具名替身。' + _Q81)
STRUCT['replace'].append(
    ('FormShow', _expect_line(783, ''), 783, _E031_B4_WHY,
     _v912_text(860, 865, widgets=[('chkAutoTrayFeed', 'TCheckBox')]).lstrip()))
# ---- B5（存檔鈕那一處）KYEC AMR 分頁（V912 :1399-1402）→ 插在 0618 :1315 fMain->BackupSetupFile() 前面（V912 :1404 那一行接在後面）
_expect_line(1314, '#endif')
_v912_expect(1399, 'if(TrayForm.bEnableAMR)')
_v912_expect(1402, 'fLotInfo->tsKYEC_AMR->TabVisible=false;')
_v912_expect(1404, 'fMain->BackupSetupFile();')
_E031_B5S_WHY = ('AI(W906-E031) Q81＝A B5 保留 V912（插入，存檔鈕那一處）。'
                 '(1) 906 0618 cTrayAssignment.cpp:1310-1315：SaveSetupFile（＋ASE_KaohSiung 另存）之後直接 BackupSetupFile，不碰 LotInfo。'
                 '(2) V912 cTrayAssignment.cpp:1399-1402（Eastsun 20260710 Merge）：存檔後 fLotInfo->tsKYEC_AMR->TabVisible＝TrayForm.bEnableAMR'
                 '（這一處 V912 沒有檢查 fLotInfo!=NULL，照 V912）。' + _Q81)
STRUCT['replace'].append(
    ('spbSaveClick', _expect_line(1315, 'fMain->BackupSetupFile();'), 1315, _E031_B5S_WHY,
     _v912_text(1399, 1402).lstrip() + '\n\n    fMain->BackupSetupFile();'))
# ---- B6 DFM：GroupBox2_KYEC Visible=False（V912 cTrayAssignment.dfm:1310；0618 dfm:1296-1309 沒有 Visible）→ STRUCT['dfm_keep']
#   golden 兩棵的程式都不改 GroupBox2_KYEC（grep cTrayAssignment.cpp 兩棵都 0 處）⇒ V912 的 AMR 群組一直隱藏、0618 一直顯示。
_E031_B6_WHY = ('AI(W906-E031) Q81＝A B6 保留 V912（DFM 設計期屬性）。'
                '(1) 906 0618 cTrayAssignment.dfm:1296-1309：GroupBox2_KYEC 沒有 Visible ⇒ AMR 群組（cbEnableAMR_KYEC 等）一直顯示'
                '（兩棵的 cTrayAssignment.cpp 都不改它）。'
                '(2) V912 cTrayAssignment.dfm:1310 Visible = False ⇒ 一直隱藏。' + _Q81)
STRUCT['dfm_keep'] = [('GroupBox2_KYEC', 'Visible', 'v912', 1310, _E031_B6_WHY)]
