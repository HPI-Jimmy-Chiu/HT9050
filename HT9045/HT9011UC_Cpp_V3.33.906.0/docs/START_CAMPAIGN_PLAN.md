# START 戰役 —— 讓瀏覽器的 START 真的啟動機台

> **使用者 20260915 裁決：「我就是要讓它動，執行」。**
> 這份是計畫書，不是提案。做到哪一步、還差什麼，一律以本檔 §5 進度表為準。

---

## 0. 一句話

瀏覽器的 START 現在只會變色。要讓它真的啟動，缺的**不是三條接線，是
`TfMain::Start()` 這 1,875 行本身** —— 移植樹裡它是一行空函式。

---

## 1. 現況（20260915 實測，不是引用）

| 層 | 現況 | 位置 |
|---|---|---|
| 網頁按鈕 | 只 `classList.toggle('on')`，不送任何命令 | `web/page/Main.gbControlBtn.html:80` |
| wb_serve 分派 | 認得 8 個命令，**沒有 START** | `tools/wb_serve.cpp:634` 起 |
| C++ 啟動函式 | **一行空函式** | `forms/fMain.cpp:276` |
| golden 的本體 | **1,875 行** | `main.cpp:4385-6259` |

golden 的呼叫鏈：

```
BtnStartClick (main.cpp:6261)
  └─> Start(Func) (main.cpp:4385-6259)   1,875 行的啟動檢查
        └─> …設定 SoftStart…
              └─> kernel tick 看到 SoftStart==true -> 「啟動檢查」區塊
                    └─> SystemStart=true            (ckernel.cpp:517)
                    └─> CheckSafeDoorIsClosed()==false
                          -> SystemStart=false; StopAllMotor(); return false
                                                    (ckernel.cpp:526-531)
```

⚠ **不可以只寫 `SystemStart = true`。** 那會跳過 1,875 行檢查與安全門互鎖。
`bSystemStart` 更不是它 —— 那是 `ComputeCanChangeToSocket()` 的傳值參數
（意思是「已在運轉時不允許切換 Socket」），不是啟動旗標。
真正的全域在 `cmydef.cpp:286`（移植樹）/ `cmydef.cpp:282`（golden）。

### 1.1 為什麼「翻完就會動」

`MachineType.h:48` 的 `SOFT_SIMULTE` 是**註解掉的**，`CMakeLists.txt:1292` 明寫
「this build does not define SOFT_SIMULTE」，而 `wb_serve` 連了完整 god-stack
（`ht9045_motor`、`ht9045_io` 都在 `CMakeLists.txt:3016` 的 LINK_GROUP 裡）。
**翻完並掛上分派，按下去就是真的動。**

### 1.2 ★★ `fMain->Start()` 有 **19 個**呼叫點，只有 2 個被閘住

> **20260915 更正。** 本節第一版只列了 `Command.cpp` 那兩個 `#if 0`，
> 並據此下結論「翻譯不會順手武裝遠端啟動」。**那個結論是錯的** ——
> 我第一次的 grep 樣式太窄（只抓 `Start("`），漏掉巨集與其他檔。
> 用 ripgrep 全樹重掃之後才浮出全貌。

| 檔 | 行 | 是什麼 | 閘住了嗎 |
|---|---|---|---|
| `SECSGEM/uHGemHT9045.cpp` | 4722, 5795, 5800, 7842, 7860, 7873 | **SECS/GEM 遠端 START**（host 下 RCMD 就啟動） | ❌ **活的** |
| `csystem.cpp` | 5646, 5660, 5753, 5806, 5842, 5912 | **Clean-out 完成後自動重啟**（經 `W7C1_FMAIN_START` 巨集） | ❌ **活的** |
| `ainarm9045.cpp` | 10393, 10411, 10472, 10488 | **InArm 取料錯誤後自動重啟** | ❌ **活的** |
| `Automation/uRENESAS_Server.cpp` | 1590, 1831 | RENESAS 自動化 | ❌ **活的** |
| `Automation/automation.cpp` | 1981 | `ProcessBuffer` | ❌ **活的** |
| `Command.cpp` | 16305, 17056 | 遠端控制 / TCP `HTSET,333` | ✅ `#if 0` |

`SECSGEM/uHGemHT9045.cpp:4722` 的實際形狀 —— 裸的條件，沒有任何 gate：

```cpp
if(SystemStart==false)
{
    fMain->Start("SECS GEM RCMD : START");
}
```

`csystem.cpp:4977` 的巨集註解自己寫「**the macros dispatch to the real fMain**」。

**後果：只要 override `TfMain::Start()`，這 19 個全部同時武裝。**
那不是「讓網頁的 START 會動」，是「所有可能啟動機台的路徑同時開始運作」。
`forms/fMain.cpp:276` 那句 `W7-C1: offline do NOT auto re-start` 防的就是這個。

> 順帶記一個 golden 自身的缺陷（`Command.cpp:17054` 自陳）：
> TCP 那條回 `sData1="OK"` 但 Start 根本沒觸發。對面以為啟動了。

---

## 2. 範圍（實測）

| | 行數 |
|---|---|
| `TfMain::Start()` 本體 | **1,875** |
| 它呼叫的 95 個相異符號，移植樹已有 | 87 |
| 移植樹**缺**的 8 個 | **1,276** |
| 合計 | **~3,151** |

缺的 8 個，兩個大的可以先閘、其餘 6 個只有 149 行：

| 符號 | golden | 行數 | 處置 |
|---|---|---|---|
| `TransformFuntion` | `adam6024.cpp:1038` | **759** | 先閘（ADAM 6024 IO 模組） |
| `NETDownloadDataCheck` | `main.cpp:30425` | **368** | 先閘 |
| `CheckInOutArmZHomeSensor` | `main.cpp:32339` | 38 | 翻 |
| `Read2DIDList` | `BarCode/BarCode.cpp:8209` | 38 | 翻 |
| `CheckSmartAutoCleanCanStart` | `AutoClean/uCleaning.cpp:2879` | 37 | 翻 |
| `CheckEmployeeID` | `mymessbox.cpp:1353` | 17 | 翻 |
| `CheckingCheckList` | `uLotInfo.cpp:12413` | 10 | 翻 |
| `spbStartComClick` | `fAOI.cpp:3884` | 9 | 翻 |

`Start()` 碰到的子系統（字面證據，非推論）：
客戶碼分支 48 處／25 個客戶碼、警報與訊息框 82、Tester 介面 72、溫控 ATC 61、
視覺條碼 46、安全門互鎖 28、MES 紀錄 19、氣缸 IO 3。

---

## 3. 三步，順序不可換

| 步 | 內容 | 可逆？ | 阻塞 |
|---|---|---|---|
| **S1** | FW-W3：`control.acquire/release` 單一操作權 | 是（純記帳，不可能讓機台動） | ⛔ `tools/wb_serve.cpp` 被另一個 session 佔用 |
| **S2** | 翻譯 `TfMain::Start()` + 6 個小相依 | 是（**不掛分派就沒人叫得到**） | 無 |
| **S3** | 掛上 wb_serve 分派 + 網頁按鈕送命令 | **否 —— 機台會動** | ⛔ 同 S1，且**要人站在機台旁** |

**S2 與 S3 必須是不同的 commit。** 翻譯與武裝是兩件事 ——
這是 pt-wave 的既有政策（「行為變更自動做，但單獨一顆 commit、單獨量」）。

S1 是使用者 20260819 裁決的第 3 條（「同時只允許一個瀏覽器有操作權」），
到今天還沒做。**沒有它就掛 S3 = 兩個瀏覽器可以同時按 START。**

---

## 3.5 ★ 20260915 修正：翻譯**不可以**寫進 `forms/fMain.cpp`

動手前查 `forms/fMain.h` 的 FACADE CONTRACT，發現原本的打算是錯的。

> **契約第 1 條**：方法是 `virtual`，`forms/fMain.cpp` 裡的空函式是
> **PERMANENT OFFLINE IMPLEMENTATION —— not scaffolding to be deleted**。
> 真實實作應由 `TfXxxImpl : public TfXxx` 覆寫，再把 `fXxx` 指過去。
> **契約第 4 條**：out-of-scope bodies stay documented no-op stubs,
> **never silently "implemented"**。

也就是說：把 `Start()` 直接翻進 `forms/fMain.cpp`，會讓**每一個建構 `TfMain`
的 ctest 都拿到真的啟動序列**（15 個測試檔直接 deref facade）。
那是「武裝沒編過的碼」的放大版。

### 正確的落點

D3 本來規劃的 impl 是 MFC 那一側的（`ui/FormsFacadeMfc.cpp` 持有
`TfMainImpl : public TfMain`）。**但 MFC 已自產品移除**（`7b86cfd`，
memory `ht9045-ui-direction-web-plus-cpp-core`），而現在的 UI 是 web。

所以 `Start()` 的家是 **web 側的 impl**，形狀照 D3：

```
  WebStart.h / WebStart.cpp（樹根）    class TfMainWeb : public TfMain
                                         bool StartFromWeb(AnsiString Func);   <- 1,875 行住這裡
                                       ★ 刻意**不是** override
  wb_serve.cpp（S3 才動）              fMain = &webImpl;（同一個物件，共用狀態）
  那 19 個 fMain->Start(...)           照舊拿到基底的空函式 -> 全部維持惰性
  ctest / 其他所有行程                  從來不建 impl -> 拿到 forms/fMain.cpp 的 no-op
```

**★ 關鍵：不 override，改加新方法。** §1.2 量到 19 個呼叫點、只有 2 個被閘住。
override 一次就把 SECS/GEM 遠端 START、clean-out 自動重啟、InArm 取料錯誤重啟
全部打開。改用新名字 `StartFromWeb()`，爆炸半徑剛好是一個呼叫者 ——
wb_serve 的指令分派。要讓其他 18 個也能動，是 **18 個獨立的裁決**。

**為什麼還是要重指 `fMain`**：golden 的 Start() 是 `TfMain` 的成員函式，
本體直接寫 `spbUserName->Caption="Operator"` 這種語法，動的是 fMain 自己的狀態。
如果 `TfMainWeb` 是另一個物件，那些寫入會落在一份沒人看的副本上。
所以 S3 要做 `fMain = &webImpl` —— 同一個物件、同一份狀態，
而舊呼叫點因為沒有 override，拿到的仍是基底空函式。

**簽名不用改**：`TfMain::Start` 維持 `virtual void`。實測那 19 個呼叫點
**沒有一個使用回傳值**（`grep -c "if(fMain->Start\|= *fMain->Start"` 三個主要檔
全部回 0），所以改動面積是零。`StartFromWeb()` 自己回 `bool`，承接 golden
那十幾條 early-return false。

---

## 4. S2 的波次切法

安全關鍵的碼要切小才比對得動（pt-wave 單波上限 15k 是給一般翻譯的）。
以頂層控制流為界，每塊約 300 行：

| 波 | 內容 / golden 範圍 | 行數 |
|---|---|---|
| **ST-W0** | **建 `TfMainWeb : public TfMain` 空殼 + CMake 落點 + 簽名 `void`→`bool`。沒有人指向它，零行為變更。** | ~60 |
| ST-W1 | `main.cpp:4385-4693` | 309 |
| ST-W2 | `main.cpp:4694-5044` | 351 |
| ST-W3 | `main.cpp:5045-5345` | 301 |
| ST-W4 | `main.cpp:5346-5646` | 301 |
| ST-W5 | `main.cpp:5647-5958` | 312 |
| ST-W6 | `main.cpp:5959-6259` | 301 |
| ST-W7 | 6 個小相依（149 行）＋ 2 個大的閘起來 | 149 |

**簽名要改**：移植樹現在是 `virtual void Start(AnsiString)`，golden 是
`bool __fastcall Start(AnsiString)`。golden 有多條 early-return false 路徑
（`iStartIn!=0`、`CheckMachineStationOnStart()==false`…），回傳值是承重的。
改 `void` → `bool` 要同步改 `forms/fMain.h:230` 與兩個 `#if 0` 的呼叫點。

**每一波的驗收**：`-fsyntax-only` 自檢 → Debug build 綠 →
`tools/gateverdict.sh <tag>` 失敗集合逐項等於常駐五項。
**絕不看 ctest 的 exit code**（它對任何失敗數都回 8）。

---

## 5. 進度

| 波 | 狀態 | commit |
|---|---|---|
| 範圍量測 | ✅ 20260915 | `34ef1fc` |
| 架構落點更正（見 §3.5） | ✅ 20260915 | `f912fa2` |
| §1.2 更正：19 個呼叫點只有 2 個被閘住 | ✅ 20260915 | 見下 |
| **ST-W0 建 impl 接縫** | **✅ 20260915** | `41871f5` |
| **ST-W1** `main.cpp:4385-4693`（309 行） | **✅ 20260915** | `f149a0b` |
| **ST-W2** `main.cpp:4694-5044`（351 行） | **✅ 20260915** | `d17c04b` |
| **ST-W3** `main.cpp:5045-5345`（301 行） | **✅ 20260915** | `c3010e2` |
| **ST-W4** `main.cpp:5346-5646`（301 行） | **✅ 20260915** | `210580b` |
| **ST-W5** `main.cpp:5647-5958`（312 行） | **✅ 20260915**（經三方對抗式稽核） | 見下 |
| **ST-W6** `main.cpp:5959-6259`（301 行） | **✅ 20260915**（經三方稽核：翻譯完整性 / 引用逐字 / gate 分類） | 見下 |
| ST-W7 | ⛔ **卡住**（`kWave7State=2`，`WebStart.cpp:186`，20260915 改 1->2、20260917 上午 2->1、同日下午 1->2）—— 交付了 6 個（W5-A / W2-G / W1-E / W2-C / W6-H / W1-C），**剩下 43 個 gate 全部需要使用者裁決或屬於別的戰役**（§5.6）。⚠ **2 不是 3**：`/st-wave` 會停，但那是做不下去不是做完，§5.1 的還債清單沒有清空 | |
| S1（FW-W3 操作權） | ⛔ 等 `wb_serve.cpp` 釋出 | |
| S3（掛分派） | ⛔ 等 S1、S2 完成**且使用者在機台旁** | |

---

## 5.1 ST-W1 閘掉的 12 處 —— 這是還債清單，不是完成清單

ST-W1 忠實翻了 309 行，但其中 12 處因為相依不存在而 `#if 0`。
**分兩類，處置的急迫性完全不同。**

### 🔴 會擋啟動的（閘掉 = 少一道安全檢查）

| 標記 | golden | 少了什麼 | 缺的相依 |
|---|---|---|---|
| `W906-ST-W1-I` | :4623-4685 | **Sigurd FTP Automation 比對清單不合也照樣啟動** | `fLotInfo->CheckingCheckList()`（uLotInfo.cpp:12413，10 行） |
| `W906-ST-W1-E` **🟡 部分已還（ST-W7-C，20260915）** | :4537 | ~~Bin 設定有錯也照樣啟動~~ → **已擋住**； 但三個 `ShowMyMessagePWD` 對話框仍閘著，擋下來時**沒有訊息** | `CheckOLPError()` 判定已翻；`ShowMyMessagePWD` 未移植 |
| `W906-ST-W1-H` | :4606-4621 | **有待裝更新包時照樣啟動** | `fFTPClient->DownloadUpdateAutomatically()` |
| `W906-ST-W1-A` | :4432-4439 | CC_PANTHER 少一道啟動前站點檢查（這台不是 PANTHER） | `patFunc` |

**這四個在 S3（掛分派）之前必須補完或明確裁決。**
ST-W7 排 `CheckingCheckList` / `CheckOLPError`；另兩個要單獨決定。

### 🟡 純顯示 / 唯讀通知（閘掉不影響啟動判斷）

| 標記 | golden | 少了什麼 |
|---|---|---|
| `W906-ST-W1-B` | :4441-4460 | RTC 視覺端收不到 auto-teach / step-aside 模式切換（4 個列舉不存在） |
| ~~`W906-ST-W1-C`~~ **✅ 已還（ST-W7-H，20260915）** | :4483 | ~~2DID 不在 List 內的分 bin 清單沒載入（`Read2DIDList`）~~ → 已翻成 `W906ST_Read2DIDList()`（`WebStart.cpp:696`），解閘點 `:1239-1244`。⚠ 兩個消費者仍未移植，所以今天行為 delta = 0（見 §5.5 那段警語） |
| `W906-ST-W1-D` | :4518-4523 | ASE-CL 那個訊息出現後，Tray Mapping 視窗不會自動跳出來切頁籤 |
| `W906-ST-W1-F` | :4556-4560 | 2D sort 標籤的可見性（`lbl2DSort`） |
| `W906-ST-W1-G` | :4598-4604 | SECS/GEM 員工 ID 確認（`CheckEmployeeID`） |
| `W906-ST-W1-J` | :4688 | 工程師用的 SITE 開關按鈕狀態（`sbEngSite`） |
| `W906-ST-W1-K` | :4496 | 權限降級後按鈕上的字（`spbUserName`）—— **降級本身照做** |
| `W906-ST-W1-L` | :4531-4532 | 關燈按鈕上的字（`spbLight`）—— **關燈本身照做** |

> ★ K 與 L 是**部分閘**的範例：golden 那幾行同時做「動作」與「把動作顯示出來」，
> 只閘顯示、保留動作。整段閘掉會變成行為變更（JCET 按 START 不再切 OP 權限、
> ASE-CL 的 CCD 燈不會關）。

### ST-W2 閘掉的 11 處

#### 🔴 會擋啟動的

| 標記 | golden | 少了什麼 | 缺的相依 |
|---|---|---|---|
| `W906-ST-W2-H` | :4782-4831 | **ON_LINE 下 GPIB / RS232 沒接好也能按 START**，板子版本不符也不擋。涵蓋 TCP_IP / GPIB / RS232 / TTL 四種介面 | `bFind`（全樹只有 `bFindPickICFail` / `bFindProgram`） |
| `W906-ST-W2-J` | :4855-4879 | 開了 A30 的機台，ON_LINE 下沒確認完 setup teach 點位也能啟動；且未啟用時 `bNeedSetupTeach` 不會被清 | `fAutoTeach`（`forms/fOffSet.h:154-155` 明文記載全樹沒有）、`TfOffSet::fShow/Show` |
| ~~`W906-ST-W2-G`~~ **✅ 已還（ST-W7-B，20260915）** | :4844-4851 | ~~In/Out Arm 的 Z 軸原點感測器沒到位也照樣啟動~~ | `CheckInOutArmZHomeSensor()` 已翻進 `TfMainWeb`，gate 解除。 ⚠ 這條路徑通往**運動指令**，見 §5.3 ST-W7-B |
| ~~`W906-ST-W2-C`~~ **✅ 已還（ST-W7-D，20260915）** | :4734-4735 | ~~Smart AutoClean 發生 Alarm 後，資料還沒清就能按 START~~ | golden `uCleaning.cpp:2879` 那 37 行已翻成自由函式，gate 解除 |
| `W906-ST-W2-D` | :4737-4755 | 開了 A19 的機台，到了 PM 停機日照樣啟動，PM 警告也不跳 | `fPMAlarmInterFace` / `PMAlarm_SYS` |
| `W906-ST-W2-E` | :4757-4769 | 開了 OEE 的機台，OEE 檢查不過也照樣啟動 | `fProductionInfo->CheckOEE_WhenStart()` |
| `W906-ST-W2-I` | :4833-4837 | 海思流程的啟動前判斷不執行 | `FormHS` 是 stub，沒有 `CheckCanRunStart_HS` |
| `W906-ST-W2-K` | :5025-5043 | 2D sorting 模式下 `SortBy2DID_*.csv` 不存在也能按 START | `fSCKART->iInfo_MultiLotCnt` |

#### 🟡 純顯示

| 標記 | golden | 少了什麼 |
|---|---|---|
| `W906-ST-W2-A` | :4694-4695 | KYEC 按 START 時 BarCode 視窗不會自動關（`FormBarcodeReader`） |
| ~~`W906-ST-W2-B`~~ | :4716-4717 | **✅ 20260917 已解**（ST 批次 1）—— `TTrayEditForm_Facade` 落地（`acatchtray_shims.h`）。`fShow==false` 使整個 if 不成立，**行為零改變**；差別只在它從此被型別檢查過 |
| `W906-ST-W2-F` | :4839-4842 | KYEC 按 START 時不會切到 Sort Count 頁面（`fSortCT->PageControl1`） |

> ★ **H / J / K 三處刻意整段閘，不做部分閘** —— 跟 W1 的 K/L 相反，理由要講清楚：
> · **H**：`bFind` 是四個介面分支共用的判斷，只閘其中一支等於替 golden 選路。
> · **J**：golden 的 `else` 分支會寫 `LastSet.bNeedSetupTeach = false`。
>   拿掉條件留 else、或反過來，都是發明行為。
> · **K**：`if / else-if / else` 三岔裡只有中間那個條件缺相依，
>   只閘中間那支就等於替 golden 決定走哪一條路。
> 判準是一致的：**閘掉「做不到的事」可以，替 golden「選一條路」不行。**

### ST-W3 閘掉的 1 處

這一波 301 行**只有一個 gate** —— 移植樹在這一段（幾乎全是各客戶的 LotID／
Lot Start 卡關檢查）的相依已經齊全，9 個相異函式缺 0 個。

| 標記 | golden | 少了什麼 | 缺的相依 |
|---|---|---|---|
| 🔴 `W906-ST-W3-A` | :5047-5080 | 開了 2DID 白名單且 N23 下載方式=2 的機台，白名單是空的也能按 START —— **白名單功能整個失效** | `TfBarCode_Shim` 沒有 `list2DWhitle` |

> ★ 同樣是**整段閘**：golden 是 `if/else` 二岔，只拿掉 `if` 那支會讓 `else`
> （Lot Start / run mode 檢查）在 `N23==2` 時也跑 —— 那是改行為，不是少做事。

#### 照翻但值得記的一個 golden 行為

`golden :5187-5198`（`USE_RFID_READER`，SJSEMI 的 RFID Reader）那一支
**只顯示訊息，不 `return false`、也不重設 `iStartIn`**。看起來像缺陷
（提示完照樣往下跑，而且 `iStartIn` 留在 1），但那是 golden 的行為，
照翻並在原地註明。要改它是使用者的決定。

### ST-W4 閘掉的 8 處

#### 🔴 會擋啟動的

| 標記 | golden | 少了什麼 | 缺的相依 |
|---|---|---|---|
| `W906-ST-W4-A` | :5346-5356 | ATC 主動冷卻的機台，環溫設定超出 25~30 也照樣啟動 | `ATCAmbientTemperCheck()` |
| `W906-ST-W4-B` | :5369-5387 | CC_ASE_M + N13 ARMS：伺服器資料沒下載成功也不要求密碼，直接放行 | `NETDownloadDataCheck()`（368 行）、`fARMS`、`fLotInfo->pnARMSTitle` |
| ~~`W906-ST-W4-C`~~ **✅ 20260923 已解（T7，使用者 17:4x 裁決「b 排在 T6 之後」）** | :5487-5499 | ~~CC_KYEC_LEE 走 ART 模式時，A10 沒開或工作檔不支援也照樣啟動~~ | 純判定早以 `ComputeCheckARTSetupFile`（MainCalcCore.cpp:364）落地；WebStart.cpp 加檔案層 `CheckARTSetupFile()` 餵 golden 讀的五個值，極性與呼叫端副作用照 golden |
| ~~`W906-ST-W4-E`~~ **✅ 20260919 已解（T5-W4E）** | :5550-5563 | ~~ON_LINE 下 FTP 下載資料比對不過也照樣啟動~~ | `NETDownloadDataCheck()` 已逐行翻完（golden 368 行 → `WebStart.cpp:746` 的 `TfMainWeb::NETDownloadDataCheck()`，零個閘），呼叫點解開。碼上 `:737`／`:2659` 的 GATE 字樣是歷史標記 |
| `W906-ST-W4-F` | :5407-5419 | JCET / Murata 機台 OFF_LINE 啟動時不再跳確認框，直接放行 | `ShowMyMessageBox_YES_NO()`（已知未移植的 W7-UI modal） |
| `W906-ST-W4-G` | :5541-5548 | 開了 `bEnableCCDUSETCPIP` 的機台，CCD 辨識失敗也照樣啟動 | `TCCDInterfaceFormShim::iIdentificationStatus` |
| `W906-ST-W4-H` | :5601-5611 | 裝 magazine 的機台不再詢問「空 tray 是否補滿」，直接放行 | `ShowMyMessageBox_YES_NO()` |

#### 🟡 輔助

| 標記 | golden | 少了什麼 |
|---|---|---|
| ~~`W906-ST-W4-D`~~ **✅ 20260925 已解（W906-R28TORQ，使用者裁決 RULINGS_20260925 §6）** | :5528 | ~~`COM2->iWriteAndCheckMotorTorqueDelay.On()` —— 不影響 Start 判斷，但它抑制的是**暫停時的 Torque Time Out**。缺了它，暫停時可能出現 Torque 逾時。~~ | golden TCOM2 的扭力那一半翻進 `rs232.cpp`，`TCOM2Shim` 有了 `iWriteAndCheckMotorTorqueDelay`（golden rs232.h:174），呼叫點照 golden 解開 |

> ★ **W4-F 是這個戰役第一個「部分閘」保留狀態寫入的例子**：
> golden `:5405-5424` 的 `#ifndef SOFT_SIMULTE` 區塊裡，只有 `:5409` 的確認框
> 缺相依。我只閘掉內層 `if (bflag == false) {...}`，保留 `else { bflag = false; }`
> （`:5423`）—— 那是 ON_LINE 時的狀態寫入。
>
> 連帶後果：`bflag` 的**唯一讀取點**被閘掉了，只剩兩個寫入，
> `-Wunused-but-set-variable` 會紅。處置是加 `(void)bflag;` 並註明，
> **不刪 golden 的那兩個寫入** —— 刪掉會讓日後解閘的人以為狀態機本來就沒有這一段。
>
> ⚠ 另外：`cinitial.cpp` 有一個回固定 "NO" 的替代品
> `W8N2_ShowMyMessageBox_YES_NO`（`forms/fLotInfo.h:1346` 提到）。
> **不採用** —— 那會讓確認框永遠答 NO，等於 JCET/Murata 永遠不能 OFF_LINE 啟動。
> 那是改行為，不是少做事。

### ST-W5 閘掉的 9 處（**經三方對抗式稽核，修正兩處分類**）

這一波派了三個唯讀 agent 獨立稽核：引用行號、gate 分類、遺漏。
前兩者的結論我都逐條對 golden 複驗過。

| 稽核 | 結果 |
|---|---|
| 引用行號 | **180 筆全部正確**（我抽驗三條吻合） |
| 遺漏／竄改 | **277 個敘述零缺漏、零條件竄改、零數值竄改、零控制流竄改** |
| gate 分類 | **抓到我兩個錯** ↓ |

#### 🔴 會擋啟動 / fail-open 的

| 標記 | golden | 少了什麼 | 缺的相依 |
|---|---|---|---|
| ~~**`W906-ST-W5-B`**~~ **✅ 20260923 已解（T7）** | :5736-5776 | ~~**⚠ 最嚴重**。見下方「稽核抓到的第一個錯」~~ | `fMain->cbRunStartMode` 已存在（W906-P10）且由 `SetRunStartMode()` 維護 `->Text`；⚠ 缺口：wb_serve 開機沒走 golden 的 SetStartModeData，第一次 SetRunStartMode 前 Text 為空 —— 此時兩個分岔都不成立（＝閘著時的行為），列晨報 |
| ~~`W906-ST-W5-A`~~ **✅ 已還（ST-W7-A，20260915）** | :5677-5681 | ~~裝分離式 SLK 的機台，汽缸位置不對也照樣啟動~~ | `CheckSLKSensor()` 已翻進 `TfMainWeb`，gate 解除 |
| ~~`W906-ST-W5-F`~~ ① **✅ 20260923 已解（ST-W7-W5F，使用者裁決依 §0.5）** | :5797-5813 | ~~ASE 高雄的 OP 權限 + OFF_LINE 卡關不執行~~ | `TfShuttleMove::fShow` 已存在；四個 fShow 包 `W906_FShow`，golden :5814 的 `else if` 已還原 |
| ~~`W906-ST-W5-F`~~ ② **✅ 20260923 已解（同上）** | :5929-5939 | ~~Auto 流道上有 tray 盤也照樣啟動~~（golden 註解寫 "Door not close"） | 同上 |
| ~~`W906-ST-W5-H`~~ **✅ 20260919 已解（T5-W5H）** | :5865-5869 | ~~VTest MES 批次資訊檢查不過也照樣啟動~~ | `TfMesSystem::CheckLotInfor()`（golden 602 行）已翻進 `forms/fMesSystem.cpp`，連帶 `bNoRTBinFixFlag` 的連結阻塞也依 `cmydef.cpp` 的慣例解掉。碼上 `:3068` 那一大段是保留的歷史說明 |

#### 🟡 不影響啟動判斷

| 標記 | golden | 少了什麼 |
|---|---|---|
| `W906-ST-W5-C` | :5906-5909 | OneCycle/CleanOut 後 GPIB 不會被關再開（`fMain->CloseGpibProgram`） |
| `W906-ST-W5-D` | :5917-5918 | 省電倒數不會在 Start 時重啟（`tPSM`） |
| **`W906-ST-W5-E`** | :5927 | 見下方「稽核抓到的第二個錯」 |
| `W906-ST-W5-G` | :5879-5901 | **ATC 不會自動上線／開冷機／開始運轉**（標頭衝突，非缺符號） |

---

#### ★ 稽核抓到的第一個錯：W5-B 是 🔴 不是「只是旗標沒設」

我原本寫「起始模式下 `fAllMotorHome` 不會被清」就停住了。追下去：

```
golden :5739  fAllMotorHome = false;        ← 被這個 gate 吞掉的那一行
golden :6123  if (fAllMotorHome == false)   ← 全 Start() 裡唯一的讀取點
golden :6164  Home("Home by Start");
golden :6165  iStartIn = 0;
golden :6166  return false;
```

實測 `fAllMotorHome` 在 `main.cpp:4385-6259` 內**只出現這兩次**，一寫一讀。
所以 `:5739` 正是逼出 `:6166` 那條 `return false` 的輸入。

**閘掉之後，Initial Start / Initial Retest 在 `fAllMotorHome` 已經是 true 時，
不會走「先回原點、return false」那條路，而是一路往下真的開跑 —— 也就是沒 Home 就跑。**
方向是 fail-open。

`:6123` 落在 ST-W6 範圍，今天還是潛伏的，但分類**現在**就要標 🔴 ——
否則 ST-W6 落地時沒有人會回頭看這裡。

> 順帶更正我自己另一句話：原註解寫「`bResetIsPressed` 不會被放掉」，**那是錯的** ——
> golden `:5958` 的 `bResetIsPressed = false;` 是無條件的，而且已經翻進去了。

#### ★ 稽核抓到的第二個錯：W5-E 是 🟡 不是 🔴

我看到 golden 註解寫「for 安全門未關按 Start 時 IndexArm 會先動作」就標成 🔴 ——
**被歷史成因的註解帶偏，沒去看它實際做什麼。**

`bStartKeyPressCheck` 的實際機制在 golden `csystem.cpp:4657-4672`：
讀到 true 時做一次性的 Galil VS/SP 速度補寫。**沒有 return、沒有 StopAllMotor、
沒有動 SystemStart。**

而且**移植樹裡那個唯一的讀取點本來就已經被閘了** ——
`csystem.cpp:18960` 的 `#if 0 // GATE G22`，banner 自己寫明
「the one-shot Galil VS/SP speed re-issue ... is not performed」。

所以這個 gate 今天的行為 delta 是 **0**。標 🔴 會擠掉真正 fail-open 的 W5-A / W5-F。

#### 兩處結論對但原本沒留證，已補

- **W5-F①** 把 `else if`（golden :5814）升格成裸 `if`。看起來違反「不要替 golden 選路」，
  證據是 `MachineType.h:308 CC_ASE_KaohSiung=936` 與 `:346 CC_GIGAS=970` ——
  同一個 `CUSTOMER_CODE` 不可能同時等於兩個相異常數，那個 `else` 從來不承重。
- **W5-G** 裡有一行**不是因為缺符號而被閘**：`golden :5891 bRunATC = true;`。
  `bRunATC` 是活的（`cmydef.h:4091` / `cmydef.cpp:4263`），技術上可以留在外面。
  刻意一起閘掉，因為它有真正的消費者（`SECSGEM/uHGemHT9045_SV.cpp:238` 把
  **SVID 1045** 直接註冊到 `&bRunATC`）—— 在 ATC 根本沒被叫起來的情況下設成 true，
  等於**對外謊報機台狀態**，比少做事糟。
  這是本檔「保留狀態寫入」慣例（W1-K/L、W4-F、W5-C）的**唯一例外**。

#### 一個新型的失敗：`-fsyntax-only` 綠但 `ld` 炸，而且連鎖

W5-H 不是編譯期看得到的：

```
WebStart.cpp 引用 TfMesSystem::CheckLotInfor()
  -> 連結器去 libht9045_forms.a 抽出 fMesSystem.cpp.obj
    -> 暴露出那支 TU 自己的未解符號 bNoRTBinFixFlag
```

⚠ **`bNoRTBinFixFlag` 全樹沒有定義**（實測掃過所有 `libht9045_*.a`）。
`fMesSystem.cpp` 一直帶著一個壞掉的相依，只是從來沒有任何 target 連結它。
**下一個把它連進去的人會再撞一次。** 這是樹裡既有的缺陷，修它不在 ST 戰役範圍。

**教訓：ST 波次的驗收必須跑到「連結」，不能停在 `-fsyntax-only`。**

#### ST-W7 的優先序（依量測修正）

1. ~~`CheckSLKSensor`（W5-A，氣缸位置）~~ **✅ 20260915 做完，見 §5.3**
2. `CheckInOutArmZHomeSensor`（W2-G，Z 軸原點感測器）← **現在做這個**
3. `fMain->cbRunStartMode`（W5-B，沒 Home 就跑）
4. **`fShow` 那一組 —— 但它不是「補一個成員」，見 §5.2**

W5-E 不在優先序內（delta = 0）。

### ST-W6 閘掉的 12 處

`main.cpp:5959-6259`（301 行）翻完，1875/1875 行到頂。12 處 `#if 0`。
**經第三份稽核逐一複驗分類，零誤標**，但有三處原本沒寫出 🔴/🟡 標籤，已補。

#### 🔴 會擋啟動的

| 代號 | golden | 缺什麼 | 閘掉的後果 |
|---|---|---|---|
| W6-A | :5961-5973 | `TfShuttleMove::fShow` / `fTeach` | 見 §5.2（`fShow` 那一組） |
| W6-I | :5975-5983 | `CheckSiteMapState()` 無參數多載 | 跑 AutoSiteMap 的機台，在主畫面關掉 site 也照樣啟動 |
| W6-C | :6097-6108 | `fBarCode->RunCheckBarcodeByServerData()` | 條碼比對伺服器資料的啟動前檢查不做 |
| ~~**W6-F**~~ **✅ 20260918 已解閘（S3-B1，武裝）** | **:6177-6232** | ~~`fShuttleMove->fShow`（:6179/:6202）、`fContact->fShow`（:6204）~~ | ⚠⚠ **這一列原本寫「★ `SoftStart` 不會被設成 true —— 機台不會開始跑」，那句話從 20260918 起就是錯的。** `AI(W906-ST-S3-B1)` 把 `#if 0` 改成 `#if 1`（`WebStart.cpp:3635`），分支結構完整保留，**三個 `SoftStart = true`（golden :6196/:6207/:6230）全部活著**。還原＝改回 `#if 0` |

> ⚠ **W6-F 標 🔴 是刻意的，儘管 golden 那一段沒有 `return false`。**
> §5.1 的判準是「golden 有沒有用它擋啟動」，而這一段是**唯一**把 `SoftStart`
> 設成 true 的地方。標 🟡（純顯示）會嚴重低估它。
> 實測：`WebStart.cpp` 三個 `SoftStart = true` 全在這個 `#if 0` 內，
> 而下游鏈路是活的（`ckernel.cpp:816 if(SoftStart==true)` → `:1015 SystemStart=true`）。
> **刀口就在這裡。** 這也是為什麼 ST-W6 翻完之後 `StartFromWeb()` 雖然有
> `return true` 的路徑（`:2560` 是唯一一個），走到底仍然不會讓機台動。

#### 🟡 不擋啟動

| 代號 | golden | 缺什麼 | 閘掉的後果 |
|---|---|---|---|
| W6-B | :5992-5995 | `fBarCode->spbStartComClick()` | 啟動時不重開條碼通訊埠 |
| W6-L | :5996-5997 | `fNote->t2DCode` | 2D code 備註欄不清空 |
| W6-J | :6042-6046 | `TCPCommandServer` / `TeraTCPResultServer` | 關掉 TeraPower 功能時那兩個伺服器不被停用（今天 delta≈0） |
| W6-K | :6110-6113 | `fMain->machineTime` | CC_PANTHER 的運轉時數統計少算 |
| ~~W6-D~~ | :6154-6159 | **✅ 20260917 已解**（ST-W7）—— 唯一阻塞是 `RunCheckStart()`，已翻（golden `main.cpp:32170-32209`，本體在 `forms/fMain.cpp` 檔尾）。`fShuttleMove->fShow` 與 `Zteach->fShow` 在 ST 批次 1 已備齊 |
| ~~W6-E~~ | :6169-6173 | **✅ 20260917 已解**（ST-W7）—— 同 W6-D，唯一阻塞是 `RunCheckStart()` |
| ~~**W6-G**~~ **✅ 20260919 已解閘（P2b）** | :6233-6254 | ~~`TransformFuntion()`（golden `adam6024.cpp:1038`，759 行）~~ 759 行已翻進 `adam6024.cpp`，44 個 `fContactForce->…` 引用由 P2a 的 `ContactForceTables()` 承接 | ~~少寫一次 IO~~ → 現在**會**呼叫 `ADAM_DirectWriteData`（golden :6252）。⚠ 今天它是 no-op 樁（`atester_shims.cpp:326`）所以寫不出去；**樁換成真本體那天這個呼叫點會跟著活** |
| ~~W6-H~~ **✅ 已還（ST-W7-G，20260915）** | :6257 | ~~`SET_ESD_Tri_Temp()`~~ 已翻，gate 解除（碼上已無此標記） | ~~ESD 三溫設定不套用~~ |

> ⚠ **W6-G 的 🟡 跟其他 🟡 不同級。** 其餘 🟡 是少顯示一行字，
> 這一個是**少寫一次類比輸出**（`ADAM_DirectWriteData`，下壓力道）。
> 解閘時要照安全關鍵流程走，不可跟顯示類 🟡 批次處理。

#### 三份稽核抓到的自我更正（ST-W6）

| # | 我原本寫的 | 實際 | 抓到的人 |
|---|---|---|---|
| 1 | `ckernel.cpp:517` 設 SystemStart（裸引用） | 那是 **golden** 的行號；移植樹同一行在 `CheckBinSet()` 裡，`SystemStart=true` 在 **:1015**。這棵樹的慣例是裸檔名＝port-relative，所以會指錯路 | 引用稽核 |
| 2 | `MainCalcCore.h:351` | :351 是文件註解內文，`ComputeCheckSiteMapState` 宣告在 **:384** | 引用稽核 |
| 3 | `MainCalcCore.h:375-377` | 引的那句英文跨到 **:378** | 引用稽核 |
| 4 | 「四個分支裡有三個要讀 `fShuttleMove->fShow`」 | **五個**分支（if / else-if ×3 / else）裡**兩個**讀（:6179/:6202），另有一支讀 `fContact->fShow`（:6204）。引的行號本身是對的，錯的是兩個數字 | 引用稽核 |
| 5 | gate D/F/G 只有文字沒有 🔴/🟡 | 已補，並說明 F 為何是 🔴、G 的 🟡 為何不同級 | gate 稽核 |

186 筆 golden `main.cpp` 引用逐字驗證，**0 筆錯**；165/165 golden 行覆蓋、
153/153 字面相同、0 行自創、0 處順序錯誤。

---

---

## 5.3 ST-W7 還債進度

`kWave7State`（`WebStart.cpp`）是這一節的機器可讀版本：
**0 未開始 / 1 進行中 / 2 卡住（等使用者裁決）/ 3 完成**。
`/st-wave` 步驟 0.5 讀它決定要不要交棒給 `/night-loop`。
⚠ **2 和 3 都停，但意義完全不同**：3 是做完，2 是做不下去。

| # | 項目 | 狀態 |
|---|---|---|
| **A** | `CheckSLKSensor()`（還 W5-A 🔴） | **✅ 20260915** |
| **B** | `CheckInOutArmZHomeSensor()`（還 W2-G 🔴） | **✅ 20260915** |
| **C** | `fMain->cbRunStartMode`（W5-B 🔴，沒 Home 就跑） | **✅ 20260923 已解**（`9bd55c4f`「T7：解 W5-B 與 W4-C 兩道 🔴 閘」，WebStart.cpp:2908-2990；0926 對帳時更正本列，原文如下）⚠ **20260923 更新：「做不下去」的前提已經沒了。** 20260915 的死因是「`forms/fMain.h` 沒有 cbRunStartMode 這個 widget」；W906-P10（20260921）已把它加進 facade（`forms/fMain.h:306` 宣告、`forms/fMain.cpp:103` 建構），活碼在維護它的 `->Text`。⇒ **相依齊全、可還**，而且它是今天唯一真的 fail-open 的 gate。下面 20260915 那一整節保留作史料，但結論不再成立。見 §5.8 |
| **D** | `CheckSmartAutoCleanCanStart()`（還 W2-C 🔴） | **✅ 20260915** |
| E | `ATCAmbientTemperCheck()`（W4-A 🔴） | ⛔ **卡住** —— 純計算已存在，缺的是 `iATC_MODE_TYPE` 的來源（整條 ATC 介面未移植）。見下 |
| F | `fShow` 那一組 **12 個**（原記 10 個，實測更正） | ⛔ **卡住** —— 設計提案已寫好：`docs/FSHOW_WEB_VISIBILITY_DESIGN.md`，等使用者回答其中三題 |
| G | `CheckEmployeeID()`（W1-G 🟡，17 行） | ⛔ **卡住** —— 是 `ShowModal()` 家族。照翻會對 host 送出**帶空白憑證**的 SECS 事件，比閘掉更糟。見下 |
| H | ~~其餘 27 個未分類 gate 裡的剩餘候選（如 `CheckARTSetupFile`）~~ **這個敘述已作廢** | **✅ 20260923 對帳收掉，見 §5.7** —— 「未分類」這一類在 §5.5（20260915）就不存在了，而舉例的 `CheckARTSetupFile` 正是 `W906-ST-W4-C`（`WebStart.cpp:2569`），:317 早已列為 🔴。真正的缺口是**閘冊漂了 11 列**（漏列 5 ＋ 已解卻沒劃掉 6，其中 3 個標 🔴、1 個讓文件對安全狀態說反話），不是分類沒做 |

### ST-W7-A — `CheckSLKSensor()`

golden `main.cpp:32424-32500`（77 行），落點 `TfMainWeb::CheckSLKSensor()`。

**為什麼可以翻，而 `MainCalcCore` 不行**（這一條值得記著，是一類判斷不是個案）：
`MainCalcCore.h:113-118` 自己寫明它「STILL OUT OF SCOPE」，理由是它讀全域
`Cylinder[]` 硬體陣列（ht9045_sm substrate）。
**那個理由對 `MainCalcCore` 成立 —— 那個 TU 刻意保持 UI-free / globals-free ——
對 `WebStart.cpp` 不成立**：它是 `add_executable(wb_serve)` 的直接來源，
本來就連著整個 god-stack。
> **「某個 TU 不能收它」不等於「這棵樹不能翻它」。**
> 讀到 out-of-scope 的註記時要先問「對誰 out of scope」。

**四關**
| 關 | 結果 |
|---|---|
| `-fsyntax-only -Wall -Wextra` | rc=0，`WebStart.cpp` 自己 **0 個診斷** |
| `cmake --build build` | `BUILD_RC=0`，160 個 target |
| preprocessed 比對 | 不適用（只動自己的兩個檔） |
| `nm` | 見下 ★ |

★ **第四關證明接上的是真的機台碼，不是樁**：
```
T __ZN9TfMainWeb14CheckSLKSensorEv     <- 函式本體在 exe 裡
T __ZN11TMyCylinder8OnSensorEv         <- T = 實體程式碼
T __ZN11TMyCylinder8OnStatusEv
B _Cylinder                            <- B = 真的 BSS 陣列（mycylin.cpp:41）
R _C_SLK1_Clamp / _C_SLK1_Unclamp / _C_SLK2_Clamp / _C_SLK2_Unclamp
```
也就是引用 `Cylinder[]` 成功把 `mycylin.cpp.obj` 從 archive 抽進連結 ——
這正是「build 綠不等於接上了」那一關要看的東西。

**行為 delta = 0**：唯一呼叫點在 `StartFromWeb()`，而它全樹零個呼叫者。

⚠ **一個要留給後人的事實**：移植樹的 `ShowMyMessage`（`canary_support.cpp:143`）
是**觀測樁** —— 記 S1、計數、printf、轉給 hook，**不**彈 modal、**不**呼叫
`StopAllMotor()`。所以在這裡呼叫它不會停機。
但這代表：**未來若有人把它換成忠實版本，`CheckSLKSensor` 會連帶獲得「停所有馬達」
的副作用。** 那是那一次變更要評估的事，這裡先把它寫下來，不要讓它變成靜默的意外。

---

### ST-W7-B — `CheckInOutArmZHomeSensor()`

golden `main.cpp:32339-32376`（38 行），落點 `TfMainWeb::CheckInOutArmZHomeSensor()`。

**★★ 這一支跟 ST-W7-A 不同級：它會下運動指令。**

```cpp
MOT[MInArmX].PCIL132_StopMotor();      // golden :32353 / :32366
MOT[MInArmY].PCIL132_StopMotor();      // golden :32354 / :32367
```

移植樹 `Motor/mymotor.cpp:1278` 的本體會走到 **`Motor->DecStop()`（減速停止）**，
不是樁 —— `nm` 實測 `T __ZN8TMyMotor17PCIL132_StopMotorEv`。

#### 為什麼照翻而不是把 StopMotor 那兩行閘掉

golden 的語意是「偵測到 Z 軸原點感測器不對 → **先把手臂停下來** → 報訊息 → 回 false」。
閘掉 StopMotor 會變成「偵測到異常卻讓手臂繼續跑」—— 那是 **fail-dangerous，
比整段不執行更糟**。

本戰役的規矩是「閘掉『做不到的事』可以，替 golden『選一條路』不行」。
這裡兩個相依都在、都連得上，**沒有做不到的事**，所以沒有閘的理由。

#### ⚠ golden 自身的一個不對稱（照翻並註明，不要順手修）

**出料側**失敗時（golden :32366-32367），停的仍然是 **`MInArmX` / `MInArmY`**，
不是 OutArm 的馬達。golden 兩支分支停的是同一對。
看起來像筆誤，但那是 2016 年至今的出貨行為 —— 改它等於改機台行為，要使用者決定。

#### 四關

| 關 | 結果 |
|---|---|
| `-fsyntax-only -Wall -Wextra` | rc=0，`WebStart.cpp` 自己 **0 個診斷** |
| `cmake --build build` | `BUILD_RC=0`，160 個 target |
| preprocessed 比對 | 不適用（只動自己的兩個檔） |
| `nm` | `T __ZN9TfMainWeb24CheckInOutArmZHomeSensorEv`；`T __ZN8TMyMotor17PCIL132_StopMotorEv`；`B _MOT` / `B _InArmSuck` / `B _OutArmSuck`；`R _MInArmX` / `_MInArmY` / `_MInArmZA` / `_MOutArmZA` |

**行為 delta = 0**：唯一呼叫點在 `StartFromWeb()`，全樹零個呼叫者。

#### 一個要記住的標頭選擇

`InArmSuck` / `OutArmSuck` 取自 **`aHotPlateSubstrate.h:624/:627`**，
**不是** `mykitsuck.h:452` —— 陷阱 #1：兩個標頭的 `TMyKitSuck` 佈局不同，
選錯會**乾乾淨淨地連起來，然後每個欄位讀錯偏移**。
`WebStart.cpp` 當初就是為此選了前者，這一波沿用，沒有「順手改成看起來更合理的那個」。

#### 量測方法（下次照做）

相依不是一個一個 grep 出來的，是**寫一支編譯探針一次驗完**：
把 golden 用到的每一個符號各寫一行放進一個 `.cpp`，`-fsyntax-only` 跑一次。
rc=0 就代表「每一個都存在而且型別對得上」。
> **用編譯器判死活，不要自己寫掃描器。** grep 答得了「有沒有這個字」，
> 答不了「型別對不對、是不是同一個宣告」。

---

### ST-W7-C 原定標的 `cbRunStartMode`（W5-B）—— 量完確認做不下去

原本的計畫是「golden 讀 widget，但移植樹有 `LastSet.iRunStartMode` 這個 C++ 全域，
改讀它即可」。**量完之後這條路是死的。**

| 量測 | 結果 |
|---|---|
| golden 誰寫 `LastSet.iRunStartMode` | `main.cpp:559`（在 `SetRunStartMode()` 內）、`:569`、`:574`、`:12216`/`:12221`/`:12243`/`:12248`。**每一處都同時寫 widget 與全域**；只寫 widget 的路徑都緊接著呼叫 `cbRunStartModeChange()` → `SetRunStartMode(rsmNull, cbRunStartMode->Text)` 回寫全域。所以 golden 裡兩者是同步的 |
| 移植樹誰寫它 | **零個。** `SetRunStartMode(int)` 在 `aHotPlateSubstrate.cpp:1074` 是空的 `{}`；產品碼（排除 `tests/`）對 `LastSet.iRunStartMode` 的**寫入點一個都沒有** |
| 它會不會從 ini 載入 | **不會。** `ReadLastSetIni()`（`cprod.cpp:3106`）與 `SaveLastSetIni()`（`:3236`）都沒有這個鍵；**golden 的那兩支也沒有** |

⇒ ⚠⚠ **20260915 夜更正：這句話是錯的，而且錯的方向讓問題看起來比實際輕。**

原文寫「它在移植樹**永遠是靜態初始值**」。**實測推翻：**

```
wb_serve 實際 snapshot：  lastset.runStartMode  2
                          startmode.value       'Re-Test Continuous'   ← rsmContinuRetest == 2
```

**它有被載入。** 載入者不是 `ReadLastSetIni()`（那支確實沒有這個鍵），而是
`ReadLastDataFile()`（`cprod.cpp:1691`）——它做的是
`std::fread((char*)&LastSet.LastOpenFilename[0], sizeof(LAST_GENERAL_SET), 1, Fp3)`，
**整個結構的 raw blob 讀取**，來源是寫死的 `D:\HT9045\system\lastdata.dat`，
也就是**這台機台自己上一次存的狀態**。
`LoadMachineConfig()`（`database.cpp:3069`）在 `HSys.ReadGeneralIni()` 裡把它一起跑掉。

### 正確的敘述

> `LastSet.iRunStartMode` **開機時從 `lastdata.dat` 載入一次**，
> 之後**永遠不再更新**，因為 `SetRunStartMode(int)`（`aHotPlateSubstrate.cpp:1074`）是 `{}`。

### 為什麼這個更正讓問題**更嚴重**而不是更輕

| | 我原本以為 | 實際 |
|---|---|---|
| 值 | 恆為 0 | **凍結在機台上次存的值**（這台現在是 2） |
| 那些條件 | 永遠不成立 → 解閘等於沒解 | **會成立或不成立，取決於凍結的值** |
| 可預測性 | 全機台一致 | **每台不同**，而且會隨「上次關機時是什麼模式」而變 |
| 看起來像 | 明顯壞掉 | **看起來完全正常** ← 這才是危險的地方 |

**凍結在一個合理的值，比恆為 0 危險得多** —— 因為它讀起來像真的資料，
而它不會跟著操作員改模式走。

### 我怎麼會弄錯的（值得記住的形狀）

我量了**寫入者**（零個，正確），但**沒量載入者**。
`ReadLastSetIni` 沒有這個鍵是真的，我就停在那裡了 ——
沒想到同一條鏈上還有一個 **raw struct blob** 的讀取會一次填滿整個結構。

⚠ 而且稽核 agent 也給了同樣的錯誤結論並附「證據」
（「`LastSet.cpp:39` 是零初始化 → `iRunStartMode==0`」），
**我接受了它，因為它跟我自己的結論一致**。
那是確認偏誤：`agent 的論證比程式碼更常錯`這條規矩，
在 agent 同意我的時候我沒有照樣執行。


改讀它＝「補一個永遠讀同一個值的樁」，正是陷阱 #6 要避免的東西。

> ⚠ **順帶量到一個不是 ST 造成的既有缺陷**：移植樹有大量活的
> `LastSet.iRunStartMode == ...` 讀取點（`acarry.cpp`、`acatchtray.cpp` 等），
> **它們全部在讀一個沒有人維護的值**。這不是 ST 戰役搞出來的，
> 但它是一個真的問題，應該獨立立項。
> （注意：另有一個**裸的** `iRunStartMode` 全域被拿來當 `TrayForm.LoaderToEmptyColor[]`
> 的索引 —— 那是**不同的變數**，不要混為一談。）

**歸屬**：`cbRunStartMode` 是主畫面的下拉選單，代表「操作員選了哪個啟動模式」。
那個狀態在新架構下**在瀏覽器**。要讓 C++ 知道，需要 web → C++ 的**設定回報通道** ——
跟 §5.2 的 `fShow` 同一個家族，但它是「選了什麼」而不是「哪一頁開著」，
所以屬 **FW-W 指令通道**的地盤，不是本戰役能決定的。

---

### ST-W7-C — `CheckOLPError()`（改做這個）

原定標的確認做不下去後，重新掃了 `WebStart.cpp` 現存的 **50 個 gate**
（用腳本窮舉，不是憑印象），發現其中 **27 個**的阻因不在已知清單裡，
裡面至少四個是「翻一個小函式就能解」的。挑了最小的 🔴：

golden `main.cpp:33986-34012`（27 行），落點 `TfMainWeb::CheckOLPError()`。

#### ★ 這是**部分**翻譯，而且是刻意的

| 成分 | 相依 | 處置 |
|---|---|---|
| **判定**（有沒有任何 `OLPSetBinErr[i] >= 3`） | `ComputeCheckOLPErrorHasErr()`（`MainCalcCore.h:404`）✅ 已存在 | **翻** |
| **三個對話框**（i==0/1/2 各一個） | `ShowMyMessagePWD()`（golden `mymessbox.h:52`）❌ 全樹未移植 | **閘**（`W906-ST-W7-C-DLG` 🟡） |

照本戰役的規矩「能部分閘就不要整段閘」。
⚠ **不用「改叫 `ShowMyMessage`」代替** —— 那支是觀測樁且沒有密碼確認，
換一支不同語意的函式是替 golden 選路，不是翻譯。

#### ⚠⚠ 解閘後的實際行為，要講清楚

OLP Bin 設定有錯時 → **機台拒絕啟動，但畫面沒有任何訊息**。

仍然選擇解閘，理由是本戰役自己的判準：

| | 閘著 | 解閘 |
|---|---|---|
| 行為 | 設定錯了**照樣跑** | **不跑** |
| 分類 | **fail-open**（會出壞品） | **fail-safe**（但沒說為什麼） |

ST-W5 的稽核已經因為 fail-open 修正過一次分類（W5-B），這裡用同一個標準。

⛔ **S3 之前必須補 `ShowMyMessagePWD`**，否則現場會遇到
「按 START 沒反應、也沒有任何提示」。已列入 §6。

---

### ST-W7-D — `CheckSmartAutoCleanCanStart()`

golden `AutoClean/uCleaning.cpp:2879-2915`（37 行，Sam 20250916
「Alarm 後需要清除資料才能 Start」）→ `W906ST_CheckSmartAutoCleanCanStart()`，W2-C 解閘。

**落點是自由函式，不是 `TfCleaning` 的方法。** 三個理由，順序是量測在前：

1. 移植樹的 `forms/fCleaning.h` **沒有宣告過它**（實測）——
   這其實是好消息：**不會有「呼叫 facade 靜默拿到樁值」的陷阱**。
2. 把 `fCleaning` 重指到 impl 屬於 **S3**，硬停止線明文禁止。
3. ★ **最重要**：golden 用到的每一個狀態在移植樹**全部是全域**，不是 `TfCleaning`
   的成員（編譯探針一次驗完，rc=0 零診斷）——
   `IniConfig.bEnableAutoCleanFunction` / `TestIF.*` / `iACSmartCount(_CTF)` /
   `sACRecAlarmCode` / `sACRecEPortCode` / `K_RETRY` / `MMInterface`。
   所以「成員 vs 自由函式」**不改變讀到的東西**。

自由函式是本樹既有慣例（`MainCalcCore` 就是把 TfMain 方法抽成自由函式）。

照翻的三個細節（都容易被「順手改好」）：
- **`TestIF` 不是 `TestIF_File`** —— 本檔別處大量用後者，golden 這支讀前者。
- golden 這個擋啟動點**沒有** `iStartIn = 0;`（跟其他擋啟動點不同），照翻不補。
- golden `:2909` 被註解掉的 `ShowMyMessage("Please Reset AI AutoClean","")`
  原樣保留為註解，不啟用。

四關：syntax rc=0 / 0 診斷；build rc=0 / 160 target；
`nm` = `t __ZL34W906ST_CheckSmartAutoCleanCanStartv`（static 故小寫 t）＋
`B _TestIF` / `B _iACSmartCount` / `B _iACSmartCount_CTF` / `B _sACRecAlarmCode`。

---

### ST-W7-E — `ATCAmbientTemperCheck()`（W4-A）：查完是死路，而原本的死因寫錯了

原 gate 註解寫「該函式在移植樹不存在」。**實測結果相反：**

| | 狀態 |
|---|---|
| 純計算 `ComputeATCAmbientTemperCheck(double,int,int)` | ✅ **早就存在**（`MainCalcCore.h:318`，忠實保留 `==0` sentinel 與 `-5` 的二段判斷） |
| `IniConfig.dATCAmbientTemperature` / `ATC_SYSTEM` | ✅ 拿得到 |
| **`iATC_MODE_TYPE`** | ❌ **沒有來源**。`ATC/ATC_Handler_Side.h` 不存在（實測 include 失敗）；唯一替身 `acarry_shims.h:112` **只在 ctor 設成 0，全樹再無寫入**（`acarry_shims.cpp:72`） |

#### ★ 為什麼不拿那個 shim 來湊 —— 這條界線要記住

`iATC_MODE_TYPE` 恆為 0 ⇒ `(mode==33||35||61)` 恆假 ⇒ 永遠走 `else` ⇒
環溫一超出 25~30 就一律擋。對「新 ATC 系統 + mode 33/35/61 且環溫在 -5~25」的機台，
**golden 會放行而我們會擋** —— 那是替 golden 選路。

> **這跟 W1-E（ST-W7-C）的部分解閘不同級：**
> W1-E 是**忠實算出判定、只省略輸出**（三個對話框）。
> 這裡若照做，是**用捏造的輸入去算判定**。
> **省略輸出 ≠ 捏造輸入。**

**歸屬（陷阱 #6 的第三類）**：這個狀態**不在 web、也不在 C++** ——
它是 ATC 控制器回報的模式，整條 ATC 介面尚未移植。
所以它**不屬 §5.2 的 fShow 家族**，是獨立的一大塊。

---

### ST-W7-F — `CheckEmployeeID()`（W1-G）：17 行，但是最硬的一類

**★ 這一條的價值是它推翻了我自己的一個判斷。**

ST-W7-C 那次窮舉 gate 時，我把 `CheckEmployeeID`（17 行）列進
「翻一個小函式就能解」的候選。**那是錯的：行數不是難度。**

golden `mymessbox.cpp:1353-1369` 的流程：

```
把提示字寫進 fPassword 的兩個 Label
  -> fPassword->ShowModal()          ← 阻塞，等操作員輸入帳密
  -> 讀 edUserName / edPassword
  -> EventReport(SECS_EVENT.BarcodeReaderEnter)   ← 把憑證回報給 host
```

移植樹 `forms/fPassword.h:337` 是 **`void ShowModal() {}`**（離線 no-op），
同檔 `:164-172` 自己寫明理由：

> golden's modal loop does not exist headless … **the web layer owns show/dismiss**

#### ⇒ 照翻比閘掉糟得多

`ShowModal()` 立刻返回 → 讀到**空的**帳號密碼 →
**對 host 送出一個帶空白憑證的 SECS 事件**。
那等於向 host **謊報「操作員已通過身分確認」**。

> **這不是「少做一件事」，是「做錯一件對外的事」。**
> 前者閘著就好；後者連翻都不能翻。

相依現況（編譯探針實測）：`fPassword` / `edUserName` / `edPassword` 都在；
缺 `Label6` / `EventReport` / `SECS_EVENT`。
**但就算把那三個補齊也不該解閘** —— 真正的阻塞點是 `ShowModal()` 的往返語意。

**歸屬**：跟 §5.2 同一個設計題，而且是其中最難的「往返」那一種
（見 `docs/FSHOW_WEB_VISIBILITY_DESIGN.md` §6）。屬 FW-W 指令通道，不是本戰役。

> 給下一波的一般化教訓：**挑 ST-W7 候選時，先看它會不會 `ShowModal()` 或
> 對外送訊息，再看行數。** 一個 17 行但會對 host 發言的函式，
> 比一個 77 行純讀感測器的函式難得多。

---

## 5.4 ★★ `LastSet.iRunStartMode` 是死的，而且它已經在讓活的碼靜默判錯

> 20260915 ST-W7-E/F 查訪時量到。**這不是 ST 戰役造成的，但 ST 是第一個撞到它的。**
> 它比任何單一 gate 都重要，所以獨立成節。

### 事實（三個量測）

| 量測 | 結果 |
|---|---|
| golden 誰寫 `LastSet.iRunStartMode` | 7 處，**每處都同時寫 widget 與全域**；只寫 widget 的路徑都會回呼 `cbRunStartModeChange()` → `SetRunStartMode(rsmNull, cbRunStartMode->Text)` 補上 |
| 移植樹誰寫 | **零個。** `SetRunStartMode(int)` 在 `aHotPlateSubstrate.cpp:1074` 是空的 `{}`；產品碼（排除 `tests/`）零寫入點 |
| 會不會從 ini 載入 | **不會。** `ReadLastSetIni()`（`cprod.cpp:3106`）／`SaveLastSetIni()`（`:3236`）都沒這個鍵 —— **golden 那兩支也沒有** |

⇒ ⚠ **這一句已於 20260915 夜更正** —— 它**不是**恆為靜態初始值，
而是**開機從 `lastdata.dat` 載入一次後凍結**。實測 `lastset.runStartMode = 2`。
完整更正見 §5.3 同一段（本節與那裡講的是同一件事）。

### ★ 影響面（量出來的，不是估計）

| 位置 | 數量 |
|---|---|
| `WebStart.cpp` 被閘的碼裡讀它 | **6 個 gate**（W1-H / W1-I / W4-C / W4-F / W4-H / W6-I） |
| **`WebStart.cpp` 已翻譯、沒被閘的活碼裡讀它** | **6 處** —— `:1805-1807`（golden :5443-5445）、`:1872-1874`（golden :5503-5505） |
| 移植樹其他地方的活讀取點 | 大量（`acarry.cpp`、`acatchtray.cpp` 等，未逐一計數） |

**那 6 處活碼是重點**：它們是 ART 模式的判斷
（`rsmInitial_ART` / `rsmContinuStart_ART` / `rsmContinuRetest_ART` / `rsmAutoRetest`）。
今天 `StartFromWeb()` 沒有呼叫者所以不執行；**S3 一旦武裝，它們會永遠取同一個分支**。

⚠ 這也解釋了為什麼 **W4-C（`CheckARTSetupFile`）看起來可做、實際不可做**：
`ComputeCheckARTSetupFile()` 在 `MainCalcCore.h:407` **已經存在**，
但 W4-C 的**外層守衛**讀的就是 `LastSet.iRunStartMode`。
解閘會得到一段**永遠不會執行的檢查** —— 記憶裡那條「不可能當掉的 gate 不是 gate」。

### 看起來很近的修法，以及為什麼**不做**

`SetRunStartMode(int)` 在移植樹**有 34 個活呼叫點**，另有 58 個在 `#if 0` 裡。

> ⚠ AI(W906-RSM-AUDIT) 20260916 更正：原文寫「**約 10 個**」並列了 9 個位置。
> **實測是 34。** 量法是 `g++ -E`（真前處理器，不是自己寫的掃描器）對 11 個含呼叫的
> `.cpp` 逐檔展開後數 `SetRunStartMode(`，扣掉標頭帶進來的宣告：
> `csystem.cpp` 10、`SECSGEM/uHGemHT9045.cpp` 9、`Command.cpp` 5、`HANA_ART.cpp` 2、
> `ainarm_SearchPlacePlate.cpp` 2、`AutoRetest.cpp` / `SCK_ART_Remainder.cpp` /
> `auto9045.cpp` / `uRENESAS_Server.cpp` / `ainarm2.cpp` / `cinitial.cpp` 各 1。
> 原文的清單本身也有兩處不對：`ainarm2.cpp` 的活呼叫在 **`:7060`** 不是 `:7050`，
> 而 `csystem.cpp` 與 `SECSGEM/uHGemHT9045.cpp`（兩個最大宗，合計 19 個）**整個漏列**。
>
> 過程中我先用自己寫的 `#if 0`-aware 掃描器量，得到 33、再修一次得到 30，**兩次都錯**
> ——一次是 `//` 註解裡的 `/*` 開了假的區塊註解吞掉整個檔尾，一次是 `'"'` 字元常值讓
> 字串狀態卡住。這是記憶裡「**用編譯器判死活，不要自己寫掃描器**」那一條的又一次實證。

把它的本體從 `{}` 改成 golden `main.cpp:559` 的 `LastSet.iRunStartMode = iMode;`
**是一行**，而且忠實。

⛔ **但本戰役不做，理由是爆炸半徑不是一行：**
那 34 個呼叫點今天全是 no-op；一旦生效，會同時喚醒**全樹 724 個**
`LastSet.iRunStartMode == ...` 的讀取點 —— 料盤路由、複測流程、ART 判斷全在裡面。
記憶裡的「**武裝沒編過的碼會弄壞別條線**」講的正是這個形狀。

**處置：獨立立項、單獨一顆 commit、跑完整 Debug+Release gate、且要使用者在場。**
不可以夾在任何一個 ST 波次裡順手做。

### 20260916 唯讀稽核（使用者裁決「先量再談修」的產物）

**724 這個數字的量法**：`g++ -E`（真前處理器）對 **73 個非測試 `.cpp`** 逐檔展開，
數展開後的 `.iRunStartMode` / `->iRunStartMode`，**0 個前處理失敗**。
獨立進行的稽核 agent 用完全不同的方法（`#if 0`-aware 掃描 1,253 個 tracked 檔）
也得到 724。兩把不同的尺同值。

**⚠ 名稱衝突 —— 這是本稽核最容易害人的一點。**
樹上有**第二個、完全無關**的全域也叫 `iRunStartMode`：
`cmydef.h:3112` 宣告、`cmydef.cpp:3340` 定義 `int iRunStartMode=0;`。
它比對的是 `FT`（`const int FT=1;`, `cprod.cpp:84`），不是任何 `rsm*` 常數，
**`SetRunStartMode()` 不會寫它**，而且它也沒有任何非測試寫入者 → 永遠是 0，
所以它的活讀取（`cprod.cpp:3512-3788` 等）裡的 `iRunStartMode==FT` **恆為 false**。
（稽核 agent 量到 133 個活讀取；**這個數字我沒有獨立複驗** —— 用 `g++ -E` 數會被
每個 TU 各算一次的標頭宣告灌水，要另外扣，我沒做。`.iRunStartMode` 那個 724
不受這個問題影響，因為宣告寫的是 `int iRunStartMode;`，沒有點號。）
一句 `git grep iRunStartMode` 會回一千多筆並把兩個混在一起。
**有人聽到「那個全域現在會寫了」，很可能會誤以為 `cprod.cpp` 那一片也活了。它沒有。**

**三件必須先做的事（順序不可換）：**

1. **`auto9045.cpp:1506` 要先擋住或修好。** 它的前置守衛是
   `CheckCanChangeRealDummy()`，而移植樹的替身（`auto9045.cpp:208-217`）是
   **硬寫的 `return true;`**，註解自己標著「JUDGMENT CALL (flagged for review)」。
   一旦 stub 真的會寫，外部 OLP/TCP 客戶端就能**在跑批途中**把 FT↔RT 翻掉，中間沒有互鎖。
2. **ECID 1517 要一起裁決。** `SECSGEM/uHGemHT9045_EC.cpp:471` 把
   `&LastSet.iRunStartMode` 的**裸指標**交給 SECS host（"Start Mode For HT9045"）。
   今天還沒有 S2F15 的寫回消費者，但一旦有，host 會直接寫這個欄位，
   而 golden `main.cpp:363-1115` 的本體一行都不會跑 → **兩個寫入者、兩套語意**。
3. **「一行」其實是 753 行。** golden 的寫入者是 `main.cpp:363-1115`。
   只寫 `LastSet.iRunStartMode = iMode;` 會漏掉 `QABackupStatus()`、`ModifyTester()`、
   MES2155/2157 記錄、`bMustCleanAllTray`，以及最危險的
   **`main.cpp:563-578`** —— 當 `TestIF_File.iTestMode==SingleSite` 時把
   `rsmAutoSiteMap` 改寫成 `rsmContinuStart`/`rsmContinuRetest`。漏掉它，
   單站機台會掉進 AutoSiteMap 那一大群讀取點裡，而 golden 是刻意把它們擋在外面的。
   golden `:557` 還有 `if(Mode!=rsmNull)` 的守衛，裸賦值會存進 −1。

**另外兩點**：
- **凍結值是機台狀態，不是程式常數** —— 這台機器上 `wb_serve` 讀到 **2**，
  而任何跳過 `LoadMachineConfig()` 的 harness 讀到 **0**。
  **兩個 harness 在 64 個讀取點上走相反的分支。**「今天的行為」有兩個答案。
- `rsmFIFOMode`(12) 有 72 個活讀取點但**沒有任何活寫入者**
  （外部命令在 `auto9045.cpp:1503` 被限制在 0..3）。修完之後的普查會看起來比實情健康。

---

## 5.5 ★ 48 個 gate 的完整分類（20260915，ST-W7 的單一決策面板）

> 這張表是 ST-W7「還能不能自動做下去」的唯一依據。
> 做法：5 個唯讀 agent 各分約 9 個 gate，**每一個 FEASIBLE/BIG 宣稱再派對抗式複驗**
> （預設 refuted=true）。**9 個宣稱裡 7 個被推翻。**
> 主迴圈對實際動手的那一個另外自己重跑探針才動工。

### 分類分佈

| 類 | 數 | 意思 | 能不能自動做 |
|---|---|---|---|
| **MISSING-OBJECT** | 12 | 缺整個表單／介面物件（連 facade 都沒有） | ❌ 要新建物件，屬別的戰役 |
| **WEB-HOLDS** | 10 | UI 狀態在瀏覽器（`fShow` 那一族） | ❌ 等 §5.2 的設計裁決 |
| **OUTBOUND-MODAL** | 7 | 會 `ShowModal()` 或對外送訊息 | ❌ 照翻會**做錯一件對外的事** |
| **DEAD-GLOBAL** | 4 | 守衛讀 `LastSet.iRunStartMode` | ⚠ **分類名稱有誤導性**（20260915 夜更正）：那個全域**不是死的，是凍結的** ——開機從 `lastdata.dat` 載入一次後不再更新。所以解閘**會**執行，只是永遠依凍結值走同一支。見 §5.4 |
| **BIG** | 4 | 翻得動但很大 | 🟡 **各自要一波**，不是還債 |
| **NOT-THIS-MACHINE** | 2 | CC_PANTHER 專屬 | 🟡 現況 delta=0 |
| **FEASIBLE** | 5 → **複驗後 2** | 相依齊全 | ✅ |

### BIG 四個的真實大小（**這欄推翻了我原本的估計**）

| gate | 缺的本體 | golden 行數 |
|---|---|---|
| W1-I | `GenerateCheckList()`（`uLotInfo.cpp:12424-13277`） | **854** |
| W6-G | `TransformFuntion()`（`adam6024.cpp:1038-1796`） | **759** |
| W5-H | `TfMesSystem::CheckLotInfor()`（`Mes/fVATMesFileSys.cpp:1037-1638`） | **602** |
| W4-E | `NETDownloadDataCheck()`（`main.cpp:30425-30792`） | **368** |

⚠ W1-I 的 gate 註解原本只寫「`uLotInfo.cpp:12413`（10 行）尚未移植」——
**低估了一個數量級**。10 行是入口，背後是 854 行、帶 60 組 `ReadIniData`/`WriteIniData`
且會落檔的 `GenerateCheckList`。

### ★ 這次稽核抓到**我自己**寫錯的 gate 註解（四筆）

| gate | 我原本寫的 | 實際 |
|---|---|---|
| W1-A | 「`patFunc` 全樹不存在」 | `class PAT_Function` 整支 2413 行**都在**（`ProductionInfo/uPAT_Function.cpp`），缺的只是**實例指標** |
| W1-D | 「`forms/fTrayMapping.h` 的 facade 沒有 `fShow`」 | **指錯檔案**。`forms/fTrayMapping.h` 裡是**另一個同名 class** `TfTrayMappingForm`（它有 `fShow`）；全域 `fTrayMapping` 綁的是 `acatchtray_shims.h:219` 那個 minimal class |
| W1-I | 「10 行尚未移植」 | 背後 **854 行** |
| W5-D | 「省電計時器物件全樹不存在」 | `class TPowerSaving` 與 `Restart()` **都在**，缺的是**被 new 出來的實例** |

> 共同形狀：**我把「這個名字在我搜尋的地方沒出現」寫成「它不存在」。**
> 兩者差很遠，而且第二種說法會讓後人放棄一條其實走得通的路。
> （這也正是 §「遞迴 grep 會回假的查無」那條的實際代價。）

### 複驗殺掉的 7 個（**不要拿去行動**）

| gate | 宣稱 | 更正後 |
|---|---|---|
| W2-A | FEASIBLE | **WEB-HOLDS** |
| W3-A | FEASIBLE | **DEAD-GLOBAL** |
| W5-G | FEASIBLE | **OUTBOUND-MODAL** |
| W1-I | BIG | **WEB-HOLDS**（更擋路） |
| W4-E | BIG | **DEAD-GLOBAL** |
| W5-H | BIG | **NOT-THIS-MACHINE** |
| W6-G | BIG | **MISSING-OBJECT** |

### 存活的 2 個 FEASIBLE

| gate | 內容 | 狀態 |
|---|---|---|
| **W6-H** | `SET_ESD_Tri_Temp()`（golden `main.cpp:34528-34550`，23 行） | **✅ ST-W7-G 做掉了** |
| **W1-C** | `Read2DIDList()` + `listError2DID`（golden `BarCode/BarCode.cpp:8209-8246`，38 行） | **✅ ST-W7-H 做掉了**（`WebStart.cpp:696` 本體 / `:1239-1244` 解閘）。⚠ **20260923 對帳更正**：本欄原寫 ⬜「最後一個可自動做的」，與 §5.6 的交付表直接矛盾 —— **交付表是對的**，碼上 `W1-C` 的 GATE 標記已不存在 |

⚠ W1-C 有一個要一起看的事實：它的**兩個消費者都還沒移植**
（`fBarCode->b2DIDIsInsideList` / `b2DIDIsInsideToErrorBin` 全樹零定義，
唯四的呼叫點都在 golden-verbatim `#if 0` 裡）。
解閘＝**載入兩份沒有人讀的清單**。仍然值得做（gate 誠實還掉），
但要在註解裡寫明它今天不會改變任何行為。

---

### ⇒ ST-W7 的終點在哪，現在是可量的

做完 W1-C 之後，**剩下 43 個 gate 全部需要使用者裁決或屬於別的戰役**：

- 10 個等 §5.2 的 `fShow` 設計裁決（提案已寫：`docs/FSHOW_WEB_VISIBILITY_DESIGN.md`）
- 4 個等 §5.4 的 `SetRunStartMode` 裁決
- 12 個要新建表單／介面物件
- 7 個照翻會做錯對外的事
- 4 個各自要一波（368~854 行）
- 2 個不是這台機台

**那時候才可以寫 `kWave7State = 2`，而且要逐條列出上面這五類在等什麼。**

---

## 5.6 ⛔ ST-W7 自動部分結束：`kWave7State = 2`（**卡住，不是完成**）

> 20260915 夜間。`/st-wave` 依步驟 0.5 交棒給 `/fw-wave`。
> **2 不是 3。§5.1 的還債清單沒有清空。**

### ST-W7 實際交付了 6 個

| # | gate | 內容 |
|---|---|---|
| A | W5-A 🔴 | `CheckSLKSensor()`（golden 77 行） |
| B | W2-G 🔴 | `CheckInOutArmZHomeSensor()`（38 行，**含運動指令**） |
| C | W1-E 🔴 | `CheckOLPError()`（27 行，**部分**：判定翻、對話框閘） |
| D | W2-C 🔴 | `CheckSmartAutoCleanCanStart()`（37 行） |
| G | W6-H 🟡 | `SET_ESD_Tri_Temp()`（23 行） |
| H | W1-C 🟡 | `Read2DIDList()`（38 行，**動到既有檔**，preprocessed 比對通過） |

### 剩下 43 個 gate 在等什麼（逐類，這是交棒回報要講的內容）

| 等什麼 | 數 | 誰能解 | 已備好的東西 |
|---|---|---|---|
| **`fShow` 設計裁決** | 10 | **使用者** | `docs/FSHOW_WEB_VISIBILITY_DESIGN.md` —— 三個方案、租約設計、**最後一節是要回答的三題** |
| **`SetRunStartMode` 裁決** | 4 | **使用者** | §5.4。**不是一行改動**——34 個活呼叫點、724 個讀取點，且有三個必須先做的前置（見 §5.4 的 20260916 稽核） |
| 缺整個表單／介面物件 | 12 | 別的戰役 | `fPMAlarmInterFace` / `fFTPClient` / `TrayEditForm` / `fARMS` / `CCDInterface` … |
| 照翻會**做錯一件對外的事** | 7 | 別的戰役 | `ShowModal` 收不到輸入、或對 MES/視覺/SECS 送錯內容 |
| 各自要一波（368~854 行） | 4 | 別的戰役 | `GenerateCheckList` 854 / `TransformFuntion` 759 / `CheckLotInfor` 602 / `NETDownloadDataCheck` 368 |
| 不是這台機台 | 2 | — | CC_PANTHER 專屬，現況 delta = 0 |

### ⚠⚠ 這個結論的已知弱點（不要當成鐵板）

分類是 5 個唯讀 agent 做的，**對抗式複驗只驗了「說可做」那個方向**
（9 個 FEASIBLE/BIG 宣稱被推翻 7 個）。

> **「說做不動」那個方向沒有被驗。**
> 也就是說，43 個裡可能有被**誤判為卡住、其實做得動**的 —— 那是**假陰性**。
> 記憶裡那條「假陽性會被人判掉、假陰性沒人會來找」講的正是這個：
> 我們會發現「翻了結果翻不動」，但不會發現「其實可以翻卻沒翻」。

而且這次稽核本身就抓到**我自己**四筆把「搜尋不到」寫成「不存在」的 gate 註解
（W1-A / W1-D / W1-I / W5-D，見 §5.5）—— 同一種錯很可能還藏在那 43 個裡。

**處置建議**：下一次要重啟 ST 戰役時，**先派一輪反方向的稽核**
（「找出被誤判為卡住、其實相依齊全的 gate」），再決定要不要繼續。
不要直接相信這張表的「卡住」欄。

---

## 5.2 ★ `fShow` 不是缺一個欄位，是一個架構問題

> **使用者 20260915 指出**：`fContact->fShow` 這一類後續會改成**網頁的顯示狀態**。
> 我在寫 ST-W1..W6 的時候**沒有把這件事接起來**，所有 gate 註解都寫成
> 「facade 沒有這個成員」，ST-W7 也寫成「補 `fShuttleMove->fShow`」。
> **那個框法是錯的，而且錯得會生出十個靜默缺陷。**

### ⚠⚠ 20260916 用 `g++ -E` 稽核自己的交付時發現：**不是只有被閘的那 12 個**

前面整節都寫得像「`fShow` 的問題全部關在 12 個 gate 裡」。**不對。**

把 `WebStart.cpp` 預處理後、只取歸屬於它自己的 1,478 行來看，
裡面有 **5 處活的 `->fShow` 讀取**（不在任何 `#if 0` 裡）：

```
if (fContact->fShow == false && fTemp_Set->fShow == false)
if (fContact->fShow == true && ...
    fContact->fShow == false)
    fContact->fShow == false)
    fContact->fShow == true && ...
```

原因很單純：**facade 有沒有那個成員決定了它被閘還是被翻。**

| facade | 有 `fShow` 嗎 | 結果 |
|---|---|---|
| `fContact` | ✅ `forms/fContact.h:1482` | **已翻譯成活碼** |
| `fTemp_Set` | ✅ `forms/fTemp_Set.h:505` | **已翻譯成活碼** |
| `fShuttleMove` / `Zteach` / `fTrayMapping` / `TrayEditForm` | ❌ | 被閘（就是那 12 個） |

#### 那兩個活的讀取今天讀到什麼

| | 初值 | 誰寫 |
|---|---|---|
| `fContact->fShow` | `fShow(false)` 在 ctor 初始化列表（`forms/fContact.cpp`） | **移植樹沒有人設成 true** |
| `fTemp_Set->fShow` | `= false`（`fTemp_Set.h:505`） | `uTemp_Set.cpp:1153` 設 true、`:4436` 設 false —— **但那是 FormShow/FormClose，離線沒人呼叫** |

⇒ 兩者今天都恆為 `false`。**而那是對的** —— 離線時那些 VCL 畫面**真的沒有開**。
本樹別處也是同一個慣例並寫在註解裡（`Command.cpp:242`
「real facade, offline fShow=false」、`atester_shims.h:151` 同義）。

⚠ **（查證紀錄）** `forms/fContact.h:1482` 寫的是 `bool fShow;`（無初始值），
與 `fTemp_Set.h:505` 的 `bool fShow = false;` 不同，我一度懷疑是未初始化讀取。
**查了 ctor，`fShow(false)` 在初始化列表裡，沒有這個缺陷。**

#### 這件事把設計題的爆炸半徑放大了

§5.2 原本的框法是「**決定了才能解那 12 個 gate**」。實際上：

> 一旦 web 層開始回報「這一頁開著」，**那 5 處已經在跑的判斷也會跟著改變行為** ——
> 它們今天穩定地走「畫面沒開」那一支，之後會開始走另一支。

所以這不只是「解閘」，是**同時改變已翻譯活碼的行為**。
裁決時要把這 5 處一起看，不能只看 gate 清單。

---

### 使用者 20260915 補充（把設計題收斂成兩條）

> 1. `if (fContact->fShow == false)` → 意思是 **web contact 畫面是不是有開啟**
> 2. `fContact->Show()` / `fContact->ShowModal()` → 是**顯示 web contact 畫面**

這兩條把 §5.2 從「未決設計題」變成「已知設計、但在 ST 迴圈的硬停止線外」。
`fShow` 這件事有**兩個方向**，而且不對稱：

| 方向 | golden 寫法 | 新架構的意思 | 性質 |
|---|---|---|---|
| **讀** | `fXxx->fShow` | 瀏覽器 → C++：那一頁現在開著嗎 | 狀態查詢 |
| **寫** | `fXxx->Show()` | C++ → 瀏覽器：把那一頁叫出來 | 指令 |
| **寫（阻塞）** | `fXxx->ShowModal()` | C++ → 瀏覽器：叫出來**並等使用者關掉** | **往返** |

⚠ **`ShowModal()` 是最硬的一條。** golden 在它的下一行就假設「對話框已經被關掉了，
而且我知道使用者按了什麼」（回傳值 `mrOk` / `mrCancel`）。
在網頁那不是一個函式呼叫，是一次 request/response 往返 —— C++ 這一側要嘛阻塞等待，
要嘛把後續程式碼拆成 callback。**兩種都是控制流改寫，不是翻譯。**

**結論不變，理由變強**：這 10 個 gate 仍然維持閘住。
不是因為「不知道該怎麼做」，是因為做它需要在 `wb_serve.cpp` 開一條雙向的
頁面可見性通道 —— 那是 **S1/S3 的地盤**，而 ST 迴圈的硬停止線第 4 條就寫著
「撞到需要改 `tools/wb_serve.cpp` → 停」。

---

### 錯在哪

`fShow` 在 golden 是 VCL 的「這個表單現在顯示著嗎」。在新架構下
（UI = web、底層 = C++，20260812 定案）**那個狀態不在 C++ 這一側** ——
它在瀏覽器：現在開的是哪一頁。

所以「補一個 `bool fShow` 進 facade」會得到一個**永遠讀到 false 的樁**。
`forms/fShuttleMove.h:35-40` 自己就警告過這件事：
「Adding it would be inventing an offline default (i.e. a silent branch selection)」。

而 golden 用 `fShow` 的方式全部是**例外條件** ——
「這個畫面開著的時候不要卡權限 / 不做 run check / 不檢查流道」。
樁回 false 等於「那些畫面永遠沒開」，也就是**所有例外都不成立**，
每一個都是靜默的行為偏離。

### 規模

實測（20260915 重量，用腳本掃 `WebStart.cpp` 的每個 gate 區塊）是 **12 個**：

```
W1-D  W1-I  W2-B  W2-J  W3-A  W4-H  W5-F  W6-A  W6-D  W6-E  W6-F  W6-G
```

> ### ⚠ 20260917 逐閘實測：**它們不是「只差一個 fShow」**
>
> ST 批次 1 補齊了 `TfShuttleMove::fShow`、`TfTrayMapping::fShow` 與
> `TTrayEditForm_Facade`，然後**逐個用編譯器問**還差什麼。結果：
>
> | 閘 | 補完 fShow 之後還差什麼 |
> |---|---|
> | **W2-B** | 無 ⇒ **已解**（唯一一個）|
> | W6-D / W6-E | **`RunCheckStart()`** —— 全樹不存在。`csystem.cpp:9755` 的 gate 註解自己寫著「no member of either name exists on the FormsFacade TfMain」。golden 是 `main.cpp:32170-32209`，約 40 行 |
> | W5-F ×2 | 成員齊了，但 `fShow==false` 在這裡是**打開保護**那一支 ⇒ 解開會**新增一個擋啟動**（ASE 高雄／OP／OFF_LINE）。且 `WebStart.cpp:2455` 註解記著下一支原本是 `else if`、被閘掉才變裸 `if` ⇒ 還要做鏈結構還原。**不是零行為改變** |
> | W6-A | `fTeach`（全域刻意留 NULL，SIOF 迴避，見 `forms/fTeach.cpp:67`）|
> | W2-J | `fAutoTeach` 全樹不存在 |
> | W1-D | 6 個成員，含 `PageControl1` / `TabSheet2` |
>
> **所以「補兩個 bool 就解得開大部分」是錯的估計**（我在
> `docs/ST_FSHOW_SIMPLE_PROPOSAL.md` 第一版這樣寫）。真實比例是 **1 / 12**。
> 教訓與上面那條同形：**要用編譯器問，不要用推的。**

> ⚠ **本節原本寫「10 個」並漏了 `W3-A` 與 `W4-H`。** 那是憑印象列的清單，
> 不是量出來的。20260915 寫設計文件時用腳本重掃才發現。
> 同一次還發現我在設計文件第一版把「涉及的表單」寫成 2 個 —— **實際是 8 個**。
> 教訓照舊：**清單要用腳本產，不要用記憶列。**

**涉及 8 個表單**，而且其中 **4 個在 web 端根本還沒有頁面**：

| golden 表單 | 次數 | web/page/ 有對應頁？ |
|---|---|---|
| `fContact` | 12 | ✅ `Setup.Contact.html` |
| **`fShuttleMove`** | **12** | ❌ 沒有（而它在 W6-F ——**啟動觸發**——裡） |
| `Zteach` | 4 | ❌ 沒有 |
| `fTemp_Set` | 3 | ✅ `Setup.Temp_Set.html` |
| `fOffSet` | 2 | ✅ `Setup.OffSet.html` |
| `fTrayMapping` | 1 | ❌ 沒有 |
| `TrayEditForm` | 1 | ❌ 沒有 |
| `fTeach` | 1 | ✅ `HW.teach.html` |

★ **「有 4 個沒有網頁」這件事把問題變簡單了**：畫面不存在，操作員就不可能把它
開著，所以「沒開」**不是發明出來的離線預設值，它就是事實**。
但**不可以寫死 `return false`** —— 那會在有人把 `fShuttleMove` 做成網頁的那天
繼續說謊。正解是讓答案來自「有沒有客戶端回報這一頁開著」：沒有那一頁就沒有人
回報，自然是 false，而且那一頁上線當天**自動變對**。

**完整設計提案（含三個方案、租約、要使用者回答的三題）：
`docs/FSHOW_WEB_VISIBILITY_DESIGN.md`（20260915 寫，等裁決）。**

也就是 `fShow` 這一件事**單獨佔了 ST-W7 還債清單的一大半**。

### 正確的問法

不是「怎麼補這個成員」，是：

> **web 層要用什麼機制告訴 C++「操作員現在開著哪一頁」？**

這是設計題，不是翻譯題。可能的形狀（都要使用者裁決，本戰役不自行決定）：
- 瀏覽器在頁面載入/卸載時送一個 `ui.page.enter` / `ui.page.leave` 指令，
  wb_serve 維護一份「當前開啟頁面」集合 —— 但那是**指令通道**，屬於 S1/S3 的地盤
- 或者由 tag 反向查詢
- 或者宣告「web 架構下這些例外條件不再適用」，逐條重新決定 golden 的意圖

⚠ **在這個問題有答案之前，這 10 個 gate 不要動。**
補樁比閘著更糟：閘著是「這段不執行」，補樁是「這段執行了但判斷永遠錯」。

### 這個錯誤的形狀（給下一波記著）

我把「符號不存在」當成終點，沒問「它在新架構裡對應的是什麼」。
**編譯器只告訴你前者。** 以後 facade 缺成員時，先問：

> 這個成員在 golden 裡代表什麼**狀態**？那個狀態在 web 架構下由誰持有？

三種答案對應三種處置：C++ 仍持有 → 補；web 持有 → 這一節；沒有人持有 → 重新設計。

---

#### golden 自身的一個缺陷（照翻並註明）

`golden :5509-5521`（SPIL 的 ART Bin setting 保護）：

```cpp
int Pos = Prod.iT6PosCate[i];          // :5509
...
if (Pos != Prod.iT6PosCate[i])         // :5514  ← 恆為 false
```

`Pos` 就是上面從 `Prod.iT6PosCate[i]` 取出來的，中間沒有任何東西改過它，
所以**整個錯誤分支是死碼** —— 那個「良品與不可重測 Bin 必須在相同 Unload 軌道」
的檢查從來沒有真的執行過。照翻並在原地註明；改它要使用者決定。

---

## 5.7 20260923 閘冊對帳 —— §5.3 的「H」收掉，並補上漏的 5 列

> **量的方法**：`python tools/st_gate_ledger.py`（本次新增的常駐工具）。
> **base commit `e19e8bd`**，未 pull 上游（使用者 20260923 裁決本 session 只動 906）。
> ⚠ 上游有一批 `cSetUp.cpp` +922 尚未併入，併入後**要重跑這支**，數字可能變。

### 先講結論：§5.3 的「H」問錯了問題

H 列原本寫「其餘 **27 個未分類 gate** 裡的剩餘候選（如 `CheckARTSetupFile`）」。
這個敘述在寫下的當下也許成立，但 **§5.5（20260915）把當時 48 個 gate 全部分類完之後就作廢了** ——
分類表裡沒有「未分類」這一類，§5.6 講得更明確：「剩下 43 個 gate 全部需要使用者裁決或屬於別的戰役」。
而 H 自己舉的例子 `CheckARTSetupFile` 就是 `W906-ST-W4-C`（`WebStart.cpp:2569`），
**:317 早就列為 🔴**。

**真正的缺口是閘冊本身漂了 11 列**（漏列 5 ＋ 該劃未劃 6），而不是有 27 個閘沒分類。

### 對帳結果（工具產出，不要手抄）

> ⚠⚠ **本節第一版（20260923 上午）有一個錯誤前提，下面是更正後的數字。**
> 第一版假設「碼上有 `GATE(tag)` ⇒ 那個閘還活著」。**錯。** 這棵樹解閘之後
> **會把 `SAFETY-GATE(...)` 註解留在原地**，只在旁邊加一行「已解」。
> 於是第一版把 51 個標記全報成活閘（實際 **44**），而且漏掉四列該劃未劃的
> —— 因為它們的 tag **還在碼裡**，「碼上無、文件有」那條規則看不到它們。

**gate 有三種狀態，判定順序不可換**（`#if 0` 優先序最高：活閘的長註解可能順帶提到
別的 gate「已解」，先看字面就會把一個 🔴 的擋啟動檢查從還債清單上抹掉）：

| 狀態 | 判準 | 數 |
|---|---|---:|
| `LIVE` | 註解區塊後**緊接 `#if 0`** | 40 |
| `OMISSION` | golden 那一行根本沒翻，沒有 code 可包（4 個 `T3-PAUSE-*`，明列白名單） | 4 |
| `DISCHARGED` | 註解區塊裡有「已解／已還／解開／解閘」，閘已經不在 | 7 |
| | **活閘合計** | **44** |

| 量 | 對帳前 | 對帳後 |
|---|---:|---:|
| GATE 標記（相異 tag，已剔除外戰役引用） | 51 | 51 |
| └ **活閘**（LIVE＋OMISSION） | 44 | 44 |
| └ 已解、標記留作歷史 | 7 | 7 |
| └ ❓ 判不出狀態 | 0 | 0 |
| 閘冊列（相異 tag） | 52 | **57** |
| **漂移 1：活閘但閘冊無列** | **5** | **0** |
| **漂移 2：碼上已解但閘冊沒標** | **6** | **0** |
| `#if 0` 區塊 / 其中無 GATE 標記 | 41 / 0 | 41 / 0 |

**本次沒有動任何一行程式碼** —— 活閘 44 這個數字前後相同，就是證據。改的全部是帳。

### ⚠⚠ 漂移 2 抓到的四筆裡三筆是 🔴，其中一筆讓本文件對安全狀態說反話

| 列 | 閘冊原本說 | 實際 |
|---|---|---|
| **W6-F**（:464） | 「★ `SoftStart` 不會被設成 true —— **機台不會開始跑**」 | ⚠⚠ **從 20260918 起這句話就是錯的。** `AI(W906-ST-S3-B1)` 把 `#if 0` 改成 `#if 1`（`WebStart.cpp:3635`），分支結構完整保留，**三個 `SoftStart = true`（golden :6196/:6207/:6230）全部活著**。今天讀那一列的人會得到與事實相反的結論。還原＝改回 `#if 0` |
| W4-E（:318） | 🔴 FTP 下載資料比對不過也照樣啟動 | 20260919 已解（T5-W4E）：`NETDownloadDataCheck()` 368 行逐行翻完（`WebStart.cpp:746`），零個閘 |
| W5-H（:362） | 🔴 VTest MES 批次資訊檢查不過也照樣啟動 | 20260919 已解（T5-W5H）：`CheckLotInfor()` 602 行翻進 `forms/fMesSystem.cpp`，連帶解掉 `bNoRTBinFixFlag` 的連結阻塞 |
| W6-G（:484） | 🟡 少寫一次 IO | 20260919 已解閘（P2b）：759 行翻進 `adam6024.cpp`。現在**會**呼叫 `ADAM_DirectWriteData`；今天它是 no-op 樁（`atester_shims.cpp:326`），**樁換真本體那天這個呼叫點會跟著活** |

> **這就是為什麼閘冊需要對帳工具而不是靠人記得。** 這四筆分別由三個不同的波次
> （S3-B1、T5、P2b）在 20260918–19 解掉，每一次解閘的人都在**碼裡**寫清楚了，
> 但沒有人回頭改 §5.1 —— 而 §5.5 自稱是「ST-W7 的單一決策面板」。

剔除的 2 個外戰役引用：`GATE(CC3)`（`cCounterClear.cpp:419` 的閘，W1-K 註解引用它）、
`GATE(W906-FW-CMD-C)`（`Command.cpp` 兩個被閘的 `fMain->Start()`，`WebStart.h:45-46` 引用）。
**它們不是 ST 的閘，不該進閘冊。**

### 漏的 5 列（🟡，碼上都已指名缺的符號，缺的只是這張表）

**根因**：§5.1 的範圍是 **START 路徑**。其中 4 個閘在 `TfMainWeb::PauseFromWeb()`
（`WebStart.cpp:3767`，golden `:6325-6378`）——**PAUSE 路徑從來沒有自己的還債節**，
所以 T3 波加的閘沒有任何一張表收得到它們。

| 標記 | 位置 | golden | 缺的相依 | 閘掉的後果 |
|---|---|---|---|---|
| `W906-ST-W7-C-DLG` | `WebStart.cpp:514`（另 `:1329` 引用） | :33993-34007 | `ShowMyMessagePWD()`（golden `mymessbox.h:52`）全樹未移植；`PowerSavingMode.cpp:937` 的 `GATE (3)` 同因 | 🟡 golden 不看它的回傳值，**不參與判定**。擋是擋住了，但操作員不知道是哪一項 Bin 設定出錯。這是 ST-W7-C **部分交付**留下的另一半 |
| `W906-T3-PAUSE-1` | `WebStart.cpp:3799` | :6351 | `fStartCondition->WritePickerCount()` —— `forms/fStartCondition.h:268/:466` 自記 INTEGRATION-PENDING，無 `extern TfStartCondition *fStartCondition` 可 include | 🟡 純統計（吸嘴真空次數）。按 PAUSE 那一次的 picker count 不落檔，下次開機讀到的計數少一批 |
| `W906-T3-PAUSE-2` | `WebStart.cpp:3807` | :6353-6354 | `bShowInOutAlarm` / `lblInOutAlarm` 不是 `TfMain` 成員（`csystem.cpp:16852` G27a、`:16858` G32 記過同一個洞，這是第三個呼叫點） | 🟡 純顯示。暫停後主畫面 In/Out 警告字樣不清空，停在暫停前的最後一則 |
| `W906-T3-PAUSE-3` | `WebStart.cpp:3815` | :6355 | `UpdateTaskList()`（golden `main.cpp:6381-6400`）本體是寫 VCL `TStringGrid sgTaskList`，全樹 0 命中 | 🟡 純顯示。Task List 分頁停在暫停前的內容。**正解是發成 tag，不是把 TStringGrid 搬進來** |
| `W906-T3-PAUSE-4` | `WebStart.cpp:3848` | :6374-6377 | `fMain->machineTime.Pause()` —— `TMachineTimeManager` 宣告在 `ProductionInfo/uPAT_Function.h`，PAT_Function 一族未整合（使用者 20260918：「缺 PAT_Function 沒關係，和這有關的先 Mark」） | 🟡 只有 `CUSTOMER_CODE==CC_PANTHER` 走到，純紀錄。鴻谷的稼動時間少了 PAUSE 這個轉折點 |

> ✅ 這 5 個在 §0.5 下**全部合法** —— 每一個的加閘理由都是一個具名的、確實不存在的符號，
> 沒有任何一個是「有風險」。缺陷純粹是**帳沒記**，不是閘下錯了。
> ⚠ 但「沒記帳」的代價是實的：它們不在任何還債清單上，所以 S3 之前的檢查會漏掉它們。

### 該劃未劃的 2 列（已補）

| 列 | 實際 |
|---|---|
| `W906-ST-W1-C`（:247） | **ST-W7-H 20260915 已還** —— `W906ST_Read2DIDList()` 在 `WebStart.cpp:696`，解閘點 `:1239-1244`。碼上 `W1-C` 的 GATE 標記已不存在 |
| `W6-H`（:485） | **ST-W7-G 20260915 已還** —— §5.5「存活的 2 個 FEASIBLE」其實已經寫了「✅ 做掉了」，只有 §5.1 這一列沒跟上 |

### 順帶修掉一個文件自相矛盾

§5.5「存活的 2 個 FEASIBLE」把 `W1-C` 標 ⬜「ST-W7 最後一個可自動做的」，
而 §5.6「ST-W7 實際交付了 6 個」把它列為**已交付**。
**交付表是對的** —— 碼上有本體、gate 已解、`WebStart.cpp:249` 的檔內註解也自述
「ST-W7 交付了 6 個（W5-A / W2-G / W1-E / W2-C / W6-H / W1-C）」。§5.5 那一欄已更正。

### 這不改變 `kWave7State`

`WebStart.cpp:287` 仍是 `kWave7State = 2`（**卡住，不是完成**），而且**本次對帳沒有解掉任何閘**。
它反而讓「2」更站得住：ST-W7 的剩餘工作除了 §5.6 那 43 個之外，還要加上這 5 個
——其中 `W7-C-DLG` 與 PAUSE-1/2/3 的正解都指向 web tag，屬 §5.2 的設計題，不是翻譯題。

### 怎麼重量

```
python tools/st_gate_ledger.py
python tools/st_gate_ledger.py --check 0 0 0 0   # CI：四個數字任一變了就 exit 1
```

四個數字依序是：**活閘但閘冊無列**、**碼上已解但閘冊沒標**、**無 GATE 標記的 `#if 0`**、
**三種狀態都判不出來的 tag**。對帳後四個都是 0。任一變成非 0，就是
「有人改了碼但沒記帳」「有人加了無標記的閘」或「有人加了新閘法」。

⚠ 第四個數字是**防漏**：一個 tag 若沒有 `#if 0`、沒有已解註記、又不在 `OMISSION`
白名單，工具**直接算失敗**，而不是默默把它歸到某一類。這是照 `wave_wrapup_gate.ps1`
的 `$NotRun` 學的 —— 那支自己的檔頭寫著「a runner that can silently omit a gate
is the thing it is supposed to prevent」。

⚠ `--check` 的數字變了代表閘冊與碼又漂了，**回來對帳，不要改期望值**。
工具做了三件人工 grep 做不到的事：正規化 `W906-ST-W5-F` / `W5-F` / `` `W906-ST-W5-F` ① ``
為同一個（那個 ① 在本次第一版盤點就害我誤報一次）、剔除外戰役引用、
以及用「往上掃過連續註解區塊」而非固定行數視窗去判 `#if 0` 有沒有標記
（固定 7 行視窗誤報了 10 個，實際是 0 個）。

---

## 5.8 20260923 分類面板重量 —— §5.5 的「48 個」已經不是今天的 44 個

> **方法**：照 §5.5 自己記載的作法，5 個唯讀 agent 各分約 9 個 gate（44 / 5）。
> **但這次多一道**：每一筆 `FEASIBLE` 都由主迴圈**自己重跑探針**才採信 ——
> 因為 FEASIBLE 是唯一會**授權動手**的分類，而這棵樹的實證是
> 「agent 的論證比它寫的程式碼更常錯」。那一道抓到一筆過度宣稱（見下 W6-I）。
> base commit `e19e8bd`＋本日 906 commits，未 pull 上游。

### 分類分佈：20260915（48 個標記）vs 20260923（44 個活閘）

| 桶 | §5.5（20260915） | 今天 | 變化 |
|---|---:|---:|---|
| **MISSING-OBJECT** | 12 | **20** | ▲8 |
| **WEB-HOLDS** | 10 | **4** | ▼6 |
| **OUTBOUND-MODAL** | 7 | 8 | ▲1 |
| **DEAD-GLOBAL** | 4 | **0** | ▼4 **這一類消失了** |
| **BIG** | 4 | 3 | ▼1 |
| **NOT-THIS-MACHINE** | 2 | 3 | ▲1 |
| **FEASIBLE** | 5 → 複驗後 2 | **5** | ▲3 |
| ❓ UNSURE | — | 1 | — |
| | | **44** | |

🔴 會擋啟動 **21** ／ 🟡 **23**。

⚠ `DEAD-GLOBAL` 歸零不是「解掉了」，是**那個標籤本來就貼錯**。它的定義是
「守衛讀 `LastSet.iRunStartMode`」，而被歸進去的 W3-A 整段（golden :5047-5080）
**沒有任何一行讀 iRunStartMode** —— 讀的是 `IniConfig.iN23DownloadMethod` /
`fBarCode->list2DWhitle->Count` / `RunInfo.bLotStart` / `cbRunMode` /
`CheckNoRetestBinFlag`。複驗當初要推翻它的 FEASIBLE 是對的，換錯了理由。

### ★ 今天就可以還的 5 個（這推翻 §5.6「剩下 43 個全部需要裁決或屬於別的戰役」）

| gate | | 狀態（主迴圈已獨立複驗） |
|---|---|---|
| **W5-B** | 🔴 | ✅ **相依齊全**。`fMain->cbRunStartMode` 存在：`forms/fMain.h:306` 宣告、`forms/fMain.cpp:103` `new TfLotInfoRunMode()`，由 W906-P10（20260921）加入 facade。§5.1:358 與碼上 `WebStart.cpp:2884` 的「沒有這個 widget」都寫於 20260915，已過期。**⚠ 它是本批唯一今天真的 fail-open 的 gate** |
| **W4-C** | 🔴 | ✅ `ComputeCheckARTSetupFile` 本體在 `MainCalcCore.cpp:364`，`MainCalcCore.h:22` 明文「(ATCAmbientTemperCheck, CheckARTSetupFile) are RESOLVED this batch」，另有 9 個測試。§5.1:317 與 `WebStart.cpp:2571` 的「移植樹不存在」是**假的不存在宣稱** |
| ~~**W5-F**~~ | 🔴 | ✅ 相依齊全（`forms/fShuttleMove.h:134 bool fShow;`＋`.cpp:37` 已建構）。**✅ 20260923 已解**：下方 §0.5 裁決題使用者選 a（`AI(W906-ST-W7-W5F)`） |
| **W5-E** | 🟡 | ⚠ 缺一個 bool 成員 `bStartKeyPressCheck`（全樹只出現在註解與兩個 `#if 0` 內）。加它是一行，但**單獨解它不會有任何效果**：唯一讀取點在 `csystem.cpp:19161` 的 `#if 0 // GATE G22` 內，還有 `csystem.cpp:16810` 的 GATE G7。三個要一起看 |
| ~~W6-I~~ | 🔴 | ❌ **FEASIBLE 是過度宣稱，主迴圈複驗推翻。** agent 說「無參數多載移植樹無」，但 `MainCalcCore.h:44-46` 自己寫著 golden `main.cpp:31148-31172` **已經翻成 `ComputeCheckSiteMapState`**。真正缺的是一層 wrapper（回傳極性相反、少 `MyDBIProcess` 與 `ShowErrorMessage` 兩個副作用），而 `WebStart.cpp:3352-3356` 明講「自己補那個 wrapper 等於在波次中途寫新行為，不是翻譯」 |

### ⚠⚠ 一個 §0.5 裁決題：W5-F 的閘沒有合法理由

§0.5：**加閘的唯一合法理由是「相依不存在」**。W5-F 的相依齊全，而樹自己
（`WebStart.cpp:272-274`，20260917 自評）寫的兩個不解閘理由是：

1. 解開會**新增一個擋啟動**（ASE 高雄 / OP 權限 / OFF_LINE）；
2. `:2411` 記著下一支原本是 `else if`、被閘掉才變成裸 `if`，**還要順手還原鏈結構**。

那是**後果**與**工作量**，不是相依不存在。依 §0.5 兩者都不是合法的加閘理由。
⇒ 這一條要使用者裁決：照 §0.5 解閘（會多一道擋啟動），還是為它寫一條明文例外。

**✅ 20260923 使用者裁決：a，照 §0.5 解閘。已做（`AI(W906-ST-W7-W5F)`）。**
兩處的 `#if 0` 都拿掉；四個 `fShow` 照 P6-b 慣例包 `W906_FShow`（Zteach 在 Q20-甲 名單內，恆回關著）；
golden :5814 的 `else if` 還原。新增的擋啟動只影響 `CC_ASE_KaohSiung`（MES1052，kcode==0，
經 `AI(W906-Q30-KZERO)` 不會卡住 tick）；第二處的 `CheckAutoHasTray(false)` 對所有客戶生效，
只在安全門沒關時回 true（靜默擋啟動，golden 同）。

### 這次對到的 14 筆「計畫書寫錯或過期」（44 筆中 32%）

全部落在「缺的相依」欄或「後果」欄 —— **沒有任何一筆改變 🔴/🟡 分級**，
所以安全結論不變；壞的是**決策面板的可信度**。

| gate | 計畫書說 | 實際 |
|---|---|---|
| W4-A 🔴 | 缺 `ATCAmbientTemperCheck()` | 本體在 `MainCalcCore.cpp:244`。碼上 20260915 已自我更正，**文件沒跟上** |
| W4-B 🔴 | 三個缺件，含 `NETDownloadDataCheck()` | 那支 20260919 已翻（`WebStart.cpp:746`）。只剩 2 個。碼上註解同樣過期 |
| W4-C 🔴 | 缺 `CheckARTSetupFile()` | **假的不存在宣稱**（見上表） |
| W5-B 🔴 | 缺 `fMain->cbRunStartMode` | 已存在（見上表） |
| ~~W5-F~~ 🔴 | 缺 `TfShuttleMove::fShow` | 已存在（見上表）；20260923 已解 |
| W6-A 🔴 | 缺 `TfShuttleMove::fShow` / `fTeach`，導向 §5.2 設計題 | 前半過期。五個 fShow 讀取點有四個今天就能安全求值，**唯一硬阻塞是刻意留 NULL 的 `fTeach` 全域**（SIOF 迴避）—— 那是實例缺席，不是 WEB-HOLDS，該移出 §5.2 |
| W1-I 🔴 | 缺 `CheckingCheckList()`（**10 行**） | §5.5 自己（:965-975）早已推翻：背後是 `GenerateCheckList()` **854 行**。§5.1 與碼上都還是舊數字 |
| W2-D 🔴 | 缺 `fPMAlarmInterFace` / `PMAlarm_SYS` | **漏了 `fAutoTeach`**，它在同一個 if 的第三個條件（`WebStart.cpp:1638`）。就算 `fPMAlarmInterFace` 落地了這個閘仍開不了。同文件 doc:266（W2-J）有寫出來，兩列不一致 |
| W6-C 🔴 | 缺 `fBarCode->RunCheckBarcodeByServerData()` | **那不是阻塞點**（golden 三行純述詞，兩個欄位都在）。真正缺的是內層 `TfMesSystem::Get2DIDFromServer`：宣告有、定義零，解閘會**連結失敗** |
| W3-A 🔴 | 複驗改判 `DEAD-GLOBAL` | 標籤錯（見上）。推翻 FEASIBLE 是對的，理由要換 |
| W6-B 🟡 | 結論正確 | golden 出處指錯 class：`fAOI.cpp:3884` 是 `TFrmAOI::spbStartComClick`，而 golden :5994 的接收者 `fBarCode` 是 `TfBarCode*` |
| W6-L 🟡 | 後果「2D code 備註欄不清空」 | golden :5996-5997 **不清任何東西**，是兩個賦值把入料吸嘴列/行寫進格陣尺寸。正確後果是「格陣尺寸不跟著吸嘴配置更新」。碼上 `:3387` 寫對了，文件寫錯 |
| T3-PAUSE-2 🟡 | 引 `csystem.cpp:16852` G27a / `:16858` G32 | 那兩行只是 GATE REGISTER 的索引列；真正的閘在 `csystem.cpp:19549`（G27a）與 `:20183`（G32）。實質判斷正確，**行號引用壞了** |
| T3-PAUSE-4 🟡 | 「PAT_Function 那一族尚未整合」 | **不成立**：`CMakeLists.txt:2648-2653` 把 `ProductionInfo/uPAT_Function.cpp` 編進 archive，其清冊把 `TMachineTimeManager` 八支全標 ACTIVE |

### ❓ 一筆判不出來：W2-H 🔴

缺 `TfMain::bFind`（golden `main.h:1213`）與維護它的 `ProcessHVisionConnect()`
（golden `main.cpp:17694-17872`，179 行，靠 `FindWindow("TSerialPoll","Interface")`
判斷外部 GPIB 橋接程式在不在）。`bFind` 全樹不存在這件事已複驗成立，
但「要不要把一個靠 Win32 `FindWindow` 探測外部行程的機制搬進 web 架構」
不是翻譯題。留 UNSURE。

### 這不改變 `kWave7State`

仍是 2。本次**沒有解掉任何閘、沒有動任何一行程式碼**。
但它改變了「剩下什麼」的形狀：**有 3～4 個今天就能還**（W5-B / W4-C / W5-F 待裁決 / W5-E 需與 G7・G22 合併），
而不是 §5.6 說的「43 個全部需要使用者裁決或屬於別的戰役」。

---

## 6. 硬邊界

- ⛔ **S3 之前必須先裁決 `SECSGEM/uHGemHT9045.cpp` 的四個 `SoftStart = true`**
  （`:5489` / `:5523` / `:7797` / `:7879`，S2F42 host command 路徑）。
  20260915 ST-W6 稽核的新發現，**不是這次改出來的，是既有狀態**：
  這四行全部是活的（逐行量過預處理器深度，不在任何 `#if 0` 裡），
  而且它們**不經過 `fMain->Start()`** —— 所以 §1.2 那套「19 個呼叫點」的盤點
  盤不到它們。下游是通的（`ckernel.cpp:816` → `:1015 SystemStart=true`）。
  也就是說今天這棵樹上，host 端下 S2F42 就能讓機台開始跑，
  完全不經過 `StartFromWeb()`、不經過那 1,875 行檢查。
  ⛔ 本戰役**不碰它**（SECS/GEM 地盤 + 安全關鍵），只負責讓它被看見。
- ⛔ **S3 之前必須單獨審 `CheckInOutArmZHomeSensor()` 這條運動路徑**
  （ST-W7-B，20260915 翻入）。一旦 `StartFromWeb()` 被掛上分派，
  Z 軸原點感測器異常就會讓 `MInArmX` / `MInArmY` 收到 `DecStop()`。
  方向是安全的（**停**，不是動），但它**是一個真的機台動作**，
  不可以在使用者不在場時第一次被觸發。
  同一條路徑上還有 `ShowMyMessage` —— 目前是觀測樁（不停機），
  但換成忠實版本後會連帶帶來 `StopAllMotor()`。兩件事要一起審。
- ⛔ **S3 之前必須補 `ShowMyMessagePWD`**（golden `mymessbox.h:52`，全樹未移植）。
  ST-W7-C 解開了 OLP Bin 設定的擋啟動判定，但三個說明對話框仍閘著 ——
  現場會遇到「按 START 沒反應、也沒有任何提示」。
  同一個缺口也閘著 `PowerSavingMode.cpp:937`。
- ⛔ 不可以只寫 `SystemStart = true` 繞過檢查
- ⛔ S2 與 S3 不可以同一顆 commit
- ⛔ 不解閘 `Command.cpp:16305` / `:17056` 那兩個遠端啟動點（那是另一個決定）
- ⛔ 翻譯階段不碰 `tools/wb_serve.cpp`（另一個 session 的在製工作）
- ⛔ golden 不合理處照翻並註明，改行為要使用者決定
- ⛔ golden 是 Big5：`cp950` 讀、UTF-8 寫、EOL 逐檔保持

標記：`AI(W906-ST-Wn) 20260915: 描述`
