// ===========================================================================
//  JsonBridge/actions/MainClarnData.h -- act.main.clarnData 的本體與安裝。
//
//  AI(W906-SJSON-S11) 20260923.
//  golden：HT9011UC_Code_V3.33.912.0_20260908_Jimmy / main.cpp:15458-15648
//          （`void __fastcall TfMain::Clarn_Data(int Tag, AnsiString Msg)`，191 行）
//  規格：.claude/skills/ht9045-json-bridge/SKILL.md 4.5、八 S11
//
//  ⚠ **golden 樹名**：這一支引的行號全部是 **V912**。
//    派工單寫的 golden 是 `HT9011UC_Code_V3.33.906.0_20260618`，但那棵樹
//    **不在這台機器上**（20260923 實測：`D:\HT9045` 底下只有 899 / 908 /
//    910 / 912 四棵）。SKILL.md §二 的裁決本來就是「改用 V912」，而派工單
//    給的行號 `main.cpp:15456` 也正好命中 V912 的 Clarn_Data 註解行
//    （15458 是函式簽章），所以這裡用 V912。
//    porting-gaps.md 開頭記過：跨樹引用不標樹名，得到的是「看起來有憑有據的錯」。
//
//  ---------------------------------------------------------------------------
//  為什麼先做這一支（SKILL §4.5）
//  ---------------------------------------------------------------------------
//  它是 47 個 golden 呼叫點的匯流點。做完一個原語，SortCT／CounterClear／
//  LotInfo／SECS／遠端指令那幾條路就一起有了落地。按鈕處理器（如
//  `btnClearCountClick` 123 行）剩下的只是守衛與確認框。
//
//  ---------------------------------------------------------------------------
//  ⚠⚠ 它以前在這棵樹是一個**空殼**
//  ---------------------------------------------------------------------------
//  `forms/fMain.cpp` 原本逐字是：
//      void TfMain::Clarn_Data(int, AnsiString) { W906_Clarn_DataCallCount++; }
//  191 行一行都沒翻。所以這不是「包一層」，是從零翻譯。
//  （SKILL.md §十二 R6 記的就是這件事。）
//
//  ---------------------------------------------------------------------------
//  刻意偏離 golden 的地方（只有三處，逐條給理由）
//  ---------------------------------------------------------------------------
//  D1. `fContactCT->sgYield->Refresh()`（golden :15516）**不翻**。
//      它是 VCL 重繪，新架構沒有控制項（SKILL §4.5「VCL 刷新行刪除」）。
//      畫面更新由 tag patch 完成。
//      ⚠ `fSortCT->ShowLoadingIC()` / `ShowSortIC()`（:15637-15638）**照翻**，
//        因為它們**不是**純顯示：forms/fSortCT.h:51-63 逐字記著
//        ShowLoadingIC 會重設 LastSet.SendCT[2]、ShowSortIC 會把
//        LastSet.BinCT[0][] 匯總進 RunInfo.iUnloadCount。刪掉它們會丟資料。
//        （它們在移植樹目前是 no-op facade —— 那是另一個已登記的缺口，
//          不是本波次造成的。）
//  D2. `slQtyLog` 改成**延遲建構**。golden 在 TfMain 建構子裡
//      （main.cpp:1622-1639）用當時的 `Prod.iTrayType[]` / `s6TrayName[]`
//      組出表頭。移植樹的 TfMain 建構子在 ht9045_forms，碰不到 TMyStringList
//      （ht9045_public）。表頭只在檔案第一次建立時寫出，而 golden 的建構子
//      本來就跑在配方載入之前，所以延後到第一次真的要寫的時候組表頭，
//      結果只會**更接近**當下的盤面配置，不會更差。
//  D3. `iMode` 的宣告位置：golden 在函式開頭宣告 `int iMode;` 而只在
//      `Tag==0` 用。這裡照放在開頭（保持行對行可比對），編譯器會警告
//      「set but not used」的話是 golden 自己的形狀，不改。
//
//  ---------------------------------------------------------------------------
//  ⚠⚠ 這一支會寫三個真實檔案
//  ---------------------------------------------------------------------------
//      D:\HT9045\system\lastdata.dat           WriteLastDataFile(false)
//      D:\HT9045\system\lastdata_backup.dat    同上（cprod.cpp:2047）
//      D:\HT9045_Log\QtyData\YYYYMM\<name>.csv slQtyLog
//  前兩個是**硬編路徑**（cprod.cpp:2022/:2047），`--dry` 重導不到
//  （SKILL §十二 R8）。所以 `act.main.clarnData` 的 dryRun 預設是 true。
// ===========================================================================
#ifndef HT9045_JSONBRIDGE_ACTIONS_MAINCLARNDATA_H
#define HT9045_JSONBRIDGE_ACTIONS_MAINCLARNDATA_H

#include <string>
#include <vector>

#include "vclcompat/vcl_compat.h"

namespace ht9045 {
namespace sjson {

// golden main.cpp:15458-15648 的逐字翻譯。由 forms/fMain.cpp 的
// TfMain::Clarn_Data 經 W906_ClarnDataBody 呼叫。
void ClarnDataBody(int Tag, AnsiString Msg);

// 把上面那一支裝進 forms 的安裝座。wb_serve 開機時明確呼叫一次
// （為什麼不能自我登錄，見 forms/fMain.h 的安裝座註解）。
void InstallClarnDataBody();

// 已經裝好了嗎。給 act.* 的回應用 —— 沒裝就別假裝執行過。
bool ClarnDataBodyInstalled();

// -------------------------------------------------------------------------
//  dryRun 的預覽：這個 Tag 會清掉哪些 ct*（名字），以及共用尾段會清掉什麼。
//  **純函式**：只讀 LastSet / IniConfig / CUSTOMER_CODE，不呼叫任何 Clear。
//  它是把上面那一支的 12 個分支「再讀一次」，所以兩邊改動時要一起改；
//  test_sjson_clarndata.cpp 用一組固定的 bCTClear 盤面同時釘住兩者。
// -------------------------------------------------------------------------
std::vector<std::string> ClarnDataPreview(int Tag);

}  // namespace sjson
}  // namespace ht9045

#endif  // HT9045_JSONBRIDGE_ACTIONS_MAINCLARNDATA_H
