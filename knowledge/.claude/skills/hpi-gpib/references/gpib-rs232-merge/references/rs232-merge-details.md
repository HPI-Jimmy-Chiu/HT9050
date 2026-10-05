# RS232 Merge — 詳細實作內容

本文件為 `gpib-rs232-merge` SKILL 的詳細參考資料，
從 SKILL.md 搬移而來，涵蓋架構圖、全域變數、修改清單、HT9045 端修改等。

> Bug 修正詳見 [bug-log.md](bug-log.md)  
> 設計決策詳見 [design-decisions.md](design-decisions.md)

---

## 架構概念（完整流程圖）

```
HT9045 main.cpp
  SendMSG_TestMode()
    HHandler2Gpib.iLotStatus = TestIF_File.iTestType   ← C9 (bug fix)
    HHandler2Gpib.GPIBBin    = iGpibMode / iRs232Mode / iDioMode+TTL
    --> WM_COPYDATA (MSG_CMD_TesterMode)

  SendMessageToGpibProg()
    HHandler2Gpib.iLotStatus = TestIF_File.iTestType   ← Phase 1
    HHandler2Gpib.GPIBBin    = iTestBinCount
    --> WM_COPYDATA (MSG_CMD_ChangeGpib)

  ProcessHVisionConnect()  ← 每秒執行
    static HWND sLastHVisionWnd = NULL
    if(HVisionWnd!=NULL && sLastHVisionWnd==NULL)      ← 首次連上時觸發一次
        SendMessageToGpibProg()
    sLastHVisionWnd = HVisionWnd (or NULL when disconnected)

GPIB_RS232 Main.cpp
  OnMyCopyMsg → MSG_CMD_ChangeGpib (C8)
    bGpibMode   = GHandler2Gpib->bGpibMode
    g_iTestType = GHandler2Gpib->iLotStatus
    --> CloseTesterComm() / OpenTesterComm()

  OnMyCopyMsg → MSG_CMD_TesterMode (C7)
    g_iTestType   = GHandler2Gpib->iLotStatus
    g_sRecipePath = GHandler2Gpib->Message
    --> WriteLog (TTL/RS232/GPIB 分流)
    --> Qorvo guard (GPIB_MODE only)
    --> OpenTesterComm() / LoadSetupData(g_sRecipePath)

  ReadLastDataFile()   ← startup 還原
    g_iTestType = CheckAndReadIniData(..., GPIB_MODE)

  WriteLastDataFile()  ← 關閉時持久化
    WriteIniData(..., g_iTestType)

RS232Std.cpp
  ShowCommData(sUnitName, bHex)         ← 同時寫入 slRS232Log + SerialPoll->WriteLog
  ShowCommData(sUnitName, sMsg1, sMsg2) ← 同時寫入 slRS232Log + SerialPoll->WriteLog
```

---

## 全域變數（GPIB_RS232 Main.cpp）

| 變數 | 型別 | 初始值 | 說明 |
|------|------|--------|------|
| `g_iTestType` | `int` | `GPIB_MODE` | 當前介面模式（TTL=0 / GPIB=1 / RS232=2） |
| `g_sRecipePath` | `AnsiString` | `""` | Recipe 資料夾路徑，MSG_CMD_TesterMode 到來時才有值 |
| `bGpibMode` | `bool` | `true` | Handler 傳來的 GPIB on/off 旗標 |

### 模式常數（cmydef.h）

```cpp
#define TTL_MODE    0
#define GPIB_MODE   1
#define RS232_MODE  2
```

---

## 設計原則：兩個欄位職責分離

| 變數 | 職責 | 值域 |
|------|------|------|
| `g_iTestType` | **介面種類**（誰在通訊）| `TTL_MODE / GPIB_MODE / RS232_MODE` |
| `LastSet.iTesterMode` | **GPIB 子協定**（GPIB 時用什麼格式）| `InterfaceType_ADVAN_Type1` ... `InterfaceType_Delta_Castle` |

- `iTesterMode` 只在 GPIB 模式下有意義
- Qorvo 相關設定必須用 `g_iTestType == GPIB_MODE` guard
- **禁止**新增 `InterfaceType_RS232Standard` 等混用兩種概念的 define

---

## Log 路徑統一（2026-04-09）

所有 log 集中到 `D:\GPIBLOG\`，方便交叉除錯：

| 用途 | 舊路徑 | 新路徑 |
|------|--------|--------|
| GPIB 通訊 log | `D:\GPIBLOG\Log\` | 不變 |
| RS232 通訊 log（TMyStringList）| `D:\RS232Log\LOG\` | `D:\GPIBLOG\RS232_Log\` |
| RS232Aux Save_Log | `D:\RS232Log\Log\` | `D:\GPIBLOG\RS232_Log\` |
| RS232 BinData | `D:\RS232Log\BinLog\` | `D:\GPIBLOG\RS232_BinLog\` |
| TTL BinData | `D:\RS232Log\BinLog_TTL\` | `D:\GPIBLOG\RS232_BinLog_TTL\` |

### RS232 log 同步寫入 GPIB log（C10）

`RS232Std.cpp` 的兩個 `ShowCommData` 重載，在原有 `slRS232Log` 記錄外，
額外呼叫 `SerialPoll->WriteLog()` 寫入 `lstRecord`（GPIB log 視窗 + `D:\GPIBLOG\Log\`）：

```cpp
// ShowCommData(sUnitName, bHex)
AnsiString sLog;
if(bHex.empty())
    sLog.sprintf("%s", sUnitName);
else
    sLog.sprintf("%s  ASCII:%s  HEX:%s", sUnitName, sAscii, sHex);
SerialPoll->WriteLog(sLog);  //Steven 20260409

// ShowCommData(sUnitName, sMsg1, sMsg2)
AnsiString sLog;
if(sMsg2=="")
    sLog.sprintf("%s  %s", sUnitName, sMsg1);
else
    sLog.sprintf("%s  %s  %s", sUnitName, sMsg1, sMsg2);
SerialPoll->WriteLog(sLog);  //Steven 20260409
```

- RS232Std.cpp 加入 `#include "Main.h"` 以存取 `SerialPoll`
- 兩種格式並存：`slRS232Log`（CSV 詳細格式）+ `SerialPoll->WriteLog`（純文字格式）

---

## 關鍵修改清單（C1–C10）

### C1：cmydef.h — 模式常數與 extern 宣告
```cpp
#define TTL_MODE    0
#define GPIB_MODE   1
#define RS232_MODE  2
extern int g_iTestType;
extern AnsiString g_sRecipePath;
```

### C2：Main.cpp — 全域變數宣告
```cpp
int g_iTestType = GPIB_MODE;   //Steven 20260409
AnsiString g_sRecipePath = ""; //Steven 20260409
```

### C3：ReadLastDataFile() — startup 從 ini 還原
```cpp
g_iTestType = CheckAndReadIniData(asGeneralPath, "SystemSetup", "iTestType", (int)GPIB_MODE);
```
- 讀取位置：`D:\GPIB9045\system\general.ini [SystemSetup] iTestType`
- 預設值：`GPIB_MODE`（ini 不存在時）

### C4a：RS232Aux.h — LoadSetupData 簽名擴展
```cpp
void LoadSetupData(AnsiString sRecipeDir = "");
```

### C4b：RS232Aux.cpp — LoadSetupData 讀取 Tester.Data RS-232C
讀取 `sRecipeDir\Tester.Data [RS-232C]` 的 BaudRate / ByteSize / StopBits / Parity。

### C5：RS232Aux.cpp — OpenTesterComm() 使用 g_iTestType
依 `g_iTestType` 選擇 RS232 或 TTL 開啟方式。

### C6：RS232Std.cpp — iUseRS232Mode 來自 g_iTestType
```cpp
if(g_iTestType == TTL_MODE && (TTL_CARD_TYPE==2 || TTL_CARD_TYPE==3))
    iUseRS232Mode = InterfaceType_TTL;
else if(g_iTestType == RS232_MODE)
    iUseRS232Mode = 0;
else
    iUseRS232Mode = -1;
```

### C7：MSG_CMD_TesterMode handler — log 分流 + Qorvo guard + COM port 開關
```cpp
g_iTestType   = GHandler2Gpib->iLotStatus;
g_sRecipePath = AnsiString(GHandler2Gpib->Message);

// log 依介面種類分流
if(g_iTestType == TTL_MODE)
{
    AnsiString sDIOName = CheckAndReadIniData(g_sRecipePath+"\\Tester.Data", "DIO", "TypeName", AnsiString(""));
    WriteLog("Handler ==> Change tester interface: TTL (" + sDIOName + ")");
}
else if(g_iTestType == RS232_MODE)
{
    int iRs232TypeIdx = CheckAndReadIniData(g_sRecipePath+"\\Tester.Data", "RS-232C", "Type", 0);
    AnsiString sRs232TypeName;
    if     (iRs232TypeIdx == 0) sRs232TypeName = "Standard";
    else if(iRs232TypeIdx == 1) sRs232TypeName = "32 Bin";
    else                        sRs232TypeName = IntToStr(iRs232TypeIdx);
    WriteLog("Handler ==> Change tester interface: RS232 (" + sRs232TypeName + ")");
}
else  // GPIB_MODE
{
    // 原有 InterfaceType_xxx WriteLog
}

// Qorvo 設定僅在 GPIB 模式下套用
if(g_iTestType == GPIB_MODE)
{
    if(LastSet.iTesterMode != InterfaceType_15BinQorvo)
    {
        LastSet.bConfigureSRQ = false;
        LastSet.bHaveContactorInfo = false;
    }
    else
        LastSet.iTesterType = 1;
}

// COM port 開關
if(bGpibMode && fRS232Aux->bCommConnect)
    fRS232Aux->CloseTesterComm();
else if(!bGpibMode && !fRS232Aux->bCommConnect &&
        (g_iTestType == RS232_MODE ||
         (g_iTestType == TTL_MODE && (TTL_CARD_TYPE==2 || TTL_CARD_TYPE==3))))
    fRS232Aux->OpenTesterComm();

if(g_iTestType == RS232_MODE ||
   (g_iTestType == TTL_MODE && (TTL_CARD_TYPE==2 || TTL_CARD_TYPE==3)))
    fRS232Aux->LoadSetupData(g_sRecipePath);
```

### C8：MSG_CMD_ChangeGpib handler
```cpp
bGpibMode   = GHandler2Gpib->bGpibMode;
g_iTestType = GHandler2Gpib->iLotStatus;  //Steven 20260409

if(bGpibMode)
{
    WriteLog("Handler ==> GPIB MODE");
    if(fRS232Aux->bCommConnect)
        fRS232Aux->CloseTesterComm();
}
else if(!bGpibMode && !fRS232Aux->bCommConnect &&
        (g_iTestType == RS232_MODE ||
         (g_iTestType == TTL_MODE && (TTL_CARD_TYPE==2 || TTL_CARD_TYPE==3))))
{
    if(g_iTestType == RS232_MODE)
        WriteLog("Handler ==> RS232 MODE");
    else if(g_iTestType == TTL_MODE)
        WriteLog("Handler ==> TTL MODE");
    fRS232Aux->OpenTesterComm();
}
else
    WriteLog("Handler ==> Other MODE");
```

### C9（Bug Fix）：HT9045 SendMSG_TestMode — 補設 iLotStatus
```cpp
HHandler2Gpib.iLotStatus = TestIF_File.iTestType;  //Steven 20260409
```
**Bug 說明**：未設定時 `iLotStatus` 為 struct 預設值 0 = `TTL_MODE`，
導致 GPIB 模式下 log 顯示「Change tester interface: TTL ()」。
→ 詳見 [bug-log.md § BUG-001](bug-log.md)

### C10：RS232Std — ShowCommData 同步 WriteLog + log 路徑統一
（詳見上方 Log 路徑統一區塊）

### WriteLastDataFile() — 關閉時持久化
```cpp
WriteIniData(asGeneralPath, "SystemSetup", "iTestType", g_iTestType);  //Steven 20260409
```

---

## HT9045 端修改（HT9011UC_Code_V3.33.901.0_20260408_Steven）

### Phase 1：SendMessageToGpibProg() 攜帶 iTestType
```cpp
HHandler2Gpib.iLotStatus = TestIF_File.iTestType;  //Steven 20260409
```

### 首次連線通知（ProcessHVisionConnect）
```cpp
static HWND sLastHVisionWnd = NULL;  //Steven 20260409

if(sLastHVisionWnd==NULL && HVisionWnd!=NULL)
    SendMessageToGpibProg();         //Steven 20260409 : 首次連上時通知一次
sLastHVisionWnd = HVisionWnd;

// HVisionWnd==NULL 分支（GPIB 斷線）
sLastHVisionWnd = NULL;             //Steven 20260409 : 重置，下次重連可再觸發
```

**設計原則**（→ 詳見 [design-decisions.md § DD-004](design-decisions.md)）：
- ❌ 不放在 `bFind=true` 後 → 每秒重複執行
- ✅ 用 `static HWND` 記錄前次狀態，只在「NULL → 有值」時觸發一次

---

## FormDestroy / FormClose 安全修正（2026-04-09）

→ 詳見 [bug-log.md § BUG-002](bug-log.md)

### 修正後 FormDestroy
```cpp
void __fastcall TfRS232Std::FormDestroy(TObject *Sender)
{
    InitialOK = false;

    for(auto iter = MY_DUT_PAL.begin(); iter != MY_DUT_PAL.end(); ++iter)
        delete *iter;
    MY_DUT_PAL.clear();

    Application->OnException = NULL;

    if(slRS232Log != NULL)
    {
        slRS232Log->Clear();
        delete slRS232Log;
        slRS232Log = NULL;
    }
}
```

### 修正後 FormClose（防 double-free）
```cpp
delete sBarCode;        sBarCode = NULL;
delete sBarCode_ASE_CL; sBarCode_ASE_CL = NULL;
```

---

## iLotStatus 欄位複用說明

→ 詳見 [design-decisions.md § DD-005](design-decisions.md)

| CMD | iLotStatus 原用途 | 現在 |
|-----|-------------------|------|
| `MSG_CMD_LotStatus` | Lot FT/RT 狀態（2/4/8/10/12）| **不動** |
| `MSG_CMD_ChangeGpib` | CC_MTI/PTI 為 0，C8 handler 未讀 | **改為攜帶 iTestType** |
| `MSG_CMD_TesterMode` | 未定義（struct 預設值 0）| **改為攜帶 iTestType** |

---

## DIO / RS232 Type log 對應

### TTL 模式（DIO TypeName）
```cpp
AnsiString sDIOName = CheckAndReadIniData(recipePath+"\\Tester.Data", "DIO", "TypeName", "");
WriteLog("Handler ==> Change tester interface: TTL (" + sDIOName + ")");
```

### RS232 模式（eRs232Mode index）
```cpp
enum eRs232Mode { eRs232Standard=0, eRs23232Bin=1 };
// 0 → "Standard"，1 → "32 Bin"
```

---

## Ini 持久化

| 路徑 | Key | 說明 |
|------|-----|------|
| `D:\GPIB9045\system\general.ini` | `[SystemSetup] iTestType` | Cold start 時還原 g_iTestType，預設 GPIB_MODE |

---

## 注意事項

- `TestGPIB()` 及其呼叫鏈在 RS232/TTL 模式下不執行，無需 guard
- `ReadLastDataFile` / `WriteLastDataFile` 中的 `GPIBVersionCheck` 是硬體版本保護，與 g_iTestType 無關
- 禁止新增 `InterfaceType_RS232Standard` 等，避免模糊 g_iTestType / iTesterMode 職責邊界
