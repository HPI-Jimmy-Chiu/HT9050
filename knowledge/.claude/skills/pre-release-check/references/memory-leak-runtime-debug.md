# Memory Leak Runtime Debug（記憶體洩漏現場追查指南）

> **範圍聲明**：本文件屬於 **Runtime 現場除錯**（事後追查）工具，**不屬於靜態掃描 / pre-release check 流程**。
> 靜態 code pattern 請見 [patterns.md §P10](patterns.md) 與 [patterns.md §P10b](patterns.md)。
> 機種：HT-1032（V3.32.825.0）　整理日期：2026-06-18
> 本資料夾為可攜範本：同層已附兩個對照 LOG，方便直接照著看一次完整流程。

---

## 0. 使用前必讀

> ⚠️ **這是「現場臨時埋 code」的 debug 手法，不是正式版功能。**
>
> - 埋點只在追問題時暫時加進去、追完就拔掉，**不要留在出貨版本**。
> - 定時抓 MEM、寫檔都有 I/O 與 CPU 成本，常駐會影響生產節拍效能。
> - 全程用 `[DebugLog]` 開關 gate 住，預設關閉；就算誤留，至少不會在客戶端持續耗效能。

### 為什麼叫「Simple」

實際做下來發現：機台有**多個 timer / event 與 MainProc 並行**，所以在 `DoAllProcess()` 裡逐個 stage 分切量前後差值（delta）**效益不大**——culprit 不一定發生在 MainProc 這條 thread 上，分切結果會被別條 thread 的配置干擾。

這個範本真正有效的地方很單純：

- **定時記錄整體 process 記憶體（MEM Sample）＋ 拿時間戳對照 EventLog**。
- 適合快速抓「**使用者操作驅動的大型 leak**」（例：按一次某按鈕跳 100MB）。
- 對「**小型、緩慢累積的 leak**」幫不上忙——那種要靠專門的 heap profiler 或逐模組 review。

先認清這個定位，再決定要不要用它。

---

## 1. 埋 Log 方法

### 1.1 底層寫檔（既有機制，不另發明）

直接沿用既有的 `Save_Msg_Log()`，寫到 `D:\HT9045_Log\Msg_Logs\yyyy_mm_dd_log.txt`。

`csystem.cpp` → `Save_Msg_Log()`
```cpp
void Save_Msg_Log(AnsiString sTemp)
{
    char str[256],t[2048];
    FILE *F;

    AnsiString sDir = "D:\\HT9045_Log\\Msg_Logs";
    if(DirectoryExists(sDir)==false)
        ForceDirectories(sDir);

    sprintf(str,"%s\\%s_log.txt",sDir,Now().FormatString("yyyy_mm_dd"));
    F=fopen(str,"a");
    if(F!=NULL)
    {
        sTemp = Now().FormatString("hh:nn:ss:zzz :") + sTemp + "\n";
        sprintf(t,"%s",sTemp);
        fputs(t,F);
    }
    fclose(F);
}
```

### 1.2 主工具：在 MainProc 定時抓一筆 MEM Sample

這是整個範本的核心。取 process 記憶體最容易移植的寫法是 `GetProcessMemoryInfo()`（需 `#include <psapi.h>`、連結 `psapi.lib`）。

可攜最小版（fork 沒有現成 helper 時直接抄）：
```cpp
void TraceMemSample(AnsiString sTag)
{
    PROCESS_MEMORY_COUNTERS pmc;
    if(GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
    {
        AnsiString s;
        s.sprintf("MEM|Mode=Sample|%s|WS_KB=%lu|PF_KB=%lu",
                  sTag.c_str(),
                  (unsigned long)(pmc.WorkingSetSize/1024),
                  (unsigned long)(pmc.PagefileUsage/1024));
        Save_Msg_Log(s);
    }
}
```

在 `MainProc()` 入口加定時節流（每 N 毫秒一筆，避免每個 tick 都寫檔）：
```cpp
if(bEnableMemTrace)
{
    DWORD dwNow=MyTickCount();
    if(dwLastMemTrace==0 || (dwNow-dwLastMemTrace)>=(DWORD)iMemSampleMS)
    {
        TraceMemSample("Tag=MainProc");
        dwLastMemTrace=dwNow;
    }
}
```

> 本專案已把這套包成 `TraceMemorySample("Tag=MainProc")`，由 `IsMemoryTraceLogEnabled()` 控制，參數讀自 `General.ini [DebugLog]`（見 `csystem.cpp`）。fork 若沒有，就用上面的最小版。

輸出長相：
```txt
MEM|Mode=Sample|Tag=MainProc|WS_KB=399280|PF_KB=398708
```

### 1.3 參數（General.ini）

```ini
[DebugLog]
bEnableMemoryTraceLog=1      ; 總開關，追問題時才開，出貨關掉
iMemoryTraceSampleMS=30000   ; MainProc 採樣週期(ms)；要看突發跳升再改小到 5000
```

| 參數 | 用途 | 建議 |
|---|---|---|
| `bEnableMemoryTraceLog` | 埋點總開關 | 預設 0，現場追問題時才設 1 |
| `iMemoryTraceSampleMS` | MainProc 採樣週期 | 先 30000 建大時間軸；想抓單次跳升降到 5000 |

> 這些參數是 process 啟動讀入一次，**改完 General.ini 要重開程式**才生效。

> 補充：本範本不採用「DoAllProcess 各 stage 量 delta」與「逐容器 count」那一套加大版——原因見 §0（多 thread 並行使 delta 失準），且加越多埋點對效能與判讀都越不利。保持 Simple。

---

## 2. Log 怎麼看

### 2.1 判讀順序

1. 先看 `Mode=Sample` 的 `WS_KB` 有沒有真的往上走。
2. 圈出**第一次明顯跳升**的時間區間。
3. 拿那段時間去對 **EventLog**，看當下使用者按了什麼（Teach / Motor Tools / Reload / 換 Lot…）。
4. 跳升能對到某個按鈕或 reload 動作 → 往那個按鈕事件、reload path 查 `new` / `clear` 不 `delete`。
5. 重做同一動作 3～5 次：每次都再往上堆 = 真 leak；停在某水位 = 一次性 cache，不是 leak。

### 2.2 對照範例（本資料夾附的兩個 LOG）

- MEM 跳升樣本：[2026_05_26_log.txt](2026_05_26_log.txt)
- 同時段操作：[EventLogTxt_20260526.csv](EventLogTxt_20260526.csv)

---

## 3. 實戰案例：Reload Motor Data 每按一次漏 ~130MB

### 3.1 證據鏈

MEM Sample 出現階梯式跳升，與 EventLog 的「Reload Motor Data」按鈕一一對上：

| 時間 | WS_KB | 跳升 | 對應操作（EventLog） |
|------|-------|------|----------------------|
| 14:00:04 | 399,280 | — | — |
| 14:00:34 | 520,032 | +118 MB | 14:00:12 btnReloadMotorData |
| 14:53:06 | 659,972 | +129 MB | 14:52:52 btnReloadMotorData |
| 14:59:07 | 804,428 | +270 MB | 14:58:51 btnReloadMotorData |

型態是「**按一次跳一階、不釋放**」，典型的使用者操作驅動大型 leak——正是這個範本最擅長抓的。

### 3.2 追到根因：InitialMotorParameter() 重建 PordRec

路徑：Motor Tools「Reload Motor Data」→ `btnReloadMotorDataClick()` → `InitialMotorParameter()`。

函式結尾對 18 個 tray 馬達 × `_MAX_X_ITEM` × `_MAX_Y_ITEM` 個格子配置 `TMyProductionRecord`。原本**無條件 `new`，舊指標直接被覆寫沒 delete**，每按一次就整批重配一次。每個 record 內含一個 `TStringList`，乘上格子數後量很可觀。

`cinitial.cpp` → `InitialMotorParameter()`（**修正前**：18 行無條件 new）
```cpp
MOT[MMTrayY].Tray.PordRec[x][y]=new TMyProductionRecord();   //舊指標沒 delete 就被覆寫
//...其餘 17 個 tray 馬達同理
```

`cinitial.cpp` → `InitialMotorParameter()`（**修正後＝目前 code base**：只在 NULL 時配置）
```cpp
    const int iProdRecMotorCount=18;                                            //Ray 20260526 : Free old production record before rebuild
    int iProdRecMotor[iProdRecMotorCount]={MMTrayY,
                                           MMPlate1,
                                           MMPlate2,
                                           MMAuto1,
                                           MMAuto2,
                                           MMAuto3,
                                           MMAuto4,
                                           MMAuto5,
                                           MMAuto6,
                                           MManualTray1,
                                           MManualTray2,
                                           MManualTray3,
                                           MManualTray4,
                                           MManualTray5,
                                           MManualTray6,
                                           MOutRotateKit,
                                           MInRotateKit,
                                           MMBulkboxKit};

    for(int x=0; x<_MAX_X_ITEM; x++)                                            //Steven 20221005 : Production Log減少記憶體使用量
    {
        for(int y=0; y<_MAX_Y_ITEM; y++)
        {
            for(int i=0; i<iProdRecMotorCount; i++)                             //Ray 20260526 : No need to create production record for motor reload
            {
                if(MOT[iProdRecMotor[i]].Tray.PordRec[x][y]==NULL)
                    MOT[iProdRecMotor[i]].Tray.PordRec[x][y]=new TMyProductionRecord();
            }
        }
    }
```

為什麼是「保留舊物件」而不是「先 delete 再重建」：reload 可在 lot 進行中按，`PordRec` 存的是在席 IC 的生產履歷；重建只會得到一個位元相同的空物件（建構子不讀任何 csv），且 MainProc thread 隨時在 dereference 這些指標，UI thread 去 delete 會造成跨執行緒懸空。所以正解是「首次啟動配置一次、保證非 NULL」，與同函式 `MOT[i].Motor` 既有寫法一致。

### 3.3 同型的 reload vector\<T*\>：LoadMotData() 與 LoadIoData()

同一型 bug 出現在兩個 reload 路徑（commit 5a463618 一起補）：`MotTable`（`vector<TMOTDATA*>`）與 `IOTable`（`vector<TIODATA*>`）原本都直接 `.clear()` 把指標丟掉，物件本體沒回收。reload 前要先逐一 delete。

`database.cpp` → `LoadMotData()`（Motor Tools「Reload Motor Data」→ `InitialMotorParameter()` 內觸發；目前 code base）
```cpp
for(unsigned int i=0; i<MotTable.size(); i++)                           //Ray 20260526 : Free old motor table before reload
{
    delete MotTable[i];
}
mapMotTable.clear();
MotTable.clear();
```

`database.cpp` → `LoadIoData()`（IO 畫面 Reload 按鈕觸發；目前 code base）
```cpp
for(unsigned int i=0; i<IOTable.size(); i++)                            //Ray 20260526 : Free old IO table before reload
{
    delete IOTable[i];
}
mapIOTable.clear();
IOTable.clear();
```

> `mapMotTable` / `mapIOTable` 存的是索引不是指標，`clear()` 即可；要 delete 的是 `MotTable` / `IOTable` 裡 `new` 出來的元素。
>
> 通則：`vector<T*>` 的 `.clear()` / `.erase()` 之前，必須先 `delete` 每個元素。

### 3.4 順手修到的真 bug：InitialRecord() 的「假重建」

這一處**不是漏記憶體那麼單純，是一行寫錯的「重建」**，值得單獨記住。

`Public/MyProductionRecord.cpp` → `InitialRecord()`（**修正前**）
```cpp
if(asBuffer->Count!=eDataTotal)
{
    DeleteProductionRecord();
    TMyProductionRecord();          //bug:暫時物件,沒有任何效果
}
```

關鍵觀念：**`TMyProductionRecord();` 這行不是在「重新初始化目前這個物件」。** 在 C++ 裡把建構子當成一條語句呼叫，只會生出一個**匿名暫時物件**、用完立刻銷毀——它配置的內部 `TStringList` 漏掉，而 `this->asBuffer` 在前一行 `DeleteProductionRecord()` 之後仍是 NULL。接著下方迴圈 `asBuffer->Strings[i]` 直接對 NULL 解參考 → 當機。觸發條件是欄位數 `eDataTotal` 與既有 buffer 不一致（升版改過 enum 就會踩到）。

`Public/MyProductionRecord.cpp` → `InitialRecord()`（**修正後＝目前 code base**：直接重建成員）
```cpp
if(asBuffer==NULL || asBuffer->Count!=eDataTotal)
{
    DeleteProductionRecord();
    //TMyProductionRecord();        //保留註解標示原 bug 寫法
    asBuffer=new TStringList();     //要重建的是「成員」，直接 member = new ...
    for(int i=eScheduleName; i<eDataTotal; i++)
        asBuffer->Add("");
}
```

搭配建構子收斂成單一初始化出處（行為不變，`DeleteProductionRecord()` 對 NULL 是 no-op）：

`Public/MyProductionRecord.cpp` → `TMyProductionRecord()`（目前 code base）
```cpp
TMyProductionRecord::TMyProductionRecord()
{
    asBuffer=NULL;          //buffer 建立統一交給 InitialRecord()
    InitialRecord();
```

---

## 4. 通用防範原則（code review 時對照）

1. `vector<T*>` 的 `.clear()` / `.erase()` 前必須先 `delete` 元素。
2. 可重入的 `Initial*` / `Load*` / `Reload*` 函式內，`成員/全域指標 = new` 必須有 `if(ptr==NULL)` guard 或先 delete。本程式慣例是 NULL guard（參考 `MOT[i].Motor`）。
3. `new` 與函式結尾 `delete` 之間，每一條 early return 都要補 delete。
4. `TMyProductionRecord();` 這種「建構子當語句」**不會重新初始化 this**，只會產生暫時物件——重建成員請直接 `member = new ...`。

---

## 5. 蒐證包（要別人幫看時，至少附這些）

1. 當天 Msg Log：`D:\HT9045_Log\Msg_Logs\yyyy_mm_dd_log.txt`（含 `MEM|Mode=Sample`）。
2. 同時段 EventLog（對照按鈕 / reload / 換 lot）。
3. 現場使用中的 `General.ini`（至少含 `[DebugLog]`）。
4. 重現步驟，精確到按鈕或功能名稱（例：進 Teach → Motor Tools → Reload Motor Data，重複 5 次）。
5. 問題前後 3～5 分鐘時間窗，不要只丟單點截圖。

> 本資料夾已附 [2026_05_26_log.txt](2026_05_26_log.txt) 與 [EventLogTxt_20260526.csv](EventLogTxt_20260526.csv) 兩個對照範例，可直接照 §2 走一遍判讀流程。
