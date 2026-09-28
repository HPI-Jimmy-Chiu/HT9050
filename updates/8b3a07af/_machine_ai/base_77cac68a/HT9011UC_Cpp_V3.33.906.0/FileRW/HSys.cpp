// ===========================================================================
//  FileRW/HSys_C.cpp -- 結構 HSys（Handler System）的讀寫檔，C 路（具名替身）。
//    system\Gerneral.ini（asGeneralPath，WriteIniDataGeneral）＋ D:\RS232Standard\System\Setup.ini
//    ＋ D:\GPIB9045\system\general.ini（[Version] Model）。
//
//  Steven 團隊 20260925.  規格：.claude/skills/ht9045-json-bridge/references/write-inventory.md「Gerneral.ini（形狀 E）」。
//
//  ⚠ 檔名：FileRW/HSys.cpp 是前一版 A 形狀（tools/formbridge/THandlerSystem.py 產生），整合時退役；
//    gen_editlist.py 的 _editlist_sources.cmake 列的是 FileRW/<struct>.cpp ＝ FileRW/HSys.cpp —— 整合者要把本檔改名
//    成 HSys.cpp（或改清單），否則建置會編到 A 形狀那支。
//
//  golden THandlerSystem（HT9011UC_Code_V3.33.912.0_20260908_Jimmy HandlerSys.cpp）由 tools/gen_editlist.py
//  （tools/editlist/HSys.py）轉成 HSys.gen.inc（元件改成具名替身）：
//    建構子（:63）＝ slCustomerCode；
//    FormShow（:132）＝開頁（SortItemToMap（純畫面）→ LoaderSystemSet :177 → LoaderSafeDoorSet :1132 → GPIB 型號）；
//    SaveBtnClick（:1121）＝存檔鈕（SaveSystemSet :575：YES/NO → 逐鍵 WriteIniDataGeneral／WriteIniData → 部分全域立即改）。
//  沒有 HTEditList —— 存檔流程讀的替身全部是 mustSend。
//
//  Steven 20260925 裁決：開頁照 golden FormShow、讀檔照 golden SYSTEM_MODULAR::ReadGeneralIni（移植樹 database.cpp:313，
//  LoadMachineConfig 開機跑）；golden 開表單補寫缺鍵、建 D:\RS232Standard\System、補寫 GPIB general.ini 的行為照 golden 保留。
//  editlist.get／editlist.save 都在主迴圈跑（呼叫端持 FormLock），HSys.ReadGeneralIni() 在這裡呼叫沒有 HTTP 執行緒問題
//  （審查第 8 輪 H-1）。
//
//  bHandlerModel（golden TfMain::FormShow main.cpp:9589-9594）：
//    golden 開機 HSys.ReadGeneralIni()（database.cpp:302）讀 D:\GPIB9045\system\general.ini [Version] Model，不在白名單 →
//    bHandlerModel=false 並提早 return（其餘 Gerneral.ini 全域都沒讀）；TfMain::FormShow 看到 false 就 MessageDlg ＋
//    Application->Terminate()，整個程式停住 —— 任何表單都開不到、存不到。
//    網頁版不可以結束程式 → 開頁與存檔都先檢查：false 時 ELMessage（golden 原字串）＋ ELMark("model_read_error")，
//    不跑 golden FormShow（它會 CheckAndReadIniDataGeneral 補寫 Gerneral.ini —— golden 在這個狀態下根本走不到）、
//    拒絕存檔。頁面要把這則訊息當告警顯示。
//
//  存檔後跑不跑 HSys.ReadGeneralIni()（判斷，Steven 團隊 20260925）：跑 —— 只在「真的寫了檔」（YES 且型號正常）之後，
//    照 golden 離開鈕 ExitBtnClick（:1179）的資料那一半：HSys.ReadGeneralIni() → SaveSafeDoorSet()。理由：
//    * golden 的存檔鈕只寫檔＋改一部分全域（AUTO_EMPTY_COLOR、InOutArmPickerUseMotor、LOAD_Z_USE_MOTOR[]、CUSTOMER_CODE…），
//      另一部分（NUMBER_PANEL_TYPE、各 COM 埠、ATC…）要等離開鈕的 ReadGeneralIni 才進記憶體；golden 使用者存完按「Exit」
//      離開（表單正規的離開路徑），兩個鈕合起來記憶體＝檔案。網頁沒有「離開」事件，存完不跑就永遠停在半新半舊。
//    * 不跑的只有 golden 用右上角 X 關（FormClose :1215，不重讀）這條路 —— 那是例外路徑，不是 golden 的設計意圖。
//    * 沒寫檔（答 NO、型號錯）時不跑：golden 離開鈕此時重讀的是原檔，結果與不跑相同（SaveSafeDoorSet 的門數覆寫除外，
//      那只在 SAFE_DOOR_AMOUNT 改過之後才有差，而改它必須先存檔）。
//    * 不跑離開鈕的 Close()：網頁存完頁面仍開著（不記 "closed"）。
//    * 照 golden 不重跑 TfMain 建構子那兩段（FileRW_HSys_ReadMainCtorKeys）：EP_Install／ION_FAN_TYPE 的新值 golden 也要
//      重開程式才生效（SaveSystemSet :592 `//EP_Install=…` 是 golden 自己註解掉的）。
//    ⚠ 副作用：ReadGeneralIni 會重讀 config.ini（ReadLastSetIni → IniConfig）、CustomerFunctionSelect、補寫 Gerneral.ini 缺鍵；
//      SaveSafeDoorSet 改 Sen[SnSafeDoor1..10／SnHeaterDoor／SnHeaterDoor2].Enable（安全門感測器啟用，golden 同樣在離開鈕做）。
//
//  開機：golden HT9045.cpp:210 CreateForm(THandlerSystem)（在 TfConfiguration :207 之後）。
//
//  //AI(W906-FRW-P8) 20260926: 逐通道溫控器廠牌（golden 912 EN_HEATER_SHEET=1，Heater 分頁 grpHeater）接上：
//    * 表與小工具在本檔下方（golden MachineTypeUtility.cpp／HandlerSys.cpp:32-60），宣告 FileRW/HSys_Heater.h；
//    * 開機建 23 組具名替身 lb/cbHeaterInsOpt_<通道>（W906_HeaterInsCreateProxies），開頁照 golden 讀 71 個 [TempCtrl] 鍵
//      （缺鍵補寫 -9999），存檔照 golden 寫 71 鍵＋HEATER_CTRL_TYPE；
//    * rgHeaterType 被改（golden OnClick＝rgHeaterTypeClick，golden 按下即寫檔）→ beforeApply 在套值前重播（見 BeforeApply）；
//      //AI(W906-FRW-Q14) 20260927: [W906] 偏離 golden：Steven 20260927 Q14＝B —— 重播只改記憶體與畫面，寫檔等整頁存檔 SaveSystemSet；
//    * 頁面版面由 extraJson 給（web/page/ht9045_hsys_heater_c.js 照著在 grpHeater 裡建 Label／下拉）。
//    溫控流程本身仍讀單一廠牌 TC401HeaterControl（底層照 906，RULINGS_20260926 第 26 條；逐通道溫控列給 Jimmy）。
// ===========================================================================
#include "Public/cJSON.h"             //AI(W906-FRW-P8) 20260926: beforeApply 解析頁面值；放在產生檔之前（同 FileRW/TestIF_File_SetUp.cpp:27 的理由）
#include "WebBridge/JsonWriter.h"     //AI(W906-FRW-P8) 20260926: extraJson（逐通道溫控 ComboBox 的版面）
#include "FileRW/HSys.gen.inc"

#include <cmath>                     //AI(W906-FRW-S90) 20260926: std::fabs（EP 四鍵多寫者規則）
#include <cstdio>
#include <cstdlib>                    //AI(W906-FRW-S90) 20260926: std::atof

#include "FileRW/_EditPage.h"

#include <cstring>                    //AI(W906-FRW-P8) 20260926: memset（golden MachineTypeUtility.cpp GetStaTable_HeaterInsOpt）

// ===========================================================================
// //AI(W906-FRW-P8) 20260926: golden V912 逐通道溫控器廠牌表（EN_HEATER_SHEET=1）的本體。宣告與範圍說明在 FileRW/HSys_Heater.h 檔頭。
//   (1) golden HandlerSys.cpp:32-60   g_vecHeaterTypeIdxForShow／兩張對照表／TypeIdxToShowIdx／ShowIdxToTypeIdx／HeaterInsOpt_Read
//   (2) golden MachineTypeUtility.cpp:15-295（整檔逐字；只有 #include 不同）
//   (3) 移植樹專用：W906_HeaterInsProxyName／W906_HeaterInsCreateProxies（golden 建構子 :70-112 的 C 路版）
//   Jimmy 翻溫控底層時，(1)(2) 照 golden 搬到根目錄 MachineTypeUtility.cpp（ht9045_globals）與 HandlerSys 對應處；(3) 留在這裡。
// ===========================================================================
// ---- (1) golden HandlerSys.cpp:32-60 ----
std::vector<int>   g_vecHeaterTypeIdxForShow;
std::map<int, int> g_mapHeaterTypeIdxToHeaterShowIdx;
std::map<int, int> g_mapHeaterShowIdxToHeaterTypeIdx;

int TypeIdxToShowIdx(int iTypeIdx)
{
    std::map<int, int>::iterator it = g_mapHeaterTypeIdxToHeaterShowIdx.find(iTypeIdx);
    if(it == g_mapHeaterTypeIdxToHeaterShowIdx.end()) return INVALID_INT_VAL_NEG;
    return it->second;
}
int ShowIdxToTypeIdx(int iShowIdx)
{
    std::map<int, int>::iterator it = g_mapHeaterShowIdxToHeaterTypeIdx.find(iShowIdx);
    if(it == g_mapHeaterShowIdxToHeaterTypeIdx.end()) return INVALID_INT_VAL_NEG;
    return it->second;
}
// golden：缺鍵時 CheckAndReadIniDataGeneral 會把 -9999 寫進 Gerneral.ini [TempCtrl]（71 鍵），記憶體再回退成 HEATER_CTRL_TYPE。
//   golden 912 在 HandlerSys 開頁（LoaderSystemSet :265）與溫控第一次執行（bthermo.cpp DoThermo:1173）都呼叫；移植樹只有開頁這一處
//   （DoThermo 那一處屬溫控底層，列給 Jimmy）。
void HeaterInsOpt_Read()
{
    int iHeaterInsOpt_Old = CheckAndReadIniDataGeneral("TempCtrl","HEATER_CTRL_TYPE",KT4H);
    for(int iTypeIdx=0; iTypeIdx<eHeaterType_Count; ++iTypeIdx)
    {
        g_tHeaterInsInfo[iTypeIdx].m_iHeaterInsOpt =
            CheckAndReadIniDataGeneral("TempCtrl", g_tHeaterInsInfo[iTypeIdx].m_asSaveName, INVALID_INT_VAL_NEG);
        if(INVALID_INT_VAL_NEG==g_tHeaterInsInfo[iTypeIdx].GetHeaterInsOpt())
            g_tHeaterInsInfo[iTypeIdx].m_iHeaterInsOpt = iHeaterInsOpt_Old;     //回退 HEATER_CTRL_TYPE(=rgHeaterType 設定值)，非固定 KT4H；其他機型進版沿用原廠牌
    }
}

// ---- (2) golden MachineTypeUtility.cpp:15-295 ----
#if EN_HEATER_SHEET
bool g_bGetStaTable_HeaterInsOpt_Already = false;
int  g_StaTable_HeaterInsOpt[eHeaterType_Count];

AnsiString g_HeaterInsOptStr[eHeaterInsOpt_Count] = {
                                                        "TC401",            //0 = TC401
                                                        "Panasonic KT4H",   //1 = KT4H
                                                        "Omron E5DC",       //2 = E5DC
                                                        "No Heater",        //3 = NoHeater
                                                        "DTK4848",          //4 = DTK4848
                                                    };

// per-channel heater brand table (replaces single global TC401HeaterControl).
// occupy = show a combobox for this channel on the Heater setting page.
// NOTE(RogerYang): index 55..70 occupy values are a draft - confirm per machine.
THeaterInsInfo g_tHeaterInsInfo[eHeaterType_Count]={
    { THeaterInsInfo(true , "HotPlate1",  "HeaterInsOpt_HotPlate1",   DEFAULT_HEATER_INS_OPT) },  //0 HotPlate1
    { THeaterInsInfo(true , "HotPlate2",  "HeaterInsOpt_HotPlate2",   DEFAULT_HEATER_INS_OPT) },  //1 HotPlate2
    { THeaterInsInfo(true , "Shuttle1",   "HeaterInsOpt_Shuttle1",    DEFAULT_HEATER_INS_OPT) },  //2 Shuttle1
    { THeaterInsInfo(true , "Shuttle2",   "HeaterInsOpt_Shuttle2",    DEFAULT_HEATER_INS_OPT) },  //3 Shuttle2
    { THeaterInsInfo(true , "Head1",      "HeaterInsOpt_Head1",       DEFAULT_HEATER_INS_OPT) },  //4 Head1
    { THeaterInsInfo(true , "Head2",      "HeaterInsOpt_Head2",       DEFAULT_HEATER_INS_OPT) },  //5 Head2
    { THeaterInsInfo(true , "Head3",      "HeaterInsOpt_Head3",       DEFAULT_HEATER_INS_OPT) },  //6 Head3
    { THeaterInsInfo(true , "Head4",      "HeaterInsOpt_Head4",       DEFAULT_HEATER_INS_OPT) },  //7 Head4
    { THeaterInsInfo(true , "Socket",     "HeaterInsOpt_Socket",      DEFAULT_HEATER_INS_OPT) },  //8 Socket
    { THeaterInsInfo(true , "Chamber",    "HeaterInsOpt_Chamber",     DEFAULT_HEATER_INS_OPT) },  //9 Chamber
    { THeaterInsInfo(true , "CCD",        "HeaterInsOpt_CCD",         DEFAULT_HEATER_INS_OPT) },  //10 CCD
    { THeaterInsInfo(false, "Aa1",        "HeaterInsOpt_Aa1",         DEFAULT_HEATER_INS_OPT) },  //11 Aa1
    { THeaterInsInfo(false, "Ab1",        "HeaterInsOpt_Ab1",         DEFAULT_HEATER_INS_OPT) },  //12 Ab1
    { THeaterInsInfo(false, "Ac1",        "HeaterInsOpt_Ac1",         DEFAULT_HEATER_INS_OPT) },  //13 Ac1
    { THeaterInsInfo(false, "Ad1",        "HeaterInsOpt_Ad1",         DEFAULT_HEATER_INS_OPT) },  //14 Ad1
    { THeaterInsInfo(false, "Ba1",        "HeaterInsOpt_Ba1",         DEFAULT_HEATER_INS_OPT) },  //15 Ba1
    { THeaterInsInfo(false, "Bb1",        "HeaterInsOpt_Bb1",         DEFAULT_HEATER_INS_OPT) },  //16 Bb1
    { THeaterInsInfo(false, "Bc1",        "HeaterInsOpt_Bc1",         DEFAULT_HEATER_INS_OPT) },  //17 Bc1
    { THeaterInsInfo(false, "Bd1",        "HeaterInsOpt_Bd1",         DEFAULT_HEATER_INS_OPT) },  //18 Bd1
    { THeaterInsInfo(false, "Aa2",        "HeaterInsOpt_Aa2",         DEFAULT_HEATER_INS_OPT) },  //19 Aa2
    { THeaterInsInfo(false, "Ab2",        "HeaterInsOpt_Ab2",         DEFAULT_HEATER_INS_OPT) },  //20 Ab2
    { THeaterInsInfo(false, "Ac2",        "HeaterInsOpt_Ac2",         DEFAULT_HEATER_INS_OPT) },  //21 Ac2
    { THeaterInsInfo(false, "Ad2",        "HeaterInsOpt_Ad2",         DEFAULT_HEATER_INS_OPT) },  //22 Ad2
    { THeaterInsInfo(false, "Ba2",        "HeaterInsOpt_Ba2",         DEFAULT_HEATER_INS_OPT) },  //23 Ba2
    { THeaterInsInfo(false, "Bb2",        "HeaterInsOpt_Bb2",         DEFAULT_HEATER_INS_OPT) },  //24 Bb2
    { THeaterInsInfo(false, "Bc2",        "HeaterInsOpt_Bc2",         DEFAULT_HEATER_INS_OPT) },  //25 Bc2
    { THeaterInsInfo(false, "Bd2",        "HeaterInsOpt_Bd2",         DEFAULT_HEATER_INS_OPT) },  //26 Bd2
    { THeaterInsInfo(true , "HeatGun1",   "HeaterInsOpt_HeatGun1",    DEFAULT_HEATER_INS_OPT) },  //27 HeatGun1
    { THeaterInsInfo(true , "HeatGun2",   "HeaterInsOpt_HeatGun2",    DEFAULT_HEATER_INS_OPT) },  //28 HeatGun2
    { THeaterInsInfo(true , "DUT1",       "HeaterInsOpt_DUT1",        DEFAULT_HEATER_INS_OPT) },  //29 DUT1
    { THeaterInsInfo(true , "DUT2",       "HeaterInsOpt_DUT2",        DEFAULT_HEATER_INS_OPT) },  //30 DUT2
    { THeaterInsInfo(true , "DUT3",       "HeaterInsOpt_DUT3",        DEFAULT_HEATER_INS_OPT) },  //31 DUT3
    { THeaterInsInfo(true , "DUT4",       "HeaterInsOpt_DUT4",        DEFAULT_HEATER_INS_OPT) },  //32 DUT4
    { THeaterInsInfo(false, "Ae1",        "HeaterInsOpt_Ae1",         DEFAULT_HEATER_INS_OPT) },  //33 Ae1
    { THeaterInsInfo(false, "Af1",        "HeaterInsOpt_Af1",         DEFAULT_HEATER_INS_OPT) },  //34 Af1
    { THeaterInsInfo(false, "Ag1",        "HeaterInsOpt_Ag1",         DEFAULT_HEATER_INS_OPT) },  //35 Ag1
    { THeaterInsInfo(false, "Ah1",        "HeaterInsOpt_Ah1",         DEFAULT_HEATER_INS_OPT) },  //36 Ah1
    { THeaterInsInfo(false, "Be1",        "HeaterInsOpt_Be1",         DEFAULT_HEATER_INS_OPT) },  //37 Be1
    { THeaterInsInfo(false, "Bf1",        "HeaterInsOpt_Bf1",         DEFAULT_HEATER_INS_OPT) },  //38 Bf1
    { THeaterInsInfo(false, "Bg1",        "HeaterInsOpt_Bg1",         DEFAULT_HEATER_INS_OPT) },  //39 Bg1
    { THeaterInsInfo(false, "Bh1",        "HeaterInsOpt_Bh1",         DEFAULT_HEATER_INS_OPT) },  //40 Bh1
    { THeaterInsInfo(false, "Ae2",        "HeaterInsOpt_Ae2",         DEFAULT_HEATER_INS_OPT) },  //41 Ae2
    { THeaterInsInfo(false, "Af2",        "HeaterInsOpt_Af2",         DEFAULT_HEATER_INS_OPT) },  //42 Af2
    { THeaterInsInfo(false, "Ag2",        "HeaterInsOpt_Ag2",         DEFAULT_HEATER_INS_OPT) },  //43 Ag2
    { THeaterInsInfo(false, "Ah2",        "HeaterInsOpt_Ah2",         DEFAULT_HEATER_INS_OPT) },  //44 Ah2
    { THeaterInsInfo(false, "Be2",        "HeaterInsOpt_Be2",         DEFAULT_HEATER_INS_OPT) },  //45 Be2
    { THeaterInsInfo(false, "Bf2",        "HeaterInsOpt_Bf2",         DEFAULT_HEATER_INS_OPT) },  //46 Bf2
    { THeaterInsInfo(false, "Bg2",        "HeaterInsOpt_Bg2",         DEFAULT_HEATER_INS_OPT) },  //47 Bg2
    { THeaterInsInfo(false, "Bh2",        "HeaterInsOpt_Bh2",         DEFAULT_HEATER_INS_OPT) },  //48 Bh2
    { THeaterInsInfo(true , "2D",         "HeaterInsOpt_2D",          DEFAULT_HEATER_INS_OPT) },  //49 2D
    { THeaterInsInfo(true , "LB",         "HeaterInsOpt_LB",          DEFAULT_HEATER_INS_OPT) },  //50 LB
    { THeaterInsInfo(true , "IndexESD",   "HeaterInsOpt_IndexESD",    DEFAULT_HEATER_INS_OPT) },  //51 IndexESD
    { THeaterInsInfo(true , "CCD_2",      "HeaterInsOpt_CCD_2",       DEFAULT_HEATER_INS_OPT) },  //52 CCD_2
    { THeaterInsInfo(true , "ATCHotAir1", "HeaterInsOpt_ATCHotAir1",  DEFAULT_HEATER_INS_OPT) },  //53 ATCHotAir1
    { THeaterInsInfo(true , "ATCHotAir2", "HeaterInsOpt_ATCHotAir2",  DEFAULT_HEATER_INS_OPT) },  //54 ATCHotAir2
    { THeaterInsInfo(false , "OutSht1",    "HeaterInsOpt_OutSht1",     DEFAULT_HEATER_INS_OPT) },  //55 OutSht1
    { THeaterInsInfo(false , "OutSht2",    "HeaterInsOpt_OutSht2",     DEFAULT_HEATER_INS_OPT) },  //56 OutSht2
    { THeaterInsInfo(false , "Base1",      "HeaterInsOpt_Base1",       DEFAULT_HEATER_INS_OPT) },  //57 Base1
    { THeaterInsInfo(false , "Base2",      "HeaterInsOpt_Base2",       DEFAULT_HEATER_INS_OPT) },  //58 Base2
    { THeaterInsInfo(false , "Base3",      "HeaterInsOpt_Base3",       DEFAULT_HEATER_INS_OPT) },  //59 Base3
    { THeaterInsInfo(false , "Base4",      "HeaterInsOpt_Base4",       DEFAULT_HEATER_INS_OPT) },  //60 Base4
    { THeaterInsInfo(false , "Base5",      "HeaterInsOpt_Base5",       DEFAULT_HEATER_INS_OPT) },  //61 Base5
    { THeaterInsInfo(false , "Base6",      "HeaterInsOpt_Base6",       DEFAULT_HEATER_INS_OPT) },  //62 Base6
    { THeaterInsInfo(false , "HotPlate3",  "HeaterInsOpt_HotPlate3",   DEFAULT_HEATER_INS_OPT) },  //63 HotPlate3
    { THeaterInsInfo(false , "HotPlate4",  "HeaterInsOpt_HotPlate4",   DEFAULT_HEATER_INS_OPT) },  //64 HotPlate4
    { THeaterInsInfo(false , "Shuttle3",   "HeaterInsOpt_Shuttle3",    DEFAULT_HEATER_INS_OPT) },  //65 Shuttle3
    { THeaterInsInfo(false , "Shuttle4",   "HeaterInsOpt_Shuttle4",    DEFAULT_HEATER_INS_OPT) },  //66 Shuttle4
    { THeaterInsInfo(false , "Door1",      "HeaterInsOpt_Door1",       DEFAULT_HEATER_INS_OPT) },  //67 Door1
    { THeaterInsInfo(false , "Door2",      "HeaterInsOpt_Door2",       DEFAULT_HEATER_INS_OPT) },  //68 Door2
    { THeaterInsInfo(false , "LBUp",       "HeaterInsOpt_LBUp",        DEFAULT_HEATER_INS_OPT) },  //69 LBUp
    { THeaterInsInfo(false , "LBDown",     "HeaterInsOpt_LBDown",      DEFAULT_HEATER_INS_OPT) },  //70 LBDown
};
#endif
//---------------------------------------------------------------------------
#if !EN_HEATER_SHEET
extern int TC401HeaterControl;
#endif
//---------------------------------------------------------------------------
bool IsValEqual_HeaterInsOpt(int iHeaterTypeIdx, int iHeaterInsOpt_Cmp)
{
    if(iHeaterInsOpt_Cmp<0 || iHeaterInsOpt_Cmp>=eHeaterInsOpt_Count)
        return false;
#if !EN_HEATER_SHEET
    return (iHeaterInsOpt_Cmp==TC401HeaterControl);
#else
    if(iHeaterTypeIdx<0 || iHeaterTypeIdx>=eHeaterType_Count)
        return false;
    return (iHeaterInsOpt_Cmp==g_tHeaterInsInfo[iHeaterTypeIdx].GetHeaterInsOpt());
#endif
}
//---------------------------------------------------------------------------
// machine-level NoHeater: always judged by global rgHeaterType(=TC401HeaterControl),
// independent of EN_HEATER_SHEET / per-channel g_tHeaterInsInfo table.
bool IsNoHeaterMachine()                                                        //RogerYang 20260630 : Add NoHeater 判斷
{
    return (TC401HeaterControl==NoHeater);
}
//---------------------------------------------------------------------------
bool IsAllValEqual_HeaterInsOpt(int iHeaterInsOpt_Cmp, bool bJustForShowItem, int *piHeaterInsOpt_AllSame)
{
    if(iHeaterInsOpt_Cmp<0 || iHeaterInsOpt_Cmp>=eHeaterInsOpt_Count)
        return false;
#if !EN_HEATER_SHEET
    return (iHeaterInsOpt_Cmp==TC401HeaterControl);
#else
    bool bIsAllSame = true;
    int  iHeaterInsOpt_Tmp = INVALID_INT_VAL_NEG;
    for(int i = 0; i<eHeaterType_Count; ++i)
    {
        if(bJustForShowItem && !g_tHeaterInsInfo[i].GetOccupy())
            continue;
        if(INVALID_INT_VAL_NEG == iHeaterInsOpt_Tmp)
            iHeaterInsOpt_Tmp = g_tHeaterInsInfo[i].GetHeaterInsOpt();
        if(iHeaterInsOpt_Tmp != g_tHeaterInsInfo[i].GetHeaterInsOpt())
        {
            bIsAllSame = false;
            break;
        }
    }
    if(bIsAllSame)
    {
        if(NULL != piHeaterInsOpt_AllSame)
            *piHeaterInsOpt_AllSame = iHeaterInsOpt_Tmp;
    }
    return (bIsAllSame && iHeaterInsOpt_Cmp == iHeaterInsOpt_Tmp);
#endif
}
//---------------------------------------------------------------------------
int GetFirstHeaterInsOpt(bool bJustForShowItem)
{
#if !EN_HEATER_SHEET
    return TC401HeaterControl;
#else
    int iTypeIdx = -1;
    if(!bJustForShowItem)
        iTypeIdx = 0;
    else
    {
        if(g_vecHeaterTypeIdxForShow.size()>0)
            iTypeIdx = g_vecHeaterTypeIdxForShow[0];
        else
            iTypeIdx = -1;
    }
    if(0<=iTypeIdx && iTypeIdx<eHeaterType_Count)
        return g_tHeaterInsInfo[iTypeIdx].GetHeaterInsOpt();
    else
        return -1;
#endif
}
//---------------------------------------------------------------------------
bool IsAllSame_HeaterInsOpt(bool bJustForShowItem, int *piFirstHeaterInsOpt)
{
#if !EN_HEATER_SHEET
    if(NULL != piFirstHeaterInsOpt)
        *piFirstHeaterInsOpt = TC401HeaterControl;
    return true;
#else
    int iInsOpt = GetFirstHeaterInsOpt(bJustForShowItem);
    if(NULL != piFirstHeaterInsOpt)
        *piFirstHeaterInsOpt = iInsOpt;
    if(0<=iInsOpt && iInsOpt<eHeaterInsOpt_Count)
        return IsAllValEqual_HeaterInsOpt(iInsOpt, bJustForShowItem);
    else
        return false;
#endif
}
//---------------------------------------------------------------------------
bool GetStaTable_HeaterInsOpt(int *pStaTable, bool bJustForShowItem)
{
    if(NULL==pStaTable)
        return false;
    int  iHeaterInsOpt = 0;
    bool bStaAllEle = true;
    memset(pStaTable, 0, sizeof(int)*eHeaterType_Count);
#if !EN_HEATER_SHEET
    iHeaterInsOpt = TC401HeaterControl;
    if(iHeaterInsOpt<0 || iHeaterInsOpt>=eHeaterInsOpt_Count)
        bStaAllEle = false;
    else
        pStaTable[iHeaterInsOpt] = eHeaterType_Count;
#else
    for(int i = 0; i<eHeaterType_Count; ++i)
    {
        if(bJustForShowItem && !g_tHeaterInsInfo[i].GetOccupy())
            continue;
        iHeaterInsOpt = g_tHeaterInsInfo[i].GetHeaterInsOpt();
        if(iHeaterInsOpt<0 || iHeaterInsOpt>=eHeaterInsOpt_Count)
        {
            bStaAllEle = false;
            continue;
        }
        pStaTable[iHeaterInsOpt]++;
    }
#endif
    return bStaAllEle;
}
#if EN_HEATER_SHEET
//---------------------------------------------------------------------------
bool IsExistVal_HeaterInsOpt(int iHeaterInsOpt_Cmp, bool bJustForShowItem)
{
    if(iHeaterInsOpt_Cmp<0 || iHeaterInsOpt_Cmp>=eHeaterInsOpt_Count)
        return false;
    if(!g_bGetStaTable_HeaterInsOpt_Already)
    {
        GetStaTable_HeaterInsOpt(g_StaTable_HeaterInsOpt, bJustForShowItem);  //fix: arg order (int*, bool)
        g_bGetStaTable_HeaterInsOpt_Already = true;
    }
    return (g_StaTable_HeaterInsOpt[iHeaterInsOpt_Cmp]>0);
}
//---------------------------------------------------------------------------
bool GetCtrlItemVisProp(int iHeaterTypeIdx)
{
    if(iHeaterTypeIdx<0 || iHeaterTypeIdx>=eHeaterType_Count)
        return false;
    if(!g_tHeaterInsInfo[iHeaterTypeIdx].GetOccupy())
        return false;

    switch(iHeaterTypeIdx)
    {
        case tcHotPlate1:   //{   return (HotPlateForm.iPlateSelect==1 || HotPlateForm.iPlateSelect==3);  }
        case tcHotPlate2:   //{   return (HotPlateForm.iPlateSelect==2 || HotPlateForm.iPlateSelect==3);  }
        case tcShuttle1:
        case tcShuttle2:        break;
        case tcHead1:
        case tcHead2:
        case tcHead3:
        case tcHead4:
        {
            if(USE_16_HEATER==eht16Heater       ||
               USE_16_HEATER==eht16HeaterEJ1N   ||
               USE_16_HEATER==eht32HeaterEJ1N   ||
               USE_16_HEATER==eht32HeaterKT4H   ||
               USE_16_HEATER==eht16HeaterDTME08 ||
               USE_16_HEATER==eht32HeaterDTME08)
                return true;
            else
                return false;
        }
        case tcSocket:      //{   return (iSocketBaseTempCount==eDut1ea); }
        case tcChamber:         break;
        case tcCCD:         {   return (RTC_TemperNumber>=1);   }
        case tcCCD_2:       {   return (RTC_TemperNumber>=2);   }
        case tcHeatGun1:
        case tcHeatGun2:    {   return (INSTALL_HEAT_GUN>0);  }
        case tcDUT1:
        case tcDUT2:
        case tcDUT3:
        case tcDUT4:
        {
            if(iSocketBaseTempCount==eDut2ea && (iHeaterTypeIdx==tcDUT1 || iHeaterTypeIdx==tcDUT2))
                return true;
            else if(iSocketBaseTempCount==eDut4ea && (iHeaterTypeIdx==tcDUT1 || iHeaterTypeIdx==tcDUT2 || iHeaterTypeIdx==tcDUT3 || iHeaterTypeIdx==tcDUT4))
                return true;
            else
                return false;
        }
        case tc2D:          {   return (CCD2_TEMPER>0);         }
        case tcLB:          {   return (LB_TEMP>0);             }
        case tcIndexESD:    {   return (Index_ESDAir>0);        }
        case tcATCHotAir1:
        case tcATCHotAir2:  {   return (INSTALL_ATC_HEAT_GUN>0);}
    }
    return true;   //fix: default visible for occupied channels (also covers 906-only channels)
}
#endif
//---------------------------------------------------------------------------

// ---- (3) 移植樹專用（golden 沒有）----
// 替身名稱＝HTML 元件 id：lb<m_asSaveName>／cb<m_asSaveName>（例 cbHeaterInsOpt_HotPlate1）。golden 的動態元件沒有 Name，
//   這裡用存檔鍵名命名，頁面、探針、ini 三邊一看就對得上。
AnsiString W906_HeaterInsProxyName(int iTypeIdx, bool bCombo)
{
    if(iTypeIdx<0 || iTypeIdx>=eHeaterType_Count) return AnsiString("");
    return AnsiString(bCombo ? "cb" : "lb") + g_tHeaterInsInfo[iTypeIdx].GetSaveName();
}

// golden THandlerSystem 建構子 HandlerSys.cpp:70-112（EN_HEATER_SHEET=1）的 C 路版，逐句對照：
//   :71-87  顯示序對照表 —— 逐字。
//   :92-93  new TLabel(this)／new TComboBox(this) → filerw::EL<>（具名替身，同一個名稱永遠同一個物件 ⇒ 重跑冪等）。
//   :95-96  Parent = grpHeater → 父子表（ELEditable 看祖先 Enabled／Visible，同 golden 容器停用連帶子元件）。
//   :97-98  Width／Height：vclcompat TControl 沒有這兩個欄位 → 頁面照 ExtraJson 的 size 排（同一組常數）。
//   :99-102 Top／Left —— 逐字（TControl::Left／Top 只存值）。
//   :104-106 Items／ItemIndex —— Items 先清掉（重跑冪等）；ItemIndex 用 ELComboIndex（VCL：超出清單＝-1）。
//   :107    OnClick = cbHeaterInsOptChange_Base：golden 本體（:124-128）只有 dynamic_cast 就 return，沒有動作 → 不接。
//   :108-109 Tag、SetCtrlItemProp(GetCtrlItemVisProp(...)) —— 逐字（可見與否在開機時決定，golden 同：建構子只跑一次）。
void W906_HeaterInsCreateProxies()
{
#if EN_HEATER_SHEET
    g_vecHeaterTypeIdxForShow.clear();
    g_mapHeaterTypeIdxToHeaterShowIdx.clear();
    g_mapHeaterShowIdxToHeaterTypeIdx.clear();
    {
        int iShowCount = 0;
        for(int i = 0; i<eHeaterType_Count; ++i)
        {
            if(g_tHeaterInsInfo[i].GetOccupy())
            {
                g_mapHeaterTypeIdxToHeaterShowIdx[i] = iShowCount;
                g_mapHeaterShowIdxToHeaterTypeIdx[iShowCount] = i;
                g_vecHeaterTypeIdxForShow.push_back(i);
                iShowCount++;
            }
            else
                g_mapHeaterTypeIdxToHeaterShowIdx[i] = INVALID_INT_VAL_NEG;
        }
        for(int i = 0; i<(int)g_vecHeaterTypeIdxForShow.size(); ++i)
        {
            int iTypeIdx = g_vecHeaterTypeIdxForShow[i];
            int iHorIdx  = i / COUNT_OF_ITEM_IN_COL;
            int iVerIdx  = i % COUNT_OF_ITEM_IN_COL;
            const AnsiString asLb = W906_HeaterInsProxyName(iTypeIdx, false);
            const AnsiString asCb = W906_HeaterInsProxyName(iTypeIdx, true);
            TLabel    *pLb = filerw::EL<TLabel>("THandlerSystem", asLb.c_str());     // golden :92 new TLabel(this)
            TComboBox *pCb = filerw::EL<TComboBox>("THandlerSystem", asCb.c_str());  // golden :93 new TComboBox(this)
            {                                                                        // golden :95-96 Parent = grpHeater
                const char* const pr[2][2] = { { asLb.c_str(), "grpHeater" }, { asCb.c_str(), "grpHeater" } };
                filerw::ELSetParents("THandlerSystem", pr, 2);
            }
            pLb->Top  = TOP_OF_LB + iVerIdx*(HEIGHT_OF_LB_HEATER + GAP_OF_VER);
            pCb->Top  = TOP_OF_CB + iVerIdx*(HEIGHT_OF_CB_HEATER + GAP_OF_VER);
            pLb->Left = LEFT_OF_1ST + iHorIdx*(WIDTH_OF_LB_HEATER + GAP_OF_HOR_IN_SET + WIDTH_OF_CB_HEATER + GAP_OF_HOR_OUT_SET);
            pCb->Left = pLb->Left + (WIDTH_OF_LB_HEATER + GAP_OF_HOR_IN_SET);
            pLb->Caption = g_tHeaterInsInfo[iTypeIdx].GetShowName();
            pCb->Items->Clear();
            for(int j = 0; j<eHeaterInsOpt_Count; ++j)
                pCb->Items->Add(g_HeaterInsOptStr[j]);
            filerw::ELComboIndex(pCb, g_tHeaterInsInfo[iTypeIdx].GetHeaterInsOpt());   // golden :106 pCb->ItemIndex = ...GetHeaterInsOpt()
            pCb->Tag = i;
            g_tHeaterInsInfo[iTypeIdx].SetCtrlItemProp(GetCtrlItemVisProp(iTypeIdx), pLb, pCb);
        }
    }
#endif
}


// ---------------------------------------------------------------------------
// golden TfMain::TfMain（main.cpp）建構子裡讀 Gerneral.ini 的兩段，逐行翻譯。
// Steven 團隊 20260925：移植樹沒有 TfMain 建構子的這兩段（grep `EP_Install\s*=`／`ION_FAN_TYPE\s*=`／
// `InOutArmPickerUseMotor\s*=` 20260925：只有 cmydef.cpp 的初值、FileRW/HSys.cpp（A 形狀存檔）與 tests）⇒ 三個全域一直是
// cmydef.cpp 的初值（0／1／0），EP_MAXKPA… 也一直是 0.0；頁面顯示初值、存檔把初值寫回 EP_Install／INOUT_ARM_PICKER_USE_MOTOR／
// ION_FAN_TYPE（A 形狀回報的 sourceGap）。
// 呼叫時機：golden 的 SYSTEM_MODULAR HSys 是全域物件，建構子（ReadGeneralIni）在 TfMain 建構之前跑 ⇒ 整合者在
// LoadMachineConfig()（＝HSys.ReadGeneralIni()）之後、FileRW_HSys_Boot() 之前呼叫一次。
// golden 缺鍵時問 YES/NO：開機不能問 → 照 MessageBox(MB_YESNO) 的預設鈕（第一顆 YES；golden 沒給 MB_DEFBUTTON2）處理，
// printf 說明。INOUT_ARM_PICKER_USE_MOTOR 的題目 golden 自己已註解掉（:1801-1802），照 golden 直接寫 1。
// ---------------------------------------------------------------------------
void FileRW_HSys_ReadMainCtorKeys()
{
    int iTemp;
    // AI(W906-CTORKEYS) 20260925: golden 906 main.cpp:1693-1698（在上面 :1772 那段之前）。CUSTOMER_CODE 移植樹已由 ReadGeneralIni 讀好（database.cpp）。
    //   移植樹原本沒讀 ⇒ ZSafePos 一直是 Motor/mymotor.cpp:81 的初值 20（非 SCK 客戶 golden 預設 50，筆電 Gerneral.ini [In Arm] ZSafePos=50）。
    //   RULINGS_20260925 第 25 條（使用者「要翻」）。
    {
        extern int ZSafePos;                                                    // Motor/mymotor.cpp:81（Motor/mymotor.h:392）
        if(CUSTOMER_CODE==CC_SCK)                                                   //Steven 20220517 : 確認是不是這個造成機台歸零異常   // golden main.cpp:1695
            ZSafePos=CheckAndReadIniDataGeneral("In Arm", "ZSafePos", 20);
        else
            ZSafePos=CheckAndReadIniDataGeneral("In Arm", "ZSafePos", 50);        // golden main.cpp:1698
    }
//----------------------------------                                            // golden main.cpp:1772
    if(CheckIniData(asGeneralPath, "System", "EP_Install")==false)
    {
        iTemp=1;
        // golden :1776  if(Application->MessageBox("Machine have install Electrons-Press(EP)?", "EP", MB_YESNO | MB_TOPMOST)!=IDYES) iTemp=0;
        std::printf("FileRW HSys: Gerneral.ini [System] EP_Install missing -- golden main.cpp:1776 asks \"Machine have install Electrons-Press(EP)?\" "
                    "(YES/NO); boot cannot ask, MB_YESNO default button YES taken => EP_Install=1 written\n");
        WriteIniDataGeneral("System", "EP_Install", iTemp);
        EP_Install=iTemp;
    }
    else
    {
        EP_Install=CheckAndReadIniDataGeneral("System", "EP_Install", 0);
        if(EP_Install!=0)                                                       //Steven 20140524 : 配合統一由外面讀取
        {
            EP_MAXKPA=CheckAndReadIniDataGeneral("System" , "EP_MAXKPA" , 499.0);
            EP_MAXAFB=CheckAndReadIniDataGeneral("System" , "EP_MAXA"   , 5.013);
            EP_MINMPA=CheckAndReadIniDataGeneral("System" , "EP_MINMPA" , 0.001);                                       //JerryYang 20171023 (wei) add PA Min
            EP_MinAFB=CheckAndReadIniDataGeneral("System" , "EP_MINA_FeedBack", 0.908);

            EPDual_MAXKPA=CheckAndReadIniDataGeneral("System" , "EPDual_MAXKPA" , 899.0);                               //kevin 20200325 add dual force EP
            EPDual_MAXAFB=CheckAndReadIniDataGeneral("System" , "EPDual_MAXAFB"   , 4.905);
            EPDual_MINMPA=CheckAndReadIniDataGeneral("System" , "EPDual_MINMPA" , 0.001);
            EPDual_MinAFB=CheckAndReadIniDataGeneral("System" , "EPDual_MinAFB", 0.968);                                //kevin 20200325 add dual force EP
        }
    }
//----------------------------------                                            // golden main.cpp:1797
    if(CheckIniData(asGeneralPath, "System", "INOUT_ARM_PICKER_USE_MOTOR")==false)
    {
        iTemp=1;
//        if(Application->MessageBox("In/Out Arm ZA to ZH is use motor control?", "In/Out Arm", MB_YESNO | MB_TOPMOST) != IDYES)
//            iTemp=0;
        WriteIniDataGeneral("System", "INOUT_ARM_PICKER_USE_MOTOR", iTemp);
        InOutArmPickerUseMotor=iTemp;
    }
    else
    {
        InOutArmPickerUseMotor=CheckAndReadIniDataGeneral("System", "INOUT_ARM_PICKER_USE_MOTOR", 1);

        if(InOutArmPickerUseMotor==0)                                           //Steven 20240603 : 強制寫1
        {
            InOutArmPickerUseMotor=1;
            WriteIniDataGeneral("System", "INOUT_ARM_PICKER_USE_MOTOR", InOutArmPickerUseMotor);
        }
    }                                                                           // golden main.cpp:1815
    // golden :1817-1828 SUPPORT_2_EMPTY_EMPTY 不在這裡：golden 912 已由 ReadGeneralIni 讀（HandlerSys.cpp:179 的註解
    // 「RogerYang 20250823:搬去ReadGeneralIni()」；移植樹 database.cpp:547），ReadGeneralIni 的 CheckAndRead 先補了鍵，
    // main.cpp 那一段的題目在 golden 也不會再出現。

    if(CheckIniData(asGeneralPath, "System", "ION_FAN_TYPE")==false)            //Steven 20100226 : ION Fan    // golden main.cpp:2042
    {
        iTemp=0;
        // golden :2045  if(Application->MessageBox("Machine Using KEYENCE's ION FAN?", NULL, MB_YESNO | MB_TOPMOST)==IDYES) iTemp=1; else iTemp=0;
        iTemp=1;
        std::printf("FileRW HSys: Gerneral.ini [System] ION_FAN_TYPE missing -- golden main.cpp:2045 asks \"Machine Using KEYENCE's ION FAN?\" "
                    "(YES/NO); boot cannot ask, MB_YESNO default button YES taken => ION_FAN_TYPE=1 written\n");
        WriteIniDataGeneral("System", "ION_FAN_TYPE", iTemp);
        ION_FAN_TYPE=iTemp;
    }
    else
    {
        ION_FAN_TYPE=CheckAndReadIniDataGeneral("System", "ION_FAN_TYPE", 1);
    }                                                                           // golden main.cpp:2055
    // AI(W906-CTORKEYS) 20260925: golden 906 main.cpp:2008-2105 其餘的機台鍵（RULINGS_20260925 第 25 條「要翻」），逐段照翻、順序照 golden。
    // golden :2008 InitialSuperVisorPassword(CUSTOMER_CODE) 不在這裡：wb_serve 開機的 WebLogin_Boot()（Steven 20260924）已照 golden 呼叫。
    // golden :2009-2021 INDEX_SUCKER_TYPE —— 閘住，相依不存在：負壓機（INDEX_SUCKER_TYPE!=0，筆電這台 Gerneral.ini 就是 1）上，
    //   golden 靠 Index 吸嘴的真空／破真空「泵」ProcessIndexSuckDestroy1/2（golden 引擎 49 個呼叫點）與 bNeedCheck 讀取端，移植樹兩者都還是替身
    //   （NB2 預勘 docs/nb2_assist/RD5軟體_NB2預勘_D44泵_開機序列_翻譯佇列_20260925_031105.md §1、§3 P1）⇒ 先載入會讓 WAR1604 被 bIndexCheck1 永久關掉。
    //   解閘條件：泵＋bNeedCheck 讀取端翻好（要人在機台旁驗，會動 Index 真空 IO）。HandlerSys 頁（HandlerSys.cpp:174）照舊只把它讀進畫面。
    // AI(W906-FRW-Boot) 20260926: V912 對照（Steven 團隊；開機讀檔照 912，RULINGS_20260925 S37）。本函式逐句與 V912 main.cpp 比過
    //   （去掉空白與行尾註解後只差 asString 的宣告位置），906 與 912 這一段內容相同，不用改碼。V912 行號：ZSafePos :1743-1747、
    //   EP_Install :1773、INOUT_ARM_PICKER_USE_MOTOR :1798、ION_FAN_TYPE :2042、InitialSuperVisorPassword :2057（WebLogin_Boot 做）、
    //   INDEX_SUCKER_TYPE :2058-2070、IndexTimeSet :2072、ASEK15UsePW :2083、ASEK15PassWord :2094、EPuser／EPPass／ConfigPass :2105-2111、
    //   CC_GIGAS :2113-2117、SetupFileCheckList :2119-2120、bContaceTorque :2122、bUse_NewAutoCleanForm :2134、bUseNewCleanModeKit :2145、
    //   AGVModal :2156（golden 自己註解掉）。
    //   （20260927 更新：Jimmy IDXSUCK-3 ffd74fd8 已解開 GATE W906-CTORKEYS-SUCKER，下面照 golden 讀；以下是 20260926 的原紀錄）
    //   INDEX_SUCKER_TYPE 的閘照 Jimmy（212c8e1d）保留 —— 20260926 重查解閘條件只到一半：bNeedCheck 讀取端 033358a2 已接回真成員，
    //   泵 ProcessIndexSuckDestroy1/2 仍是替身（aTester_Front.cpp:207、atester_32Site.cpp:205、AutoClean/AutoClean.cpp:288 都回 true；
    //   真本體 forms/fIoSetView.h:658-659 標 GATE S-08／S-09）。⚠ 網頁 HandlerSys 存檔仍會即時設 INDEX_SUCKER_TYPE
    //   （HSys.gen.inc:3584，全樹唯一活的寫入點），待 Jimmy 決定（NB2 R50 Q8b-J2）。
    //   密碼類（ASEK15PassWord、EPuser、EPPass、sPassWord、sGigasFTPPassWord）只進記憶體；本檔的 printf 不印它們（S41）。
    if(CheckIniData(asGeneralPath, "System", "INDEX_SUCKER_TYPE")==false)       //jou 2010-05-19 start : 負壓   //AI(W906-IDXSUCK) 20260927: GATE W906-CTORKEYS-SUCKER 解開（原 #if 0 那一行與 #endif 拿掉、行數不變）—— 前提（泵與 bNeedCheck 讀取端都還是替身）不成立了：泵 33fd6570 照 golden 翻進 TfiosetviewShim，bNeedCheck 前後臂 0926 W2-A46、32-site 本顆
    {
#if 0   // GATE(W906-IDXSUCK-ASK) 鍵不存在時 golden 開機跳是／否框問「Index 真空產生器用負壓嗎」；wb_serve 開機時沒有網頁能回答 ⇒ 留著閘、INDEX_SUCKER_TYPE 維持預設 0（＝golden 答「否」）、不寫檔。鍵存在的機台（一般都有）走下面照 golden 讀
        if(Application->MessageBox("index sucker use negative press? (index真空產生器使用負壓系統?)", NULL, MB_YESNO | MB_TOPMOST)==IDYES)
            iTemp=1;
        else
            iTemp=0;
        WriteIniDataGeneral("System", "INDEX_SUCKER_TYPE", iTemp);
        INDEX_SUCKER_TYPE=iTemp;
#endif
    }
    else
    {
        INDEX_SUCKER_TYPE=CheckAndReadIniDataGeneral("System", "INDEX_SUCKER_TYPE", 0);
    }

    if(CheckIniData(asGeneralPath, "System", "IndexTimeSet")==false)            //kevin 20130408 Index Time add   // golden main.cpp:2023
    {
        iTemp=0;
        WriteIniDataGeneral("System", "IndexTimeSet", iTemp);
        bIndexTimeSet=iTemp;
    }
    else
    {
        bIndexTimeSet=CheckAndReadIniDataGeneral("System", "IndexTimeSet", 0);
    }
//----------------------------------
    if(CheckIniData(asGeneralPath, "VENDER", "ASEK15UsePW")==false)             //kevin 20130701   // golden main.cpp:2034
    {
        iTemp=0;
        WriteIniDataGeneral("VENDER", "ASEK15UsePW", iTemp);
        bASEK15UsePW=iTemp;
    }
    else
    {
        bASEK15UsePW=CheckAndReadIniDataGeneral("VENDER", "ASEK15UsePW", 0);
    }

    AnsiString asString;
    if(CheckIniData(asGeneralPath, "VENDER", "ASEK15PassWord")==false)          //kevin 20130701   // golden main.cpp:2045
    {
        iTemp=0;
        WriteIniDataGeneral("VENDER", "ASEK15PassWord", iTemp);
        ASEK15PassWord=iTemp;                                                   // golden 同：缺鍵時寫整數 0、密碼變 "0"（照翻）
    }
    else
    {
        asString="27025312";
        ASEK15PassWord=CheckAndReadIniDataGeneral("VENDER", "ASEK15PassWord", asString);
    }

    asString="HonPrec";                                                         // golden main.cpp:2057
    EPuser=CheckAndReadIniData(asGeneralPath, "VENDER", "EPuser", asString);
    asString="16943420";
    EPPass=CheckAndReadIniData(asGeneralPath, "VENDER", "EPPass", asString);
    asString="990418";
    sPassWord=CheckAndReadIniData(asGeneralPath, "VENDER", "ConfigPass", asString);                                     //kevin 20180411 password

    if(CUSTOMER_CODE==CC_GIGAS)                                                 //Isaac 20210128 : 為了讓Enable FTP不被鎖住   // golden main.cpp:2064
    {
        asString="16943420";
        sGigasFTPPassWord=CheckAndReadIniData(asGeneralPath, "VENDER", "GigasFTPPassWWord", asString);
    }

    asString="HisiATC_SetupCheckList_HT9045.dat";                               // golden main.cpp:2070
    asSetupFileCheckList=CheckAndReadIniDataGeneral("System",  "SetupFileCheckList", asString);                         //Ifor 20200914 add:Setup File Check List
    //---------kevin 20131217傳給台積 force------------------------------------------------------
    if(CheckIniData(asGeneralPath, "System", "bContaceTorque")==false)          // golden main.cpp:2073
    {
        iTemp=0;
        WriteIniDataGeneral("System", "bContaceTorque", iTemp);
        bContaceTorque=iTemp;
    }
    else
    {
        bContaceTorque=CheckAndReadIniDataGeneral("System", "bContaceTorque", 0);
    }
//-------------------------------------------------------------kevin 20141111 socketsensor-

    if(CheckIniData(asGeneralPath, "System", "bUse_NewAutoCleanForm")==false)   //20150507 使用Clean 模組   // golden main.cpp:2085
    {
        iTemp=0;
        WriteIniDataGeneral("System", "bUse_NewAutoCleanForm", iTemp);
        bUse_NewAutoCleanForm=iTemp;
    }
    else
    {
        bUse_NewAutoCleanForm=CheckAndReadIniDataGeneral("System", "bUse_NewAutoCleanForm", 0);
    }
//--------------------------------------------------------------------------------------------------------------------
    if(CheckIniData(asGeneralPath, "System", "bUseNewCleanModeKit")==false)     //kevin 20150701 使用autoclean模組 校正位置在 基座上 tray pin1   // golden main.cpp:2096
    {
        iTemp=0;
        WriteIniDataGeneral("System", "bUseNewCleanModeKit", iTemp);
        bUseNewCleanModeKit=iTemp;
    }
    else
    {
        bUseNewCleanModeKit=CheckAndReadIniDataGeneral("System", "bUseNewCleanModeKit", 0);
    }                                                                           // golden main.cpp:2105（:2107-2115 AGVModal golden 自己註解掉）
    { extern int ZSafePos;
      std::printf("FileRW HSys: golden TfMain ctor keys (906 main.cpp:1695-1698, :2023-2105): ZSafePos=%d IndexTimeSet=%d ASEK15UsePW=%d SetupFileCheckList=\"%s\" bContaceTorque=%d bUse_NewAutoCleanForm=%d bUseNewCleanModeKit=%d (INDEX_SUCKER_TYPE still gated, =%d)\n",
                  ZSafePos, (int)bIndexTimeSet, (int)bASEK15UsePW, asSetupFileCheckList.c_str(), (int)bContaceTorque,
                  (int)bUse_NewAutoCleanForm, (int)bUseNewCleanModeKit, INDEX_SUCKER_TYPE); }
    std::printf("FileRW HSys: golden TfMain ctor keys read (main.cpp:1772-1815, :2042-2055): EP_Install=%d InOutArmPickerUseMotor=%d ION_FAN_TYPE=%d\n",
                EP_Install, InOutArmPickerUseMotor, ION_FAN_TYPE);
}

// golden TfMain::FormShow main.cpp:9589-9594 的網頁版（見檔頭）。回 true ＝ 型號讀取失敗（golden 在這裡停機）。
static bool HS_ModelReadError()
{
    if(bHandlerModel==false)                                                    //jou 20200601 : GPIB 型號讀取失敗需Alarm,不應該回寫型號
    {
        // golden: MessageDlg("D:\\GPIB9045\\system\\general.ini \"Model\" read error!!", mtConfirmation, TMsgDlgButtons()<<mbOK, 0);
        //         Application->Terminate(); return;
        filerw::ELMessage("D:\\GPIB9045\\system\\general.ini \"Model\" read error!!",
                          "D:\\GPIB9045\\system\\general.ini 的 \"Model\" 讀取失敗（golden 會結束程式；網頁版不開頁資料、不存檔）");
        filerw::ELMark("model_read_error");
        return true;
    }
    return false;
}

namespace {
bool g_booted = false;
bool Booted() { return g_booted; }

// ---- 多寫者：Gerneral.ini [System] EP 四鍵（AI(W906-FRW-S90) 20260926，Steven 團隊）----
// Steven 20260926 14:4x 裁決（經 github-02）：比照 Setup.ContactForce（FileRW/ContactForce.cpp，S57），採「沒動過的欄位不把舊值蓋回去」。
// golden 對照（V912 HandlerSys.cpp）：開頁 LoaderSystemSet :481-484 用 CheckAndReadIniDataGeneral 把四鍵讀進 edMaxKpa／edMaxMpaFB／
//   edMinMpa／edtMinMpaFB；存檔 SaveSystemSet :976-979 **不看任何條件**把四個替身的字寫回；:1008-1013 EP_Install!=0 才把四鍵讀回
//   EP_MAXKPA／EP_MAXAFB／EP_MINMPA／EP_MinAFB 全域。另一個寫者是 golden ContactForce.cpp WriteFile :1357-1360（同四鍵、同寫法）。
//   golden 兩張表單都是「開頁讀、存檔寫畫面上的值」，沒有防互蓋；HandlerSystem 是 ShowModal、單一操作者，互蓋機會很小；網頁可以同時開
//   好幾個分頁，機會變大（golden 沒有的風險）。
// 偏離 golden 的地方（只有一種情況）：存檔時「這一欄頁面沒改過（＝開頁時的值）而檔案在開頁之後已經被別人改了」→ 用檔案現在的字串寫回
//   （等於不動），ack messages 說明。其餘情況照 golden：頁面改過 → 寫頁面值（後寫的贏；檔案也被改過就另外警告）；檔案沒被改 → 照 golden。
//   四鍵讀回全域（:1008-1013）與寫檔順序都不變。kHS_KeepNewerOverlap=false 就完全照 golden（寫頁面值）＋警告。
// 快照時機：只在開頁（editlist.get＝golden FormShow）與真的寫了檔之後。**不在 Reload**：答 NO／沒寫檔時 _EditPage.cpp:164 會 reload
//   把替身還原成檔案值，若那時重拍快照，頁面帶 YES 重送時「開頁時的值」就變成別人的新值，規則會失效。
const bool kHS_KeepNewerOverlap = true;
struct HSOverlap {
    const char* widget;
    const char* key;
    AnsiString openText;     // 開頁（golden FormShow 之後）替身的字
    AnsiString openFile;     // 開頁時檔案裡的字
    bool openHad;
};
HSOverlap g_hsOvl[4] = {
    {"edMaxKpa",    "EP_MAXKPA",        "", "", false},     // golden :481／:976
    {"edMaxMpaFB",  "EP_MAXA",          "", "", false},     // golden :482／:977
    {"edMinMpa",    "EP_MINMPA",        "", "", false},     // golden :483／:978
    {"edtMinMpaFB", "EP_MINA_FeedBack", "", "", false},     // golden :484／:979
};
const char* const kHSMissing = "<<W906-FRW-S90 missing>>";

// 純讀（不補鍵）：Gerneral.ini 是 write-through（vclcompat/IniFiles.cpp:299），讀到的是檔案現況
AnsiString HSGeneralRaw(const char* key, bool* had)
{
    const AnsiString v = ReadIniData(asGeneralPath, AnsiString("System"), AnsiString(key), AnsiString(kHSMissing));
    *had = (v != kHSMissing);
    return *had ? v : AnsiString("");
}

void HSTakeSnapshot()
{
    for (HSOverlap& o : g_hsOvl) {
        o.openText = EL<TEdit>("THandlerSystem", o.widget)->Text;
        o.openFile = HSGeneralRaw(o.key, &o.openHad);
    }
}

void HSApplyOverlapRule()
{
    for (HSOverlap& o : g_hsOvl) {
        bool hadNow = false;
        const AnsiString now = HSGeneralRaw(o.key, &hadNow);
        if (!o.openHad || !hadNow) continue;
        if (now == o.openFile || std::fabs(std::atof(now.c_str()) - std::atof(o.openFile.c_str())) < 1e-12) continue;   // 檔案沒被別人改
        TEdit* e = EL<TEdit>("THandlerSystem", o.widget);
        const bool touched = !(e->Text == o.openText);
        AnsiString en, zh;
        if (!touched && kHS_KeepNewerOverlap) {
            en.sprintf("Gerneral.ini [System] %s was changed by another page after this page was opened (%s -> %s); "
                       "this page did not change it, so the newer value %s is kept", o.key, o.openFile.c_str(), now.c_str(), now.c_str());
            zh.sprintf("Gerneral.ini [System] %s 在本頁開啟後被其他頁面改過（%s → %s）；本頁沒有改這一欄，保留較新的 %s，不把舊值蓋回去",
                       o.key, o.openFile.c_str(), now.c_str(), now.c_str());
            e->Text = now;
        } else if (!touched) {
            en.sprintf("Gerneral.ini [System] %s was changed by another page (%s -> %s) and is overwritten with the value shown when this page "
                       "was opened (%s), as golden does", o.key, o.openFile.c_str(), now.c_str(), e->Text.c_str());
            zh.sprintf("Gerneral.ini [System] %s 已被其他頁面改成 %s，本頁照 golden 寫回開頁時的 %s", o.key, now.c_str(), e->Text.c_str());
        } else {
            en.sprintf("Gerneral.ini [System] %s was changed by another page (%s -> %s) and also on this page (%s); this page's value is written "
                       "(last writer wins, as golden)", o.key, o.openFile.c_str(), now.c_str(), e->Text.c_str());
            zh.sprintf("Gerneral.ini [System] %s 其他頁面改成 %s、本頁也改成 %s：照 golden 寫本頁的值（後存的為準）",
                       o.key, now.c_str(), e->Text.c_str());
        }
        filerw::ELMessage(en, zh);
    }
}

// editlist.get：golden FormShow（型號錯時不跑）
void OpenPage()
{
    if (HS_ModelReadError()) return;
    HS_FormShow();
    HSTakeSnapshot();                                //AI(W906-FRW-S90) 20260926: EP 四鍵開頁快照（見上）
}

// editlist.save：golden 存檔鈕；真的寫了檔 → golden 離開鈕的資料半段（理由見檔頭）
void SaveFlow()
{
    if (HS_ModelReadError()) return;                 // golden 停機：走不到存檔
    HSApplyOverlapRule();                            //AI(W906-FRW-S90) 20260926: 在 golden SaveSystemSet 讀替身之前（見上）；答 NO 時沒寫檔、替身由 reload 還原
    HS_SaveBtnClick();
    if (filerw::ELMarked("SaveSystemSet:write")) {
        filerw::ELMark("ExitBtnClick(data half): HSys.ReadGeneralIni + SaveSafeDoorSet");
        HS_ExitBtnClick();
        HSTakeSnapshot();                            //AI(W906-FRW-S90) 20260926: 寫了檔 → 以剛寫好的值當新的基準
    }
}

// 沒寫檔時把替身還原成檔案值 —— golden 重開表單（FormShow）。型號錯時不跑（它會補寫 Gerneral.ini）。
void Reload()
{
    if (bHandlerModel == false) return;
    HS_FormShow();
}

// //AI(W906-FRW-P8) 20260926: PageDesc::beforeApply —— golden rgHeaterType 的 OnClick＝rgHeaterTypeClick（HandlerSys.cpp:1519，
//   DFM :2991）：使用者點選別的廠牌 → 71 個逐通道 ComboBox 全設成同一個廠牌（golden 同時立刻寫 [TempCtrl] 71 鍵與 HEATER_CTRL_TYPE）。
//   網頁送的是最後狀態：rgHeaterType 與伺服器端不同 ＝ 使用者點過 → 先照 VCL 設 ItemIndex（夾 -1..Count-1）再跑 golden 事件，
//   列入 handled（PageSave 不再套它）。之後頁面送來的逐通道下拉照通用規則套（使用者點完廠牌後又個別改的值留著；看不見的通道
//   頁面值不收，保留事件設的廠牌 —— 與 golden 相同：看不見的 ComboBox 使用者改不到，但事件照樣改它）。
//   //AI(W906-FRW-Q14) 20260927: [W906] 偏離 golden：Steven 20260927 Q14＝B（RULINGS_20260926 S136，todo ★ Q14）——
//   事件只改記憶體與畫面（golden :1531-1535：m_iHeaterInsOpt、各通道 ComboBox），不寫檔：tools/editlist/HSys.py 把 golden :1536
//   （各通道鍵）與 :1538（HEATER_CTRL_TYPE）兩個 WriteIniDataGeneral 換成空敘述。這兩組鍵只由整頁存檔 SaveSystemSet :782-793
//   （答 YES＝SaveSystemSet:write 之後）寫。寫的鍵與值跟 golden 按下即寫的相同：
//     * 各通道鍵：golden :1531 看得到的通道（TypeIdxToShowIdx≠-9999）寫 iOpt、其餘寫 -9999；SaveSystemSet :782-788 看得到的通道寫
//       該通道 ComboBox 的 ItemIndex（事件剛設成 iOpt；看得到的通道一定有 ComboBox —— W906_HeaterInsCreateProxies 逐一建）、其餘 -9999。
//     * HEATER_CTRL_TYPE：golden :1538 寫 iOpt；SaveSystemSet :789-793 全部同廠牌寫 iFirst（＝iOpt），否則寫 rgHeaterType（＝iOpt）。
//     * 點完廠牌又個別改某通道：golden 存檔時同樣寫 ComboBox 的值、蓋掉按下時寫的 → 存完兩邊檔案相同。
//     * 鍵的順序：開頁 LoaderSystemSet :262-278（HeaterInsOpt_Read）已把缺的鍵補寫成 -9999（HEATER_CTRL_TYPE 由 CheckAndReadIniDataGeneral
//       補），存檔時全部是改寫既有的鍵、不新增鍵 → 位元組與 golden 相同。
//   只有「沒存成」時不同：答 NO、套值失敗（_EditPage.cpp:148 reload）——golden 按下時已寫檔，這裡不寫；記憶體由 reload
//   （＝FormShow → LoaderSystemSet → HeaterInsOpt_Read）還原成檔案值。
//   ⚠ 點 A 再點回原廠牌：頁面最後值＝伺服器值 → 不重播（golden 兩次事件會把看不見的通道也設成原廠牌）。邊角差異，列在交件報告。
//   bHandlerModel==false（golden 停機）時不重播：存檔會被 SaveFlow 拒絕。
void BeforeApply(const std::string& widgetsJson, std::vector<std::string>* handled)
{
    if (bHandlerModel == false) return;
    cJSON* root = cJSON_Parse(widgetsJson.c_str());
    if (!root) return;
    const cJSON* rg = cJSON_GetObjectItemCaseSensitive(root, "rgHeaterType");
    const cJSON* idx = (rg && cJSON_IsObject(rg)) ? cJSON_GetObjectItemCaseSensitive(rg, "itemIndex") : nullptr;
    if (idx && cJSON_IsNumber(idx) && filerw::ELEditable("THandlerSystem", "rgHeaterType")) {
        TRadioGroup* r = EL<TRadioGroup>("THandlerSystem", "rgHeaterType");
        int v = idx->valueint;
        const int n = r->Items ? r->Items->Count : 0;                   // VCL TCustomRadioGroup.SetItemIndex 的夾限
        if (v >= n) v = n - 1;
        if (v < -1) v = -1;
        if (v != r->ItemIndex) {                                        // 值有變才有 OnClick
            r->ItemIndex = v;
            filerw::ELMark("rgHeaterTypeClick (golden HandlerSys.cpp:1519, replayed before apply)");
            HS_rgHeaterTypeClick();
            handled->push_back("rgHeaterType");
        }
    }
    cJSON_Delete(root);
}

// //AI(W906-FRW-P8) 20260926: PageDesc::extraJson —— 逐通道溫控 ComboBox 是 golden 建構子動態建的（DFM 沒有、頁面產生器畫不出來），
//   頁面照這份在 grpHeater 裡建 Label／下拉（id＝替身名稱），之後的值／可見／可改走通用 proxies。
//   純讀：不寫檔。options＝golden g_HeaterInsOptStr；位置＝golden 建構子算的 Left／Top；size＝golden 版面常數。
std::string ExtraJson()
{
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("heater").BeginObject();
    w.Key("sheet").Bool(EN_HEATER_SHEET != 0);
    w.Key("golden").String("V912 HandlerSys.cpp:70-112 ctor / :262-278 LoaderSystemSet / :779-794 SaveSystemSet / :1519 rgHeaterTypeClick; "
                           "MachineTypeUtility.cpp g_tHeaterInsInfo");
    w.Key("options").BeginArray();
    for (int j = 0; j < eHeaterInsOpt_Count; ++j) w.String(g_HeaterInsOptStr[j].c_str());
    w.EndArray();
    w.Key("size").BeginObject();
    w.Key("lbW").Number((wb_int64)WIDTH_OF_LB_HEATER);
    w.Key("lbH").Number((wb_int64)HEIGHT_OF_LB_HEATER);
    w.Key("cbW").Number((wb_int64)WIDTH_OF_CB_HEATER);
    w.Key("cbH").Number((wb_int64)HEIGHT_OF_CB_HEATER);
    w.EndObject();
    w.Key("channels").BeginArray();
    for (int s = 0; s < (int)g_vecHeaterTypeIdxForShow.size(); ++s) {
        const int ti = g_vecHeaterTypeIdxForShow[s];
        if (ti < 0 || ti >= eHeaterType_Count) continue;
        TLabel* lb = g_tHeaterInsInfo[ti].GetCtrlItem_Lb();
        TComboBox* cb = g_tHeaterInsInfo[ti].GetCtrlItem_Cb();
        w.BeginObject();
        w.Key("typeIdx").Number((wb_int64)ti);
        w.Key("show").Number((wb_int64)s);
        w.Key("lb").String(W906_HeaterInsProxyName(ti, false).c_str());
        w.Key("cb").String(W906_HeaterInsProxyName(ti, true).c_str());
        w.Key("caption").String(g_tHeaterInsInfo[ti].GetShowName().c_str());
        w.Key("saveName").String(g_tHeaterInsInfo[ti].GetSaveName().c_str());
        w.Key("lbLeft").Number((wb_int64)(lb ? lb->Left : 0));
        w.Key("lbTop").Number((wb_int64)(lb ? lb->Top : 0));
        w.Key("cbLeft").Number((wb_int64)(cb ? cb->Left : 0));
        w.Key("cbTop").Number((wb_int64)(cb ? cb->Top : 0));
        w.Key("visible").Bool(g_tHeaterInsInfo[ti].GetShow());
        w.Key("value").Number((wb_int64)g_tHeaterInsInfo[ti].GetHeaterInsOpt());
        w.EndObject();
    }
    w.EndArray();
    w.Key("tc401HeaterControl").Number((wb_int64)TC401HeaterControl);
    w.Key("underlying").String("temperature control (bthermo/rs232/cConfiguration/OmronEJ1N) still uses the single brand "
                               "TC401HeaterControl = [TempCtrl] HEATER_CTRL_TYPE (906 underlying, RULINGS_20260926 #26); "
                               "per-channel brand dispatch is V912 underlying work (Jimmy)");
    w.EndObject();
    w.EndObject();
    return w.Str();
}

const filerw::PageDesc kPage = {
    "HSys", "THandlerSystem", "HW.HandlerSys.html",
    nullptr, nullptr, 0,
    kHS_SaveReads, (int)(sizeof(kHS_SaveReads) / sizeof(kHS_SaveReads[0])),
    &OpenPage, &SaveFlow, "SaveSystemSet:write", &Reload, &Booted,
    &BeforeApply, &ExtraJson,   //AI(W906-FRW-P8) 20260926: rgHeaterTypeClick 重播（Q14＝B：只改記憶體，20260927）／逐通道溫控 ComboBox 版面
};
filerw::PageRegistrar g_reg(&kPage);
}  // namespace

// golden THandlerSystem 建構（HT9045.cpp:210）：DFM 設計期狀態 → 建構子 → 存檔流程讀的替身
void FileRW_HSys_Boot()
{
    if (g_booted) return;
    HS_DfmItems();
    HS_DfmState();
    HS_THandlerSystem();
    HS_CreateSaveProxies();
    HS_CreateContainerProxies();
    std::printf("FileRW HSys: THandlerSystem proxies ready (%d save reads) -- golden HandlerSys.cpp\n",
                (int)(sizeof(kHS_SaveReads) / sizeof(kHS_SaveReads[0])));
    g_booted = true;
}
