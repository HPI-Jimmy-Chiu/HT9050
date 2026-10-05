# N07 SECS/GEM 連線監測警報 (NetworkMonitor)

> 客戶：943_JSCC（鴻勁興業）｜需求編號：JSCC-20260327-NetConn
> 釋出版本：V3.33.905.3（修正 905.1/905.2 燈不閃問題）
> 建立日期：2026-06-05｜作者：Steven

---

## 1. 功能概述

Handler 生產中即時監測與 MES 主機之間的 SECS/GEM（HSMS）連線。
連線中斷時發出**不停機**警示（閃爍紅框 + 閃爍塔燈紅燈 + 蜂鳴器 + 事件日誌）；
連線恢復後自動解除。預設關閉，由 INI `bN07_Alarm` 啟用。

---

## 2. 全域旗標（cmydef.cpp / cmydef.h）

| 旗標 | 定義位置 | 用途 |
|------|---------|------|
| `bN07AlarmActive` | `cmydef.cpp` ~4455 | 警報是否作用中（main.cpp 設定，ckernel 讀取驅動塔燈/蜂鳴） |
| `bN07BuzzerSilenced` | `cmydef.cpp` | Alarm Reset 是否已消音（蜂鳴靜音但視覺持續） |

> 兩者皆為**全域**，因為「狀態判定」在 main.cpp `Timer2Timer()`，
> 而「塔燈/蜂鳴實體輸出」在 ckernel.cpp `ShowRunLed()`，需跨檔共享。

---

## 3. 狀態機（main.cpp `TfMain::Timer2Timer()`）

### 啟用條件（缺一不可）
```cpp
bool bN07Enable = (IniConfig.bEnable_SECS_GEM==true &&
                   IniConfig.bN07_Alarm==true &&
                   LastSet.iTester==ON_LINE &&
                   SystemStart==true &&     //B2 : 生產中才偵測(與 ckernel else if(SystemStart) 一致)
                   HGem!=NULL);             //A  : 防 SECS 物件空指標當機
```

### 檢查頻率：首次即時 + 後續節流
| 狀態 | 頻率 | 行為 |
|------|------|------|
| 未警報（`bN07AlarmActive==false`） | **每 1 秒** | 一偵測到斷線**立即**觸發（零延遲） |
| 警報中（`bN07AlarmActive==true`） | **每 10 秒** | 查連線是否恢復（節流，避免日誌洗版） |

### 表現層：邊緣偵測（edge detection）
- `bN07AlarmActive==true` → 每輪維持閃爍紅框/label（防被其他邏輯蓋掉）。
- `bN07LastAlarm==true && bN07AlarmActive==false`（下降邊緣）→ 復原一次：
  `bAlarmBuzzer=false`、`bN07BuzzerSilenced=false`、`LoadTestModePicture()`、寫 Released log。
- 下降邊緣統一涵蓋「連線恢復」與「功能關閉/停機」兩種解除原因。

> **為何用下降邊緣**：解除有兩種原因（`bN07Enable=false` 或連線恢復），
> 分屬不同分支；綁任一分支都會漏掉另一個。用 `bN07LastAlarm` 記住上一輪狀態，
> 在 `bN07AlarmActive` 由 true→false 那一刻統一復原一次，兩種原因都涵蓋。

---

## 4. 實體輸出（ckernel.cpp `ShowRunLed()`）

### RunState + 蜂鳴器分支（在 `else if(SystemStart)` 內）
```cpp
else if(bN07AlarmActive)   //Steven 20260603
{
    RunState=LED_Message;
    if(!bN07BuzzerSilenced) bAlarmBuzzer=true;   //尊重 Alarm Reset 消音
}
```

### 塔燈紅燈閃爍（FlushFlag-gated LED 區塊，`else` 分支）
```cpp
if(bN07AlarmActive)   //Steven 20260605 : SECS disconnect -> blink red (JSCC)
{
    fMain->ledRed->Value    = FlushFlag;   //紅燈閃
    fMain->ledGreen->Value  = false;
    fMain->ledYellow->Value = false;       //⚠ 905.1/905.2 此處誤寫 ledRed=false 導致紅燈恆滅
}
else
{
    // 原 LastSet.MessageLight[RunState][x] 三段式
}
```

### Alarm Reset 消音（ScanKey SnRKAlarmReset / SnFKAlarmReset）
```cpp
if(bN07AlarmActive) bN07BuzzerSilenced=true;   //Steven 20260603
```

---

## 5. UI 紅框閃爍（main.cpp 表現層）
```cpp
Off_lineDisplay->BorderWidth = 20;
Off_lineDisplay->Color       = (FlushFlag)?clRed:clBtnFace;   //隨 FlushFlag 閃爍
labTesterMode->Caption       = "SECS DISCONNECTED";
```
> `Off_lineDisplay` 為**共用**面板（OFF-LINE 閃爍、`LoadTestModePicture()` 也用）。
> N07 要求 ON_LINE，與 OFF-LINE 閃爍狀態互斥。復原**交回** `LoadTestModePicture()`（共用擁有者）處理，不硬寫死值。

---

## 6. 精簡日誌（main.cpp `WriteN07Log()`）
```cpp
static void WriteN07Log(const char *pszEvent)
{
    NewRecordProcess("", pszEvent);   //事件日誌（僅邊緣，不洗版）
    // 另寫 D:\SECS_GEM_LOGS\YYYY\MM_DD\NetworkMonitor.log（與 SECS log 同目錄,沿用既有打包）
}
```
事件字串：`SECS/GEM Connection Lost` / `SECS/GEM Connection Restored` / `SECS/GEM Connection Alarm Released`。

---

## 7. 組態（Config.h / cConfiguration.cpp）

| 項目 | 值 |
|------|----|
| INI 欄位 | `IniConfig.bN07_Alarm` |
| UI 控制 | `fConfiguration->chkN07_3_2`（SECS GEM 群組「SECS GEM Alarm」） |
| 預設 | 關閉 |

---

## 8. 已知陷阱（開發踩過）

| 陷阱 | 後果 | 正解 |
|------|------|------|
| 在 main.cpp 設 `ledRed->Value` 想驅動塔燈 | 被 ckernel `ShowRunLed()` 每輪重算蓋掉，塔燈不動 | 塔燈/蜂鳴一律在 ckernel `ShowRunLed()` 內依 `bN07AlarmActive` 強制 |
| ckernel N07 分支 `ledRed=false`（誤打，應 `ledYellow=false`） | 紅燈恆滅、塔燈不閃（905.1/905.2 bug） | 改 `ledYellow=false` |
| 釋放條件含 `bN07AlarmActive==false` | 狀態機死鎖、計數歸零、log 洗版、永不觸發 | 釋放只看 `bN07Enable`；解除用下降邊緣 |
| 每秒無條件 `bAlarmBuzzer=false` | 踩全域共用蜂鳴旗標、誤關其他警報 | 只在下降邊緣設一次 |
| main.cpp 顯示不檢查 SystemStart、ckernel 在 `else if(SystemStart)` 內 | 停機斷線時 UI 紅、塔燈/蜂鳴不動（不一致） | 採 B2：`bN07Enable` 加 `SystemStart`，兩邊一致 |

---

## 9. 驗收測試

| # | 步驟 | 預期 |
|:-:|------|------|
| 1 | 生產中拔網路線 | 立即閃紅框 + 塔燈紅燈閃 + 蜂鳴 + Lost log |
| 2 | 按 Alarm Reset | 蜂鳴靜音，紅框/塔燈持續閃 |
| 3 | 插回網路線 | 自動解除、復原、Restored/Released log |
| 4 | 斷網期間 | 生產不停機 |
| 5 | INI 開關關閉 | 完全靜默，與舊版一致 |
| 6 | 檢視 `D:\SECS_GEM_LOGS\YYYY\MM_DD\NetworkMonitor.log` | 中斷/解除均有時間戳 |
