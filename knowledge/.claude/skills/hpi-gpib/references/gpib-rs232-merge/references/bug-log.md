# Bug Log — gpib-rs232-merge

記錄三介面整合開發過程中遇到的 bug 及修正方法。

---

## BUG-001：GPIB 模式 log 顯示「TTL ()」

**日期**：2026-04-09  
**嚴重度**：中（log 資訊錯誤，不影響功能）  
**元件**：HT9045 `main.cpp` → `SendMSG_TestMode()`

### 症狀
切換為 GPIB 模式後，GPIB_RS232 log 視窗顯示：
```
Handler ==> Change tester interface: TTL ()
```

### 根因分析
`HHandler2Gpib` 是 struct，C++ 保證 POD member 初始化為 0。
`SendMSG_TestMode()` 設定了 `iSendCommand`, `bCloseGpib`, `HandlerHwnd`, `GPIBBin`，
**但漏掉** `iLotStatus`，致使 GPIB_RS232 端收到的 `iLotStatus = 0 = TTL_MODE`。

GPIB_RS232 的 `MSG_CMD_TesterMode` handler：
```cpp
// C7 handler
g_iTestType = GHandler2Gpib->iLotStatus;  // 取到 0 → TTL_MODE
```
然後分流到 TTL 分支，log 輸出「TTL」。

### 修正
`main.cpp` `SendMSG_TestMode()` 函式開頭加入：
```cpp
HHandler2Gpib.iLotStatus = TestIF_File.iTestType;  //Steven 20260409
```

### 防止復發
- `MSG_CMD_TesterMode` / `MSG_CMD_ChangeGpib` 兩個 CMD 都依賴 `iLotStatus`
- 任何新增 `WM_COPYDATA` 送出點，都必須先設定 `iLotStatus`
- HT9045 側 `struct H2GForm` 沒有 constructor，每次發送前要明確初始化所有欄位

---

## BUG-002：FormDestroy 例外崩潰

**日期**：2026-04-09  
**嚴重度**：高（應用程式異常結束）  
**元件**：`RS232Std.cpp` → `FormDestroy`

### 症狀
關閉 RS232 視窗時，視窗消失後程式崩潰，或彈出 Borland 例外對話框。

### 根因分析（三點）

1. **AppException dangling pointer**  
   VCL 銷毀子控件時若發生例外，`Application->OnException` 回調觸發 `ShowCommData`，
   而 `slRS232Log` 已在 `FormDestroy` 前半段被 delete。
   
2. **MY_DUT_PAL 殘留**  
   正常流程：`FormClose` → `FormDestroy`。  
   強制關閉流程：直接呼叫 `FormDestroy`，`MY_DUT_PAL` 未清空。

3. **slRS232Log 無 NULL 賦值**  
   `delete slRS232Log` 後未設 NULL，其他路徑（如 Timer）再次存取 → dangling pointer。

### 修正

```cpp
void __fastcall TfRS232Std::FormDestroy(TObject *Sender)
{
    InitialOK = false;

    for(auto iter = MY_DUT_PAL.begin(); iter != MY_DUT_PAL.end(); ++iter)
        delete *iter;
    MY_DUT_PAL.clear();

    Application->OnException = NULL;    // 截斷 VCL 例外回呼

    if(slRS232Log != NULL)
    {
        slRS232Log->Clear();
        delete slRS232Log;
        slRS232Log = NULL;              // 清 NULL，防止 dangling
    }
}
```

`FormClose` 補：
```cpp
delete sBarCode;        sBarCode = NULL;
delete sBarCode_ASE_CL; sBarCode_ASE_CL = NULL;
```

### 防止復發
- 任何以 `new` 分配的成員指標，`delete` 後必須立即設 `NULL`
- `Application->OnException` 若有設定自定義 handler，FormDestroy 必須清除
- Timer 在 `InitialOK=false` 後必須跳出，勿存取已釋放物件

---

## BUG-003：Timer1Timer crash（MY_DUT_PAL 未初始化）

**日期**：2026-04-09  
**嚴重度**：高（啟動時崩潰）  
**元件**：`RS232Std.cpp` → `Timer1Timer`

### 症狀
啟動程式時，進入 `Timer1Timer`（每秒觸發）時崩潰，crash 點：
```cpp
MY_DUT_PAL[i]->cbBin->ItemIndex = 0;
```

### 根因分析
`MY_DUT_PAL.push_back(new TMyDutPanel(...))` 原本在 `FormShow` 中執行，
但 `Timer1Timer`（已啟用）在 `FormShow` 之前觸發，此時 `MY_DUT_PAL` 是空的 vector，
`MY_DUT_PAL[i]` 存取越界。

### 修正
將 `MY_DUT_PAL` 初始化迴圈移到 Constructor `TfRS232Std::TfRS232Std`：
```cpp
// Steven 20260409 : 移至 Constructor，避免 Timer1Timer 在 FormShow 前觸發 crash
for(int i = 0; i < USE_SITE_COUNT; i++)
    MY_DUT_PAL.push_back(new TMyDutPanel(palSite, i));
```

### 防止復發
- Constructor 中初始化所有「容器型」成員（vector/list），避免 Timer 早觸發
- 若必須放在 FormShow，應在 Timer 進入時先檢查 `MY_DUT_PAL.size()`

---

## BUG-004：ProcessHVisionConnect 每秒重複呼叫 SendMessageToGpibProg

**日期**：2026-04-09  
**嚴重度**：中（頻繁送 WM_COPYDATA 可能干擾 GPIB 程序）  
**元件**：HT9045 `main.cpp` → `ProcessHVisionConnect()`

### 症狀
GPIB_RS232 log 視窗每秒出現：
```
Handler SendMsg (MSG_CMD_ChangeGpib) ...
```

### 根因分析
`ProcessHVisionConnect()` 每秒由 timer 呼叫。
`SendMessageToGpibProg()` 直接放在 `bFind=true` 路徑下，每秒觸發。

第一次修正嘗試（錯誤）：放在 `WakeupGPIBdelay.Off()` 區塊內。
`Off()` 在 delay 過期後每次都回傳 true（無法自動 reset），仍會每秒觸發。

### 最終修正
```cpp
static HWND sLastHVisionWnd = NULL;  //Steven 20260409

if(HVisionWnd != NULL)
{
    if(sLastHVisionWnd == NULL && HVisionWnd != NULL)
        SendMessageToGpibProg();     // 首次 NULL→非NULL 觸發一次
    sLastHVisionWnd = HVisionWnd;
}
else
{
    sLastHVisionWnd = NULL;          // 斷線後 reset，下次重連可再觸發
}
```

### 防止復發
- 在 timer 驅動的函式內，「只做一次」的動作必須用 `static` 狀態變數或 flag 保護
- `WakeupGPIBdelay.Off()` 不能做 one-shot：它只有 `Off()`（開始計時）和過期時返回 true，沒有「只返回一次 true」的語意

---

## BUG-005：RS232 模擬連線時頻繁出現 [Tester] Closed Site Have Bin ERROR!!

**日期**：2026-04-09  
**嚴重度**：高（每次測試循環都報錯，影響模擬測試）  
**元件**：`RS232Std.cpp` → `Timer1Timer` → `SendResultFinish()`

### 症狀
使用 RS232_MODE + 模擬連線（RS232 Connect: Fail）時，每次 START TEST 後約 170ms 出現：
```
[Tester] Closed Site Have Bin ERROR!!
```

### 根因分析（四層追蹤）

**層 1：共享 extern global**

`bSimulate` 和 `iStart[]` 都是在 `cmydef.h` 宣告的 extern global，由 Main.cpp 與 RS232Std **共用**：
```cpp
// cmydef.h
extern bool bSimulate;
extern int iStart[MAX_SITE_COUNT];
```

**層 2：HT9045 只送訊息給 Main.cpp**

在 RS232_MODE 下，HT9045 發送 MSG_CMD_NONE（START TEST）給 Main.cpp 的 TSerialPoll 視窗。
Main.cpp 的 `OnMyCopyMsg` 處理後：
- 設定共享 global：`bSimulate=true`, `iStart[0]=1`, `iStart[1]=1`（active sites）
- 其餘 `iStart[2..31]=0`（closed sites）
- 設定 ECHO bytes（次要議題）

**RS232Std 的 `OnMyCopyMsg` 並不觸發**（因為 HT9045 只送給 Main.cpp），
因此 RS232Std 的 `cbSiteOn->Checked` **不會更新**，維持預設值 `false`（所有 site 均視為關閉）。

**層 3：Timer1Timer 的 simulate 路徑使用 iStart 生成 Result**

RS232Std 的 Timer1Timer（300ms）在 START TEST 後約 170ms 觸發，進入 simulate 路徑：
```cpp
if(bSimulate)  // true（共享 global，Main.cpp 已設 true）
{
    for(int i=0; i<USE_SITE_COUNT; i++)
    {
        if(iStart[i]==1)  // 共享 global，Main.cpp 已設 0,1 為 1
            iResult[i]=ct;  // active site → Result=1
        else
            iResult[i]=0;   // closed site → Result=0
        GGpib2Handler.Result[i]=iResult[i];
        // cbSiteOn->Checked 未更新！仍是預設 false
    }
    ...
    SendResultFinish();  // 進入錯誤確認邏輯
}
```

**層 4：SendResultFinish 的 cbSiteOn 與 Result 不一致**

在 `SendResultFinish()` 中：
```cpp
if(MY_DUT_PAL[i]->cbSiteOn->Checked==true)  // false（預設！）
    S.sprintf("%d", GGpib2Handler.Result[i]);
else
{
    if(GGpib2Handler.Result[i]!=0 &&          // Result[0]=1 → 1!=0 ✓
       GGpib2Handler.Result[i]!=999)          // 1!=999 ✓
    {
        bClosedSiteHaveBin=true;              // ← ERROR 觸發！
    }
}
```

Site 0,1 的 `cbSiteOn=false`（因 RS232Std 從未收 MSG_CMD_NONE），
而 `Result[0]=Result[1]=1`（由 iStart 驅動的 simulate 結果），
→ `1 != 0 && 1 != 999` → `bClosedSiteHaveBin=true` → 報錯。

### 修正

在 RS232Std.cpp `Timer1Timer` 的 simulate 迴圈中，在生成 `Result[i]` 之後，
同步設定 `cbSiteOn->Checked`，確保與 `iStart[i]` 一致：

```cpp
// RS232Std.cpp Timer1Timer simulate 迴圈（BUG-005 修正）
GGpib2Handler.Result[i]=iResult[i];
MY_DUT_PAL[i]->cbSiteOn->Checked=(iStart[i]==1);  //Steven 20260409 : CBSiteOn同步修正
MY_DUT_PAL[i]->plSite->Caption=AnsiString(tstr[i]);
```

### 修正原則
- 修改最小化（不改動 SendResultFinish，不改動 GPIB simulate 路徑）
- 只在 simulate 路徑中加入同步，不影響正式 RS232/TTL 流程
- `cbSiteOn` 在 `iStart[]` 被清零（`iStart[i]=false`）之前同步，時序正確

### 防止復發
- **任何** 讀取 `bSimulate` 或 `iStart[]` 的 RS232Std 路徑，都必須確保 `cbSiteOn->Checked` 已與 `iStart[i]` 同步
- 若日後 RS232Std 增加新的 simulate 路徑，同樣需要在呼叫 `SendResultFinish()` 前加同步
- 考慮在 `SendResultFinish()` 開頭加防護斷言（debug build）：
  ```cpp
  // 如果 cbSiteOn[i]=false 但 Result[i]!=0，在 DEBUG 模式下記錄警告
  ```
