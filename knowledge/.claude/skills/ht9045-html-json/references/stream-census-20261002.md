# tag 串流普查與「少送」的空間（20261002 實測，Jerry）

> 本檔回答兩個問題：**①「四大類」在程式裡存不存在？② 現在每拍送的東西，有多少是沒人要的？**
> 全部是對**跑著的 `wb_serve`** 量的，唯讀（只做 WS 被動收訊與 HTTP GET，不送任何指令訊框）。
> 分母會變 —— **引用任何數字前先重跑**。

---

## 0. 先講三個會讓人白做工的坑

### 0.1 ⛔ 不要自己寫 WebSocket client —— 樹裡已經有兩支

| 工具 | 做什麼 |
|---|---|
| `HT9011UC_Cpp_V3.33.906.0/tools/webprobe/snap_dump.py` | 倒出第一個 `{"type":"snapshot"}` 訊框的每一個 key（`--out` 可存 JSON 供離線重算） |
| `HT9011UC_Cpp_V3.33.906.0/tools/webprobe/channel_census.py` | **本檔的數字就是它產的。** C++ 發了什麼 × 網頁有沒有人讀 × 值有沒有被維護，三層一起答 |

`tools/webprobe/README.md` 的姿態寫得很清楚：**這個目錄裡不要有第二支 frame parser**
（`snap_dump.py` 自己也是 `from cmd_probe import ws_handshake, read_frames`，不重寫）。

> 20261002 Jerry 手刻了一支 WS client 才發現這兩支已經存在。手刻版的 `changed=6`、`null=721`、
> `published=1790` 與 `channel_census.py` **完全相同**，所以不是結果不對，是**白做**。
> 要量這類東西，**先 `ls tools/webprobe/`**。

### 0.2 ⛔ `W906_OPLOG_DIR` 開著的時候，`[STREAM]` 的 apiCache 數字是灌水的

```cpp
// tools/wb_serve.cpp:6424
::LeaveCriticalSection(&g_apiCacheLock);
if (built[3]) ::W906_OpLogMotorRuntime(g_apiCache[3]);   // ← op log 開著時才跑
ht9045::StreamNoteApiCache(ht9045::StreamNowUs() - streamT0, ...);   // ← 計時在它「之後」才停
```

`W906_OpLogMotorRuntime`（`tools/wb_serve.cpp:8344`）在 `g_opOn` 為真時，會對**整包 motor runtime
JSON 做一次 `cJSON_Parse`**，逐顆馬達取 20 幾個欄位、與上一次比對、把變化寫進磁碟 —— **每秒約 6 次**，
而且**整段算在 apiCache 的計時裡**。

`g_opOn` 只由環境變數 `W906_OPLOG_DIR` 打開（`W906_OpLogInit`，`:8316`）。

⇒ **要量 apiCache 的真實成本，必須先把 `W906_OPLOG_DIR` 清掉。**
20261002 第一次量到的「apiCache 每拍約 69 ms」**是在 op log 開著的狀態下量的，尚未扣除這一項**。

### 0.2a 量測前的「無磁碟」檢查單（20261002 Jerry 實測）

跑著的 `wb_serve` 在閒置時只有兩個寫碟者，量過寫入速率：

| 檔 | 速率 | 是什麼 | 量測時要不要關 |
|---|---:|---|---|
| `D:\HT9045_Log\oplog\oplog_<date>.txt` | **+10,843 B / 30 s（361 B/s）** | op log，**量測工具自己的** | ✅ **關**。而且真正的成本不是這些位元組，是 `W906_OpLogMotorRuntime` 每秒 6 次的 `cJSON_Parse` |
| `D:\HT9045_Log\Temperature\<yyyymm>\<date>.csv` | +186 B / 30 s（6 B/s） | 溫度記錄，**golden 本來就有的行為** | ❌ **不要關**。它是真實工作量的一部分，關掉反而不具代表性 |

**關法**（PowerShell，使用者環境變數）：

```powershell
[Environment]::SetEnvironmentVariable('W906_OPLOG_DIR',$null,'User')     # 關
[Environment]::SetEnvironmentVariable('W906_OPLOG_DIR','D:\HT9045_Log\oplog','User')   # 還原
```

⚠ **必須把 VS Code 整個關掉再開**（結束 `Code.exe`，不是開新視窗）。
`W906_OpLogInit` 只在啟動時讀一次環境變數，而 F5 起的 `wb_serve` 繼承的是 VS Code 行程的環境。

✅ **`[STREAM]` 不會跟著消失**：`WebStreamStats.cpp:206-212` 先 `printf` 到主控台，
op log 只是額外掛上去的 `g_sink`。所以關掉 op log 之後 A／B 對照照樣做得成。

> `channel_census.py` 本身只在結束時寫 `--out` / `--json`，量測窗內不碰碟；要完全不寫檔就兩個都不給。

### 0.3 這台機器 `python` 不能用，要用 `py`

`python` 解析到 Microsoft Store 的 App Execution Alias，**靜默 exit 49**（沒有任何輸出）。
一律 `py script.py`。

---

## 1. 「四大類」在程式裡不存在

### 1.1 線上格式是完全扁平的

```cpp
// WebBridge/TcpTagLink.cpp:122
{"type":"snapshot","gen":N,"data":{ "tag":value, ... }}

// WebBridge/TagSnapshot.h:76
typedef std::map<std::string, TagValue> TagMap;
```

**沒有分類欄位、沒有分組、沒有巢狀。** 而且這是**刻意的**，決定記在本 skill 的
「⛳ B 路的正式契約」那一節：metadata 不掛在每個 tag 上（會撞
`WebBridgeServer.cpp:258` 的 **64 KiB 單則訊息上限**，超過直接斷線），
`updateClass` 留在 HTML 端。

⇒ **分類的唯一載體是 tag 名字的點號前綴，那是命名慣例，沒有任何程式強制它。**

### 1.2 程式裡實際存在的是三套互不相通的局部分類

| 機制 | 位置 | 分幾類 | 目的 | 涵蓋 |
|---|---|---|---|---|
| `StreamFamilies` | `WebStreamStats.cpp:117` | 4：`pci1203.` / `secs.sv.` / `motionView.` / **other** | 只為了印 `[STREAM]` | 3 個前綴寫死；20261002 的 `other` = 1015，佔 57% |
| `W906_PageStreamWanted(webId)` | `WebPageTable.cpp:694` | 按**消費頁面**分組 | **真的會少送** | **只接了 3 個家族**（見 §3） |
| `wbtest::IsBuildFactTag` | `tests/wb_buildfact_tags.h` | 1 個白名單 | 測試豁免（編譯期事實可非 null） | `build.*` |

**沒有 tag registry。** 沒有任何一份檔列出「現在有哪些 tag、屬於哪類、誰在看、多久變一次」。
`tests/test_wb_tags.cpp:88` 的家族普查是**印出來不斷言**的，註解自己寫了理由：
寫死的數字一定會過時。

程式自己知道缺這塊 —— `WebBridgeTags.cpp:620` 的註解開頭就是
**「A new tag CATEGORY, and the category matters more than the individual tags.」**

---

## 2. 20261002 的普查數字

跑法（機台閒置、`pump.guard.systemStart=false`、筆電無 1203 卡、探針未註冊任何頁面）：

```
py tools/webprobe/channel_census.py --port 8045 --seconds 20 --out <報告.md> --json <sidecar.json>
```

| 項目 | 數量 |
|---|---:|
| C++ 發布的 tag | **1790**（snapshot 1 訊框＋patch 80 訊框） |
| 　值為 null | 721（40.3%） |
| 　20 秒觀測窗內值有變動 | **6** |
| **兩邊都有**（有發、有人讀） | 701 |
| **有發無人收** | **1089（60.8%）** |
| **有收無人發**（頁面讀、C++ 沒發） | 42 |

### 2.1 「有發無人收」的家族分佈

| family | tags | bytes |
|---|---:|---:|
| **`secs`** | **772** | 15,537 |
| **`temp`** | **213**（217 之中；214 個是 null） | 6,182 |
| `def` | 61 | 1,627 |
| `prod` | 16 | 281 |
| `alarm` | 7 | 147 |
| `guard` | 6 | 181 |
| `log` | 4 | 57 |
| `io` / `machine` / `motor` | 各 3 | 186 |
| `cfg` | 1 | 12 |
| **合計** | **1089** | **24,210 B（43.1%）** |

### 2.2 `secs.*` 772 個 —— 整個網頁樹一次都沒出現

```
grep -o "secs\." web/page/*.html web/page/*.js web/js/*.js web/js/*/*.js   →  0
```

（對照：`def.` 54、`sort.` 58、`lot.` 120、`site.` 87、`temp.` 18、`pump.` 15、`tcat.` 16。
`web_merged_20260919/` 底下的 `secs` 命中全是英文字 seconds，而且那不是在跑的樹。）

**它的來源是驗收條件，不是畫面需求** —— `docs/PLAN_START_TO_RUN.md:2591`：

> 「**完成條件**：`wb_serve` 的快照裡出現 ≥745 個 `secs.sv.*`」

⇒ 要不要繼續發，是**裁決題不是技術題**。若要停發，得先確認沒有 ctest 綁著那個數字。

### 2.3 20 秒內真的變過的只有 6 個

```
pump.mainProcCalls   40 次   ← 診斷計數器
pump.ticks           40 次   ← 診斷計數器
secs.sv.1032         40 次
secs.sv.1035         40 次
clock.text           20 次
tower.amber           7 次
```

187 筆變更裡 **80 筆（43%）是 `pump.*` 兩個診斷計數器**。
⇒ publish 每拍整理 1790 個 tag，找出來的變化平均不到 2 個。

⚠ **這是閒置基準。** 機台跑起來第 4 類的變化會多很多，但 `def.*`／`machine.*` 這類
開機就定了的照樣每拍重整理。

---

## 3. 開頁閘：機制已經在跑，但只接了 3 個家族

頁面表（St01 skill `ht9045-page-table-fshow`，`WebPageTable.h:64-155`）：

| | |
|---|---|
| 列數 | **90**：網頁 **69**（`kPgWeb` 62＋`kPgBoth` 7）、C++ 對話框 10、沒有網頁 11 |
| 誰回報 | `web/background.html` 的 `pushRegistry`：開／關／縮小當下立刻送、**5 秒心跳**、`pagehide` 一次全關 |
| 新鮮度 | **15 秒**；聯集判斷（任一瀏覽器說開就是開） |
| 問法 | `bool W906_PageStreamWanted(const char* webId)`（`WebPageTable.cpp:694`） |
| 線上可見 | tag `ui.pages` 就是整張表（**11,009 B，佔快照 20%**，單一最大 tag） |

**已接上的三個**（全樹只有這三處）：

| 家族 | 呼叫點 | 效果 |
|---|---|---|
| `pci1203` | `WebBridgeTags.cpp:1403` | 沒開 1203 頁就省掉 **~5,900 個 tag** |
| `motionview` | `tools/wb_serve.cpp:2894` | `motionView.trays.*` / `screenScale*` |
| `home` | `tools/wb_serve.cpp:2895` | Home Monitor 的 `home.*` |

> 1790 這個數字本身就是閘在運作的證據 —— 探針沒註冊任何頁面，所以 1203 區塊沒被整理。

⛔ **誰可以動**：`RULINGS_20260930` 第 12 條 —— 網頁更新頻率（C++ 的 tag 整理／發布、hub、
各頁輪詢）由**筆電全部接手**，St01／St02 不做串流／輪詢／發布的改動。

---

## 4. ⚠ 靜態分析定不出「哪個 tag 屬於哪一頁」

大部分 tag 名字是執行期組出來的（`tcat.sgArm1.r0.c0`、`sort.art.auto1.count`），
所以「這個 tag 只有某一頁在看」沒辦法只靠掃原始碼得到定數。Jerry 20261002 跑了兩種判準：

| 判準 | 單一頁面專屬 | 沒有消費者 |
|---|---:|---:|
| 寬鬆（允許退到家族名比對） | 759 tags / 42.4% | 857 / 47.9% |
| 嚴格（只認至少兩層的完整字面） | 326 tags / 18.2% | 1,457 / 81.4% |

兩者差 2.3 倍 ⇒ **不可以拿任何一組直接去改程式。**

`channel_census.py` 已經把這個問題制度化，它的 `via` 欄位記的是**消費等級**而不是檔案：

| 等級 | 意思 |
|---|---|
| `strict` | 出現在真正的訂閱語境（wire 檔的 `tags:{}`、`HT9045Recipe.tags` 的 `get/has/on/subscribe`、`ws.js` 的 snapshot／patch 索引） |
| `loose` | 任何字串常值剛好等於已發布的 tag 名 |
| `prefix:<pre>` | 字串常值後面接 `+` 或 `${`（動態組名），已發布且以它開頭的都算被讀 |

⇒ **要定出逐頁歸屬，必須走「每個家族找它的 wire 檔實際訂了什麼」，不是掃字串。**

---

## 5. 可以省多少（20261002 閒置基準，1790 tags / 56,189 B）

| | 內容 | 省 | 難度 |
|---|---|---:|---|
| **A** | 停發「有發無人收」的 1089 個 | **1089 tags / 24,210 B＝ 60.8% / 43.1%** | 低；但 `secs.*` 要先裁決（§2.2） |
| **B** | 把開頁閘從 3 個家族推廣到單頁專屬家族 | 待定（§4：靜態分析給不出定數） | 低，機制與 ctest（`test_streamgate.cpp`）都現成 |

**A 單獨做就把每拍要整理的量砍掉六成。** 而且 A 不需要寫閘，只需要決定還要不要發。

---

## 6. 本檔的量測限制（引用前務必看）

1. **機台是停的**（`pump.guard.systemStart=false`）。這是閒置基準。
2. **筆電沒有 1203 卡**，`io.*`／`motor.*` 全 null，1203 區塊也沒被整理。
3. **探針沒註冊任何頁面**，所以開頁閘擋掉的家族不在 1790 裡面。真的瀏覽器開著頁時數字更大。
4. **`W906_OPLOG_DIR` 當時是開著的**（§0.2）⇒ apiCache 的計時含 op log 成本，尚未扣除。
5. 「有收無人發」那 42 個（普查報告 §B）是另一類問題 —— 頁面在讀、C++ 沒發，
   多數是 `pci1203.control.*`／`pci1203.axScan.*`／`home.*`／`motionView.*`，
   其中一部分正是被開頁閘擋住的（探針沒開那些頁），**不能直接當成壞掉的接線**。
