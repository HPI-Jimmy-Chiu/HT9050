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
//      //AI(W906-W190-HEATERINS) 20261009 (Ifor01): 表、小工具與顯示序表搬到 cmydef.cpp 檔尾（ht9045_globals）；本檔留 HeaterInsOpt_Read 與第 (3)(4) 段。
//    * 開機建 23 組具名替身 lb/cbHeaterInsOpt_<通道>（W906_HeaterInsCreateProxies），開頁照 golden 讀 71 個 [TempCtrl] 鍵
//      （缺鍵補寫 -9999），存檔照 golden 寫 71 鍵＋HEATER_CTRL_TYPE；
//    * rgHeaterType 被改（golden OnClick＝rgHeaterTypeClick，golden 按下即寫檔）→ beforeApply 在套值前重播（見 BeforeApply）；
//      //AI(W906-FRW-Q14) 20260927: [W906] 偏離 golden：Steven 20260927 Q14＝B —— 重播只改記憶體與畫面，寫檔等整頁存檔 SaveSystemSet；
//    * 頁面版面由 extraJson 給（web/page/ht9045_hsys_heater_c.js 照著在 grpHeater 裡建 Label／下拉）。
//    溫控流程本身仍讀單一廠牌 TC401HeaterControl（底層照 906，RULINGS_20260926 第 26 條；逐通道溫控列給 Jimmy）。
//
//  //AI(W906-FRW-S166) 20260927 [W906] 偏離 golden（裁決過）：Q34 方案 D＋Q15（RULINGS_20260926 S166／S137；D-2＝A 依 RULINGS_20260927
//    第 7 條第 35 題）—— Heater 分頁改成「全機相同／各溫控器不同」＋逐通道站號：
//    * 新鍵 [TempCtrl] HeaterInsMode／HeaterInsIndexOpt／HeaterInsOtherOpt／HeaterInsAddr_<通道>（golden 沒有）；
//    * 開頁不寫檔：HeaterInsOpt_Read 改成不寫檔的讀法（缺鍵只在記憶體補；「不同」模式的 HeaterInsOpt_ 缺鍵＝3 No Heater）；
//      同一頁其他 215 個讀檔呼叫照 golden（缺鍵仍補寫），是 Steven 選 Q15＝B 時已知的範圍；
//    * 存檔（答 YES）寫模式三鍵＋71 鍵的實際廠牌（D-4a，不寫 -9999）＋「不同」模式的站號（D-7a）＋HEATER_CTRL_TYPE（D-6a）；
//      存檔前擋站號範圍與同一個溫控 COM 埠同廠牌同站號（D-7a／D-8a），擋下時什麼都不寫；
//    * golden LoaderSystemSet :262-278、SaveSystemSet :779-794、rgHeaterTypeClick :1529-1538 由產生器取代成第 (4) 段的函式
//      （tools/editlist/HSys.py）；本體第 (4) 段，規則摘要 FileRW/HSys_Heater.h 檔尾。
//    溫控底層（Jimmy，Q34 ⑥ 1～3、5～8）翻完之前，機台溫控仍只看 HEATER_CTRL_TYPE：頁面與存檔 ack 照 D-9a 提示。
//
//  //AI(W906-FRW-E029) 20261002 [W906] 偏離 golden（Steven 裁決過；todo E-029／Q72）：Steven 1002「溫控器少了 DTM」「這邊選不到DTM」
//    「還有EJ1N也選不到」「這些全部都要可以選,所以數量也是少了」「使用 EJ1N 或是 DTM 應該是要設定兩個變數」——
//    * 廠牌加 5 Omron EJ1N、6 Delta DTM（同一個鍵 [TempCtrl] HeaterInsOpt_<通道>；golden 的 0～4 與 g_HeaterInsOptStr 不動）；
//      ⚠ 已接受的風險：同一台之後若跑 BCB V912，5／6 的通道 golden 認不得（廠牌分派沒有 else）；
//    * 71 個通道全部有替身、全部可選；EJ1N／DTM 另有台號／站＋CH（新鍵 HeaterInsCh_<通道>）；
//    * Index 區的 EJ1N／DTM 與 [System] USE_16_HEATER（rgHeater）兩個方向連動，同一次存檔兩個鍵都寫（第 (4) 段 HmLinkIndex）；
//    * D-8a 分三條匯流排（溫控 COM 埠／EJ1N COM_PORT_OMRON／DTM Ethernet）；HEATER_CTRL_TYPE 永遠不寫 5／6。
//    規則全文：FileRW/HSys_Heater.h 檔尾 E029；底層會讀 5／6 之前只是畫面與檔案（D-9a）。
//    golden（Jimmy RULINGS_20261002 #20）：golden＝906_0625；71 個通道＝906 MachineType.h:632 enum eTempControll（V912 :638 相同）。逐通道廠牌是
//    V912 才有的功能（906_0625 沒有 MachineTypeUtility.cpp／EN_HEATER_SHEET），照 Steven Q34／Q71～Q76 當 St01 的設計留著、等 Jimmy 定
//    （NIGHT_REPORT s0 #63）；其他引用的 906 行號對照在 FileRW/HSys_Heater.h 檔尾 E029。
// ===========================================================================
#include "Public/cJSON.h"            //AI(W906-FRW-P8) 20260926: beforeApply 解析頁面值；放在產生檔之前（同 FileRW/TestIF_File_SetUp.cpp:27 的理由）
#include "WebBridge/JsonWriter.h"     //AI(W906-FRW-P8) 20260926: extraJson（逐通道溫控 ComboBox 的版面）
#include "FileRW/HSys.gen.inc"

#include <cmath>                     //AI(W906-FRW-S90) 20260926: std::fabs（EP 四鍵多寫者規則）
#include <cstdio>
#include <cstdlib>                    //AI(W906-FRW-S90) 20260926: std::atof

#include "FileRW/_EditPage.h"

#include <cstring>                    //AI(W906-FRW-P8) 20260926: memset（golden MachineTypeUtility.cpp GetStaTable_HeaterInsOpt）
#include <fstream>                    //AI(W906-FRW-E029) 20261002: DTME08_Control.ini 在不在（唯讀，W906_HeaterMixExtraJson）
#include "vclcompat/IniFiles.h"       //AI(W906-FRW-E029) 20261002: TIniFile（write-through，只讀 DTME08_Control.ini）

// ===========================================================================
// //AI(W906-FRW-P8) 20260926: golden V912 逐通道溫控器廠牌表（EN_HEATER_SHEET=1）的本體。宣告與範圍說明在 FileRW/HSys_Heater.h 檔頭。
//   (1) golden HandlerSys.cpp:50-60   HeaterInsOpt_Read
//   (2) //AI(W906-W190-HEATERINS) 20261009 (Ifor01): golden MachineTypeUtility.cpp 本體＋HandlerSys.cpp:32-49 顯示序表 → cmydef.cpp 檔尾
//       （ht9045_globals；Ifor 1009 14:0x 定，Steven 1009 13:5x「翻成 cpp、放進 cpp 現有結構」）。宣告照舊在 FileRW/HSys_Heater.h。
//   (3) 移植樹專用：W906_HeaterInsProxyName／W906_HeaterInsCreateProxies（golden 建構子 :70-112 的 C 路版）
// ===========================================================================
// ---- (1) golden HandlerSys.cpp:50-60 ----
// //AI(W906-W190-HEATERINS) 20261009 (Ifor01): golden HandlerSys.cpp:32-49（顯示序表 g_vecHeaterTypeIdxForShow／兩張對照表／TypeIdxToShowIdx／
//   ShowIdxToTypeIdx）跟著第 (2) 段搬到 cmydef.cpp 檔尾：GetFirstHeaterInsOpt 會讀它，函式庫連不到這裡。
// golden：缺鍵時 CheckAndReadIniDataGeneral 會把 -9999 寫進 Gerneral.ini [TempCtrl]（71 鍵），記憶體再回退成 HEATER_CTRL_TYPE。
//   golden 912 在 HandlerSys 開頁（LoaderSystemSet :265）與溫控第一次執行（bthermo.cpp DoThermo:1173）都呼叫；移植樹只有開頁這一處
//   （DoThermo 那一處屬溫控底層，列給 Jimmy）。
// //AI(W906-FRW-S166) 20260927 [W906] 偏離 golden：Q15（RULINGS_20260926 S137「預設選 3 No Heater，然後 B 開頁不寫檔」）併入 Q34 方案 D
//   （S166）—— 本體改成不寫檔的讀法 W906_HeaterMixReadFile()（第 (4) 段：CheckIniData＋ReadIniData，缺鍵只在記憶體補；模式／Index／
//   其他／站號照方案 D）。呼叫點不變：開頁（LoaderSystemSet :265 的取代 W906_HeaterMixLoaderSystemSet）與 Jimmy 之後照 golden 翻的
//   bthermo DoThermo:1173 —— 兩處都不寫檔（decisions-decided Q34 ④ 最後一條）。golden 原文留在下面 #if 0。
void HeaterInsOpt_Read()
{
    W906_HeaterMixReadFile();
#if 0   // golden V912 HandlerSys.cpp:50-60 原文（缺鍵時 CheckAndReadIniDataGeneral 補寫 -9999；Q15 改掉）
    int iHeaterInsOpt_Old = CheckAndReadIniDataGeneral("TempCtrl","HEATER_CTRL_TYPE",KT4H);
    for(int iTypeIdx=0; iTypeIdx<eHeaterType_Count; ++iTypeIdx)
    {
        g_tHeaterInsInfo[iTypeIdx].m_iHeaterInsOpt =
            CheckAndReadIniDataGeneral("TempCtrl", g_tHeaterInsInfo[iTypeIdx].m_asSaveName, INVALID_INT_VAL_NEG);
        if(INVALID_INT_VAL_NEG==g_tHeaterInsInfo[iTypeIdx].GetHeaterInsOpt())
            g_tHeaterInsInfo[iTypeIdx].m_iHeaterInsOpt = iHeaterInsOpt_Old;     //回退 HEATER_CTRL_TYPE(=rgHeaterType 設定值)，非固定 KT4H；其他機型進版沿用原廠牌
    }
#endif
}

// ---- (2) //AI(W906-W190-HEATERINS) 20261009 (Ifor01): 搬到 cmydef.cpp 檔尾（ht9045_globals）——golden 913 rs232.cpp／bthermo.cpp（ht9045_sm）
//   要呼叫 IsValEqual_HeaterInsOpt／IsNoHeaterMachine，留在這裡（只編進 wb_serve）連不到。Ifor 1009 14:0x 定放 cmydef.cpp（跟 TC401HeaterControl 同檔）；
//   Steven 1009 13:5x：照 913 翻成 cpp、放進 cpp 現有結構，不照 BCB 開 MachineTypeUtility.cpp。宣告照舊在 FileRW/HSys_Heater.h。----

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
static void W906_HeaterMixCreateProxies();   //AI(W906-FRW-S166) 20260927 [W906]：第 (4) 段
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
            //AI(W906-FRW-E029) 20261002 [W906] 偏離 golden：golden :104-105 加 eHeaterInsOpt_Count（5）個 g_HeaterInsOptStr；這裡加 7 個
            //   （＋Omron EJ1N＝5、Delta DTM＝6；Steven 1002「溫控器少了 DTM」「還有EJ1N也選不到」，見 FileRW/HSys_Heater.h 檔尾 E029）
            for(int j = 0; j<W906_HEATER_INS_OPT_COUNT; ++j)
                pCb->Items->Add(g_W906HeaterInsOptStr[j]);
            filerw::ELComboIndex(pCb, g_tHeaterInsInfo[iTypeIdx].GetHeaterInsOpt());   // golden :106 pCb->ItemIndex = ...GetHeaterInsOpt()
            pCb->Tag = i;
            g_tHeaterInsInfo[iTypeIdx].SetCtrlItemProp(GetCtrlItemVisProp(iTypeIdx), pLb, pCb);
        }
    }
    W906_HeaterMixCreateProxies();   //AI(W906-FRW-S166) 20260927 [W906] golden 沒有：Q34 方案 D 的替身（模式、Index／其他、Index 32 區廠牌、55 個站號），見第 (4) 段
#endif
}


// ===========================================================================
// ---- (4) //AI(W906-FRW-S166) 20260927 [W906] Q34 方案 D＋Q15（golden 沒有；偏離 golden，裁決過）----
//   裁決：RULINGS_20260926 S166（Steven 20260927「開工，其他照建議」）＝D-1b、D-2a（RULINGS_20260927 第 7 條第 35 題：Index 位置＝
//     Head1～4＋Index 32 區，golden V912 MachineType.h:638-645；Socket、DUT1～4、IndexESD、Door1～2 算其他位置）、D-3a、D-4a、D-5a、
//     D-6a、D-7a、D-8a、D-9a；⑥ 第 4 點（No Heater 通道讓迴圈跳過）不做（RULINGS_20260927 第 7 條第 34 題：照 golden）。
//     Q15＝S137（缺鍵＝3 No Heater、開頁不寫檔）；Q14＝S136（按 Heater Type 不寫檔，整頁存檔才寫）。
//   全文：D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md「### Q34.」；鍵與規則摘要：FileRW/HSys_Heater.h 檔尾。
//   替身（＝HTML 元件 id，Parent＝grpHeater；頁面 web/page/ht9045_hsys_heater_c.js 照 ExtraJson 的 "mix" 建）：
//     rgHeaterInsMode（TRadioGroup：0 全機相同／1 各溫控器不同）、cbHeaterInsIndexOpt／cbHeaterInsOtherOpt（TComboBox，5 個廠牌）、
//     cbHeaterInsOpt_<通道>（有下拉的 23 個＝golden 建構子那一組；Index 32 區本段另建）、edHeaterInsAddr_<通道>（TEdit，同 55 個；空白＝預設站號）。
//   替身一律可改（「全機相同」時逐通道表的唯讀由頁面做）：同一次存檔可以先切模式再改通道，伺服器端停用的話頁面送的值會被丟掉。
//   Jimmy 翻完溫控底層（⑥ 1～3、5～8）之前，機台溫控只看 TC401HeaterControl＝HEATER_CTRL_TYPE：頁面與存檔 ack 照 D-9a 提示。
//   //AI(W906-FRW-E029) 20261002 [W906]（todo E-029／Q72；Steven 1002「溫控器少了 DTM」「這邊選不到DTM」「還有EJ1N也選不到」
//     「這些全部都要可以選,所以數量也是少了」「使用 EJ1N 或是 DTM 應該是要設定兩個變數」）：
//     * 廠牌 7 個（＋5 Omron EJ1N、6 Delta DTM，g_W906HeaterInsOptStr）；下拉替身全部 7 個。
//     * 71 個通道全部有替身、全部可改（以前 55 個，Index 32 區以外 golden 沒有下拉的 16 個沒有）：cbHeaterInsOpt_<通道>、
//       edHeaterInsAddr_<通道>、新的 edHeaterInsCh_<通道>（EJ1N／DTM 的 CH）。golden 可見條件（GetShow）只拿來判斷「這台有裝」。
//     * Index 區 EJ1N／DTM 跟 [System] USE_16_HEATER（rgHeater）連動，兩個鍵同一次存檔一起寫（HmLinkIndex、W906_HeaterMixSave）。
//     * D-8a 分匯流排：溫控 COM 埠／EJ1N（COM_PORT_OMRON）／DTM（Ethernet）（HmDuplicates）。
//     規則全文與已接受的 BCB 風險：FileRW/HSys_Heater.h 檔尾 E029。
// ===========================================================================
int g_iHeaterInsMode     = HEATER_INS_MODE_SAME;
int g_iHeaterInsIndexOpt = DEFAULT_HEATER_INS_OPT;
int g_iHeaterInsOtherOpt = DEFAULT_HEATER_INS_OPT;
int g_iHeaterInsAddr[eHeaterType_Count];         // INVALID_INT_VAL_NEG＝沒有鍵（下面 s_hmInitArrays 填；E029 起，以前 0）
int g_iHeaterInsCh[eHeaterType_Count];           // //AI(W906-FRW-E029) 20261002：HeaterInsCh_<通道>
//AI(W906-FRW-E029) 20261002 [W906] 偏離 golden：頁面下拉的 7 個字。0～4 逐字＝golden V912 MachineTypeUtility.cpp:19-25（上面 (2) 的
//   g_HeaterInsOptStr，golden 那一份與 eHeaterInsOpt_Count 不動）；5、6 是 Steven 1002 要的新代碼（FileRW/HSys_Heater.h 檔尾 E029）。
AnsiString g_W906HeaterInsOptStr[W906_HEATER_INS_OPT_COUNT] = {
    "TC401",            //0 = TC401
    "Panasonic KT4H",   //1 = KT4H
    "Omron E5DC",       //2 = E5DC
    "No Heater",        //3 = NoHeater
    "DTK4848",          //4 = DTK4848
    "Omron EJ1N",       //5 = W906_HEATER_INS_EJ1N
    "Delta DTM",        //6 = W906_HEATER_INS_DTM
};

namespace {
// 同一個 TU 依定義順序做動態初始化 ⇒ 這個物件建構時上面兩個陣列已經在（零值），之後才有人讀
struct HmInitArrays {
    HmInitArrays() { for (int i = 0; i < eHeaterType_Count; ++i) { g_iHeaterInsAddr[i] = INVALID_INT_VAL_NEG; g_iHeaterInsCh[i] = INVALID_INT_VAL_NEG; } }
};
HmInitArrays s_hmInitArrays;

const char* const kHmForm       = "THandlerSystem";
const char* const kHmModeProxy  = "rgHeaterInsMode";
const char* const kHmIndexProxy = "cbHeaterInsIndexOpt";
const char* const kHmOtherProxy = "cbHeaterInsOtherOpt";
const char* const kHmU16Proxy   = "rgHeater";               // //AI(W906-FRW-E029) 20261002：golden 的 Index Heater Counts（[System] USE_16_HEATER）
// D-9a 原文（ExtraJson 帶給頁面；存檔 ack 也帶）。//AI(W906-FRW-E029) 20261002：後半句加上 EJ1N／DTM（前半句不動，頁面與測試用前綴比對）
const char* const kHmHintZh = "目前溫控只看 HEATER_CTRL_TYPE（單一廠牌、預設站號），這些設定要等底層翻完才生效"
                              "（逐通道的 Omron EJ1N／Delta DTM 也一樣：底層要會讀 HeaterInsOpt_ 的 5／6 與台號／CH）";
const char* const kHmHintEn = "Temperature control currently reads HEATER_CTRL_TYPE only (one brand, default stations); "
                              "these settings take effect after the lower layer (bthermo) is translated "
                              "(per-channel Omron EJ1N / Delta DTM, codes 5 / 6, too)";

TComboBox* s_pHmCb[eHeaterType_Count]   = {};    // 逐通道廠牌替身：有下拉的 23 個＝golden 那一組（GetCtrlItem_Cb）；其他 48 個本段建（E029：以前只有 Index 32 區）
TEdit*     s_pHmAddr[eHeaterType_Count] = {};    // 逐通道站號替身（E029：71 個；EJ1N＝台號、DTM＝內部站號）
TEdit*     s_pHmCh[eHeaterType_Count]   = {};    // //AI(W906-FRW-E029) 20261002：EJ1N／DTM 的 CH 替身（71 個）
int        s_iHmOpenU16 = -1;                    // //AI(W906-FRW-E029) 20261002：開頁時（golden :260 剛讀進 rgHeater）／上一次存檔時的 USE_16_HEATER

bool HmValidOpt(int v)    { return v >= 0 && v < W906_HEATER_INS_OPT_COUNT; }   // E029：0～6（以前 0～4）
bool HmValidGolden(int v) { return v >= 0 && v < eHeaterInsOpt_Count; }         // golden 的 0～4（D-1b 推斷模式照舊只看這幾個）
bool HmIsAreaOpt(int v)   { return v == W906_HEATER_INS_EJ1N || v == W906_HEATER_INS_DTM; }
bool HmComCtrl(int v)     { return v == TC401 || v == KT4H || v == E5DC || v == DTK4848; }   // 溫控 COM 埠上的溫控器（HEATER_CTRL_TYPE 可以寫的值，3 除外）
bool HmIsZone(int ti)  { return (ti >= tcAa1 && ti <= tcBd2) || (ti >= tcAe1 && ti <= tcBh2); }   // Index 32 區（golden V912 MachineType.h:640-641、:644-645）
// //AI(W906-FRW-E029) 20261002：USE_16_HEATER 的組數（golden MachineType.h:717-723；eht4Heater＝0 組 Index 區）
int  HmCount(int u)
{
    if (u == eht16Heater || u == eht16HeaterEJ1N || u == eht16HeaterDTME08) return 16;
    if (u == eht32HeaterEJ1N || u == eht32HeaterKT4H || u == eht32HeaterDTME08) return 32;
    return 0;
}
// Index Heater Counts 管的 Index 區：Aa1～Bd2，32 組再加 Ae1～Bh2（Steven 1002）。4 Heaters（0 組）時算 Aa1～Bd2＝選 EJ1N／DTM 會打開的那 16 組。
bool HmInArea(int ti, int u) { return (ti >= tcAa1 && ti <= tcBd2) || (HmCount(u) == 32 && ti >= tcAe1 && ti <= tcBh2); }
// 這台有裝（存檔檢查與站號寫檔的範圍；頁面同一套規則決定顯示哪幾列）：
//   //AI(W906-FRW-E029) 20261002 [W906] 偏離 golden（Steven 1002 17:1x，經 ST01-E2 問過、選建議）：「index區裡面, Head 1234 跟 Ax Bx這32組屬於互斥的,
//   也就是同時間只會顯示其中的一種」「socket跟 Dut 1~4也是互斥的」——
//   * Head1～4 vs Index 區：照 Index Heater Counts（u＝rgHeater）：4 Heaters＝Head1～4；16 組＝Aa1～Bd2；32 組＝Aa1～Bh2。
//     ⚠ 跟 golden V912 MachineTypeUtility.cpp:241-293 GetCtrlItemVisProp 相反（golden :256-266：u≠0 才顯示 Head1～4、4 Heaters 反而藏起來；
//     本檔第 (2) 段的逐字副本 GetCtrlItemVisProp 不動）——Steven 知道這點仍這樣選。
//   * Socket vs DUT1～4：照 Dut Heater Count（dut＝rgUse4DUT＝[System] SocketBasedAdd4Temp，golden enum eSocketTempControll：0 eDut1ea、
//     1 eDut4ea、2 eDut2ea，MachineType.h:742-744；畫面上的順序是 1 EA、4 EA、2 EA，照 enum 對，不照字的位置）：1 EA＝Socket；2 EA＝DUT1～2；
//     4 EA＝DUT1～4。DUT 那半 golden 本來就有（:270-279）；Socket 是把 golden :268 註解掉的 `//{ return (iSocketBaseTempCount==eDut1ea); }` 打開。
//   * golden 的可見條件只在開機（建構子）算一次；這兩組改成跟著頁面上的 rgHeater／rgUse4DUT（存檔時用替身的值；頁面即時，新行為）。
//   其他通道：有下拉的照 golden 可見條件（開機 GetCtrlItemVisProp）；其他 16 個（OutSht、Base、HotPlate3／4、Shuttle3／4、Door、LBUp／Down）
//   golden 沒有可見條件 ⇒ 不算（「各溫控器不同」指定了站號才算）。頁面上這些通道照樣列出、可選（Steven「這些全部都要可以選」），只標「這台沒裝」。
bool HmActive(int ti, int u, int dut)
{
    if (ti >= tcHead1 && ti <= tcHead4) return u == eht4Heater;
    if (HmIsZone(ti)) return HmCount(u) > 0 && HmInArea(ti, u);
    if (ti == tcSocket) return dut == eDut1ea;
    if (ti == tcDUT1 || ti == tcDUT2) return dut == eDut2ea || dut == eDut4ea;
    if (ti == tcDUT3 || ti == tcDUT4) return dut == eDut4ea;
    if (g_tHeaterInsInfo[ti].GetOccupy()) return g_tHeaterInsInfo[ti].GetShow();
    return false;
}
// 頁面上的兩組互斥（ExtraJson 的 "pair"；頁面照它即時顯示／隱藏）
const char* HmPair(int ti)
{
    if (ti >= tcHead1 && ti <= tcHead4) return "head";
    if (ti >= tcAa1 && ti <= tcBd2) return "zone16";
    if (ti >= tcAe1 && ti <= tcBh2) return "zone32";
    if (ti == tcSocket) return "socket";
    if (ti == tcDUT1 || ti == tcDUT2) return "dut12";
    if (ti == tcDUT3 || ti == tcDUT4) return "dut34";
    return "";
}
// golden 的 Index 區接線：iTempCode 第 p 個（cmydef.cpp:111-117：Aa1、Ba1、Ab1、Bb1…）＝EJ1N 第 p/4+1 台 CH p%4+1（bthermo.cpp:3903-3918、
//   :3943-3958）＝DTM 內部站號 p/8 CH p%8+1（fDTME08.cpp:266-270 GetStationAndNumber）。Index 區以外沒有對照 → false。
bool HmDefaultUnitCh(int ti, int b, int* unit, int* ch)
{
    if (!HmIsAreaOpt(b) || !HmIsZone(ti)) return false;
    int p = -1;
    for (int k = 0; k < INDEX_HEAT_COUNT; ++k) if (iTempCode[k] == ti) { p = k; break; }
    if (p < 0) return false;
    if (b == W906_HEATER_INS_EJ1N) { *unit = p / 4 + 1; *ch = p % 4 + 1; }
    else                           { *unit = p / 8;     *ch = p % 8 + 1; }
    return true;
}
bool HmUnitChInRange(int b, int unit, int ch)
{
    if (b == W906_HEATER_INS_EJ1N) return unit >= HEATER_INS_EJ1N_UNIT_MIN && unit <= HEATER_INS_EJ1N_UNIT_MAX && ch >= 1 && ch <= HEATER_INS_EJ1N_CH_MAX;
    if (b == W906_HEATER_INS_DTM)  return unit >= HEATER_INS_DTM_STATION_MIN && unit <= HEATER_INS_DTM_STATION_MAX && ch >= 1 && ch <= HEATER_INS_DTM_CH_MAX;
    return false;
}

// Q15（S137）不寫檔的讀法：先 CheckIniData（移植樹 common.cpp:634，只看鍵在不在），在才 ReadIniData（common.cpp:851）；
//   取代 golden 會補寫的 CheckAndReadIniDataGeneral（移植樹 common.cpp:1643-1654）。Gerneral.ini 的 TIniFile 是 write-through
//   （vclcompat/IniFiles.cpp:299），每次讀的都是檔案現況。缺鍵回 def，只放記憶體。
int HmReadInt(const AnsiString& key, int def, bool* had = nullptr)
{
    const bool h = CheckIniData(asGeneralPath, AnsiString("TempCtrl"), key);
    if (had) *had = h;
    return h ? ReadIniData(asGeneralPath, AnsiString("TempCtrl"), key, def) : def;
}
int HmCtrlTypeNoWrite() { return HmReadInt("HEATER_CTRL_TYPE", KT4H); }   // golden HandlerSys.cpp:52／:271 同鍵同預設（KT4H），但不寫檔

AnsiString HmChanName(int ti)          // 通道名＝HeaterInsOpt_<通道> 去掉前綴（71 個都等於 golden 的 ShowName）
{
    AnsiString s = g_tHeaterInsInfo[ti].GetSaveName();
    const AnsiString pre("HeaterInsOpt_");
    if (s.Pos(pre) == 1) s = s.SubString(pre.Length() + 1, s.Length() - pre.Length());
    return s;
}
AnsiString HmAddrKey(int ti)     { return AnsiString("HeaterInsAddr_") + HmChanName(ti); }
AnsiString HmAddrProxy(int ti)   { return AnsiString("ed") + HmAddrKey(ti); }
AnsiString HmChKey(int ti)       { return AnsiString("HeaterInsCh_") + HmChanName(ti); }     // //AI(W906-FRW-E029) 20261002
AnsiString HmChProxy(int ti)     { return AnsiString("ed") + HmChKey(ti); }
AnsiString HmOptName(int v)      { return HmValidOpt(v) ? g_W906HeaterInsOptStr[v] : AnsiString("(") + AnsiString(v) + ")"; }
int        HmMaxStation(int opt) { return opt == E5DC ? HEATER_INS_ADDR_MAX_E5DC : HEATER_INS_ADDR_MAX; }

// 站號欄的字：空白＝預設（*blank=true）；否則只收十進位整數（前後空白可）；*ok=false＝不是數字
int HmParseStation(const AnsiString& text, bool* blank, bool* ok)
{
    const std::string s = text.Trim().c_str();
    *blank = s.empty();
    *ok = true;
    if (s.empty()) return 0;
    if (s.size() > 6) { *ok = false; return 0; }
    for (std::size_t i = 0; i < s.size(); ++i)
        if (s[i] < '0' || s[i] > '9') { *ok = false; return 0; }
    return std::atoi(s.c_str());
}

// //AI(W906-FRW-E029) 20261002：頁面現在的 USE_16_HEATER（rgHeater 替身；開頁時 golden LoaderSystemSet :260 剛從檔案讀進來）
int HmProxyU16()
{
    const int u = EL<TRadioGroup>(kHmForm, kHmU16Proxy)->ItemIndex;
    return (u >= eht4Heater && u <= eht32HeaterDTME08) ? u : USE_16_HEATER;
}
// //AI(W906-FRW-E029) 20261002：頁面現在的 Dut Heater Count（rgUse4DUT 替身＝[System] SocketBasedAdd4Temp；golden LoaderSystemSet 讀它在
//   :262-278 之後，開頁讀檔時還是舊值 ⇒ 只在存檔（頁面值）與 ExtraJson（開頁全部跑完）用）
int HmProxyDut()
{
    const int d = EL<TRadioGroup>(kHmForm, "rgUse4DUT")->ItemIndex;
    return (d == eDut1ea || d == eDut4ea || d == eDut2ea) ? d : iSocketBaseTempCount;
}
AnsiString HmU16Label(int u)
{
    TRadioGroup* r = EL<TRadioGroup>(kHmForm, kHmU16Proxy);
    if (r->Items && u >= 0 && u < r->Items->Count) return r->Items->Strings[u];
    return AnsiString(u);
}
// 「全機相同」時通道的廠牌：Index 組（D-2a）＝Index 的廠牌、其他＝其他位置的。
//   //AI(W906-FRW-E029) 20261002：Index 選 EJ1N／DTM 時只有 Index Heater Counts 管的 Index 區是 EJ1N／DTM；Head1～4（照 golden 留在溫控
//   COM 埠，bthermo.cpp:1331-1367 只跳過 Index 32 區）與這台組數以外的 Index 區（例 16 組時的 Ae1～Bh2）跟著「其他位置」。
int HmBrandSame(int ti, int iIndex, int iOther, int u)
{
    if (!W906_HeaterInsIsIndexGroup(ti)) return iOther;
    if (!HmIsAreaOpt(iIndex)) return iIndex;
    return (HmIsZone(ti) && HmInArea(ti, u)) ? iIndex : iOther;
}

// 載入後把記憶體填回方案 D 的替身（開頁、開機建替身時）
void HmFillMixProxies(int u)
{
    EL<TRadioGroup>(kHmForm, kHmModeProxy)->ItemIndex = g_iHeaterInsMode;
    TComboBox* ci = EL<TComboBox>(kHmForm, kHmIndexProxy);
    filerw::ELComboIndex(ci, g_iHeaterInsIndexOpt);
    filerw::ELComboIndex(EL<TComboBox>(kHmForm, kHmOtherProxy), g_iHeaterInsOtherOpt);
    //AI(W906-FRW-E029) 20261002 [W906]：以前 EJ1N／DTME08 時停用 Index 下拉（S166 St01 的解讀）；現在 Index 可選 EJ1N／DTM、跟 USE_16_HEATER
    //   連動（Steven 1002 Q72），一律可改。
    ci->Enabled = true;
    for (int ti = 0; ti < eHeaterType_Count; ++ti) {
        if (!s_pHmCb[ti] || !s_pHmAddr[ti] || !s_pHmCh[ti]) continue;
        //AI(W906-FRW-E029) 20261002 [W906] 偏離 golden：71 個全部看得到、可改（Steven 1002「這些全部都要可以選」）。golden 的可見條件
        //   （有下拉的 23 個開機 GetCtrlItemVisProp）留在 g_tHeaterInsInfo[].GetShow()，只拿來判斷「這台有裝」（HmActive）。
        //   替身看不見的話 PageSave 會把頁面的值丟掉（ELEditable 看 Visible）。
        s_pHmCb[ti]->Visible = true;
        s_pHmAddr[ti]->Visible = true;
        s_pHmCh[ti]->Visible = true;
        const int b = g_tHeaterInsInfo[ti].GetHeaterInsOpt();
        const int a = g_iHeaterInsAddr[ti], c = g_iHeaterInsCh[ti];
        AnsiString ta, tc;
        if (W906_HeaterInsBusOf(b) == W906_HEATER_BUS_COM) {
            const int def = W906_HeaterInsDefaultStation(ti, b);
            if (a != INVALID_INT_VAL_NEG && a != def) ta = AnsiString(a);   // 空白＝預設站號（頁面灰字顯示預設值）
        } else if (HmIsAreaOpt(b)) {                                        // EJ1N／DTM：台號＋CH 一組；等於 golden 預設（Index 區）就空白
            int du = 0, dc = 0;
            const bool hasDef = HmDefaultUnitCh(ti, b, &du, &dc);
            if (a != INVALID_INT_VAL_NEG && c != INVALID_INT_VAL_NEG && !(hasDef && a == du && c == dc)) { ta = AnsiString(a); tc = AnsiString(c); }
        }
        s_pHmAddr[ti]->Text = ta;
        s_pHmCh[ti]->Text   = tc;
    }
    (void)u;
}

// 存檔要寫的內容（由替身算；頁面值已由 PageSave 套進替身）
struct HmPlan {
    int  u16;                          // //AI(W906-FRW-E029) 20261002：要寫的 USE_16_HEATER（rgHeater 替身，HmLinkIndex 之後）
    int  mode;
    int  iIndex;
    int  iOther;
    int  brand[eHeaterType_Count];
    int  station[eHeaterType_Count];   // 站號（COM 埠廠牌）／台號（EJ1N）／內部站號（DTM）；INVALID＝沒有
    int  ch[eHeaterType_Count];        // //AI(W906-FRW-E029) 20261002：EJ1N／DTM 的 CH；INVALID＝沒有
    bool inUse[eHeaterType_Count];     // //AI(W906-FRW-E029) 20261002：在某條匯流排上（這台有裝，或「不同」模式指定了站號）＝D-8a 檢查、寫站號
    bool addrOk[eHeaterType_Count];    // station／ch 有值
    bool custom[eHeaterType_Count];    // 站號跟預設不同（D-9a「改過站號」）
};
struct HmProblems {
    std::vector<std::string> zh, en;
    void Add(const AnsiString& z, const AnsiString& e) { zh.push_back(z.c_str()); en.push_back(e.c_str()); }
};

// //AI(W906-FRW-E029) 20261002 [W906]（Q72，Steven 1002：「使用 EJ1N 或是 DTM 應該是要設定兩個變數」）：Index 區的 EJ1N／DTM 與
//   [System] USE_16_HEATER（rgHeater）對齊——在 HmFromProxies 之前改替身，golden SaveSystemSet :777 寫的 USE_16_HEATER 與
//   W906_HeaterMixSave 寫的 71 鍵就是同一組。頁面（ht9045_hsys_heater_c.js）即時做同樣的連動；這裡是頁面沒做時（舊頁面、別的用戶端）
//   的補救與保證。規則（FileRW/HSys_Heater.h 檔尾 E029）：
//     H＝Heater 分頁的 Index 區：「相同」看 Index 下拉；「不同」看 Index 區通道（同時有 EJ1N 與 DTM → 擋）。R＝rgHeater 的控制器。
//     H＝R：一致（「不同」模式把 Index 區沒跟上的通道補成同一種）。
//     rgHeater 跟開頁時不同、H 沒變 → Heater 分頁跟著 rgHeater（EJ1N／DTME08 → Index 區＝5／6；改回 1／4 → 原本的 5／6 改 KT4H）。
//     rgHeater 跟開頁時相同、H 變了 → rgHeater 跟著（W906_HeaterInsU16For：組數不變，0＝4 Heaters→16）。
//     兩邊都變了又對不上 → 擋下（不知道要以哪一邊為準；整頁不存，同 D-8a）。
void HmSetArea(int u, int code)
{
    for (int ti = 0; ti < eHeaterType_Count; ++ti)
        if (HmIsZone(ti) && HmInArea(ti, u) && s_pHmCb[ti]) filerw::ELComboIndex(s_pHmCb[ti], code);
}
void HmApplyU16ToTab(int u, int uOld, bool diff)
{
    const int code = W906_HeaterInsU16Area(u);
    TComboBox* ci = EL<TComboBox>(kHmForm, kHmIndexProxy);
    if (code > 0) {
        if (!diff) filerw::ELComboIndex(ci, code); else HmSetArea(u, code);
        return;
    }
    if (!diff) { if (HmIsAreaOpt(ci->ItemIndex)) filerw::ELComboIndex(ci, KT4H); return; }
    for (int ti = 0; ti < eHeaterType_Count; ++ti)       // 原本 EJ1N／DTM 的 Index 區（新舊兩種組數的範圍）→ KT4H（golden eht16Heater／eht32HeaterKT4H）
        if (HmIsZone(ti) && (HmInArea(ti, u) || HmInArea(ti, uOld)) && s_pHmCb[ti] && HmIsAreaOpt(s_pHmCb[ti]->ItemIndex))
            filerw::ELComboIndex(s_pHmCb[ti], KT4H);
}
// 「各溫控器不同」時「全機相同」那兩個下拉照樣存檔（切回時用）⇒ Index 下拉也跟 USE_16_HEATER 對齊（讀檔 HmReadFile 同規則，存完再開頁位元組不變）
void HmAlignIndexProxy(int u)
{
    TComboBox* ci = EL<TComboBox>(kHmForm, kHmIndexProxy);
    const int code = W906_HeaterInsU16Area(u);
    if (code > 0) { if (ci->ItemIndex != code) filerw::ELComboIndex(ci, code); }
    else if (HmIsAreaOpt(ci->ItemIndex)) filerw::ELComboIndex(ci, KT4H);
}
void HmLinkIndex(HmProblems* pr, HmProblems* notes)
{
    TRadioGroup* rh = EL<TRadioGroup>(kHmForm, kHmU16Proxy);
    const int u = rh->ItemIndex;
    if (u < eht4Heater || u > eht32HeaterDTME08) return;               // 不是 golden 的 7 個選項：不連動（golden 照存）
    const int uOpen = (s_iHmOpenU16 >= eht4Heater && s_iHmOpenU16 <= eht32HeaterDTME08) ? s_iHmOpenU16 : u;
    const bool diff = EL<TRadioGroup>(kHmForm, kHmModeProxy)->ItemIndex == HEATER_INS_MODE_DIFF;
    TComboBox* ci = EL<TComboBox>(kHmForm, kHmIndexProxy);
    int H = -1;
    if (!diff) {
        H = HmIsAreaOpt(ci->ItemIndex) ? ci->ItemIndex : -1;
    } else {
        bool e = false, d = false;
        for (int ti = 0; ti < eHeaterType_Count; ++ti) {
            if (!HmIsZone(ti) || !HmInArea(ti, u) || !s_pHmCb[ti]) continue;
            e = e || s_pHmCb[ti]->ItemIndex == W906_HEATER_INS_EJ1N;
            d = d || s_pHmCb[ti]->ItemIndex == W906_HEATER_INS_DTM;
        }
        if (e && d) {
            pr->Add("Index 區同時有 Omron EJ1N 與 Delta DTM 的通道：Index Heater Counts（USE_16_HEATER）只能選一種，Index 區要改成同一種",
                    "the Index area has both Omron EJ1N and Delta DTM channels: Index Heater Counts (USE_16_HEATER) takes one of them");
            return;
        }
        H = e ? W906_HEATER_INS_EJ1N : (d ? W906_HEATER_INS_DTM : -1);
    }
    const int R = W906_HeaterInsU16Area(u);
    const AnsiString hName = H > 0 ? HmOptName(H) : AnsiString("溫控 COM 埠的廠牌");
    const AnsiString hNameEn = H > 0 ? HmOptName(H) : AnsiString("a heater COM port brand");
    if (H == R) { if (diff && R > 0) HmSetArea(u, R); HmAlignIndexProxy(u); return; }
    if (u != uOpen && H == W906_HeaterInsU16Area(uOpen)) {             // Index Heater Counts 改了 → Heater 分頁跟著
        HmApplyU16ToTab(u, uOpen, diff);
        HmAlignIndexProxy(u);
        const int code = W906_HeaterInsU16Area(u);
        AnsiString z, e;
        z.sprintf("Index Heater Counts 改成「%s」：Heater 分頁的 Index 區跟著改成 %s（兩個一起存）", HmU16Label(u).c_str(),
                  code > 0 ? HmOptName(code).c_str() : "Panasonic KT4H（原本是 EJ1N／DTM 的通道）");
        e.sprintf("Index Heater Counts is now \"%s\": the Heater tab's Index area follows (%s); both are saved together", HmU16Label(u).c_str(),
                  code > 0 ? HmOptName(code).c_str() : "Panasonic KT4H for the EJ1N / DTM channels");
        notes->Add(z, e);
        return;
    }
    if (u == uOpen) {                                                   // Heater 分頁改了 → Index Heater Counts 跟著
        const int u2 = W906_HeaterInsU16For(H, u);
        rh->ItemIndex = u2;
        if (diff && H > 0) HmSetArea(u2, H);
        HmAlignIndexProxy(u2);
        AnsiString z, e;
        z.sprintf("Heater 分頁的 Index 區選了 %s：Index Heater Counts（USE_16_HEATER）跟著改成「%s」（兩個一起存）", hName.c_str(), HmU16Label(u2).c_str());
        e.sprintf("the Heater tab's Index area is %s: Index Heater Counts (USE_16_HEATER) follows -> \"%s\"; both are saved together",
                  hNameEn.c_str(), HmU16Label(u2).c_str());
        notes->Add(z, e);
        return;
    }
    AnsiString z, e;
    z.sprintf("Index Heater Counts 選「%s」，Heater 分頁的 Index 區是 %s：兩邊都改過、對不上，不知道要以哪一邊為準（這兩個要一起存），請改成一致",
              HmU16Label(u).c_str(), hName.c_str());
    e.sprintf("Index Heater Counts \"%s\" and the Heater tab's Index area (%s) were both changed and do not agree", HmU16Label(u).c_str(), hNameEn.c_str());
    pr->Add(z, e);
}

void HmFromProxies(HmPlan* p, HmProblems* pr)
{
    p->u16 = HmProxyU16();
    const int u = p->u16, dut = HmProxyDut();
    const int m = EL<TRadioGroup>(kHmForm, kHmModeProxy)->ItemIndex;
    if (m != HEATER_INS_MODE_SAME && m != HEATER_INS_MODE_DIFF)
        pr->Add("溫控器廠牌的模式沒有選（全機相同／各溫控器不同）", "heater brand mode is not selected (same for all / per heater)");
    p->mode   = (m == HEATER_INS_MODE_DIFF) ? HEATER_INS_MODE_DIFF : HEATER_INS_MODE_SAME;
    p->iOther = EL<TComboBox>(kHmForm, kHmOtherProxy)->ItemIndex;
    //AI(W906-FRW-E029) 20261002 [W906]：以前 EJ1N／DTME08 時 Index 一律＝其他位置；現在 Index 照下拉（可選 EJ1N／DTM，HmLinkIndex 已跟 USE_16_HEATER 對齊）
    p->iIndex = EL<TComboBox>(kHmForm, kHmIndexProxy)->ItemIndex;
    //AI(W906-FRW-E029) 20261002 [W906]：Steven 1002 17:1x「當選擇全機相同, 那就是 index 跟其他部位的分成兩種溫控器進行選擇 EJ1N 跟 DTM 在index站都是可以選的」
    //   ＋答 ST01-E2：EJ1N／DTM 只在「Index 位置溫控器」；「其他位置溫控器」只有 golden 的 5 個（替身也只有 5 項，見 W906_HeaterMixCreateProxies）
    if (!HmValidGolden(p->iOther)) pr->Add("「其他位置溫控器」沒有選廠牌（只能選 golden 的 5 個：TC401、KT4H、E5DC、No Heater、DTK4848）",
                                           "\"other positions\" heater brand is not selected (one of the 5 golden brands)");
    if (!HmValidOpt(p->iIndex)) pr->Add("「Index 位置溫控器」沒有選廠牌", "\"Index positions\" heater brand is not selected");
    for (int ti = 0; ti < eHeaterType_Count; ++ti) {
        p->station[ti] = INVALID_INT_VAL_NEG;
        p->ch[ti]      = INVALID_INT_VAL_NEG;
        p->inUse[ti]   = false;
        p->addrOk[ti]  = false;
        p->custom[ti]  = false;
        if (p->mode == HEATER_INS_MODE_SAME) {                           // 相同：Index 組用 Index 的廠牌、其他用其他的（D-2a；E029 HmBrandSame）
            const int b = HmBrandSame(ti, p->iIndex, p->iOther, u);
            p->brand[ti] = b;
            if (!HmActive(ti, u, dut) || !HmValidOpt(b) || b == NoHeater) continue;
            p->inUse[ti] = true;                                         // 站號＝預設（D-8a 用；相同模式不寫站號，D-7a）
            if (W906_HeaterInsBusOf(b) == W906_HEATER_BUS_COM) { p->station[ti] = W906_HeaterInsDefaultStation(ti, b); p->addrOk[ti] = true; }
            else { int a = 0, c = 0; if (HmDefaultUnitCh(ti, b, &a, &c)) { p->station[ti] = a; p->ch[ti] = c; p->addrOk[ti] = true; } }
            continue;
        }
        if (!s_pHmCb[ti] || !s_pHmAddr[ti] || !s_pHmCh[ti]) {          // （E029 起 71 個都有替身；保險）
            p->brand[ti] = g_tHeaterInsInfo[ti].GetHeaterInsOpt();
            continue;
        }
        const int b = s_pHmCb[ti]->ItemIndex;
        p->brand[ti] = b;
        const AnsiString nm = HmChanName(ti);
        if (!HmValidOpt(b)) { pr->Add(nm + "：沒有選廠牌", nm + ": heater brand is not selected"); continue; }
        if (b == NoHeater) continue;
        bool blank = true, ok = true;
        const int st = HmParseStation(s_pHmAddr[ti]->Text, &blank, &ok);
        if (!ok) {
            pr->Add(nm + "：站號「" + s_pHmAddr[ti]->Text + "」不是數字",
                    nm + ": station \"" + s_pHmAddr[ti]->Text + "\" is not a number");
            continue;
        }
        if (W906_HeaterInsBusOf(b) == W906_HEATER_BUS_COM) {
            p->inUse[ti] = HmActive(ti, u, dut) || !blank;               // E029：沒裝的通道指定了站號＝要用它
            if (!p->inUse[ti]) continue;
            const int def = W906_HeaterInsDefaultStation(ti, b);
            if (blank) { p->station[ti] = def; p->addrOk[ti] = true; continue; }   // D-7a：沒改＝預設值
            if (st < HEATER_INS_ADDR_MIN || st > HmMaxStation(b)) {      // D-7a：1～247；E5DC 1～99
                AnsiString z, e;
                z.sprintf("%s：站號 %d 超出範圍（%s 是 %d～%d）", nm.c_str(), st, HmOptName(b).c_str(), HEATER_INS_ADDR_MIN, HmMaxStation(b));
                e.sprintf("%s: station %d out of range (%s: %d-%d)", nm.c_str(), st, HmOptName(b).c_str(), HEATER_INS_ADDR_MIN, HmMaxStation(b));
                pr->Add(z, e);
                continue;
            }
            p->station[ti] = st;
            p->addrOk[ti]  = true;
            p->custom[ti]  = (st != def);
            continue;
        }
        //AI(W906-FRW-E029) 20261002 [W906]：EJ1N／DTM —— 台號（EJ1N）／內部站號（DTM）＋CH 一組（FileRW/HSys_Heater.h 檔尾 E029）
        const bool ej = (b == W906_HEATER_INS_EJ1N);
        const char* const unitZh = ej ? "台號" : "站號";
        bool cBlank = true, cOk = true;
        const int c = HmParseStation(s_pHmCh[ti]->Text, &cBlank, &cOk);
        if (!cOk) {
            pr->Add(nm + "：CH「" + s_pHmCh[ti]->Text + "」不是數字", nm + ": CH \"" + s_pHmCh[ti]->Text + "\" is not a number");
            continue;
        }
        p->inUse[ti] = HmActive(ti, u, dut) || !blank || !cBlank;
        if (!p->inUse[ti]) continue;
        int du = 0, dc = 0;
        const bool hasDef = HmDefaultUnitCh(ti, b, &du, &dc);
        if (blank && cBlank) {
            if (hasDef) { p->station[ti] = du; p->ch[ti] = dc; p->addrOk[ti] = true; continue; }
            AnsiString z, e;
            z.sprintf("%s：選了 %s，要填%s與 CH（golden 只有 Index 區有 %s 的接線對照）", nm.c_str(), HmOptName(b).c_str(), unitZh,
                      ej ? "EJ1N" : "DTM");
            e.sprintf("%s: %s needs a %s and a CH (golden maps only the Index area)", nm.c_str(), HmOptName(b).c_str(), ej ? "unit" : "station");
            pr->Add(z, e);
            continue;
        }
        if (blank || cBlank) {
            AnsiString z, e;
            z.sprintf("%s：%s 的%s與 CH 要一起填", nm.c_str(), HmOptName(b).c_str(), unitZh);
            e.sprintf("%s: %s %s and CH go together", nm.c_str(), HmOptName(b).c_str(), ej ? "unit" : "station");
            pr->Add(z, e);
            continue;
        }
        if (!HmUnitChInRange(b, st, c)) {
            AnsiString z, e;
            if (ej) {
                z.sprintf("%s：Omron EJ1N 第 %d 台 CH%d 超出範圍（台號＝SW1 %d～%d、CH 1～%d）", nm.c_str(), st, c,
                          HEATER_INS_EJ1N_UNIT_MIN, HEATER_INS_EJ1N_UNIT_MAX, HEATER_INS_EJ1N_CH_MAX);
                e.sprintf("%s: Omron EJ1N unit %d CH%d out of range (unit %d-%d, CH 1-%d)", nm.c_str(), st, c,
                          HEATER_INS_EJ1N_UNIT_MIN, HEATER_INS_EJ1N_UNIT_MAX, HEATER_INS_EJ1N_CH_MAX);
            } else {
                z.sprintf("%s：Delta DTM 站 %d CH%d 超出範圍（內部站號 %d～%d、CH 1～%d）", nm.c_str(), st, c,
                          HEATER_INS_DTM_STATION_MIN, HEATER_INS_DTM_STATION_MAX, HEATER_INS_DTM_CH_MAX);
                e.sprintf("%s: Delta DTM station %d CH%d out of range (station %d-%d, CH 1-%d)", nm.c_str(), st, c,
                          HEATER_INS_DTM_STATION_MIN, HEATER_INS_DTM_STATION_MAX, HEATER_INS_DTM_CH_MAX);
            }
            pr->Add(z, e);
            continue;
        }
        p->station[ti] = st;
        p->ch[ti]      = c;
        p->addrOk[ti]  = true;
        p->custom[ti]  = !(hasDef && st == du && c == dc);
    }
}

// D-8a：「不同」模式列出的通道都在同一個溫控 COM 埠上（[TempCtrl] COM_PORT，9600 8N1，golden V912 rs232.cpp:268-273）——
//   兩個通道同廠牌同站號就擋下，列出是哪兩個。TC401 一台 4 個通道（通道＝序號%4，D-7a 不能指定；golden V912 cpublic.cpp:422-461），
//   所以 TC401 是「同一台、同一個通道」才算重複（同一台不同通道是正常接法）。No Heater（不是溫控器）不算。
//AI(W906-FRW-S166) 20260928 [W906] R113（Steven 20260928：「不同廠牌共用同一個站號要不要擋。 要擋, 目前有遇到 E5DC + KT4H + DTK4848混用了,
//   新的機台架構是改用全機DTM」；D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md「### R113.」＝選項 C）——
//   同一個溫控 COM 埠上，**不同廠牌**只要站號相同也擋，所有廠牌都算（TC401 的站號＝第幾台：E5DC 站號 5 與 TC401 第 5 台也擋）；
//   同廠牌照上面 D-8a 原規則、原訊息（TC401 同一台不同通道仍可以）。一個站號一則訊息，列出用這個站號的全部通道與廠牌。
//   「同一個溫控 COM 埠」＝W906_HeaterInsListed（D-3a）：列出的通道全在 [TempCtrl] COM_PORT 上（golden V912 database.cpp:522
//   sTempComPort → rs232.cpp:258-273 Comm2 → bthermo.cpp DoThermoReal 的輪詢迴圈）；沒列出的不在這條埠上 —— EJ1N／DTME08 的 Index 區
//   走 COM_PORT_OMRON（database.cpp:523、rs232.cpp:176-182；bthermo.cpp:1331-1367 Task=300 跳過），看不見的通道（開機
//   GetCtrlItemVisProp）沒裝。溫控 COM 埠只有這一條，所以不用再按埠分組。
//   「全機相同」模式也檢查（R113 看的是接線，不是畫面模式；D-8 原文「同一個溫控 COM 埠上站號重複」也沒分模式）：站號＝golden 預設
//   （W906_HeaterInsDefaultStation）。同廠牌在這個模式撞不到（KT4H／DTK4848／E5DC＝序號＋1 各不相同；TC401 同一台的通道＝序號%4
//   各不相同），所以原規則照舊只在「不同」模式有作用；不同廠牌只有 Index≠其他、其中一邊是 TC401 才會撞（例 Index＝TC401、其他＝KT4H：
//   Head1～4 是 TC401 第 2 台，HotPlate2 的 KT4H 預設站號也是 2）→ 擋下，訊息請操作員改用「各溫控器不同」指定站號。
//AI(W906-FRW-E029) 20261002 [W906]（Steven 1002：D-8a 的「同一個 COM 埠、同廠牌、同站號」要把 EJ1N 與 DTM 當成各自的匯流排）：
//   ⛔ 上面「＝W906_HeaterInsListed」更正：範圍改成 HmPlan.inUse（這台有裝＋「不同」模式指定了站號的通道），再依廠牌分三條匯流排——
//   溫控 COM 埠（TC401／KT4H／E5DC／DTK4848：D-8a＋R113 照舊）、EJ1N（[TempCtrl] COM_PORT_OMRON：同台同 CH 擋）、
//   DTM（Ethernet 一條連線：同站同 CH 擋）。不同匯流排之間不比（EJ1N 第 1 台與 KT4H 站號 1 不衝突）；R113 只在溫控 COM 埠上。
void HmDuplicates(const HmPlan& p, HmProblems* pr)
{
    struct Ent { int ti, b, st, ch; };
    std::vector<Ent> v, ve, vd;                                         // 溫控 COM 埠／EJ1N／DTM（通道順序）
    for (int ti = 0; ti < eHeaterType_Count; ++ti) {
        if (!p.inUse[ti] || !p.addrOk[ti]) continue;
        const int b = p.brand[ti];
        const int bus = W906_HeaterInsBusOf(b);
        if (bus == W906_HEATER_BUS_COM) {
            const int st = p.station[ti];
            if (st <= 0) continue;
            const Ent e = { ti, b, st, (b == TC401) ? ti % 4 : -1 };
            v.push_back(e);
        } else if (bus == W906_HEATER_BUS_EJ1N || bus == W906_HEATER_BUS_DTM) {
            const Ent e = { ti, b, p.station[ti], p.ch[ti] };
            (bus == W906_HEATER_BUS_EJ1N ? ve : vd).push_back(e);
        }
    }
    // D-8a：同廠牌同站號（TC401：同一台同一通道）
    std::map<long long, int> seen;
    for (std::size_t i = 0; i < v.size(); ++i) {
        const int ti = v[i].ti, b = v[i].b, ch = v[i].ch, st = v[i].st;
        const long long k = ((long long)b * 1000 + st) * 10 + (ch + 1);
        std::map<long long, int>::const_iterator it = seen.find(k);
        if (it == seen.end()) { seen[k] = ti; continue; }
        const AnsiString a = HmChanName(it->second), c = HmChanName(ti);
        AnsiString z, e;
        if (b == TC401) {
            z.sprintf("%s 與 %s 都是 TC401 第 %d 台的通道 %d（同一個溫控 COM 埠上同廠牌同站號，回覆會打架）",
                      a.c_str(), c.c_str(), st, ch);
            e.sprintf("%s and %s are both TC401 unit %d channel %d on the same heater COM port", a.c_str(), c.c_str(), st, ch);
        } else {
            z.sprintf("%s 與 %s 都是 %s、站號 %d（同一個溫控 COM 埠上兩台溫控器同站號，回覆會打架、讀到的溫度可能是另一台的）",
                      a.c_str(), c.c_str(), HmOptName(b).c_str(), st);
            e.sprintf("%s and %s are both %s station %d on the same heater COM port", a.c_str(), c.c_str(), HmOptName(b).c_str(), st);
        }
        pr->Add(z, e);
    }
    // R113：不同廠牌同站號
    std::map<int, std::vector<std::size_t> > byStation;
    for (std::size_t i = 0; i < v.size(); ++i) byStation[v[i].st].push_back(i);
    for (std::map<int, std::vector<std::size_t> >::const_iterator it = byStation.begin(); it != byStation.end(); ++it) {
        const std::vector<std::size_t>& g = it->second;
        bool mixed = false;
        for (std::size_t k = 1; k < g.size() && !mixed; ++k) mixed = v[g[k]].b != v[g[0]].b;
        if (!mixed) continue;
        std::string zl, el;
        for (std::size_t k = 0; k < g.size(); ++k) {
            const Ent& x = v[g[k]];
            AnsiString z1, e1;
            if (x.b == TC401) {
                z1.sprintf("%s（TC401 第 %d 台通道 %d）", HmChanName(x.ti).c_str(), x.st, x.ch);
                e1.sprintf("%s (TC401 unit %d channel %d)", HmChanName(x.ti).c_str(), x.st, x.ch);
            } else {
                z1.sprintf("%s（%s）", HmChanName(x.ti).c_str(), HmOptName(x.b).c_str());
                e1.sprintf("%s (%s)", HmChanName(x.ti).c_str(), HmOptName(x.b).c_str());
            }
            zl += (k ? "、" : "") + std::string(z1.c_str());
            el += (k ? ", " : "") + std::string(e1.c_str());
        }
        const bool same = (p.mode != HEATER_INS_MODE_DIFF);
        AnsiString z, e;
        z.sprintf("站號 %d 被不同廠牌共用：%s（同一個溫控 COM 埠上不同廠牌用同一個站號，回覆會打架，R113 一律擋下）%s",
                  it->first, zl.c_str(),
                  same ? "；「全機相同」模式用的是預設站號，要讓不同廠牌共用這條 COM 埠，請改用「各溫控器不同」模式替通道指定站號" : "");
        e.sprintf("station %d is shared by different brands on the same heater COM port: %s (R113: refused)%s", it->first, el.c_str(),
                  same ? "; \"same for all\" mode uses the default stations -- switch to \"per heater\" mode and set the stations" : "");
        pr->Add(z, e);
    }
    // E029：EJ1N 同台同 CH、DTM 同站同 CH
    for (int k = 0; k < 2; ++k) {
        const std::vector<Ent>& w = k ? vd : ve;
        std::map<int, int> seen2;
        for (std::size_t i = 0; i < w.size(); ++i) {
            const int key = w[i].st * 100 + w[i].ch;
            std::map<int, int>::const_iterator it = seen2.find(key);
            if (it == seen2.end()) { seen2[key] = w[i].ti; continue; }
            const AnsiString a = HmChanName(it->second), c = HmChanName(w[i].ti);
            AnsiString z, e;
            if (k == 0) {
                z.sprintf("%s 與 %s 都是 Omron EJ1N 第 %d 台 CH%d（同一個 EJ1N COM 埠 COM_PORT_OMRON 上同台同 CH，回覆會打架）",
                          a.c_str(), c.c_str(), w[i].st, w[i].ch);
                e.sprintf("%s and %s are both Omron EJ1N unit %d CH%d on the same EJ1N COM port", a.c_str(), c.c_str(), w[i].st, w[i].ch);
            } else {
                z.sprintf("%s 與 %s 都是 Delta DTM 站 %d CH%d（同一條 DTM 乙太網路連線上同站同 CH，讀寫會打架）",
                          a.c_str(), c.c_str(), w[i].st, w[i].ch);
                e.sprintf("%s and %s are both Delta DTM station %d CH%d on the same DTM Ethernet link", a.c_str(), c.c_str(), w[i].st, w[i].ch);
            }
            pr->Add(z, e);
        }
    }
}

// D-6a：HEATER_CTRL_TYPE 存什麼（V899／V906 BCB6 機台與移植樹今天的溫控只讀這一個；V912 拿它判斷「整台沒有加熱」）
//   相同：Index 與其他都是 No Heater 才寫 3；否則寫其他位置的廠牌，其他位置是 No Heater 時寫 Index 的廠牌
//     （不沿用「其他＝HEATER_CTRL_TYPE」：其他＝No Heater 時整台會被當成不加熱，golden V912 bthermo.cpp:1177、rs232.cpp:165）。
//   不同：用最多通道的那個廠牌，不算 No Heater（全部是 No Heater 才寫 3）。先數「不同」模式列出的通道（真的在溫控 COM 埠上輪詢的，
//     D-3a）；列出的都是 No Heater 才數全部 71 個。同票取通道順序先出現的。
//     ⚠ 「數哪些通道」是本檔的解讀（D-6a 原文「用最多通道的那個廠牌」沒寫範圍）：數全部 71 個的話，沒列出的 48 個（多半沿用舊廠牌）
//       會蓋過真的在用的通道，V899 機台就會用錯廠牌。
//   //AI(W906-FRW-E029) 20261002 [W906]：HEATER_CTRL_TYPE 是 golden 的整台廠牌（0～4，Heater Type 單選），**永遠不寫 5／6**——只算溫控
//     COM 埠的廠牌（0、1、2、4）。「列出的通道」改成 HmPlan.inUse。都沒有 COM 埠廠牌、但有 EJ1N／DTM → 沿用檔案的 HEATER_CTRL_TYPE
//     （不是 0、1、2、4 時用 KT4H＝golden 預設）；**不寫 3**：golden main.cpp:10939-10961 EJ1N／DTME08 要 HEATER_CTRL_TYPE≠NoHeater 才啟動，
//     IsNoHeaterMachine 也會讓整台不加熱。全部是 No Heater 才寫 3。
int HmCtrlTypeFor(const HmPlan& p)
{
    const int iFile = HmCtrlTypeNoWrite();
    const int keep = HmComCtrl(iFile) ? iFile : KT4H;
    if (p.mode == HEATER_INS_MODE_SAME) {
        if (HmComCtrl(p.iOther)) return p.iOther;
        if (HmComCtrl(p.iIndex)) return p.iIndex;
        if (p.iIndex == NoHeater && p.iOther == NoHeater) return NoHeater;
        return keep;
    }
    for (int pass = 0; pass < 2; ++pass) {
        int cnt[W906_HEATER_INS_OPT_COUNT] = {0}, first[W906_HEATER_INS_OPT_COUNT];
        for (int b = 0; b < W906_HEATER_INS_OPT_COUNT; ++b) first[b] = eHeaterType_Count;
        for (int ti = 0; ti < eHeaterType_Count; ++ti) {
            if (pass == 0 && !p.inUse[ti]) continue;
            const int b = p.brand[ti];
            if (!HmComCtrl(b)) continue;
            if (cnt[b]++ == 0) first[b] = ti;
        }
        int best = -1;
        for (int b = 0; b < W906_HEATER_INS_OPT_COUNT; ++b)
            if (cnt[b] > 0 && (best < 0 || cnt[b] > cnt[best] || (cnt[b] == cnt[best] && first[b] < first[best]))) best = b;
        if (best >= 0) return best;
    }
    for (int ti = 0; ti < eHeaterType_Count; ++ti) if (HmIsAreaOpt(p.brand[ti])) return keep;
    return NoHeater;
}

// //AI(W906-FRW-E029) 20261002 [W906]：EJ1N／DTM 的連線設定給頁面唯讀顯示（ST01-E 1002：「golden 讀這些，頁面要看得到」）。
//   EJ1N：[TempCtrl] COM_PORT_OMRON（golden database.cpp:523，預設 COM7）——HW.HandlerSys「Com Port」分頁的 cbComTempOmron 可改，
//     頁面直接讀那個元件。DTM：DTME08_Control.ini [SocketSetting] asAddress／asPort——golden 讀 exe 資料夾（uDTME08Control.h:54-56），
//     移植樹讀 system\（EJ1N/uDTME08Control.h:135-144，GATE (1)，todo D-035）；兩棵樹讀不同檔，本頁**不寫**這個檔（ST01-E：要加寫檔先問）。
//   純讀：存在才用 write-through TIniFile 讀（不建檔、不寫）。測試縫 W906_DTME08INI_PATH（只讀）。程式內預設 127.0.0.1:59999
//   （EJ1N/uSocketServerClient.cpp:202-203）。
//AI(W906-I03-W61) 20261005 (Ifor01): W-61 (b), Ifor 1005 -- the default moved to D:\HT9045\EXE\DTME08_Control.ini, the file the DTM
//  core reads since D-035 (MR !115, EJ1N/uDTME08Control.h:135-144 = golden's exe folder). The "移植樹讀 system\ ... todo D-035" above
//  is history: before this line the page showed a different file than the one the machine uses. Test seam unchanged.
std::string HmDtmIniPath()
{
    const char* e = std::getenv("W906_DTME08INI_PATH");
    return (e && *e) ? std::string(e) : std::string("D:\\HT9045\\EXE\\DTME08_Control.ini");
}

// editlist.get 的 extra.heater.mix（頁面 ht9045_hsys_heater_c.js 照這份建畫面）。純讀：HEATER_CTRL_TYPE 用不寫檔的讀法。
void W906_HeaterMixExtraJson(webbridge::JsonWriter& w)
{
    const int u = HmProxyU16();
    w.Key("mix").BeginObject();
    w.Key("ruling").String("Q34 plan D (RULINGS_20260926 S166; D-2=A per RULINGS_20260927 #7 #35; D-1b D-3a D-4a D-5a D-6a D-7a D-8a D-9a) "
                           "+ Q15 (S137: missing key = 3 No Heater, page open writes nothing) + Q14 (S136: Heater Type click does not write) "
                           "+ E029 / Q72 (Steven 1002: EJ1N = 5, DTM = 6, all 71 channels, Index area auto-linked with USE_16_HEATER)");
    w.Key("modeProxy").String(kHmModeProxy);
    w.Key("indexProxy").String(kHmIndexProxy);
    w.Key("otherProxy").String(kHmOtherProxy);
    w.Key("u16Proxy").String(kHmU16Proxy);
    w.Key("mode").Number((wb_int64)g_iHeaterInsMode);
    w.Key("indexOpt").Number((wb_int64)g_iHeaterInsIndexOpt);
    w.Key("otherOpt").Number((wb_int64)g_iHeaterInsOtherOpt);
    //AI(W906-FRW-E029) 20261002：⛔ "indexLocked"／"indexLockedLabel"（EJ1N／DTME08 時停用 Index 下拉）拿掉——Steven 1002：Index 下拉自己選 EJ1N／DTM
    w.Key("use16Heater").Number((wb_int64)u);                           // E029：頁面的 rgHeater（＝檔案值），以前是全域
    w.Key("dutProxy").String("rgUse4DUT");                              // E029：Socket／DUT1～4 互斥跟著它（Steven 1002 17:1x）
    w.Key("use4Dut").Number((wb_int64)HmProxyDut());
    w.Key("dutEnum").BeginObject();                                     // golden MachineType.h:742-744（畫面順序 1 EA、4 EA、2 EA）
    w.Key("one").Number((wb_int64)eDut1ea);
    w.Key("four").Number((wb_int64)eDut4ea);
    w.Key("two").Number((wb_int64)eDut2ea);
    w.EndObject();
    w.Key("use16Open").Number((wb_int64)s_iHmOpenU16);
    w.Key("heaterCtrlType").Number((wb_int64)HmCtrlTypeNoWrite());
    w.Key("hint").String(kHmHintZh);
    w.Key("hintEn").String(kHmHintEn);
    w.Key("brand").BeginObject();
    w.Key("tc401").Number((wb_int64)TC401);
    w.Key("kt4h").Number((wb_int64)KT4H);
    w.Key("e5dc").Number((wb_int64)E5DC);
    w.Key("noHeater").Number((wb_int64)NoHeater);
    w.Key("dtk4848").Number((wb_int64)DTK4848);
    w.Key("ej1n").Number((wb_int64)W906_HEATER_INS_EJ1N);
    w.Key("dtm").Number((wb_int64)W906_HEATER_INS_DTM);
    w.EndObject();
    w.Key("options").BeginArray();                                      // Index 下拉與逐通道下拉：7 個
    for (int j = 0; j < W906_HEATER_INS_OPT_COUNT; ++j) w.String(g_W906HeaterInsOptStr[j].c_str());
    w.EndArray();
    w.Key("otherOptions").BeginArray();                                 // 「其他位置溫控器」：golden 的 5 個（Steven 1002 17:1x）
    for (int j = 0; j < eHeaterInsOpt_Count; ++j) w.String(g_HeaterInsOptStr[j].c_str());
    w.EndArray();
    w.Key("stationMin").Number((wb_int64)HEATER_INS_ADDR_MIN);
    w.Key("stationMax").Number((wb_int64)HEATER_INS_ADDR_MAX);
    w.Key("stationMaxE5DC").Number((wb_int64)HEATER_INS_ADDR_MAX_E5DC);
    w.Key("ej1nUnitMin").Number((wb_int64)HEATER_INS_EJ1N_UNIT_MIN);
    w.Key("ej1nUnitMax").Number((wb_int64)HEATER_INS_EJ1N_UNIT_MAX);
    w.Key("ej1nChMax").Number((wb_int64)HEATER_INS_EJ1N_CH_MAX);
    w.Key("dtmStationMin").Number((wb_int64)HEATER_INS_DTM_STATION_MIN);
    w.Key("dtmStationMax").Number((wb_int64)HEATER_INS_DTM_STATION_MAX);
    w.Key("dtmChMax").Number((wb_int64)HEATER_INS_DTM_CH_MAX);
    w.Key("indexGroup").BeginArray();                                   // D-2a 的 36 個通道（頁面灰字列出）
    for (int ti = 0; ti < eHeaterType_Count; ++ti)
        if (W906_HeaterInsIsIndexGroup(ti)) w.String(HmChanName(ti).c_str());
    w.EndArray();
    w.Key("conn").BeginObject();                                        // E029：連線設定（唯讀顯示）
    w.Key("comTempKey").String("[TempCtrl] COM_PORT");
    w.Key("comTempWidget").String("cbComTemp");
    w.Key("comOmronKey").String("[TempCtrl] COM_PORT_OMRON");
    w.Key("comOmronWidget").String("cbComTempOmron");
    {
        const std::string path = HmDtmIniPath();
        std::ifstream f(path.c_str());
        const bool exists = f.good();
        f.close();
        AnsiString addr, port;
        if (exists) {
            TIniFile ini(AnsiString(path.c_str()));
            addr = ini.ReadString("SocketSetting", "asAddress", "");
            port = ini.ReadString("SocketSetting", "asPort", "");
        }
        w.Key("dtm").BeginObject();
        w.Key("file").String(path.c_str());
        w.Key("exists").Bool(exists);
        w.Key("address").String(addr.c_str());
        w.Key("port").String(port.c_str());
        w.Key("defAddress").String("127.0.0.1");
        w.Key("defPort").String("59999");
        w.Key("golden").String("golden V912 reads DTME08_Control.ini from the exe folder (EJ1N/uDTME08Control.h:54); this tree reads system\\ "
                               "(EJ1N/uDTME08Control.h:135-144, GATE (1), todo D-035); this page does not write it");
        w.EndObject();
    }
    w.EndObject();
    w.Key("rows").BeginArray();                                         // E029：71 個通道全列（listed＝D-3a 的舊旗標，只留著對照）
    for (int ti = 0; ti < eHeaterType_Count; ++ti) {
        if (!s_pHmCb[ti] || !s_pHmAddr[ti] || !s_pHmCh[ti]) continue;
        w.BeginObject();
        w.Key("typeIdx").Number((wb_int64)ti);
        w.Key("name").String(HmChanName(ti).c_str());
        w.Key("saveName").String(g_tHeaterInsInfo[ti].GetSaveName().c_str());
        w.Key("addrKey").String(HmAddrKey(ti).c_str());
        w.Key("chKey").String(HmChKey(ti).c_str());
        w.Key("cb").String(W906_HeaterInsProxyName(ti, true).c_str());
        w.Key("lb").String(g_tHeaterInsInfo[ti].GetOccupy() ? W906_HeaterInsProxyName(ti, false).c_str() : "");
        w.Key("addr").String(HmAddrProxy(ti).c_str());
        w.Key("chEd").String(HmChProxy(ti).c_str());
        w.Key("golden").Bool(g_tHeaterInsInfo[ti].GetOccupy());       // golden 有下拉（occupy）
        w.Key("goldenShow").Bool(g_tHeaterInsInfo[ti].GetOccupy() && g_tHeaterInsInfo[ti].GetShow());   // golden 看得到＝這台有裝
        w.Key("zone").Number((wb_int64)((ti >= tcAa1 && ti <= tcBd2) ? 1 : (ti >= tcAe1 && ti <= tcBh2) ? 2 : 0));   // Index 區：1＝Aa1～Bd2、2＝Ae1～Bh2
        w.Key("indexGroup").Bool(W906_HeaterInsIsIndexGroup(ti));
        w.Key("pair").String(HmPair(ti));                                               // E029：兩組互斥（頁面即時顯示／隱藏）
        w.Key("active").Bool(HmActive(ti, u, HmProxyDut()));                            // E029：這台有裝（開頁時的 rgHeater／rgUse4DUT）
        w.Key("listed").Bool(W906_HeaterInsListed(ti));
        w.Key("value").Number((wb_int64)g_tHeaterInsInfo[ti].GetHeaterInsOpt());
        w.Key("station").Number((wb_int64)g_iHeaterInsAddr[ti]);
        w.Key("ch").Number((wb_int64)g_iHeaterInsCh[ti]);
        w.Key("defTc401").Number((wb_int64)W906_HeaterInsDefaultStation(ti, TC401));   // TC401：第 (序號÷4)+1 台
        w.Key("defOther").Number((wb_int64)W906_HeaterInsDefaultStation(ti, KT4H));    // KT4H／DTK4848／E5DC：序號＋1
        w.Key("tc401Ch").Number((wb_int64)(ti % 4));                                    // TC401 的通道（D-7a 不能指定）
        int a = -1, c = -1;
        if (!HmDefaultUnitCh(ti, W906_HEATER_INS_EJ1N, &a, &c)) { a = -1; c = -1; }
        w.Key("ej1nUnit").Number((wb_int64)a);                                          // E029：Index 區的 golden 接線（其他通道 -1＝沒有預設）
        w.Key("ej1nCh").Number((wb_int64)c);
        if (!HmDefaultUnitCh(ti, W906_HEATER_INS_DTM, &a, &c)) { a = -1; c = -1; }
        w.Key("dtmStation").Number((wb_int64)a);
        w.Key("dtmCh").Number((wb_int64)c);
        w.EndObject();
    }
    w.EndArray();
    w.EndObject();
}
}  // namespace

bool W906_HeaterInsIsIndexGroup(int iTypeIdx)       // D-2a（RULINGS_20260927 第 7 條第 35 題）：36 個
{
    return (iTypeIdx >= tcHead1 && iTypeIdx <= tcHead4) || HmIsZone(iTypeIdx);
}
bool W906_HeaterInsHasRow(int iTypeIdx)
{
    //AI(W906-FRW-E029) 20261002 [W906]：71 個全部（Steven 1002「這些全部都要可以選,所以數量也是少了」）；以前＝有下拉的 23 個＋Index 32 區
    return iTypeIdx >= 0 && iTypeIdx < eHeaterType_Count;
}
// D-3a：golden 的 23 個有下拉的通道照 golden 可見條件（開機 GetCtrlItemVisProp，golden 同：建構子只算一次）＋依 USE_16_HEATER
//   真的走溫控 COM 埠的 Index 區：16 Heaters（eht16Heater）＝Aa1～Bd2、32 Heaters with KT4H（eht32HeaterKT4H）＝Aa1～Bh2
//   （golden V912 bthermo.cpp:1331-1367：EJ1N／DTME08 版本的 Index 區 Task=300 跳過、不走 COM 迴圈）。USE_16_HEATER 用目前的全域
//   （存檔後 ExitBtnClick 資料半段重讀）。
//   ⛔ 20261002 更正（E029）：頁面已經 71 個全列，存檔檢查改看 HmPlan.inUse＋匯流排；這支只留著當「golden 溫控 COM 埠的輪詢範圍」。
bool W906_HeaterInsListed(int iTypeIdx)
{
    if (iTypeIdx < 0 || iTypeIdx >= eHeaterType_Count) return false;
    if (g_tHeaterInsInfo[iTypeIdx].GetOccupy()) return g_tHeaterInsInfo[iTypeIdx].GetShow();
    if (iTypeIdx >= tcAa1 && iTypeIdx <= tcBd2) return USE_16_HEATER == eht16Heater || USE_16_HEATER == eht32HeaterKT4H;
    if (iTypeIdx >= tcAe1 && iTypeIdx <= tcBh2) return USE_16_HEATER == eht32HeaterKT4H;
    return false;
}
//AI(W906-FRW-E029) 20261002 [W906]：⛔ W906_HeaterInsIndexLocked() 拿掉（S166 時 St01 的解讀：USE_16_HEATER 是 EJ1N／DTME08 → 「全機相同」的
//   Index 下拉停用、Index 跟著其他位置）。Steven 1002 17:1x「當選擇全機相同, 那就是 index 跟其他部位的分成兩種溫控器進行選擇 EJ1N 跟 DTM 在index站
//   都是可以選的」⇒ Index 下拉自己選 EJ1N／DTM，並連動 USE_16_HEATER（HmLinkIndex）。
// golden 算的站號（golden 沒有「指定站號」）：KT4H／DTK4848／E5DC＝序號＋1（golden V912 cpublic.cpp:185、:196、:210-226、:467、:484）；
//   TC401＝第 (序號÷4)+1 台（bthermo.cpp:2522、:2556 → cpublic.cpp:422-461）。No Heater／不合法的廠牌也回序號＋1（只給頁面顯示）。
int W906_HeaterInsDefaultStation(int iTypeIdx, int iHeaterInsOpt)
{
    return (iHeaterInsOpt == TC401) ? iTypeIdx / 4 + 1 : iTypeIdx + 1;
}
int W906_HeaterStationIdx(int iTypeIdx)
{
    if (iTypeIdx < 0 || iTypeIdx >= eHeaterType_Count) return iTypeIdx;
    const int opt = g_tHeaterInsInfo[iTypeIdx].GetHeaterInsOpt();
    if (W906_HeaterInsBusOf(opt) != W906_HEATER_BUS_COM) return iTypeIdx;   //AI(W906-FRW-E029) 20261002：EJ1N／DTM 用 W906_HeaterInsUnitCh
    int st = W906_HeaterInsDefaultStation(iTypeIdx, opt);
    if (g_iHeaterInsMode == HEATER_INS_MODE_DIFF) {
        const int a = g_iHeaterInsAddr[iTypeIdx];
        if (a >= HEATER_INS_ADDR_MIN && a <= HmMaxStation(opt)) st = a;
    }
    return st - 1;
}
// //AI(W906-FRW-E029) 20261002 [W906]：匯流排、EJ1N／DTM 位址、USE_16_HEATER 連動（宣告與規則：FileRW/HSys_Heater.h 檔尾 E029）
int W906_HeaterInsBusOf(int iHeaterInsOpt)
{
    if (HmComCtrl(iHeaterInsOpt)) return W906_HEATER_BUS_COM;
    if (iHeaterInsOpt == W906_HEATER_INS_EJ1N) return W906_HEATER_BUS_EJ1N;
    if (iHeaterInsOpt == W906_HEATER_INS_DTM) return W906_HEATER_BUS_DTM;
    return W906_HEATER_BUS_NONE;
}
int W906_HeaterInsBus(int iTypeIdx)
{
    if (iTypeIdx < 0 || iTypeIdx >= eHeaterType_Count) return W906_HEATER_BUS_NONE;
    return W906_HeaterInsBusOf(g_tHeaterInsInfo[iTypeIdx].GetHeaterInsOpt());
}
bool W906_HeaterInsUnitCh(int iTypeIdx, int* piUnit, int* piCh)
{
    if (iTypeIdx < 0 || iTypeIdx >= eHeaterType_Count || !piUnit || !piCh) return false;
    const int b = g_tHeaterInsInfo[iTypeIdx].GetHeaterInsOpt();
    if (!HmIsAreaOpt(b)) return false;
    if (g_iHeaterInsMode == HEATER_INS_MODE_DIFF && HmUnitChInRange(b, g_iHeaterInsAddr[iTypeIdx], g_iHeaterInsCh[iTypeIdx])) {
        *piUnit = g_iHeaterInsAddr[iTypeIdx];
        *piCh   = g_iHeaterInsCh[iTypeIdx];
        return true;
    }
    return HmDefaultUnitCh(iTypeIdx, b, piUnit, piCh);
}
int W906_HeaterInsU16Area(int iU16)
{
    if (iU16 == eht16HeaterEJ1N || iU16 == eht32HeaterEJ1N) return W906_HEATER_INS_EJ1N;
    if (iU16 == eht16HeaterDTME08 || iU16 == eht32HeaterDTME08) return W906_HEATER_INS_DTM;
    return -1;
}
int W906_HeaterInsU16For(int iIndexAreaOpt, int iU16Now)
{
    const bool b32 = HmCount(iU16Now) == 32;                            // 組數不變；0（4 Heaters）→ 16（Steven 1002）
    if (iIndexAreaOpt == W906_HEATER_INS_EJ1N) return b32 ? eht32HeaterEJ1N : eht16HeaterEJ1N;
    if (iIndexAreaOpt == W906_HEATER_INS_DTM)  return b32 ? eht32HeaterDTME08 : eht16HeaterDTME08;
    if (iU16Now == eht16HeaterEJ1N || iU16Now == eht16HeaterDTME08) return eht16Heater;       // 改回溫控 COM 埠：16 → 1
    if (iU16Now == eht32HeaterEJ1N || iU16Now == eht32HeaterDTME08) return eht32HeaterKT4H;   //                   32 → 4
    return iU16Now;
}

// HeaterInsOpt_Read 的本體（Q15：不寫檔；方案 D 讀檔規則，decisions-decided Q34 ②「讀檔規則」、③、④）
//   //AI(W906-FRW-E029) 20261002：u＝這次要套的 USE_16_HEATER（溫控底層＝全域；開頁＝rgHeater 替身，golden :260 剛讀的檔案值）
static void HmReadFile(int u)
{
    const int iCtrl = HmCtrlTypeNoWrite();                              // golden :52（預設 KT4H；不寫檔）
    int  val[eHeaterType_Count];
    bool has[eHeaterType_Count];
    for (int ti = 0; ti < eHeaterType_Count; ++ti) {
        bool h = false;
        const int raw = HmReadInt(g_tHeaterInsInfo[ti].GetSaveName(), INVALID_INT_VAL_NEG, &h);
        has[ti] = h;
        val[ti] = (h && raw == INVALID_INT_VAL_NEG) ? iCtrl : raw;      // D-5a：檔案裡的 -9999 照 V912＝HEATER_CTRL_TYPE（golden :57-58）
    }
    // 模式：HeaterInsMode；缺鍵（V899／V912 的舊檔）照 D-1b —— 有下拉的 23 個裡 0～4 的值彼此不同＝不同，否則＝相同
    bool hadMode = false;
    const int m = HmReadInt("HeaterInsMode", HEATER_INS_MODE_SAME, &hadMode);
    int common = -1;
    bool differ = false;
    for (int ti = 0; ti < eHeaterType_Count; ++ti) {
        if (!g_tHeaterInsInfo[ti].GetOccupy() || !has[ti] || !HmValidGolden(val[ti])) continue;
        if (common < 0) common = val[ti];
        else if (val[ti] != common) differ = true;
    }
    const int mode = hadMode ? (m == HEATER_INS_MODE_DIFF ? HEATER_INS_MODE_DIFF : HEATER_INS_MODE_SAME)
                             : (differ ? HEATER_INS_MODE_DIFF : HEATER_INS_MODE_SAME);
    // Index／其他：缺鍵時 —— 相同＝23 個的共同值（一個都沒有＝V899 檔 → HEATER_CTRL_TYPE）；不同＝HEATER_CTRL_TYPE
    const int fb = (mode == HEATER_INS_MODE_SAME && common >= 0) ? common : iCtrl;
    int iIndex = HmReadInt("HeaterInsIndexOpt", fb);
    int iOther = HmReadInt("HeaterInsOtherOpt", fb);
    if (HmIsAreaOpt(iOther)) iOther = KT4H;                            // E029：「其他位置」只有 golden 的 5 個（Steven 1002 17:1x）
    //AI(W906-FRW-E029) 20261002 [W906]（Q72）：⛔ 以前「EJ1N／DTME08 → Index 跟著其他位置」（W906_HeaterInsIndexLocked 的舊解讀）；
    //   改成讀檔以 USE_16_HEATER 為準（golden 真正看的是它）：EJ1N／DTME08 → Index 區＝EJ1N／DTM；溫控 COM 埠的版本卻讀到 5／6 → KT4H。
    const int code = W906_HeaterInsU16Area(u);
    if (code > 0) iIndex = code;
    else if (HmIsAreaOpt(iIndex)) iIndex = KT4H;
    for (int ti = 0; ti < eHeaterType_Count; ++ti) {
        int b;
        if (mode == HEATER_INS_MODE_SAME) b = HmBrandSame(ti, iIndex, iOther, u);   // 71 個 HeaterInsOpt_ 只是 V912 的相容副本
        else {
            b = has[ti] ? val[ti] : NoHeater;                                         // Q15：缺鍵＝3 No Heater
            if (HmIsZone(ti) && HmInArea(ti, u)) {                                    // E029：Index 區跟 USE_16_HEATER 對齊
                if (code > 0) b = code;
                else if (HmIsAreaOpt(b)) b = KT4H;
            }
        }
        g_tHeaterInsInfo[ti].m_iHeaterInsOpt = b;
        g_iHeaterInsAddr[ti] = HmReadInt(HmAddrKey(ti), INVALID_INT_VAL_NEG);   // 缺鍵＝預設站號（只在「不同」模式用，W906_HeaterStationIdx）
        g_iHeaterInsCh[ti]   = HmReadInt(HmChKey(ti), INVALID_INT_VAL_NEG);     // E029：EJ1N／DTM 的 CH
    }
    g_iHeaterInsMode     = mode;
    g_iHeaterInsIndexOpt = iIndex;
    g_iHeaterInsOtherOpt = iOther;
}
void W906_HeaterMixReadFile()
{
    HmReadFile(USE_16_HEATER);                                          // 溫控底層（golden bthermo.cpp:1167-1175）：全域的 USE_16_HEATER
}

// golden LoaderSystemSet :262-278 的取代（開頁；產生器 tools/editlist/HSys.py）。跟 golden 同一個骨架：讀逐通道表 → 擋事件 →
//   回填 Heater Type（有下拉的全同＝那個廠牌，否則＝HEATER_CTRL_TYPE）→ 回填各通道下拉 → 恢復事件；多了方案 D 的替身。
//   偏離 golden（Q15）：讀檔不寫檔（HeaterInsOpt_Read 與 :271 的 HEATER_CTRL_TYPE 都用不寫檔的讀法）。
void W906_HeaterMixLoaderSystemSet()
{
    //AI(W906-FRW-E029) 20261002：golden :265 HeaterInsOpt_Read() → 同一個本體 HmReadFile，USE_16_HEATER 用 golden :260 剛讀進 rgHeater 的
    //   檔案值（頁面上看到的那個；全域要等存檔後 ExitBtnClick 才重讀），並記下開頁時的值（HmLinkIndex 判斷誰改了）。
    const int u = HmProxyU16();
    HmReadFile(u);                                                              // golden :265（Q15：不寫檔）
    s_iHmOpenU16 = u;
    s_bSuppressHeaterTypeEvent=true;                                            // golden :266
    { int iFirst=-1;
      TRadioGroup* rg = EL<TRadioGroup>(kHmForm, "rgHeaterType");
      if(IsAllSame_HeaterInsOpt(true,&iFirst) && 0<=iFirst && iFirst<eHeaterInsOpt_Count)   // golden :268-269
          rg->ItemIndex=iFirst;
      else
          rg->ItemIndex=HmCtrlTypeNoWrite();                                    // golden :271（CheckAndReadIniDataGeneral → 不寫檔的讀法）
      for(int ti=0; ti<eHeaterType_Count; ++ti)                                 // golden :272-276（有下拉的 23 個）＋其他 48 個（E029）
          if(NULL!=s_pHmCb[ti]) filerw::ELComboIndex(s_pHmCb[ti], g_tHeaterInsInfo[ti].GetHeaterInsOpt()); }
    HmFillMixProxies(u);
    s_bSuppressHeaterTypeEvent=false;                                           // golden :277
}

// golden rgHeaterTypeClick :1529-1538 的取代（BeforeApply 重播；:1522-1527 的擋事件與範圍保護照 golden 留在產生檔）。
//   方案 D ⑤：點 Heater Type＝「全機相同，Index 與其他都設成這個廠牌」。golden :1531 看不到下拉的通道記憶體放 -9999 → D ⑤ 放實際廠牌；
//   golden :1536／:1538 寫檔 → Q14＝B 不寫（整頁存檔 W906_HeaterMixSave 才寫）。
//   //AI(W906-FRW-E029) 20261002 [W906]：golden 按 Heater Type 不動 USE_16_HEATER ⇒ USE_16_HEATER 是 EJ1N／DTME08 時 Index 留 EJ1N／DTM
//   （Index 區照舊），其他位置與 Head1～4＝點的廠牌（HmBrandSame）；不是時跟以前一樣 Index＝其他＝點的廠牌。
void W906_HeaterMixTypeClick(int iOpt)
{
    const int u = HmProxyU16();
    const int code = W906_HeaterInsU16Area(u);
    const int iIndex = code > 0 ? code : iOpt;
    for (int ti = 0; ti < eHeaterType_Count; ++ti) {
        const int b = HmBrandSame(ti, iIndex, iOpt, u);
        g_tHeaterInsInfo[ti].m_iHeaterInsOpt = b;                               // golden :1531-1532
        if (NULL != s_pHmCb[ti]) filerw::ELComboIndex(s_pHmCb[ti], b);          // golden :1533-1535（＋其他 48 個）
    }
    g_iHeaterInsMode = HEATER_INS_MODE_SAME;
    g_iHeaterInsIndexOpt = iIndex;
    g_iHeaterInsOtherOpt = iOpt;
    EL<TRadioGroup>(kHmForm, kHmModeProxy)->ItemIndex = HEATER_INS_MODE_SAME;
    filerw::ELComboIndex(EL<TComboBox>(kHmForm, kHmIndexProxy), iIndex);
    filerw::ELComboIndex(EL<TComboBox>(kHmForm, kHmOtherProxy), iOpt);
}

// 存檔前檢查（SaveFlow 在 golden SaveBtnClick 之前呼叫）：D-7a 範圍、選項不合法、D-8a 重複 → false＋ELMessage，什麼都不寫
//   //AI(W906-FRW-E029) 20261002：先 HmLinkIndex（Index 區 EJ1N／DTM ⇄ USE_16_HEATER；會改 rgHeater 等替身，golden SaveSystemSet :777
//   接著寫的 USE_16_HEATER 就是連動後的值）；伺服器自己連動了才在 ack 說明（頁面已連動時什麼都不說）。
bool W906_HeaterMixSaveCheck()
{
    HmProblems pr, notes;
    HmLinkIndex(&pr, &notes);
    HmPlan p;
    if (pr.zh.empty()) {
        HmFromProxies(&p, &pr);
        HmDuplicates(p, &pr);
    }
    if (pr.zh.empty()) {
        for (std::size_t i = 0; i < notes.zh.size(); ++i) filerw::ELMessage(AnsiString(notes.en[i].c_str()), AnsiString(notes.zh[i].c_str()));
        return true;
    }
    std::string z = "Heater 分頁的設定有問題，這次不存檔（整頁什麼都沒寫）：";
    std::string e = "Heater settings refused, nothing on this page was written:";
    for (std::size_t i = 0; i < pr.zh.size(); ++i) {
        z += (i ? "；" : "") + pr.zh[i];
        e += (i ? "; " : " ") + pr.en[i];
    }
    filerw::ELMessage(AnsiString(e.c_str()), AnsiString(z.c_str()));
    return false;
}

// golden SaveSystemSet :779-794 的取代（答 YES 之後；產生器 tools/editlist/HSys.py）。寫的鍵（順序）：
//   HeaterInsMode、HeaterInsIndexOpt、HeaterInsOtherOpt → 71 個 HeaterInsOpt_<通道>（實際廠牌，D-4a；golden :782-788 看不到下拉的寫 -9999）
//   → 「不同」模式列出的通道 HeaterInsAddr_<通道>（D-7a；沒改＝預設值）→ HEATER_CTRL_TYPE（D-6a；golden :789-793）。
//   缺的鍵由 WriteIniDataGeneral 加在 [TempCtrl] 段尾（移植樹 vclcompat/IniFiles.cpp:500，同 BCB6 WritePrivateProfileString）。
//   記憶體同步放實際廠牌（D ⑤；golden :787 放 -9999）。
//   //AI(W906-FRW-E029) 20261002 [W906]：站號的範圍改成 HmPlan.inUse（這台有裝＋指定了站號的通道），EJ1N／DTM 另寫 HeaterInsCh_<通道>
//   （同一個通道 Addr 之後緊接 Ch）；最後再寫一次 [System] USE_16_HEATER（＝golden SaveSystemSet :777 剛寫的 rgHeater，同值）——
//   Steven 1002「使用 EJ1N 或是 DTM 應該是要設定兩個變數」：兩個鍵一定在同一次存檔裡，就算有人只呼叫這一支。
void W906_HeaterMixSave()
{
    HmProblems pr, notes;
    HmLinkIndex(&pr, &notes);                                           // SaveCheck 做過（冪等）
    HmPlan p;
    if (pr.zh.empty()) {
        HmFromProxies(&p, &pr);
        HmDuplicates(p, &pr);
    }
    if (!pr.zh.empty()) {   // SaveFlow 已先擋（W906_HeaterMixSaveCheck）；走到這裡＝有人繞過檢查 → 溫控鍵不寫
        filerw::ELTodo("W906_HeaterMixSave: heater settings failed the D-7a/D-8a/E029 check -- [TempCtrl] heater keys were NOT written");
        return;
    }
    WriteIniDataGeneral("TempCtrl", "HeaterInsMode",     p.mode);
    WriteIniDataGeneral("TempCtrl", "HeaterInsIndexOpt", p.iIndex);
    WriteIniDataGeneral("TempCtrl", "HeaterInsOtherOpt", p.iOther);
    for (int ti = 0; ti < eHeaterType_Count; ++ti) {
        g_tHeaterInsInfo[ti].m_iHeaterInsOpt = p.brand[ti];
        WriteIniDataGeneral("TempCtrl", g_tHeaterInsInfo[ti].m_asSaveName, p.brand[ti]);
    }
    bool bStation = false;
    if (p.mode == HEATER_INS_MODE_DIFF)
        for (int ti = 0; ti < eHeaterType_Count; ++ti) {
            if (!p.inUse[ti] || !p.addrOk[ti]) continue;
            WriteIniDataGeneral("TempCtrl", HmAddrKey(ti), p.station[ti]);
            g_iHeaterInsAddr[ti] = p.station[ti];
            if (HmIsAreaOpt(p.brand[ti])) {
                WriteIniDataGeneral("TempCtrl", HmChKey(ti), p.ch[ti]);
                g_iHeaterInsCh[ti] = p.ch[ti];
            }
            bStation = bStation || p.custom[ti];
        }
    WriteIniDataGeneral("TempCtrl", "HEATER_CTRL_TYPE", HmCtrlTypeFor(p));
    WriteIniDataGeneral("System", "USE_16_HEATER", p.u16);              // E029：同 golden :777 的值（見上）
    g_iHeaterInsMode     = p.mode;
    g_iHeaterInsIndexOpt = p.iIndex;
    g_iHeaterInsOtherOpt = p.iOther;
    s_iHmOpenU16         = p.u16;
    // D-9a（E029：「相同」模式 Index＝EJ1N／DTM 是 golden 本來就有的 USE_16_HEATER，不算）
    const bool sameMix = p.mode == HEATER_INS_MODE_SAME && p.iIndex != p.iOther && !HmIsAreaOpt(p.iIndex);
    if (p.mode == HEATER_INS_MODE_DIFF || sameMix || bStation)
        filerw::ELMessage(kHmHintEn, kHmHintZh);
}

// 開機建方案 D 的替身（W906_HeaterInsCreateProxies 的尾巴＝golden 建構子的時序）；純記憶體，不讀不寫檔
static void W906_HeaterMixCreateProxies()
{
    TRadioGroup* rm = EL<TRadioGroup>(kHmForm, kHmModeProxy);
    rm->Items->Clear();
    rm->Items->Add("Same brand for all heaters");        // 頁面顯示「全機相同」
    rm->Items->Add("Different brand per heater");        // 頁面顯示「各溫控器不同」
    {   //AI(W906-FRW-E029) 20261002：Index 7 個（＋EJ1N／DTM）、其他位置 golden 5 個（Steven 1002 17:1x）
        TComboBox* ci = EL<TComboBox>(kHmForm, kHmIndexProxy);
        ci->Items->Clear();
        for (int j = 0; j < W906_HEATER_INS_OPT_COUNT; ++j) ci->Items->Add(g_W906HeaterInsOptStr[j]);
        TComboBox* co = EL<TComboBox>(kHmForm, kHmOtherProxy);
        co->Items->Clear();
        for (int j = 0; j < eHeaterInsOpt_Count; ++j) co->Items->Add(g_HeaterInsOptStr[j]);
    }
    {
        const char* const pr[3][2] = { { kHmModeProxy, "grpHeater" }, { kHmIndexProxy, "grpHeater" }, { kHmOtherProxy, "grpHeater" } };
        filerw::ELSetParents(kHmForm, pr, 3);
    }
    for (int ti = 0; ti < eHeaterType_Count; ++ti) {
        if (!W906_HeaterInsHasRow(ti)) continue;
        if (g_tHeaterInsInfo[ti].GetOccupy()) {
            s_pHmCb[ti] = g_tHeaterInsInfo[ti].GetCtrlItem_Cb();               // golden 建構子那一組（上面建的）
        } else {                                                                // golden 沒有下拉（occupy=false）：Index 32 區＋E029 的 16 個
            const AnsiString n = W906_HeaterInsProxyName(ti, true);
            TComboBox* cb = EL<TComboBox>(kHmForm, n.c_str());
            cb->Items->Clear();
            for (int j = 0; j < W906_HEATER_INS_OPT_COUNT; ++j) cb->Items->Add(g_W906HeaterInsOptStr[j]);
            filerw::ELComboIndex(cb, g_tHeaterInsInfo[ti].GetHeaterInsOpt());
            const char* const pr[1][2] = { { n.c_str(), "grpHeater" } };
            filerw::ELSetParents(kHmForm, pr, 1);
            s_pHmCb[ti] = cb;
        }
        const AnsiString an = HmAddrProxy(ti);
        s_pHmAddr[ti] = EL<TEdit>(kHmForm, an.c_str());
        const AnsiString cn = HmChProxy(ti);                                    // E029：CH
        s_pHmCh[ti] = EL<TEdit>(kHmForm, cn.c_str());
        const char* const pr[2][2] = { { an.c_str(), "grpHeater" }, { cn.c_str(), "grpHeater" } };
        filerw::ELSetParents(kHmForm, pr, 2);
    }
    HmFillMixProxies(HmProxyU16());
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
    //AI(W906-FRW-S166) 20260927 [W906] golden 沒有：Q34 方案 D 存檔前檢查（D-7a 站號範圍、D-8a 同一個溫控 COM 埠同廠牌同站號）。
    //   放在 golden SaveBtnClick（:1121 → SaveSystemSet 的 YES/NO 與逐鍵寫檔）之前：擋下時整頁什麼都不寫（不是只有溫控鍵），
    //   ELMessage 列出原因；沒有 SaveSystemSet:write → PageSave reload 還原替身（_EditPage.cpp）。
    if (!W906_HeaterMixSaveCheck()) { filerw::ELMark("W906_HeaterMixSaveCheck: refused (nothing written)"); return; }
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
//   ⛔ 20260927 更正（//AI(W906-FRW-S166) Q34 方案 D＋Q15，RULINGS_20260926 S166／S137；decisions-decided Q34 ⑤「影響」）：上面三條
//     「寫的鍵與值跟 golden 相同」「位元組與 golden 相同」已不成立 ——
//     * 事件改成「全機相同、Index＝其他＝點的廠牌」：golden :1529-1538 由產生器取代成 W906_HeaterMixTypeClick（71 個通道記憶體都放
//       點的廠牌、不放 -9999；各通道下拉＋模式／Index／其他替身跟著改；仍不寫檔）；
//     * 開頁不再補寫缺鍵（Q15），所以第一次存檔會新增鍵（模式三鍵、缺的 HeaterInsOpt_、「不同」模式的 HeaterInsAddr_）；
//     * 存檔 W906_HeaterMixSave 寫 71 鍵的實際廠牌（D-4a，沒有下拉的 48 個也寫廠牌，不寫 -9999）、HEATER_CTRL_TYPE 照 D-6a ——
//       本來就刻意跟 golden 存出來的檔不同（新鍵、D-4、D-6）。
//     仍成立的：按下不寫檔、答 NO／套值失敗什麼都不寫，記憶體由 reload（開頁，也不寫檔）還原。
//   只有「沒存成」時不同：答 NO、套值失敗（_EditPage.cpp:148 reload）——golden 按下時已寫檔，這裡不寫；記憶體由 reload
//   （＝FormShow → LoaderSystemSet → HeaterInsOpt_Read）還原成檔案值。
//   ⚠ 點 A 再點回原廠牌：頁面最後值＝伺服器值 → 不重播（golden 兩次事件會把看不見的通道也設成原廠牌）。邊角差異，列在交件報告。
//   bHandlerModel==false（golden 停機）時不重播：存檔會被 SaveFlow 拒絕。
//AI(W906-HSYS-EVT) 20261002: golden 906 HandlerSys.cpp:1132-1136 THandlerSystem::rgRotateKit_TypeClick (the generator has no copy:
//   tools/editlist/HSys.py lists only the handlers the save flow calls); replayed by BeforeApply below.
static void W906_HS_rgRotateKit_TypeClick()
{
    EL<TRadioGroup>("THandlerSystem", "rgRotateKitIn")->Enabled =(EL<TRadioGroup>("THandlerSystem", "rgRotateKit_Type")->ItemIndex==0);   // golden :1134
    EL<TRadioGroup>("THandlerSystem", "rgRotateKitOut")->Enabled=(EL<TRadioGroup>("THandlerSystem", "rgRotateKit_Type")->ItemIndex==0);   // golden :1135
}
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
    //AI(W906-HSYS-EVT) 20261002: the two other golden OnClick handlers on this form that change what can be edited --
    //   rgTTLCard OnClick = rgTTLCardClick (906 HandlerSys.cpp:1246-1255: card 3 -> rgTTLUseAddress ItemIndex=1, Enabled=false, else
    //   Enabled=true) and rgRotateKit_Type OnClick = rgRotateKit_TypeClick (906 :1132-1136: rgRotateKitIn / rgRotateKitOut Enabled =
    //   (ItemIndex==0)). The page replays them on click (web/page/ht9045_hsys_events_c.js); they are replayed here as well, the same
    //   way as rgHeaterType above, because the drop of non-editable values (_EditPage.cpp, right after beforeApply) must see golden's
    //   Enabled: opened with card 3 (address greyed), the operator picks card 1 and sets the address -> golden saves the address;
    //   without the replay the server proxy is still Enabled=false and the address is dropped. Only on a change (golden: a click).
    struct Replay { const char* name; const char* mark; void (*click)(); };
    static const Replay kReplay[] = {
        { "rgTTLCard",        "rgTTLCardClick (golden 906 HandlerSys.cpp:1246, replayed before apply)",        &HS_rgTTLCardClick },
        { "rgRotateKit_Type", "rgRotateKit_TypeClick (golden 906 HandlerSys.cpp:1132, replayed before apply)", &W906_HS_rgRotateKit_TypeClick },
    };
    for (const Replay& e : kReplay) {
        const cJSON* w = cJSON_GetObjectItemCaseSensitive(root, e.name);
        const cJSON* ix = (w && cJSON_IsObject(w)) ? cJSON_GetObjectItemCaseSensitive(w, "itemIndex") : nullptr;
        if (!ix || !cJSON_IsNumber(ix) || !filerw::ELEditable("THandlerSystem", e.name)) continue;
        TRadioGroup* r = EL<TRadioGroup>("THandlerSystem", e.name);
        int v = ix->valueint;
        const int n = r->Items ? r->Items->Count : 0;                       // VCL TCustomRadioGroup.SetItemIndex clamp (as above)
        if (v >= n) v = n - 1;
        if (v < -1) v = -1;
        if (v == r->ItemIndex) continue;                                    // no change = no click
        r->ItemIndex = v;
        filerw::ELMark(e.mark);
        e.click();
        handled->push_back(e.name);
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
    //AI(W906-FRW-E029) 20261002 [W906]：頁面下拉的廠牌＝7 個（golden 5 個＋Omron EJ1N＋Delta DTM）；golden 原本的 5 個另放 goldenOptions
    w.Key("options").BeginArray();
    for (int j = 0; j < W906_HEATER_INS_OPT_COUNT; ++j) w.String(g_W906HeaterInsOptStr[j].c_str());
    w.EndArray();
    w.Key("goldenOptions").BeginArray();
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
    W906_HeaterMixExtraJson(w);   //AI(W906-FRW-S166) 20260927 [W906]：Q34 方案 D 的 "mix"（見第 (4) 段；純讀，不寫檔）
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

// ===========================================================================
//  AI(W906-EVB10C) 20260929 [W906]：事件批次 B10 part c（FileRW/WindowEdgeTails.h 第 16 列；Steven 20260928「如果沒有移植的, 我們直接實作」、
//    20260929「照 BCB 的邏輯」）。golden THandlerSystem::FormClose（V912 HandlerSys.cpp:1215-1221）：tsCustomerCode->TabVisible=false;
//    myLog.Do_Log(Sender, asUser, asLogPath); ＝產生檔的 HS_FormClose（tools/editlist/HSys.py 20260929 加進 methods；Do_Log 照 FormShow :174
//    閘掉，GATE H22-1：TMyLog 逐一比對表單控制項寫 ChangeLog 檔，伺服器端沒有表單物件 ⇒ 記一筆 todo）。
//    檔頭「不跑的只有 golden 用右上角 X 關（FormClose :1215，不重讀）這條路」⇒ 現在 ✕ 也跑 FormClose 了（它本來就不重讀）。
//  頁面表的關窗邊緣（FileRW/MainClick.cpp W906_EvB10A_WindowEdge）呼叫：這一次開窗 golden FormShow 跑過、FormClose 還沒跑過
//    （filerw::PageCloseEdgeRefused）才跑；運轉中不跑（ShowModal cTemperFrom.cpp:1778）。Handler System 視窗只在 debug 模式建立
//    （D:\HT9045\web\background.html debugOnly），出貨模式沒有這個邊緣。
// ===========================================================================
#include "FileRW/WindowEdgeTails.h"

const char* FileRW_HSys_WindowEdge(bool open)
{
    if (open) return "no open-edge action";
    if (!g_booted) return "not run: THandlerSystem proxies are not booted";
    if (const char* no = filerw::PageCloseEdgeRefused("HSys")) return no;
    HS_FormClose();                                                             // golden HandlerSys.cpp:1215
    return "ran golden THandlerSystem::FormClose (HandlerSys.cpp:1215-1221): tsCustomerCode->TabVisible=false; myLog.Do_Log gated (GATE H22-1, as FormShow :174)";
}

// ===========================================================================
//  //AI(W906-EBOOT005) 20261001 [W906] (St01): golden THandlerSystem::GetCustomerName (V912 HandlerSys.cpp:1242-1272), line by line,
//    reading the THandlerSystem named proxy rgCustomerList (items = V912 HandlerSys.dfm, 213 rows in DFM order, FileRW/HSys.gen.inc:1115;
//    put there at boot by FileRW_HSys_Boot -> HS_DfmItems). Jimmy TO_STEVEN 20261001 12:3x (census 129 (e)): golden TfMain::FormShow
//    (V912 main.cpp:11056-11057; 906 main.cpp:10615-10616) RunInfo.Factory=HandlerSystem->GetCustomerName(); + fObserver->labFactory->Caption
//    never ran -- the port has no HandlerSystem global (forms/fHandlerSys.h:229 INTEGRATION-PENDING). The boot-slot call is the laptop's
//    (tools/wb_serve.cpp); consumers: SECS SV 1005 (uHGemHT9045_SV.cpp:451), cObserver.cpp:8108 (labFactory).
//  * Golden never reads ItemIndex: each row's first 3 chars are compared with CUSTOMER_CODE, the first equal row wins (3 customers have a
//    fixed name), otherwise the text from char 5 up to the first ASCII space. No equal row (or an empty list) -> "HonPrec" (golden :1244
//    initial value, :1271 return).
//  * Golden quirk kept, not fixed: a row with no ASCII space after the name gives "" (AnsiPos(" ")=0 -> SubString(1,-1)=""). V912 has two:
//    "895 BARUN" and "970 GIGAS" + full-width spaces (U+3000 is not 0x20; its Big5 form A1 40 is not either).
//  * Not booted (FileRW_HSys_Boot not run, no proxy): treated as an empty list -> "HonPrec", golden's own path for a list with no rows.
//    ELFind, not EL<>: reading must not create an empty proxy. After boot the list is full whether or not the HandlerSys page was ever
//    opened (page open = golden FormShow, which does not touch this list). Only golden edtSearchCodeChange (:1291-1309, the search box)
//    changes it; golden then reads the filtered list too -- same here.
//  * Thread: FormLock (JsonBridge/FormJson.cpp, recursive) around the lookup and the list walk, as FileRW/_EditList.cpp:554
//    FileRW_ProxyChecked; the caller need not hold it.
//  * Same algorithm as the port's own translation HandlerSys.cpp:821-852 (ht9045_sm). Its list is the 906_0618 DFM (211 rows,
//    HandlerSys.cpp:569): no 807 TFAMD_M / 808 AMD_US, and 898 is AMD_SUZHOU (V912: TFAMD_SUZHOU).
//  * //AI(W906-D044) 20261002 [W906] (St01): Jimmy RULINGS_20261002 #9 (NIGHT_REPORT s0 item 42, SECS SV 1005 RunInfo.Factory):
//    "42用906的對照表" -- St01 puts 807 / 808 / 898 back to the 906 names. W906_HSys906Row (just below) hands the loop the 906 list's
//    view of those three rows: 807 and 808 have no row in 906 -> skipped -> no equal row -> golden :1244 initial value "HonPrec";
//    898 -> 906's row text (HandlerSys.cpp:682 "898 AMD_SUZHOU ...") -> "AMD_SUZHOU". Every other row is V912's, as before.
//    Only this lookup changes: the HandlerSys page (FileRW/HSys.gen.inc:1115, V912 list, 213 rows) is not touched.
//    Search box (golden edtSearchCodeChange): the filter runs on the V912 text, so a search that keeps "898 TFAMD_SUZHOU" but not
//    906's "898 AMD_SUZHOU" ("TFAMD") still gives "AMD_SUZHOU" here (906 would give "HonPrec"). Debug-only page; noted, not fixed.
//    ctest EBoot005_CustomerName: on the port's own 906 list this == HandlerSys.cpp:821-852 for every code; on the V912 list the two
//    differ exactly at 807, 808, 898.
//  Fallback for binaries without this file: FileRW/_fallback.cpp, end of file ("HonPrec").
// ===========================================================================
#include "JsonBridge/FormJson.h"   //AI(W906-EBOOT005) 20261001 [W906] (St01): FormLock / FormUnlock

//AI(W906-D044) 20261002 [W906] (St01): the rows where the 906 list (906_0618 DFM = the port's HandlerSys.cpp:569 kCustomerListItems)
//  differs from V912's for the name lookup (Jimmy RULINGS_20261002 #9, banner above). false = 906 has no row with this code (the
//  loop skips the V912 row, as 906's loop never sees one); true = use *item, replaced by 906's row text where it differs.
static bool W906_HSys906Row(int iCode, AnsiString* item)
{
    switch (iCode)
    {
        case 807: return false;                                                // V912 "807 TFAMD_M ..." (HSys.gen.inc:1154): no 807 in 906 (HandlerSys.cpp:607 806 STM -> :608 809 IMEC)
        case 808: return false;                                                // V912 "808 AMD_US ..." (HSys.gen.inc:1155): no 808 in 906 (same gap)
        case CC_AMD_SUZHOU: *item="898 AMD_SUZHOU           AMD  蘇州"; return true;   // 906 HandlerSys.cpp:682 (V912 "898 TFAMD_SUZHOU ..." HSys.gen.inc:1230); CC_AMD_SUZHOU=898 MachineType.h:360
        default:  return true;
    }
}

AnsiString FileRW_HSys_CustomerName()
{
    struct Lock { Lock() { ht9045::formjson::FormLock(); } ~Lock() { ht9045::formjson::FormUnlock(); } } lk;   // [W906] see banner
    TRadioGroup* rgCustomerList = dynamic_cast<TRadioGroup*>(filerw::ELFind("THandlerSystem", "rgCustomerList"));   // [W906] golden HandlerSystem->rgCustomerList
    AnsiString Str="HonPrec", tmp;                                              // golden :1244
    int iCustomerCode;                                                          // golden :1245
    if(rgCustomerList==nullptr) return Str;                                     // [W906] no proxy = empty list: golden :1246 loop runs 0 times -> :1271
    for(int i=0; i<rgCustomerList->Items->Count; i++)                          // golden :1246
    {
        AnsiString item=rgCustomerList->Items->Strings[i];                      // [W906] StringsProxy has no SubString/AnsiPos/Length (same as HandlerSys.cpp:827)
        iCustomerCode=atoi(item.SubString(1, 3).c_str());                       // golden :1248 取出前三碼的客戶代碼
        if(!W906_HSys906Row(iCustomerCode, &item)) continue;                    //AI(W906-D044) 20261002 [W906] (St01): 906's view of 807 / 808 / 898 (RULINGS_20261002 #9)
        if(iCustomerCode==CUSTOMER_CODE)                                        // golden :1249
        {
            if(CUSTOMER_CODE==CC_SPIL_CHINA_SUZHOU && SPIL_FOR_QLE==1)          // golden :1251 Steven 20230110 : For渠梁
            {
                Str="QLE";                                                      // golden :1253
            }
            else if(CUSTOMER_CODE==CC_IFXTH_Thai)                               // golden :1255 Ifor 20251113 add:客戶要求顯示Infineon
            {
                Str="Infineon";                                                 // golden :1257
            }
            else if(CUSTOMER_CODE==CC_Carsem_Thai)                              // golden :1259 Ifor 20260331 add:客戶要求顯示CARSEM
            {
                Str="CARSEM";                                                   // golden :1261
            }
            else
            {
                tmp=item.SubString(5, item.Length());                           // golden :1265 去除前四碼為客戶代碼+空格
                Str=tmp.SubString(1, tmp.AnsiPos(" ")-1);                       // golden :1266 取得從第一個字元到第一個空格的字串
            }
            return Str;                                                         // golden :1268
        }
    }
    return Str;                                                                 // golden :1271
}

//AI(W906-D043) 20261002 [W906] (St01): hand the lookup to cObserver.cpp's labFactory note (W906_ObsFactoryNoteNeeded, end of
//  cObserver.cpp): with it the note no longer calls golden's own "" (895 / 970) a missing boot step.  Static init; the pointer there
//  is zero-initialised before any dynamic initialiser runs.
extern AnsiString (*W906_ObsGoldenFactoryHook)();
namespace { struct W906ObsFactoryHookInstall { W906ObsFactoryHookInstall() { W906_ObsGoldenFactoryHook = &FileRW_HSys_CustomerName; } } g_w906ObsFactoryHookInstall; }
