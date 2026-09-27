// ===========================================================================
//  FileRW/Winway.cpp -- golden TfWinway（ATC\WinWaySetting.cpp，V912，jimmychiu 20210903 AMD-M WinWay 4 站 ATC）的 C 路入口：
//  D:\HT9045\config\ATCWinWay.ini（ATCWinWayPath）的讀寫段。沒有頁面（golden 主畫面 sbATC 鈕，ATC_SYSTEM==eWinWay 才 fWinway->ShowModal()，
//  main.cpp:29607-29609）。
//
//  //AI(W906-FRW-S109) 20260926: 新檔（Steven 團隊，RULINGS_20260926 S109）。設定：tools/editlist/Winway.py（每條 replace 附原因）。
//
//  golden 方法由 tools/gen_editlist.py 轉成 Winway.gen.inc（元件改具名替身，名稱＝golden 元件名）：
//    建構子（:12）            ＝ FileRW_Winway_Boot()：edtSetTemp="0"、iWinwayATCIndex=0、new 4 個 ATC_WinWay(WW_Comm[0..3])，
//                                每站 LoadCommData（:116）→ SaveCommData（:135）。
//    FormShow（:34）／ShowCommData（:43）／cbbWinwayATCIndexChange（:150）＝ 開頁／切換站別：記憶體 → 元件（不讀檔）。
//    btnUpdateClick（:78）    ＝ Update 鈕：元件 → 這一站的 ATC_WinWay／TComm 欄位 → SaveCommData 寫這一段。
//
//  ---- golden 讀寫時機（本檔提供函式，呼叫點由整合者插進 tools/wb_serve.cpp，片段見交件報告）--------------------------
//    開機：CreateForm(TfWinway)（HT9045.cpp:272，TfAutoAlignment :270 之後）→ 建構子 → FileRW_Winway_Boot()。
//          ⚠ **每台機台每次開機都讀、都寫**（golden 同：建構子不看 ATC_SYSTEM）。讀 [COMPort1..4] 6 鍵，再原樣寫回 24 鍵；
//          檔不在 → 用預設值（COM15..COM18／9600／ByteSize=3（_8）／StopBits=0／Parity=0／SetTemperature=0）建出來。
//          這台 D:\HT9045\config\ATCWinWay.ini 在版控裡、內容就是那組預設值 ⇒ 寫回的位元組與原檔相同（Win32 WritePrivateProfileString
//          的就地修改，vclcompat/IniFiles.cpp W906_Win32WriteProfileString；SetTemperature 0 → FloatToStr "0"）。
//          SetST(讀到的設定溫度) 只存值：4 站都沒開 COM（bCommConnect=false，ATC/ATC_WinWay.cpp:195），不送 Modbus。
//    換配方：不讀（ATCWinWay.ini 不跟配方）。
//    開頁：FileRW_Winway_FormShow()；切換站別：FileRW_Winway_cbbWinwayATCIndexChange()；存檔：FileRW_Winway_btnUpdateClick()。⚠ 三支目前都沒有呼叫者。
//
//  ---- 狀態放哪 ------------------------------------------------------------------------------------------------
//    arrATC_Site[4]、iWinwayATCIndex：門面 forms/fWinway.h 的 fWinway（Winway.gen.inc 用 #define 接過去 —— ⚠ gen.inc 之後這兩個名字是巨集，
//    本檔直接寫名字，不要寫 fWinway->arrATC_Site）。門面自己的建構子在 static init 把 arrATC_Site 設成 NULL（forms/fWinway.cpp:79-82），
//    本檔開機時照 golden :22-25 填上 4 個物件 —— 門面檔頭「permanently NULL」那段說明從 S109 起只到開機為止。
//    golden 的讀者 bthermo.cpp:4609-4659（顯示 PV）、forms/fLotInfo.cpp:4276（OpenCommPort）目前都在 #if 0；解閘後讀到的是這 4 個物件。
//    edtSetTemp／cbbWinwayATCIndex：門面那兩個物件（WW_AdoptPortWidgets 開機最先 ELKeep）。
//    4 個 TComm（golden DFM WinWayATCComm1..4）：Winway.gen.inc 的 WW_Comm[4]（ATC_WinWay::WinwayCOM 指向它們；沒有人 StartComm）。
//
//  ---- 不在本檔（通訊，交 Jimmy）------------------------------------------------------------------------------------
//    OpenCommPort（:156／:164，CreateFile＋StartComm）、CloseCommPort（:176）、btnUpdateClick 結尾的重開 COM（:111-112，ELTodo）、
//    ShowCommData 的 lblShowPT=GetPT()（:74，連線時送 Modbus 01 03，ELTodo）、SetST（:170）、btnSendTempClick／SetTemprature／
//    SetTempratureAll（:184／:190／:195，送設定溫度 Modbus 01 06）、btnGetPVClick（:203）、WinWayATCComm1ReceiveData（:208，收 PV）。
// ===========================================================================
#include "FileRW/Winway.gen.inc"

#include <cstdio>

namespace {
bool g_booted = false;

// 門面 forms/fWinway.h 已有的同名同型別元件 → 直接當替身（產生器 'adopt' 的手寫版：門面寫的是 `new TEdit()`，產生器只認
// `new vclcompat::TEdit()`）。要在任何 EL<>("TfWinway", …) 之前（DfmItems／DfmState 會建）。
void WW_AdoptPortWidgets()
{
    filerw::ELKeep("TfWinway", "edtSetTemp", fWinway->edtSetTemp);
    filerw::ELKeep("TfWinway", "cbbWinwayATCIndex", fWinway->cbbWinwayATCIndex);
}
}  // namespace

// golden TfWinway 建構（HT9045.cpp:272 CreateForm）：收養門面元件 → DFM 設計期 Items／狀態 → 存檔流程讀的替身 → 容器替身與父子
// → 建構子（:12；讀＋寫 D:\HT9045\config\ATCWinWay.ini）。冪等。
void FileRW_Winway_Boot()
{
    if (g_booted) return;
    WW_AdoptPortWidgets();
    WW_DfmItems();
    WW_DfmState();
    WW_CreateSaveProxies();
    WW_CreateContainerProxies();
    WW_TfWinway();
    g_booted = true;
    std::printf("FileRW Winway: TfWinway ctor :12 done -- config\\ATCWinWay.ini read+written back for %d sites (golden: every boot, "
                "any ATC_SYSTEM; COM ports NOT opened):", SiteNum);
    for (int i = 0; i < SiteNum; ++i)
        std::printf(" [%d] %s %u/%d/%d/%d ST=%s", i + 1, arrATC_Site[i]->sWinwayCommName.c_str(), arrATC_Site[i]->WinwayCOM->BaudRate,
                    (int)arrATC_Site[i]->WinwayCOM->ByteSize, (int)arrATC_Site[i]->WinwayCOM->StopBits,
                    (int)arrATC_Site[i]->WinwayCOM->Parity, FloatToStr(arrATC_Site[i]->GetST()).c_str());
    std::printf("\n");
}

// golden TfWinway::FormShow（:34）：開頁（記憶體 → 元件，不讀檔）。目前沒有呼叫者（沒有頁面）。
void FileRW_Winway_FormShow()
{
    if (!g_booted) FileRW_Winway_Boot();
    WW_FormShow();
}

// golden TfWinway::cbbWinwayATCIndexChange（:150）：頁面換站別（先把 cbbWinwayATCIndex 的替身值套好）。目前沒有呼叫者。
void FileRW_Winway_cbbWinwayATCIndexChange()
{
    if (!g_booted) FileRW_Winway_Boot();
    WW_cbbWinwayATCIndexChange();
}

// golden TfWinway::btnUpdateClick（:78）：寫 D:\HT9045\config\ATCWinWay.ini 目前那一站的 6 鍵。重開 COM 那兩行不做（ELTodo，交 Jimmy）。
// 目前沒有呼叫者（沒有頁面、沒有 WS 指令）；給之後的頁面用（頁面值先套到替身，見 kWW_SaveReads）。
void FileRW_Winway_btnUpdateClick()
{
    if (!g_booted) FileRW_Winway_Boot();
    WW_btnUpdateClick();
}
