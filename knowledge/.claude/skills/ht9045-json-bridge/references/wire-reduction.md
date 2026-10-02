# §六、通訊縮減（我們有 JSON 結構設計權） —— 全文

> 從 `SKILL.md` **§六** 拆出（拆出日期 2026-09-23，skill 維護第二輪續拆）。
> **章節編號不變**：§六 同名同號，兩段 ⛔更正（Alarm 分工、IO 算術）原位保留。原文**逐字**保留（含已被更正段推翻的原句 —— 那是稽核軌跡），
> `SKILL.md` 原處留 stub（一句話結論 ＋ 指到本檔）。
> 原本被誰引用：`JsonBridge/ChanIo.h:25`、`tests/test_sjson_chan.cpp:200`（IO 算術那段）；SKILL.md §〇、§一、§4.3、§十三 #3／#10（「SKILL §六 更正段」「SKILL §六 `:586`」）。
> 本檔內的 `§X`／`§4.x` 交叉引用一律指 `SKILL.md` 的章節編號，`SKILL.md` 那裡搜得到、再一跳就到對應 reference；
> `references/<檔>.md` 這種路徑是以 `SKILL.md` 所在目錄為基準寫的（原文未改），在本檔內即同目錄的 `<檔>.md`。

---

## 六、通訊縮減（我們有 JSON 結構設計權）

原則：**API 語意不變，線上少送。** 每條都要有量測值支撐，不憑感覺。

| 通道 | 現況 | 縮減方式 | 預期 |
|---|---|---|---|
| 設定 GET | `/api/recipe` 每鍵送 `{value,type,raw}` 三份 | `value-only`；型別／原始字串只在 `/schema` 送一次 | 約 **1/3** |
| 設定 GET | 整份結構 | `?page=` 投影（pagewire 三元組） | 依頁面，`Setup.Contact` 118 個 input vs `TestIF` 788 欄 |
| 設定 GET | 每次全量 | `?since=<ver>`；無變動 304 | 開站一次全量，之後幾乎為 0 |
| 設定 PUT | — | 只送改動的欄位；陣列用稀疏索引物件 `{"3": v}`（`system.levels.put` 已是這形狀） | 一次 Save 通常 1～5 欄 |
| 生產數值 | tag 每 500 ms patch | 沿用 snapshot＋patch，但**只 stage 有變動的**；計數器送差量 | 靜止時 0 |
| IO | 4,481 個 `pci1203.*` 個別 tag；`io.*` 0 個 | ~~`io.di`／`io.do` 各一個 tag，**位元打包 base64**：DI 320 bit = 40 bytes ≈ 56 字元；DO 192 bit = 24 bytes ≈ 32 字元。附 `ver`~~ ⛔ **算術錯誤，見下方更正** | ~~512 個 tag → **2 個**~~ |
| Motor | `motor.*` 0 個 | `motor.axes` 定序陣列 `[[pos,vel,status],…]`，順序由 `/schema` 給一次 | 一個 tag |
| Alarm | 尚無 `alarm.*` tag；對話框走 `Dialog-bridge-contract.json` v1.3.0 | **事件物件**（發生／解除各一筆），不輪詢；佇列「丟最舊、保留最新」 ~~沿用 `web/config/AlarmNonStop.json`~~ ⛔ **分類不在 C++，見下方更正** | 事件驅動 |
| 傳輸層 | WS 一律 | **HTTP 拿全量、WS 只送 patch／put**；HTTP 開 gzip | 大訊息不走 WS |
| WS 大訊息 | 上限未量、且各層可能不同 | **橋接層自己切斷**：超過 `kWsChunkBytes`（建議 32 KB，可調）就分塊送 `{type:"chunk", id, seq, total, data}`，收端組回；必要時對 `data` 先壓縮（deflate，`enc:"deflate"`），收端解壓 | 不依賴任何一層的上限（使用者 20260923 裁決） |

### ⛔ 更正（20260923，§十三 待辦 #10 結案）：Alarm 那一列的「沿用 `AlarmNonStop.json`」講反了分工

原文讓人以為 **C++ 會讀 `web/config/AlarmNonStop.json` 做 NonStop 分類**。實作不是這樣，
而且是**刻意**不這樣（`JsonBridge/AlarmChannel.h:63-66` 的檔頭已寫明，原句保留）：

> 「`web/config/AlarmNonStop.json` 的 `neverNonStop` 四類（EMG／安全門／ESD／溫度）在移植樹
> **同樣不會停機**。那份檔自己的警語說得很清楚：『這張表只管顯示。它不會、也不能讓機台停下來』。
> 本檔不重複那張表、也不做分類 —— **分類是 HTML 的事，C++ 只負責把事實講對。**」

| | C++（`AlarmChannel.*`） | HTML |
|---|---|---|
| 送什麼 | `code`／`kCode`／`position`／`action`／`pressedButton`／`seq`／`dropped` —— **事實** | — |
| 分類 NonStop／neverNonStop | ❌ 不做 | ✅ 讀 `web/config/AlarmNonStop.json` 自己分 |
| 佇列策略 | ✅「丟最舊、保留最新」＋`dropped` 計數（與 S1 EventLog ring 同源，`EventLog.cpp:118-122`） | — |
| `stopAllMotor` | 一律 **null**（`StopAllMotor()` 在移植樹逐字是 `{}`，`aHotPlateSubstrate.cpp:1255`）；另送恆 false 的 `alarm.stopMotorPorted` 讓瀏覽器分得出「沒停機」與「不知道」。⛔ **20260926 核對已過期**：`c3c459f2`（20260924）起告警照 golden 停機、`StopAllMotor` 兩個多載統一，現在每筆事件 `stopAllMotor:true`、`alarm.stopMotorPorted:true`（`JsonBridge/ChanAlarm.cpp:211`／`:185`；`open-todos.md` #5） | — |

⇒ 上表 Alarm 列請讀成：**C++ 只送事實，分類在 HTML。**
⚠ 不更正的話，下一個人會照規格在 C++ 裡**再做一份**分類表 —— 兩份分類表遲早分岔，
  而分岔的那天沒有人會發現，因為兩邊都「有在動」。

⚠ 分塊與壓縮的三條規則：
1. **一律先分塊、再考慮壓縮**——壓縮是「必要時」，分塊是「一定」。小訊息（< `kWsChunkBytes`）維持原樣，不多包一層。
2. 分塊在**應用層**做（`JsonBridge.cpp`），不改 `WebBridge/WsFrame.cpp`。這樣 `WebBridge/` 維持獨立可測。
3. 收端要能處理**亂序與遺失**：以 `id` 收集，`seq` 全到才交付，逾時（建議 5 s）整組丟棄並回 `chunk.timeout`；不要交付殘缺 JSON。

⚠ 縮減不得改變語意：`null`（未載入）與 `0`（讀到零）**仍然要分得開**（`WebBridgeTags.h` 開頭那條規則），位元打包只用在 IO 這種本來就是 bit 的東西。

### ⛔ 更正（20260923，S9 實作時發現）：上表 IO 那一列的算術是錯的

原文寫「DI 320 bit = 40 bytes ≈ 56 字元」。**1203 監看器的一個「port」是一個位元組，不是一個位元。**

證據（20260923 實測原始碼）：
- `Pci1203DiSample::byteData` 宣告成 `unsigned char`（`EtherCAT/Pci1203Monitor.h:1099`；DO 側 `:1136`）。
- 舊的 `pci1203.di<i>` tag 送的就是那個 **0..255** 的值，不是 0/1。

⇒ 正確的四個平面（實作已照這個做，`GET /api/struct/io/schema` 可覆核）：

| tag | bytes | bits | base64 長度 | 是什麼 |
|---|---|---|---|---|
| `io.di` | **320** | 2,560 | **428** | 資料平面，每個 port 一個 byte |
| `io.di.valid` | 40 | 320 | 56 | **在位平面**，每個 port 一個 bit |
| `io.do` | **192** | 1,536 | **256** | 資料平面 |
| `io.do.valid` | 24 | 192 | 32 | 在位平面 |

⇒ 原文那個「40 bytes／56 字元」**剛好等於在位平面的大小**，兩個數字長得像，差 8 倍。
⚠⚠ **照原文開 40 bytes 的緩衝去裝 320 個 port 的值，是一次 280 bytes 的溢位，而且前 40 個 port 看起來還是對的。**
`tests/test_sjson_chan.cpp` 已加斷言把 320 B→428、192 B→256 釘住（並與 schema 的 `b64len` 對帳），
免得有人照原文把緩衝改小而測試仍然全綠。

⇒ **減量是 10.9 倍不是 ~100 倍**（實測：512 個值 tag 有真值時約 9,508 bytes → 7 個 `io.*` tag 約 870 bytes）。
⚠ 而且**今天還沒有減量**：512 個舊 tag 刻意保留（`web/js/pci1203/view.js:3736/:3764/:3794/:3979` 還在讀），
兩套並存反而多 870 bytes。刪除必須與 web 同一個 commit。

⚠ 第二個平面（`*.valid`）**不是可選的**：一個 DI byte 有三態（沒有這個 port／讀失敗／讀到 0x00），
只送資料平面會把後兩態壓成同一個 0 —— 那正是本節最後一句「`null` 與 `0` 要分得開」禁止的事。

