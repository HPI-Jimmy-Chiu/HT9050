# 評估：移植樹要不要改回多執行緒（20261002，Jerry）

> **狀態：評估，不是工作卡。** Jerry 1002 跟 Jimmy 口頭確認「先在 Jerry 這邊評估，結果推給 Jimmy 排」。
> 本文只做唯讀調查與實測，**沒有改任何程式**。
> 起因：Jerry 模擬時「流程都有跑但動作有點慢」→ 追到 `kServeTickMs = 500`（`RULINGS_20260917.md` B13）。

---

## 0. 一句話結論（20261002 20:3x **第三版**，數字以本版為準）

**瓶頸不是機台邏輯，是餵網頁。** 乾淨實測（op log 關閉，7 個 10 秒窗）每 500 ms：

```
apiCache  ≈ 23.2 ms         ← 服務網頁的 /api/struct/{io,motor} 輪詢
publish   ≈ 15.4 ms         ← 整理 1790 個 tag
          ─────────
            38.5 ms         ← 餵網頁的合計
PumpTick  ≈  2 ms           ← 機台邏輯本身
```

⇒ **`kServeTickMs` 加速的障礙，九成以上跟多執行緒無關。**

### 0.1 ⚠ 而且真正的水管不在 WS，在 HTTP

同一份 log 的 `http` 欄（同 7 個窗）：

| 通道 | 流量 | 說明 |
|---|---:|---|
| **HTTP `/api/struct/io/runtime`** | **512 KB/s** | 5 次／秒 × 約 102 KB（HW.IoSetView 每 200 ms 一次） |
| HTTP dialog mailbox | 16 KB/s | **30 次／秒**（dialog-bridge.js 每 100 ms） |
| **WS tag 串流（patch）** | **0.3 KB/s** | 每 10 秒 3.3 KB |

**HTTP 的 io runtime 輪詢是 WS tag 串流的 1,700 倍。**
tag 串流那邊的問題（1790 個 tag、1089 個沒人讀）管的是**節拍上的 CPU**（publish 15.4 ms／拍），
**不是頻寬**。兩件事要分開談。

⛔ **這也否決了本評估第二版提的「選項 D：apiCache 重建 6 次/秒 → 1–2 次/秒」。**
apiCache 重建 6 次/秒，正是為了餵那個 **5 次/秒**的 HTTP 輪詢；只降重建而不降輪詢，
網頁會拿到過期資料。要省必須**兩邊一起降**，而輪詢側屬 `RULINGS_20260930` 第 12 條的筆電地盤。

⚠ **但多執行緒仍然該做，只是解決的是另一個問題** —— 見 §1.3 的更正：
golden **不是**全部序列化，`ScanBtn` 是真並行、而且**直接對馬達下命令**。
那是「阻塞式 I/O 不要卡住主線」的問題，跟節拍是兩回事。

### ⛔ 本評估前兩版的錯誤（已更正，留著避免有人引用舊結論）

| 版本 | 寫的 | 實際 |
|---|---|---|
| 第一版 | 「golden 自己也是序列化的，多執行緒買不到並行」 | **過度推論**。8 條裡有 **7** 條只是計時器（`Synchronize`），**但 `ScanBtn` 是真並行**（§1.3）。Steven 說「方向還是要往多執行緒走」有依據 |
| 第一版 | 「publish 是大頭，要雙緩衝」 | publish **只佔 24%**。大頭是 apiCache 的 73%（§3.4） |
| 第一版 | 「328 個 tag」 | **1790 個**。328 是 `WebBridgeTags.cpp` 20260908 的舊註解 |
| **第二版** | 「`ScanBtn` **與 `TShuttleThread`** 兩條是真並行」 | **只有 `ScanBtn` 一條**。`uShuttleThread.cpp:60` 有 `Synchronize(ShuttleThreadProcess)`，它跟其餘六條同形 |

---

## 1. golden 怎麼做的（實測，`HT9011UC_Code_V3.33.906.0_20260618`）

### 1.1 有 8 條 `TThread`

```
TRunControl         uruncontrol.h:7          ← 驅動 MainProc
TShuttleThread      uShuttleThread.h:8
THeaterThread       uHeaterThread.h:8
ScanBtn             ScanBtnThread.h:8
TPLCIOThread        MyPLC/MyPLC_IO_Modbus.h:27
TPadRS232Thread     uPadInterface.h:56
TRS232Thread        EJ1N/OmronEJ1N.h:328
TOmronProcessThread EJ1N/OmronEJ1N.h:339
```

### 1.2 其中**七條**只是計時器 —— 用 `Synchronize()` 把工作排回 UI 執行緒

```cpp
// uruncontrol.cpp:42-58
void __fastcall TRunControl::Execute() {
    timeBeginPeriod(1);
    do {
        Synchronize(ThreadProcess);          // ← 排到 UI 執行緒，並等它做完
        if (SystemStart==false || iHome==1 || fContact->fShow==true)
            MySleepEx(1,true);               // ← 只有閒著才睡 1 ms
        else ct++;                           // ← 真的在跑時連睡都不睡
    } while(...);
}
void __fastcall TRunControl::ThreadProcess(void) { if(InitialOK) MainProc(); }
```

```cpp
// uHeaterThread.cpp — 同一個形狀
do { Synchronize(HeaterThreadProcess); MySleepEx(20,true); } while(!bEnd);
```

`Synchronize()` 是 VCL 的「把這個方法排到 **UI 執行緒**執行並等它完成」。

**語意上這等於交回並行**：那段期間工作跑在 UI 執行緒、呼叫端整條卡著等 ⇒ 兩條執行緒同時只有一條在做事。
所以判斷「是不是真的多執行緒」要看的是 **`Synchronize` 外面還剩什麼**。
上面 `TRunControl` 外面只剩一個 `MySleepEx` ⇒ **它實質上是一個計時器，不是工作執行緒**。

**逐支讀過 `Execute()`，這七條是同一個形狀**（`Synchronize(X); sleep;`，本體沒有別的）：

| 執行緒 | 節拍 | `Synchronize` 位置 |
|---|---|---|
| `TRunControl` | 跑的時候**不睡**，閒置 1 ms | `uruncontrol.cpp:49` |
| `THeaterThread` | 20 ms | `uHeaterThread.cpp:72` |
| `TShuttleThread` | 1 ms | `uShuttleThread.cpp:60` |
| `TPLCIOThread` | 1 ms | `MyPLC/MyPLC_IO_Modbus.cpp:242` |
| `TRS232Thread` | 5 ms | `EJ1N/OmronEJ1N.cpp:40` |
| `TOmronProcessThread` | 50 ms | `EJ1N/OmronEJ1N.cpp:59` |
| `TPadRS232Thread` | 1 ms | `uPadInterface.cpp:46` |

> 取得方式：Grep `^\s*Synchronize\(` 掃全樹 `*.cpp` ⇒ **7 個呼叫點、6 個檔**（`OmronEJ1N.cpp` 有兩條執行緒）。
> 8 條 `TThread` 減掉這 7 條 ⇒ **只剩 `ScanBtnThread.cpp` 一個檔完全沒有 `Synchronize`**。

`TRS232Thread` 與 `TPadRS232Thread` 的那一行後面還留著作者的註解：
**「用這個會影響主程式的繪圖效能」** —— 他知道 `Synchronize` 會卡 UI，還是選了它。

### 1.3 ⛔ 但**還有一條是真並行**（`ScanBtn`）—— 第一版漏看了，這裡更正

```cpp
// ScanBtnThread.cpp:97-120   ScanBtn::Execute() —— 全檔沒有一個 Synchronize
do {
  if (bThreadFlag) {
    WaitForSingleObject(eFreeArm, INFINITE);     // ← 在自己的執行緒上無限期阻塞
    WaitForSingleObject(eStart,   INFINITE);
    if (Sen[SnRealTimeCCDStop].IsOn()) {         // ← 直接讀感測器
      MOT[MTestY1].Gali_Command("VS0;SP0,0,0,0;", __FUNC__);  // ← 直接對馬達下煞車
      bRealTimeCCDStop = true;  SetArmState(true);
    } else bRealTimeCCDStop = false;
  }
} while(!bEndThread);
```

這是典型的「**阻塞式等待絕對不可以放在主線上**」：兩個 `INFINITE` 等待一旦放進 `MainProc`，
整台機就停住；所以作者把它獨立成一條真執行緒，並且**在 UI 執行緒之外直接對 Galil 下命令**。

⇒ **更正後的事實**：
* golden 的**機台主邏輯**（`MainProc`）確實序列化在 UI 執行緒，§1.2 那七條都只是它的鬧鐘；
* **但不是全部工作都序列化** —— 即時 CCD 急停掃描是真並行，**而且會碰硬體**。

⇒ 第一版寫的「多執行緒買不到並行」**是錯的**。Steven 1002 口頭說「方向還是要往多執行緒走」
**有 golden 的依據**，只是：
* 依據只有**一條執行緒**（不是第二版寫的兩條），
* 而且它解決的問題**跟節拍無關**（見 §5 的 C′）。

---

## 2. 移植樹現況（實測）

| | golden | 移植樹 |
|---|---|---|
| 機台邏輯在幾條執行緒上 | **1（UI 執行緒）** | **1（wb_serve serve loop）** |
| 驅動者 | `TRunControl` 專職，`SystemStart` 時不睡 | `pumpBeat` 閘，**500 ms 一次** |
| 同一條線上還有什麼 | 畫面重繪 | **1203 Poll、publish、`select()`、api cache、TesterComm、原生視窗泵…** |
| `TRunControl` 有沒有翻譯 | — | ❌ **沒有**（`csystem.cpp:82` 自記 `no Win32 thread translated`） |
| 行程內實際執行緒數 | 8+ | **2**（主迴圈 ＋ 看門狗 `WdThread`，`wb_serve.cpp:300`） |
| socket | — | `select()`，**在主迴圈裡**（`WebBridgeServer.cpp:791`） |

### 2.1 迴圈的形狀

```cpp
// tools/wb_serve.cpp:4565 附近
DWORD wait = ...; if (wait > 50u) wait = 50u;     // 迴圈本身 ≤50 ms 一圈
...
const bool pumpBeat = (long)(now - nextPump) >= 0;
if (pumpBeat) { W906_IoTiming(1); ht9045::PumpTick(); W906_IoTiming(2); ... }
```

**迴圈早就有 50 ms 的能力，只是 `PumpTick` 被 `pumpBeat` 關在 500 ms 上。**
`kServeTickMs = 500` 定義在 `wb_serve.cpp:2931`，**單一常數**。

---

## 3. 實測數字（Jerry 1002，筆電，模擬執行中，**無 1203 卡 ⇒ 無 Poll**）

### 3.1 節拍量化：501 樣本，**100%** 落在 500 ms 格子上

來源：`act.main.stateRecord {"taskListOnly":true}` → `Task_ListWithTime.csv`，取 14:57:00 之後的連續執行窗。

```
每個停留與最近 500 ms 格子的距離：中位數 7 ms、90% 19 ms、最大 39 ms
落在 ±60 ms 內：501 / 501 = 100.0%

 1 拍 = 0.5 s   217 次   43%
 2 拍 = 1.0 s   179 次   36%
 ------------------------------
                396 次   79%   ← 這些轉換「什麼都沒等」，純粹在等下一拍
```

### 3.2 CPU 預算：**每拍 94 ms、整體 18.8%**

```
量測區間 20,019 ms / CPU 3,766 ms = 18.8%
每 500 ms 的拍子裡約 94 ms 在真的做事
```

⚠ 94 ms 是**整個迴圈**在 500 ms 內做的所有事，**不是 `PumpTick` 自己**。這是上界。

### 3.3 ~~只調常數的天花板（用上界估）~~ ⛔ **已被 §3.4 推翻，整節作廢**

> 下表把 94 ms 全算成 `PumpTick`，實測只有 2 ms（§3.4）。**不要引用這張表。**
> 保留原文是因為「140 ms Poll」那兩段警語仍然成立。

| 節拍 | 迴圈佔用 | 可行？ |
|---|---:|---|
| 500 ms（現在） | 19% | ✅ |
| 200 ms | 47% | ⚠ |
| 100 ms | 94% | ❌ |
| 50 ms | 188% | ❌ 跟不上 |

**機台上更緊**：1203 `Poll()` 每 200 ms 一次、**一次約 140 ms**（`wb_serve.cpp:4519` 引同事量的，`wb_publish.cpp:877`）
⇒ 光 Poll 就吃 70%。
⚠ 但 **Poll 不是今天的瓶頸**：500 ms 的閘比 140 ms 大，Poll 只是讓單一拍子抖動（`:4517` 保持相位），
不改變平均頻率。**Poll 是「加速的天花板」，不是「現在的瓶頸」。**
⚠ 140 ms 這個數字**我沒有自己量過**，而且是他那台、那個時間點的；要當決策依據應在機台上重量。

### 3.4 ✅ **94 ms 拆開了**（18:2x，`[STREAM]` 實測，8 個 10 秒窗）

> ✅ **20:3x 已重量，本節下面那組是汙染版，保留只為了記錄汙染有多大 —— 請用 §3.5 的乾淨版。**
>
> ⚠⚠ **19:4x 發現：下面的 apiCache 數字被量測工具自己汙染了。**
> `tools/wb_serve.cpp:6424` 在 apiCache 的計時區間**內**呼叫 `W906_OpLogMotorRuntime`，
> 它在 op log 開著時對整包 motor runtime JSON 做 `cJSON_Parse` 並逐欄位比對寫檔，**每秒約 6 次**
> —— 而 op log 正是為了量 `[STREAM]` 才打開的（`W906_OPLOG_DIR`，實測寫入 361 B/s）。
> 環境變數已清，**重開 VS Code 後要重跑 A／B 對照**。
> `[STREAM]` 不依賴 op log（`WebStreamStats.cpp:210` 先 `printf` 到主控台），對照做得成。
> ⇒ 「apiCache ≈69 ms」是**上界**；publish ≈23 ms 與 PumpTick ≈2 ms **不受影響**（不在那段區間裡），
> 所以 §0 的主結論（瓶頸是餵網頁、不是機台邏輯）仍然成立，只是 apiCache 的佔比會往下修。
> 量法與檢查單：`.claude/skills/ht9045-html-json/references/stream-census-20261002.md` §0.2／§0.2a。

取得方式：`setx W906_OPLOG_DIR D:\HT9045_Log\oplog` → **完全關掉 VS Code 再開** → F5 → 跑 1 分鐘。
（`setx` 只影響之後新啟動的行程；wb_serve 繼承 VS Code 的環境，**開新視窗不夠，要結束 `Code.exe`**。
`StreamEmit` 的 sink 在 `W906_OpLogInit`（`wb_serve.cpp:8340`）才裝，**沒設環境變數就一行都不寫** ——
這就是先前在偵錯主控台找不到 `[STREAM]` 的原因。）

原始輸出（`D:\HT9045_Log\oplog\oplog_20261002.txt`，18:27:44–18:28:54 共 8 筆，取其一）：

```
[STREAM] 10.1s publish n=50 staged avg=1790 ms avg=7.83 max=12.54
         | tags=1790 pci1203=3 secs.sv=772 motionView=0 other=1015
         | apiCache n=60 ms avg=21.08 max=29.50 io=102.1KB
```

8 個窗的範圍：**publish n=50–54、avg 7.60–11.04 ms、max 12.54–55.84**；
**apiCache n=57–67、avg 18.99–27.41 ms、max 29.50–66.15**。

#### 換算成「每 500 ms（一拍）」

| | 每秒次數 | 平均 ms | **每拍耗時** | 佔 94 ms |
|---|---:|---:|---:|---:|
| **apiCache** | 6.0 | 23 | **≈ 69 ms** | **73%** |
| **publish** | 5.1 | 9 | **≈ 23 ms** | **24%** |
| **PumpTick ＋ 其餘** | 2.0 | — | **≈ 2 ms** | **~2%** |

⇒ **`PumpTick()` 本身只佔約 2 ms。吃掉那條執行緒的是「餵網頁」的兩件事，合計約 92 ms。**

#### 兩個順帶更正

* **tag 是 1790 個，不是 328**（`staged avg=1790`）。328 是 `WebBridgeTags.cpp` 20260908 的註解，
  本評估第一版與 §4 #2 都照抄了。
* **publish／apiCache 不跟著 500 ms 的拍子**：每秒各約 5 次與 6 次，都比 `PumpTick`（每秒 2 次）密。
  它們不是「一拍做一次」，是**獨立於節拍、由網頁輪詢驅動**的供應工作。

#### 這把「只調節拍」的天花板整個改寫了

§3.3 的表是用 94 ms 當 `PumpTick` 的上界估的，**那個上界高了 47 倍**。用實測的 2 ms 重算：

| 節拍 | `PumpTick` 佔用 | 可行？ |
|---|---:|---|
| 500 ms（現在） | 0.4% | ✅ |
| 100 ms | 2% | ✅ |
| **50 ms** | **4%** | ✅ |
| 10 ms | 20% | ⚠ 要看機台上的 1203 Poll |

⇒ **狀態機從來不是瓶頸，§3.3 那張「100 ms 就 94%」的表作廢。**
真正擋住節拍的是同一條線上的網頁供應工作（機台上另外還有 1203 Poll）。

---

### 3.5 ✅✅ **乾淨重量（20:3x，op log 關閉）—— 數字以本節為準**

取得方式：使用者停掉 F5 → 從終端機跑**同一支 exe**
（`D:\HT9045\Obj\V906\build_dbg\wb_serve.exe`，cwd＝樹根、無參數）、stdout 轉向到檔案，跑 95 秒。
log 第 175 行 `oplog: off (W906_OPLOG_DIR not set)` 確認環境乾淨。
取 7 個 10 秒窗（丟掉第一個窗 —— 伺服器計數器在那時才設基準）。
`staged avg=1790 max=1790` 每窗相同；`ws conns=1`（使用者的瀏覽器自己接回來，HW.IoSetView 開著）。

| | 每秒次數 | 平均 ms | **每拍（500 ms）** | 汙染版 | 差 |
|---|---:|---:|---:|---:|---:|
| **apiCache** | 6.0 | **7.70** | **23.2 ms** | ~69 ms | **−46 ms（−67%）** |
| **publish** | 4.2 | **7.30** | **15.4 ms** | ~23 ms | −7.6 ms |
| **合計** | | | **38.5 ms** | ~92 ms | **−54 ms** |

⇒ **原本量到的「每拍 94 ms」裡，大約 46 ms 是量測工具自己**
（`W906_OpLogMotorRuntime` 的 `cJSON_Parse`，`wb_serve.cpp:6424`，在 apiCache 的計時區間內）。

其他乾淨數字：

```
WS diff 計算      平均 1.80 ms／次
WS patch          每 10 秒 3.3 KB
HTTP io runtime   每 10 秒 50.1 次 / 5,118 KB   => 512 KB/s
HTTP mailbox      每 10 秒 300.9 次 / 159 KB    => 16 KB/s（30 次／秒）
HTTP motor runtime 0（Motor Test／Motion View 沒開）
```

#### 用乾淨數字重算節拍天花板

> ⛔ **20261005 更正：本表第一版的「迴圈總佔用」欄整欄算錯**（500 ms 那列除外）。
> 它把 38.5 ms 當成「每一拍都會發生」去除以新的節拍 —— 這跟它自己上一行寫的
> 「不隨節拍變快而增加」自相矛盾。正確算法在下面，**結論因此從「100 ms 勉強可行」
> 變成「50 ms 都還有餘裕」**。原表數值（100 ms＝41%、50 ms＝81%）請不要引用。

apiCache 與 publish 由**網頁輪詢**驅動，不隨節拍變快而增加，是每秒固定成本：

```
apiCache   6.0 次/秒 × 7.70 ms = 46.2 ms/s
publish    4.2 次/秒 × 7.30 ms = 30.7 ms/s
                       固定合計 = 76.9 ms/s
PumpTick   2 ms × (1000 / 節拍ms)      ← 只有這一項隨節拍變
```

| 節拍 | `PumpTick` ms/s | 總計 ms/s | **CPU** | 可行？ |
|---|---:|---:|---:|---|
| 500 ms（現在） | 4 | 80.9 | **8.1%** | ✅ |
| 200 ms | 10 | 86.9 | **8.7%** | ✅ |
| **100 ms** | 20 | 96.9 | **9.7%** | ✅ **建議值** |
| 50 ms | 40 | 116.9 | **11.7%** | ✅ |
| 20 ms | 100 | 176.9 | 17.7% | ⚠ 見下 |

#### 真正的限制不是 CPU％，是「單趟最壞延遲」與迴圈的睡眠上限

1. **單趟最壞值**：實測 apiCache `max` 到 18.85 ms、publish `max` 到 13.56 ms。
   兩者落在同一拍就是約 32 ms ⇒ **節拍小於 50 ms 會開始有拍子追不上**。
2. **迴圈本身已經有 50 ms 的睡眠上限**（`wb_serve.cpp:4565`，`if (wait > 50u) wait = 50u;`）
   ⇒ `kServeTickMs` 設成小於 50 的值**不會真的生效**，要一起改那個上限。

⇒ **筆電上的安全區是 50～100 ms；建議取 100 ms**（5 倍，CPU 9.7%，不必動其他任何東西）。
⚠ **機台上另有 1203 `Poll()`（200 ms 一次、約 140 ms）** —— 那個數字我沒自己量過，
而且它會直接吃掉節拍。**機台上的值必須在機台上量過再定，不能照抄筆電的。**

---

## 4. 真正的阻礙（逐項查證）

| # | 阻礙 | 查證 |
|---|---|---|
| 1 | **`PumpTick()` 不持任何鎖** | `WebBridgeTags.cpp:598` 本體無 `FormLock`。⇒ 今天「命令處理器 ↔ 狀態機」的互斥**完全靠同一條執行緒**。任何把 `PumpTick` 搬走的方案都要先解決這個 |
| 2 | **publish 是同一份狀態的讀者** | `PublishHandlerTags`（`WebBridgeTags.cpp:669`）讀 `IniConfig`／`CustomerCode`／`LastSet` 與 **1790** 個 machine tag（§3.4 實測；原寫 328 是舊註解）—— **正是 MainProc 在改的東西**。⇒ 直接搬到別的執行緒＝資料競爭 |
| 3 | `TPci1203Control` 不是執行緒安全的 | `EtherCAT/Pci1203Control.h:799-801` |
| 4 | golden 全域宇宙沒有任何同步 | `cmydef` / `cprod` / `LastSet` … |

### 4.1 ⛔ 更正一條被寫成規則的描述

`glossary.md:59` 寫「**1203 單執行緒規則 ｜ 板卡廠商 API 只能從同一條執行緒呼叫**」。
**原文沒有這樣說。** `Pci1203Control.h:799-801` 的原文是：

```
Not thread-safe, and driven from the SAME thread that polls the monitor --
the vendor API is called from exactly one thread everywhere in this tree.
```

拆開是兩句：①**這個 C++ 類別自己沒有做鎖**；②**這是在描述「移植樹目前的做法」**。
**不是廠商規定。** Advantech SDK 本身是不是 thread-safe，那份註解沒講，本評估也沒查。

⚠ 這種「描述被寫成規則」的條目會讓之後的人以為架構改不了。**建議修正 glossary 那一條。**

---

## 5. 選項（第二版：§3.4 量完後重寫）

| | 內容 | 能買到什麼 | 風險／成本 |
|---|---|---|---|
| **D（新，建議先做）** | **降低 apiCache 的重建頻率**：每秒 6 次 → 1–2 次 | **~50 ms／拍** | **極低**，改一個節流常數 |
| **A** | 只調 `kServeTickMs` | 節拍 500 → 100 ms 以下 | 低，但**要先做 D／B′**，否則 92 ms 的供應工作會吃滿 |
| **B′** | 把 **apiCache ＋ publish** 搬到 worker 執行緒 | **~92 ms／拍** | 中 —— 要解 §4 #1／#2 的資料競爭 |
| **C′** | 照 golden 補 `ScanBtn` 那種**真並行**的執行緒 | **跟節拍無關** | 高，但**那是 Steven 要的方向，而且 golden 真的有**（§1.3） |
| ~~C~~ | ~~補翻 `TRunControl` 讓 `MainProc` 跑在自己的執行緒~~ | ~~並行跑機台邏輯~~ | **不建議** —— golden 的 `TRunControl` 自己就 `Synchronize` 回 UI 執行緒（§1.2），這個收益 golden 根本沒有 |

### 5.1 ⚠ C′ 跟 B′／D 解決的是**不同問題**

| | 問題 | 症狀 |
|---|---|---|
| **D／B′** | **節拍太慢** | 「流程都有跑但動作有點慢」；79% 的狀態轉換什麼都沒等，純粹在等下一拍（§3.1） |
| **C′** | **阻塞式 I/O 卡住主線** | 畫面凍住、某個等待把整台機停住 |

**Steven 口頭講的多執行緒比較像 C′。** 它不會讓節拍變快，但它是 golden 唯一有真並行的地方。
⇒ **兩件事都該做，但不是同一張工作卡，也不該互相當前置。**

### 5.2 為什麼 D 排第一

* apiCache 服務的是網頁每 500 ms 的輪詢（`/api/struct/{io,motor}/*`），**但它每秒重建 6 次** ——
  也就是**同一份資料在被取用之前平均重建了 3 次**。
* 降到 1–2 次/秒，網頁看到的新鮮度**不會比它自己的輪詢間隔差**，當場省下約 50 ms／拍。
* 不動執行緒、不動資料所有權、不碰 §4 的任何一條阻礙。

⚠ **D 要先驗一件事**：apiCache 的 6 次/秒**是不是真的由輪詢驅動**，還是有別的呼叫端（例如每次
WS 命令都順手重建）。如果是後者，正確作法是加「髒標記」而不是單純節流。**這點我還沒查。**

---

## 6. 還缺的（§3.4 已經補掉原本的第一塊）

| # | 缺的 | 為什麼要 |
|---|---|---|
| 1 | **機台上的同一份量測** | 筆電沒有 1203 卡 ⇒ 沒有 Poll。140 ms／200 ms 那個數字是同事那台、那個時間點的，**我沒自己量過**。決策要用機台那份 |
| 2 | **apiCache 6 次/秒的驅動來源**（§5.2 的 ⚠） | 決定 D 是「節流」還是「髒標記」 |
| 3 | **Steven 的原生 C++ web 版省多少**（`-DW906_NATIVE_FORMS=ON`，`ui/native/`） | 如果它本來就砍掉大部分 publish／apiCache，**D 和 B′ 可能都不用做** |
| 4 | **`ScanBtn` 在移植樹對應到什麼**（C′ 的前置） | 要知道那兩個 `INFINITE` 等待現在被翻成什麼 |

機台那份的取法：`setx W906_OPLOG_DIR <路徑>` → 重開 → 跑一分鐘 → 回傳 `oplog_YYYYMMDD.txt`。
**唯讀、不改行為、只多寫一個 log 檔**，比原本 §6 寫的「點一次 IO 按鈕」安全（那會切輸出點）。

---

## 7. 給 Jimmy 的問題

1. **先做 D 嗎？**（改 apiCache 節流常數，~50 ms／拍，極低風險）
   —— 如果可以，我需要先查 §6 #2 再提工作卡。
2. **C′（真並行執行緒）要不要立成獨立工作卡給 Steven？**
   它跟 D／B′ 解決的不是同一個問題（§5.1），**不應該互相擋**。
3. **要不要先請 ES02 在機台上取一份 `[STREAM]`？**（§6 #1，唯讀）
   機台有 1203 Poll，那份才是決策數字。
4. **`glossary.md:59` 那條「1203 卡只能一條執行緒」要不要改掉？**（§4.1）
   原文只說「這個類別沒做鎖」＋「目前這棵樹都從同一條呼叫」，**不是廠商規定**。
   留著會讓之後的人以為 B′／C′ 做不了。

---

## 附：本評估引用的位置

| 事實 | 位置 |
|---|---|
| golden 8 條 TThread | `uruncontrol.h:7`、`uHeaterThread.h:8`、`uShuttleThread.h:8`、`ScanBtnThread.h:8`、`MyPLC/MyPLC_IO_Modbus.h:27`、`uPadInterface.h:56`、`EJ1N/OmronEJ1N.h:328`／`:339` |
| golden **7 個** `Synchronize` 呼叫點 | `uruncontrol.cpp:49`、`uHeaterThread.cpp:72`、`uShuttleThread.cpp:60`、`MyPLC/MyPLC_IO_Modbus.cpp:242`、`EJ1N/OmronEJ1N.cpp:40`／`:59`、`uPadInterface.cpp:46` |
| golden **唯一**沒有 Synchronize 的執行緒 | `ScanBtnThread.cpp:97-120` |
| `[STREAM]` 實測原始資料 | `D:\HT9045_Log\oplog\oplog_20261002.txt` 18:27:44–18:28:54 |
| `[STREAM]` sink 的裝設點 | `tools/wb_serve.cpp:8340`（`W906_OpLogInit`，需 `W906_OPLOG_DIR`） |
| `TRunControl` 沒翻譯 | `csystem.cpp:82` |
| 節拍常數與警告 | `tools/wb_serve.cpp:2931`、`:4509-4525` |
| 迴圈 ≤50 ms、`pumpBeat` | `tools/wb_serve.cpp:4565`、`:4578-4597` |
| B13 裁決 | `docs/RULINGS_20260917.md` B13 |
| PumpTick 不持鎖 | `WebBridgeTags.cpp:598` |
| publish 讀 golden 全域 | `WebBridgeTags.cpp:669-673` |
| 1203 執行緒註解原文 | `EtherCAT/Pci1203Control.h:799-801` |
| 分段計時器 | `JsonBridge/IoBtnPanelClick.cpp:203`、`:505` |
| 節拍實測 501 樣本 | `FROM_JERRY.md` J-15 |
