# ★ 已決斷（Steven 已裁決，或 St01 已照建議做）

> 從 `todo.md` 的 ★ 節拆出來（Steven 20260927 要求分檔）。還沒決定的在 [decisions-pending.md](decisions-pending.md)。
> **R 題**是 St01 照建議先做的，Steven 看了有意見仍可推翻（在對話裡說，或寫在該題「目前狀態」後面）。
> 裁決的正式紀錄：St01 的 `HT9011UC_Cpp_V3.33.906.0/docs/RULINGS_20260926.md`（S 編號）、筆電的 `RULINGS_20260927.md`（第 N 條）。
> **裁決進度表**（每條裁決的狀態、最後更新、原文連結）：[rulings-index.md](rulings-index.md)（20260927 從 `D:\docs\ops\registers\HT9045_裁決進度表.md` 搬進來）。

## St01（Steven01，資料讀寫）

> **路徑寫法**（Steven 20260927 要求改成絕對路徑）：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\` 開頭＝golden V912（BCB6 原始碼，cp950／Big5 編碼，VS Code 要用「Reopen with Encoding」選 Big5 才不會亂碼）；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\` 開頭＝移植樹（C++，UTF-8）；`D:\HT9045\HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422\` 開頭＝V899（BCB6，cp950）；`D:\HT9045\web\` 開頭＝網頁。只寫 `:行號` 的，是同一句（或表格同一格）前面那個檔。行號是 20260927 當下的檔案。commit 後面括號裡是那顆 commit 改的主要檔案。
> 裁決紀錄：RULINGS_20260926（S 編號）＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md`；RULINGS_20260927（第 N 條，筆電寫的）＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260927.md`；FROM_STEVEN／TO_STEVEN／CHAT_ST02 的唯讀快照在 `D:\HT9045_handoff\`。

### Steven 已裁決的 Q

### Q1. 主畫面狀態列線上格式怎麼送（S112 第二步／todo E-006）
**背景**：主畫面狀態列（Index 節拍、UPH、版本…）第一步已把假值改 "---"（`8ebb43f9`：D:\HT9045\web\page\main.html、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cObserver.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebShowBinSelect.cpp、D:\HT9045\web\page\ht9045_showbinselect_wire.js 等 10 個檔），第二步要決定資料怎麼送到頁面。
**選項**：A 伺服器組好顯示字串（caption）直接送，頁面照收照顯示／B 伺服器只送數值＋可見度，頁面自己照 golden 組字，伺服器不用管顯示格式。
**St01建議**：B——不動 Jimmy 的引擎顯示邏輯，伺服器職責單純，之後改版更彈性。
**目前狀態**：**已裁決（Steven 20260927 08:5x，經 github-02 轉述；RULINGS_20260926 S123：B）**——C++ 只送數值、頁面照 golden 組字；Index time 每次 Index 完成更新（約 3 秒）、UPH 在 loader 換盤計算時更新（最快約 1 分鐘）、版本號只在開機送一次。St01 派工中（E-006 第二步）。（先前紀錄：等決定，沒動。）　**已做 `4e49cbf6`**（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp、D:\HT9045\web\page\main.html；S123：mainsb.uph 整數、mainsb.indexTime、mainsb.version；頁面組字）。

### Q2. Data.ContactCT 唯讀 *.get 要不要免權杖（S115／todo E-008）
**背景**：Data.ContactCT 要即時更新，需要頻繁送唯讀查詢指令，但目前系統設計是指令都要帶權杖才能執行。
**選項**：A 不免權杖，頁面拿不到權杖時就跳過這一拍不更新（比照 Data.Observer 現行做法）／B 唯讀指令一律免權杖（要改 WebBridge 共用元件，Jimmy 也要點頭）。
**St01建議**：A——不動共用元件，風險小。
**目前狀態**：**已裁決（Steven 20260927 08:5x，經 github-02 轉述；RULINGS_20260926 S124：B）**——唯讀 *.get 免權杖（「一點點誤差是沒關係的，因為可能一秒鐘就更新一次」）。權杖檢查在 WebBridge 共用元件，St01 出片段給 Jimmy；S115 解除阻塞、派工中。（先前紀錄：等決定，沒動。）

### Q3. DIO 設定檔第二個寫入口要不要防呆（S101 交件發現／todo F-005）
**背景**：TTLCfg 頁存的 DIO ini 檔名是動態的，wb_serve 的 `CRouteOwner`（`kOwned` 表）認不出這個動態檔名，舊的 B 路 `system.file.put tag=dio` 還是能繞過 C 路守衛直接寫同一個檔案。
**選項**：A 讓 `CRouteOwner` 能比對 DIO 動態檔名、擋掉 B 路／B 維持現狀（有機會被舊指令繞過寫壞檔案）。
**St01建議**：A。
**目前狀態**：**已裁決（Steven 20260927 08:5x，經 github-02 轉述；RULINGS_20260926 S125：A）**——CRouteOwner 比對 DIO 動態檔名、擋掉 B 路。St01 派工中。（先前紀錄：等決定，沒動。）

### Q4. S12 第一型 `/api/form` 剩下的路由可否退役（skill 同步發現／todo F-006）
**背景**：第一型 `/api/form`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\FormJson.cpp`＋`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\gen\form_*.gen.cpp` 那 4 頁）已全部改走 C 路或 A 形狀，看起來沒有頁面在用，但沒查證是否還留著死路徑。
**選項**：A 退役第一型（保留 `/api/form` 路由本身與第二型，因為 `HotPlate` 還在用第二型），退役前先確認 `D:\HT9045\web\page\ht9045_contact_wire.js` 沒有呼叫第一型／B 維持現狀不動。
**St01建議**：A。
**目前狀態**：**已裁決（Steven 20260927 10:4x 重選，經 github-02 轉述；RULINGS_20260926 S126 更正＝S160）：B**——先把 Setup.Speed、Setup.TrayAssignment、Config.DIOInterFaceCFG 三頁從第一型搬到第二型（A 形狀 FormBridge，St02 09:24 發現這三頁的 formOverlay 靠第一型顯示 DoIniDataToForm 算過的值），再退役第一型；FormLock／FormUnlock 不動。St01 做（用 St01 的 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\gen_formbridge.py）。（先前紀錄：**已裁決（Steven 20260927 08:5x，經 github-02 轉述；RULINGS_20260926 S126：A）**——第一型退役，/api/form 路由與第二型（HotPlate）保留；FormLock 不動。St01 派工中。（先前紀錄：等決定，沒動。）　**追問（ST01-E 20260927 14:4x）**：查證結果三頁從來沒靠第一型（開頁走 C 路）、第一型已由 St02 退役並進 main（`834fcc78`→`ed7df426`），請 Steven 確認不必再搬第二型——見 `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md` 的「### Q4-2.」。
**⚠ 重選（20260927 09:2x）**：S126 已裁 A，但 St02 唯讀盤點發現前提不成立——`D:\HT9045\web\page\ht9045_wire_engine.js` formOverlay（:1553，現在位移到 :1554；每個引擎頁載入後都呼叫）打 `/api/form/<頁>`，**Setup.Speed、Setup.TrayAssignment、Config.DIOInterFaceCFG 現在靠第一型**顯示 C++ `DoIniDataToForm` 算過的值；直接退役，這三頁只剩檔案原值。請選：**A** 照樣退役／**B** 先把這三頁搬到第二型再退役（St01 認為 B 比較安全）。回之前不動（St02 已直接問過 Steven）。）

### Q5. Jam 匯出完成要不要跳選單通知（S54-1／todo 無）
**背景**：golden Jam 匯出改到背景執行緒後，97 秒跑完目前頁面沒有主動提示。
**選項**：A 維持現狀不主動跳提示／B 完成後跳一個通知。
**St01建議**：A，先簡單，需要再加。
**目前狀態**：**已裁決（Steven 20260927 08:5x，經 github-02 轉述；RULINGS_20260926 S127：A）**——不跳選單通知；可選做匯出進度條。（先前紀錄：等決定，沒動。）

### Q6. Jam 匯出補鍵要不要寫回真檔（S54-2／todo 無）
**背景**：匯出時發現缺鍵會補值，但目前只在複製到 `%TEMP%` 的副本上補，不動原始檔案。
**選項**：A 維持只補副本／B 也寫回真檔（跟 golden 更一致，但改到正式資料檔）。
**St01建議**：A。
**目前狀態**：**已裁決（Steven 20260927 09:0x 確認，經 github-02 轉述；RULINGS_20260926 S128：B）**——Jam 匯出時缺鍵的預設值寫回真的 `D:\HT9045\Error\English\JAM0000.dat`；寫回要在主執行緒、寫前備份。St01 派工中。

### Q7. Jam 匯出要不要加速（S54-3／todo 無）
**背景**：目前忠實照 golden 逐行跑，97 秒左右，跟 BCB6 一樣慢。
**選項**：A 維持現行速度（照 golden，好對照）／B 優化加速（可能偏離 golden 邏輯，需重新驗證正確性）。
**St01建議**：A。
**目前狀態**：**已裁決（Steven 20260927 09:0x，經 github-02 轉述；RULINGS_20260926 S129：短期 A）**——維持照 golden；長期可換做法，但改格式要換新檔名（舊機台還有舊檔）⇒ 長期待辦，現在不做。（先前紀錄：等決定，沒動。）

### Q8. `security.passwd` 傳輸本身要不要另外加密（S55-1／todo 無）
**背景**：密碼本身不會出現在傳輸內容裡，但 WS 傳輸層本身沒有額外加密（跟其他指令一樣）。
**選項**：A 維持現狀（密碼不進訊息內容已算保護）／B 傳輸層額外加密（工程量較大）。
**St01建議**：A。
**目前狀態**：**已裁決（Steven 20260927 09:xx，經 github-02 轉述；RULINGS_20260926 S130）**——＝**A**：維持現狀（密碼不進訊息內容）。（先前紀錄：等決定，沒動。）

### Q9. 本機二進位密碼本要不要支援（S55-2／todo 無）
**背景**：`D:\HT9045\system\login.dat`（客戶 791 專用）只有特定客戶碼在用，golden 對它的按鈕本來就是空操作。
**選項**：A 維持現在做法：回絕、只列名單，不支援這本密碼本／B 之後支援讀寫。
**St01建議**：A（golden 本身也沒真的用它）。
**目前狀態**：**已裁決（Steven 20260927 09:xx，經 github-02 轉述；RULINGS_20260926 S131）**——＝**B**：之後支援 login.dat 讀寫；做法要等 Q10（S132）釐清兩種 login.dat 寫法後再開工（建議交 Steven02）。（先前紀錄：等決定，沒動。）　**追加（Steven 20260927 下午，St02 記在 `c01e6821`／`v906/steven-gpib-widget`）**：Q9 備份＝A——保留最新一份 `<book>.bak_<時間戳>`，更舊的刪掉（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\LoginDatBook.h` 的 `kKeepLatestBackup=true`；這個檔在 St02 的 `v906/steven-st02-on-cbridge`，St01 樹還沒有）。

### Q10. golden 密碼存檔跟現行「不送不印」不一致要不要在意（S55-3／todo 無）
**背景**（St01 20260927 逐行核對 V912 後改寫。原本寫「golden 存密碼是明文」不對，Steven 說得對：有一部分密碼存檔前會先用一把金鑰轉換）：
- 金鑰：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\common.cpp:46` `asKeyStr="HontechPassword"`（原註解「密碼本的金鑰, 不能改」）。
- 轉換：`EncodeStr`（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\common.cpp:267-293`）把每個字元「減 1，再跟金鑰對應位置的字元做 XOR」，金鑰 15 個字元循環使用；算出來剛好是 0 時改放金鑰那個字元（避免字串被截斷）。`DecodeStr`（`:295-321`）反過來算。這是換字，不是真正的加密：知道金鑰的人一行程式就能還原。
- 用哪一本密碼本由客戶碼決定（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:10843-10863`）：AMKOR China／QUALCOMM 用 `D:\HT9045\system\userid.com`；「用 login.dat 當密碼本」的客戶碼（`bUseLoginDatToSetLevel`）用 `D:\HT9045\system\login.dat`；其他客戶用 `C:\winnt\system32\tech.com`，這個檔不存在就是「下拉選單模式」。
- 所以 golden 有兩種密碼本，再加上下拉選單模式，**同一個 login.dat 有兩種寫法**。「明文還是轉換過」要一條一條看（表一）。

表一　golden（V912）所有「登入密碼寫進檔、讀出檔」的路徑

| # | 檔 | 什麼時候走這條 | 檔裡存的是 | 寫入點（golden 行號） | 讀出時怎麼解（golden 行號） |
|---|---|---|---|---|---|
| 1 | 文字密碼本 `tech.com`／`userid.com`，每行「帳號 等級 密碼」 | 檔案存在，且 `bPasswordSecret`=false（預設值 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\CosFunction.cpp:4125`；只有第 2、3 列那 3 個客戶碼是 true） | **明文** | Security 頁改密碼 `ChangePassword`：New `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSecurity.cpp:723`、Delete `:748`、Edit `:779`（寫回的是舊密碼，見 Q11），存檔 `:791` | 不解碼，直接比對（不分大小寫）`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:15141-15160` |
| 2 | 同上 | `bPasswordSecret`=true（AMKOR China `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\CosFunction.cpp:2030`、RF360 `:2467`、QUALCOMM `:2572`），且 `D:\HT9045\system\Gerneral.ini [Password] Change`=0（還沒轉換過） | 第一次按下任何一顆改密碼鈕：整本每一行的**密碼欄**改成 EncodeStr、`Change` 設成 1、跳「Password Secret finish!!」，這一次不開對話框 | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSecurity.cpp:613-621`、`:646-666` | — |
| 3 | 同上 | `bPasswordSecret`=true，且 `Change`=1 | 密碼欄存 **EncodeStr**（帳號、等級不轉換） | New `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSecurity.cpp:718`；Edit 用 EncodeStr(舊密碼) 比對 `:759`，寫入 EncodeStr(新密碼) `:774` | 先 `DecodeStr` 再比對 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:15156-15159` |
| 4 | 同第 1～3 列那本文字密碼本 | SECS/GEM 主機遠端改密碼（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\SECSGEM\uHGemHT9045.cpp:2614` 呼叫） | **明文**。這條不看 `bPasswordSecret`：加密模式下被改的那一行會變成明文，下次登入時 DecodeStr 對不上（golden 潛在缺陷） | `TfMain::ChangePassword` `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:32819-32850` | 同第 1、3 列 |
| 5 | `D:\HT9045\system\login.dat`（二進位 `PASS_WORD` 結構，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cprod.h:2710-2716`，固定 64004 位元組＝筆數 4＋帳號 1000×30＋密碼 1000×30＋等級 1000×4） | `bUseLoginDatToSetLevel` 的客戶碼，共 13 處：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\CosFunction.cpp:1018/1130/1604/1638/1723/2164/2280/2387/2405/2693/3250/3629/3812`。791＝CC_SJ_Semiconductor 在 `:1604`，Steven01 這台就是 | 帳號、密碼**都存 EncodeStr** | **golden handler 本身不產生這本**：由外部工具 PW_Editor 產生（`D:\HT9045\Password_V1.00.648\password_editor.cpp:73-122`，`:97`／`:100` EncodeStr，一次只編前 30 格），或從網路下載（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:10852` pwName）。handler 只做兩件事：開機時 `ReadPassword`（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cprod.cpp:1374-1386`，呼叫點 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:9420`、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cinitial.cpp:5847`）讀進記憶體；關閉 Security 頁時 `SavePassword`（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSecurity.cpp:464` → `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cprod.cpp:1360-1372`）把記憶體原封寫回，不做轉換。這個模式下 Security 頁的改密碼鈕走的是第 1 列的文字路徑（`FileExists(login.dat)` 為真，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSecurity.cpp:611`）：二進位檔被當成文字讀進來、存成文字，關頁時又被 `:464` 蓋回去。結果是畫面顯示成功，實際沒有改 | 帳號、密碼都先 `DecodeStr` `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:14986-15029` |
| 6 | 同一個 `login.dat`、同一個結構 | **下拉選單模式**（沒有文字密碼本，也不屬於第 5 列的客戶碼）：在 Security 頁按 Engineer／Supervisor 鈕設定該等級的密碼 | **明文**，放在第（等級－1）格，帳號欄留空 | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSecurity.cpp:795-811`（`:809` strncpy 新密碼）→ `SavePassword` `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cprod.cpp:1360-1372` | 不解碼，直接比對 `USER.PassWord[i]` `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:13250-13320` |
| 7 | `D:\HT9045\system\Gerneral.ini [VENDER] HONPREC`（最高權限密碼） | 所有模式 | **明文**（缺鍵時寫入程式內建的預設值；舊鍵名 HONTECH 會搬過來） | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:13221-13232` | 不解碼 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:13264-13265` |
| 8 | `D:\HT9045\system\lastdata.dat`（`LastSet.szSupervisor`） | Configuration 頁設定 Supervisor 密碼的那顆鈕 | **明文** | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6052-6059`（寫進 LastSet，跟著 lastdata.dat 存檔，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cprod.cpp:1925`） | 不解碼 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:10568-10569` |
| — | `D:\HT9045\system\levelset.dat` | — | **沒有密碼**，只有 256 個權限等級（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cprod.h:1164-1168`、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSecurity.cpp:1474-1515`） | — | — |

（FTP、EP、ESD、RMS 這類設備密碼不是登入密碼，不在這題範圍。）

表二　移植樹現在每一條路怎麼做（路徑都在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0` 底下）

| # | 移植樹 | 跟 golden 一樣嗎 |
|---|---|---|
| 1～3 寫入 | 網頁改密碼 `security.passwd`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:563-1158`）：整本轉換 `:1014-1024`、New `:1072-1093`、Delete `:1094-1119`、Edit `:1120-1150`、存檔 `:1153`（這一格的行號是 commit `c9d0dd00`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp） 修 Q11 之前的；修了之後 Edit 在 :1120-1159、存檔在 :1162、整段到 :1167） | 一樣，逐行照翻（連 Q11 的缺陷 `:1141` 也照翻了） |
| 1～3 讀出 | 登入 `WebLogin_BookLogin`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:383-470`，解碼條件在 `:455-458`） | 一樣 |
| 4 | 沒翻：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp:512` 的 `TfMain::ChangePassword` 只累加呼叫次數 | 不一樣（還沒做） |
| 5 讀出 | 登入 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:411-439`（fread＋DecodeStr）；名單 `:741-755` | 一樣 |
| 5 寫入 | `security.passwd` 直接擋下，回 `guard:"binary-book"`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:923-939`），只回名單。開機 `ReadPassword` 有做（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:3471`）；關頁時的 `SavePassword` 沒做（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLevelSet.cpp:266`，S64／Q24＝B，排在後面；那是當時的行號，這個檔 commit `855a6a83`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLevelSet.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSecurity.cpp、D:\HT9045\web\page\ht9045_wire_statussecurity.js、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLevelSet.h） 改過，現在跳過 SavePassword 的地方在 :620-624） | 不一樣：golden 是「顯示成功、實際沒改」，這裡直接回報做不到。要等 Q9（S131＝B） |
| 6 | `security.passwd` 下拉模式 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:941-978`（`:967` 存明文、`:968` 寫 login.dat）；登入比對 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:210` | 一樣 |
| 7 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:176-187`（讀取，缺鍵時補寫預設值） | 一樣 |
| 8 | 讀取有做（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:90-91`）；寫入那顆鈕沒翻（在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW`／`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist` 搜 `BitBtn1Click`，0 筆） | 讀取一樣，寫入還沒做 |

網頁傳輸（golden 沒有這一段，是移植樹多出來的環節）：
- C++ → 網頁：回應**一律不含密碼**，明文和 EncodeStr 後的都不送，只送帳號名單、等級和結果碼（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:591`；名單 `:884`）。C++ 不 printf 密碼，頁面也不 console.log（`D:\HT9045\web\page\ht9045_wire_statussecurity.js:392`）。
- 網頁 → C++：使用者輸入的密碼**原字**放進 WS 訊息送出（登入：`D:\HT9045\web\page\ht9045_recipe_client.js:550-551` 的 `auth.login`；改密碼：`D:\HT9045\web\page\ht9045_wire_statussecurity.js:541` 的 `oldPassword`／`newPassword`）。WS 只聽 127.0.0.1（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridge\WebBridgeServer.h:39`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4305`），封包不會離開這台電腦。
- 所以原題「golden 明文、這裡不送不印」其實把兩件事混在一起了：**存檔格式**移植樹已經跟 golden 一樣；**傳輸**是 golden 本來沒有的環節。

**例子**（帳號 op1、等級 1、密碼 1234）：

| 情況 | 檔裡（或封包裡）的位元組 |
|---|---|
| 第 1 列：tech.com 沒加密 | 一行 `op1 1 1234` ＝ `6F 70 31 20 31 20 31 32 33 34 0D 0A` |
| 第 3 列：tech.com 加密模式 | 一行 `op1 1 x^\G` ＝ `6F 70 31 20 31 20 78 5E 5C 47 0D 0A`（只有密碼欄轉換：'1'→'x'、'2'→'^'、'3'→'\'、'4'→'G'） |
| 第 5 列：login.dat（791） | 第 k 格：帳號在位移 4+30k ＝ `26 6F 5E 00 …`（EncodeStr("op1")＝"&o^"）；密碼在位移 30004+30k ＝ `78 5E 5C 47 00 …`；等級在位移 60004+4k ＝ `01 00 00 00` |
| 第 6 列：login.dat（下拉模式，沒有帳號） | Engineer 那一格（第 0 格）：帳號在位移 4，全是 `00`；密碼在位移 30004 ＝ `31 32 33 34 00 …`（明文） |
| 網頁 → C++（現在的做法） | `"oldPassword":"1234"` |

Steven01 這台實測（只讀、只數格子，沒有印出內容）：`D:\HT9045\system\Gerneral.ini:8` `CUSTOMER_CODE=791`，所以走第 5 列。`D:\HT9045\system\login.dat` 64004 位元組，剛好等於結構大小。有 8 格填了帳號：用 DecodeStr 解開後 8 個都是英數字，不解開只有 1 個剛好像英數字。這證明這本確實轉換過，也就是 Steven 說的「經過一個金鑰去做了一次的轉換」。`C:\winnt\system32\tech.com` 不存在。

**Steven 選 B（照 golden）具體要改什麼**：
1. 存檔格式：**不用改**。第 1～3、6、7 列移植樹已經照 golden，該 EncodeStr 的 EncodeStr，golden 寫明文的也寫明文。
2. 第 5 列 login.dat（這台就是）：golden 程式本身沒有真正寫進這本的路（上面講的「顯示成功、實際沒改」）。照 golden 比較合理的做法是：做 Q9（S131＝B，之後支援 login.dat 讀寫）時，**寫出來的位元組要跟 PW_Editor 一模一樣**（帳號、密碼都 EncodeStr，等級照填，總共 64004 位元組）；讀的時候照 golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:15006-15020` 做 DecodeStr。Q9 開工以前，維持現在的 binary-book 擋下。
3. 第 4、8 列（SECS/GEM 遠端改密碼、Configuration 設定 Supervisor 密碼）：照 golden 就是存明文。兩條都還沒翻，翻的時候照 golden 做。
4. 網頁傳輸：golden 沒有對應的東西，「照 golden」可以有兩種讀法，請 Steven 再選一次（見下面的 B1／B2）。

**選項**（Steven 已經選 B，下面是 B 的兩種讀法）：
- **B1**：存檔照 golden（現在已經是），傳輸維持現狀。密碼原字從網頁送進 C++（只走本機 127.0.0.1），C++ 從不把密碼送回網頁。程式不用改，只要把這題的描述改正。
- **B2**：存檔照 golden，傳輸也套用同一把金鑰。網頁送出前先 EncodeStr 並轉成十六進位字串，C++ 收到後先 DecodeStr，再照 golden 流程走。例如 `"oldPassword":"1234"` 會變成 `"oldPassword":"785E5C47"`。好處是封包和除錯紀錄裡看不到原字。限制是金鑰寫在網頁 JS 裡，看得到網頁原始碼的人一樣能還原，只能擋「路過看到」，不是真的加密（真正的加密是 Q8，Steven 已經回 A 不做）。要改的地方：`D:\HT9045\web\page\ht9045_wire_statussecurity.js`（改密碼）、`D:\HT9045\web\page\ht9045_recipe_client.js`（登入用的 authLogin／authSelect），以及 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp` 的三個入口（`WebLogin_BookLogin` `:383`、`WebLogin_Select` `:266`、`W906_SecurityPasswdOp` `:829`）各加一次 DecodeStr；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp` 可以不動。告警框輸入的密碼（`D:\HT9045\web\page\dialog-bridge.js`）要不要一起處理，另外決定。

**St01建議**：B1。存檔已經跟 golden 一致；B2 的金鑰會公開在網頁 JS 裡，保護有限，還要動 3 個別人正在改的檔；而且 Q8 Steven 已經回 A（傳輸不另外加密）。如果 Steven 在 S55 說的「密碼需要加密」指的是網頁送出的那一段，就選 B2。
- 註 1（golden 潛在問題，照翻的移植樹也一樣有）：EncodeStr 對數字是安全的（0～9 不會變成分隔字元），但有些英文字母放在某些位置會變成空白、逗號、冒號、Tab、CR 或 LF。例如密碼第 1 個字是 'i' 會變成空白，是 'C' 會變成換行。加密模式下文字密碼本的那一行就會被切錯（切欄位的程式在 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cpublic.cpp:312-349`），那個帳號就登不進去。
- 註 2：PW_Editor V1.00.648 的 EncodeStr（`D:\HT9045\Password_V1.00.648\password_editor.cpp:183-202`）跟 handler 的（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\common.cpp:267-293`）有一處不同，就是轉出來剛好是 0 的字元（第 k 個字元等於金鑰第 k 個字元＋1，例如第 1 個字是 'I'）。PW_Editor 會寫 00，字串在那裡斷掉；handler 則寫金鑰字元（20210330 修正，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\common.cpp:308`）。移植樹做 Q9 寫 login.dat 時用 handler 版，才讀得回來。這種情況很少見。
- 註 3（不在這題範圍，列給主 session）：網頁告警框送出的密碼（`D:\HT9045\web\page\dialog-bridge.js:486-502` 的 `credentials.password`）在 C++ 端找不到接收的程式（搜 `Dialog-auth`，0 筆）。debug 離線模式下可能被 `D:\HT9045\web\page\json-writer.js` 寫成檔案（該檔檔頭寫明 release 不載入）。

**目前狀態**：**已裁決（Steven 20260927 11:xx，經 github-02 轉述；RULINGS_20260926 S132 定案）：B1**——存檔照 golden（移植樹已是），網頁傳輸維持現狀，程式不改；本題描述已照上面的核對改正。（先前紀錄：Steven 20260927 09:xx 回「B 你仔細看BCB原始碼, 跑lodin.dat時, 雖然是明碼, 但是還是有經過一個金鑰去做了一次的轉換」（RULINGS_20260926 S132）。St01 20260927 已把上面的核對寫好，等 Steven 選 B1／B2。程式沒動。）

### Q11. golden Edit 元件寫回舊密碼的既有缺陷要不要順手修（S55-4／todo 無）
**背景**（缺陷是什麼）：
- 位置：golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSecurity.cpp:756-789`（`ChangePassword` 的 case 2「Edit」）。V899 也有同一個缺陷（`D:\HT9045\HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422\cSecurity.cpp:777`），所以現場所有 BCB6 機台都有。
- 發生條件（三個都要成立）：① 使用**文字密碼本**（`C:\winnt\system32\tech.com`，檔案存在）；② **沒開** `bPasswordSecret`（除了 AMKOR China、RF360、QUALCOMM，其他客戶都沒開。用 `D:\HT9045\system\userid.com` 的 AMKOR China／QUALCOMM 都有開，所以不會碰到）；③ 在 Security 頁按等級鈕，對話框選 **Edit**，輸入帳號、舊密碼、新密碼。
- 程式實際的步驟：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSecurity.cpp:764` 組出「帳號 等級 舊密碼」→ `:767` 在密碼本裡找完全相同的一行（找到就代表舊密碼對）→ `:770` 刪掉那一行 → `:779` 再加回一行「帳號 等級 **舊密碼**」（應該用 `edLoginNewPassword`，這裡寫成 `edLoginOldPassword`）→ `:782` 跳 MES1675 → `:791` 存檔。加密的那一支（`:772-776`）寫的是 `EncodeStr(新密碼)`，是對的，只有沒加密的這一支寫錯。
- 操作員看到的：「修改使用者完成／Modify User is complete.」（MES1675，`D:\HT9045\Error\Chinese\MES1675.dat`）。
- 實際結果：新密碼被丟掉。密碼本內容不變，只是那一行被移到最後。

**例子**：tech.com 原本是
```
admin 3 7777
op1 1 1234
eng1 2 abcd
```
op1 在 Edit 輸入舊密碼 1234、新密碼 5678，畫面顯示「修改使用者完成」，檔案變成
```
admin 3 7777
eng1 2 abcd
op1 1 1234
```
之後 op1 用 5678 登入，會出現「使用者名稱或密碼錯誤」（WAR1677）；用 1234 還是登得進去。

- 移植樹現況：網頁改密碼 `security.passwd` 照翻了，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:1141` 寫入的是 `oldPw`（`:1141-1142` 已有 ⚠ 註解；這是 commit `c9d0dd00`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp） 修改前的行號，修了之後這一支在 :1150 寫 `newPw`）；MFC 驗證用的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSecurity.cpp:868` 也照翻。
- 要修的話改哪裡：把 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:1141` 的 `oldPw` 改成 `newPw`，只改一個字。另外建議同時擋掉「新密碼空白」，回 WAR1678（比照 golden 下拉模式 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSecurity.cpp:802-806`）。原因是 golden 的 Edit 沒檢查新密碼是不是空的；修好以後如果新密碼空白，檔裡會存成「op1 1 」，讀密碼本時第三欄切不到，會沿用上一欄的等級字（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cpublic.cpp:317-320` 切不到時不動輸出），等於密碼變成「1」。golden 加密那一支同樣沒擋，建議一起擋。MFC 的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSecurity.cpp:868` 是驗證用，不一定要跟著改（要改也只是同一個字）。
- 修了之後跟 BCB6 機台會不會不一致：檔案格式完全一樣（還是一行「帳號 等級 密碼」明文），BCB6 讀得懂 V906 改過的密碼本，反過來也讀得懂。不一致的只有「同一顆 Edit 鈕，V906 真的會改，BCB6 不會改」。如果客戶多台機台共用同一本密碼本（例如從網路下載，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:10847`、`:10862` pwName），在 V906 改的新密碼到 BCB6 機台上也會生效，這正是應該有的結果。V912 不改（RULINGS_20260927 第 1 條），只通報 Jimmy。

**選項**：A 照翻不修（跟所有 BCB6 機台一樣，Edit 顯示成功但沒有改）／B 順手修（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:1141` 改成寫新密碼，並擋掉空白新密碼；偏離 golden，但行為正確）。
**St01建議**：B（連同擋空白新密碼），並通報 Jimmy：V899、V912 都有同一個缺陷。
**目前狀態**：**已裁決（Steven 20260927 11:xx，經 github-02 轉述；RULINGS_20260926 S133 定案）：B，已做 `c9d0dd00`**（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp）——Edit 寫新密碼、空白新密碼回 WAR1678（加密、非加密兩支都擋）；V899／V912 同一缺陷只通報 Jimmy。（先前紀錄：Steven 20260927 回「缺陷是什麼? 寫到這文件裡面給我確認」（RULINGS_20260926 S133）。St01 已寫好上面的說明，等 Steven 選 A／B。程式沒動。）

### Q12. 主畫面要不要放 PE 模式按鈕（P8 第1題／todo 無，關聯 E-003）
**背景**：S58 PE 模式目前只做成客戶專屬按鈕（KYEC／ASE-K），沒有放進 `D:\HT9045\web\page\main.html` 主畫面。
**選項**：A 維持客戶專屬、不放主畫面／B 放進主畫面給所有客戶碼用。
**St01建議**：A（照 golden 客戶碼範圍）。
**目前狀態**：**已裁決（Steven 20260927 09:xx，經 github-02 轉述；RULINGS_20260926 S134）**——＝**A**：維持客戶專屬，不放主畫面。（先前紀錄：等決定，沒動。）

### Q13. golden `main.cpp:33726` 守衛疑似寫反要不要照翻（P8 第2題／todo 無）
**背景**：翻譯 PE 模式時發現 golden 這行判斷式看起來邏輯寫反了。golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:33728`（`TfMain::sbPEModelClick` 從 :33726 開始）是 `if(fSecurity->Insufficient(128)==false && HasICUnderMachine()==false) return;`，同一行的註解是「機台內有IC不可切換PE模式 && PE模式權限卡控」。`HasICUnderMachine()`（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\csystem.cpp:13226`）在機台內有 IC 時回 true，所以這一行實際是「沒有權限、而且機台內沒有 IC 才 return」：有權限的人不管機台內有沒有 IC 都切得了，沒權限的人只要機台內有 IC 也切得了，跟註解的意思相反（照註解應該是 `Insufficient(128)==false || HasICUnderMachine()==true`）。移植樹照翻在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp:113-114`（說明註解 :106-112；commit `c913d5e5`：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys_Heater.h、D:\HT9045\web\page\ht9045_hsys_heater_c.js、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.gen.inc 等 8 個檔）。
**選項**：A 照翻 golden（忠實但可能帶入 bug）／B 修正成看起來合理的邏輯（偏離 golden）。
**St01建議**：待確認，需要 Jimmy 在 BCB6 核對 golden 原意後才能判斷。
**目前狀態**：**已裁決（Steven 20260927 09:xx，經 github-02 轉述；RULINGS_20260926 S135）**——＝**A**：照翻 golden，程式註解註明「疑似寫反，待 Jimmy 在 BCB6 確認原意」。（先前紀錄：等決定，沒動。）

### Q14. Heater Type 何時寫檔（P8 第3題／todo 無）
**背景**：TC401 逐通道 Heater 頁目前是按下 `rgHeaterTypeClick` 立刻寫檔，還是要等整頁存檔一起寫，兩種對使用者操作習慣不同。
**選項**：A 按下立刻寫（golden 現行方式）／B 等整頁存檔一起寫。
**St01建議**：A（照 golden）。
**目前狀態**：**已裁決（Steven 20260927 09:xx，經 github-02 轉述；RULINGS_20260926 S136）**——＝**B**（偏離 golden）：等整頁存檔一起寫，按 Heater Type 不立即寫檔。（先前紀錄：等決定，沒動。）

### Q15. 開頁要不要主動補寫 71 個 -9999 缺鍵（P8 第5題／todo 無）
**背景**（這題在講什麼。St01 20260927 重查後，更正原題的前提）：
- 哪個檔：`D:\HT9045\system\Gerneral.ini` 的 `[TempCtrl]` 段。
- 哪些鍵：71 個「每個通道各用哪一種溫控器」的鍵 `HeaterInsOpt_<通道>`，例如 `HeaterInsOpt_HotPlate1`、`HeaterInsOpt_Shuttle1`、`HeaterInsOpt_Socket`，一直到 `HeaterInsOpt_LBDown`（golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\MachineTypeUtility.cpp:30-102`；移植樹同一張表在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.cpp`）。其中 23 個會在 HandlerSys 頁的 Heater 分頁顯示成下拉選單，另外 48 個不顯示（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\MachineTypeUtility.cpp:31-101`，每列第一個參數 true／false）。這 71 個鍵是 V912 才加的，V899 裡沒有（V899 全樹 0 筆）。
- 值的意思：0 TC401、1 Panasonic KT4H、2 Omron E5DC、3 No Heater、4 DTK4848（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\MachineTypeUtility.cpp:19-25`）。**-9999**（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cmydef.h:169` `INVALID_INT_VAL_NEG`）表示「這個通道沒有自己的設定，跟著舊的單一設定 `[TempCtrl] HEATER_CTRL_TYPE` 走」（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\HandlerSys.cpp:57-58`）。
- golden 什麼時候補：`HeaterInsOpt_Read()`（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\HandlerSys.cpp:50-60`）逐鍵用 `CheckAndReadIniDataGeneral` 讀。這支函式遇到缺鍵會**直接把預設值寫進檔案**（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\common.cpp:1444-1455`，TIniFile 立即寫檔），預設值就是 -9999。golden 呼叫它的時機有三個：① 溫控迴圈第一次執行時（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\bthermo.cpp:1165-1175`，等於開機後沒多久就寫）；② 開 HandlerSys 頁時（`FormShow` `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\HandlerSys.cpp:132-135` → `LoaderSystemSet :265`）；③ 按 Load 鈕時（`:1127-1129`）。另外按存檔（`SaveSystemSet :782-788`）會把 71 個鍵全部寫一遍：顯示的 23 個寫下拉選單的值，不顯示的 48 個寫 -9999。
- 移植樹現在什麼時候補：**開頁時就會寫進檔案**，跟 golden 的 ② 一樣。網頁開 HandlerSys 頁的流程是 `editlist.get` → `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.cpp:766-771` `OpenPage` → `HS_FormShow` → `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.gen.inc:3075` `HeaterInsOpt_Read`（本體在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.cpp:96-106`）。用的也是會寫檔的 `CheckAndReadIniDataGeneral`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\common.cpp:1643-1654`；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\vclcompat\IniFiles.h:28-31` 寫入會立即存到檔案）。存檔同樣跟 golden 一樣寫 71 個鍵（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.gen.inc:3647-3658`）。**唯一沒做的是 ① 開機那一次**，那屬於溫控底層，列給 Jimmy（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.cpp:93-95`）。
- 結論：原題寫的「目前是讀到缺鍵才在記憶體補 -9999，不是開頁當下就把 71 個鍵都寫滿檔案」**不對**。移植樹開頁時就會把缺的鍵寫成 -9999，而且只寫缺的，已經有的鍵不動，所以也不會「每次開頁都多寫檔」。真正跟 golden 不同的地方只有「開機時不補」。

**例子**：
- Steven01 這台：`D:\HT9045\system\Gerneral.ini` 第 353～423 行已經有全部 71 個鍵，值全是 -9999；第 311 行 `HEATER_CTRL_TYPE=2`（Omron E5DC），所以 71 個通道都跟著 E5DC。檔案修改時間是 2026-09-24 13:47，比移植樹加進這段程式的 commit `c913d5e5`（2026-09-26 12:26；D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys_Heater.h、D:\HT9045\web\page\ht9045_hsys_heater_c.js、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.gen.inc 等 8 個檔）早，可見這 71 行不是移植樹寫的。最可能是 BCB6 V912 在這台跑過（開機就會寫），但 St01 沒有證據確認是哪一支程式寫的。**所以這台不管選 A 還是 B，開頁時檔案都不會變。**
- 從 V899 升上來的機台（檔裡 0 個 `HeaterInsOpt_` 鍵）：選 A，第一次開 HandlerSys 頁時 `[TempCtrl]` 段會多出 71 行 `HeaterInsOpt_xxx=-9999`，之後再開就不會再變；畫面上 23 個下拉選單都顯示 HEATER_CTRL_TYPE 那個廠牌。選 B，開頁時檔案不變，要等按存檔才寫 71 行（23 個寫下拉選單的值、48 個寫 -9999）。
- 這台按存檔（什麼都不改）：23 個顯示的鍵從 -9999 變成 2，48 個不顯示的維持 -9999。A、B 結果相同，golden 也一樣。

**選項**（照更正後的事實重新列）：
- **A 維持現狀（＝golden 的開頁行為）**：開頁時缺哪個鍵就寫 -9999，已經有的鍵不動（所以只有第一次開頁會真的改到檔案）。開機那一次跟溫控底層一起交給 Jimmy。
- **B 開頁不寫檔**：開頁時只在記憶體裡改用 HEATER_CTRL_TYPE，缺的鍵等使用者按存檔才寫。這樣會偏離 golden：golden 開 HandlerSys 頁時，整頁 215 個 CheckAndReadIniData 讀檔呼叫（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\HandlerSys.cpp:177-574`）都是「缺鍵就補寫」，只改這 71 個會跟同一頁其他鍵不一致。要改的地方是 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.cpp:96-106`（HSys* 目前由別的工程師在改）。還有一點：同一個檔案裡 Q14＝B 的實作（20260927 別的工程師正在改，見 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.cpp` 的 `//AI(W906-FRW-Q14)` 註解）是靠「開頁時已經補好缺鍵」，來保證存檔時只改寫既有的鍵、檔案位元組跟 golden 相同。選 B 會讓這個前提不成立，存檔時會新增鍵。

**St01建議**：A。
（移植樹行號是 20260927 當下的工作樹；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys*` 正有別的工程師在改，行號可能還會位移。）
**目前狀態**：**已裁決（Steven 20260927 11:xx，經 github-02 轉述；RULINGS_20260926 S137 定案）**：「預設選 3 No Heater, 然後B 開頁不寫檔」＋「Q15要跟這個一起做」（跟 Q34）⇒ 71 個 HeaterInsOpt_<通道> 缺鍵時預設值改成 3（No Heater）、開 HandlerSys 頁不寫檔（缺鍵只在記憶體補、按存檔才寫）；**跟 Q34 的新設計一起出方案、一起做**，不單獨先改。（先前紀錄：Steven 20260927 回「這是什麼? 寫到文件裡面給我看」（RULINGS_20260926 S137）。St01 已寫好上面的說明（並更正了原題的前提），等 Steven 選 A／B。程式沒動。）

### Q16. FixAICCD 光源存檔寫 0 的 golden 問題要不要修（S63 交件發現／todo E-001）
**背景**：翻譯 `TfFixAICCD` 存檔時發現 golden 本身怪處：開機後直接存檔會把光源值寫成 0（不是使用者設定的值）。
**選項**：A 照 golden 翻（忠實但保留怪行為）／B 修正（偏離 golden，需 Jimmy 判斷 V912 是否也要一起改）。
**St01建議**：A，並請 Jimmy 判斷 V912 要不要修。
**目前狀態**：**已裁決（Steven 20260927 09:xx，經 github-02 轉述；RULINGS_20260926 S138）**——＝**A**：照 golden 翻；V912 要不要修記給 Jimmy（RULINGS_20260927 第 1 條本專案不動 V912，只記錄）。（先前紀錄：等決定，沒動。）

### Q17. Magazine 開頁要不要照 golden 改寫 MOT 盤面資料（S62 交件發現／todo E-001）
**背景**：golden `TfMagazine` 開頁時會改寫 MOT 盤面資料，移植樹目前還沒有頁面，這步等做頁面時再決定。
**選項**：A 做頁面時比照 golden 改寫盤面資料／B 做頁面時不改寫，只顯示。
**St01建議**：做頁面時再定，不急著現在決定。
**目前狀態**：**已裁決（Steven 20260927 09:xx，經 github-02 轉述；RULINGS_20260926 S139）**——＝**A**：將來做 Magazine 頁時照 golden 改寫盤面資料（做頁面本身仍往後排）。（先前紀錄：等決定，沒動（要不要做網頁本身已列「往後排」）。）

### Q19. Configuration 頁「Soft 速度」／「P26 OCR Tray Lot」兩個欄位現在要不要一起補上（S65 Q1／todo D-007 相關）
**背景**：盤點 `D:\HT9045\system\lastdata.dat` 觸發者時發現 Configuration 頁還有兩個欄位 golden 有、移植樹沒做；跟 S95／S120-1 的關站寫檔時機無關，是單純的欄位補齊問題。
**選項**：A 現在不做／B 現在補上。
**St01建議**：A，等 Configuration 頁後續工作再一起補。
**目前狀態**：**已裁決（Steven 20260927 09:xx，經 github-02 轉述；RULINGS_20260926 S140）**——Soft 速度：**不做**；P26 OCR Tray Lot：**現在補上**。（先前紀錄：等決定，沒動。）

### Q21. SortCT 頁「點兩下清除」要不要維持借用 `act.sortCT.clearCount` 指令通道（S65 Q4／todo D-007 相關）
**背景**：SortCT 頁點兩下清除（golden `D:\HT9045\system\lastdata.dat` 23 個寫入觸發者之一）目前借用既有的 `act.sortCT.clearCount` 指令通道來觸發，沒有另外開一個專屬指令；這題只問要不要開專屬通道，跟 S95／S120-1 的關站寫檔時機無關。
**選項**：A 維持借用現有通道／B 另開專屬指令通道。
**St01建議**：A。
**目前狀態**：**已裁決（Steven 20260927 09:xx，經 github-02 轉述；RULINGS_20260926 S141）**——＝**A**：維持借用現有通道。（先前紀錄：等決定，沒動。）

### Q22. TZteach `[j][j]` 筆誤要不要照翻（S68 D3／todo E-003）
**背景**：翻譯 TZteach 存檔時發現 golden 原始碼有個明顯筆誤（陣列索引 `[j][j]`，看起來應為 `[i][j]`）：golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\AutoTeach\InOutArmZteach.cpp:4741`／`:4743`（`TZteach::SaveSetupFile` 入料臂那一支）寫檔用 `dPick[sel][j][j]`，但前面剛算好存進去的是 `dPick[sel][j][i]`，出料 shuttle 那一支 `:4733`／`:4736` 寫的也是 `[j][i]`；結果只有 i＝j 的格子（A、D 的 PickUp 與 Place）寫對，其他格子寫到的是上一輪留在對角線格子裡的值（commit 本文：16 格裡有 12 格是舊值）。移植樹照翻在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Zteach.cpp:146-151`（檔頭說明 :30-34；commit `58bd9425`：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Zteach.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\AutoCalSuckZ.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\AutoCalSuckZ.gen.inc、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fProductionInfo.cpp 等 11 個檔）。
**選項**：A 照翻 golden 原樣並通報 Jimmy／V912 維護者／B 直接修正筆誤（偏離 golden）。
**St01建議**：A，照翻並通報。
**目前狀態**：**已裁決（Steven 20260927 09:xx，經 github-02 轉述；RULINGS_20260926 S142）**——＝**A**：照翻；程式註解把「筆誤」改寫成「可能因 MTray 元件 [Col][Row] 的慣例而刻意如此，待 Jimmy 確認」。（先前紀錄：等決定，沒動。）

### Q23. 兩個 `uPoint2D` 定義放哪（S68／S70 D4／todo E-001／E-003）
**背景**：TZteach／AutoCalSuckZ 各自定義了同名的 `uPoint2D` 型別，純架構問題，不影響行為。
**選項**：A 放共用標頭檔，兩處都改成引用它／B 維持各自獨立定義（重複但互不影響）。
**St01建議**：B，風險小，之後真的衝突再合併。
**目前狀態**：**已裁決（Steven 20260927 09:xx，經 github-02 轉述；RULINGS_20260926 S143）**——＝**B**：維持各自獨立定義。（先前紀錄：等決定，沒動。）

### Q24. golden `SavePassword`／`ReadPassword` 要不要照跑（S64 七題之一／todo 無）
**背景**：golden 這兩支函式在 `D:\HT9045\system\levelset.dat` 存檔流程裡，翻譯時討論要不要照樣呼叫。
**選項**：A 不跑（維持現狀）／B 照 golden 呼叫。
**St01建議**：A。
**目前狀態**：**已裁決（Steven 20260927 09:xx，經 github-02 轉述；RULINGS_20260926 S144）**——＝**B**：照 golden 呼叫，但不急、排後。（先前紀錄：等決定，沒動。）

### Q25. 每次存檔前要不要重算 `Insufficient(29)`（S64／todo 無）
**背景**：golden 存檔前會重算這個值，決定移植樹要不要照做。
**選項**：A 每次存檔前重算（照 golden）／B 不重算，維持既有值。
**St01建議**：A。
**目前狀態**：**已裁決（Steven 20260927 09:xx，經 github-02 轉述；RULINGS_20260926 S145）**——＝**A**：每次存檔前重算（照 golden）。（先前紀錄：等決定，沒動。）

### Q26. 停用格子被改到要不要整批拒寫（S64／todo 無）
**背景**：使用者若動到已停用的權限格子，要決定整批存檔動作要不要因此被拒絕。
**選項**：A 整批拒寫（安全，但使用者可能不清楚原因）／B 只擋停用格子本身，其餘正常存。
**St01建議**：A。
**目前狀態**：**已裁決（Steven 20260927 09:xx，經 github-02 轉述；RULINGS_20260926 S146）**——＝**B**（跟 St01 原建議相反）：只擋停用格子本身，其餘正常存。（先前紀錄：等決定，沒動。）

### Q27. 備份驗證通過就刪、11 份舊備份要不要清（S64／todo 無）
**背景**：`D:\HT9045\system\levelset.dat` 存檔會先備份、驗證新檔沒問題再決定要不要刪備份；機器上已累積 11 份舊備份。
**選項**：A 驗證通過就刪這次的備份，11 份舊備份另外問 Steven 要不要一次清掉／B 全部保留不刪。
**St01建議**：A（新備份的部分）；11 份舊的要 Steven 點頭才清，避免誤刪還有用的備份。
**目前狀態**：**已裁決**（RULINGS_20260926 S147＝A；11 份舊備份 S150：Steven「要得」）——St01 已處理：`D:\HT9045\system\levelset.dat.bak_20260916_*` 共 11 份搬到 St01 scratch 的封存資料夾（搬前搬後 SHA256 逐檔相符）；`D:\HT9045\system\levelset.dat` 本體、`D:\HT9045\system\levelset backup.dat`、`D:\HT9045\system\levelset.dat.sjsonbak` 沒動。Steven 要永久刪時再刪。

### Q28. 權限頁還差的重排與 5 階 radio 要不要現在做（S64／todo 無）
**背景**：權限畫面還有兩個子功能（`SecurityPalVisible` 重排、5 階角色 radio）沒做，工程師建議另開項目處理。
**選項**：A 另開一個項目後續處理／B 現在一起做完。
**St01建議**：A。
**目前狀態**：**已裁決（Steven 20260927 09:xx，經 github-02 轉述；RULINGS_20260926 S148）**——＝**B**（跟原建議相反）：現在一起做完。（先前紀錄：等決定，沒動。）

### Q29. levelset.dat 檔案不在或大小不對要不要拒寫（S64／todo 無，偏離 golden）
**背景**：golden 沒有這個檢查，工程師建議加一道「檔案不在或大小不對就拒絕寫入」的保護，這是主動偏離 golden、變嚴格。
**選項**：A 加這道保護（偏離 golden，更安全）／B 完全照 golden，不加檢查。
**St01建議**：A。
**目前狀態**：**已裁決（Steven 20260927 09:xx，經 github-02 轉述；RULINGS_20260926 S149）**——＝**A**：加這道保護（偏離 golden，更安全）。（先前紀錄：等決定，沒動。）

### Q30. `FormClose`／`SecurityExitClick` 潛在越界點要不要接（S64／todo 無）
**背景**：golden 這兩個函式裡有幾個潛在陣列越界風險點，移植樹目前選擇不接。
**選項**：A 維持不接（風險留在 golden 裡）／B 接上但先修好越界風險。
**St01建議**：A。
**目前狀態**：**已裁決（Steven 20260927 09:xx～10:xx，經 github-02 轉述；RULINGS_20260926 S151）**——＝**B**（跟原建議相反）：接上，但先修好越界風險。派工中（跟 Q25～Q29 同一位）。（先前紀錄：等決定，沒動。）

### Q31. golden 兩個既有錯誤要不要修（S69／todo E-001）
**背景**：翻譯 AOI.Data（配方檔 `D:\HT9045\IniData\Data\<配方>\AOI.Data`）讀寫時發現 golden 有兩處既有資料錯置（Top 延遲值寫進 Bottom 鍵、`TimeOut` 鍵名讀寫不一致）。
**選項**：A 照翻 golden（忠實但保留兩個錯誤）／B 請 Jimmy 在 V912 一併修正（偏離 golden）。
**St01建議**：請 Jimmy 在 V912 修，這樣兩邊資料格式才會一致。
**目前狀態**：**已裁決（Steven 20260927 11:xx，經 github-02 轉述；RULINGS_20260926 S152 定案）：A'**——906 C++ 單邊修正 AOI.Data 那兩個 golden 既有錯誤，接受 AOI.Data 格式跟 BCB6 機台不一致；V912 不改，只記給 Jimmy。**已做 `725038a6`**（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\AOISetup.py`、重產 `FileRW\AOISetup.gen.inc`，只改寫檔兩行，讀檔照 golden）；鍵名選擇見 R71；AOI 頁接上後才生效。（先前紀錄：**Steven 20260927 09:xx～10:xx，經 github-02 轉述回「B」，但 B 的前提（V912 一起修）因 RULINGS_20260927 第 1 條不成立 ⇒ 待 Steven 重選**（RULINGS_20260926 S152）——**待 Steven 重選**：原選項 B＝請 Jimmy 在 V912 一併修正（兩邊資料格式一致），但 RULINGS_20260927 第 1 條不動 V912，前提不成立；github-02 已請 Steven 在 A（照翻，兩邊一致）／A'（906 單邊修，格式跟 BCB6 機台不一致）之間重選。St01 建議 A。（先前紀錄：等決定，沒動。））

### Q32. Top&Bottom 70 多個參數要不要先做純資料結構（S69／todo E-001）
**背景**：`TTopBottomInspect`（Top&Bottom 兩頁）還有 70 多個參數沒移植。
**選項**：A 現在先做純資料結構（頁面之後再說）／B 等 Jimmy 那邊的相關流程一起做。
**St01建議**：B，等 Jimmy。
**目前狀態**：**已裁決（Steven 20260927 09:xx～10:xx，經 github-02 轉述；RULINGS_20260926 S153）**——＝**A**（跟原建議相反）：現在先做純資料結構（頁面之後）；建議交 Steven02 做成新檔（golden TTopBottomInspect），St01 之後在 AOISetup 接讀寫。（先前紀錄：等決定，沒動。）

### Q34. HandlerSys 逐通道 Heater 廠牌與「底層照 906」怎麼搭（P8 第 4 題／RULINGS_20260926 第 26 條、S154；Q15＝S137 併入／todo 無）

**背景**：
- 畫面（golden V912）：HandlerSys 頁的 Heater 分頁可以把 71 個溫控通道各設一個廠牌（0 TC401、1 Panasonic KT4H、2 Omron E5DC、3 No Heater、4 DTK4848），存在 `[TempCtrl] HeaterInsOpt_<通道>` 71 個鍵，其中 23 個有下拉、48 個不顯示（golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\MachineTypeUtility.cpp:19-102`）。開頁讀 71 鍵（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\HandlerSys.cpp:50-60`、`:262-278`），存檔寫 71 鍵＋`HEATER_CTRL_TYPE`（同檔 `:779-794`；看不見的 48 個一律寫 -9999，`:783-788`）。
- 底層（golden V912）：溫控迴圈逐通道判斷廠牌（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\bthermo.cpp:1167-1178`、`:2520-2598`、`:3215-3219`；`rs232.cpp:742-760`；`cConfiguration.cpp:5591-5760`；`EJ1N\OmronEJ1N.cpp:622-623`）。**站號是寫死的**：KT4H／DTK4848／E5DC 站號＝通道序號＋1（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cpublic.cpp:185`、`:196`、`:210-226`、`:467`、`:484`），TC401 是「第 (序號÷4)+1 台、通道 序號%4」（`bthermo.cpp:2522`、`:2556`；`cpublic.cpp:422-461`）。golden 沒有任何「指定站號」的設定。所有廠牌共用同一個溫控 COM 埠、固定 9600 8N1（`rs232.cpp:268-273`）。
- 移植樹現況（第 26 條「底層照 906、畫面照 912」）：頁面讀寫已照 912（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.cpp:96-106`、`:813-834`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.gen.inc:3072-3091`、`:3644-3659`，頁面 `D:\HT9045\web\page\ht9045_hsys_heater_c.js`）；溫控流程只看單一廠牌 `TC401HeaterControl`＝`HEATER_CTRL_TYPE`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\database.cpp:541`；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\bthermo.cpp:1264`、`:2663-2698`、`:2725-2760`、`:2775-2790`、`:3503`）。而且**溫控迴圈目前在 wb_serve 根本沒有跑**：沒有人建 HeaterThread（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\StageThermo.cpp:30-42`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp:877-879`），TC401／KT4H／E5DC 的通訊函式還在 `#if 0`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cpublic.cpp:292-349`、`:581-649`，只有 DTK4848 是活的），溫控 COM 埠那一半的 rs232 也還沒翻（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\rs232.cpp:243-255` 列出 golden `:163-182` 埠檢查與 `:264-281` Comm2 開埠都沒翻）；另外 DTK4848 雖然有編進去，但它用的 `g_pDTKComm` 在 wb_serve 是 nullptr（`cpublic.cpp:157`，只有 `tests\test_cpublic_foundation.cpp` 會設），一樣送不出去。所以今天頁面怎麼存，機台的溫控都不受影響。
- Steven 20260927 的新設計（S154）：「舊機台使用的是全機同一個品牌，但是新的機器可能混搭，要改多個變數配合：先選相同或不同；相同＝要選 Index 的溫控器與其他位置的溫控器；不同＝每個溫控器單獨設定，並且可以指定站號」。Q15（S137）：「預設選 3 No Heater，然後 B 開頁不寫檔」＋「Q15 要跟這個一起做」。Q14（S136）＝B：按 Heater Type 不立刻寫檔，等整頁存檔。

**選項**（A／B／C 是原本的三個，留著對照；D 是 Steven 的新設計）：
- A 維持（頁面照 912 逐通道設、存檔照寫 71 鍵；溫控仍只看 HEATER_CTRL_TYPE，頁面加一行提示「溫控只看總設定」）。
- B 頁面限制成全部同一廠牌（跟 906 底層一致，但偏離 912 畫面）。
- C 等 Jimmy 把逐通道溫控翻進底層再說。
- **D 新設計（相同／不同兩種模式＋站號）**，內容如下。

**D 的內容**

① 畫面（HandlerSys 頁「Heater」分頁，移植樹 `D:\HT9045\web\page\HW.HandlerSys.html` 的 grpHeater，由 `D:\HT9045\web\page\ht9045_hsys_heater_c.js` 建）：
- 最上面一列「溫控器廠牌」：○ 全機相同　○ 各溫控器不同。
- 選「全機相同」：兩個下拉——「Index 位置溫控器」與「其他位置溫控器」，選項都是 TC401／Panasonic KT4H／Omron E5DC／No Heater／DTK4848。「Index 位置」包含哪些通道見 D-2，旁邊用灰字列出。`USE_16_HEATER` 是 EJ1N 或 DTME08 的版本時，Index 那個下拉停用、顯示「Omron EJ1N（依 Index Heater Counts）」或「DTME08」——那些 Index 區不走溫控 COM 埠，golden 溫控迴圈直接跳過（golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\bthermo.cpp:1331-1367`）。下面的逐通道表變唯讀，只顯示每個通道算出來的廠牌與預設站號，方便核對。
- 選「各溫控器不同」：逐通道表可以改，每列：名稱｜廠牌下拉｜站號（數字框；空白＝預設，灰字顯示 golden 算的預設值）。列哪些通道見 D-3；TC401 的站號規則見 D-7。
- 「Index Items」分頁原本的 Heater Type 單選（rgHeaterType，golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\HandlerSys.dfm:2970-2991`）保留：點一下＝「全機相同，Index 與其他都設成這個廠牌」，也就是 golden `rgHeaterTypeClick`（`HandlerSys.cpp:1519-1540`）把全部通道設成同一廠牌的意思；照 Q14＝B 只改畫面，存檔才寫。
- 存檔前檢查：同一個溫控 COM 埠上站號重複（D-8）；「不同」模式或 Index≠其他時，提示底層還沒支援（D-9）。

② 檔案格式（都在 `D:\HT9045\system\Gerneral.ini` 的 `[TempCtrl]` 段，這是量產機共用的真實檔）：

| 鍵 | 值 | 誰讀 | 新／舊 |
|---|---|---|---|
| `HEATER_CTRL_TYPE` | 0～4 | 全部版本。BCB6 V899／V906 只讀這一個；V912 與移植樹拿它判斷「整台沒有加熱」（`TC401HeaterControl==NoHeater`）並當 -9999 的回退值 | 舊，意思不變，寫什麼見 D-6 |
| `HeaterInsOpt_<通道>`（71 個） | 0～4，或 -9999 | V912 逐通道；新程式「不同」模式 | 舊（V912），兩種模式都寫「這個通道實際的廠牌」（D-4） |
| `HeaterInsMode` | 0＝全機相同、1＝各溫控器不同 | 只有新程式 | 新 |
| `HeaterInsIndexOpt` | 0～4 | 只有新程式，「相同」模式的 Index 廠牌 | 新 |
| `HeaterInsOtherOpt` | 0～4 | 只有新程式，「相同」模式的其他位置廠牌 | 新 |
| `HeaterInsAddr_<通道>` | 站號（1 起算，就是溫控器上撥的號碼） | 只有新程式，「不同」模式 | 新；缺鍵＝golden 算的預設站號，行為跟今天一樣 |

- 不沿用 `HEATER_CTRL_TYPE` 當「其他位置廠牌」：「其他位置＝No Heater、Index＝KT4H」時它會變成 3，V912 與移植樹就把整台當成沒有加熱（golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\bthermo.cpp:1177`、`rs232.cpp:165`；移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\bthermo.cpp:1264`、`uTemp_Set.cpp:3249`、`cObserver.cpp:3831`），連 Index 都不加熱。所以另開兩個鍵。
- 讀檔規則：模式＝`HeaterInsMode`；缺這個鍵時照 D-1 判斷。「相同」模式時每個通道的廠牌＝它屬於 Index 組就用 IndexOpt、否則用 OtherOpt，71 個 `HeaterInsOpt_` 只當 V912 的相容副本，不拿來控制。「不同」模式時每個通道讀自己的 `HeaterInsOpt_<通道>`：**缺鍵＝3 No Heater（Q15）**；-9999 照 D-5。

③ 舊檔拿到新程式、新檔拿回 BCB6 機台：
- V899 升上來的檔（0 個 `HeaterInsOpt_`、沒有模式鍵）→ 判成「相同」，Index＝其他＝`HEATER_CTRL_TYPE`，跟今天完全一樣；開頁不寫檔。第一次按存檔（答「是」）才在 `[TempCtrl]` 段尾多出 3 個模式／組別鍵＋71 個 `HeaterInsOpt_`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\vclcompat\IniFiles.cpp:500` 插在該段最後一個鍵後面，BCB6 的 WritePrivateProfileString 也是加在段尾）。
- V912 寫過、71 鍵全是 -9999 的檔（例如 Steven01 這台）→ -9999 跟著 `HEATER_CTRL_TYPE`，全部同一廠牌 → 判成「相同」，跟今天一樣。
- V912 寫過、各通道真的不同的檔 → 判成「不同」（D-1），照 71 鍵讀；站號用預設。
- 新程式寫的檔拿到 BCB6 V912 機台：V912 不認得的新鍵直接忽略；它照 71 鍵逐通道跑，因為 D-4 寫的是實際廠牌，**Index 與其他不同廠牌在 V912 也會照樣生效**。兩個例外：(1) 指定過的站號 V912 看不到，仍送到「序號＋1」那一站 → 那個通道通訊失敗、溫度顯示 999（golden V912 `bthermo.cpp:2709-2715` 的通訊錯誤處理）；(2) 在 V912 上開 HandlerSys 頁再按存檔，V912 會把看不見的 48 個通道寫回 -9999（`HandlerSys.cpp:783-788`）——Index 32 區（Aa1～Bh2）全在這 48 個裡，它們就會掉回 `HEATER_CTRL_TYPE` 的廠牌。
- 新程式寫的檔拿到 BCB6 V899／V906 機台：只讀 `HEATER_CTRL_TYPE`，全機一個廠牌（寫什麼見 D-6）。

④ Q15 併入：缺鍵預設 3、開頁不寫檔：
- 開頁（網頁 `editlist.get` → 移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.cpp:767-772` OpenPage → `HS_FormShow` → `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.gen.inc:3075` → 本體 `FileRW\HSys.cpp:96-106`）：`HeaterInsOpt_Read` 改用不寫檔的讀法（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\common.cpp:634` CheckIniData＋`:851` ReadIniData，取代會補寫的 `CheckAndReadIniDataGeneral`，`common.cpp:1643-1654`）；缺的鍵只在記憶體補，71 鍵缺鍵＝3，新鍵缺鍵照上面的讀檔規則。`HEATER_CTRL_TYPE` 開機時 ReadGeneralIni 就已補好（移植樹 `database.cpp:541`），開頁不會再寫到它。
- 範圍：只有這 71 鍵與新鍵不寫。同一頁其他 215 個讀檔呼叫照 golden，缺鍵仍然開頁就補寫（golden V912 `HandlerSys.cpp:177-574`）。這是 Steven 選 B 時已知的偏離。
- 底層第一次跑溫控時（golden V912 `bthermo.cpp:1167-1175` 會呼叫會寫檔的 `HeaterInsOpt_Read`）也改用同一支不寫檔的讀法，所以開機也不寫這些鍵。

⑤ 跟 Q14＝B 怎麼搭：
- 按 Heater Type 單選：照現在的重播機制（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.cpp:813-834` BeforeApply；產生器 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\HSys.py:140-150` 把 golden `:1536`／`:1538` 的寫檔拿掉）只改記憶體與畫面，意思改成「模式＝相同、Index＝其他＝這個廠牌」。
- 存檔（答「是」＝`SaveSystemSet:write`）才寫全部：`HEATER_CTRL_TYPE`、`HeaterInsMode`、`HeaterInsIndexOpt`、`HeaterInsOtherOpt`、71 個 `HeaterInsOpt_`，「不同」模式另寫站號鍵（D-7）。golden 的存檔段（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.gen.inc:3644-3659`，對 golden V912 `HandlerSys.cpp:779-794`）用產生器取代段換成新規則。答「否」或套值失敗：什麼都不寫，記憶體由 reload（再跑一次開頁，也不寫檔）還原。
- 影響：Q14 現在的說法「開頁已補好缺鍵，存檔只改寫既有的鍵、位元組跟 golden 相同」（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.cpp:807-808`）不再成立：第一次存檔會新增鍵，而且新鍵本來就是 golden 沒有的，D 的存檔內容刻意跟 golden 不同（新鍵、D-4、D-6）。那段註解要跟著改。
- 記憶體：golden 存檔時把沒有下拉的 48 個通道（occupy=false；有下拉但被 GetCtrlItemVisProp 藏起來的照樣寫下拉值，golden `HandlerSys.cpp:76-86`）的記憶體設成 -9999（`HandlerSys.cpp:787`），之後重讀設定也不會讀回它們（golden V912 `database.cpp:433` 只讀 `HEATER_CTRL_TYPE`、`HandlerSys.cpp:1179-1185` ExitBtnClick），V912 在重開頁或重開程式之前，這 48 個通道對每個廠牌的判斷都是 false。D 存檔時記憶體直接放「實際廠牌」，不留 -9999。

⑥ 底層要改哪裡（移植樹；歸 Jimmy）：
1. 移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\bthermo.cpp:1264`：照 golden V912 `bthermo.cpp:1167-1178` 加「第一次執行時讀逐通道表」（用 St01 給的不寫檔讀法，Q15），整台 No Heater 的判斷改成 `IsNoHeaterMachine()`（仍看 `TC401HeaterControl`，移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.cpp:218-221`）。
2. 移植樹 `bthermo.cpp:2663-2698`（送設定溫度）、`:2725-2760`（讀溫度）、`:2775-2790`（選下一個 Task）、`:3503`（KT4H 警報值）：`TC401HeaterControl==X` 換成 `IsValEqual_HeaterInsOpt(Addr, X)`，照 golden V912 `bthermo.cpp:2520-2598`、`:3219`；`:3497` 換通道後加 `g_iHeaterTypeIdx_SendCmd=Addr`（golden `:3215`）。
3. **站號**：通訊函式目前用「傳進來的序號＋1」當站號（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cpublic.cpp:332`、`:343`、`:359-377`、`:589`、`:609`、`:627`、`:644`）。建議 bthermo 呼叫時傳「站號−1」（St01 提供一支 `W906_HeaterStationIdx(Addr)`，沒設站號時就回 Addr，跟今天一樣；TC401 傳 `(站號−1, Addr%4)`），通訊函式本身不動；`OldTemp[Addr]`、`UN150Read[Addr]` 這些陣列仍用通道序號。
4. **No Heater 通道會卡住整個溫控迴圈**（golden V912 同樣有這個問題）：golden 的廠牌分派沒有 else（golden V912 `bthermo.cpp:2520-2598`），一個已安裝（`bUT150Install[Addr]==true`）的通道若是 No Heater 或 -9999，四個廠牌都不符 → 不送命令、`Task` 停在 100 → 下一拍又是同一個通道 → **後面所有通道都不再輪詢**，溫度停在最後一次的值。Q15 把缺鍵改成 3 以後，這個情況會變多。建議底層加「廠牌不是 0～4 的有效溫控器（含 3）就 `Task=300` 換下一個通道」——偏離 golden，由 Jimmy 決定；V912 同一個缺陷只通報（RULINGS_20260927 第 1 條不改 V912）。
5. 溫控 COM 埠的接收（golden V912 `rs232.cpp:742-760` 依目前通道的廠牌決定用 TC401 的 8 byte 二進位還是 ASCII 解）：移植樹 rs232 那一半還沒翻（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\rs232.cpp:243-255`），翻的時候照 912 逐通道。
6. Configuration 頁手動設溫／讀溫（golden V912 `cConfiguration.cpp:5591-5760` UpdateUT150Comm）與 Omron EJ1N（golden V912 `EJ1N\OmronEJ1N.cpp:622-623`）：移植樹都還沒有（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fConfiguration.h:2036`；移植樹 `EJ1N\` 沒有 OmronEJ1N.cpp），翻的時候照 912。
7. 逐通道表搬家：目前表與小工具只在 HandlerSys 的 TU 裡（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys_Heater.h:16-25`），Jimmy 翻 bthermo 時照 golden 搬到 MachineType.h＋MachineTypeUtility.cpp（ht9045_globals），D 的新全域（模式、兩個組別廠牌、71 個站號、不寫檔讀法）一起搬。
8. 前提（不在 D 裡，另外估）：溫控迴圈要先在 wb_serve 跑起來（HeaterThread、`cpublic.cpp` GA1-B3 兩段閘、rs232 溫控那一半）。在那之前，D 只影響畫面與檔案。

**「預設 3」在底層的風險**（白話）：缺鍵的通道會被當成「沒有溫控器」——不加熱、也不讀溫度。(a) 在「相同」模式與 D-1 的推斷下，從 V899 升上來、71 鍵全缺的機台會判成「相同」，不會整台變成不加熱；(b) 真正會用到「缺鍵＝3」的只有「不同」模式裡某個鍵不見了（手動刪掉、或之後 golden 新增通道），那個通道會安靜地不加熱，畫面上會顯示 No Heater；(c) 底層若沒做第 4 點，那個通道還會把整個溫控迴圈卡住。

**D 裡要 Steven 選的細節**（沒寫到的照建議）：
- **D-1 檔案沒有 `HeaterInsMode`（舊檔）時算哪一種**：a 一律「相同」／**b 看 71 鍵推斷**：有顯示下拉的 23 個通道裡，值在 0～4 的彼此不同就算「不同」，否則「相同」（-9999 先換成 `HEATER_CTRL_TYPE` 再比）。建議 b（V912 已經混搭的機台不會被壓成單一廠牌）。
- **D-2 「Index 位置」包含哪些通道**：**a Head1～4＋Index 32 區（Aa1～Bd2、Ae1～Bh2，golden V912 `MachineType.h:638-645`），共 36 個**／b 再加 Socket、DUT1～4、IndexESD、Door1～2。建議 a（就是「Index Heater Counts」那個選項管的通道；b 那幾個請 Steven 或 Jimmy 依機構確認）。
- **D-3 「不同」模式列哪些通道**：**a golden 的 23 個有下拉的通道（照 golden 可見條件）＋依 `USE_16_HEATER` 實際走溫控 COM 埠的 Index 區**（16 Heaters：Aa1～Bd2；32 Heaters with KT4H：Aa1～Bh2）／b 71 個全列。建議 a（列出來的都是這台真的會輪詢的）。
- **D-4 沒有下拉的 48 個通道存檔寫什麼**：**a 寫實際廠牌**／b 照 golden 寫 -9999。建議 a（V912 機台才讀得到 Index 32 區的廠牌；代價是跟 golden 存出來的檔不同）。
- **D-5 檔案裡已經是 -9999 的通道（V912 或移植樹以前開頁補的）**：**a 照 V912＝跟著 `HEATER_CTRL_TYPE`**／b 跟缺鍵一樣當 3。建議 a（Steven01 這台 71 個全是 -9999，選 b 的話切到「不同」模式會全部變成 No Heater）。
- **D-6 `HEATER_CTRL_TYPE` 存檔寫什麼**：**a 全部通道都是 No Heater 才寫 3；否則寫「其他位置」的廠牌，其他位置是 No Heater 時寫 Index 的廠牌**（「不同」模式寫用最多通道的那個廠牌）／b 照 golden（有下拉的通道全同就寫那個值，否則寫 Heater Type 單選的值）。建議 a（不會因為「其他＝No Heater」把整台變成不加熱）。
- **D-7 站號**：**a 範圍 1～247；E5DC 1～99（golden 用十進位兩位數組命令，`cpublic.cpp:467`、`:484`）；TC401 只指定站號，通道仍是序號%4；站號鍵只在「不同」模式存檔時寫（列出的通道都寫，沒改＝預設值）**／b TC401 另外可指定通道；c 站號鍵每次存檔 71 個全寫。建議 a。
- **D-8 站號重複**：**a 存檔時同一個溫控 COM 埠上兩個通道同廠牌同站號就擋下，列出是哪兩個**／b 只提示。建議 a（兩台溫控器同站號，回覆會打架，讀到的溫度可能是另一台的）。
- **D-9 Jimmy 的底層還沒做完之前**：**a 頁面照樣可以設、可以存，但「不同」模式、Index≠其他、或改過站號時，頁面顯示「目前溫控只看 HEATER_CTRL_TYPE（單一廠牌、預設站號），這些設定要等底層翻完才生效」**／b 底層好之前「不同」與 Index≠其他選不了。建議 a（檔案先準備好，也能拿去 V912 機台用）。

**St01建議**：D，細節照 D-1b、D-2a、D-3a、D-4a、D-5a、D-6a、D-7a、D-8a、D-9a。
- 工作量：St01（頁面與讀寫檔）約 1.5 天——讀寫檔與記憶體表（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.cpp`、`FileRW\HSys_Heater.h`、`tools\editlist\HSys.py` 取代段、重產 `FileRW\HSys.gen.inc`）約半天；頁面（`D:\HT9045\web\page\ht9045_hsys_heater_c.js` 重做、`FileRW\HSys.cpp:839-885` ExtraJson 加欄位）約半天；ctest（V899 檔、V912 全 -9999 檔、V912 混搭檔、新檔來回；開頁不寫檔；存檔內容逐鍵比對；一律用暫存檔，不碰 `D:\HT9045\system\Gerneral.ini`）＋兩組態 gate 約半天。Jimmy（底層）：⑥ 第 1～4 點約 1 天，但要等第 8 點的前提（溫控迴圈接起來）一起做，那一塊另外估。
- 順序：① Steven 確認 D-1～D-9 → ② Jimmy 看新鍵名與 D-2 的 Index 通道清單 → ③ St01 做讀寫檔＋頁面＋ctest（底層不動，頁面照 D-9 提示）→ ④ Jimmy 在溫控迴圈接起來那一波做 ⑥ 第 1～7 點 → ⑤ St01 拿掉 D-9 的提示 → ⑥ 機邊驗證（要有人在機台旁，確認每個通道的站號與廠牌都回得了溫度）。

**例子**：
- **Steven01 這台**（`D:\HT9045\system\Gerneral.ini`：第 19 行 `USE_16_HEATER=2`＝16 Heaters with Omron EJ1N；第 311 行 `HEATER_CTRL_TYPE=2`＝Omron E5DC；第 353～423 行 71 鍵全是 -9999；沒有模式鍵）：開頁判成「相同」；Index 下拉停用、顯示「Omron EJ1N（依 Index Heater Counts）」，其他位置＝Omron E5DC；檔案不變。什麼都不改直接存檔：多 3 行（`HeaterInsMode=0`、`HeaterInsIndexOpt=2`、`HeaterInsOtherOpt=2`）；71 鍵裡有下拉的 23 個 -9999→2（golden 也是），看不見的 48 個 -9999→2（D-4a；golden 維持 -9999）；`HEATER_CTRL_TYPE` 仍是 2。
- **新機台混搭**（`USE_16_HEATER=1`＝16 Heaters，Index 的 Head1～4 與 Aa1～Bd2 走溫控 COM 埠）：選「相同」，Index＝Panasonic KT4H、其他＝Omron E5DC，存檔 → `HeaterInsMode=0`、`HeaterInsIndexOpt=1`、`HeaterInsOtherOpt=2`、`HeaterInsOpt_Head1`～`Head4`＝1、`HeaterInsOpt_Aa1`～`Bd2`＝1、`HeaterInsOpt_HotPlate1`＝2…、`HEATER_CTRL_TYPE=2`。拿到 BCB6 V912 機台：Index 用 KT4H、其他用 E5DC，正常；但在那台 V912 開 HandlerSys 頁存一次，Aa1～Bd2 會被寫回 -9999、變成 E5DC，Index 16 區就讀不到溫度。拿到 V899／V906 BCB6 機台：全部當 E5DC。移植樹在 Jimmy 做完之前：也是全部當 E5DC，頁面顯示 D-9 的提示。
- **「不同」模式加站號**：Shuttle1 選 DTK4848、站號 21（預設是 3）→ 存檔 `HeaterInsMode=1`、`HeaterInsOpt_Shuttle1=4`、`HeaterInsAddr_Shuttle1=21`。拿到 V912 機台：V912 還是送到第 3 站 → Shuttle1 溫度 999（通訊錯誤）。
- **從 V899 升上來的機台**（0 個 `HeaterInsOpt_`）：開頁畫面顯示「相同」、兩組都是 `HEATER_CTRL_TYPE` 的廠牌，檔案不變（Q15 B）；按存檔才在 `[TempCtrl]` 段尾一次加 74 行（3＋71）。
- **Q15 的缺鍵＝3**：「不同」模式的檔裡 `HeaterInsOpt_CCD_2` 被人手動刪掉 → 開頁 CCD_2 顯示 No Heater；Jimmy 的底層做完之後，那個通道不加熱、不讀溫度（要有 ⑥ 第 4 點，否則會卡住整個溫控迴圈）。

**目前狀態**：**已裁決（RULINGS_20260926 S166）：方案 D 開工**——Steven 20260927 18:1x（對話中直接回覆 ST01-E）：「開工，其他照建議」；D-2＝A（RULINGS_20260927 第 7 條第 35 題，Steven 0927 17:1x「依據建議」）：Index 位置＝36 個通道（Head1～4＋Index 32 區，golden V912 MachineType.h:638-645），Socket、DUT1～4、IndexESD、Door1～2 算其他位置；D-1＝b、D-3＝a、D-4＝a、D-5＝a、D-6＝a、D-7＝a、D-8＝a、D-9＝a 照建議；⑥ 第 4 點（No Heater 通道讓迴圈跳過）**不做**（RULINGS_20260927 第 7 條第 34 題：照 golden 不加 else）。Q15（S137）併入一起做。St01 做讀寫檔＋頁面＋ctest；底層 ⑥ 1～3、5～8 歸 Jimmy（溫控迴圈那一波）。（先前紀錄：**Steven 20260927 選新設計（D）**（RULINGS_20260926 S154；ST01-E 20260927 轉述 Steven「要使用新的方案, 把方案列出來成為選項D」），**等 Steven 確認 D-1～D-9 的細節才開工**；Q15（S137「預設選 3 No Heater，然後 B 開頁不寫檔」）併入 D 一起做，不單獨先改。程式沒動（移植樹目前的行為仍是 A，還沒加提示）。　**Jimmy 回覆（TO_STEVEN 20260927 16:2x 第 169 列）**：新鍵名 OK；D-2（Index 位置包含哪些通道）要機構確認（他們的第 35 題）；No Heater 通道跳過是偏離 golden，他們建議選 B＝跳過（第 34 題）；底層 ⑥ 1～8 跟溫控迴圈那一波一起做。）　**St01 部分已做 `bc970c38`**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.cpp` 第 (4) 段、`tools\editlist\HSys.py`、`D:\HT9045\web\page\ht9045_hsys_heater_c.js`、ctest HSys_HeaterMix 105/105）；衍生 R111～R114；底層 ⑥ 1～3、5～8 歸 Jimmy。
（行號是 20260927 移植樹工作樹 `v906/steven-cbridge-review6` 當下的；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys*` 別的工程師也在改，行號可能位移。）

### Q35. 按主畫面 Exit 後要不要讓 wb_serve 自己結束（S95 關站那一半／todo D-007）
**背景**：Exit 現在照 golden 先存生產資料（BinCount、lastdata、DailyJamRate、Arm*.dat），但 wb_serve 除了 `--seconds` 到期之外沒有正常結束的入口，所以存完頁面會說「程式不會自己結束，請在主控台 Ctrl-C」。golden FormClose 其餘的關站動作（SaveMachineRecord、停馬達、加熱器與風扇 Off、ATC OffLine…）大多是 Jimmy 的範圍。
**選項**：A 維持現狀（只存檔，提示 Ctrl-C）／B 加一個結束旗標：Exit 第二框確認後，主迴圈晚一圈 break，走既有的正常關站路（存檔→Program Close=1→server.Stop()）；硬體停機那些項目先問 Jimmy。
**St01建議**：B 當下一步；硬體停機項目等 Jimmy 回覆再接。
**目前狀態**：**已裁決**——Exit：RULINGS_20260926 S121（已做 `a684f171`：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp、D:\HT9045\web\page\ht9045_main_close.js、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp）；追問 (1)：Steven 20260927 10:5x（經 github-02 轉述）「操作員按一下 start」⇒ 網頁重新連上後要操作員按一下 Start 才繼續生產，不自動接續（RULINGS_20260926 S161）；追問 (2)：見 Q39（RULINGS_20260927 #17）。關分頁／重新整理要不要停馬達（S122、RULINGS_20260927 #18）另案，仍待 Steven。（先前紀錄：**Steven 22:xx 已回答**：Exit「要通知 c++ 完全停工，馬達跟加熱都要關掉，安全第一」（RULINGS_20260926 S121；**已做 `a684f171`**：Exit 第二框確認後存檔、照 golden FormClose 順序停馬達／鎖煞車／關加熱器繼電器與風扇，再結束 wb_serve；替身沒停的項目在頁面與主控台標「未停」）；關分頁「要看是關閉什麼分頁，如果是 teach 或 motor test，會要求重新 find home」、重新整理「應該是不影響生產，但是可以的話，還是先把馬達停下來，等網頁重新開啟後，才繼續生產」（S122，交 Jimmy）。**還要 Steven 確認**：(1) 網頁重新連上後「繼續生產」要自動接續，還是操作員按一下 Start／Continue（St01 建議操作員按）；(2) 主控台 Ctrl-C／關視窗——已拆成 Q39，請在 Q39 回答。
**⚠ 還要 Steven 回**：追問 (1) 網頁重新連上後「繼續生產」要自動接續，還是操作員按一下 Start／Continue（St01 建議操作員按）；追問 (2) 已在 Q39 裁決。另：RULINGS_20260927 第 2 條第 18 題（S122：關分頁／重新整理要不要停馬達）原題沒有建議，**也待 Steven**。）
⛔ **更正（ST01-E 20260927 15:0x）**：上面「⚠ 還要 Steven 回」已過期——追問 (1) 已裁決（S161「操作員按一下 start」）；第 18 題也已裁決：RULINGS_20260927 第 2 條第 18 題＝**B**（Steven 0927 07:4x「依據建議」）：只做「關掉 Teach／Motor Test 分頁 ⇒ 標成必須重新 find home」，「主畫面斷線或重新整理就暫停生產」**不做**（golden 沒有這個行為），歸 **St01** 做、筆電知會。因為重新整理不停產，S161 的「重新連上要按 Start」目前沒有觸發點，留著備用。　**S122 方案（ST01-E 20260927 16:3x）**：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\S122_TEACH_LEAVE_PLAN_20260927.md`；判斷規則與細節寫成 decisions-pending 的 R80～R83。另更正：關 Teach 清旗標**不是**比 golden 嚴，golden `main.cpp:28846-28847` 本來就清（先前 S122／NIGHT_REPORT 第 18 題的「比 golden 嚴」說法有誤）。

### Q36. 主畫面狀態列 [0] Index 節拍要不要現在就接（S114 交件發現／todo E-006）
**背景**：S114 已經把狀態列 [2] UPH、[5] 測試秒數用 St01 自己的 tag（`mainsb.uph`、`mainsb.testSec`）送出去（見 R31）。狀態列 [0] 的 Index 節拍，golden 是在 Index 完成那一處寫 `[0]＝秒數＋" Sec"`；移植樹 `RunInfo.IndexTime` 已經有值（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp:24169），只差一個 tag。
**選項**：A 比照 R31，加 `mainsb.indexTime`，值＝RunInfo.IndexTime＋" Sec"／B 等 Q1（狀態列線上格式）定案再一起做。
**St01建議**：A。例：Index 一完成，狀態列最左格顯示 `0.662 Sec`；照 B 在 Q1 定案前都是 "---"。
**目前狀態**：**已裁決（隨 Q1＝S123）**——照 Q1 做：`mainsb.indexTime` 只送數值（RunInfo.IndexTime），「 Sec」由頁面組字。St01 派工中。（先前紀錄：等決定，沒動。）　**已做 `4e49cbf6`**（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp、D:\HT9045\web\page\main.html；S123：mainsb.uph 整數、mainsb.indexTime、mainsb.version；頁面組字）。

### Q37. Lot End 的「會寫哪些檔」清單要不要補上兩份測試總表（G-030 交件發現／todo G-030）
**背景**：G-030（`b40b140f`：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fLotInfo.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cObserver.cpp、D:\HT9045\web\page\main.html、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLotInfo.cpp 等 8 個檔）接上 golden 的 SaveTestSummary 之後，一般機台按 Lot End 會多寫兩份總表：`D:\HT9045_Log\Summary_Lot\<年月>\… Summary.txt`（Site／Bin 計數表）與 `D:\HT9045_Log\Summary\<年>\<月>\<LotID>_…txt`（Hard Bin Summary）。網頁確認框的文字已經補上這兩個資料夾；但 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLotInfo.cpp:292-307（commit `b40b140f` 當時的行號；現在 `WriteLotEndWrites` 在 :296-329）回給頁面的「writes」清單（列出這次會寫的完整檔名）還沒補。
**選項**：A 在 `WriteLotEndWrites` 的 `w.EndArray()` 前補兩行（St01 已備好片段，條件照 golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\SCK_ART.cpp:1620-1646：[N09] 沒開、不是 2D 排序、不是 93K SECS ART；Summary_Lot 那一份只在非 TCP/IP 測試）／B 維持只有確認框文字。
**St01建議**：A。例：Lot End 回應的 writes 清單會多兩行，例如 `D:\HT9045_Log\Summary\2026\09\<LotID>_<Process>_FT_…txt（Hard Bin Summary）`。
**目前狀態**：**已裁決（Steven 20260927 09:xx～10:xx，經 github-02 轉述；RULINGS_20260926 S155）**——＝**A**：已做（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLotInfo.cpp WriteLotEndWrites，條件照 golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\SCK_ART.cpp:1620-1646）。（先前紀錄：等決定，沒動（確認框文字已在 `b40b140f`）。）

### Q38. 測試總表的根目錄要不要加測試用接縫（G-030 交件發現／todo G-030）
**背景**：Hard Bin Summary 的根目錄 `asSummaryPath`（移植樹 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\common.cpp:164，對應 golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\common.cpp:27）寫死 `D:\HT9045_Log\Summary`，沒有環境變數可以轉開。目前 tests 裡沒有任何測試會觸發 Lot End（查過 0 筆），但將來 ctest 若驅動 Lot End，會寫到真機的資料夾。另一份 Summary_Lot 已有 `W906_SUMMARYLOT_ROOT` 可轉開。
**選項**：A 加一個 `W906_SUMMARY_ROOT` 環境變數當接縫（沒設就是 golden 原字）／B 維持 golden 原字。
**St01建議**：A。
**目前狀態**：**已裁決（Steven 20260927 09:xx～10:xx，經 github-02 轉述；RULINGS_20260926 S156）**——＝**B**：不加 `W906_SUMMARY_ROOT`，維持 golden 原字；SIM 按 Lot End 會寫到真的 D:\HT9045_Log\Summary（ctest 不涵蓋），驗完要自己清。（先前紀錄：等決定，沒動。）

### Q39. 主控台 Ctrl-C／按主控台視窗 X 也要走停機嗎（S121 交件 D1＋D2／Q35 第 (2) 點／todo D-007）
**背景**：Exit 鈕已經會停機（`a684f171`：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp、D:\HT9045\web\page\ht9045_main_close.js、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp）。但直接在 wb_serve 主控台按 Ctrl-C 或視窗右上角 X 時，程式目前**什麼停止命令都沒送**：不停馬達、不鎖煞車、不關加熱器繼電器和風扇，也不存今天的生產資料；卻還是寫了 `Program Close=1`，下次開機會當成正常關機。輸出之後會不會維持原狀，要看 PCIE-1203 驅動和從站看門狗，repo 裡沒有量測紀錄，在量過之前要當作「維持原狀」。
**選項**：A 維持現狀／B 接到 Exit 同一條關站路：Ctrl-C 只設結束旗標，由主執行緒停機、存檔再結束；按視窗 X 系統大約 5 秒就強制結束，所以那條路**先停機、再存檔**；`Program Close=1` 改成停機和存檔都做完才寫。
**St01建議**：B。例：操作員直接按主控台 X——現在繼電器和煞車可能維持原狀、今天的產量沒存、下次開機不提醒清料；改 B 以後跟按 Exit 一樣先停機再結束。另外有三個等待迴圈（警報框開著時）是 Jimmy 登記的段，要 Jimmy 一起改才會在警報框開著時也能結束。
**目前狀態**：**已裁決（Steven 20260927 07:2x，RULINGS_20260927 第 2 條 #17：A）**——Exit 與 Ctrl-C 都照 golden `TfMain::FormClose` 的順序停。St01 已接 Exit 的順序（`a684f171`）；Ctrl-C 那條與還沒有本體的停機函式由筆電補（驗證照第 4 條：模擬組態驗過就算完成，上機要看什麼寫進夜間報告）。（先前 St01 的建議保留：上機前先做低能量量測——開一個無害 DO，例如 SwHeaterFan，按 Ctrl-C，看 10 秒內 DO 燈會不會熄，確認卡片行為。）

### Q40. 「從資料庫選一筆就自動填欄位」要不要接、怎麼接（S98 收尾交件／todo G-014）
**背景**：HotPlate、TrayForm、Cleaning 三頁都有一個「從資料庫選」的下拉清單。清單現在列得出來（HotPlate `7d490f7c`：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\formbridge\TfHotPlate.py、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HotPlateForm_File.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\CfgTrayPlate.cpp；TrayForm／Cleaning `217e7e5e`：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\CfgTrayPlate.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ACTForm.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Winway.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Monitor.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cConfiguration.cpp 等 22 個檔），但選了一筆**不會**把那一列的值填進欄位——golden 是靠下拉的 OnChange 事件（HotPlate D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cHotPlate.cpp:412-438），而網頁的表單橋接目前只有「開頁」和「存檔」兩個入口，沒有「事件」入口。存檔仍送畫面上的欄位值，所以不會存錯，只是要手動打。
**選項**：A 做一條三頁共用的表單事件指令（例如 `form.event`，帶元件名與選到第幾筆；伺服器照 golden 跑那個事件、回傳改過的欄位）——要動共用的產生器、`D:\HT9045\web\page\ht9045_wire_engine.js`（Jimmy 的檔）與 wb_serve 片段／B 每頁各做一條專用指令（例如 `hotplate.selectFromDB`），本體照 golden 逐字手寫，只動 St01 的檔，比較快，但三頁各一套。
**St01建議**：A（三頁一次解決，之後別頁的事件也能用）；要先跟 Jimmy 對 wire_engine.js。例：這台 HotPlate 頁選第 3 項 QFN2X2，應該填入 HotPlateName=QFN2X2、XST1=27.5、YST1=32.5、XPitch1=15、YPitch1=25、XCT1=8、YCT1=12；選第 0 項（表頭 Package Type）照 golden 不填。
**目前狀態**：**已裁決（Steven 20260927 09:xx～10:xx，經 github-02 轉述；RULINGS_20260926 S157）**——＝**A**：三頁（HotPlate、TrayForm、Cleaning）共用 `form.event`。照 Jimmy 00:2x 意見：`ht9045_wire_engine.js` 加選用送出點（只有標 `data-ht-event` 的控制項才送），C++ 分派 St01 寫；St01 已把訊息格式給 github-02 轉 Jimmy。（先前紀錄：等決定，沒動。Jimmy 20260927 00:2x（TO_STEVEN §4，`D:\HT9045_handoff\TO_STEVEN.md`）意見：可以在 `ht9045_wire_engine.js` 加一個**選用**的送出點——只有標了屬性（例如 `data-ht-event="change"`）的控制項才送 `form.event`（頁名、控制項名、事件、值），沒標的頁完全不變；C++ 的分派由 St01 寫。Steven 選 A 之後，St01 在 §3 貼訊息格式，Jimmy 照格式加。）　**C++ 分派已做 `76058840`**（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_FormEvent.cpp`、`JsonBridge\FormBridge.cpp`、`FileRW\_EditPage.cpp`，wb_serve `:4826` 同一行）：HotPlate 7 格、TrayForm 三個 Type 各 10 格照 golden；Cleaning 照 golden 擋（R77）；衍生 R78、R79。等筆電在 `D:\HT9045\web\page\ht9045_wire_engine.js` 加送出點；探針 `tools\webprobe\formevent_probe.py` 還沒跑。

### Q41. 讀寫檔本體大致做完了，接下來要不要開始做「有讀寫、沒頁面」那幾頁（S73 往後排的那一批／todo E-001、G-002、G-014、G-019、G-025～G-027）
**背景**：S73 時 Steven 說「往後排，先讀寫檔」。到 20260927 為止，St01 名下的配方讀寫與生產資料顯示／存檔大致都接完了（剩下的不是等 Steven 決定、就是卡 Jimmy，或是 S105 這種已說往後排的）。下面這些檔的讀寫本體已經在，但操作員沒有畫面可以改：Rotate、AutoAlignment、FixAICCD、Magazine、AutoCalSuckZ、AOISetup（E-001／G-002）、Configuration 頁的 Tray／HP 兩個分頁（S98）、AutoTemperature（S108）、ATC WinWay（S109）、Monitor（S110）、DynamicTemp（S103）。
**選項**：A 現在開始做頁面，先做跟配方有關、換料會用到的（Configuration 的 Tray／HP 分頁、Magazine、FixAICCD、AutoCalSuckZ、AOISetup），再做機台設定類（Rotate、AutoAlignment、AutoTemperature、WinWay、Monitor、DynamicTemp）／B 繼續往後排，St01 先做別的（例如 S105 從 Server／RMS 下載配方）／C Steven 指定順序。
**St01建議**：A。例：現在要改某個配方的 Tray 表，只能直接編輯 `D:\HT9045\System\TrayForm.csv`；做了頁面之後就可以在網頁上改、按 Save 照 golden 存。
**目前狀態**：**已裁決（Steven 20260927 09:xx～10:xx，經 github-02 轉述；RULINGS_20260926 S158）**——新頁面**先不做**；先把**既有頁面**裡還沒實作的 golden 事件（OnChange／OnClick 等）與權限等級檢查（Insufficient(n) 等）補齊。St01 先盤點成清單（頁、事件／等級、golden 出處、歸誰），適合的分給 Steven02。（先前紀錄：等決定。Steven 沒回之前，St01 先做不用等決定的收尾（例如 Observer Yield 圖表）。）

### Q42. Data.Observer 視窗要不要照 golden 檢查權限等級（S116 殘項交件／todo E-009）
**背景**：golden 開 Observer 視窗要過 `fSecurity->Insufficient(5)`（D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28934）。網頁版主畫面直接開這一頁，沒有等級檢查——網頁的開窗權限表（D:\HT9045\web\background.html MODAL_POLICY）目前只有六列，刻意沒列 Observer。以前這一頁只能看；現在 Yield 分頁多了會改記憶體狀態的操作（點 site 開關線、改 Y 軸、Clear）。
**選項**：A 維持不檢查／B 在開窗權限表加 Observer 一列（等級 5，照 golden）。
**St01建議**：B，但那張表歸 Jimmy 管，要先問 Jimmy。例：低權限的操作員現在可以開 Observer 按 Clear 清掉 site 的 Bin 歷史；照 golden 這種帳號連視窗都開不了。
**目前狀態**：**已裁決（Steven 20260927 07:2x，RULINGS_20260927 第 2 條 #21：A）**——筆電在 D:\HT9045\.github\specs\page-access-policy.md §3 加第 7 列 observer＝等級 5，St01 照表接（等筆電的政策表改動進 main）。（先前紀錄：筆電 20260927 01:5x 意見照 golden 要等級 5（906 main.cpp:27933-27936；行號待確認：golden／移植樹都對不上，應該是 906 BCB 對照樹的行號，這台沒有那棵樹），`D:\HT9045\web\background.html:484` 寫明 MODAL_POLICY 不自行擴大，要改政策表）

### Q43. St01 要不要開始 build（RULINGS_20260927 第 4 條／todo 無）
**背景**：Steven 20260926 給 St01 的常設指示是「不 build，只做語法檢查」。RULINGS_20260927 第 4 條「這兩天不會有人在機台旁邊，機台驗證先不用等，只要編譯和模擬驗證成功就當做成功」——完成標準是兩組態全量 gate＋該項模擬驗證。這條沒有明講 St01 要不要自己 build；目前 St01 分支的改動要等筆電合進 main 時才會被 gate 編到。
**選項**：A St01 維持只做語法檢查，gate 由筆電合分支時跑／B St01 開始在這台跑兩組態 gate（出貨＋模擬）與相關 ctest，每一批交件前自己驗。
**St01建議**：B——第 4 條把「編譯＋模擬驗證」當完成標準，St01 這條線累積了很多只做過語法檢查的 commit，越晚編越難找是哪一顆壞的。例：S121 停機、S113 累計時間這幾批，現在連結會不會過都只是推論。
**目前狀態**：**已裁決（Steven 20260927 09:xx～10:xx，經 github-02 轉述；RULINGS_20260926 S159）**——＝**有條件的 B**：手上沒有其他可做的工作時，St01 可以在這台跑 build 驗證（兩組態 gate／相關 ctest）；用 St01 自己的 build 目錄，避開 github-02 替 Steven02 代編（D:\AI_TempFile\st02-gb-p1-build*）的時段；注意 `-k` 會留舊 exe 讓 ctest 假通過（看 exe 時間戳與 exit code）。其餘時間照舊只做語法檢查。（先前紀錄：等決定；決定之前照舊只做語法檢查。）

### St01 已照建議先做的 R（可推翻）

### R1. SortCT Lot ID 只在 CC_ASE_M 生效（S97 Q97-1／todo G-013）
**背景**：普查一開始以為這是通用功能，交件時發現 golden 其實只在 `CC_ASE_M` 客戶碼下才會跑。
**選項**：A 照 golden 只在 `CC_ASE_M` 生效／B 放寬給所有客戶碼用。
**St01建議**：A。
**目前狀態**：已照建議先做，可推翻：commit `d606b1d8`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TTLCfg.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebSortCT.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\BinSelect.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSortCT.cpp 等 15 個檔）。

### R2. 1000 ms 的 Timer1 tick 要不要接上（S97 Q97-2／todo G-013）
**背景**：golden 這功能靠 Timer1 每秒觸發一次，移植樹目前還沒接這個 tick（錨點屬於 Jimmy／EastSun 正在改的 pump 段）。
**選項**：A 現在想辦法接上（可能動到別人正在改的錨點）／B 先不接，停機時打的字要等下次送 SystemStart 中的請求才會寫入。
**St01建議**：B，避免跟其他人正在改的段落衝突（工程師原本建議 A；主 session 查到錨點那一行是 EastSun／Jimmy 的 pump 行，改選 B）。
**目前狀態**：20260927 改成 **A**（已接上）：當初選 B 只因為錨點是 EastSun／Jimmy 的 pump 行；現在改掛在主迴圈底部 St01 自己的行尾（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5953，S121 結束檢查那一行），不碰別人的行。只有 CC_ASE_M 開過 SortCT 頁才會啟用，其他機台一進來就 return。原本 commit `d606b1d8`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TTLCfg.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebSortCT.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\BinSelect.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSortCT.cpp 等 15 個檔）；接上是 commit `6db687d4`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebSortCT.cpp 等 3 個檔）：`W906_SortCTTimer1Tick` 掛在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5953` 那一行的尾端，主迴圈每一圈都會跑到；它自己節流成 1000 ms（golden Timer1 的預設間隔），沒有 CC_ASE_M 開過 SortCT 頁就直接 return。

### R3. BinSel 存檔要不要另開新指令（S99 Q99-1／todo G-015）
**背景**：這功能可以走既有的 `editlist.save`（多帶 op 欄位），也可以另開一支新指令。
**選項**：A 走既有 `editlist.save tag=BinSel` 帶 op（跟其他頁面存檔方式一致）／B 另開新指令。
**St01建議**：A。
**目前狀態**：已照建議先做，可推翻：commit `d606b1d8`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TTLCfg.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebSortCT.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\BinSelect.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSortCT.cpp 等 15 個檔）。

### R4. golden 既有 bug（Drop 寫成 "1" 不是 "2.00"）要不要照翻（S100 Q100-1／todo G-016）
**背景**：`sbSetupClick` 尾段在 `_8Site1X4` 模式下寫 `D:\HT9045\IniData\Data\<配方>\Contact.Data`，golden 因型別解析選錯多載函式，實際寫進檔案的 `Drop` 值是 "1" 不是 "2.00"，這是 golden 本身既有的 bug。
**選項**：A 照翻 golden 這個 bug，並請 Jimmy 看 BCB6 端要不要一起修／B 移植樹順手修正（偏離 golden）。
**St01建議**：A，請 Jimmy 確認 BCB6 端。
**目前狀態**：已照建議先做，可推翻：commit `d606b1d8`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TTLCfg.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebSortCT.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\BinSelect.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSortCT.cpp 等 15 個檔）。

### R5. Contact 頁同時開著的互蓋風險要不要處理（S100 Q100-2／todo G-016）
**背景**：這幾個零星讀寫項目若 Contact 頁跟其他頁同時開著，可能有互蓋風險（類似 S90 情況），這次沒加防護。
**選項**：A 先不處理（風險小，情境少見）／B 比照 S90 的做法加防護。
**St01建議**：A。
**目前狀態**：已照建議先做，可推翻：commit `d606b1d8`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TTLCfg.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebSortCT.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\BinSelect.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSortCT.cpp 等 15 個檔）。

### R6. `SetToDefineValue` 守衛要不要照 golden 只改記憶體（S100 Q100-3／todo G-016）
**背景**：`SetToDefineValue1Click`（吸嘴 VacuumOn/OffTime 等 dummy 時間）golden 只改記憶體不寫檔。
**選項**：A 照 golden，只改記憶體／B 順便寫檔持久化。
**St01建議**：A。
**目前狀態**：已照建議先做，可推翻：commit `d606b1d8`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TTLCfg.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebSortCT.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\BinSelect.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSortCT.cpp 等 15 個檔）。

### R7. DIO 設定檔刪除要不要加確認框（S101 Q101-1／todo G-017）
**背景**：golden `spbDeleteClick` 刪除 DIO 設定檔時沒有跳確認框，直接刪。
**選項**：A 照 golden，不加確認框／B 網頁版加一個確認框（更安全，但偏離 golden）。
**St01建議**：A。
**目前狀態**：已照建議先做，可推翻：commit `d606b1d8`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TTLCfg.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebSortCT.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\BinSelect.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSortCT.cpp 等 15 個檔）。

### R8. 移植樹額外加的兩道守衛要不要保留（S101 Q101-2／todo G-017）
**背景**：移植樹在 DIO 設定檔刪除多加了 golden 沒有的兩道防護（存檔前重查權限 31、只允許刪清單上的檔）。
**選項**：A 保留這兩道額外守衛（偏離 golden 但只是加嚴）／B 拿掉，完全照 golden。
**St01建議**：A。
**目前狀態**：已照建議先做，可推翻：commit `d606b1d8`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TTLCfg.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebSortCT.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\BinSelect.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSortCT.cpp 等 15 個檔）。

### R9. `SendCCDCommand` 要不要維持閘住（S94 Q1／todo G-011）
**背景**：LotData／TrayIDByLot／OCRLot 清單清除鈕裡，golden 有一個會送出相機清除指令，歸屬不明確。
**選項**：A 維持閘住，不送出（安全但功能不完整）／B 接上讓它真的送出（需先確認相機通訊介面）。
**St01建議**：A，等相機通訊介面明確後再開。
**目前狀態**：已照建議先做，可推翻：commit `26d0b3f8`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLotInfo.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainBoot.cpp、D:\HT9045\web\page\ht9045_lotinfo_wire.js、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fLotInfo.cpp 等 11 個檔）。

### R10. 三份清單開機讀回要不要照 golden 接（S94 Q2／todo G-011）
**背景**：`D:\HT9045_Log\2DBarCode\LotData.txt`／`D:\HT9045_Log\2DBarCode\TrayIDByLot.txt`／`D:\HT9045_Log\OCR\OCRLot.txt` 開機時 golden 會讀回這些清單。
**選項**：A 照 golden 開機讀回／B 不讀回，維持空清單。
**St01建議**：A。
**目前狀態**：已照建議先做，可推翻：commit `26d0b3f8`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLotInfo.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainBoot.cpp、D:\HT9045\web\page\ht9045_lotinfo_wire.js、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fLotInfo.cpp 等 11 個檔）。

### R11. 網頁 Clear List 要不要保留兩段式確認（S94 Q3／todo G-011）
**背景**：網頁清除清單按鈕目前要點兩次才會真的清除，避免誤觸。
**選項**：A 維持兩段式確認／B 改成一次點擊就清除。
**St01建議**：A。
**目前狀態**：已照建議先做，可推翻：commit `26d0b3f8`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLotInfo.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainBoot.cpp、D:\HT9045\web\page\ht9045_lotinfo_wire.js、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fLotInfo.cpp 等 11 個檔）。

### R12. OCR Clean List 按鈕要不要先畫出來（S94 Q4／todo G-011）
**背景**：OCR 清單清除功能本體已翻好，網頁上對應按鈕這次先不畫。
**選項**：A 先不畫（功能在但沒入口）／B 現在就畫出來。
**St01建議**：A。
**目前狀態**：已照建議先做，可推翻：commit `26d0b3f8`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLotInfo.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainBoot.cpp、D:\HT9045\web\page\ht9045_lotinfo_wire.js、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fLotInfo.cpp 等 11 個檔）。

### R13. 配方 MD5／自動備份本體放哪裡（S92 Q1／todo G-009）
**背景**：`BackupSetupFile` 本體目前寫在只給 wb_serve 用的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainBoot.cpp` 副本，還沒搬進共用的 `ht9045_globals`。
**選項**：A 維持現狀，先放專屬副本，恢復 build 後再評估搬共用模組／B 現在就搬進共用模組（需先恢復 build 才能驗證）。
**St01建議**：A。
**目前狀態**：已照建議先做，可推翻（本體已寫在工作樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainBackup.cpp`，等 Steven02 對 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp:458` 一行替換點頭才會 commit 套上）。

### R14. CC_ASE_SG 整個刪除重建 SetupFile 資料夾的行為要不要照翻（S92 Q3／todo G-009）
**背景**：golden 對 `CC_ASE_SG`（新加坡 ASE）客戶碼，配方備份會把整個 `D:\SetupFile\` 資料夾刪掉重建。
**選項**：A 照 golden 保留並加註客戶碼（這台不是新加坡 ASE 不會觸發）／B 移植樹改成不刪除、只覆蓋。
**St01建議**：A。
**目前狀態**：已照建議先做，可推翻（同 R13，等 commit）。

### R15. ContactForce 建構子要不要開機就跑（S57 決策題 1／todo D-003）
**背景**：golden `TfContactForce` 建構子在一開機就會跑初始化，移植樹目前選擇延後（開頁才跑）。
**選項**：A 維持延後（開頁才跑，開機快，但跟 golden 時序不同）／B 開機就跑（照 golden 時序）。
**St01建議**：A。
**目前狀態**：**已裁決（Steven 20260927 11:0x，看過 St01 查證後，經 github-02 轉述；RULINGS_20260926 S162）：B**——ContactForce 建構子開機就跑（照 golden 時序）。副作用：開機時依 912 [SLK Type] 在 D:\HT9045\system\ContactInfo.ini 補 64 段 Ind 預設鍵、沒檔時建檔 ⇒ 列入筆電 sysguard 預期變動、知會 Jimmy。**已做 `725038a6`**（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce.cpp` FileRW_ContactForce_Boot 開機跑 V912 建構子本體 ContactForce.cpp:421-653；呼叫位置 `tools\wb_serve.cpp:4062` 不變、只補註解）；Steven01 這台模擬：ContactInfo.ini 補 40／56／60／80 各 16 段、共 64 段 128 把鍵；衍生題 R72。（先前紀錄：已照建議先做，可推翻：commit `21d37f2b`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce.gen.inc、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce_Panels.h、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ContactForceLoad.cpp 等 9 個檔）。　**Steven 20260927（經 github-02 轉述）**：「這數值生產中應該會用到. 是的話就是B」⇒ St01 查證：生產端（adam6024、cContact）讀的底層共用表 `ContactForceTables()` 開機時就由 906 載入器 `LoadContactForceTables()` 讀齊（CheckAndReadIniData 的預設值與缺鍵補寫也在開機），延到第一次開頁的只有 V912 建構子的頁面面板物件與「依 912 [SLK Type] 在 ContactInfo.ini 補 64 段 Ind 預設鍵、沒檔時建檔」⇒ 選 B 的差別只是這個開機寫檔副作用，生產數值不變；St01 建議維持 A，已回 Steven 再選。）

### R16. Ind 表型號清單要跟 906 還是 912（S57 決策題 2／todo D-003）
**背景**：ContactForce 頁的型號下拉清單，906 與 912 版本內容不同。
**選項**：A 跟 906（現行底層版本一致）／B 底層改成跟 912 的 `[SLK Type]` 一致（跟畫面顯示版本對齊）。
**St01建議**：B（已照做）。
**目前狀態**：已照建議先做，可推翻：commit `21d37f2b`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce.gen.inc、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce_Panels.h、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ContactForceLoad.cpp 等 9 個檔）。　⛔ **更正（ST01-E 20260927 14:4x，skill 整理時核對）**：上面「St01建議 B（已照做）」不對——移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ContactForceLoad.cpp:164-170` 機台用的表（`ContactForceTables()`）到 HEAD 仍讀 906 的 `[SLK Type Ind]`（這支檔 `21d37f2b` 之後沒再改），S57 交件報告也寫「現況：機台用 906 的 [SLK Type Ind]…建議 B」，所以**程式現況是 A**；只有頁面面板照 912 `[SLK Type]`。現況 A 符合 RULINGS_20260926 第 26 條「底層照 906、畫面照 912」。Steven 若要 B（底層改讀 912 `[SLK Type]`）請回；沒回就維持 A。

### R17. 必送清單要不要嚴格檢查（S57 決策題 4／todo D-003；決策題 3「EP 四鍵舊值蓋回」已於 S90 解決，不重列）
**背景**：存檔時有一份必送欄位清單，討論檢查要嚴格（缺一項就拒存）還是寬鬆（補預設值）。
**選項**：A 嚴格（缺項拒絕存檔）／B 寬鬆（缺項補預設值）。
**St01建議**：A。
**目前狀態**：已照建議先做，可推翻：commit `21d37f2b`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce.gen.inc、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce_Panels.h、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ContactForceLoad.cpp 等 9 個檔）。

### R18. `TTrackBar` 的 `SetMin` 守衛要不要照現行寫法（S57 決策題 5／todo D-003）
**背景**：ContactForce 頁面拉桿元件設最小值有個邊界守衛寫法，需要 Jimmy 在 BCB6 核對是否跟 golden 一致。
**選項**：A 維持現行守衛寫法，等 Jimmy 之後核對／B 現在先改別的寫法。
**St01建議**：A。
**目前狀態**：已照建議先做，可推翻：commit `21d37f2b`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce.gen.inc、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce_Panels.h、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ContactForceLoad.cpp 等 9 個檔）。

### R19. 網頁 B 路直改 AOI.Data 記憶體不重讀要不要處理（S86 Q2／todo D-003）
**背景**：若有人透過 B 路通用配方寫入指令直接改 `D:\HT9045\IniData\Data\<配方>\AOI.Data`，移植樹目前不會自動重讀進記憶體，跟 C 路存檔會重讀的行為不一致。
**選項**：A 現在維持現狀，等真的做 OCR 頁面時改走 C 路擋掉／B 現在就補上 B 路寫檔後自動重讀。
**St01建議**：A。
**目前狀態**：已照建議先做（維持現狀），可推翻。

### R20. AOI OCR SETTING 沒有頁面要不要現在做（S86 Q3／todo D-003）
**背景**：讀寫檔本體已翻好，但沒有對應操作頁面，目前沒有觸發點會真的呼叫存檔。
**選項**：A 依 S73 建頁往後排的原則，先不做頁面／B 現在就做頁面。
**St01建議**：A。
**目前狀態**：已照建議先做，可推翻。

### R21. Lot End 鈕什麼時候顯示（S117 Q1／todo G-029）
**背景**：golden 的 Lot End 所在那一塊（palSecsGem）在 DFM 預設看得到，只有 CC_MTI 會藏（D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uLotInfo.cpp FormShow :370／:411）；任務單原本寫「只在 SECS/GEM 或 ATP 鎖參數時顯示」是誤讀。
**選項**：A 照 golden，一般機台都看得到，只有 CC_MTI 藏／B 只在 SECS/GEM 或 ATP 鎖參數時顯示。
**St01建議**：A，照 golden。例：這台 CUSTOMER_CODE=791 沒開 SECS/GEM，golden 畫面上有 Lot End，照 B 網頁就沒有這顆鈕。
**目前狀態**：已照建議先做，可推翻：commit `5d58c98d`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fLotInfo.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLotInfo.cpp、D:\HT9045\web\page\ht9045_lotinfo_wire.js、D:\HT9045\web\page\Data.LotInfo.html 等 6 個檔）。

### R22. 批次結束的測試總表 SaveTestSummary(1) 要不要接（S117 Q2／todo G-030）
**背景**：golden SetLotEnd（D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uLotInfo.cpp:2258-2261）在 [N09] 沒開時（一般機台也會走）會存一份這一批的 TSV 測試總表；移植樹 TfSCKART 沒有這支，已翻的 SckArtRem_SaveTestSummary 需要的狀態物件全樹沒有實例。
**選項**：A 開一個 St01 後續工作去接（排在 S95 之後；D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp:2701、:3913 兩個空樁要跟 Jimmy 對）／B 先維持閘住。
**St01建議**：A。例：一般機台按 Lot End 後，golden 會多一份這一批的 TSV 總表，移植樹目前不會。
**目前狀態**：已照建議排進 todo G-030（還沒做，閘名 GATE (W906-PROD-S117-SUMMARY)）。

### R23. 網頁 Lot End 要不要兩段式確認（S117 Q3／todo G-029）
**背景**：golden 除了 OEE 機台，按下去就直接結批。誤按的話 D:\HT9045\config\config.ini 的 Lot 資訊馬上被清空、START 也會被擋。
**選項**：A 保留確認（先例 barcode.clearCount）／B 改成一段式照 golden。
**St01建議**：A。
**目前狀態**：已照建議先做，可推翻：commit `5d58c98d`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fLotInfo.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLotInfo.cpp、D:\HT9045\web\page\ht9045_lotinfo_wire.js、D:\HT9045\web\page\Data.LotInfo.html 等 6 個檔）。

### R24. Lot End 的 SECS 事件 EventReport(DoLotEnd) 要不要活著（S117 Q4／todo G-029）
**背景**：移植樹的 SECS 是模擬計數器，Lot Start 那邊的 DoLotStart 已經是活的。
**選項**：A 活的，跟 Lot Start 對稱／B 先閘住等 Steven02 的 HSMS。
**St01建議**：A。例：開 SECS/GEM 的機台如果閘住，就會只記到 Lot Start、沒有 Lot End。
**目前狀態**：已照建議先做，可推翻：commit `5d58c98d`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fLotInfo.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLotInfo.cpp、D:\HT9045\web\page\ht9045_lotinfo_wire.js、D:\HT9045\web\page\Data.LotInfo.html 等 6 個檔）。

### R25. 每日 Jam 率檔一個行程只存一次（S95 A1／todo D-007）
**背景**：golden 每次關程式存一次；網頁 Exit 存完但程式沒結束的話，之後 `--seconds` 到期會再存一次，那時記憶體已被第一次存檔清掉，照字面會把檔案寫成 0 筆。
**選項**：A 一個行程只存一次（第二次起跳過）／B 照 golden 字面每次都存。
**St01建議**：A。例：早上 12 筆 Jam，按 Exit 存了但沒關程式，下午又存一次，B 會把檔案蓋成 0 筆。
**目前狀態**：已照建議先做，可推翻：commit `6905f8eb`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp、D:\HT9045\web\page\ht9045_main_close.js、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cCounterClear.cpp 等 10 個檔）。　**Steven 20260927（經 github-02 轉述）**：「這個屬於統計的問題, 後續可能會改方法, 類似使用資料庫的方式, 待確認」⇒ 現行 A 維持；統計類資料之後可能改成類似資料庫的方式存，方法待 Steven 確認，現在不做。

### R26. Exit 照 golden 問兩次確認（S95 A3／todo D-007）
**背景**：golden 先問「Sure close??」，存完 BinCount／lastdata 再問「Sure To Exit?」。
**選項**：A 照 golden 問兩次／B 合成一次。
**St01建議**：A。
**目前狀態**：已照建議先做，可推翻：commit `6905f8eb`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp、D:\HT9045\web\page\ht9045_main_close.js、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cCounterClear.cpp 等 10 個檔）。

### R27. SECS 的 DoExit 事件等真的關站才送（S95 A4／todo D-007）
**背景**：Exit 目前不會真的結束程式；如果按 Exit 就送 DoExit，主機會以為機台已經離線。
**選項**：A 等真的關站（Q35 做了之後）才送／B 按 Exit 就送。
**St01建議**：A。
**目前狀態**：已照建議先做（目前不送），可推翻：commit `6905f8eb`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp、D:\HT9045\web\page\ht9045_main_close.js、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cCounterClear.cpp 等 10 個檔）。

### R28. `--seconds` 跑的探針結束時也照 golden 存生產資料（S95 A5／todo D-007）
**背景**：接上 S95 之後，每一次用 `--seconds` 跑的探針結束時都會寫 lastdata*、D:\HT9045\config\config.ini [O_Count]、DailyJamRate、Arm*.dat；探針原本「真實檔 0 變動」的基準會亮起來。
**選項**：A 照 golden，探針把這幾個檔列為預期變動／B 探針加旗標跳過（新發明的行為）。
**St01建議**：A。
**目前狀態**：已照建議先做，可推翻：commit `6905f8eb`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp、D:\HT9045\web\page\ht9045_main_close.js、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cCounterClear.cpp 等 10 個檔；探針基準之後恢復 build 時一起更新）。

### R29. 告警框開著時，Counter Clear 的收尾等框關掉才做（S111 B1／todo G-028）
**背景**：golden 的 Timer10 在告警框開著時照跑；移植樹的 tick 在告警框（阻塞等待迴圈）期間不跑，收尾會晚到框關掉。等待迴圈是 Jimmy 登記的段落。
**選項**：A 接受這個延遲／B 把收尾加進等待迴圈（要動 Jimmy 的段）。
**St01建議**：A。
**目前狀態**：已照建議先做，可推翻：commit `6905f8eb`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp、D:\HT9045\web\page\ht9045_main_close.js、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cCounterClear.cpp 等 10 個檔）。

### R30. btSavelog 不算生產資料，從 S95 清單拿掉（S95／todo D-007）
**背景**：golden FormClose 的 btSavelog 在 `#ifdef DEBUG_AUTO_CLEAN` 裡，golden 出貨版沒開這個巨集；它存的是 Auto Clean 除錯 memo（d:\AutoCleanLogs\），不是生產資料。
**選項**：A 照 golden 保留同一個 #ifdef（出貨版不編），從 S95 清單拿掉／B 硬是做。
**St01建議**：A。
**目前狀態**：已照建議先做，可推翻：commit `6905f8eb`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp、D:\HT9045\web\page\ht9045_main_close.js、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cCounterClear.cpp 等 10 個檔）。

### R31. 狀態列 UPH／測試秒數先用 St01 自己的 tag 名（S114 Q-a／todo E-006）
**背景**：狀態列怎麼送（Q1）還沒定；S114 要讓 [2] UPH、[5] 測試秒數先有真值。
**選項**：A 先用 `mainsb.*` 送 golden 原字，等 Q1 定案再決定要不要搬到 `status.*`／B 先定 Q1，直接改 `status.uph`。
**St01建議**：A——不動 Jimmy 那邊的 `status.*` 與 kUnloadedTags。例：`mainsb.uph = "UPH = 533"`、`mainsb.testSec = 12`。
**目前狀態**：已照建議先做，可推翻：commit `b40b140f`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fLotInfo.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cObserver.cpp、D:\HT9045\web\page\main.html、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLotInfo.cpp 等 8 個檔）。⚠ 20260927 Steven 的 Q1 裁決（S123：C++ 送數值、頁面組字）⇒ `mainsb.uph` 要從伺服器組好的 "UPH = 533" 改成只送數值，頁面組字；派工中。　**已做 `4e49cbf6`**（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp、D:\HT9045\web\page\main.html；S123：mainsb.uph 整數、mainsb.indexTime、mainsb.version；頁面組字）。

### R32. Test Information 的 Index 時間平均，更新時間點比 golden 早（S116／todo E-006）
**背景**：golden 的 Index 時間平均來自 RecordTimeInfo 寫進 Observer 格子 [5][11]（測完才更新）；RecordTimeInfo 是 Jimmy 的 W2-3，還沒翻。S116 先用 RunInfo.IndexTime 在 Index 完成時算平均，RecordTimeInfo 翻好後自動改用格子。
**選項**：A 接受這個時間點差異，等 RecordTimeInfo 翻好自動讓位／B 現在就把 RecordTimeInfo 列為必翻。
**St01建議**：A。例：Index 一完成，Now 列就從 0.66 變成 0.70；golden 要等測完才變。
**目前狀態**：已照建議先做，可推翻：commit `b40b140f`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fLotInfo.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cObserver.cpp、D:\HT9045\web\page\main.html、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLotInfo.cpp 等 8 個檔；RecordTimeInfo 已轉 Jimmy）。

### R33. Configuration 頁 HP 表按 Save 後，重讀的是 Tray 表（S98-(a)／todo G-014）
**背景**：golden `sbUpdateHPClick`（D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:7215-7216）存完 HP 表之後，呼叫的是 Tray 表的重讀，看起來是抄錯沒改。
**選項**：A 照 golden／B 改成重讀 HP 表。
**St01建議**：A。例：HP 分頁改一格按 Save，檔案有存，但畫面重讀的是 Tray 表。
**目前狀態**：已照建議先做，可推翻：commit `217e7e5e`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\CfgTrayPlate.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ACTForm.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Winway.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Monitor.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cConfiguration.cpp 等 22 個檔；頁面還沒有這兩個分頁）。

### R34. Tray／HP 表存一次再讀，第 16 欄會掉（S98-(b)／todo G-014）
**背景**：golden 的 reload 只收「後面有逗號」的欄位，最後一欄存檔後重讀就不見。
**選項**：A 照 golden／B 修 reload。
**St01建議**：A。例：表頭 BlockPitchY 按一次 Save 再 Load 就空了，下一次 Save 就從檔案消失。
**目前狀態**：已照建議先做，可推翻：commit `217e7e5e`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\CfgTrayPlate.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ACTForm.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Winway.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Monitor.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cConfiguration.cpp 等 22 個檔）。

### R35. Tray／HP 表存檔時空欄位不加引號（S98-(c)／todo G-014）
**背景**：golden 是 BCB6 的 CommaText，空欄位不加引號；移植樹共用的 vclcompat 會寫成 `""`。
**選項**：A 只在這支檔照 BCB6 寫（不動共用元件）／B 修 vclcompat，全樹所有用 CommaText 寫檔的地方一起變，要跑完整 ctest。
**St01建議**：A，等可以 build 的那一輪再考慮 B。例：`QFN,,Memo` 這一列，vclcompat 會寫成 `QFN,"",Memo`。
**目前狀態**：已照建議先做，可推翻：commit `217e7e5e`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\CfgTrayPlate.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ACTForm.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Winway.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Monitor.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cConfiguration.cpp 等 22 個檔；依據是 Delphi 6/7 原始碼，沒有在 BCB6 實機上量過）。

### R36. ATCWinWay.ini 每次開機都讀、並寫回（S109／todo G-026）
**背景**：golden TfWinway 建構子每次開機都對 4 站 LoadCommData 再 SaveCommData，不看 ATC_SYSTEM；開機時 COM 沒開，只存值、不送指令。
**選項**：A 照 golden／B 開機只讀，Update 鈕才寫。
**St01建議**：A。例：這台 `D:\HT9045\config\ATCWinWay.ini` 在版控裡、內容就是 golden 預設值，寫回的位元組不變；新機台沒有這個檔時第一次開機會建出來（golden 也是）。
**目前狀態**：已照建議先做，可推翻：commit `217e7e5e`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\CfgTrayPlate.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ACTForm.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Winway.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Monitor.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cConfiguration.cpp 等 22 個檔）。

### R37. AutoTemperature.ini 開機不讀、開頁才讀（S108／todo G-025）
**背景**：golden TACTForm 建構子在設定檔名之前就讀檔，讀到的是空檔名，所以開機時 ACTData 全是預設值，要到開頁才真的讀。
**選項**：A 照 golden／B 開機就讀檔。
**St01建議**：A；開頁前沒有人用 ACTData。例：檔裡 iThermoCtrlType=0，開機仍照 DTB4824 的預設值走。
**目前狀態**：已照建議先做，可推翻：commit `217e7e5e`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\CfgTrayPlate.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ACTForm.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Winway.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Monitor.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cConfiguration.cpp 等 22 個檔；還沒有頁面）。

### R38. Exit 停機不加 golden 沒有的伺服 OFF／切馬達動力電（S121 D3／todo D-007）
**背景**：golden FormClose 會停馬達、鎖煞車，但不做伺服 OFF，也不切 SwMotorRelay。
**選項**：A 照 golden 不加／B 加伺服 OFF 或切馬達動力電。
**St01建議**：A；煞車已經鎖了，先等 Q39 的量測結果再說。
**目前狀態**：已照建議先做，可推翻：commit `a684f171`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp、D:\HT9045\web\page\ht9045_main_close.js、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp）。

### R39. SIM 建置接真卡時，Exit 關不掉 1203 頁打開的輸出（S121 D4／todo D-007）
**背景**：SIM 建置的 SW[] 寫的是模擬值，Exit 時關加熱器繼電器、風扇那幾步只到模擬後端；網頁 1203 頁直接打開的真實輸出不會被關。
**選項**：A 頁面照實標「未停」（現況），並規定 SIM 建置不在真機上通電加熱／B SIM 建置額外用 1203 命令面把 DO 全關（偏離 golden）。
**St01建議**：A。例：筆電用 SIM 建置接真機、從 1203 頁打開加熱器繼電器，按 Exit 後繼電器仍通電，頁面會列出「未停：SwHeaterRelay」。
**目前狀態**：**已裁決（Steven 20260927 11:xx，經 github-02 轉述；RULINGS_20260926 S163）**：「必須先停下才能關閉」⇒ 不接受「頁面標未停就照樣關」：Exit 要確定輸出（含 SIM 建置接真卡時 1203 頁打開的真實 DO）都停了才能關閉——關閉流程真的把它們關掉，或有「未停」項目時不讓關、要操作員先停。St01 提做法中（寫進本題），定案再做。（先前紀錄：已照建議先做（頁面標「未停」），可推翻：commit `a684f171`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp、D:\HT9045\web\page\ht9045_main_close.js、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp）；「SIM 建置不在真機上通電加熱」這條規定要 Steven 點頭。　**做法**：ST01-E 20260927 寫成新題 **Q44**（`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md` 的「### Q44.」：A 自動關＋讀回＋擋／B 只擋不自動關，細節 Q44-1～Q44-6），等 Steven 選。
**⚠ 還要 Steven 回**：「SIM 建置不在真機上通電加熱」這條規定要不要定下來（點頭就搬到已決斷）。）
**20260929 11:1x 結案（ST01-M 依 Steven 既有裁決判定，不再問）**：Steven 20260929 07:2x Q44 原話「SOFT_SIMULTE因為馬達不會真的動作, 所以關閉時沒有限制」——前提就是 SIM 建置不拿來驅動真機，所以「SIM 建置不在真機上通電加熱」這條規定成立；SIM 按 Exit 不擋（`5789eeea`）。Steven 不同意再說。

### R40. `--seconds` 到期結束時也照 golden 停機（S121 風險 ⑦／todo D-007）
**背景**：以前 `--seconds` 到期只存檔就結束；現在跟 Exit 一樣會停馬達、鎖煞車、關加熱器繼電器／風扇／蜂鳴器，也會寫 D:\HT9045\system\machinerecord.dat、EventLogTxt、HANDLER LOG。
**選項**：A 接受（方向是安全的）／B 探針另加旗標跳過停機。
**St01建議**：A。例：在真機上用出貨組態跑 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\exe_startup_gate.ps1，結束時加熱器繼電器會被關、煞車會鎖。測試不想碰真檔要設 W906_MACHINERECORD_DIR、W906_UNLOADERINFO_ROOT。
**目前狀態**：已照建議先做，可推翻：commit `a684f171`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp、D:\HT9045\web\page\ht9045_main_close.js、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp）。

### R41. 沒讀到機型（bHandlerModel=false）時照樣停機（S121 風險 ⑧／todo D-007）
**背景**：golden 在那種狀態下開機就結束程式，根本沒機會打開任何東西；wb_serve 則照常跑，網頁可能打開過輸出。
**選項**：A 照樣停機、只是不存檔（偏離 golden，安全方向）／B 照 golden 什麼都不做。
**St01建議**：A。
**目前狀態**：已照建議先做，可推翻：commit `a684f171`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp、D:\HT9045\web\page\ht9045_main_close.js、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp）。

### R42. Observer 累計時間每拍呼叫放在 wb_serve 主迴圈，不放進 PumpTick（S113 D1／todo D-009）
**背景**：golden 是 Timer1（約 30 ms）呼叫 UpdateRecordScreen、Timer2（1 秒）呼叫 UpdateRunInfo。PumpTick 所在的 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp 也編進兩支 ctest（test_wb_tags、test_wb_simpump），直接在 PumpTick 裡呼叫會讓那兩支測試連結失敗。
**選項**：A 放在 wb_serve 主迴圈（同一個 500 ms 拍子，不進 PumpTick）／B 放進 PumpTick（要另加函式指標或替兩支測試補檔）。
**St01建議**：A。
**目前狀態**：已照建議先做，可推翻：commit `0b38b6b5`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainRecord.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp 等 3 個檔）；20260927 從 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4598（Jimmy 的 STATEREC／MotorAccessTick 行）搬到主迴圈底部 St01 自己的行尾 :5953，避免合併撞到別人的行（github-02 提醒）。

### R43. 累計時間只看 InitialOK，不看 SystemInitialOK（S113 D2／todo D-009）
**背景**：golden 還要 SystemInitialOK==true 才累計；wb_serve 裡沒有人把它設成 true（golden FormShow D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:10092 才設）。
**選項**：A 不看它，只看 InitialOK／B 照 golden 看。
**St01建議**：A。例：照 B 的話 wb_serve 開一整天，Power On Time 仍停在 D:\HT9045\system\lastdata.dat 讀進來的數字。
**目前狀態**：已照建議先做，可推翻：commit `0b38b6b5`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainRecord.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp 等 3 個檔）。

### R44. 系統時間從 wb_serve 開機就開始算（S113 D3／todo D-009）
**背景**：golden 的系統計時器起點在 SYSTEM_MODULAR 建構子（D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\database.cpp:47），程式一載入就開始算；移植樹那個建構子整段 #if 0。
**選項**：A 從第一拍才開始算／B 在 wb_serve 開機讀完機台設定那一行補上起點（St01 自己那一行）。
**St01建議**：B。例：wb_serve 從讀設定到第一拍如果花 20 秒，選 A 的話每次開機 Power On 少這 20 秒。
**目前狀態**：已照建議先做（B），可推翻：commit `0b38b6b5`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainRecord.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp 等 3 個檔）。

### R45. golden 的 `/60>25` 怪處照翻（S113 D4／todo D-009）
**背景**：golden UpdateRunInfo（D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:22748）用 `SystemAccSecond[0][stPowerOn]/60>25` 判斷「前 25 分鐘每分鐘記一次」，但 SystemAccSecond 從 20190714 起是毫秒，所以這條件其實是「超過 1.5 秒」，實際上一開機就只在分鐘數是 5 的倍數時記。
**選項**：A 照翻（跟客戶機台上的 BCB6 一致）／B 改成 /60000，讓註解說的行為成立。
**St01建議**：A。例：清過計數後 08:03 開機，A 在 08:05、08:10…記；B 在 08:03、08:04…每分鐘記，直到滿 25 分鐘。
**目前狀態**：已照建議先做，可推翻：commit `0b38b6b5`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainRecord.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp 等 3 個檔）。

### R46. 告警框開著時照樣累計 Jam Time（S113 D5／todo D-009）
**背景**：golden 告警框、訊息框開著時，框自己的 Timer 會呼叫主畫面 Timer1，所以 Jam Time 照樣累計。移植樹框開著時 wb_serve 在等待迴圈裡（W906_ModalWaitTick，Jimmy 登記的段），主迴圈的每拍呼叫不會跑。
**選項**：A 請 Jimmy 在 W906_ModalWaitTick 同一行加一段呼叫（St01 已備好片段）／B 不加。
**St01建議**：A。例：告警框開 3 分鐘，A 的話 Jam Time +3 分鐘；B 的話 Jam Time 幾乎 +0，機台內有 IC 時這 3 分鐘會算進 Pause Time。
**目前狀態**：已照建議把片段交給 Jimmy（github-02 轉），Jimmy 套上之前是 B 的行為。

### R47. 這台開發機的 lastdata.dat 會開始累加時間（S113 風險／todo D-009）
**背景**：S113 之前 wb_serve 讀進來的 SystemAccSecond 原封不動寫回；S113 之後每次 wb_serve 執行都會累加，任何一個會寫 D:\HT9045\system\lastdata.dat 的地方（關站、counter.clear、SortCT 清除、TowerLight 存檔…）都會存進去；同一台電腦上的 BCB6 HT9045.exe 會讀到累加後的值。這跟 golden 在真機上的行為一樣。
**選項**：A 接受（跟 golden 一樣）／B 開發機上另外轉開 lastdata.dat 路徑。
**St01建議**：A。另外提醒：VTEST 客戶開了定時 Auto Clean 時，這個累加會讓 Auto Clean 真的觸發（客戶分支，照 S25 保留）。
**目前狀態**：已照建議先做，可推翻：commit `0b38b6b5`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainRecord.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp 等 3 個檔）。

### R48. Q40 做完之前，開頁不另外顯示「選了不會填」的提示（S98 收尾交件／todo G-014）
**背景**：選下拉不會填欄位（見 Q40），可以在開頁的待辦清單裡一直掛一條提示，或只寫在程式註解。
**選項**：A 開頁一直顯示這條待辦／B 不顯示，跟 TrayForm、Cleaning 兩頁一致，只寫在程式註解。
**St01建議**：B。例：選 A 的話 D:\HT9045\web\page\Setup.HotPlate.html 開頁回來的 todo 會一直有這一條；選 B 就是空的。
**目前狀態**：已照建議先做，可推翻：commit `7d490f7c`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\formbridge\TfHotPlate.py、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HotPlateForm_File.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\CfgTrayPlate.cpp）。

### R49. 關程式存 RMS 資訊時，兩個值都空就先不寫（S95 收尾／todo D-007）
**背景**：golden 關程式時（機台內有 IC、有開 Lot Info）會把 Lot Info 頁的裝置名、溫度寫進 `D:\HT9045\config\config.ini` 的 [Server] Product Name／Product Temp。golden 開機時會先從同一個檔把這兩個值讀回頁面，所以寫回去等於不變；移植樹開機讀回那一步還閘著（Jimmy 的 GATE WC-1），頁面上這兩格是空的，照字面寫會把檔裡的值清掉。
**選項**：A 照 golden 字面寫（空的就寫空的）／B WC-1 解閘之前，兩個值都空就不寫，任一個有值（例如條碼掃入）才照 golden 寫。
**St01建議**：B（工程師原本建議 A；主 session 整合時改 B，理由是 A 會改到真檔、B 的結果才跟 golden 一樣）。例：機台內還有 IC、BCB6 上次存了 Product Name=ABC123，改跑 wb_serve 再按 Exit——A 會把它清成空，B 會保留 ABC123。這台 config.ini 這兩個鍵本來就是空的，兩種做法結果相同。
**目前狀態**：守衛已關掉（20260927 `61c96910`：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fLotInfo.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainBoot.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp 等 5 個檔）：Jimmy 00:5x 說 WC-1 歸 St01，St01 把 WC-1（開機與 Lot Info 開頁時讀回 Product Name／Temp，golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uLotInfo.cpp:14820-14834）翻好接上了，所以關程式時照 golden 寫。原本 B 的做法在 `bd40ffcb`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.h）；程式裡的守衛留著（`s_bRmsBlankGuard`，改回 true 就恢復 B）。

### R50. 沒讀到機型時不裝 RunMode.txt 的寫檔（S95 收尾／todo D-007）
**背景**：golden 在沒讀到機型（bHandlerModel=false）時開機就結束程式；wb_serve 照常跑，那時 LastSet 是零。
**選項**：A 那種狀態下不寫 D:\HT9045\system\RunMode.txt（偏離 golden，安全方向）／B 完全照 golden。
**St01建議**：A。例：D:\GPIB9045\system\general.ini 的 Model 不在白名單時，網頁換配方會觸發 SetRunStartMode——A 不寫；B 會把 RunMode.txt 從 `RunMode=2` 蓋成 `RunMode=0`。
**目前狀態**：已照建議先做，可推翻：commit `bd40ffcb`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.h）。

### R51. RunMode.txt、LotSummary.csv 不另加測試用轉向（S95 收尾／todo D-007）
**背景**：這兩個檔（D:\HT9045\system\RunMode.txt、D:\HT9045\System\LotSummary.csv）的路徑是 golden 寫死的字面值；寫檔只在 wb_serve 發生，ctest 不會寫（ctest 不裝 SaveRunMode、也不編 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp）。
**選項**：A 維持 golden 字面值／B 比照 W906_BINCOUNT_PATH 加 W906_RUNMODE_PATH、W906_LOTSUMMARY_PATH。
**St01建議**：A。例：開發機跑 wb_serve 後按 Exit，A 會改寫 `D:\HT9045\system` 那兩份（寫回的是這台自己讀到的值加上累計）；B 可以導到暫存資料夾。
**目前狀態**：已照建議先做，可推翻：commit `bd40ffcb`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.h）。

### R52. Observer Yield 圖的線色用 golden 實際畫出來的顏色（S116 殘項／todo E-009）
**背景**：golden 畫 Yield 圖時每一點都帶了顏色（跟 site 格子的底色相同，D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cObserver.cpp:857），TeeChart 用點的顏色畫線；DFM 裡每條線另有一個 SeriesColor，其中好幾條是白色。
**選項**：A 用 golden 實際畫出來的點色（跟 site 格子同色）／B 用 DFM 的 SeriesColor。
**St01建議**：A。例：Site Aa 在 A 是銀灰（跟 site 格子「Aa」同色），在 B 是紅色；Site Ba～Bh 在 B 全是白色，在底色上看不見。代價：A 的 Site Aa、Site Ad 在底色上也偏淡（推斷 golden 也是這樣，沒在 BCB6 機台上看過）。
**目前狀態**：已照建議先做，可推翻：commit `340a3ecf`（D:\HT9045\web\page\ht9045_observer_wire.js、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cObserver.cpp、D:\HT9045\web\page\Data.Observer.html）。

### R53. Observer Yield 分頁的四個操作也在伺服器端擋連點（S116 殘項／todo E-009、F-004）
**背景**：observer.get 原本整條列在伺服器防連點的「純讀」白名單；這次 Yield 分頁多了四個會改狀態的操作（點 site、改 Y 軸上下限、Clear），照白名單只有頁面那一道在擋。
**選項**：A 維持整條放行，只靠頁面／B 改成依 act 判斷：讀取類照舊放行，四個新操作進伺服器防連點。
**St01建議**：B（照 Steven 20260926「全部按鈕要防連點」）。例：兩個分頁同時開 Observer，在 400 ms 內各點一次同一格，那條線會切兩次、等於沒點；B 之後第二次回 busy，頁面顯示提示。
**目前狀態**：已照建議先做（B），可推翻：commit `8314e3bd`（R53 這部分改在 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebCmdGuard.cpp＋D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_webcmdguard.cpp；這顆 commit 另外還改了 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cObserver.cpp、D:\HT9045\web\page\ht9045_observer_wire.js、D:\HT9045\web\page\Data.Observer.html）。

### R54. Yield 分頁按 Clear 之後畫面不立刻變（S116 殘項／todo E-009）
**背景**：golden 的 Clear（SpeedButton1Click）只把 site 的 Bin 歷史清成 0，不重畫格子；要等下一顆測完或點 site 格才會更新。
**選項**：A 照 golden／B Clear 後立刻重畫（偏離 golden）。
**St01建議**：A。例：機台閒置時按 Clear，畫面看起來沒反應；測下一顆之後只有 Now 那欄有值，Last 1～9 變空。
**目前狀態**：已照建議先做，可推翻：commit `8314e3bd`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cObserver.cpp、D:\HT9045\web\page\ht9045_observer_wire.js、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebCmdGuard.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_webcmdguard.cpp、D:\HT9045\web\page\Data.Observer.html）。

### R55. Y 軸上下限框「拿到焦點＝打開鍵盤」（S116 殘項／todo E-009）
**背景**：golden 是點框才跳出鍵盤（OnClick）。網頁版用焦點判斷，Tab 經過框再離開也會送一次（照 golden 用框裡的字重設 Y 軸）。
**選項**：A 焦點就算／B 只有滑鼠點才算（但用 Tab 移進去直接打的字，會在下一次每秒更新時被蓋掉）。
**St01建議**：A。例：剛開窗時軸是 -5～105、框裡寫 100／0；用 Tab 經過上限框，軸上限就會變成 100（golden 要用滑鼠點框才會這樣）。
**目前狀態**：已照建議先做，可推翻：commit `8314e3bd`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cObserver.cpp、D:\HT9045\web\page\ht9045_observer_wire.js、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebCmdGuard.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_webcmdguard.cpp、D:\HT9045\web\page\Data.Observer.html）。

### R56. 狀態列「沒有數字」時用負數表示（S123 交件／todo E-006）
**背景**：S123 改成 C++ 只送數值、頁面組字（Steven Q1＝B）。有些時候還沒有數字（例如開機後第一盤還沒算出 UPH、Index 時間顯示關著）。
**選項**：A 用負數（`mainsb.uph` -1＝顯示「UPH = 」、-2＝空白；`mainsb.indexTime` -1＝空白），tag 型別固定是數字／B 用空字串，tag 會在數字和字串之間切換。
**St01建議**：A。例：開機後第一盤時 `mainsb.uph=-1`，畫面顯示「UPH = 」，跟 golden 一樣。
**目前狀態**：已照建議先做，可推翻：commit `4e49cbf6`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp、D:\HT9045\web\page\main.html）。

### R57. 狀態列版本號由 C++ 送整串（含尾碼）（S123 交件／todo E-006）
**背景**：golden 狀態列 [3] 的版本字是 asHandlerVersion 再依機台設定加尾碼（_SLT、_CCD…，golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:10689-10754）。
**選項**：A C++ 照 golden 組好整串送（開機一次）／B C++ 只送 asHandlerVersion，尾碼的旗標另外送、由頁面組。
**St01建議**：A——尾碼是機台設定決定的內容，不是顯示格式。例：[A38] 開時送 "V3.33_SLT"。
**目前狀態**：已照建議先做，可推翻：commit `4e49cbf6`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp、D:\HT9045\web\page\main.html）。

### R58. 狀態列 [1]／[4]／[7] 三格不補（S123 交件／todo E-006）
**背景**：golden 狀態列共 8 格（D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.dfm:4111-4143）。[1]、[4] 在 golden 永遠是空白（沒有有效的寫入點）；[7] 即時 UPH（"Net UPH"／"Curr UPH"）的來源 timer 在移植樹沒有被呼叫，沒有真值。
**選項**：A 不補這三格／B 照 golden 補齊 8 格，[7] 顯示 "---"。
**St01建議**：A。
**目前狀態**：已照建議先做，可推翻：commit `4e49cbf6`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp、D:\HT9045\web\page\main.html）。

### R59. 狀態列時間格 [6] 維持 Jimmy 的 clock.text 字串（S123 交件／todo E-006）
**背景**：Q1＝B 是「C++ 送數值、頁面組字」；時間格 [6] 目前是 Jimmy 的 `clock.text` tag，送的是 golden 格式的字串（例 "2026-09-27 10:00:00"）。
**選項**：A 不改／B 改成送數值、由頁面組字。
**St01建議**：A——時間字串本身就是 golden 的格式，而且是 Jimmy 的 tag。
**目前狀態**：已照建議先做，可推翻：commit `4e49cbf6`（這一格沒有改動；這顆 commit 改的是 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp、D:\HT9045\web\page\main.html）。

### R101. 網頁開設定頁時要不要照 golden 記「Enter …」事件（Q41 第 3 項交件）
**背景**：BCB 版每次打開設定視窗，都會在事件紀錄記一筆「進入某頁」（NewRecordProcess），例如 Contact MES2170、Bin MES2171、Ld_ULd MES2176、Speed MES2187（golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28306`／`:28323`／`:28452`／`:28694`）。網頁開頁目前完全不記；這次的「關窗尾段」只補了關窗後那一段。
**選項**：A 照 BCB 版，在網頁開頁（editlist.get）時記——網頁存完會自動重讀，所以每存一次會多記一筆／B 不記／C 只在操作員自己打開頁面時記、存完自動重讀不算（要 Jimmy 的網頁引擎多送一個旗標）。
**St01建議**：A——不用改網頁，多出來那筆從時間看得出是存完重讀。例：操作員打開 Speed 頁、改速度、存檔、關掉 → BCB 版一筆「Enter Speed」；A 兩筆；B 零筆；C 一筆。
**目前狀態**：**已裁決（Steven 20260927 17:xx，對話中直接回覆；RULINGS_20260926 S165）：「要記」＝A**——網頁開設定頁（editlist.get）時照 golden 記「Enter …」事件（NewRecordProcess），存完自動重讀多一筆接受。待實作（C 路開頁共用層那位工程師交件後派）。（先前紀錄：等 Steven 決定，程式沒動。）　**已做 `342779cc`**（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp` 檔尾 kOpenEnters 21 列＋PageJson／Teach／Offset_File／IniConfig 呼叫點；ctest OpenEnterLog 50/50）；只在這一頁伺服器端還沒開著時記，所以存完自動重讀不會重複記（Configuration 例外，見 R110）。衍生 R108～R110。

### 20260928～29 Steven 裁決的 St01 題（20260929 07:4x 從 decisions-pending.md 搬來；RULINGS_20260926 S167～S169）

### Q44. 按 Exit 時「必須先停下才能關閉」要怎麼做（「頁面標未停、照樣關」〔R39〕被推翻之後的做法／Steven 20260927 的裁決 S163「必須先停下才能關閉」／關程式時寫檔與停機的待辦〔todo D-007〕）
（這題是 R39——「SIM 建置接真卡時，Exit 關不掉 1203 頁打開的輸出，頁面標『未停』照樣關」——被 Steven 20260927 11:xx 的裁決 S163（Exit 要確定輸出都停了才能關閉）推翻之後，要 Steven 選的做法。R39 原文與 S163 原話在 `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md` 的 R39。）

**背景**：
- 白話：操作員在網頁主畫面按 Exit，現在程式會先存生產資料、再照 BCB6 的順序送停機命令，然後結束；但停機的結果只印在主控台、頁面看不到，而且**不管有沒有真的停下來都會結束**。SIM 建置（模擬用的建置）接真的 1203 卡（PCIE-1203 EtherCAT 卡）時，從網頁 1203 頁打開的輸出在 Exit 之後還是開著。Steven 要的是：確定輸出、馬達都停了才准關。
- 現在 Exit 怎麼走（移植樹＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0`，V906 的 C++ 版；頁面在 `D:\HT9045\web\page`）：
  1. 頁面 `D:\HT9045\web\page\ht9045_main_close.js:138-211` 分三步送關程式指令（act.main.closeProgram）；C++ 是 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp:1180-1383` 的 `W906_Main_CloseProgramOp`。第 0 步（step 0）先過 golden 的關程式守衛（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp:368-414` 的 CloseGuards：運轉中、KYEC 條碼、低權限時馬達要在原點、實跑時機台內有 IC），再問 "Sure close??"；第 1 步存 BinCount／lastdata／machinerecord／RunMode／LotSummary 後問 "Sure To Exit?"；第 2 步存 JamRate／TimerRecordLoaderDate／WriteCTInfo，然後設結束旗標（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp:1356-1357`），回給頁面的是關站段的**預估**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp:1358` `ShutdownSequence(false)`，phase=plan）。
  2. wb_serve（移植樹的網頁伺服器程式）的主迴圈晚一圈才離開（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5953` 的 `W906_ServeQuitDue`），**離開主迴圈之後**才照 golden FormClose 的順序停機（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5964` → `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp:1082-1096` → `ShutdownSequence(true)` `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp:675-969`，對照 golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:11881-12474`），接著寫 `Program Close=1`（寫進 `D:\HT9045\system\Gerneral.ini` 的 [Record]，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5965`）、`server.Stop()`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5967`）。
  3. 所以停機是在「已經決定要結束、連線快斷」之後才做；做完的結果只印在主控台（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp:1085-1095`），頁面看不到，而且**不管有沒有停下來都會結束**。頁面的「未停」清單（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp:995-999`、`D:\HT9045\web\page\ht9045_main_close.js:94-100`）只是預估。
- 為什麼 SIM 建置接真卡時關不掉：SIM 建置（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\MachineType.h:63-65` 預設開 SOFT_SIMULTE）裡，翻譯過來的機台程式（引擎）的 IO 走模擬後端，引擎 IO 到 1203 卡的路由不裝（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\Pci1203IoRoute.cpp:347-348`），golden 的關輸出 `SW[].Off()` 只寫到模擬值（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp:549-551`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp:584`）。但 1203 的命令面（網頁 1203 頁直接對卡下命令的那一層）在 SIM 建置照樣編進去，而且是 LIVE（真的送到卡，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\MachineType.h:105-106`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\MachineType.h:121`）；1203 頁的「設輸出位元／位元組」指令（`pci1203.do.setBit`／`setByte`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5791` → `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:6236-6300` `W906_Dispatch1203Ex`）直接打到卡（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\Pci1203Control.cpp:1672-1692`）。
- 出貨建置（非模擬）也有漏網的：路由裝上後 `SW[].Off()` 會真的到卡，但沒有引擎物件的 1203 輸出（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\MachineType.h:1762-1764` 列的 54 個：`C_*Off` 第二線圈、`C_*EdgeClip`、`C_*DrawerLock`、`C_InPnPDrop1-4`…）與 1203 頁直接打開的點，golden 順序裡沒有任何一步會關。
- 目前「未停」的判法會讓 Exit 永遠關不掉：有些項目**每一台都固定是未停**，原因是移植樹沒翻，不是真的有東西開著——例如 EP 電壓歸零（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp:746-748`，ADAM 空殼，不分機台一律 stub＝只有空殼）、Index kit 吸嘴（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp:801-802`，一律 missing＝沒翻）、StopAllMotor 的非 1203 馬達卡（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp:896`，一律 unverified＝送了但看不到結果）。如果「有任何未停就擋」，每一台都會卡在關不掉。所以要把「有證據還開著」跟「看不見」分開。

**(a) 照 golden 順序之外要加的東西**（全部是移植樹的做法、偏離 golden；golden 自己的順序一步不改）：
1. **關站段搬進主迴圈、在連線還在時做**：`W906_ServeQuitDue`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5953`）那一拍不直接 break，先跑停機＋讀回確認，結果送回頁面；全部確認停了才 break、寫 `Program Close=1`、`server.Stop()`。
2. **關站中只放行「關」與「停」**：結束旗標設了以後，所有輸出／移動命令只收「設成 0」「軸停止」「馬達停止（motor.stop）」，任何「打開」或「移動」一律拒絕並回原因（入口在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:6199-6200` 的輸出命令表、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:6213` `W906_DispatchIoClick`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:6236` `W906_Dispatch1203Ex`）。不然別的分頁可以在關站途中又把東西打開。
3. **golden 最後一步之後加「1203 輸出清零」**（見 (b)），清零範圍見 Q44-1。
4. **1203 軸停止後讀回**：既有的 `W906_Stop1203AllHook`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp:630-661`，減速停止＋ExtDrive 0）之後，看監看器的軸狀態（`Acm_AxGetState`，READY＝1，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\vendor\AdvMotDrv.h:793-798`）與命令速度，確認每一軸都停了。照 R38（Steven 已同意的「Exit 停機不加 golden 沒有的伺服 OFF／切馬達動力電」）不做伺服 OFF。
5. **讀回**：清零與停軸之後跑一次監看器 Poll（只讀；Poll 前後把「輸出優先」掛鉤拿掉，做法同 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:7627`），再判定。
6. **判定＋擋**：照 (c) 的分類；有「擋」類就不結束。
7. `Program Close=1` 維持「真的要結束時才寫」。

**(b) 1203 命令面能不能在關閉時把 DO（數位輸出）全關**：能，**用現有的 API 就做得到，不用改 EastSun 的檔**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\Pci1203Monitor.h`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\Pci1203Monitor.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\Pci1203Control.h`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\Pci1203Control.cpp` 只讀、只呼叫）：
- 讀：`Pci1203Monitor()->doCount()`／`do_(k)`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\Pci1203Monitor.h:1436-1439`；每個 DO byte 帶 ring（EtherCAT 環）／站號／站內 byte／目前值，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\Pci1203Monitor.h:1148-1175`）。
- 寫：`Pci1203Control()->Execute(kCmdDoSetByte, port=k, value)`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\Pci1203Control.cpp:667-672` 驗證，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\Pci1203Control.cpp:1683-1694` 用 ring／站號送 `Acm_DaqDoSetByteEx`，成功後自己排讀回 `ForceDoReread`）。
- 寫之前先過路由的檢查 `Pci1203RouteCanWriteBit`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\Pci1203IoRoute.cpp:332-341`，本體 `CheckWrite_` `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\Pci1203IoRoute.cpp:119-179`：ring 0 拒寫 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\Pci1203IoRoute.cpp:144-149`、驅動器站拒寫 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\Pci1203IoRoute.cpp:171-176`）：命令面沒武裝、卡沒開、**ring 0（馬達環）**、對不到 byte、**驅動器站**一律不寫。⚠ 這一步不能省：命令面自己的 DO 路徑**不擋驅動器站**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\Pci1203Control.cpp:654-672` 只驗 port／bit 範圍），ring 0 的 DO byte 是伺服的 RxPDO（伺服收的控制資料），寫 0 可能清掉伺服的控制字。
- 限制：(1) 只有命令面 LIVE 時才真的到卡（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\MachineType.h:106`）；DRY RUN（命令只記錄、不送到卡）或沒武裝 → 那些點算「未停」。(2) 廠商 API 只能在主執行緒呼叫（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\Pci1203Control.h:800-803`），所以 Ctrl-C 的處理函式只能設旗標，由主執行緒做（見 Q44-5）。(3) 這台電腦（Steven01）沒有 1203 卡：監看器沒開過 → 沒有任何真實輸出 → 這一步算「這台沒有」，不擋；**這次開機曾經開過卡、後來失聯**才算「未停」。
- 需要 EastSun 確認（不改他的檔）：ring 1 各 IO 站清零有沒有副作用、保留清單（Q44-1）要放哪些點、從站看門狗在程式結束時會不會自己清輸出（Steven 已裁決的「主控台 Ctrl-C／按 X 也要走停機」那題〔Q39〕背景提過，repo 沒有量測紀錄）。

**(c) 有未停時擋關閉**：
- 三類：
  - **擋**（有證據還在出力，或停止命令失敗）：① 1203 DO 讀回仍是 1 的位元（ring ≥1 的 IO 站、保留清單以外）；② 1203 軸讀回還在動，或停止命令被拒／廠商回錯；③ 經路由的 `SW[].Off()`／`IndexMotorBreakerOFF()` 被拒或廠商回錯（現在標 failed 的那些）；④ 這次開機開過 1203 卡、現在讀不到（清不了也確認不了）。SIM 建置 `SW[].Off()` 只到模擬值的那些（現在標 stub）不直接擋，改看 ① 的真卡讀回。
  - **要操作員確認**（看不見，但這台有這個裝置）：`stub`（只有空殼）／`missing`（沒翻）／`unverified`（送了但看不到結果）而且這台真的有裝的——例如 EP 電壓沒歸零（EP_Install≠0）、ESD 程式沒開所以沒收到停止（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp:760-775`）、HonPrec／ATC6.0 沒送 STOP（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp:787-794`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp:935-941`）、Real Time CCD 光源、Galil 沒重置、MN200、Air Stream、Index kit 吸嘴、非 1203 馬達卡的 StopAllMotor。每一項都寫出「為什麼看不到」。這一類的處理見 Q44-2。
  - **不擋**：`done`（已停）與 `noop`（這台沒有）。
- 頁面（`D:\HT9045\web\page\ht9045_main_close.js`）：
  1. 按 Exit（第 0 步）時，C++ 先做一次預檢，第一個確認框就列出「現在還開著的輸出／在動的軸（關閉時會關掉）」與「關閉時看不到、要你確認的裝置」。
  2. 兩個確認框都答「是」之後，蓋層顯示「機台停機中…」；C++ 在主迴圈裡停機、讀回，把**實際結果**（不是預估）送回頁面。
  3. 全部停了：蓋層改「已結束」，跟現在一樣。
  4. 有「擋」類：蓋層改成紅色「無法關閉：以下還沒停」，逐項列出（點名、ring／站號／bit、讀回值、原因），下面三個鈕：**［重試停機］**（再跑一次清零＋停軸＋讀回）、**［強制關閉］**（Q44-3）、以及一行字「可以到 1203 頁或 IO 頁把它們關掉；關站中只收『關』與『停』」。
- 操作員怎麼先停、之後怎麼再關：到 1203 頁／IO 頁按「關」（關站中照樣收）或處理硬體（例如卡片重新上電、按 EMG）→ 回主畫面蓋層按［重試停機］→ 讀回全停就結束。程式在這段期間停在「關站中」：狀態機已停（golden `SoftStop=true`、`InitialOK=false`、`bSystemClose=true` 都已經設了，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp:751-753`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp:861-862`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp:932-933`），**不能取消關閉回去生產**（golden 答「是」之後程式一定結束），要生產只能重開 wb_serve。

**選項**：
- **A 自動關＋讀回確認＋擋**：關閉流程自己把 1203 輸出清零、停軸，讀回確認；關不掉的才擋，操作員處理後按［重試停機］。例：見下面 St01建議的例 1～例 3。
- **B 只擋、不自動關**：第 0 步預檢到有開著的輸出就不讓關，列出清單要操作員先去 1203 頁／IO 頁關掉，再按 Exit；關閉流程只照 golden（不加清零）。例：見下面例 1 的「照 B」。
- C 只自動關、不擋（清零後不管讀回照樣結束）——不符合 S163（必須先停下才能關閉），只列對照。例：關站途中清零命令被卡片拒絕，程式照樣結束，SwHeaterRelay 可能還開著，頁面也不會說。

**St01建議**：A。理由：B 要操作員一個一個點掉 1203 頁的輸出，而且 golden 自己會關的加熱器繼電器、風扇在 SIM 建置也得手動關；A 只有真的關不掉時才需要人。
- 例 1（SIM 建置接 HT9050 真機）：筆電從 1203 頁打開 SwHeaterRelay 和 SwHeaterFan，按 Exit → 第一框列出「還開著：SwHeaterRelay、SwHeaterFan（1203 卡讀回＝1），關閉時會關掉」→ 答兩次「是」→ golden 的 `SW[].Off()` 只到模擬值 → 加的清零把那兩個 byte 寫 0 → 讀回 0 → 結束。照 B：第一框就擋「請先到 1203 頁關掉 SwHeaterRelay、SwHeaterFan」，關完再按一次 Exit。
- 例 2（關站途中卡片失聯）：清零被拒（「1203 卡沒有開，或監看器已停止輪詢」）→ 蓋層紅字「無法確認輸出已關（1203 卡沒有回應）」→ 操作員照機台程序按 EMG 或斷電 → 按［強制關閉］（Q44-3）→ 程式結束，log 記「強制關閉，未停：…」。
- 例 3（HT9045、有 EP）：沒有「擋」類，只有「EP 電壓沒有歸零（ADAM 沒翻，看不到）」→ 蓋層要操作員勾「我已確認」（Q44-2a）→ 結束。

**A 裡要 Steven 選的細節**（沒寫到的照建議）：
- **Q44-1 清零範圍**：**a ring 1 所有 IO 站的 DO 全清，保留清單除外；保留清單預設只有 SwMotorRelay**（R38 照 golden 不切馬達動力電；HT9050 表 `D:\HT9045\machines\HT9050\IO_Table.csv:86` 這一點是 1203 輸出，ISABase=3）／b 只清「1203 頁這次開機以後打開過、現在還是 1」的位元（要在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:6236` `W906_Dispatch1203Ex` 記錄，是機台端的段）／c 全清、不保留（等於推翻 R38）。建議 a。例：a＝SwHeaterRelay、SwHeaterFan 和其他 ring 1 輸出都清成 0，只有 SwMotorRelay 維持原狀；b＝只清這次從 1203 頁打開過的 SwHeaterFan，不是從 1203 頁打開、golden 順序也不關的輸出維持原狀；c＝連 SwMotorRelay 也清成 0，馬達動力電被切掉。⚠ 全清的副作用要 Steven／EastSun 知道：真空關掉，吸嘴上的 IC 會掉（關程式守衛 CloseGuards 只在實跑時擋機台內有 IC，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp:399-411`）；單線圈氣缸回到彈簧位置會動；門鎖會放開。要保留的點請加進保留清單。
- **Q44-2 「看不見」的項目**：**a 列出來、操作員勾「我已確認」才關**／b 一律擋（那些還沒翻之前，那台就永遠關不掉）／c 不擋也不問。建議 a。例（HT9045、有 EP）：a＝蓋層列「EP 電壓沒有歸零」，勾「我已確認」就結束（上面例 3）；b＝ADAM 翻好之前每次按 Exit 都被擋，這台永遠關不掉；c＝直接結束，EP 沒歸零也不提醒。
- **Q44-3 關不掉時的出口**：**a［強制關閉］鈕：權限要到 `LevelSet.AccessLevel[6]`（golden Exit 的同一個等級門檻，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp:383`），再確認一次，log 記「強制關閉，未停：…」**／b 沒有出口，只能 EMG 或工作管理員。建議 a（卡片失聯時讀回永遠不會成功）。例：a＝上面例 2；b＝同樣卡片失聯，蓋層一直紅字，只能按 EMG 或用工作管理員結束 wb_serve。
- **Q44-4 無人值守跑固定秒數就結束的參數 `--seconds` 到期（R40：到期結束時也照 golden 停機）**：**a 同樣自動關＋讀回；關不掉就等 1 秒重試一次，還是不行就照樣結束，但 exit code 回 3、主控台列出未停**／b 不結束，一直等到停下來。建議 a（`--seconds` 是無人值守的探針與 gate 在用，沒有人能處理；exit code 讓腳本知道失敗）。例：探針用 `--seconds` 跑，到期時某個 1203 輸出讀回仍是 1 → a＝重試一次仍不行，照樣結束、exit code 3，腳本判失敗；b＝wb_serve 一直不結束，腳本卡住。
- **Q44-5 Ctrl-C（Steven 已裁決 Exit 與 Ctrl-C 都照 golden 順序停〔Q39＝A〕，筆電做）**：**a 第一次 Ctrl-C 只設結束旗標（處理函式回 TRUE、不讓系統殺掉行程），主執行緒走跟 Exit 一樣的停機＋讀回（不問兩個確認框，golden 這條路本來就沒有）；關不掉就不結束，主控台列清單並提示「10 秒內再按一次 Ctrl-C＝強制結束（輸出維持現狀）」**／b 關不掉就一直不結束，只能關視窗。建議 a。現在的處理函式只寫 `Program Close=1` 就交給系統結束（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:6153-6159`）。例：主控台按 Ctrl-C，SwHeaterFan 讀回仍是 1 → a＝主控台列出 SwHeaterFan、提示 10 秒內再按一次＝強制結束；b＝一直不結束，只能按主控台視窗 X。
- **Q44-6 按主控台視窗 X、登出、關機**：系統大約 5 秒後一定強制結束，擋不住。**a 處理函式設旗標後最多等約 4 秒，主執行緒先做「停輸出」（清零、停軸、煞車、加熱器繼電器），再存檔，超時就算了並在 log 記未確認**／b 維持只寫 `Program Close=1`。建議 a。例：按主控台 X → a＝約 4 秒內先清零、停軸、煞車、關加熱器繼電器，存檔沒做完就被系統結束，log 記「未確認」；b＝什麼停止命令都沒送，輸出維持原狀。

**分工**：
- St01：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp`（分類、清零、讀回、重試／強制、結果送頁面）、`D:\HT9045\web\page\ht9045_main_close.js`（預檢清單、紅色蓋層、三個鈕）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5953`／`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5964` St01 自己那兩行的行尾（把停機搬進主迴圈）；另外提供一支「現在就停輸出」的函式給筆電的 Ctrl-C 用。
- 機台端／筆電：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:6199-6200`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:6213`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:6236` 三個輸出入口各插一行「關站中只收關與停」（照 20260927 08:3x 的前例，同一行插入、不改其他行，事後知會）；Q44-5／Q44-6 的主控台處理函式（Q39 筆電在做）。
- EastSun：不改檔；確認 (b) 最後一點的三件事與 Q44-1 保留清單。
- Jimmy：警報框開著時的三個等待迴圈要一起看結束旗標（Q39 已提）；「看不見」類裡 ADAM／ATC／ESD／Galil 翻好一項，就少一項要人確認。

**工作量與順序**：St01 約 1 天（MainClose 改寫＋頁面蓋層＋兩行 wb_serve）；驗證：SIM 建置在這台（沒有 1203 卡）只能驗「沒有卡＝不擋」與蓋層流程，**真卡清零、讀回、擋關閉要在 HT9050 機台上驗**（要有人在機台旁；先用一個無害的點，例如 SwHeaterFan，照 Q39 建議的低能量量測）。順序：① Steven 選 A／B 與 Q44-1～Q44-6 → ② EastSun 回保留清單與清零副作用 → ③ St01 做 MainClose／頁面、機台端插三行、筆電做 Ctrl-C → ④ 兩組態（出貨＋模擬）完整建置測試（gate）→ ⑤ 機邊驗證。

**目前狀態**：**Steven 20260927 11:xx 裁決方向（RULINGS_20260926 S163「必須先停下才能關閉」）**；做法等 Steven 選 A／B 與 Q44-1～Q44-6，程式沒動。在那之前 Exit 維持 `a684f171`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp`、`D:\HT9045\web\page\ht9045_main_close.js`）的行為（頁面標「未停」、照樣結束）。
**Steven 20260928 已裁決**：「Q44 bcb還沒移植的都先列為待辦, 至少馬達跟溫度的要停下來, 這個只要兩個命令就可以做到了」（08:0x（在 ST01-M 對話裡））
ST01-E 再用選項題問 Steven（09:0x），三題都選建議：① 兩個命令＝停馬達（StopAllMotor，加上 1203 卡這次開機開過的每一軸的停止）＋關加熱器繼電器 SwHeaterRelay（SIM 建置接真卡時用 1203 的 DO 寫入關掉）；② 讀回還沒停就不關閉，給［重試停機］和［強制關閉］（等級照 Exit 的 `LevelSet.AccessLevel[6]`，log 記一筆）；③ Q44 其餘（選項 A 的其他部分、Q44-1～6、所有 BCB 還沒移植的項目）列成待辦，不當 Exit 的擋關條件。
⇒ 意思：Exit 先做「最小停機」：停馬達＋關加熱器繼電器，讀回確認停了才關，停不了就重試或有權限的人強制關閉；其他都排進待辦。
（行號是 20260927 移植樹工作樹 `v906/steven-cbridge-review6` 當下的。）
**附帶一題（R39 留下、還沒問到）**：模擬版程式（SIM 建置：給筆電測試用、平常不會真的控制機台的那一版）如果接在真機上，網頁的 IO 頁（1203 卡頁面）可以直接打開真的輸出，例如加熱器繼電器；按 Exit 關程式時，模擬版的關機步驟關不到它。要不要另外定一條規定「模擬版接真機時不通電加熱」？例：定了 → 筆電用模擬版接真機時只測不通電的動作，加熱一律用出貨版測；不定 → 照這一題（Q44）選的做法處理，Exit 時擋下或自動關掉。St01建議：定（多一道人為規定，跟 Q44 的程式保護疊在一起）。原題與 Steven 的裁決 S163 在 `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md` 的 R39。回法：「R39 附帶＝定」或「不定」。
**Steven 20260929 07:2x 補充**：「Q44. SOFT_SIMULTE因為馬達不會真的動作, 所以關閉時沒有限制, 機台上只要c++有回復 StopAllMotor() 是已經發送, 且加熱io也有off, 就可以當成已停機」⇒ 模擬版按 Exit 沒有任何限制；機台上「已停機」＝C++ 回報 StopAllMotor() 已送出＋加熱器 IO（SwHeaterRelay）已關，不用再讀回各軸；［重試］／［強制關閉］只留給送不出去或關加熱器失敗時。ST01-E 照這個改 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp`。
**已做（commit `5789eeea`，ST01-E 20260929 08:5x）**：模擬版按 Exit 不擋；機台上停機命令送出、加熱器關閉寫入成功就算已停，只有命令送不出去（而且當下讀回也不是停）才擋。測試 MainCloseStop 101／101 通過。

### Q4-2. （Steven 選「先把三頁搬到第二型再退役」〔S160〕的追問）三頁已經改用「C++ 照 BCB6 開畫面程式算好再送網頁」的做法（C 路）顯示、第一型也已經從 main 退役——請確認不必再「搬到第二型」（第一型要不要退役那一列待辦〔todo F-006〕）
**背景**：
- 網頁要顯示「BCB6 打開這個畫面時算出來的值」（強制值、換算、依機種藏起來或鎖住的欄位），不能只顯示設定檔原字。移植樹先後有三種做法：第一型（20260924，4 頁：C++ 只跑 BCB6「把檔案值填進畫面」那一段〔DoIniDataToForm〕，網頁讀完設定檔後再用這份值蓋上去，走 `/api/form`）、第二型（C++ 在網頁伺服器收 HTTP 請求的執行緒裡跑 BCB6 的讀檔程式〔A 形狀 FormBridge〕，現在只剩 HotPlate）、C 路（20260924～25 起，現在 20 多頁都用它：開頁時 C++ 照 BCB6 的開畫面程式整段跑一次再送給網頁，存檔也照 BCB6 的存檔鈕流程）。
- St02 09:24 看到網頁引擎 `D:\HT9045\web\page\ht9045_wire_engine.js:1554` 有一段（formOverlay）會問第一型，以為 Setup.Speed、Setup.TrayAssignment、Config.DIOInterFaceCFG 三頁靠它，Steven 10:4x 因此選 B（Steven 20260927 的裁決 S160：先把這三頁搬到第二型再退役）。
- 逐行查證：三頁在同一支檔 `D:\HT9045\web\page\ht9045_wire_engine.js:1042`、`D:\HT9045\web\page\ht9045_wire_engine.js:1044`、`D:\HT9045\web\page\ht9045_wire_engine.js:1048` 登記成 C 路，開頁在 `D:\HT9045\web\page\ht9045_wire_engine.js:1299` 就轉去 C 路（開頁時向 C++ 要畫面值的指令 editlist.get），**根本不會執行那一段**；只有網頁直接讀設定檔的舊做法（B 路）的頁（`D:\HT9045\web\page\ht9045_wire_engine.js:1453`）和 `D:\HT9045\web\page\ht9045_contact_wire.js`、`D:\HT9045\web\page\ht9045_hotplate_wire.js` 會呼叫 formOverlay。St02 10:59 也查到同一件事。
- St02 照 Steven「使用新的做法搬過去後，就把第一型退役」，判斷「新的做法」就是 C 路，已退役第一型（`834fcc78`：刪 4 個產生檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\gen\form_*.gen.cpp` 與 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\gen_formjson.py`〔都已刪除〕，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\FormJson.cpp` 430→211 行）；Jimmy 14:07 合進 main（合併 commit `ed7df426`，帶進 `834fcc78` 同一批檔），兩組態（出貨＋模擬）完整建置測試（全量 gate）通過。St01 分支還沒合到這顆。
- 三頁逐欄核對：第一型會送的每一欄 C 路都有（Speed 116／116、TrayAssignment 50／50、DIO 11／11），C 路另外多送第一型漏掉的 49 欄（Speed 30 個上下微調鈕 TUpDown、TrayAssignment 19 欄），以及 BCB6 開畫面時的強制值。C++ 沒有對應表單程式的頁，`/api/form` 回 404，引擎靜默略過（`D:\HT9045\web\page\ht9045_wire_engine.js:1650`），頁面不會壞。
**選項**：
- A 確認 S160 的「搬過去」就是指三頁已經搬到 C 路，不做第二型；第一型維持已退役（main 現況）。St01 只剩合 main、文件同步、上機把三頁各開一次。例：見下面 St01建議的 Setup.Speed 例子，選 A 畫面顯示正確的 100。
- B 照 S160 字面替三頁另做第二型再退役：要 Jimmy 改引擎才會生效、存檔仍走 C 路（同一頁兩套顯示程式）；第二型在網頁伺服器的 HTTP 執行緒跑 BCB6 讀檔、會改到機台執行中的資料，DIO 頁檔案不見時還會把 SystemStart（機台運轉中旗標）清成 false——這正是 20260924～25 退役這三頁第二型的原因（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\formbridge\_retired\README.md`）；估 1～2 人天＋Jimmy＋上機，畫面不會更正確。例：Config.DIOInterFaceCFG 開頁改由第二型讀檔，那台的 DIO 檔剛好不見 → SystemStart 被清成 false；Setup.Speed 的 Index Arm 加減速顯示的值也不會比 C 路的 100 更正確。
- C 撤回退役（還原 `834fcc78`）：沒有頁面在用第一型，等於留一條沒人走的舊路。例：還原後 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\FormJson.cpp` 回到 430 行、4 個產生檔回來，但三頁開頁照樣在 `D:\HT9045\web\page\ht9045_wire_engine.js:1299` 走 C 路，這些程式沒有人呼叫。
**St01建議**：A。例：Setup.Speed 的「Index Arm 加減速」（`edIndexAccDec`），配方檔寫 80；BCB6 開頁先填 80、再強制改成 100（golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSpeed.cpp:64`「Index ACC & DEC must be 100%」）。只讀設定檔＝80（錯）、第一型＝80（錯，只做了填檔案值）、C 路＝100（對）。選 A，畫面沒有任何一格變差。
**目前狀態**：等 Steven 確認。查證全文：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q4_TYPE1_RETIRE_CHECK_20260927.md`；⛔ 20260927 22:xx 更新：St01 分支早已包含 `ed7df426`／`834fcc78`（`1de5005b` 合 main 時帶進來，主要檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\FormJson.cpp`），Jimmy 也已把 St01 合進 main（`53a55b35`）——這一步已經做完，不影響這題。
**Steven 20260928**：「Q4-2. 是已經做完了吧?」（08:0x（在 ST01-M 對話裡））；ST01-M 回：程式已做完（`834fcc78`／`ed7df426` 已在 main 與 St01 分支），只剩上機把三頁各開一次，請 Steven 回「Q4-2 選 A」結案——**還沒回**。
**Steven 20260929 07:3x**：「Q4-2. 你說了一堆是在繞口令嗎? 工作就是把原本使用第一型的頁面換成新的方式, 然後就可以把第一型態退役了,」⇒ **已做完、結案**：三頁已改用新方式、第一型已退役（`834fcc78`，已在 main）。

### Q45. 設定頁上「要密碼才能改」的 7 個地方，網頁版要怎麼做（Steven 要先補既有頁面的 golden 事件與等級檢查〔Q41，Steven 20260927 的裁決 S158〕，盤點出來的「密碼確認還沒接」〔Q41 盤點 C-3〕；相關裁決 S41「密碼資料不可暴露給瀏覽器」、S55「名單可以，密碼需要加密」、S130「維持現狀：密碼不進訊息內容」）
**背景**：BCB 版（golden V912，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`）有 7 個設定要先輸入密碼、或重新登入到夠高的等級才能改；網頁版目前一律「改不了」（網頁的密碼框指令 `dialog.auth` 回「還沒接」，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4889-4904`）。
| # | 在哪 | golden 怎麼驗 | golden 出處 | 移植樹現況 |
|---|---|---|---|---|
| 1 | Configuration C12（PE 模式）；C13／C14／C17／C24 共用同一支處理器（C12 勾著時改它們也要問） | 程式寫死的密碼（每個版本不同） | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6775` | 改了就整次拒存（`a8eca460`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp:139-233`） |
| 2 | Configuration A27 | 程式寫死的密碼 | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6824` | 同上 |
| 3 | Configuration N07-5（員工 ID 檢查） | 程式寫死的密碼 | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6860` | 同上 |
| 4 | Configuration M01 群組（16 格） | 重新登入、等級要到權限表第 92 格（`LevelSet.AccessLevel[92]`）設的值（92 號設 0 就不問；只有裝了 RTC 的機台會問）；問完把使用者登出成 Operator | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6531` → `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6482` | 同上 |
| 5 | Configuration A32_1 群組（Check List） | `D:\HT9045\system\SG_PW.ini` 裡的密碼（沒有檔案會先寫一份預設的） | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:7274` | 群組一直藏著，本來就改不到 |
| 6、7 | SetUp 關掉 RTC（Real Time CCD）、關掉 OCR | 重新登入、等級要到權限表第 37 格（`LevelSet.AccessLevel[37]`）設的值（只有裝了 RTC 的機台會問） | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:3573-3606` → `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:4325` | 當作密碼錯：把勾改回來、其他照存 |
**選項**：
- A 網頁輸入密碼，加密後只送本機 C++ 比對：伺服器每次發一個只能用一次的亂數，網頁用「密碼＋亂數」算出摘要送回，C++ 用自己的比對目標算一次比對；#4、#6、#7 的「重新登入」改走現有網頁登入流程。做得到：密碼不出現在訊息與除錯紀錄裡、攔到摘要也不能重送、伺服器只聽本機 127.0.0.1（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridge\WebBridgeServer.cpp:381`）。做不到：擋不住能讀程式的人（golden 本來就把密碼寫在 BCB 執行檔裡，C++ 要比對也得帶同一個目標）、擋不住旁邊看人輸入。瀏覽器的雜湊功能只在 https 或 http://127.0.0.1／localhost 可用。符合 S55「密碼需要加密」與 S130（「密碼傳輸本身要不要另外加密」那題〔Q8〕選 A）「密碼不進訊息內容」；對照：現有網頁登入（auth.login）是把密碼直接放在訊息裡（`D:\HT9045\web\page\ht9045_recipe_client.js:543-551`，Jimmy 的檔），做 A 的話建議登入也一起換、另開題。工作量最大，golden 的副作用也要照做（#4 登出成 Operator、#6／#7 記「RealTimeCCD Disabled」）。例：見下面 St01建議的例 1。
- B 不用 golden 寫死的密碼，改成「目前登入者的等級夠高就可以改」：#4 用 92 號、#6／#7 用 37 號（跟 golden 同）；#1／#2／#3／#5 golden 沒有等級，要另外定。風險：#1～#3 是客戶（KYEC）要求「另外一組廠商密碼」才能開，B 等於交給任何高等級帳號；golden #4／#6／#7 即使已是高等級也要重新登入（防有人走到沒登出的畫面前改），B 少了這一層。例：任何等級夠高的帳號勾 C12（PE 模式）直接就能存，不必知道 KYEC 要的那組廠商密碼；工程師沒登出就離開，別人走過來也能改 M01。
- C 維持現狀：網頁上改不了（網頁說明原因），要到裝 BCB 版的機台上改；只跑 V906 的實驗機沒有別的辦法（手動改設定檔會繞過所有檢查，不建議）。例：見下面 St01建議的例 2。
**St01建議**：#4、#6、#7（本來就是「重新登入＋等級」）選 A，走現有登入流程，不牽涉任何寫死的密碼；#1、#2、#3、#5（寫死的密碼或 `D:\HT9045\system\SG_PW.ini`）先選 C，等有客戶真的要在網頁上切再做 A。不建議 B。例 1（A，#4）：裝了 RTC 的機台，工程師勾 M01_03 → 網頁跳「請重新登入（等級要到 92 號設定的值）」→ 登入成功、等級夠 → 存檔 → 照 golden 登出成 Operator。例 2（C，#1，已做）：勾 C12 → 按 Save → 網頁顯示「這次沒有存檔：C12（PE 模式）打勾要密碼……請改回原值再存」→ 取消 C12 → 其他設定就能存。
**目前狀態**：#1～#4 網頁一律拒存（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp:139-233`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp:306-314`，`a8eca460`：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp`）；#5 本來就改不到；#6、#7 當作密碼錯。等 Steven 決定。
**Steven 20260928 已裁決**：「Q45.  按照你的建議執行」（08:0x（在 ST01-M 對話裡））
⇒ 意思：照 St01 建議——#4、#6、#7 選 A（走現有網頁登入流程重新登入＋等級）；#1、#2、#3、#5 先選 C（網頁上改不了並說明原因）；子題 Q45-1～10 也照建議（1A 2A 3A 4A 5A 6A 7B 8A 9A 10A）。Q45-3 要另外通報 Jimmy 那是 golden 的問題。
**評估案（20260927 夜間，St01）**：`D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\q45-web-password.md`——7 個點其實是 3 種檢查：(甲) 重新登入、要到某個等級（#4、#6、#7）；(乙) golden 寫死的一組廠商密碼（#1～#3 共用）；(丙) `D:\HT9045\system\SG_PW.ini` 解鎖一個隱藏區塊（#5）。建議「甲現在做、乙丙維持改不了」（St01 約 5 人天、Jimmy 約半天）；全部做加一次性亂數摘要約 13 人天。細節拆成 10 個子題（全文在評估案第 8 節，每題有背景、選項、例子）：
- Q45-1 重新登入那一次的密碼怎麼送給 C++——建議 A：跟主畫面登入一樣只走本機（127.0.0.1）送原字，訊息欄位先留好以後可換。
- Q45-2 Configuration「監控功能」16 格什麼時候問——建議 A：按存檔時問一次，涵蓋這次改的全部格子（golden 是每點一格問一次）。
- Q45-3 問完 golden 一律登出成 Operator，跟客戶選項「切到 Operator 後不能存設定」打架（開了這個選項的機台，監控功能永遠存不進去）——建議 A 照 golden，另通報 Jimmy 這是 golden 的問題。
- Q45-4 登入框按「取消」算什麼——建議 A 照 golden：算打錯，勾選改回、登出成 Operator（小鍵盤上先寫明）。
- Q45-5 沒有密碼本（下拉選單選等級）的機台，golden 兩個比對怪處要不要照翻——建議 A 照翻。
- Q45-6 密碼錯太多次要不要暫停（golden 沒有）——建議 A 不做。
- Q45-7 （只有要做 #1～#3 時）廠商密碼的比對目標放哪——建議 B：程式裡只放雜湊值，不放密碼原字。
- Q45-8 主畫面登入要不要也改成不送原字——建議 A 不改（維持 Steven Q10＝B1「網頁傳輸維持現狀」）。
- Q45-9 （只有選了不送原字時）網頁用什麼算摘要——建議 A 網頁自帶一份程式（用區網 IP 開 HMI 時瀏覽器內建的不能用）。
- Q45-10 Jimmy 的舊文件 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RECON_cConfiguration_displayside.md` 第 132、268 行寫著 golden 的廠商密碼，要不要遮掉——建議 A 遮現行檔（同一個 repo 的 golden 原始碼本來就有這段字，git 歷史要不要清由 Steven 判斷）。
回法：「Q45 子題照建議」，或逐題寫 Q45-2＝B 這樣。
**Steven 20260929 07:3x**：「Q45. 請按照bcb的邏輯處理, 例外的部分昨天都回答過了, 你問題也太多了!」⇒ 照 BCB 邏輯做，例外照 20260928 的答案；不再問子題。

### Q46. Configuration 頁切分頁時依權限檔（`D:\HT9045\config\Security_new.def`）整頁鎖住，網頁要不要照做（Q41 盤點「切分頁的權限」〔CC-L1〕／Steven 20260927 的裁決 S158（新頁面先不做，先把既有頁面還沒做的 golden 事件與權限等級檢查補齊））
**背景**：BCB6 的 Configuration 頁每切一個分頁，就照權限檔把那一頁整頁鎖住（旗標 0 的頁變灰、不能改）；網頁切分頁只換畫面，權限檔的分頁鎖完全不生效。程式細節：golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6101` `PageControl1Change`（外層 5 頁：Soft／Comm／Config／Tray／Hot Plate）與 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6116` `pcConfigChange`（內層 14 頁：A～M 加搜尋頁）每切一頁，就依權限檔 `D:\HT9045\config\Security_new.def`（`[Configuration]` 5 鍵、`[Config]` 13 鍵，缺鍵預設 1＝開；檔案不存在會建一份全 1，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cAuthority.cpp:409-440`；golden 沒有編輯畫面，人手動改）把那一頁整頁關掉（旗標 0＝整頁停用；旗標 1＝什麼都不改，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cAuthority.cpp:444-507`）。內層只在「登入等級 ≥ 權限表第 42 格（`LevelSet.AccessLevel[42]`）」時套（等級低的人反而不受限，golden 怪處）；外層不看等級。**不會**把開頁時依等級關掉的區塊又打開（例：C 頁 pal_C1 開頁時因等級不夠被關，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:5321`；切過去時 Hardware=1 → 維持關；Hardware=0 → 整頁關，更嚴）。其他 golden 怪處：開頁時顯示的那一頁要切走再切回才會被鎖；內層 14 頁檔案只有 13 個名字（M 頁用「In/Out Arm Order」那個鍵）。移植樹：開頁有讀權限檔（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.gen.inc:8448` GetConfAuth），但兩支切頁事件沒轉，網頁切頁只換畫面 ⇒ 權限檔的分頁鎖在網頁上完全不生效，比 golden 寬鬆。
**選項**：A 照翻：切頁時網頁送「按下當下就讓 C++ 跑 golden 事件程式」的指令（form.event）、C++ 跑這兩支（要等「從資料庫選一筆就自動填欄位」那題〔Q40〕的引擎送出點；伺服器沒辦法確認網頁真的有送，漏送就不會鎖）／B 開頁就套：C++ 在開頁送畫面值（editlist.get）時把旗標 0 的分頁照 golden 條件（內層看 42 號等級）整頁標成不可改，存檔時照既有規則丟掉那些值（比 golden 嚴一點：一開頁就鎖；不靠網頁）／C 不做，維持比 golden 寬鬆。
**St01建議**：B，標 [W906]（移植樹偏離 golden 的標記）偏離。預設檔案全部是 1，大部分機台看不出差別。例：某客戶把 `D:\HT9045\config\Security_new.def` 的 `[Config] Network` 設成 0——A（照 golden）：等級 ≥ 42 號的工程師切到 N 頁時整頁變灰；B：一開頁 N 頁就是灰的、改了也存不進去；C：N 頁照樣可以改。
**目前狀態**：等 Steven 決定，程式沒動。
**Steven 20260928 已裁決**：「Q46. 是需要做的, 但是可以排在比較後面, 也就是有空的時候才做」（08:0x（在 ST01-M 對話裡））
⇒ 意思：要做，排低優先（有空才做）。A／B Steven 沒說；ST01-M 註：照同一天「任何畫面的事件都是我們做；沒移植的直接實作」，翻切頁程式（A）比較一致，做之前 ST01-E 再確認。
**Steven 20260929 07:3x**：「Q46. Configuration 頁切分頁時依權限檔（D:\HT9045\config\Security_new.def）整頁鎖住 我昨天就說過 依權限檔鎖住是要執行的, 你都問了又問」⇒ **已裁決：照 BCB 翻切頁程式**（golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6101-6123`），排後面做。上面那行 ST01-M 註作廢。

### Q47. 主畫面 Run Mode 圖示（▣，golden RunICModeChange＋imgRunModeClick）誰做（golden 主畫面裡還沒移植、會存測試模式設定〔SaveTestMode／WriteLastDataFile〕的其餘入口那一列待辦〔todo D-005〕／查證 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\D005_TFMAIN_TESTMODE_WRITERS_20260927.md`）
**已做（commit `cd496b52`，主畫面事件批 B6 的 M-2；已過全量 gate `380a6a8e`，還沒上機驗）**
**背景**：BCB6 主畫面點 ▣ Run Mode 圖示會切換 Run Mode（Real → Dummy → Tray Only 循環）並存檔；網頁上的 ▣ 現在點了沒反應。程式細節：golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:29807-29871` `RunICModeChange`（由主畫面 ▣ Run Mode 圖示的 `imgRunModeClick`〔`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:29796-29805`〕觸發）會寫 `D:\HT9045\IniData\Data\<配方>\TestMode.Data` 與 `D:\HT9045\system\lastdata.dat`；移植樹沒有這一段，網頁 `D:\HT9045\web\page\main.html:190` 的 ▣ 圖示點了不送任何指令。本體需要的符號都在移植樹（15:4x 查過）。主畫面按鈕與流程一般歸 Jimmy（機台流程）；讀寫檔本體歸 St01。另外 golden 在 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:29809-29835` 先清 HotPlate 盤資料、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:29837-29843` 才用 MES1646（機台內有 IC 不能換模式）擋，疑似會丟 IC 資料（請 Jimmy 在 BCB6 確認，St01 建議照翻加註解）。
**選項**：A Jimmy 做（跟主畫面其他流程一起）／B St01 做：在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp` 檔尾附加本體、wb_serve 加主畫面 Run Mode 指令（act.main.runMode，St01 那一行同行插入）、網頁接 click。例（A、B 做完的畫面結果相同，差在誰做、什麼時候做）：A＝等 Jimmy 排到主畫面流程時一起接，接好之前網頁的 ▣ 維持點了沒反應；B＝St01 在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp` 檔尾先接，接好後網頁點 ▣ 就照 golden 切換並存檔。
**St01建議**：A；Jimmy 排不出來再 B。例：操作員在主畫面點 ▣ 把 Run Mode 從 Real 切到 Dummy → golden 會改 `D:\HT9045\IniData\Data\<配方>\TestMode.Data` 裡的 Run Mode（有開 bLastSetInSetUpFile 時，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:29856`）並經 UpdateMainOperateMode 寫 `D:\HT9045\system\lastdata.dat`（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:29859` → `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:13642`）；現在網頁點了沒反應。（⛔ 更正 20260927 21:xx：原例寫「從 Normal 切到 Retest」不對，golden 這個圖示切的是 Real／Dummy／Tray Only，見 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:29845-29851`。）
**目前狀態**：等 Steven 決定。
**Steven 20260928 已裁決**：「Q47.  我們做,」（08:0x（在 ST01-M 對話裡））
⇒ 意思：**選 B，St01 做**（推翻 St01 原本建議的 A＝交給 Jimmy）：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp` 檔尾附加本體、wb_serve 加主畫面 Run Mode 指令、網頁接 click。同一串 Steven 又說「主畫面的行為都是我們做掉」，更正成「正確的說法是: 任何畫面的事件, 都是我們做」「如果已經有移植, 就接上, 如果沒有移植的, 我們直接實作」；ST01-M 問「我們」是誰，Steven 選「St01＋St02，分工照舊」。
**Steven 20260929 07:4x**：「Q47. 主畫面 Run Mode 圖示 ST01-E做, 太忙可以分給st02-e做, 這不是早就說了嗎!」⇒ ST01-E 做（排在 Q44 之後），太忙就分給 ST02-E。

### Q48. 主畫面溫度圖示（🌡️，golden Panel42Click → ChangeTempMode(10)）誰做（同 Q47 那一列待辦〔todo D-005〕）
**已做（commit `cd496b52`，主畫面事件批 B6 的 M-3；已過全量 gate `380a6a8e`，還沒上機驗）**
**背景**：BCB6 主畫面點 🌡️ 溫度圖示會在 Hot 和 Ambient 之間切換溫控模式並存檔；網頁上的 🌡️ 現在點了沒反應。程式細節：golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:22392` 的 `Panel42Click` 呼叫 `ChangeTempMode(10)`（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:22437`；會寫 `D:\HT9045\IniData\Data\<配方>\TestMode.Data`／`D:\HT9045\system\lastdata.dat`）；`ChangeTempMode` 本體在移植樹已有，是 Jimmy 的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\MainTempMode.cpp`。網頁 `D:\HT9045\web\page\main.html:176` 的 🌡️ 圖示點了不送任何指令。
**選項**：A Jimmy 做／B St01 做一個薄入口（wb_serve 加主畫面溫度開關指令 act.main.tempOnOff，呼叫 Jimmy 的 ChangeTempMode，要 Jimmy 點頭），網頁要不要現在就開放按由 Steven 決定。例：A＝等 Jimmy 排到時由 Jimmy 接，接好之前網頁的 🌡️ 維持點了沒反應；B＝St01 只加一層入口，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\MainTempMode.cpp` 一行不動，Jimmy 點頭後網頁的 🌡️ 就能用。
**St01建議**：B（入口很薄，本體不動）；要 Jimmy 點頭。例：操作員點 🌡️ 把溫控從 Ambient 切到 Hot → golden 呼叫 ChangeTempMode(10)；現在網頁點了沒反應。
**目前狀態**：等 Steven 決定（選 B 還要 Jimmy 點頭）。
**Steven 20260928 已裁決**：「Q48. 我們做」（08:1x（在 ST01-M 對話裡））
⇒ 意思：**選 B，St01 做**主畫面溫度圖示的入口（呼叫 ChangeTempMode(10)）；照上面的通則不用等 Jimmy 點頭，FROM_STEVEN §3 已知會 Jimmy（交接 `ea376a1d`、`dc8caeb3`）。

### Q49. 設定頁的「開窗」要怎麼對準操作員真的按開窗的那一刻（R108 延伸；R89～R93、R84、R110 同一個前提）

**背景**：
- BCB 版每按一次開窗鈕，當下做三件事：查能不能開（運轉中、登入等級）、記一筆「Enter …」事件、依當下等級決定哪些欄位能改。
- 網頁版在開站（HMI 載入）時就把 24 個設定頁在背景載好、藏起來，每頁一載好就向 C++ 要資料，C++ 那時才做這三件事；之後操作員按開窗只是把頁顯示出來（`D:\HT9045\web\background.html:906-907`、`D:\HT9045\web\page\ht9045_wire_engine.js:2108`、`D:\HT9045\web\background.html:746-771`）。
- 結果：(1)「Enter …」記在開站（今天還沒寫進檔，見 R109）；(2) 換人登入後，頁面還是開站時那個等級的樣子——等級升高看到「讀取失敗」或灰掉的欄位；等級降低時欄位看似能改，按存檔才被 C++ 擋（不會寫錯）；24 頁只有 1 頁有重讀鈕，其他要整個 HMI 重新整理；(3) Contact 開頁換算、Teach 開頁清「要重新回原點」也在開站跑（模擬組態下每次開 HMI，下次 START 都會先全部回原點；由程式推得、未實測）。
- BCB 版在表單開著時換人登入也不會重查（閒置自動登出只關 Offset 視窗：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:26022-26048`；換等級只重算主畫面按鈕：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:12926-13191`），只有一個客戶選項會在存檔時擋 Operator（例 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp:14181-14192`）。網頁版每次存檔都重查，已經比 BCB 版嚴。
- 評估全文：`D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\window-open-edge.md`。

**選項**：
- **A** 維持現況。
- **B** C++ 看網頁送來的「哪些視窗開著」清單，某頁從「沒開」變「開著」的那一下才查權限、記「Enter」；開站不再記。只改 St01 的檔（主要是 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp`），半天到一天。只修「記事件的時間」，換登入的問題不變。
- **C** B 之外，C++ 在打開那一下重查權限，並通知頁面重讀。要改 Jimmy 的網頁引擎 `D:\HT9045\web\page\ht9045_wire_engine.js` 和一個共用檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp`；畫面約 1～1.5 秒後才更新。
- **D** 請 Jimmy 把網頁引擎 `D:\HT9045\web\page\ht9045_wire_engine.js` 改成「視窗打開才向 C++ 要資料」（Motor Test 頁 `D:\HT9045\web\page\HW.MotorTest.html` 已經這樣做）；每次開窗都照當下等級重查、重跑開頁程式，跟 BCB 版一樣。Jimmy 約半天。

**St01建議**：**B＋D**——St01 先做 B（事件紀錄開始寫檔之前做完就好）；請 Jimmy 排 D。C 不做（BCB 版也不在表單開著時重查；效果跟 D 一樣但多一層）。B 之後 R110「Configuration 存完多記一筆」會自然消失。

**例子**：出貨機台 08:00 開機（登入是 Operator），這台的權限表把 Yield Monitoring 設成 Engineer 以上；10:00 工程師登入、打開 Yield Monitoring。
- BCB 版：10:00 記「Enter Yield Monitoring」，畫面以 Engineer 等級打開、可以改。
- A：08:00 開站時沒過權限、沒記；10:00 也不記；畫面是 08:00 的「等級不足、讀取失敗」，要整個 HMI 重新整理。
- B：10:00 記；畫面仍是 08:00 的樣子。
- C：10:00 記；約 1 秒後頁面自動重讀、可以改。
- D＋B：10:00 記；頁面當場以 Engineer 等級打開、可以改——跟 BCB 版一樣。

**目前狀態**：A（現況，commit `342779cc`，主要改動檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp`）；等 Steven 決定。
**ST01-M 註（Steven 沒有直接裁這題）**：Steven 20260928 裁 Q51 時定了「C++ 用一個陣列記所有頁面與每頁開／關，跟網頁互通」，並說「bcb的 fShow的flag, 使用剛剛說的那個網頁顯示的陣列來替代, html跟c++要同時修改」——開窗的那一下就是陣列裡某頁從關變開，查權限、記 Enter、跑開頁程式都掛在那一下，網頁與 C++ 一起改（等於 B＋D，由我們做、不等 Jimmy）。ST01-E 確認後可視為已由 Q51 決定。
**ST01-E 20260928 14:5x 確認：由 Q51 的裁決涵蓋**（RULINGS_20260926 S168，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md`）——Steven 定了「C++ 用陣列記所有畫面開／關，fShow 改讀它」，等於選 B＋D，而且 D（網頁引擎改成開窗才要資料）也由我們做（Jimmy 已同意引擎 `D:\HT9045\web\page\ht9045_wire_engine.js:2046-2108` 給頁面狀態第 5 步用）。這題不用再另外回，實作在頁面狀態表第 5 步。

### Q50. 要不要在 Steven01 這台，用「先整夾備份、跑完逐檔還原」的方式，實際啟動網頁程式，把還沒跑過的網頁自動測試跑完？

**背景**

- 網頁版畫面的背後有一支 C++ 機台程式（網頁程式，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp` 編出來的 wb_serve.exe）。它照 BCB6 原版開機、關站，會改這台電腦 `D:\HT9045` 底下的設定檔、配方、計數檔——跟量產機同一套檔。
- 所以目前的規則是「Steven01 不跑網頁程式，除非 Steven 同意」。結果這兩天 Steven01 工程線（St01）寫好或改過的 13 支自動測試（模擬操作員點網頁、檢查 C++ 的反應），改完之後都沒跑過（其中 9 支一次都沒跑過），只確認了「程式編得過」，不知道「行為對不對」。
  例：「權限不夠時設定頁要開不起來」「關掉 Yield 設定頁後要照原版存起動模式」「接觸次數頁每秒自動更新」都還沒驗過。另外 12 支以前跑過的，後來程式改了很多，也沒重跑。
- 以前那種「假寫入」模式（啟動參數 `--dry`）已經取消，帶那個參數程式會直接結束，不能再用。

**選項**

- **A 搬家**：讓程式把所有讀寫都改到一個暫存資料夾。
  - A-1 改程式加一個總轉向開關：要動 Jimmy、Steven02、St01 三個人約 109 個檔、841 個寫死的路徑，還要推翻兩條已經裁決的規則（「配方資料夾被轉向時程式拒絕服務」「不可以有會把存檔導到別處的開關」）。漏一處就等於沒隔離，而且不容易發現。
  - A-2 不改程式，用 Windows 內建的「拋棄式沙盒」（一台關掉就全部消失的小虛擬電腦，真檔只唯讀分享進去）：要系統管理員打開 Windows 功能、重開機；這台目前看起來沒開，公司准不准也還不知道。
- **B 備份還原**：不改任何人的程式。找一段這台沒有別的 build、測試在跑的時間，先把會被改到的資料夾整夾複製備份（約 580 MB、1～3 分鐘），啟動模擬版網頁程式跑測試，跑完逐檔比對、改過的放回、新產生的刪掉，再比一次確認全部回到原樣。
- **C 維持現狀**：這台不跑，把測試的跑法寫好交給 Jimmy 的筆電（他那邊已經這樣跑）或 HT9050 機台去跑。

**建議**：**B**。分四輪、每輪結束都還原：①只開機不測試——確認這台開機會改的檔跟筆電量到的一樣，出現意外的檔就停下來回報；②不寫檔的測試；③會寫配方的測試（一支一輪）；④舊測試回歸。
條件：跑的時段這台只做這件事——替 Steven02 代編暫停（ST01-M）、全量建置測試不跑（ST01-E）、Steven 不按 VS Code F5；執行當下跳出的權限提示由 Steven 本人按允許。第一次整套約 3～4 小時。

**例子**

- 選 A-1：要把 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\common.cpp` 裡 223 處寫死的路徑（Jimmy 的檔）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\` 底下 43 處（Steven02 的檔）一個一個改；少改一處，那一處照樣寫真檔，而測試照樣通過。
- 選 A-2：請系統管理員打開「Windows 沙盒」；之後每次跑都在沙盒裡，關掉沙盒一切消失，不必挑時段。
- 選 B：今晚 23:00～02:00 這台只跑這件事。第一輪開機 2 分鐘，預期看到 `D:\HT9045\system\ContactInfo.ini` 多了 128 個鍵、`D:\HT9045\Error\BootLog.txt` 多一行、`D:\HT9045\system\lastdata.dat` 被照原版更新——都在筆電量過的清單裡，全部還原；若看到清單外的檔（例如某個配方檔被改），就停下來先告訴 Steven。
- 選 C：St01 把 13 支測試的跑法寫進交接檔交給 Jimmy，他排進筆電的夜間實跑；結果最快隔天回來，有錯要再來回一次。

**請 Steven 回**：A-1／A-2／B／C 選一個；選 B 請給一段時段（以及第 3 輪「會寫配方的測試」要不要一起跑，還是先只跑前兩輪）。

**目前狀態**：C（現況：Steven01 不跑網頁程式，13 支改過的自動測試都還沒跑）；等 Steven 決定。
**Steven 20260928 已裁決**：「Q50 你可以在晚上的時候做」（08:4x（在 ST01-M 對話裡））
⇒ 意思：**選 B（整夾備份、跑完逐檔還原）**，晚上做。20260928 22:00 由 ST01-M 用 gate 工具跑，照 `D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\wbserve-sandbox-run.md` §2／§6：四輪、每輪還原，看到清單外的檔就停；第 3 輪（會寫配方的測試）Steven 沒另外說，照建議一起跑。跑的時段 ST01-E 不跑 gate 或 build。

### Q51. 設定頁上的分頁標籤：開頁停在哪一頁、C++ 記不記得一共幾頁、點分頁時 BCB 自動跑的程式（R105／R106 細化，5 個子題）
**背景**：很多設定畫面上有一排排可以切換的分頁標籤（BCB 叫分頁控制 TPageControl）。網頁版現在一律從最左邊那頁開始、C++ 只數得到程式有用到的那幾頁、點分頁時 BCB 自動跑的程式也還沒翻。逐頁比對有網頁的 17 頁、50 排分頁：18 排打開時停的頁一定跟 BCB 不同（11 排在 Configuration），5 排看機種或模式。評估案全文：`D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\page-control-tabs.md`（逐頁表在第 2.1 節）。
**St01建議**：Q51-1＝A、Q51-2＝A、Q51-3＝A、Q51-4＝A、Q51-5＝A；先做 Q51-1（約半天，不用等 Jimmy），再做 Q51-2（約 1～1.5 人天，畫面不會變），Q51-3／Q51-4 是 Jimmy 的網頁引擎。回法：「Q51 照建議」，或逐題寫 Q51-3＝B 這樣。
**目前狀態**：等 Steven 決定，程式沒動（R105／R106 下面仍在，這一題是它們的細化）。
**Steven 20260928 已裁決**：「Q51.  在c++端弄一個陣列, 紀錄目前總共有多少頁面, 每個頁面的狀態是開還是關, 然後跟html端互通」（08:1x（在 ST01-M 對話裡））；接著「另外, bcb的 fShow的flag, 使用剛剛說的那個網頁顯示的陣列來替代, html跟c++要同時修改, 這個看起來比較重要, 要先做好」（08:3x）
⇒ 意思：C++ 用一個陣列記住所有頁面（畫面）和每一頁現在是開還是關，跟網頁雙向同步；BCB 裡判斷畫面開沒開的 fShow 旗標（以及 Showing／Visible 這類）全部改讀這個陣列；網頁與 C++ 一起改；**第一優先**。Q51-1～5 的分頁細節要怎麼套進這個陣列，ST01-E 的設計出來後再對。
**設計（20260928）與設計子題的回覆**：設計 `D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\page-state-array.md`（commit `9df70eeb`：C++ 一張 90 列的畫面表，網頁回報＋程式自己開關都寫進去，再送回網頁；BCB 的 fShow／Showing／Visible 全部改讀這張表）。Steven 20260928 10:3x 直接回 ST01-E（選項題）：
- **Q-P1 網頁全關或沒回報時**：「system start時, 或是馬達有在操作的時候, 要確保是開著. 其他時間比較不重要, 但是關閉沒畫面的時候, c++不可以system start」，再確認為：① 沒有任何網頁畫面連著時，所有 START（網頁、面板按鍵、GPIB／遠端）一律拒絕，畫面連上才准；② 運轉中或馬達在動時瀏覽器全關 ⇒ 照正常 STOP 停機。其他時候照看得到的事實（沒連著＝關、暫時沒送＝照最後一次、這台沒有那頁＝關）。
- **Q-P2 程式自己開／關的畫面**：每個 HMI 都自動開／關（例：HOME ALL 時每個 HMI 跳出 Home Monitor）。
- **Q-P3 接上後開始生效的 69 個 golden 判斷**：直接生效。
- **主畫面 Site 格與 6 顆機台控制鈕**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md` 的 N-1）：「c++部分只需要做到事件觸發, 例如按了home, 只是 bNeedHome=true, 實際動作可以先列表, 後面通知Jimmy進行接上, 這樣可以減少衝突吧?」⇒ C++ 只做到事件觸發（設旗標），實際動作列表交 Jimmy 接。
**目前狀態**：第 1～2 步（畫面表、網頁全關的 START／STOP 規則、4 處改讀、程式開關畫面、送回網頁）09-28 起實作中。

#### Q51-1（R105 細化）：C++ 要不要記住每一排分頁「一共幾頁、依序是哪幾頁」
**背景**：設定畫面上有一排排可以切換的分頁標籤（BCB 叫分頁控制 TPageControl）。20260927 起，網頁存檔時可以順便告訴 C++「操作員現在停在第幾頁」（commit c9d3c932，主要改 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditList.cpp`）；C++ 要知道這一排一共幾頁，才能判斷送來的頁數合不合理。現在 C++ 只數得到「程式有用到的那幾頁」，所以有 10 排少算了頁，排在後面的頁會被當成不合理：存檔結果多一行假的待辦，而且 C++ 記成還停在原來那頁。有網頁的是 4 排：Handler System 的「Heater」頁、Temp_Set 溫度補償的「Multi Sensor」頁、Tester IF 的「SLT Setting」頁、Ground Man 的「Log」頁。網頁引擎還不會送頁數，所以今天操作員看不到；Jimmy 接上之後就會出現。
**選項**：A 補：轉檔工具（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\gen_editlist.py`）照 BCB 畫面設計檔替 64 排分頁各產生一張完整頁序表（28 個產生檔各多 4～42 行、只加不改，畫面與存檔結果都不變）／B 維持：少算的頁照舊被擋、照舊記假待辦。
**St01 建議**：A（約半天，不用等 Jimmy）。
**例子**：操作員在 Handler System 切到最後一頁「Heater」改完按存檔——現在存檔結果會多一行「第 10 頁超出 0～9，伺服器保留原頁」（假的）；選 A 之後照收，沒有這一行。

#### Q51-2（R106 細化一）：C++ 記的「打開畫面時停在哪一頁」要照 BCB 的哪一種
**背景**：BCB 打開設定畫面時停在哪一頁，由三件事決定：①畫面設計檔裡設計者最後停的那一頁（程式開機時套一次，常常不是第一頁）；②打開畫面的程式明寫「切到某頁」（例 Yield Monitoring 依 Run Mode 切到 Normal 或 Re-Test 頁，golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uYieldMonitoring.cpp` 第 2646-2674 行）；③沒寫的，停在操作員上次離開的那一頁。移植樹兩種都沒照：一律從最左邊那頁開始，並且把②當成純畫面丟掉（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\gen_editlist.py` 第 303-311 行）。逐頁比對有網頁的 17 頁、50 排分頁：18 排一定跟 BCB 不同，其中 11 排在 Configuration、幾乎都是①（例 Configuration 開機後第一次打開停在「N [ Network ]」，golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 457 行）；另外 5 排看機種或模式（ASE 高雄、SPIL、Shuttle 浮料檢查、下 8CCD、Retest 模式）。對照表：`D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\page-control-tabs.md` 第 2.1 節。
**選項**：A ①②都照（最像 BCB；Configuration 會開在 N 頁）／B 只照②，①一律最左邊那頁（偏離 BCB，程式裡標 [W906]＝註明這是移植樹刻意跟 BCB 不同的地方；Tray Assignment 例外仍照①停在 Retest 群組，因為 BCB 存檔會看目前在哪一頁）／C 維持現狀：一律最左邊那頁，②也不照。
**St01 建議**：A（照 golden）。這一步只改 C++ 記的值，做完畫面不會變（網頁要不要照著切是題 3），可以先做；約 1～1.5 人天。
**例子**：Run Mode 是 Retest 時打開 Yield Monitoring——BCB 停在「Re-Test」頁；A、B 都是 Re-Test；C 停在「Normal」。開機後第一次打開 Configuration——BCB 停在 N 頁；A 是 N 頁；B、C 是 A 頁。

#### Q51-3（R106 細化二，Jimmy 的網頁引擎）：網頁什麼時候照 C++ 切到那一頁
**背景**：題 2 只改 C++ 記的值；要操作員看得到，Jimmy 的網頁引擎（`D:\HT9045\web\page\ht9045_wire_engine.js` 第 1131-1223 行，載入設定頁時套值的地方）要照 C++ 給的頁數去點頁籤。網頁每次存檔後會重讀一次（會重跑 BCB 打開畫面的程式），而有的打開畫面程式寫死「切回第一頁」（例 Speed，golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSpeed.cpp` 第 49 行）；BCB 按存檔不會重跑打開畫面的程式，畫面不會跳。另外網頁在開站時就把設定頁載好（`D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\window-open-edge.md`），所以「打開時停的頁」其實是開站那一刻算的。
**選項**：A 只有第一次載入時照 C++ 切；存檔後重讀保留操作員目前的頁／B 每次載入都照 C++ 切（存完可能跳回第一頁）／C 網頁不切（C++ 記它的，畫面照舊停最左邊那頁）。
**St01 建議**：A（Jimmy 約半天，改完 17 頁各開一次看停的頁）。
**例子**：在 Speed 的「Index Arm Condition」頁改完按存檔——A 停在 Index Arm Condition（跟 BCB 一樣）；B 跳回「Speed & Acc/Dec Setting」；C 不會跳，但 Yield Monitoring 等頁打開時不會照 BCB 停在 Re-Test。

#### Q51-4（R106 細化三）：存檔時網頁要不要把「目前停在哪一頁」送給 C++
**背景**：有些 BCB 存檔程式會看操作員現在在哪一頁。Tray Assignment：Configuration 開了「FT 存檔時把 RT 設定跟 FT 一樣」（bFTBin2RTBin）時，在 Normal Test（FT）頁按存檔，BCB 會把 FT 的 bin 放置複製到 RT（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp` 第 1353-1372 行）。網頁現在不送，C++ 以為一直停在 Retest 頁（設計檔停的那頁，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TrayForm.cpp` 第 454-456 行），所以網頁存檔永遠不複製。
**選項**：A 送（Jimmy 改引擎的存檔；C++ 已經會收，不合理的只丟那一筆，R102）：在 FT 頁存才複製，跟 BCB 一樣——對網頁使用者是「行為改變」／B 不送：維持網頁永遠不複製（跟 BCB 不同）。
**St01 建議**：A；Jimmy 改完先在 Tray Assignment 上機各存一次 FT 頁、RT 頁確認，並告訴操作員這個差別。停用中的分頁排不送，免得每次存檔多一行待辦。
**例子**：開了該選項的機台，操作員在 Normal Test 頁把 Auto1 改成 Bin 2 後存檔——BCB：RT 的 Auto1 也變成 Bin 2；網頁現在：RT 不變；選 A 之後：RT 也變成 Bin 2。

#### Q51-5（切頁事件；Configuration 兩支連 Q46）：操作員點分頁時 BCB 自動跑的程式，網頁要照做哪幾支
**背景**：BCB 有些分頁在操作員點頁籤時會自動跑一段程式（切頁事件；程式自己切頁時不跑）。有網頁的設定頁還沒翻的有 6 支、分成 5 項（全文與出處：`D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\page-control-tabs.md` 第 2.3 節）：
1. Configuration 外層＋內層（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp` 第 6101-6123 行）：依權限檔整頁鎖住，ASE 高雄切到 Config 頁要輸入密碼——就是 Q46（密碼另連 Q45）。Configuration 頁收網頁即時事件的入口今晚已補上（commit `64ade3b7`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp` 檔尾的別名頁「Config.Configuration」），只剩翻切頁程式本身。
2. Temp_Set 溫度補償（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp` 第 5224-5262 行）：切到 ATC 頁時把「基準溫度」那一組（7 個輸入格＋標籤＋清除鈕）藏起來；Arm 1／Arm 2 頁才顯示排序鈕；TSMC 台南另切兩塊面板。
3. Handler System（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\HandlerSys.cpp` 第 1286-1289 行）：只在「Customer Code」頁顯示搜尋框；這一頁平常藏著，要右鍵點 Exit 才出現（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\HandlerSys.cpp` 第 1204-1212 行），網頁沒有這個動作，所以碰不到。
4. BarCode（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\BarCode\BarCode.cpp` 第 2571-2580 行）：重畫條碼統計表頭與盤面小圖，純畫面。
5. Tray Form（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayForm.cpp` 第 455-467 行）：重排「從 Type? 複製」下拉；複製本身現在是網頁自己做。
（Start Condition 那支已由網頁照做；ProductionInfo 那支還沒有網頁。）
**選項**：A 只翻 2（Temp_Set）；1 跟著 Q46（Q46 選 A 才翻，選 B 就改成開頁時鎖、不翻事件）；3、4、5 不翻並在程式註明理由／B 5 項全翻（3 要先做右鍵手勢，4、5 伺服器做的是畫面，網頁看不出差別）／C 都不翻。
**St01 建議**：A。Temp_Set 約半天 C++（要 Q51-1、Q51-2 先做）；網頁的送出點由 Jimmy 跟 Q40 的送出點一起加，送的時候要把畫面上其他格目前的值一起帶上。
**例子**：Temp_Set 切到 ATC 頁——BCB 的「基準溫度」幾格消失、改不到；網頁現在還看得到、改了也會存；選 A 之後切頁時 C++ 跑這支程式，格子跟著藏起來，存檔不收那幾格。如果操作員是先在 Normal 頁改了基準溫度才切到 ATC 頁存檔，BCB 照存，選 A 也照存（因為網頁切頁時已經把新值帶給 C++）。

---

### Q52. St01 分支到 `c20bdec2`、再到 `0bca1318` 都已跑完完整測試，可以請 Jimmy 合進 main 嗎？
**背景**：
- St01（Steven01）的工作都先推在分支 `v906/steven-cbridge-review6`，要合進 main（大家共用的主線）才會到 Jimmy 的筆電和 HT9050 機台。Steven 20260927 16:3x 說「等 `6bd0f5a4` 的 gate 綠就合」，Jimmy 已照做（main 的 `53a55b35`）；那一句只針對 `6bd0f5a4` 那一次，所以之後的批次要再問一次。
- 這一批是 `6bd0f5a4` 之後到 `c20bdec2`（約 39 顆），主要是：設定頁分頁標籤的值也能讀寫（`c9d3c932`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditList.cpp`）、關設定頁時照 BCB 補跑的收尾第二批（`9268162b`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp`）、Handler System 溫控器廠牌「全機相同／各通道不同」（Q34 方案 D，`bc970c38`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.cpp`）、開設定頁照 BCB 記「Enter …」事件（R101，`342779cc`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp`）、合進 St02 的「Observer 頁逐項查權限」（`ed365c68`）。
- 「完整測試」（gate）＝ST01-M 在另一份工作樹，把模擬版（SIM）與出貨版（SHIP）兩種組態都整個重新編譯、跑全部自動測試（ctest），並核對機台真檔沒被改到。
  - 模擬版：編譯 0 錯誤；213 個測試中 21 個沒過，全部是以前就知道、跟這批無關的（15 個只在模擬版會失敗、2 個設定檔讀取、4 個只有 Steven01 這台環境會失敗）——**新失敗 0 個**；真檔沒變。
  - 出貨版：編譯 0 錯誤；213 個測試中 6 個沒過（2 個設定檔讀取＋同樣 4 個 Steven01 環境限定）——**新失敗 0 個**。
  - 兩種組態跑完後，機台真檔（`D:\HT9045\config\config.ini`、`D:\HT9045\system\Gerneral.ini`、`D:\HT9045\system\ContactInfo.ini`、`D:\HT9045\system\levelset.dat`、`D:\HT9045\system\lastdata.dat`、JAM0000.dat 的 SHA256，以及整個 `D:\HT9045_Log` 的檔案清單）都沒變。紀錄：`D:\AI_TempFile\st01-c20bdec2-gate.log`（20260927 21:09～22:29）。
- `c20bdec2` 之後今晚又推了 St01 的新工作：表單開關掛勾（`5b73d905`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.cpp`）、Configuration 頁點擊事件（`64ade3b7`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp`）、Speed／Temp_Set 頁（`70aa17e8`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ArmSpeed_File.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Temperature.cpp`）、評估案與決策題文件。ST01-M 09-28 00:15～01:26 對 `0bca1318`（包含以上全部）再跑一次完整測試：模擬版 21 個、出貨版 6 個沒過，跟上面同一批已知的——**新失敗 0 個**，機台真檔沒變（紀錄 `D:\AI_TempFile\st01-0bca1318-gate.log`）。
- `0bca1318` 之後的 St01 commit 都是文件；還在做的 N34 送出點（客戶碼 868 一批做完送報表，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fLotInfo.cpp`）**不在這一題裡**，做完另跑測試。
- St02（ST02-E）09-28 07:16 補充一個早點合的理由：現在**沒有一棵樹是完整的**——St02 的登入（Q9／Q24，`64e2c048`／`7f0c24b1`）與 Observer 頁逐項查權限（`94f16127`）只在 St01 分支；St01 分支又沒有 St02 的資料庫（P4／W7）、W10、事件紀錄分析器（ELA）與 Q41 頁面端（在 St02 的 MR !3）。這兩邊都進 main 之前，上機要準備兩支不同的網頁程式（St02 的上機清單 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\ST02_BRINGUP_CHECKLIST.md`，在 St02 的 `v906/steven-gpib-widget` 分支）。
**選項**：
- **A** 可以：ST01-M 在交接檔寫「St01 分支到 `c20bdec2` 可以合」，請 Jimmy 合。例：Jimmy 明天合完，筆電與 HT9050 機台就有 Q34 溫控器廠牌、開頁記 Enter 等功能。
- **B** 直接合到 `0bca1318`（今晚的新工作也已跑完完整測試、兩種組態都綠）。例：Jimmy 合一次，筆電與 HT9050 機台同時拿到 Q34 溫控器廠牌、開頁記 Enter，以及 Configuration／Speed／Temp_Set 頁的點擊事件、Tester IF 關窗掛勾（St02 那一行等這顆進 main 才打開）。
- **C** 先不合，等 Steven 看過 Q／R 題再說。例：main 停在 `6bd0f5a4`，Jimmy 那邊看不到這兩批。
**St01建議**：B（兩顆都綠，合到新的那一顆，Jimmy 只要合一次；St02 的 Tester IF 關窗也要等 `5b73d905` 進 main）。
**目前狀態**：`c20bdec2` 與 `0bca1318` 兩次完整測試都綠（新失敗 0、真檔沒變；09-27 22:29、09-28 01:26 跑完）；等 Steven 決定。
**Steven 20260928 已裁決**：「Q52.  ok, 但是jimmy還沒上班」（08:2x（在 ST01-M 對話裡））
⇒ 意思：**選 B，合到 `0bca1318`**；等 Jimmy 上班再合。FROM_STEVEN §2 已寫給 Jimmy 的合併列（交接 `ea376a1d`：main `8b5a91b5` 是 `0bca1318` 的祖先、可以 fast-forward；之後只有文件與 skill 的 commit，拿分支最新一顆也可以）。

### R60. Configuration 頁 P26「OCR Tray Lot」10 個格子什麼時候寫 `D:\HT9045\system\lastdata.dat`（Steven 選「P26 現在補上」〔Q19，裁決 S140〕之後的交件／todo 沒有對應項）
**背景**：P26 是 Configuration 頁上 OCR 檢查用的 10 個 Tray Lot 門檻格子。Steven 在 Q19 選「Soft 模擬速度不做、P26 OCR Tray Lot 現在補上」（Steven 20260927 的裁決 S140）。golden V912 Configuration 頁 P26 的 10 個格子（OCRTrayLot0～9，存在 `LastSet.TrayCount[0..9]`）一改值就寫 `D:\HT9045\system\lastdata.dat`。Steven 另一題「Heater Type 何時寫檔」（Q14）選 B「按了先不寫檔，等整頁存檔」（Steven 20260927 的裁決 S136），所以要決定 P26 照 golden，還是比照 Q14。
**選項**：A（目前做法，照 golden）：格子改過，就在按存檔時、確認框之前寫 `D:\HT9045\system\lastdata.dat`／B（比照 Q14＝B）：只在整頁存檔成功時才寫。
**St01建議**：A（照 golden 翻）。例：把第 3 格從 2 改成 5，同時改了一個會被存檔前檢查（CheckConfigurationBeforeSave）擋下的欄位，再按存檔。A：頁面顯示「沒有寫入」，但 `D:\HT9045\system\lastdata.dat` 的 TrayCount[2] 已經是 5（golden 在改格子的當下就寫了，結果一樣）。B：`D:\HT9045\system\lastdata.dat` 不動。
另外要 Jimmy 知道：`LastSet.TrayCount[10]` 在 golden 同時有兩個用途——P26 的 OCR 檢查門檻（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\OCRInsp.cpp:1099`），以及 SortingBinTray 執行中遞增的計數（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\SortingBinTray\SortingBinTray.cpp:2810`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\SortingBinTray\SortingBinTray.cpp:2815`）。網頁可以改這個值，golden 也一樣。
**目前狀態**：已照建議先做，可推翻：commit `3ee547e5`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\IniConfig.py`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.gen.inc`；網頁 `D:\HT9045\web\page\ht9045_iniconfig_p26_c.js`、`D:\HT9045\web\page\Config.Configuration.html`）。
**Steven 20260928 已裁決**：「R60. ok」（08:2x（在 ST01-M 對話裡））⇒ 照建議 A，維持目前做法（`3ee547e5`）。

### R61. 配方資料夾裡同名的 DIO 設定檔副本要不要擋（Steven 選「wb_serve 要認得 DIO 設定檔的動態檔名、擋掉舊的寫入」〔Q3，裁決 S125〕之後的交件／DIO 設定檔第二個寫入口那一列待辦〔todo F-005〕）
**背景**：DIO 設定檔是測試機 DIO 介面的設定（每種介面一個 ini）。Steven 在 Q3 選 A（Steven 20260927 的裁決 S125）：wb_serve 判斷「這個檔歸 C 路管、網頁直接寫檔的舊做法（B 路）不准寫」的表（`CRouteOwner`）要認得 DIO 設定檔的動態檔名（`D:\HT9045\IniData\DioCfg\<cbDIOType>.ini`，例如 `D:\HT9045\IniData\DioCfg\TTL_A.ini`），擋掉舊的 B 路寫入。golden 另有一個開關 `bI16TTLSaveInSetupFile`（`D:\HT9045\config\config.ini` 的 [Tester]）：開著時這個檔也會跟著配方存一份到配方資料夾（`D:\HT9045\IniData\Data\<配方>\`）。問題是配方資料夾裡的同名副本，開關關著時要不要也擋。
**選項**：A（目前做法）：不管開關開沒開都擋／B：只在開關開著時擋。
**St01建議**：A，沒回就照 A。例：開關是 0，配方資料夾裡留著一份舊的 `D:\HT9045\IniData\Data\<配方>\TTL_A.ini`。A：網頁寫配方檔的指令（recipe.doc.put）寫 TTL_A 回 409（拒絕）。B：可以寫。
**目前狀態**：已照建議先做，可推翻：commit `3ee547e5`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:1278`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:1322` 同一行插入，檔尾加 `CRouteOwnerDio`〔`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:7728`〕；探針 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\q3_dio_owner_probe.py`，還沒實跑）。
**Steven 20260928 已裁決**：「R61. A」（08:2x（在 ST01-M 對話裡））⇒ A，維持目前做法（`3ee547e5`）。

### R62. 權限表畫面開頁（FormShow）迴圈的越界保護（網頁存權限表 `D:\HT9045\system\levelset.dat` 那件工作〔S64〕交件時的追加題 1／todo 沒有對應項）
**背景**：權限表畫面（BCB6 的 TfSecurity）的開頁程式有一個迴圈，在移植樹裡會讀到空陣列而讓 wb_serve 當掉；現在沒有人呼叫它，所以還沒出事。程式細節：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSecurity.cpp:330` `TfSecurity::FormShow` 的迴圈跑到 `iMaxLevelItem`（180），但畫面上那 180 格的物件清單 `mySecurityPal` 在移植樹還沒建（那段程式還沒解開，標記 GATE (SEC1)），是空的，i=0 就會讀空陣列而當掉。同一個問題在關頁程式 FormClose（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSecurity.cpp:512`）已經照 Steven 選的「接上，但先修好越界風險」（Q30＝B，Steven 20260927 的裁決 S151）加了保護。FormShow 目前沒有人呼叫（網頁的 Jam 頁程式刻意避開，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebSecurityJam.cpp:799-800`）。
**選項**：A 現在加同一個保護／B 維持不動。
**St01建議**：A。例：將來 Jam 頁的開頁改成呼叫 FormShow——B（沒有保護）i=0 就當掉；A（有保護）跑 0 次，網頁的值照樣由 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLevelSet.cpp` 放進 LevelSet。
**目前狀態**：已照建議先做，可推翻：commit `2e70279d`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSecurity.cpp:330`，同一行改寫；同一顆也改了 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLevelSet.cpp`，那是下面 R66「權限表檔案路徑加測試用環境變數」的部分）。
**Steven 20260928 已裁決**：「R62.  A」（08:2x（在 ST01-M 對話裡））⇒ A，維持目前做法（`2e70279d`）。

### R113. Q34：站號重複檢查（方案 D 第 8 點 D-8a），TC401 與跨廠牌怎麼算（Q34 交件）
**背景**：**這是什麼**：「各溫控器不同」模式可以替每個通道指定站號（溫控器在 COM 埠上的位址）；方案 D 第 8 點（D-8a）規定存檔時檢查同一條溫控 COM 埠上的站號有沒有重複。TC401 一台有 4 個通道；D-8a 只擋「同廠牌同站號」。KT4H 與 DTK4848 都是 Modbus ASCII、站號都是 `:%02X`（golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cpublic.cpp:185`、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cpublic.cpp:210`），同一條溫控 COM 埠上同站號會同時回應。
**選項**：A TC401 同一台同一通道才算重複，跨廠牌不擋（照 D-8a 字面）／B 另外把 KT4H 與 DTK4848 當同一組一起擋／C 同站號不分廠牌全擋（例：再加上例如 E5DC 站號 5 與 TC401 第 5 台也會被擋）。
**St01建議**：TC401 維持 A；跨廠牌加 B（C 會連不同協定的 TC401 也擋；RTU 與 ASCII 共線的風險請 Jimmy 判斷）。例：Shuttle1 設 KT4H 站號 5、Head1 設 DTK4848（預設站號 5）——現在可以存；照 B 會擋下並列出這兩個通道。
**目前狀態**：A（照 D-8a 字面，commit `bc970c38`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.cpp:629-657` HmDuplicates）；B 等 Steven／Jimmy 決定。
**Steven 20260928 已裁決**：「R113 不同廠牌共用同一個站號要不要擋。 要擋, 目前有遇到 E5DC + KT4H + DTK4848混用了, 新的機台架構是改用全機DTM」（08:4x（在 ST01-M 對話裡））
⇒ 意思：同一條溫控 COM 埠上，**不同廠牌只要站號相同就擋，所有廠牌都算**（比 B 範圍大，等於 C）；TC401 同一台自己的幾個通道是同廠牌，照原本同廠牌規則。背景：新機台架構改成全機 DTM。ST01-E 工程師實作中。
**Jimmy 20260928 11:0x 查 golden**（main 的 `D:\HT9045\docs\handoff\TO_STEVEN.md` §4 10:3x 那一列）：golden 的溫控**站號不能設**，由通道序號算（KT4H／DTK4848／E5DC＝序號＋1，TC401＝序號÷4＋1 台、通道＝序號%4；`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cpublic.cpp:185`），golden 也**沒有任何站號重複檢查**；而且同一條溫控線上其實是三種協定（KT4H／DTK4848＝Modbus ASCII、TC401＝Modbus RTU、E5DC＝Omron CompoWay/F），**只有同協定同站號才會真的撞**（例：KT4H 與 DTK4848 同站號兩台都會回，讀到別台溫度），跨協定不會回。St01 現在照 Steven 字面「不同廠牌共用同一個站號…要擋」全擋（`d7fa099a`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.cpp`）。如果 Steven 要改成「只擋同協定（KT4H＋DTK4848）」，只要改判斷的一行；例：E5DC 站號 5 與 KT4H 站號 5——現在擋，改了之後放行；KT4H 站號 5 與 DTK4848 站號 5——兩種做法都擋。

### R119. 速度設定頁的「全部速度」滑桿和「回預設值」鈕，會改掉畫面上被鎖住（灰掉）的欄位（Speed 頁存檔重算，St01 20260927 夜間交件）
**背景**：
- 速度設定頁（網頁 `D:\HT9045\web\page\Setup.Speed.html`；BCB 版 golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSpeed.cpp`）第一頁有兩個「一次改很多欄」的功能：
  - 「全部速度／全部加速度」滑桿：先勾要一起調的軸（Index Arm、Input Arm…），拖滑桿，勾到的軸速度一起變成同一個百分比（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSpeed.cpp:1282-1406`）。
  - 「Set to define（回預設值）」鈕：約 60 個欄位填回出廠值（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSpeed.cpp:1812-1933`）。
- 有些欄位開頁時會被鎖住、操作員打不了字：
  - ATP 開批後鎖 Index 速度、Index 真空檢查時間、Index 吹氣時間（「開批鎖關鍵參數」功能，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSpeed.cpp:317-329`）。
  - 機台內還有料時鎖 Input Arm 的 Auto Skip（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSpeed.cpp:143-154`，註解寫「避免吸嘴錯誤 hang up」）。
  - 多站（1x4、8 站以上）而且 IC 大時，吸嘴開合方式鎖成只能用 Fixed（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSpeed.cpp:131-141`，註解「IC 大於 25mm 一次吸四顆只能使用 Fix Mode」）。
- **問題**：BCB 版這兩個功能是程式直接寫欄位，不管有沒有鎖，按存檔就存進去——鎖等於可以被繞過。吸嘴開合方式例外：BCB 版讀檔時會再強制改回 Fixed（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSpeed.cpp:504-511`、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSpeed.cpp:634-642`），機台實際仍用 Fixed，只是檔案裡的值不一致。
- **移植樹現況**：
  - 滑桿這次改成存檔時由 C++ 照 BCB 版重算（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ArmSpeed_File.cpp` 檔尾，commit `70aa17e8`），鎖住的欄位跟 BCB 版一樣會被改。
  - 回預設值目前由網頁自己算（St02 的 `D:\HT9045\web\page\ht9045_speed_c.js`，在 St02 的分支，St01 這台還沒有），存檔時鎖住的欄位被 C++ 丟掉、保留原值（比 BCB 版保守）。網頁若改用這次新接的「按下就讓 C++ 跑 golden 點擊程式」（form.event）做回預設值，結果就跟 BCB 版一樣會改到鎖住的欄位——兩條路結果不同。
  - 移植樹目前「開批鎖關鍵參數」不會生效（權限陣列一直是 0，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainBoot.cpp:372-374`），Auto Skip 的「有料就鎖」會生效。
**選項**：
- **A** 照 BCB 版：兩個功能都照改鎖住的欄位並存檔（網頁回預設值改用 C++ 跑，兩條路一致）。例：機台內有料、Auto Skip 開著（被鎖住），按回預設值再存檔 → Auto Skip 變成關（同 BCB 版）。
- **B** 移植樹擋：鎖住的欄位這兩個功能都不改、存回原值，其他欄照改（跟 BCB 版機台不同）。例：同上 → 存檔後 Auto Skip 仍是開；ATP 開批後 Index 速度鎖在 80%（等開批鎖生效後），勾 Index Arm 拖到 100% 存檔 → Index 仍 80%、其他勾到的軸變 100%。
- **C** 先不定：滑桿照 BCB 版（已是現況），回預設值維持網頁自己算（鎖住的保留原值），請 Jimmy 確認 BCB 版這是不是漏洞；BCB 版改了移植樹再跟。例：拖滑桿存檔 → Index 100%（同 BCB 版）；按回預設值存檔 → Auto Skip 仍是開。
**St01建議**：C。「開批鎖關鍵參數」是客戶要求，滑桿能繞過看起來是 BCB 版的漏洞；要改應該 BCB 版與移植樹一起改（RULINGS_20260927 第 1 條：這個專案只能改 906 C++，BCB 版要 Jimmy 決定）。
**目前狀態**：滑桿是 A、回預設值是網頁端（C 的現況），commit `70aa17e8`；等 Steven 決定。
**Steven 20260928 已裁決**：「R119 BCB的不動, c++版本要修正」（08:4x（在 ST01-M 對話裡））
⇒ 意思：**選 B**——BCB 不動；C++ 版的「全部速度」滑桿與「回預設值」鈕不能改到開頁時就鎖住的欄位（保留原值，其他欄照改）。ST01-E 工程師修 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ArmSpeed_File.cpp` 中；網頁那顆回預設值鈕另告知 St02。

### R120. 確安科技的機台（客戶碼 868，程式裡叫 CC_CYUEAN）結批時「清掉『已從 FTP 下載工作檔』的記號」這一行，要不要照舊版做（N34 送出點交件）
**背景**：
- Steven 在 W22 定了 N34（客戶碼 868 的 OEE／故障報表）要做，St01 已把一批做完時送報表指令的那段照舊版整段打開（commit `bbe5e1fc`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fLotInfo.cpp:7084-7095`；舊版 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uLotInfo.cpp:2327-2334`）。同一段裡還有一行：結批時把「已從 FTP 下載工作檔」的記號（bFTPDownloadSetupFile）設成 false，意思是「換批就要重新從 FTP 下載工作檔」。
- **操作員看到的**：確安的機台，只要「設定頁［FTP］Enable FTP 有勾、連線測試機（On-line）、不是 PE 工程模式、跑出貨版程式」四個條件同時成立，每次按「Lot End 結批」之後，下一次按 START 就會跳出「Need Download FTP Setup File!!」，START 被擋下來。
  - 舊版結批時設 false：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uLotInfo.cpp:2329`；START 前讀它：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:31500-31503`（呼叫處 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:5815-5826`）；操作員在 FTP 畫面下載成功後設回 true：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\KYECFTP\FTPClient.cpp:1011-1015`。
  - 移植樹讀它的地方：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebStart.cpp:780-784`（呼叫處 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebStart.cpp:2691-2703`，只有出貨版才編，模擬版不做這個比對）。
- **移植樹跟舊版不同的地方**：舊版到 FTP 畫面重新下載一次就解開；移植樹的 FTP 下載畫面還沒接上「設回 true」這一步（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\KYECFTP\` 底下找不到 bFTPDownloadSetupFile）。移植樹目前只有三個地方會設回 true：程式重開（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cinitial.cpp:10236`）、遠端下載指令（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Automation\auto9045.cpp:2624`）、偉測工程批開批（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fLotInfo.cpp:5553`）⇒ 上面那種確安機台結批後會一直擋 START，直到程式重開。
- 不受影響：［FTP］Enable FTP 沒勾的機台（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebStart.cpp:773-774` 直接放行）、跑模擬版的機台、客戶碼不是確安的機台。這台實驗機（`D:\HT9045\system\Gerneral.ini` CUSTOMER_CODE=791、`D:\HT9045\config\config.ini`［FTP］Enable FTP=0、［N34 Function］bN34_GenerateOEEAlarmRpt=0）看不出差別。
**選項**：
- **A** 照舊版整段打開（現在就是 A）：結批時清掉記號；［N34 Function］有勾才送報表指令。例：確安機台開了 FTP，結批後按 START，畫面跳出「Need Download FTP Setup File!!」；舊版到 FTP 畫面重新下載就好，移植樹目前要重開程式，等 FTP 畫面接上「設回 true」之後就跟舊版一樣。
- **B** 只把「清掉記號」那一行留在閘裡，報表指令照開。例：確安機台開了 FTP，結批後直接按 START 不會被擋，下一批繼續用上一批下載的工作檔跑（跟舊版不同：舊版會要求重新下載）。
**St01建議**：A（照舊版；也符合 W22「功能有開的都要做」），另開一件把 FTP 下載畫面「下載成功就設回 true」接上（照舊版 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\KYECFTP\FTPClient.cpp:1011-1015`；照 Steven 20260928「任何畫面的事件都是我們做」，這件是 St01／St02 的）。
**目前狀態**：已照 A 做，可推翻：commit `bbe5e1fc`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fLotInfo.cpp`）。
**Steven 20260928 已裁決**：「ST01-E A」（09:4x（在 ST01-M 對話裡））⇒ 意思：A——維持照舊版結批時清掉記號（`bbe5e1fc`）；另外把 FTP 下載畫面「下載成功就設回 true」（舊版 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\KYECFTP\FTPClient.cpp:1011-1015`）排進「BCB 事件沒移植的一次做完」那一批。

### 20260929 照 BCB／已做、不用 Steven 裁決的 St01 R 題（從 decisions-pending.md 搬來）

> 依據：Steven 20260929 07:3x「Q45. 請按照bcb的邏輯處理, 例外的部分昨天都回答過了, 你問題也太多了!」——BCB／golden 行為清楚的就照做，不再列題。每題原文照搬，最後一行「20260929 結果」是現在的做法；標「排進事件批」「排進頁面狀態讀取批」的是還沒做的工作，由 St01 做。給 Jimmy 看的：R67、R71、R76、R124、R136、R138、R143。

### R63. 權限表的分頁依登入等級藏起來（golden 的 PageControl1 等級閘）照 golden 補上（權限表存檔那件工作〔S64〕的追加題 2／todo 沒有對應項）
**背景**：golden V912 依登入等級決定權限表能不能看（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSecurity.cpp:302-381`，FormShow 裡設 `PageControl1->Visible`）：Engineer、OP，以及 SPIL 或 SCS 客戶的 Supervisor，權限表會被藏起來，改動也會被擋。`855a6a83` 照 golden 補上。
**選項**：A 維持（照 golden）／B 顯示但唯讀。
**St01建議**：A。例：Steven01 這台 `D:\HT9045\system\levelset.dat` 的 [29]＝2，一般客戶碼下畫面看起來跟以前一樣。B 的話：Engineer 登入時看得到權限表，但每一格都改不了。
**目前狀態**：已照建議先做，可推翻：commit `855a6a83`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLevelSet.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSecurity.cpp`；網頁 `D:\HT9045\web\page\ht9045_wire_statussecurity.js`）。
**20260929 結果**：照 BCB，已做（commit `855a6a83`）。

### R64. 隱藏的權限格子也照「停用格子不能改」（Q26）擋下（權限表存檔那件工作〔S64〕的追加題 3／todo 沒有對應項）
**背景**：Steven 在 Q26 選 B（Steven 20260927 的裁決 S146：只擋停用格子本身，其餘正常存）：停用（灰掉）的權限格子不能改。`855a6a83` 把同一條規則延伸到「隱藏」的格子（Q26 只講了停用）。
**選項**：A 維持（隱藏的也擋）／B 隱藏格子允許寫入。
**St01建議**：A。例：出貨組態、客戶不是 ASE_CL 時 [131] 是隱藏的，有人直接用 WebSocket 送 [131]=3——A：會被擋下；B：會寫進 `D:\HT9045\system\levelset.dat`。
**目前狀態**：已照建議先做，可推翻：commit `855a6a83`（同 R63 的檔：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLevelSet.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSecurity.cpp`、`D:\HT9045\web\page\ht9045_wire_statussecurity.js`）。
**20260929 結果**：已做，取安全側（commit `855a6a83`）：BCB 沒有「網頁直接送隱藏格子」這種情況；隱藏的格子跟停用的一樣擋。

### R65. 開機時沒有 `D:\HT9045\system\levelset.dat`，照 golden 建一份全 0 的表（權限表存檔那件工作〔S64〕的追加題 4／todo 沒有對應項）
**背景**：golden V912 開機讀 `D:\HT9045\system\levelset.dat`，檔案不存在時建一份全 0（全部都是 Operator 等級）的表（golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSecurity.cpp:1474` 起的 GetLevelSet，寫死的檔名在 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSecurity.cpp:1476`）。
**選項**：A 維持 golden（建檔）／B 開機時拒絕建檔。
**St01建議**：A。不建檔的話記憶體一樣是全 0，拒建只是不落檔，不會比較安全。例：新機台第一次開機沒有這個檔——A：建一份全 0 的 `D:\HT9045\system\levelset.dat`，每一格都是 Operator 等級；B：不建檔，記憶體同樣全 0（全部 Operator），下次開機再來一次。
**目前狀態**：已照建議先做，可推翻：commit `855a6a83`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSecurity.cpp` GetLevelSet，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSecurity.cpp:1620` 起）。
**20260929 結果**：照 BCB，已做（commit `855a6a83`）。

### R66. `D:\HT9045\system\levelset.dat` 的路徑加測試用的環境變數 W906_LEVELSET_PATH（權限表存檔那件工作〔S64〕的追加題 5／todo 沒有對應項）
**背景**：golden 的 GetLevelSet／SetLevelSet 路徑寫死成字面值 `d:\HT9045\system\levelset.dat`（就是 `D:\HT9045\system\levelset.dat`；`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSecurity.cpp:1476`、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSecurity.cpp:1513`）。網頁存權限表時，備份、驗證、寫入要落在同一個檔；只改一邊的話會分成兩個檔，所以 `855a6a83` 沒有加半套。沒有測試縫，「停用格子不能改」（Q26）的端到端測試只能寫機台真檔。
**選項**：A 兩邊一起加環境變數 `W906_LEVELSET_PATH`（沒設＝golden 原路徑）／B 維持沒有。
**St01建議**：A。例：ctest 設 `W906_LEVELSET_PATH=C:\Temp\ls_test.dat`，整段存檔、備份、驗證都只動這個暫存檔；出貨機沒設這個變數，行為跟 golden 一樣。B 的話：同一個測試只能寫 `D:\HT9045\system\levelset.dat` 真檔。
**目前狀態**：已照建議先做，可推翻：commit `2e70279d`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSecurity.cpp:1622`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSecurity.cpp:1684`，檔尾 `W906_LevelSetPath()`〔`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSecurity.cpp:2125`〕；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLevelSet.cpp:114`）。levelset 的 ctest 還沒寫（要改 CMake）。
**20260929 結果**：已做（commit `2e70279d`）：測試用；沒設這個環境變數時跟 BCB 一樣。

### R67. 只有 4 個等級的機台，權限表檔案出現值 4（權限表存檔那件工作〔S64〕的追加題 6／todo 沒有對應項）
**背景**：4 階機台的等級只有 0～3。檔案裡若有值 4，網頁引擎（Jimmy 的 `D:\HT9045\web\page\ht9045_wire_engine.js`）會整頁拒寫；golden 則把它顯示成 3。
**選項**：A 維持整頁拒寫（大聲報錯）／B 改引擎，填正規化後的值（要動 Jimmy 的檔）。
**St01建議**：A。現在的真機檔沒有這種值。例：某格是 4——A：整頁拒寫、畫面報錯，要先把檔改對；B：那一格顯示成 3（跟 golden 一樣），照常可存。
**目前狀態**：已照建議先做，可推翻：沒有改動（維持現狀）。
**20260929 結果**：照 BCB：那一格顯示成 3。現在的真檔沒有這種值，排進待辦；網頁引擎 `D:\HT9045\web\page\ht9045_wire_engine.js` 是 Jimmy 的檔，改之前先知會 Jimmy。

### R68. 每次測試開始寫一筆 socket ID 紀錄（SocketIDLog）照 golden 翻的三個怪處（Steven 選「本體與安裝歸 St01、函式指標歸 St02」〔W11＝B〕之後的交件／St02 的測試機通訊設定統一那一列待辦〔todo H-009〕）
**背景**：Steven 在 W11 選 B 之後，本體 `W906_SC_SocketIDLog` 在移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\StartCondition.cpp:355-437`，逐行照 golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cStartCondition.cpp:1435-1492`。golden 有三個看起來不對的地方，目前都照翻：
1. **每行只用 CR 結尾**：golden 寫 `"\r"` 不是 `"\r\n"`，文字模式的 fopen 只轉換 `\n`，所以 CSV 每行結尾是單獨的 CR（記事本會看成一整行）。
2. **檔案每次整個重寫**：檔名只到日期，同一天每次測試開始都會先刪檔再寫，檔案只留最後一次的快照（`"a+"` 實際上等於 `"w"`）。
3. **其他**：`tmps1`（FT／RT）算了沒用；路徑有重複的反斜線（`SocketIDLog\\YYYYMM`，Windows 照樣找得到）；每一列的 Alarm Count 都是 `iContactAlarmCount[3]`，不分 site；迴圈上限用 FTestSuck 的列數／欄數，陣列是 [4][8]，golden 沒有檢查越界。
**選項**：A 全部照翻（跟 BCB6 機台產生的檔一樣）／B 修 1 和 2（改成 CRLF、真的附加）／C 只在第 3 點的越界加保護（範圍內行為不變）。
**St01建議**：A＋C。例：Kit 吸嘴設成 5 列時，golden 會寫到陣列外面；加保護後只寫前 4 列，其他行為跟 golden 一樣。A 的例：記事本打開整份看成一行，同一天只留最後一次測試開始的內容。B 的例：每行 CRLF 結尾、同一天每次測試開始都接在檔尾，跟 BCB6 機台產生的檔不一樣。
**目前狀態**：已照翻（A），commit `b782b00b`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\StartCondition.cpp`）；C 的越界保護 20260927 已照建議先加，可推翻：commit `e903b153`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\StartCondition.cpp:425`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\StartCondition.cpp:427`，同一行改寫，列數上限 4、欄數上限 8）。
**20260929 結果**：照 BCB 翻，已做（commit `b782b00b`）；另加的越界保護只防程式當掉，不改寫出來的內容（commit `e903b153`）。

### R69. SocketIDLog 接上之後會寫機台真的 log 目錄 `D:\HT9045_Log\SocketIDLog\`（socket ID 紀錄放哪個檔那題〔W11〕的交件／St02 的測試機通訊設定統一那一列待辦〔todo H-009〕）
**背景**：接上之後，每次測試開始都會寫 `D:\HT9045_Log\SocketIDLog\YYYYMM\SocketID_Lifetime_YYYYMMDD.csv`，這是量產機共用的 log 目錄。
**選項**：A 照 golden 寫真目錄；在開發機上跑之前，先經 Steven 同意，或先設環境變數 `W906_SOCKETIDLOG_ROOT` 指到暫存目錄／B 開發組態預設寫暫存目錄。例：開發機沒設 `W906_SOCKETIDLOG_ROOT` 就跑 wb_serve、開始測試——A：檔寫進 `D:\HT9045_Log\SocketIDLog\YYYYMM\`；B：開發組態寫到暫存目錄，只有出貨組態寫真目錄。
**St01建議**：A（出貨行為跟 golden 一樣；開發機照「跑 wb_serve 前要 Steven 同意」的規則）。
**目前狀態**：函式指標還沒裝：等 St02 在移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\atester.cpp` 做好 `W906_SocketIDLogBody` 和呼叫點（golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\atester.cpp:882`，GetTesterResult 的 case 1），ST01-E 再在移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4111` 那一行的尾端安裝。　**20260927 16:0x 已裝**：commit `9b78d312`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4111` 行尾安裝 `W906_SocketIDLogBody = &W906_SC_SocketIDLog`；St02 的呼叫點 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\atester.cpp:944-945` 已隨 main 合進來，`1de5005b`：合併 main，帶進 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\atester.cpp` 等 49 個檔）⇒ 從現在起 wb_serve 每次測試開始會寫 `D:\HT9045_Log\SocketIDLog\` 底下的檔；在開發機跑 wb_serve 前先設 `W906_SOCKETIDLOG_ROOT` 到暫存目錄，或經 Steven 同意。
**20260929 結果**：照 BCB 寫真的 log 目錄；在開發機上跑之前，先設 `W906_SOCKETIDLOG_ROOT` 指到暫存目錄。

### R70. 要不要替 SocketIDLog 開 ctest（socket ID 紀錄放哪個檔那題〔W11〕的交件／St02 的測試機通訊設定統一那一列待辦〔todo H-009〕）
**背景**：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\StartCondition.cpp` 目前只編進 wb_serve，要測它得新開一個 ctest 目標，也就是要改移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\CMakeLists.txt`。
**選項**：A 現在開（用 `W906_SOCKETIDLOG_ROOT` 寫暫存目錄）／B 等 St02 的呼叫點接好，再一起測。例：A＝現在就加一個測試目標，只驗 CSV 內容；B＝呼叫點接好之後，一次測「測試開始 → 寫出 CSV」整條路。
**St01建議**：B。RULINGS_20260927 第 4 條（這兩天不等機台驗證：編譯＋模擬驗證成功就算完成），呼叫點接好後一次測完整條路比較有意義。
**目前狀態**：已照建議先做（B），沒有改動。⛔ 20260927 22:xx 更新：St02 的呼叫點已經接好（`1de5005b` 帶進 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\atester.cpp:944-945`），St01 也已在網頁程式開機時裝上（`9b78d312`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp`）——B 的「等呼叫點接好」條件已到，整條路的測試排進評估案 Q50（在這台實際跑網頁程式）一起做。
**20260929 結果**：St02 的呼叫點已接好；ctest 跟 St02 的測試一起做，不用 Steven 決定。

### R71. AOI 設定檔（`D:\HT9045\IniData\Data\<配方>\AOI.Data`）的 Top Timeout 鍵名統一成哪一個（Steven 選「906 C++ 單邊修 AOI 設定檔兩個 golden 錯誤」〔Q31＝A'，裁決 S152〕之後的交件／六個沒有頁面的設定檔那一列待辦〔todo E-001〕）
**背景**：AOI 頁的 Top Timeout，BCB6 存檔寫一個鍵名、讀檔讀另一個鍵名，所以畫面上打的值從來沒被讀到，一直跑預設值 3。Steven 在 Q31 選 A'（Steven 20260927 11:xx 定案的裁決 S152：906 C++ 單邊修 AOI 設定檔兩個 golden 既有錯誤，接受格式跟 BCB6 機台不一致）。其中 Top Timeout：golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\fAOI.cpp:3301` 存檔寫 `TimeOutTopScanAOIView`，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\fAOI.cpp:3495` 讀檔讀的卻是 `TimeOutScanTopAOIView`（V899 `D:\HT9045\HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422\fAOI.cpp:2939`／`D:\HT9045\HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422\fAOI.cpp:3103` 也一樣），所以 BCB6 畫面打的值從來沒被讀到過，一直跑預設 3。修法要選統一成哪個名字。
**選項**：A 統一成讀檔用的 `TimeOutScanTopAOIView`，改寫檔那一行／B 統一成寫檔用的 `TimeOutTopScanAOIView`，改讀檔那一行／C 照 B，再加「新鍵讀不到就退回讀舊鍵」。
**St01建議**：A。兩個修正都只改寫檔那邊，讀檔跟 BCB6 完全一樣，所以同一份 `D:\HT9045\IniData\Data\<配方>\AOI.Data` 放在 BCB6 或 906 上讀出來的值都相同。代價是 BCB6 寫的 `TimeOutTopScanAOIView` 會一直留在檔裡，沒人讀。例：BCB6 機台 Top Timeout 打 5 並存檔 → 檔裡 `TimeOutTopScanAOIView=5`，但 BCB6 實際跑 3。選 A：這份配方拿到 906 一樣跑 3、頁面顯示 3；在 906 改成 5 存檔後寫 `TimeOutScanTopAOIView=5`，拿回 BCB6 也會跑 5。選 B：拿到 906 馬上跑 5，但 906 存過的檔拿回 BCB6 還是只跑 3。選 C：跟 B 一樣拿到 906 馬上跑 5；某份檔只有另一個鍵名時，906 退回讀那一個；906 存過的檔拿回 BCB6 還是只跑 3。
**目前狀態**：已照建議先做，可推翻：commit `725038a6`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\AOISetup.py:46-54`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\AOISetup.py:177-183`，重產 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\AOISetup.gen.inc:1195-1202`）。Top 延遲值那一個錯誤（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\fAOI.cpp:3300` 寫進 Bottom 的 `StartDelayTimeScanAOIView`）同一顆改成寫 `StartDelayTimeTopScanAOIView`（就是 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\fAOI.cpp:3494` 讀的鍵），沒有選擇題。注意：AOI 頁還沒接上（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\AOISetup.cpp:106` `FileRW_AOISetup_spbSaveClick` 沒有呼叫者），接上後才會生效。
**20260929 結果**：已做（commit `725038a6`）：BCB 讀檔、寫檔用了兩個不同的鍵名（寫進去的值讀不回來），移植樹統一成讀檔那個；BCB 那邊要不要一起改由 Jimmy 看。

### R72. 沒有 `D:\HT9045\system\ContactInfo.ini` 的機台，開機時 3 個 EP 鍵會被寫成 0（Steven 改選「ContactForce 建構子開機就跑」〔R15＝B，裁決 S162〕之後的交件／ContactForce 與 AOI OCR 設定讀寫那一列待辦〔todo D-003〕）
**背景**：接觸力設定畫面（ContactForce）的建構程式現在照 golden 開機就跑；在沒有 `D:\HT9045\system\ContactInfo.ini` 的機台上，它會建這個檔，順手把 `D:\HT9045\system\Gerneral.ini` 裡 3 個 EP 鍵寫成 0。程式細節：Steven 在 R15 改選 B（Steven 20260927 11:0x 的裁決 S162：ContactForce 建構子開機就跑）。golden 建構子（V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\ContactForce.cpp:572-574`）用全域 `EP_MAXKPA_1032`／`EP_MAXAFB_1032`／`EP_MINMPA_1032` 填畫面，這時候還沒有人讀過它們，值是初值 0；如果機台沒有 `D:\HT9045\system\ContactInfo.ini`，會走建檔（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\ContactForce.cpp:1362-1365`），把 `D:\HT9045\system\Gerneral.ini` 的 `EP_MAXKPA_1032`／`EP_MAXA_1032`／`EP_MINMPA_1032` 寫成 0（原本有值也照蓋），`EP_MINA_FeedBack`／`EP_MINA_FeedBack_1032` 寫成 0.968。BCB6 機台也是這樣。
**選項**：A 照 golden（目前做法）／B 906 單邊改成先讀這幾個鍵再填。例：新機台 bUseDynamicKitDiameter 開著、沒有 `D:\HT9045\system\ContactInfo.ini`，`D:\HT9045\system\Gerneral.ini` 原本 `EP_MAXKPA_1032` 有值——A：開機後變 0（跟 BCB6 一樣）；B：開機後維持原值。
**St01建議**：A。只有「bUseDynamicKitDiameter 開著、而且沒有 `D:\HT9045\system\ContactInfo.ini`」的新機台會碰到；Steven01 這台有 `D:\HT9045\system\ContactInfo.ini`，不會碰到。
**目前狀態**：已照建議先做（照 golden），可推翻：commit `725038a6`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ContactForce.cpp:413-416` 的註解）。
**20260929 結果**：照 BCB，已做（commit `725038a6`）。

### R73. Configuration 要密碼的勾選框（C12／A27／N07-5／M01，盤點代號 CC-L2～L4）：網頁先用「整次拒存」擋（Q41 盤點／Steven 20260927 的裁決 S158（新頁面先不做，先把既有頁面還沒做的 golden 事件與權限等級檢查補齊））
**背景**：golden 是點下去當場問密碼，錯了把那一格改回、其他照存；網頁沒有「點下去當場讓 C++ 跑 golden 點擊程式」的入口（即時事件入口〔Q41 盤點 C-4〕／「從資料庫選一筆就自動填欄位」那題〔Q40〕），所以只能在存檔時擋。
**選項**：A 存檔時整次拒存，並說明原因／B 照 golden 把那幾格改回原值、其他照存，結果裡列出被改回的格子（SetUp 頁關 RTC／OCR〔SU-L1〕、BinSelect 的 Count 現在是這種做法）。
**St01建議**：A。操作員很清楚這次什麼都沒存；B 在 C12 群組會出現 golden 的怪處（改 C13，結果 C12 PE 模式被悄悄關掉）。例：勾 A27 再按 Save → A：這次什麼都沒存、畫面說明 A27 要密碼；B：A27 被改回原值、其他照存。
**目前狀態**：已照建議先做，可推翻：commit `a8eca460`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp:139-233`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp:306-314`）。
**20260929 結果**：由 Q45 的裁決取代（照 BCB；這幾格網頁版維持改不了，點了當場改回，見 R135）。

### R74. 不照翻 golden 的「程式還沒開完就不問密碼」守衛（Configuration 要密碼的勾選框〔CC-L2～L4〕／Steven 20260927 的裁決 S158（新頁面先不做，先把既有頁面還沒做的 golden 事件與權限等級檢查補齊））
**背景**：golden 在程式還沒開完（InitialOK 為 false）時，勾這幾個要密碼的框不問密碼、直接讓改（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6777`、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6827`、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6863`）；移植樹 wb_serve 從來沒把 InitialOK 設成 true（golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:10900` 那行沒翻，移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cmydef.cpp:285` 預設 false），照翻等於永遠不檢查。
**選項**：A 不照翻（一律檢查）／B 照翻。例（依下面的更正）：wb_serve 開機時啟動狀態機那一步（PumpInit）失敗時，在 Configuration 勾 C12 再存——A：照樣整次拒存；B：不問密碼、直接存進去。
**St01建議**：A。另外，把 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:10900` 接進 wb_serve 開機序列會影響所有看 InitialOK 的程式（例如開機時不產生 change log），要另外評估、另開題。
**目前狀態**：已照建議先做，可推翻：commit `a8eca460`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp`）。　⛔ **更正（ST01-E 20260927 16:4x）**：背景說「移植樹 wb_serve 從來沒把 InitialOK 設成 true」不對——wb_serve 開機時啟動狀態機那一步（PumpInit，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp:461`）成功時會設 true（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp:563`；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp:159` 的註解也要更正）。所以 A／B 只差在 PumpInit 失敗（例如開機時的機台組態自我檢查 sim canary〔`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridgeTags.cpp:463` 起〕拒絕）或開機完成前：golden（B）那時不問密碼、直接讓改；A 一律擋。現在程式是 A（比 golden 嚴一點）；照 golden 的話選 B。St01 仍建議 A（機台沒開完時更不該讓人改 PE 模式這類設定），請 Steven 決定。
**20260929 結果**：Q45 之後這幾格網頁一律改不了，這條 BCB 守衛不影響網頁；不用 Steven 決定。

### R75. SetUp 頁關 OCR／RTC：沒裝 RTC 的機台，網頁比 golden 嚴（SetUp 關 RTC／OCR 要重新驗證〔Q41 盤點 SU-L1〕的延伸／Steven 20260927 的裁決 S158（新頁面先不做，先把既有頁面還沒做的 golden 事件與權限等級檢查補齊））
**背景**：RTC＝Real Time CCD（即時影像檢查）。沒裝 RTC 的機台，BCB6 在 SetUp 頁取消 OCR 勾選直接生效；網頁（改之前）卻當作密碼錯、把勾改回來。程式細節：golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:4325` DoPassword 在沒裝 REAL_TIME_CCD 時直接回 true（不問，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSetUp.cpp:4333`）；移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\TestIF_File_SetUp.py` 原本把 DoPassword() 一律換成「密碼錯」（改之前的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_SetUp.gen.inc:3037`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_SetUp.gen.inc:3064`）⇒ 沒裝 RTC 的機台，網頁也關不掉 OCR。
**選項**：A 改成「有 REAL_TIME_CCD 才當密碼錯、沒有就直接過」（跟 golden 一樣）／B 維持現狀。
**St01建議**：A。例：沒裝 RTC 的機台在 SetUp 頁取消 OCR 勾選 → golden 直接生效（A 也是）；B（改之前的做法）：網頁會被改回來。
**目前狀態**：已照建議先做（A），可推翻：commit `97c70d62`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\TestIF_File_SetUp.py:74-77`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\TestIF_File_SetUp.py:272-277` 新增 SU_DoPassword；重產 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_SetUp.gen.inc:69`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_SetUp.gen.inc:3038`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_SetUp.gen.inc:3065`）。REAL_TIME_CCD 是執行期旗標（`D:\HT9045\system\Gerneral.ini` [System]，移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\database.cpp:750`）。沒裝 RTC：照 golden 直接過；有裝：網頁沒有密碼框，仍視同密碼錯（網頁的密碼框還沒接〔Q41 盤點 C-3／Q45〕）。（先前紀錄：還沒改（要改描述檔再重產 `--only TestIF_File_SetUp`，等產生器 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\gen_editlist.py` 上同事的改動提交後做）；Steven 沒反對就做。）
**20260929 結果**：照 BCB，已做（commit `97c70d62`）。

### R76. 網頁上的單選鈕沒有分組，On／Off 不會互斥（Q41 盤點〔既有頁面還沒做的 golden 事件與權限檢查〕時新發現；跨好幾頁）
**背景**：**這是什麼**：BCB 版（golden）畫面上成對的「On／Off」單選鈕（元件型別 TRadioButton）放在同一個框裡時會互斥——點 On，Off 自動熄掉。網頁頁面是由 BCB 的 .dfm 畫面檔轉出來的（轉換產生器叫 dfm2web，產生器程式不在這台，位置待查），轉出來的單選鈕在 HTML 裡沒有「組名」（name 屬性），瀏覽器就不會讓它們互斥；VCL 的規則是「同一個父層（同一個框）裡的單選鈕互斥」。**影響**：Yield 頁（`D:\HT9045\web\page\Setup.YieldMonitoring.html`）8 顆＝4 組 On／Off，golden 存檔時只讀「On 有沒有勾」（On->Checked，golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uYieldMonitoring.cpp:1891、1898、1903、1907`），網頁上一旦點過 On，點 Off 也關不掉、存進去還是 On；另外 SetUp 頁 6 顆（`D:\HT9045\web\page\Setup.SetUp.html`）、Temp_Set 頁 8 顆（`D:\HT9045\web\page\Setup.Temp_Set.html`）、Contact 頁 14 顆（`D:\HT9045\web\page\Setup.Contact.html`）、CounterSel 頁 12 顆（`D:\HT9045\web\page\Status.CounterSel.html`）、StartCondition 頁 3 顆（`D:\HT9045\web\page\Data.StartCondition.html`）、Configuration 頁 2 顆（`D:\HT9045\web\page\Config.Configuration.html`）要逐頁看是否同一父層。
**選項**：A 做一個共用修法：照 VCL 規則替沒有組名的單選鈕依父層自動補組名（放在網頁共用引擎 `D:\HT9045\web\page\ht9045_wire_engine.js` 或 dfm2web 產生器，要跟 Jimmy 對）／B 各頁自己補件（例：B 的話 Yield 頁補好了，Contact 頁那 14 顆要等有人另外改 `D:\HT9045\web\page\Setup.Contact.html` 才會互斥）。
**St01建議**：A。例：Yield 頁點了「On」又點「Off」→ 現在兩顆都亮、存成 On；修好後只剩 Off 亮、存成 Off。
**目前狀態**：還沒改；修在哪一層要 Jimmy 決定（引擎 `D:\HT9045\web\page\ht9045_wire_engine.js` 是 Jimmy 的檔），列入 Q41 盤點（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md`）追蹤。　**Jimmy 已做（main 分支的交接檔 `D:\HT9045\docs\handoff\TO_STEVEN.md:179`，20260927 16:2x 那一列；原寫「第 173 列」）**：在引擎做共用修法 groupBareRadios（`D:\HT9045\web\page\ht9045_wire_engine.js:2155-2180`；main commit `00f9a882`，改的主檔就是這支引擎），用 headless Edge（不開視窗的 Edge 瀏覽器）驗過 Yield 4 組、Security 頁 720 個 sec_* 單選鈕不受影響；St01 分支下次合 main 時進來。　⛔ 20260927 22:xx 更新：「St01 分支下次合 main 時進來」已經發生——`00f9a882` 已在 St01 分支裡（`D:\HT9045\web\page\ht9045_wire_engine.js:2166` 的 groupBareRadios）。
**20260929 結果**：照 BCB（同一個父層裡的單選鈕互斥）；修在網頁引擎（Jimmy 的檔），交 Jimmy（ST01-M 20260929 轉述：Jimmy 已修）。

### R77. Cleaning 頁「從資料庫選」下拉在 golden 永遠是停用的，網頁「一點就跑程式」照 golden 擋下（Q40 選 A 的交件／Steven 裁決 S157）
**背景**：**這是什麼**：Cleaning（自動清潔）頁有一個「Select from Database...」下拉（元件 cbbSelectTray），設計上是「從 Tray 資料庫選一筆，自動把盤的尺寸填進欄位」。網頁現在有一條新指令：網頁上的下拉／按鈕／勾選一動，就請 C++ 當下跑 golden 的那段處理程式（form.event；Steven 20260927 的裁決 S157：Q40 選 A，HotPlate、TrayForm、Cleaning 三頁共用這條指令，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md:391`）。**查到的事**：golden V912 畫面檔裡這個下拉是 `Enabled = False`、`Visible = False`（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\AutoClean\uCleaning.dfm:901-919`）；只有 ASE 高雄（CC_ASE_KaohSiung）、OSE、ASE 高雄 K3 三個客戶會把它設成看得見（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\AutoClean\uCleaning.cpp:1533-1545`）；整棵 V912 沒有任何地方把它設成可用（Enabled=true；St01 20260927 21:5x 用 `git -C D:\HT9045 grep -n cbbSelectTray b7fd4958 -- HT9011UC_Code_V3.33.912.0_20260908_Jimmy` 重查，10 筆都不是）⇒ golden 的處理程式 `cbbSelectTrayChange`（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\AutoClean\uCleaning.cpp:2322-2357`）永遠不會被觸發（看起來像 golden 漏寫了 Enabled=true）。
**選項**：A 照 golden 擋（網頁送這條指令，C++ 回 `bad-payload: cbbSelectTray cannot be operated now`，意思是「這個元件現在不能操作」）／B 只在 ASE 高雄／OSE／K3（看得到的那幾台）放行（偏離 golden；例：ASE 高雄的機台在網頁選一筆 Tray，C++ 照 golden 程式把盤的尺寸填好）／C 一律放行（偏離 golden；例：任何客戶的機台只要網頁送出這條指令就照填，連 golden 畫面上根本看不到這個下拉的機台也一樣）。
**St01建議**：A——沒有裁決就不修 golden。例：ASE 高雄的機台，Cleaning 頁看得到灰色的「Select from Database...」；網頁版點不了，送出也會被擋。
**目前狀態**：已照建議先做，可推翻：commit `76058840`（主檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_FormEvent.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\FormBridge.cpp`）。處理程式已照 golden 轉好、已註冊（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_Cleaning.gen.inc:3246-3310`，註冊在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_Cleaning.cpp:245`），改成放行只要改判斷條件。
**20260929 結果**：照 BCB，已做（commit `76058840`）。

### R78. 網頁「一點就跑程式」遇到「頁面上的清單已經過期」就擋（網頁才有的保護／Q40 選 A 的交件／S157：三頁共用 form.event）
**背景**：**這是什麼**：HotPlate 頁與 TrayForm 頁有「從資料庫選一筆」下拉：選第 n 筆，golden 就把 `D:\HT9045\system\PlateForm.csv`／`D:\HT9045\system\TrayForm.csv` 第 n 列的尺寸填進欄位——golden 的處理程式直接拿下拉的「第幾項」（ItemIndex）當 CSV 列號（golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cHotPlate.cpp:422-436`、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayForm.cpp:577` → `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayForm.cpp:589`）。**網頁才有的問題**：golden 視窗是當下開當下用；網頁頁面可能開很久，期間這兩個 CSV 被改過，頁面送來的「第幾項」就會對到別的列。
**選項**：A 頁面有帶選項文字（`text`）、而且跟伺服器清單第 itemIndex 項不同，就回 `bad-payload …stale, reload the page`（清單過期，請重新整理）；沒帶文字就不檢查（跟 golden 一樣）／B 拿掉這道檢查，完全照 golden（例：下面的情況會照 golden 直接填進 QFN 10X10 的數字，要操作員自己發現）。
**St01建議**：A。例：頁面開著時有人把 `D:\HT9045\system\PlateForm.csv` 第 3 列的 QFN2X2 刪掉，伺服器第 3 項變成 QFN 10X10；頁面送 3／"QFN2X2" 會被拒絕，不會默默填成 QFN 10X10 的數字。
**目前狀態**：已照建議先做，可推翻：commit `76058840`（一般設定頁共用的讀寫機制〔C 路〕那一段在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:412-417`；HotPlate 頁走較早的另一套讀寫機制〔A 形狀〕，那一段在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\FormBridge.cpp:300-305`）。
**20260929 結果**：已做，取安全側（commit `76058840`）：網頁才有的情況（BCB 的清單不會過期）；清單過期就請操作員重新整理。

### R79. HotPlate 頁每次「一點就跑程式」都先跑一次 golden 開窗程式（知會／Q40 選 A 的交件／S157：三頁共用 form.event）
**背景**：**這是什麼**：定案的做法要求「先跑 golden 的開窗顯示程式，再跑點擊處理程式」，這樣「這個元件現在點得到嗎」由 C++ 自己算，不信任頁面。HotPlate 頁每選一筆，C++ 都會先跑一次 golden 開窗程式 `TfHotPlate::FormShow`（golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cHotPlate.cpp:35`；C++ 呼叫在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\FormBridge.cpp:264`）：重讀配方的 `D:\HT9045\IniData\Data\<配方>\HotPlate.Data` 進機台正在用的記憶體、缺 `bTrayHotplateCheck` 鍵時補寫（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cHotPlate.cpp:203`）、重算 Hot Plate 的 Y pitch（SetArmHotPlateYPitch，golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cHotPlate.cpp:307`）。golden 只在開窗時跑一次；這裡的副作用跟網頁開 HotPlate 頁（`GET /api/form`）完全相同，運轉中會先被擋掉。
**選項**：A 照定案（目前；例：操作員選一筆「從資料庫選」→ C++ 先照 golden 重讀配方的 `D:\HT9045\IniData\Data\<配方>\HotPlate.Data`、判斷這個下拉點不點得到，再填 7 個欄位）／B 不重跑開窗程式，改信任頁面送的畫面狀態（伺服器就沒有「點得到嗎」的判斷，權限不夠也能填；例：同一個操作，C++ 不重讀、不判斷，頁面送什麼狀態就照填）。
**St01建議**：A。
**目前狀態**：已照建議先做，可推翻：commit `76058840`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HotPlateForm_File.cpp:416-468`；原寫 415-477，後來 commit `9268162b` 在同檔加了 3 行）。
**20260929 結果**：照 BCB，已做（commit `76058840`）。

### R80. S122：C++ 怎麼判斷操作員「關掉了」Teach／Motor Test 視窗（RULINGS_20260927 第 2 條第 18 題選 B／Steven 裁決 S122）
**背景**：**這是什麼**：Steven 20260926 22:xx 的規則 S122（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md:337`：「關瀏覽器分頁、要看是關閉什麼分頁，如果是 teach 或 motor test，會要求重新 find home」），20260927 07:4x 再裁定只做這一半（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260927.md:32`，第 2 條第 18 題 B）：關掉教導（Teach）或馬達測試（Motor Test）畫面後，機台標成「必須重新回原點」（程式裡是 `fAllMotorHome=false`），下一次按 START 會先整機回原點、回完**接著生產**（golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:6390`、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:6431`、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\csystem.cpp:10925`）。**更正**：這不是比 golden 嚴——golden 本來就在教導視窗關掉回來的下一行清旗標（`fTeach->ShowModal()` 之後，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28846-28847`）、Motor Test 關掉回來也清（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uteach.cpp:2442-2443`），是照翻。**網頁的難處**：網頁上 Teach／Motor Test 是主框架頁 `D:\HT9045\web\background.html` 裡的全螢幕視窗（關掉只是藏起來），C++ 唯一知道「視窗關了」的來源是主框架頁送來的「視窗總表」（指令 `ui.windows.put`：有變化 60 ms 內送、每 5 秒送一次心跳，15 秒沒收到新的就算過期，移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp:16`）。現在已經會清旗標的：Motor Test 的每個命令（含關視窗送的 formClose，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebMotorAccess.cpp:4137`）、Teach 頁載入（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Teach.cpp:259`）、Teach 存檔；缺的是 Teach 視窗按 EXIT。
**選項**：A 只用現成的視窗總表：每 500 ms 看 fTeach／fMotorTest（教導／馬達測試視窗）從「在用」變「不在用」就算關掉（按 EXIT 馬上算、F5 重新整理 15 秒內算、關瀏覽器／當掉要等下一個 HMI 連上；斷線重連、切到別的瀏覽器分頁不算）／B 另請 Jimmy 在網頁伺服器底層（WebBridge）開一個「連線關閉」事件（反應更快；例：操作員直接關掉瀏覽器，連線一斷 C++ 就知道，不用等下一個 HMI 連上）／C 主框架頁在瀏覽器分頁要關掉的那一刻（pagehide 事件）送最後一次總表（例：關瀏覽器前頁面先送一次「Teach 不在用」，C++ 馬上算關掉；但瀏覽器當掉時這一次送不出來）。
**St01建議**：A——不改網頁、WebBridge、視窗總表（WebWindowRegistry），全部用 St01 自己的檔（新檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.h`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp` 裡 St01 那一行同行插入，加 ctest）。例：操作員開 Teach 寸動 Z1 再按 EXIT → 主控台印 `[S122] fTeach closed -> fAllMotorHome=false` → 按 START 先回原點再接著跑。唯一例外：同時開兩個 HMI 分頁時可能誤判成關掉，代價只是下一次 START 多回一次原點。
**目前狀態**：已照建議先做（A），可推翻：commit `0b166feb`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.cpp`（每一拍的判斷 `W906_TeachLeaveTick` 在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.cpp:138`）＋`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5953` 同一行；ctest `test_teachleave`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_teachleave.cpp`）在 St01 自己的 build 目錄跑過 52 項全過）。另外：視窗總表某表單的回報全部過期時（瀏覽器關掉或分頁被節流超過 15 秒），這一拍沿用上一拍、不算一次「打開／關掉」（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.cpp:84-86`）——不然關瀏覽器會被當成「打開」而清旗標，跟本題說好的「關瀏覽器要等下一個 HMI 連上」相反。（先前紀錄：方案寫好（全文 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\S122_TEACH_LEAVE_PLAN_20260927.md` §2～§4，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\S122_TEACH_LEAVE_PLAN_20260927.md:151`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\S122_TEACH_LEAVE_PLAN_20260927.md:192`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\S122_TEACH_LEAVE_PLAN_20260927.md:260`），等工程師排到 wb_serve 空檔就做；Steven 沒反對就照 A。）　⛔ 20260927 22:xx 更新：`5b73d905` 把這一套判斷做成誰都能登記的「表單開／關掛勾」（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.cpp`，第一個用的是 St02 的 Tester IF 關窗），測試由 52 個擴充到 96 個、全過；Teach／Motor Test 這段行為沒變。
**20260929 結果**：已做（commit `0b166feb`）。

### R81. S122（關掉 Teach／Motor Test 要重新回原點）：打開 Teach／Motor Test 那一下也清旗標（照 golden）
**背景**：golden 開 Teach 就清「已回原點」旗標（教導視窗的開窗程式 FormShow，golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uteach.cpp:1610`），關掉又清。
**選項**：A 開、關都清（照 golden；例：操作員打開 Teach 之後一直沒關（例如瀏覽器當掉），打開那一下旗標已清，下一次 START 先回原點）／B 只在關掉時清（例：同一個情況，還沒偵測到關掉之前旗標不清，START 不回原點直接跑）。
**St01建議**：A。
**目前狀態**：已照建議先做（A），可推翻：commit `0b166feb`（同 R80：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.cpp`（「打開也清」的開關在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.cpp:27`）＋`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5953` 同一行；ctest `test_teachleave`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_teachleave.cpp`）在 St01 自己的 build 目錄跑過 52 項全過）。（先前紀錄：同 R80（方案 §5 S122-R2，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\S122_TEACH_LEAVE_PLAN_20260927.md:311`）。）
**20260929 結果**：照 BCB（開、關都清），已做（commit `0b166feb`）。

### R82. S122（關掉 Teach／Motor Test 要重新回原點）：運轉中（SystemStart）關掉 Teach／Motor Test 不清旗標，只記一行
**背景**：**這是什麼**：機台運轉中（程式旗標 SystemStart＝true）如果清掉「已回原點」旗標（`fAllMotorHome`），主流程 `DoAllProcess` 會每一拍都直接 return（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp:1629`），機台停在半路、沒有警報；而運轉中這兩頁的移動命令本來就全被擋（golden 運轉中也開不了這兩頁）。
**選項**：A 運轉中不清、只在主控台印一行（例：機台運轉中網頁上的 Teach 視窗被關掉 → 生產照跑，主控台多一行 `[S122] … while SystemStart=1 -> not cleared`）／B 照清（例：同一個情況，旗標被清，機台停在半路、畫面沒有警報）。
**St01建議**：A——跟第 18 題（RULINGS_20260927 第 2 條第 18 題：重新整理網頁不停產）同一個精神。
**目前狀態**：已照建議先做（A），可推翻：commit `0b166feb`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.cpp`＋`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5953` 同一行；ctest `test_teachleave`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_teachleave.cpp`）在 St01 自己的 build 目錄跑過 52 項全過）。（先前紀錄：同 R80（方案 §5 S122-R3，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\S122_TEACH_LEAVE_PLAN_20260927.md:317`）。另查到 Jimmy 的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Teach.cpp:259`（commit `a3db68f2`，主檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebMotorAccess.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Teach.cpp`）開頁時清旗標**不看 SystemStart**，可能在運轉中重新整理 HMI 時把生產停住（程式推論、未實測），已請 Jimmy 看（方案 §6 給 Jimmy 的第 1 項 J1，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\S122_TEACH_LEAVE_PLAN_20260927.md:335`）。）　⛔ 20260927 22:xx 更新：文中提到 Jimmy 的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Teach.cpp:259`「開頁就清要重新回原點、沒看機台在不在跑」那一點，Jimmy 已在 `8b5a91b5`（20260927 18:44）改成 `if (SystemStart == false) fAllMotorHome = false;`。另外評估案 Q49 查到：Teach 頁在開站時就在背景載好，這一行在每次開 HMI（機台停著時）都會跑一次 ⇒ 下次 START 會先全部回原點（由程式推得、沒實測；已列給 Jimmy）。
**20260929 結果**：已做（commit `0b166feb`）：運轉中不打斷生產，只在主控台記一行。

### R83. S122（關掉 Teach／Motor Test 要重新回原點）：操作員看到什麼
**背景**：golden 關掉 Teach 之後畫面沒有任何提示，按 START 時自己先回原點再接著跑。
**選項**：A 照 golden 不加畫面（例：操作員關掉 Teach 回到主畫面，主畫面跟平常一樣，按 START 才看到機台先回原點）／B 主畫面加一行「下次 START 會先回原點」（例：同一個情況，主畫面多這一行字，按 START 前就知道）。
**St01建議**：A。
**目前狀態**：已照建議先做（A），可推翻：commit `0b166feb`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.cpp`＋`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5953` 同一行；ctest `test_teachleave`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_teachleave.cpp`）在 St01 自己的 build 目錄跑過 52 項全過）。（先前紀錄：同 R80（方案 §5 S122-R4，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\S122_TEACH_LEAVE_PLAN_20260927.md:323`）。）
**20260929 結果**：照 BCB，不加畫面，已做（commit `0b166feb`）。

### R84. Contact 頁的「單位換算＋重新載入參數」照 golden 在開頁時跑，不是關窗後（Q41 盤點第一節第 3 項「關窗尾段」／盤點表 CT-4 列）
**背景**：**這是什麼**：golden 主畫面開了某些設定視窗之後，會接著做「單位換算（DoStructUnitConvert）＋重新載入工作參數（SetWorkParameter：機台下一動要用的速度、位置等）」，讓機台馬上用新值；Q41 盤點把它叫「關窗尾段」（盤點第一節第 3 項，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md:36`）。Contact 頁不一樣：golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28314` 是 `fContact->Show()`（Jimmychiu 20240731 把「等視窗關掉才往下」的 ShowModal 改成不等的 Show），換算和重載參數（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28315-28316`）在開窗當下就跑，所以不是「關窗尾段」（盤點表 CT-4 列寫錯，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md:203`）。網頁存完一定會自動重讀，所以實際上存完也會再跑一次。
**選項**：A 照 golden 掛在開頁（網頁開頁向 C++ 要畫面值〔editlist.get〕的地方，移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\DeviceForm_File.cpp:213`；例：操作員打開 Contact 頁、什麼都沒改就關掉 → 開頁當下跑一次換算＋重載，跟 golden 一樣）／B 跟其他頁一樣掛在存檔後（例：同一個情況沒存檔就不跑；改了值存檔才跑）。
**St01建議**：A。
**目前狀態**：已照建議先做（A），可推翻：commit `c8028b21`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\DeviceForm_File.cpp:213` 開頁時跑 `W906_Main_sbContactClickOpen`，本體在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp:693`〔檔尾〕）。（先前紀錄：方案寫好（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_CLOSETAIL_PLAN_20260927.md` §3，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_CLOSETAIL_PLAN_20260927.md:106`），工程師實作中；Steven 沒反對就照 A。）
**20260929 結果**：照 BCB（開頁時跑），已做（commit `c8028b21`）。

### R85. 其餘 8 頁存檔後照 golden 補跑「關窗尾段」（Q41 盤點第一節第 3 項／Steven 裁決 S107-1 的延伸）
**背景**：**這是什麼**：golden 關掉 Ld_ULd、TrayForm、HotPlate、TrayAssignment、Speed、Yield、BinSel、Configuration 這幾個設定視窗後，主畫面會再跑單位換算（DoStructUnitConvert）、重新載入工作參數（SetWorkParameter）等（各頁不同，逐頁表在方案 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_CLOSETAIL_PLAN_20260927.md` §3，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_CLOSETAIL_PLAN_20260927.md:106`）；網頁存完都沒跑 ⇒ 機台不一定馬上用新值。Temp_Set、SetUp 兩頁已照 Steven 20260926 17:5x 的裁決 S107-1（「存檔後就跑，不改成等 Exit」，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md:284`）做了。**更正**：盤點表（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md:332`）與 5 處程式註解（位置待查）寫的「觸發時機是 S88 待決 Q1」（S88＝溫度頁存檔後補跑收尾那一件工作，Q1＝它留下的「收尾什麼時候跑」這一題）已過期，S107-1 早就結案。S107-1 字面上只講溫度頁，這 8 頁照同一做法。**會寫檔的 3 項**：重讀 Auto Clean 資料（LoadAutoCleanData）寫配方 `D:\HT9045\IniData\Data\<配方>\HandlerCondition.Data` 的 iIndexArmAutoCleanCnt、重建起動模式清單（SetStartModeData）寫 `D:\HT9045\system\RunMode.txt`（並在事件紀錄記 MES2107「Change Start Mode」與 ChangeLog「Change bin mode」兩筆，golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:1150-1151`）、更新主畫面運轉模式（UpdateMainOperateMode，golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:13326`）寫 `D:\HT9045\system\lastdata.dat`（並切加熱器繼電器、送指令給 ATC 7.0 溫控機〔ATC7〕）。重畫分 bin 顯示（ShowBinSel）不是純畫面：它重算分 bin 用的 iBinTray，START 時不會重跑。
**選項**：A 照 S107-1：每頁存檔成功後跑它的 golden 尾段／B 等操作員關視窗（Exit）才跑（例：下面 Speed 的例子，要等操作員在網頁按 Exit 關掉 Speed 視窗，機台才改用 80）。
**St01建議**：A。例：在 Speed 頁把 Index Arm 速度從 50 改成 80 存檔 → golden 關窗後 SetWorkParameter 重載，機台下一動就用 80；現在網頁存完要重開程式才生效。
**目前狀態**：第一批已照建議先做（A），可推翻：commit `c8028b21`——Ld_ULd（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Ld_UldDelayTime.cpp:70`）、Speed（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\ArmSpeed_File.cpp:63`）、BinSel（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\BinSelect.cpp:885`，從 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\BinSelect.cpp:627` 呼叫）存檔後跑 golden 尾段，本體在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp:612`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp:636`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp:663`（檔尾）＋新標頭 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClickTail.h:25-28`；TrayForm、HotPlate、TrayAssignment、Yield、Configuration 下一批。會多寫的檔：Contact 開頁與 BinSel 存檔的 SetWorkParameter 缺鍵時會補寫 `D:\HT9045\system\Gerneral.ini` [Shuttle] CHECK_RANGE／iInShtZRange（已列給 Jimmy 的 sysguard——筆電「測試前後比對機台共用檔有沒有被改」的預期變動清單）。（先前紀錄：方案寫好；Ld_ULd、Speed、BinSel 先做（工程師實作中）；TrayForm、HotPlate（Q40 的頁）、TrayAssignment（「一點就跑程式」form.event 同事在改）、Yield、Configuration 等其他同事交件後再做。Steven 沒反對就照 A。）　**第二批也已做 `9268162b`**（主檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp:760-954`）：TrayForm（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\UserDefForm_File.cpp:79`）、HotPlate（HotPlate 頁用的較早那套讀寫機制〔A 形狀〕：產生器 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\gen_formbridge.py:330` 新增選用鍵 saveFlowAfter，重產 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HotPlateForm_File.cpp:456-461`）、Tray Assignment（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TrayForm.cpp:495`）、Yield（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_YieldMonitoring.cpp:68`）、Configuration（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp:402` 開頁記舊值、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp:351` 關窗尾段不看有沒有存成，照 golden）。多寫的檔（已列給 Jimmy 的 sysguard）：配方 `D:\HT9045\IniData\Data\<配方>\HandlerCondition.Data` 的 iIndexArmAutoCleanCnt、`D:\HT9045\system\RunMode.txt`、`D:\HT9045\system\lastdata.dat`（Configuration 每存一次都寫，答「否」也寫，golden 同）、配方 `D:\HT9045\IniData\Data\<配方>\Tester.Data` 缺鍵補寫；Configuration／Yield 存檔也會照 golden 切一次加熱器繼電器、送 ATC7。
**20260929 結果**：照 Steven 規則 S107-1，已做第一批（commit `c8028b21`）。

### R86. 每支「關窗尾段」開頭再查一次機台停著（Q41 盤點第一節第 3 項）
**背景**：**這是什麼**：尾段會重載參數、寫 `D:\HT9045\system\lastdata.dat`、切加熱器繼電器；運轉中存檔已由 RULINGS_20260927 第 2 條第 7 題（Steven 0927 07:2x 照建議：運轉中網頁存設定一律擋，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260927.md:22`）擋在一般設定頁的存檔指令 editlist.save，但 HotPlate 走另一個存檔指令 form.save（見 R87）。多查一次不影響停機時的行為。
**選項**：A 每支尾段開頭查 `SystemStart||SoftStart`（運轉中，或正要啟動／回原點），運轉中不跑並記一行（例：機台正在回原點時，有一筆存檔從沒擋到的入口進來 → 尾段不跑，主控台印 `close-tail <函式> -> not run: …`，回覆說明「…停機後這一頁再存一次（Contact 頁：再開一次）就會跑」）／B 不查（靠入口的擋；例：同一個情況，回原點途中照樣重載參數、切加熱器繼電器）。
**St01建議**：A。
**目前狀態**：已照建議先做（A），可推翻：commit `c8028b21`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp:585` CloseTailRunning）。（先前紀錄：同 R85。）
**20260929 結果**：已做，取安全側（commit `c8028b21`）：多一道「運轉中不跑」的檢查。

### R87. HotPlate 頁存檔（form.save）運轉中也擋（RULINGS_20260927 第 2 條第 7 題的延伸）
**背景**：**這是什麼**：第 7 題（Steven 0927 07:2x 照建議，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260927.md:22`）是「運轉中，一般設定頁的存檔指令 editlist.save 一律擋」，做在移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5303`（commit `f45b92f5`；這顆 commit 第 7 題那一段就是這一行，另外改 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebCmdGuard.cpp` 做第 8、9 題的防連點）；但 HotPlate 頁用的是較早的另一套讀寫機制（A 形狀），存檔走 WebSocket 指令 `form.save`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5340` → `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\FormJson.cpp:115` FormSave），**運轉中沒有擋**。
**選項**：A 照第 7 題同樣擋（回 `running`）／B 不擋。
**St01建議**：A——跟第 7 題同一件事，只是入口不同。例：機台運轉中，有人在 HotPlate 頁改 XPitch1 按存檔 → A：回「運轉中不能存」；B：直接寫進配方 `D:\HT9045\IniData\Data\<配方>\HotPlate.Data`。
**目前狀態**：已照建議先做（A），可推翻：commit `c8028b21`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\FormJson.cpp:119` FormSave 在運轉中回 `running:`（判斷本體 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\FormJson.cpp:231`），`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp` 沒動）。（先前紀錄：工程師實作中（改在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\FormJson.cpp` 的 FormSave，不動 wb_serve）；Steven 沒反對就照 A。）
**20260929 結果**：照 Steven 規則（RULINGS_20260927 第 2 條第 7 題），已做（commit `c8028b21`）。

### R88. Data.ContactCT 頁每秒更新：兩台瀏覽器看不同選項時會互相切換（St01 工作項 S115 的交件）
**背景**：**這是什麼**：Data.ContactCT 頁（`D:\HT9045\web\page\Data.ContactCT.html`；golden 視窗標題「Contact Counter Kinds」）顯示接觸次數與良率統計表，上面有一組選項（元件 rgYieldType：History、Total、Kind、Kind(%)、ByHeadYield…，golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContactCT.dfm:36-47`）切換看哪一種統計。St01 的工作項 S115（頁面開著時即時更新，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md:308`）做好後，這頁每秒送一次查詢 `contactct.get {"yieldType":<正在看的選項>}`（Steven 20260927 08:5x 的裁決 S124＝B：唯讀查詢免權杖，「一點點誤差是沒關係的，因為可能一秒鐘就更新一次」，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md:348`）。C++ 端的選項（rgYieldType 的 ItemIndex）只有一份，兩台瀏覽器同時開這頁、看不同選項時，每一拍都會觸發一次 golden 的選項點擊程式 rgYieldTypeClick（golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContactCT.cpp:724-738`）。在 `TestIF_File.bLowYieldAlarmByBin`（依 bin 判斷低良率警報）開著時，它只會改 SortCT 頁 pnlYield／pnlYieldART 的顯示字（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContactCT.cpp:735-736`），不寫檔、不動機台。golden 只有一個視窗，沒有這種情況。
**選項**：A 接受（照 S124「一點點誤差沒關係」）／B 在查詢值加「只重畫」旗標，C++ 收到就不觸發點擊程式（OnClick）（要改網頁伺服器的解析 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5232-5238`；例：下面的情況 SortCT 頁的 Yield 顯示字不再跳，代價是動 wb_serve 共用的解析）。
**St01建議**：A。例：甲電腦看 History、乙電腦看 Total（⛔ 更正：原寫「Current」，選項裡沒有這一項；選項是 History、Total、Kind、Kind(%)、ByHeadYield…，golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContactCT.dfm:36-47`），兩邊畫面都正常更新；只有 SortCT 頁的 Yield 顯示字會在兩種之間跳。
**目前狀態**：已照建議先做（A），可推翻：commit `c35f375e`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cContactCT.cpp:1646-1655`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cContactCT.cpp:1766-1774`；網頁 `D:\HT9045\web\page\ht9045_contactct_wire.js`）。
**20260929 結果**：照 Steven 規則 S124（一點點誤差沒關係），已做（commit `c35f375e`）。

### R89. 運轉中照 golden 開不了「工具」／「設定」選單裡的設定頁（Q41 盤點 C-1／C-2 列的交件）
**背景**：**這是什麼**：golden 主畫面進設定頁要先按「工具」選單鈕（sbSettingClick，golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:29030-29037`）或「設定」選單鈕（sbConfigClick，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:29009-29016`），兩支第一行都是 `if(SystemStart) return;`（運轉中直接不理），運轉中主畫面的 DoMainPadProcess（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:3970-3978`）也把兩個選單藏起來 ⇒ 運轉中開不了這兩個選單裡的任何頁（連 Setup.BinSel〔`D:\HT9045\web\page\Setup.BinSel.html`〕也是）。Q41 盤點表的 C-1／C-2 列（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md:53-54`）就是這兩道閘。C++ 現在照這個，網頁開頁要畫面值（editlist.get）時回 `running:`。運轉中經一般設定頁讀寫機制（C 路）還能開的只剩 Setup.OffSet（`D:\HT9045\web\page\Setup.OffSet.html`）、Main.AOAInfo（`D:\HT9045\web\page\Main.AOAInfo.html`）（golden 不查），以及沒在回原點時的 Setup.Speed（`D:\HT9045\web\page\Setup.Speed.html`；golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28680-28681` 只在 `SystemStart && iHome` 時不開）。**跟 Jimmy 的政策表不一致**：`D:\HT9045\.github\specs\page-access-policy.md:23-29`（§2）寫「SystemStart 不會鎖住整個 HMI（例：Bin 設定）」。
**選項**：A 照 golden 擋（目前）／B 開頁放行、只擋存檔（存檔運轉中本來就擋，RULINGS_20260927 第 2 條第 7 題）。
**St01建議**：A（照 golden）；要改 B 只要改閘表一欄。例：機台運轉中，工程師想打開 Setup.BinSel 看設定 → A：網頁回「運轉中不能開（golden sbSettingClick）」；B：可以看、不能存。
**目前狀態**：已照建議先做，可推翻：commit `cb306f89`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:759-788` 閘表 kOpenGates〔每一頁 golden 的開窗條件〕、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:792` 判斷 OpenGateRefused；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5207`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5303` 同一行）；政策表的差異已請 Jimmy 看。
**20260929 結果**：照 BCB，已做（commit `cb306f89`）。

### R90. 等級不足時不跳 golden 的「等級不足」警報框（WAR1676），把原因回給頁面（Q41 盤點 C-1／C-2）
**背景**：**這是什麼**：golden 查等級的函式 `fSecurity->Insufficient(n)` 等級不夠時會跳 WAR1676 警報框等人按。網頁單獨開一頁、旁邊沒有能顯示對話框的主框架頁時，伺服器會停在這個警報等人回答（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\data_builder_probe.py:27-28` 檔頭記過；閘表的說明在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:543-546`）。
**選項**：A 不跳，拒絕原因回給頁面（例：「等級不足：工具選單需要等級 0（目前登入 Operator）——golden main.cpp:29036」，出處是 golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:29036`）／B 照 golden 跳（例：同一個情況跳 WAR1676 框；單獨開的網頁沒有地方顯示，伺服器會停在這個警報等人按）。
**St01建議**：A。
**目前狀態**：已照建議先做，可推翻：commit `cb306f89`（主檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp`）。
**20260929 結果**：已做（commit `cb306f89`）：BCB 的警報框在伺服器上跳不到網頁，改成把原因回給頁面顯示。

### R91. 一頁有兩條 golden 開法時，任一條過就放行（Q41 盤點 C-1／C-2）
**背景**：例：BarCode 頁（`D:\HT9045\web\page\Setup.BarCode.html`）golden 可以從工具選單開（要等級 88，golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:29910-29917`），也可以從 Teach 頁的 BarCode 鈕（sbBarCode）開（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uteach.cpp:4603-4607`，不查 88）。
**選項**：A 任一條過就放行（照 golden）／B 只認網頁選單對應的主路（比 golden 嚴；例：下面那個人在網頁上開不了 BarCode 頁）。
**St01建議**：A。例：有 Teach 權限（權限項目 86 Motion View＋87 Teach）但沒有 88 的人，照 golden 可以從 Teach 頁開 BarCode。
**目前狀態**：已照建議先做，可推翻：commit `cb306f89`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:773` BarCode 那一列）。
**20260929 結果**：照 BCB，已做（commit `cb306f89`）。

### R92. Handler System 頁（HW.HandlerSys）在「當場輸入密碼」接好之前只靠「要原廠等級」這一道（Q41 盤點 C-1／C-2）
**背景**：**這是什麼**：Handler System 頁（`D:\HT9045\web\page\HW.HandlerSys.html`）是機台硬體設定頁。golden 開這頁除了要原廠等級（HonPrec，最高等級，只有原廠帳號有）、運轉中不開、ASE 高雄不開之外，還有隱藏手勢、確認框與寫死的密碼（golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTemperFrom.cpp:1744-1778`；密碼不寫進文件，Steven 20260925 晚間的裁決 S41「密碼資料不可暴露給瀏覽器」，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260925.md:177`）。網頁還沒有「當場輸入密碼」的對話框（Q41 盤點 C-3 列；怎麼做是本檔前面的 Q45 題，還沒裁決）。
**選項**：A 先只查原廠等級＋運轉中＋客戶，密碼等 Q45 一起做（例：原廠工程師用原廠帳號在停機時開這頁 → 網頁直接開得到；golden 還要做隱藏手勢、按確認、輸密碼）／B 在 Q45 定案前整頁擋（例：同一個人，Q45 定案前網頁一律開不了這頁）。兩個選項下 Operator 帳號都開不了。
**St01建議**：A——原廠等級是最高等級，只有原廠帳號有。
**目前狀態**：已照建議先做，可推翻：commit `cb306f89`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:788` HSys 那一列）。
**20260929 結果**：由 Q45 的裁決接手（照 BCB 當場重新登入＋等級）；已做的等級／運轉中／客戶檢查保留（commit `cb306f89`）。

### R93. 沒登記在閘表的設定頁一律拒絕開（Q41 盤點 C-1／C-2）
**背景**：**這是什麼**：新閘表 kOpenGates（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:759-788`，每一列記一頁 golden 的開窗條件）目前 25 列，涵蓋全部 22 個已登記的一般設定頁（C 路頁），另含 IniConfig（Configuration 頁）、Offset_File（OffSet 頁）、Teach。以後新增 C 路頁忘了補一列，那一頁就開不了（回 `no-gate:`）。
**選項**：A 沒登記一律拒絕（安全側，忘了補會立刻看到；例：之後有人把還沒有頁面的 Winway 設定接上網頁、忘了補一列 → 網頁一開就回 `no-gate:`，馬上發現）／B 沒登記就放行（例：同一個情況頁面開得到，但 golden 的等級與運轉中檢查都沒做，沒有人會發現）。
**St01建議**：A；新增 C 路頁的流程（skill 文件 `D:\HT9045\.claude\skills\ht9045-html-json\references\route-c-golden-bridge.md:376`，§3.0h）要加一步「在 kOpenGates 補一列」。
**目前狀態**：已照建議先做，可推翻：commit `cb306f89`（主檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp`）。　⛔ 20260927 22:xx 更新：建議裡「新增 C 路頁要加一步『在開窗閘表 kOpenGates 補一列』」已經寫進 skill（`D:\HT9045\.claude\skills\ht9045-html-json\references\route-c-golden-bridge.md:376`）。
**20260929 結果**：已做，取安全側（commit `cb306f89`）：沒登記的設定頁一律不開。

### R94. Tray Assignment 方向圖：C++ 替身元件的 Tag 改存「目前的圖號」，存檔時照 golden 補點（Q41 盤點 TA-1 列／Steven 裁決 S158：先補既有頁面的 golden 事件與等級檢查）
**背景**：**這是什麼**：Tray Assignment 頁（`D:\HT9045\web\page\Setup.TrayAssignment.html`）每個盤位旁有一張方向圖，golden 每點一下換下一個擺放方向（共 8 種），存檔寫進配方（盤點表 TA-1 列，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md:97`；這一批是照 Steven 20260927 的裁決 S158「先根據目前既有的頁面，有 event 或有卡 level 還沒有實作的先做」補的，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md:392`）。golden 的點擊程式（`imgLoaderClick`，V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp:971-991`）是用畫面檔（DFM）裡的 Tag（0..12，第幾個盤位）當 `iTrayDirect[]` 的索引、換下一張 type<n>.bmp。網頁共用引擎對圖片的慣例是「tag＝圖號、點一下本地 +1、存檔送 tag」（`D:\HT9045\web\page\ht9045_wire_engine.js:1166-1175`，Jimmy 的檔）。C++ 端代替 golden 元件的「替身」如果照產生器把索引放進 Tag，網頁會顯示錯圖、存檔時還把索引改掉。
**選項**：A 替身 Tag＝圖號、索引改由 `TA_ImgIndex()`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TrayForm.cpp:74`）查表；存檔時頁面送的 tag 跟 `iTrayDirect` 不同，就照 golden 一下一下點到一致（最多 8 下）／B 保留 golden 索引、改網頁引擎（Jimmy 的檔；例：C++ 不用改，但要等 Jimmy 改好引擎，改好前網頁顯示錯圖）／C 不做存檔補點（沒送事件就丟掉方向變更）。
**St01建議**：A。例：Jimmy 還沒在網頁控制項標上「要送點擊事件」（data-ht-event）之前，操作員在網頁上點 Loader 方向圖兩下再存檔 → A：伺服器照 golden 補點兩下，方向存成新值；C：方向變更被默默丟掉。
**目前狀態**：已照建議先做，可推翻：commit `4e74e8b4`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TrayForm.cpp`（存檔補點在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TrayForm.cpp:333` BeforeApply）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\TrayForm.py`，重產 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TrayForm.gen.inc`）。
**20260929 結果**：已做（程式內部做法，commit `4e74e8b4`），不用 Steven 決定。

### R95. Tray Assignment 存檔時重查「Fix 盤有料不准切模式」（Q41 盤點 TA-3 列／S158：先補既有頁面的 golden 事件）
**背景**：**這是什麼**：Tray Assignment 頁的「Fix Tray Mode」（[1] Full Bin／[2] Up Down）決定 Fix 盤怎麼用。golden 點這組選項的程式 `rgFixTrayModeClick`（golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp:1166-1195`）在任何一個 Fix 盤上還有料時，把模式改回原值 `TrayForm.iFixTrayMode`（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp:1193`）。網頁「一點就跑程式」（form.event）會照翻；但頁面直接存檔（沒送事件）就能繞過這道互鎖。
**選項**：A 存檔時也重查：頁面值跟伺服器不同就照 VCL 點一下並跑 golden 處理程式，被改回時多一句訊息（例：Fix1 盤上還有 IC，操作員在網頁把 Full Bin 改成 Up Down 直接存檔 → 模式照 golden 改回 Full Bin、其他改動照存，回覆多一句「Fix 盤上還有 IC：Fix Tray Mode 沒有切換」）／B 不重查（例：同一個情況 Up Down 直接存進配方）／C 值不同就整頁拒存（例：同一個情況整頁都不存，連其他改動也要重做）。
**St01建議**：A——這是機台馬達／料盤狀態（MOT）的互鎖，不信任前端。
**目前狀態**：已照建議先做，可推翻：commit `4e74e8b4`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TrayForm.cpp:342-353`）。
**20260929 結果**：已做，取安全側（commit `4e74e8b4`）：存檔時再查一次互鎖。

### R96. Tray Assignment 有連動的單選群組，沒送事件時存檔不重播（Q41 盤點 TA-2／TA-4 列／S158：先補既有頁面的 golden 事件）
**背景**：**這是什麼**：Tray Assignment 頁有幾組選項點了會牽動別的選項：Loader 類型（rgLoaderType）在 KYEC 機台點了要先刷條碼、沒刷就改回（golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp:1153-1164`）；RGLoader／rgLoad_RT／RGAuto2／rgAuto2_RT 點了會檢查 bin 設定、停用別的選項（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp:1233`、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp:1275`、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp:1772-1779`、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp:1197`；盤點表 TA-2／TA-4 列，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md:98`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md:100`）。golden 的結果跟點的先後有關，伺服器存檔時只看得到最後的值。
**選項**：A 維持現況（頁面直接存可以繞過 bin 守衛；等 Jimmy 標好事件後再評估；例：頁面直接改 RGAuto2 並存檔，golden 點下去時會依 bin 設定停用 RGAuto3，這一步沒發生、照存）／B 值不同就拒存（跟 R97 的 Temp_Set〔盤點 TS-1 列〕一樣；例：同一個情況整頁拒存，要頁面改用點擊事件）／C 用固定順序重播（例：伺服器照一個固定順序替頁面補點，但操作員實際點的順序不同時，結果可能跟 golden 不一樣）。
**St01建議**：A，Jimmy 在網頁控制項標好「要送點擊事件」（data-ht-event）後再看要不要改 B。
**目前狀態**：已照建議先做，可推翻：commit `4e74e8b4`（主檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TrayForm.cpp`）。
**20260929 結果**：現況先維持（commit `4e74e8b4`）；事件接好之後自然照 BCB（Steven 20260928「任何畫面的事件都是我們做」）。

### R97. Temp_Set 換加熱模式／校正來源，沒送事件就存檔照樣拒存（Q41 盤點 TS-1 列／S158：先補既有頁面的 golden 事件）
**背景**：**這是什麼**：溫度設定頁（`D:\HT9045\web\page\Setup.Temp_Set.html`）的「Index Heating Mode」（[0] Head Only … [5] Head + Chamber + Socket）與「Temperature calibration by recipe」勾選，golden 一點就重讀另一份溫度補償表（rgIndexHeatModeClick，golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp:4232-4236`；chkTempCalByRecipeClick，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp:6333-6341`；補償表在 `D:\HT9045\IniData\DefineTemp\`，golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp:2909`）。頁面沒送「一點就跑程式」（form.event）就存時，送來的補償值還是舊表的，照存會把舊模式的補償靜默寫進新模式的 DefineTemp 檔；存檔時也分不出使用者先換模式還是先改補償，沒辦法替頁面重播。比較對象改成「套值前伺服器的模式」與 `Temperature.bTempCalByRecipe`（ReadTempFile 讀表時用來選哪一份的全域變數；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Temperature.cpp:163`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Temperature.cpp:186-188`）。
**選項**：A 保留拒存（訊息改成說明「頁面沒有送 form.event」；例：操作員在網頁把 Index Heating Mode 從 Head Only 改成 Head + Chamber、沒送點擊事件就改了幾格補償值存檔 → 整頁不存，訊息說明「…頁面沒有送點擊事件（form.event）…頁面重讀後請再點一次（頁面要送 form.event），再存檔。這次沒有存檔。」）／B 拿掉（例：同一個情況照存，Head Only 那份的補償值被寫進 Head + Chamber 用的補償檔）／C 存檔時重播（例：伺服器替頁面補點一次模式再存，但分不出操作員是先換模式還是先改補償，改過的補償值可能被新表蓋掉）。
**St01建議**：A。
**目前狀態**：已照建議先做，可推翻：commit `4e74e8b4`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Temperature.cpp:178-210`）。
**20260929 結果**：已做（commit `4e74e8b4`）；事件接好之後就照 BCB 點一下跑一次。

### R98. Temp_Set 點擊時的讀檔照 golden 會補寫檔案（Q41 盤點 TS-1 列／S158：先補既有頁面的 golden 事件）
**背景**：**這是什麼**：R97 的點擊程式裡，golden 重讀補償表用的是 `ReadTempFile(false)`（golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp:1986`；移植樹照翻在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Temperature.gen.inc:2989`）。它照 golden 會補寫缺的鍵、寫 [ATC] Chiller Temp（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp:2613`），變體檔不在或版號舊時還會從基底檔複製 DefineTemp 的 *60mm／*_ATC 檔（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp:2923-3017`）——這跟 Q14＝B 有衝突（Steven 20260927 的裁決 S136：HandlerSys 的 Heater Type「按了先不寫檔、等整頁存檔一起寫」，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md:365`）。
**選項**：A 照 golden（現在；例：配方的補償檔缺了幾個鍵，操作員在網頁點 Index Heating Mode、還沒按存檔就離開 → 點的當下檔案已經補寫、Chiller Temp 已寫、必要時 *60mm 檔已從基底檔複製出來）／B 做一支不寫檔的讀法（同 Q15 那一類——Steven 20260927 的裁決 S137「開頁不寫檔」用的讀法，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md:419`；要改共用的 ReadTempFile；例：同一個情況點的當下什麼檔都不寫，只有按存檔才寫）。
**St01建議**：A。
**目前狀態**：已照建議先做，可推翻：commit `4e74e8b4`（主檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Temperature.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Temperature.gen.inc`）。
**20260929 結果**：照 BCB，已做（commit `4e74e8b4`）。

### R99. Temp_Set 一點就換掉機台記憶體的補償表（Q41 盤點 TS-1 列／S158：先補既有頁面的 golden 事件；要 Jimmy 確認溫控端）
**背景**：**這是什麼**：golden 點下去的當下，機台記憶體裡的補償表（全域 `Temperature.fTempOffSet[][]`）就換成另一份，存檔之前就生效；算加熱器設定值的 ConvertTempOffset（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\bthermo.cpp:282`）讀的就是它。不存檔的話，要等下次開頁或存檔失敗的重讀（Reload）才換回檔案值。運轉中「一點就跑程式」（form.event）直接回 running（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_FormEvent.cpp:64`）。
**選項**：A 照 golden（現在；例：停機時操作員在網頁把 Index Heating Mode 改成 Head + Chamber、看了新補償值但沒存檔就關頁 → 機台記憶體已經是新那份補償表，之後算加熱設定值就用它，直到下次開頁才換回檔案值）／B 事件後把全域值還原，只把結果顯示給頁面（偏離 golden，存檔流程也要跟著改；例：同一個情況記憶體維持原來那份，頁面只是顯示新表）。
**St01建議**：A，但請 Jimmy 確認溫控任務什麼時候讀這個陣列。
**目前狀態**：已照建議先做，可推翻：commit `4e74e8b4`（主檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Temperature.cpp`）。
**20260929 結果**：照 BCB，已做（commit `4e74e8b4`）。

### R100. 開頁時設單選群組不觸發點擊程式（既有缺口／Q41 盤點 Tray Assignment 各列／S158：先補既有頁面的 golden 事件）
**背景**：**這是什麼**：golden 開 Tray Assignment 頁時（DoIniDataToForm，golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp:548`，設值在 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp:570-626`）設單選群組（RadioGroup）的選項（ItemIndex），VCL 會順便觸發它的點擊程式（OnClick）⇒ 開頁就算好 RGAuto3／rgAuto3_RT 等的停用狀態、跑 Empty／Color 連動、甚至跳 bin 訊息；網頁的一般設定頁讀寫機制（C 路）開頁時沒有模擬這一步（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TrayForm.cpp:47` TA_SetRadioIndex 只設值、不觸發）。
**選項**：A 維持（現在；例：配方設定會讓 golden 停用 RGAuto3 → golden 開頁時 RGAuto3 已經是灰的；網頁開頁時不是灰的，要等頁面送過 RGAuto2 的點擊事件才算）／B 開頁也照 VCL 觸發（開頁行為會改變，要另外研究；例：同一個情況網頁開頁就跟 golden 一樣是灰的，但開頁也可能跳 bin 訊息）。
**St01建議**：A，先記著；Jimmy 標事件、上機驗證後再看要不要做 B。
**目前狀態**：沒有改動。
**20260929 結果**：照 BCB：開頁時照 golden 觸發連動（例：該灰的 RGAuto3 開頁就是灰的），排進事件批（Steven 20260928「任何畫面的事件都是我們做」），跟 R118 一起做。

### R102. 設定頁「目前在第幾個分頁」的值不合法時只丟那一筆、記待辦，不整份拒存（一般設定頁讀寫機制〔C 路〕共用層的分頁值 activePageIndex／S158：先補既有頁面的 golden 事件）
**背景**：**這是什麼**：很多設定頁有分頁（元件 TPageControl，例如 Tray Assignment 的一般測試／重測兩頁）。commit `c9d3c932`（主檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditList.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp`）讓一般設定頁讀寫機制（C 路）的分頁元件可以帶「目前在第幾頁」（activePageIndex）：開頁時 C++ 告訴網頁、存檔時網頁送回來。其他種類的值型別不對時，整份存檔會拒絕（回 400 錯誤，一個鍵都不寫）。
**選項**：A 只丟分頁那一筆，伺服器保留原分頁，其他改動照存，理由記進回覆的待辦（todo）／B 跟其他種類一樣整份拒。
**St01建議**：A。例：頁面找不到作用中的頁籤而送了 null——A 照樣存使用者改的值，todo 寫「伺服器保留第 1 頁」；B 整頁存不了。
**目前狀態**：已照建議先做，可推翻：commit `c9d3c932`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditList.cpp:332-347`〔不收的理由〕、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditList.cpp:364-371`〔套值〕）。
**20260929 結果**：已做（commit `c9d3c932`），網頁才有的情況。

### R103. 頁面停在藏起來的分頁（TabVisible=false）時照收（C 路共用層／S158：先補既有頁面的 golden 事件）
**背景**：**這是什麼**：分頁可以被程式設成「頁籤看不見」（TabVisible=false），內容照樣可以被切過去顯示。golden 會用程式切到藏起來的分頁（例：golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cOffSet.cpp:192` 把 OffSet 頁的 tsIndexOffset 頁籤藏起來、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cOffSet.cpp:3383` 又用程式切過去）。
**選項**：A 收（例：OffSet 頁 golden 用程式切到頁籤看不見的 Index Offset 分頁後，網頁存檔送回這一頁 → 照收）／B 拒（例：同一個情況，這一筆分頁值被丟掉、照 R102 記一筆待辦，伺服器留在原分頁）。
**St01建議**：A（照 golden）。
**目前狀態**：已照建議先做，可推翻：commit `c9d3c932`（主檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditList.cpp`）。
**20260929 結果**：照 BCB（golden 自己也會切到看不見的分頁），已做（commit `c9d3c932`）。

### R104. 存檔時先套分頁、再重播頁面沒送的 golden 事件（C 路共用層／S158：先補既有頁面的 golden 事件）
**背景**：**這是什麼**：一般設定頁存檔時，C++ 會先替頁面「重播」頁面沒送的 golden 點擊程式（BeforeApply，例 R94、R95），再存檔。Tray Assignment 的重算畫面程式 ShowCompnet 會看目前在哪個分頁（golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp:1033-1034`）。
**選項**：A 先套分頁（事件看得到使用者的分頁；事件本身也切頁時以事件為準，例如 Cleaning 頁「Mode」選項的點擊程式 rgCleanKitTypeClick 會切到 Kit／Tray 分頁，golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\AutoClean\uCleaning.cpp:2214-2261`；例：操作員在 Tray Assignment 切到重測分頁、改了 Fix Tray Mode 就存檔 → C++ 先切到重測分頁再重播 rgFixTrayModeClick，ShowCompnet 照重測分頁算）／B 最後才套分頁（頁面的分頁蓋過事件的切頁，但 ShowCompnet 會看到舊分頁；例：同一個情況重播時還在開頁時的分頁，ShowCompnet 用錯分頁算，最後才切過去）。
**St01建議**：A。
**目前狀態**：已照建議先做，可推翻：commit `c9d3c932`（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:119-150`，在存檔函式 PageSave〔`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:85`〕裡）。
**20260929 結果**：已做（程式內部順序，commit `c9d3c932`），不用 Steven 決定。

### R105. 產生器替每個分頁元件產生「頁數表」（C 路共用層／S158：先補既有頁面的 golden 事件）
**背景**：**這是什麼**：R102 的分頁值要檢查「有沒有超出頁數」。目前頁數用「父子表裡有登記的分頁數」當下限；產生器只收有用到的元件替身，所以 HSys 頁的 pcSetting、Temperature 頁的 pgcTempOffset、TesterIF 頁的 pgcRS232、GroundMan、AOAOffset 的最後一頁會被當成不合法、記一筆假的 todo。產生器（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\gen_editlist.py`；讀 DFM 父子關係的 dfm_parents 在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\gen_editlist.py:190`）已依 DFM 順序走過一次，只要每個分頁元件補一行 ELSetPageOrder（登記頁序的函式，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditList.cpp:221`）。
**選項**：A 補（範圍精確，假 todo 消失；例：操作員在 HandlerSys 頁停在最後一個分頁按存檔 → 分頁值照收、回覆乾淨）／B 維持下限（例：同一個情況值照存，但回覆多一筆假的 todo「超出範圍」，分頁留在原來那頁）。
**St01建議**：A。
**目前狀態**：還沒改（要改共用產生器，排在下一批）；Steven 沒反對就做。　⛔ 20260927 22:xx 更新：細化成 **Q51-1**（`D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\page-control-tabs.md`）。
**20260929 結果**：由 Q51 的裁決涵蓋（Steven 20260928）。

### R106. 開頁時的分頁照 golden（C 路共用層／S158：先補既有頁面的 golden 事件）
**背景**：**這是什麼**：golden 每頁開起來停在哪個分頁，是畫面檔的 ActivePage 或程式裡的 `->ActivePage=tsX` 決定的。產生器不收 DFM 的 ActivePage，也把 golden 的 `->ActivePage=tsX` 當成純畫面行丟掉（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\gen_editlist.py:307`），所以伺服器與網頁開頁都停在第 0 頁，跟 golden 不同（例：ArmSpeed、YieldMonitoring、TesterIF、UserDefForm、ACTForm、BarCode；Tray Assignment 照 DFM 是第 1 頁 tsReTestGroup）。
**選項**：A 做：開機套 DFM 的 ActivePage，並把 `->ActivePage=tsX` 翻成頁序（需要 R105）；先做 golden 會讀分頁的頁（TrayForm、StartCondition）；例：golden 某頁開起來停在第 2 個分頁 → 網頁開頁也停在第 2 頁／B 維持伺服器與網頁都從第 0 頁開（例：同一頁網頁開在第 0 頁，操作員要自己點過去；TrayForm、StartCondition 的 golden 程式會看目前分頁，停錯頁可能影響它們的結果）。
**St01建議**：A。
**目前狀態**：還沒改；Steven 沒反對就在 R105 之後做。　⛔ 20260927 22:xx 更新：細化成 **Q51-2～Q51-4**（`D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\page-control-tabs.md`）。
**20260929 結果**：由 Q51 的裁決涵蓋（Steven 20260928）。

### R107. HotPlate 關窗尾段跳的 golden 訊息，網頁上看不到（Q41 盤點第一節第 3 項第二批）
**背景**：**這是什麼**：BCB 版關 HotPlate 視窗後會重讀 Auto Clean 資料（golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:28424` 呼叫 LoadAutoCleanData，本體 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\AutoClean\uCleaning.cpp:65`），遇到不支援的組合會跳訊息（例「The site Y-pitch can not use arm 2 for auto clean!!」，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\AutoClean\uCleaning.cpp:457`、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\AutoClean\uCleaning.cpp:901`）。網頁的 HotPlate 頁走較早那套讀寫機制（A 形狀）的存檔路徑，沒有一般設定頁（C 路）那種「一次存檔把 golden 訊息收集起來回給頁面」的機制（filerw session），所以這個訊息會被吞掉；設定本身照樣重讀。
**選項**：A 先接受（交件註明）／B 尾段自己開一次訊息收集，把訊息放進 HotPlate 存檔回覆（只改 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp`，HotPlate 尾段在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp:787`）。
**St01建議**：B，但不急（這個條件很少見）。例：操作員在 HotPlate 頁把 Y Pitch 改成不支援 arm 2 清潔的值、按存檔——A：畫面只顯示存檔成功；B：畫面多一行 BCB 版的那句警告。
**目前狀態**：A（現況），Steven 沒反對就排 B。
**20260929 結果**：照 BCB（操作員要看得到那則訊息）：把訊息放進 HotPlate 存檔的回覆（選項 B），排進事件批（Steven 20260928「任何畫面的事件都是我們做」）。

### R108. 開設定頁記「Enter …」的時機：網頁的開頁其實發生在開站時（R101 的交件：網頁開設定頁照 golden 記「Enter …」）
**背景**：**這是什麼**：R101（已結案，`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md:805`；Steven 20260927 17:xx 的裁決 S165「要記」：網頁開設定頁時照 golden 在事件紀錄記一筆「Enter …」，例 Enter Speed，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md:433`）做在一般設定頁開頁向 C++ 要畫面值（editlist.get）的地方（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:57`；對照表在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp:866`）。但 `D:\HT9045\web\background.html:906-907` 開站時就把非延遲載入（非 lazy）的視窗 iframe 先載入並隱藏，網頁共用引擎一接上就發 editlist.get（`D:\HT9045\web\page\ht9045_wire_engine.js:2108`）；操作員之後按開窗鈕只是把 iframe 顯示出來、送視窗總表（`ui.windows.put`），**不會再發 editlist.get**。所以現在是網頁伺服器（wb_serve）起來後第一次開站時，閘有過的頁各記一筆「Enter」，之後操作員開窗不記。同一個前提也影響開頁等級閘（R89～R93：閘也是在開站與重讀時才查）。
**選項**：A 維持現況（開站時各記一筆）／B 改成看視窗總表從「沒開／關著」變成「開著」時記——時機和 golden 一樣準，但要改 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebWindowRegistry.cpp` 或 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp`（視窗總表是 Jimmy 的檔）／C 兩個時機都記（例：下面的情況 08:00 開站記一筆、10:00 開窗再記一筆）。
**St01建議**：B，並請 Jimmy 看視窗總表那一段要不要由 St01 加鉤子（S122 的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.cpp` 已經用同一份總表判斷開關）。例：操作員 10:00 開 Speed 頁——golden 在 10:00 記「Enter Speed」；A：08:00 開站時就記了、10:00 不記；B：10:00 記。
**目前狀態**：A（現況，commit `342779cc`，主檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp`）；等 Steven 決定。　⛔ 20260927 22:xx 更新：選項 B 說要改 Jimmy 的視窗總表——`5b73d905` 之後 St01 已有自己的「表單開／關掛勾」（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebTeachLeave.cpp`），B 可以只改 St01 的檔；完整評估與新的選項在 **Q49**（`D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\window-open-edge.md`）。
**20260929 結果**：由 Q51 的頁面狀態表接手（Q49 已由 Q51 涵蓋）：改成真的開窗那一刻記，照 BCB，排進頁面狀態讀取批。

### R109. 開頁記的「Enter …」現在寫到哪裡（R101 的交件：網頁開設定頁照 golden 記「Enter …」）
**背景**：**這是什麼**：golden 的「記一筆事件」有兩支：NewRecordProcess（帶事件代碼）與 RecordProcess。移植樹的 NewRecordProcess 還是空殼（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\acatchtray_shims.cpp:152`），RecordProcess 只印到主控台（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\canary_support.cpp:117`），所以這些「Enter」現在哪裡都沒存，只有 wb_serve 主控台看得到一行。golden 會寫事件紀錄（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cMyDB.cpp:1545` → MyDBIProcessNew `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cMyDB.cpp:724`：sqlite 資料庫的 Process 表〔`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cMyDB.cpp:740-742`〕、EventLogTxt 文字檔〔`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cMyDB.cpp:745-778`〕）。
**選項**：A 維持，等 St02 把 golden 的資料庫／事件紀錄模組 cMyDB 接回來（St02 分段工作的第 4 段 P4 之後）自動就寫進去（例：操作員開 Speed 頁 → 現在只有主控台一行，St02 接好後自動進 `D:\HT9045_Log\EventLogTxt`）／B 先寫進 C++ 網頁橋接層的事件留痕（JsonBridge 的 ring，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\EventLog.cpp`），瀏覽器用 log.tail 指令看得到（代價：所有「連整包機台程式（god-stack）又連到 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp`」的 ctest 都要多連 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\EventLog.cpp`；例：同一個操作馬上在瀏覽器的 log 畫面看得到，但不進 golden 的事件紀錄檔）／C 由 St01 先接 cMyDB 的寫入（會寫量產機共用的 `D:\HT9045\MDB\Handler.db3` 與 `D:\HT9045_Log\EventLogTxt`，是大決定；例：同一個操作馬上寫進這兩處，跟 BCB 版一樣）。
**St01建議**：A。
**目前狀態**：A（現況）。
**20260929 結果**：等 St02 把事件紀錄模組接回來（St02 P4）就自動寫進去，不用 Steven 決定。

### R110. Configuration 存檔後自動重讀會再記一筆「Enter Configuration」（R101 的交件：網頁開設定頁照 golden 記「Enter …」）
**背景**：golden 的 Configuration 頁（`D:\HT9045\web\page\Config.Configuration.html`）存檔就是關窗（FormClose），所以網頁存完、共用引擎自動重讀時會再記一筆（記的地方 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp:493`）。
**選項**：A 照現況（golden 存完就關窗，要再改得再按一次開窗，所以記一筆說得過去）／B 不記（改成「開過就算」，一行的改動；例：下面的情況網頁只有開站那一筆）。
**St01建議**：A。例：操作員改 Configuration 存檔一次——BCB 版：開窗一筆、存完關窗；網頁：開站一筆＋存完重讀一筆。
**目前狀態**：A（現況，commit `342779cc`，主檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp`）。
**20260929 結果**：由 Q51 的頁面狀態表接手：只在真的開窗時記，存檔後自動重讀不再多記（照 BCB），跟 R108 一起做。

### R111. Q34：Index 加熱用 EJ1N／DTME08 的機型 Index 下拉停用時，Index 的廠牌跟著「其他位置」（Q34 交件）
**背景**：**這是什麼**：HandlerSys 頁（`D:\HT9045\web\page\HW.HandlerSys.html`）的 Heater 分頁可以設每個溫控通道用哪個廠牌的溫控器。照 Q34 方案 D（已結案；Steven 20260927 18:1x 的裁決 S166「開工，其他照建議」，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md:439`），選「全機相同」時分別選「Index 位置溫控器」與「其他位置溫控器」的廠牌。Index 加熱用 Omron EJ1N 或 DTME08 模組的機型（`D:\HT9045\system\Gerneral.ini` 的 USE_16_HEATER），Index 那個下拉停用、顯示控制器名稱（方案 D 第 ① 點，`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md:318`），但沒說停用時 Index 的值怎麼辦。Index 組裡的 Head1～4 在 EJ1N 版本仍走溫控 COM 埠（golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\bthermo.cpp:1331-1367` 只跳過 Index 32 區）。
**選項**：A 跟著其他位置，讀檔與存檔都是／B 保留停用前的值。
**St01建議**：A。例：Steven01 這台把「其他位置」改成 Panasonic KT4H——A：Head1～4 一起變 KT4H，存成 `HeaterInsIndexOpt=1`；B：Head1～4 留在 E5DC，操作員在「全機相同」畫面改不到。
**目前狀態**：已照建議先做，可推翻：commit `bc970c38`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.cpp:776` 的 W906_HeaterInsIndexLocked；讀檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.cpp:827`、存檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.cpp:590`）。
**20260929 結果**：照 Steven 的 Q34 裁決（S154 方案 D），已做（commit `bc970c38`）。

### R112. Q34：「各溫控器不同」模式存檔時 HEATER_CTRL_TYPE 要數哪些通道（方案 D 第 6 點 D-6a 的細節／Q34 交件）
**背景**：**這是什麼**：`D:\HT9045\system\Gerneral.ini` [TempCtrl] 的 HEATER_CTRL_TYPE 是「全機一個廠牌」的舊設定：V899／V906 BCB6 機台與移植樹今天的溫控只看這一個。方案 D 的 D-6a 規定「各溫控器不同」模式存檔時寫「用最多通道的那個廠牌」，沒寫要數哪些通道。
**選項**：A 先數「不同」模式畫面上列出的通道（方案 D 第 3 點 D-3a：真的在溫控 COM 埠上輪詢的那些），列出的全是 No Heater 才改數 71 個／B 數全部 71 個。
**St01建議**：A。例：「不同」模式把列出的 12 個通道都改成 DTK4848、沒列出的 59 個仍是 E5DC——A 寫 `HEATER_CTRL_TYPE=4`；B 寫 2，V899／V906 BCB6 機台就會用 E5DC 跟 DTK4848 通訊。
**目前狀態**：已照建議先做，可推翻：commit `bc970c38`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.cpp:666-687` HmCtrlTypeFor）。
**20260929 結果**：照 Steven 的 Q34 裁決（S154 方案 D），已做（commit `bc970c38`）。

### R114. Q34：「各溫控器不同」模式沒列出的通道存檔寫什麼（Q34 交件）
**背景**：**這是什麼**：方案 D 第 3 點 D-3a 規定「各溫控器不同」模式畫面只列真的會輪詢的通道（golden 有下拉的 23 個＋依 USE_16_HEATER 實際走溫控 COM 埠的 Index 區），沒寫其餘通道（例如沒裝熱風槍時的 HeatGun1、Base1～6、Door1～2）存什麼。
**選項**：A 寫記憶體裡的實際廠牌（檔案值或開頁算出來的）／B 跟著 Index／其他。
**St01建議**：A（V912 混搭檔裡看不到的下拉值不會被改掉）。例：V912 檔裡 `HeaterInsOpt_HeatGun1=4`、沒裝熱風槍——A 存回 4；B 會改成「其他位置」的廠牌。
**目前狀態**：已照建議先做，可推翻：commit `bc970c38`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\HSys.cpp:600-603`）。
**20260929 結果**：照 Steven 的 Q34 裁決（S154 方案 D），已做（commit `bc970c38`）。

### R115. Configuration 頁「[D46] Index destroy delay」按上下箭頭、還沒存檔時，機台用哪個值（D46 上下鍵〔udD46〕，偏離 golden）
**背景**：Configuration 頁 Index 區的「[D46] Index destroy delay」數字框旁有上下箭頭（5～15），按一下加減 1。BCB 版按一下就改機台記憶體裡的等待時間（golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6098`），關視窗時才問要不要寫進 `D:\HT9045\config\config.ini`。網頁關頁不會問，伺服器也不知道頁面關了。
**選項**：
- **A**（現在做法）：按箭頭只改畫面，按存檔答 YES 才改記憶體和檔案。例：原本 5，按上三下變 8，沒存就關頁 → 機台照用 5；存檔答 YES → 機台用 8，檔案也是 8。
- **B**（照 golden）：按一下就改記憶體。例：按到 8，沒存就關頁 → 機台用 8 跑，但檔案還是 5；要等下次開 Configuration 頁或重開程式才回到 5，期間沒人看得出兩者不同。
**St01建議**：A。網頁沒有「關窗一定要回答存不存」這一步；A 跟 Steven 之前定的「設定頁按下不寫檔、存檔才寫」（RULINGS_20260926 S136，Q14＝B）同方向。另外：存檔答 NO 時，移植樹本來就立刻重讀檔，所以 A、B 答 NO 都回到檔案值；golden 答 NO 會留著按過的值。
**目前狀態**：已照 A 做，可推翻：commit `64ade3b7`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp` 檔尾）。改成 B 只要刪 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp:579` 與 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp:581` 兩行。
**20260929 結果**：照 Steven 規則 S136（設定頁按下不改、存檔才改），已做（commit `64ade3b7`）。

### R116. Configuration 頁沒送點擊事件就直接存檔時，伺服器替頁面補點一次，但看不出點的先後（存檔補點〔IC_EvBeforeApply〕）
**背景**：Configuration 頁有幾個勾選框一勾就會讓底下一組欄位出現或消失（例：「[E30] In arm use different scale」）。頁面還沒接上「點一下就讓 C++ 跑 golden 點擊程式」的事件（或送失敗）時，存檔改由伺服器照最後的勾選補點一次，所以「勾了 E30 再改框裡的值」能存進去（St02 回報的「存檔、重開才生效」就是這個）。但伺服器只看得到最後的結果、看不到先後：
- 例 1：先改 E30 底下框裡的值、再取消 E30。BCB 版會照存框裡的值（只是藏起來）；網頁伺服器只看到最後 E30 是關的，那些值會被丟掉、沿用檔案值。
- 例 2：勾「[D36] RTC auto model verify」時，BCB 版會自動取消「[D33] RTC initial start need verify」、勾上「[D35] RTC need check site number」。補點會照做，但接著頁面送的 D33／D35 值會蓋回去；BCB 版也允許勾完 D36 再手動勾回 D33，伺服器分不出是哪一種。
**選項**：
- **A**（現在做法）：以頁面最後狀態為準。例：頁面最後是 D36 勾、D33 勾 → 存成兩個都勾。
- **B**：頁面沒送點擊事件、卻改了這幾格，就整次拒存，請操作員重開頁再點（同 Temp_Set 頁的做法，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Temperature.cpp` 的存檔流程）。例：同上情形 → 不存，畫面說明要重點一次。
**St01建議**：A。St02 的頁面接上點擊事件後，伺服器每一下都跟著頁面，這兩種情形就不會發生。
**目前狀態**：已照 A 做，可推翻：commit `64ade3b7`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp:708`）。
**20260929 結果**：已做（commit `64ade3b7`）；St02 的頁面接上點擊事件後就跟 BCB 一樣，這兩種情形不會再發生。

### R117. D46 上箭頭連按太快，第二下會被防連點擋掉（網頁防連點〔WebCmdGuard〕× D46 上下鍵）
**背景**：操作員在 D46 上箭頭 0.4 秒內連按 3 下。BCB 版會加 3；網頁第二、三下跟前一下內容一樣，會被伺服器的防連點擋掉（回「busy」），只加 1。依據：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_FormEvent.cpp:14-15`；可以連按的白名單在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp`（例：手動移動馬達的 jog 鈕）。
**選項**：
- **A**：維持，頁面收到 busy 就當沒按。例：連按 3 下只加 1，畫面照伺服器回的數字顯示。
- **B**：把「D46 上下鍵」列進防連點白名單（跟 jog 一樣算連續操作），改 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp`。例：連按 3 下加 3；手抖多按一下也會多加 1，但值被夾在 5～15。
- **C**：頁面自己排隊，等上一下回來再送下一下（St02 頁面端做，伺服器不改）。例：連按 3 下，數字會在約一秒內依序加到 +3。
**St01建議**：C：每一下都送到、都照 golden 跑一次，伺服器的防連點規則不用開例外。
**目前狀態**：伺服器是 A（現況）；C 要 St02 的頁面做，等 Steven 決定。
**20260929 結果**：照 BCB（連按 3 下要加 3）：選項 C，St02 的頁面排隊送，伺服器的防連點規則不開例外。

### R118. 開 Configuration 頁時，BCB 版會自動觸發的勾選連動，網頁沒有（D36 → D33／D35；同 R100 那一類）
**背景**：`D:\HT9045\config\config.ini` 裡 D36 與 D33 都是勾（在 BCB 版勾完 D36 再手動勾回 D33 就會存成這樣）。BCB 版開機載入時，程式設 D36 勾會觸發連動，把畫面上的 D33 取消、D35 勾上；之後存檔就寫成 D33 沒勾。網頁開頁照檔案顯示 D33 勾，存檔也寫勾。依據：golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6551` 與 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Public\HTEditList.cpp:1390`（程式設勾選會觸發點擊程式）；載入順序是 D33、D35 在 D36 之前（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.gen.inc:4336`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.gen.inc:4340`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.gen.inc:4346`）。
**選項**：
- **A**：維持（照檔案）。例：上面情形網頁顯示並保留 D33 勾。
- **B**：照 BCB 版，開頁時模擬那次觸發。例：同情形開頁就看到 D33 被取消、D35 被勾上。
**St01建議**：A，跟 R100（Tray Assignment 頁開頁設單選群組時不觸發點擊程式）一起決定。其他幾格（E30、E39、D21、D47、F05）開頁的結果兩邊相同，不受影響。
**目前狀態**：A（現況，commit `64ade3b7`）；等 Steven 決定。
**20260929 結果**：照 BCB：開頁時照 golden 觸發 D36 → D33／D35 的連動，跟 R100 一起排進事件批（Steven 20260928「任何畫面的事件都是我們做」）。

### R121. Tray Assignment 頁開頁停在伺服器記的分頁、存檔時送「目前在哪一頁」——所以在 FT 頁存檔會照 BCB 把 FT 設定複製到 RT（網頁送出點交件）
**背景**：Tray Assignment 頁（網頁 `D:\HT9045\web\page\Setup.TrayAssignment.html`）有 Normal Test（FT）與 Retest（RT）兩個分頁。BCB 存檔鈕會看操作員現在停在哪一頁：Configuration 開了「FT 存檔時把 RT 設定跟 FT 一樣」（bFTBin2RTBin）時，在 FT 頁存檔會把 FT 的 bin 放置複製到 RT（golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp:1353-1372`）。網頁以前不送分頁，C++ 一直當成停在 Retest 頁，所以網頁存檔永遠不複製。今天的網頁送出點（commit `1b213d11`，`D:\HT9045\web\page\ht9045_trayassign_ev.js`）同時做了三件：開頁照 C++ 記的分頁點頁籤、按鈕事件帶目前分頁、存檔送目前分頁——這正是 Q51-3、Q51-4 建議的 A。
**選項**：
- **A**（現在做法）：照 BCB。例：開了 bFTBin2RTBin 的機台，操作員在 Normal Test 頁把 Auto1 改成 Bin 2 後存檔 → RT 的 Auto1 也變成 Bin 2（跟 BCB 一樣；對網頁使用者是行為改變）。
- **B**：存檔先不送分頁（只保留開頁點頁籤）。例：同樣操作 → RT 不變（跟以前的網頁一樣、跟 BCB 不同）。只要刪 `D:\HT9045\web\page\ht9045_trayassign_ev.js` 裡 R.editlistSave 的 pgRunMode 那一段。
**St01建議**：A（照 golden；Q51-4 的建議也是 A）。上機時請在 Tray Assignment 各存一次 FT 頁、RT 頁確認，並告訴操作員這個差別。
**目前狀態**：已照 A 做，可推翻：commit `1b213d11`。
**20260929 結果**：照 BCB，已做（commit `1b213d11`）；上機時 Normal Test、Retest 兩頁各存一次確認。

### R123. Contact 頁點 Kit 直徑、Contact 模式等選項時，機台記憶體什麼時候改（B3 交件）
**背景**：golden 點了就改機台記憶體（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp:15509` 等，還馬上送 EP 電壓），關窗時再改回；網頁沒有關窗事件。現在的做法：畫面照 golden 變，機台記憶體等存檔才改（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\DeviceForm_File.cpp` 檔尾）。
**選項**：
- **A**（現在）：存檔才改。例：點 60mm、沒存就關頁 → 機台照舊用 40mm 的壓力生產。
- **B**：照 golden 點了就改。例：點 60mm、沒存就關頁 → 機台用 60mm 的壓力生產，直到下次開這一頁。
**St01建議**：A。
**目前狀態**：已照 A 做，可推翻：commit `6ba451d5`。
**20260929 結果**：已做（commit `6ba451d5`），結果跟 BCB 相同：BCB 點了就改記憶體、關窗再改回，而設定頁只能停機時開。

### R124. Cleaning 頁 AI Clean 的「Reset Interval」會把配方裡的 Interval Contact 寫成 -1（B3 交件）
**背景**：golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\AutoClean\uCleaning.cpp:2835-2838` 按下去把 `HandlerCondition.Data` 的 iAutoClean_IntervalContact 寫成 -1，看起來是 golden 的漏洞；移植樹照翻（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_Cleaning.cpp`）。
**選項**：
- **A** 照 golden。例：按下去 iAutoClean_IntervalContact 變 -1。
- **B** 906 單邊改成回到標準值、不寫 -1。例：按下去維持原本的 50。
**St01建議**：A（照翻），另請 Jimmy 確認 BCB 版是不是漏洞；是的話兩邊一起改。
**目前狀態**：已照 A 做，可推翻：commit `6ba451d5`。
**20260929 結果**：照 BCB，已做（commit `6ba451d5`）；是不是 BCB 的漏洞請 Jimmy 看，是的話兩邊一起改。

### R125. 運轉中能不能按 Yield 頁的「Reset Interval」（B3 交件）
**背景**：共用規則是運轉中網頁的點擊事件一律拒絕（RULINGS_20260927 第 7 條）；golden 運轉中開著 Yield 視窗也按得到（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uYieldMonitoring.cpp:6058`）。
**選項**：
- **A** 維持拒絕。例：生產中按了，回「運轉中」，要先停機。
- **B** 這一顆放行。例：生產中按了，自適應良率基準當場重來。
**St01建議**：A。
**目前狀態**：A（現況，commit `6ba451d5`）。
**20260929 結果**：照 Steven 規則（RULINGS_20260927 第 7 條：運轉中網頁點擊一律拒絕），已做（commit `6ba451d5`）。

### R126. KYEC 刷條碼機台上 Cleaning 頁的「Reset」（B3 交件）
**背景**：golden 按 Reset 會先跳刷條碼框（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\AutoClean\uCleaning.cpp:2118`），刷完才歸零；網頁沒有刷條碼框，等於操作員什麼都沒刷，按了沒反應。
**選項**：
- **A** 維持。例：KYEC 機台按 Reset 沒反應，要回 BCB 畫面做。
- **B** 做網頁刷條碼框（照 Steven「任何畫面的事件都是我們做」由 St01／St02 做）。例：按 Reset 跳框，刷完才歸零。
**St01建議**：B（排進事件批，客戶專屬、優先度低）。
**目前狀態**：A（現況）；等 Steven 決定。
**20260929 結果**：照 BCB：做網頁的刷條碼框，排進事件批（Steven 20260928「任何畫面的事件都是我們做」）；客戶專屬、低優先。

### R127. Offset 頁按微調鈕時只存目前這一組（B3 交件）
**背景**：網頁按 Offset 微調鈕時，C++ 先換到頁面正在看的部位，再加減 0.1、馬上存（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cOffSet.cpp:2892-2943`）。golden 換部位時會先把上一組存起來；網頁版是「切組記住、按 Save 才一起存」。程式在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Offset_File.cpp` 檔尾。
**選項**：
- **A** 維持。例：Loader 的 X 改了沒按 Save，切到 Auto1 按「上」→ 只存 Auto1，Loader 的 X 還留在頁面上等 Save。
- **B** 按微調前，先把頁面上所有改過的組一起存。例：同樣情況，Loader 的 X 也一起寫進檔案。
**St01建議**：A（改動最小，跟網頁的 Save 行為一致）。
**目前狀態**：已照 A 做，可推翻：commit `b08ae6ad`。
**20260929 結果**：照 BCB：換部位時先存上一組（選項 B），排進事件批（Steven 20260928「任何畫面的事件都是我們做」）。

### R128. Offset 頁還沒選部位就按微調（B3 交件）
**背景**：golden 開窗後還沒選部位就按微調，存不到任何部位，但存檔尾端照跑（Pause 計數、重讀參數）。網頁版直接不送、提示先選部位。
**選項**：
- **A** 維持。例：開頁直接按「上」→ 提示「請先點一個部位」，C++ 什麼都沒做。
- **B** 照 golden 送，讓尾端也跑。例：同樣按「上」→ 沒存任何部位，但重讀一次參數。
**St01建議**：A。
**目前狀態**：已照 A 做，可推翻：commit `b08ae6ad`。
**20260929 結果**：照 BCB：沒選部位也照送、讓尾段照跑（選項 B），排進事件批（Steven 20260928「任何畫面的事件都是我們做」）。

### R129. Offset 頁 Index socket offset 的「改過」旗標要不要接（B3 交件）
**背景**：golden 改 Index socket offset 時記 iIndexChange=2，全樹沒有人讀它；而且那一格在 golden 藏起來的分頁上，C++ 端點不到。
**選項**：
- **A** 不接。例：改 Index offset 時什麼都不記，機台行為跟現在一樣。
- **B** 等切分頁事件做完再接一行。例：改完記 2，但沒有人讀。
**St01建議**：A。
**目前狀態**：A（現況，commit `b08ae6ad`）。
**20260929 結果**：照 BCB：切分頁事件做完時接上（沒有人讀這個旗標，機台行為不變）。

### R130. Temp_Set 頁 Arm1／Arm2 offset 網頁上改了存不進去（B3 交件）
**背景**：golden 的 Arm1／Arm2 offset 兩格要切到 Arm1／Arm2 分頁才顯示，那支切頁程式還沒移植（事件批 X-5），所以 C++ 一律當成看不見，網頁改了存檔會被略過。等級 17 的限制已照 golden 補上（PE 模式也擋），程式在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Temperature.cpp` 檔尾。
**選項**：
- **A** 等切頁程式一起解。例：現在改 Arm1 offset 存檔 → 回覆說略過，檔案值不變。
- **B** 先讓 C++ 把這兩格當成看得見（偏離 golden）。例：等級夠就直接存得進去。
**St01建議**：A。
**目前狀態**：A（現況，commit `b08ae6ad`）。
**20260929 結果**：照 BCB：等切分頁程式移植好一起解（事件批 X-5）。

### R131. Tray Form 頁「Bin Box Reset」沒有確認框（B3 交件）
**背景**：golden 按 Bin Box Reset 就直接把已裝數歸零、清掉 Fix3 盤的格子資料，不問（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayForm.cpp:729-734`）；網頁照 golden 不加確認。
**選項**：
- **A** 照 golden 不問。例：誤觸就清掉。
- **B** 網頁加一次確認。例：先跳「確定清除 Fix3 盤資料？」。
**St01建議**：A（照 golden），但 Fix3 盤上有料時誤觸的代價請 Steven 判斷。
**目前狀態**：A（C++ 已做 `b08ae6ad`，網頁送出點補在 B2 的頁面檔）。
**20260929 結果**：照 BCB，不加確認框，已做（commit `b08ae6ad`）。

### R132. Configuration 頁「[D47] Clear」按下就把機台記憶體的 Socket 已測次數歸零（B4 交件）
**背景**：按 Configuration 頁 [D47] 的 Clear，機台記憶體裡的「Socket 已測次數」馬上變 0（照 golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6062-6066`）。這跟 R115 的 D46 上下鍵不一樣（D46 是畫面先變、存檔才改記憶體）。程式在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp` 檔尾說明 (6)。
**選項**：
- **A**（現在，照 golden）：按下就歸零。例：按 Clear 之後沒存檔就關網頁 → 機台從 0 開始數。
- **B**（跟 D46 一樣）：畫面顯示 0，存檔才歸零。例：同上 → 機台照原本的數繼續數。
**St01建議**：A（Clear 是動作不是設定值；golden 答「否」時記憶體也已經歸零）。
**目前狀態**：已照 A 做，可推翻：commit `875d3499`。
**20260929 結果**：照 BCB，已做（commit `875d3499`）。

### R133. Configuration 頁 [A09]「機台裡有 IC 就不能改」的檢查，存檔時再查一次（B4 交件）
**背景**：golden 只在點 [A09] 的當下看機台裡有沒有 IC（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6453-6480`）；網頁版存檔時會再看一次（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp` 的 IC_EvA09Recheck）。
**選項**：
- **A**（現在）：存檔再查。例：點 A09 時機台是空的，存檔前有人放一顆 IC 到 Index → 存檔時 A09 被改回、跳 MES1645，其他設定照存。
- **B**（照 golden）：只在點的當下查。例：同上 → A09 照存。
**St01建議**：A（互鎖放在 C++、不信任網頁，跟 R95 Fix 盤「存檔一定重查」同一個做法）。
**目前狀態**：已照 A 做，可推翻：commit `875d3499`。
**20260929 結果**：已做，取安全側（commit `875d3499`）：互鎖放在 C++，存檔時再查一次（同 R95）。

### R134. Configuration 頁 EP 滑桿一次拖動只送一次（B4 交件）
**背景**：瀏覽器的滑桿放開時才送；golden 在拖動中每動一格就跑一次處理器（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6652-6684`，每一次都記一筆 EP 修改紀錄）。頁面在 `D:\HT9045\web\page\ht9045_config_st01_ev.js`。
**選項**：
- **A**（現在）：放開送一次。例：把 [D60] 56mm 從 80 拖到 90 → 事件紀錄記一筆 EP 修改。
- **B**：拖動中每一格都送。例：同上 → 記 10 筆，每一格都要一次來回（400 ms 防連點會擋掉一部分）。
**St01建議**：A。
**目前狀態**：已照 A 做，可推翻：commit `875d3499`。
**20260929 結果**：已做（commit `875d3499`）：瀏覽器的滑桿放開才送，記一筆修改紀錄。

### R135. Configuration 頁要廠商密碼的格子，網頁當場改回（B4 交件，Q45 #1～#3／#5＝C 的做法）
**背景**：網頁上點 [C12] 那一組、[A27] 或 [N07-5]，如果點完的結果存檔時會被拒（Q45：這幾格網頁版維持改不了），頁面當場把那一格改回，並說「這一項要廠商密碼，網頁版不提供，請到 BCB 版機台改」；C++ 存檔照舊整次拒存（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp:188` IC_PasswordGuard）。頁面在 `D:\HT9045\web\page\ht9045_config_st01_ev.js`。
**選項**：
- **A**（現在）：當場改回。例：勾 [A27] → 馬上彈回沒勾，狀態列寫原因。
- **B**：只標示、不改回。例：勾 [A27] → 畫面保持勾著；按存檔 → 整次拒存，要自己改回再存。
**St01建議**：A（Q45 #1～#3＝C「維持改不了、網頁說明原因」最直接的做法）。
**目前狀態**：已照 A 做，可推翻：commit `875d3499`。
**20260929 結果**：照 Steven 的 Q45 裁決（這幾格網頁版改不了），已做（commit `875d3499`）。

### R136. FTP 下載「成功沒」看的是密碼本的結果，不是工作檔的（golden 的怪處；R120 後續交件）
**背景**：舊版按 FTP 下載畫面的下載鈕，先下載工作檔（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\KYECFTP\FTPClient.cpp:998-1001`），再下載密碼本（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\KYECFTP\FTPClient.cpp:1003`，本體 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\KYECFTP\FTPClient.cpp:4468-4556`），最後才判斷成功沒（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\KYECFTP\FTPClient.cpp:1007-1015`）。密碼本那一支會把「下載失敗」的記號重寫一次，所以「從 FTP 下載密碼本」兩個開關都開的機台，決定 START 解不解開的是密碼本的結果；只開一個或都沒開的機台照工作檔的結果。移植樹照舊版寫在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebStart.cpp:3951`（W906_FTPClient_DownloadResult，commit `a99d8e6c`），目前沒有人呼叫（網頁 FTP 下載畫面是 St02 的 W62）。
**選項**：
- **A** 照舊版。例：伺服器上找不到這個工作檔、密碼本下載成功 → 當成下載成功，START 解開，機台還是用舊的工作檔。
- **B** 移植樹改成工作檔下載成功才算（跟舊版不同）。例：同上 → 當成失敗，按 START 仍跳「Need Download FTP Setup File!!」，操作員要重新下載。
**St01建議**：A（照 golden，RULINGS_20260927 第 1 條），請 Jimmy 確認舊版是不是漏洞；是的話兩邊一起改。
**目前狀態**：已照 A 寫好（還沒有呼叫者），可推翻：commit `a99d8e6c`。另外同一顆補了 golden 開機時的 FTP 下載資料快照（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainBoot.cpp` 檔尾，golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:11495-11513`）；接上呼叫點之後，出貨版開 FTP＋連線測試機時 START 就不會再一律被擋。
**20260929 結果**：照 BCB，已寫好（commit `a99d8e6c`）；是不是 BCB 的漏洞請 Jimmy 看，是的話兩邊一起改。

### R137. 主畫面控制鈕（RESET／ONE CYCLE／TRAY FEED／ALARM RESET）與 Site 格「按了」要記多久（主畫面批次交件）
**背景**：照 Steven 20260928「c++部分只需要做到事件觸發…實際動作可以先列表, 後面通知Jimmy進行接上」（RULINGS_20260926 S169），C++ 現在只記「按了」，實際動作要等 Jimmy 接（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClick.cpp` 檔尾、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClickTail.h` 的 W906_Main_TakeCtlButtonEvent／W906_Main_TakeSiteClickEvent）。
**選項**：
- **A**（現在）：3 秒沒人處理就丟掉。例：按 RESET 後 Jimmy 的程式沒在 3 秒內接手，這一下作廢，要再按一次。
- **B**：一直留著。例：今天按過的 RESET，Jimmy 接上那天第一次開機就會突然執行一次。
**St01建議**：A。
**目前狀態**：已照 A 做，可推翻：commit `cd496b52`。
**20260929 結果**：已做（commit `cd496b52`），取安全側：網頁才有的情況；3 秒沒人接就作廢，不會哪天突然執行。

### R138. Motor View「Check Encoder」取消勾選也不會關（照 golden；主畫面批次交件）
**背景**：golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:29548-29560` 勾選時把每一軸設成每次都檢查編碼器，取消勾選卻不會清回來。
**選項**：
- **A** 照 golden（現在）。例：取消勾選後，軸仍然每次檢查編碼器。
- **B** 取消勾選時清回來。例：取消勾選後回到平常的檢查方式。
**St01建議**：A，請 Jimmy 確認原意。
**目前狀態**：已照 A 做，可推翻：commit `cd496b52`。
**20260929 結果**：照 BCB，已做（commit `cd496b52`）；原意請 Jimmy 看。

### R139. Heater View「HOTPLATE 2」後門在 V912 沒效果（主畫面批次交件）
**背景**：golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:31875-31878` 點了會設一個旗標（bStartProductOnLine），但 V912 全樹沒有地方讀它。
**選項**：
- **A** 照 golden 接著（現在）。例：點了主控台記一行，機台不變。
- **B** 網頁不接。例：點了沒反應。
**St01建議**：A。
**目前狀態**：已照 A 做，可推翻：commit `cd496b52`。
**20260929 結果**：照 BCB，已做（commit `cd496b52`）。

### R140. 主畫面右邊功能清單收合的狀態放哪（主畫面批次交件）
**背景**：golden 按「⬅」收合右邊的功能清單（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:8689`），一台機台一個畫面。網頁可能有好幾台電腦同時開。
**選項**：
- **A** 伺服器一份（現在，照 golden）。例：A 電腦收合後，B 電腦第一次點看起來沒變化。
- **B** 每個瀏覽器各記各的。例：兩台畫面各自收合、互不影響。
**St01建議**：A。
**目前狀態**：已照 A 做，可推翻：commit `cd496b52`。
**20260929 結果**：照 BCB（一台一份），已做（commit `cd496b52`）。

### R141. 告警長說明選「日本語」時顯示什麼（主畫面批次交件，W42-c 延伸）
**背景**：告警視窗現在跟著主畫面選的語言（Steven W42-c「全部跟 main 畫面統一選」；`D:\HT9045\web\page\ht9045_alarm_motionview.js:277`），但機台沒有日文的告警說明檔（`D:\HT9045\Error\` 底下只有 English／Chinese／Korea）。
**選項**：
- **A** 英文（現在）。例：主畫面選日本語，告警說明是英文。
- **B** 中文。例：同上情況顯示中文。
**St01建議**：A。
**目前狀態**：已照 A 做，可推翻：commit `cd496b52`。
**20260929 結果**：已做（commit `cd496b52`）：機台沒有日文說明檔，顯示英文。

### R142. Home Monitor（回原點監看畫面）那 21 處「畫面開著沒」要不要改問頁面狀態表（頁面狀態表第 3～5 步交件）
**背景**：golden 只有回原點程式會開 Home Monitor（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uhome.cpp:2497`），所以 golden 的「Home Monitor 開著」＝「正在回原點」。網頁上操作員也可以自己從選單打開它看。其他畫面的判斷都已改問頁面狀態表（commit `87a625f0`），只有這 21 處先維持讀程式自己的狀態。
**選項**：
- **A** 維持讀程式狀態（現在）。例：操作員自己從選單開 Home Monitor ⇒ 機台什麼都不做。
- **B** 改問頁面狀態表（程式開或網頁開都算）。例：HOME ALL 剛做完、網頁還沒關視窗的半秒內，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp:32715` 會中止回原點、伺服 OFF，剛回好的原點失效。
- **C** 改問頁面狀態表，但 Home Monitor 那一列只算程式狀態。例：結果跟 A 一樣，只是寫法統一（要改 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp` 的規則與測試）。
**St01建議**：A。
**目前狀態**：A（現況，commit `87a625f0`）。
**20260929 結果**：照 BCB 的意思（Home Monitor 開著＝正在回原點），維持讀程式狀態（commit `87a625f0`）。

### R143. 「網頁全關」告警的號碼 MES1690 跟既有的 WAR1690 撞號（頁面狀態表交件）
**背景**：機台資料 `D:\HT9045\Error\AlarmCodeList.txt:949` 已有 WAR1690（No SYN-TEK Master Card!!!），golden 的事件資料庫用數字當告警編號（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cMyDB.cpp:630`），兩個告警會混在一起。新告警目前寫 MES1690（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp:46` kPageNoScreenAlarmCode），說明檔還沒建。
**選項**：
- **A** 改用 MES16441（查過沒人用：AlarmCodeList、`D:\HT9045\Error\` 說明檔、golden、移植樹、網頁 JSON 都沒有）。例：事件紀錄裡這兩個告警分得開。
- **B** 維持 MES1690。例：事件資料庫用 1690 當編號，兩個告警混在一起。
- **C** 由 Jimmy 另選號碼。例：跟機台資料的編號規則一致。
**St01建議**：A，並請 Jimmy 建說明檔（文字已給 Jimmy）。
**目前狀態**：程式還是 1690；等 Steven／Jimmy 決定（改號只動 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp:46` 與 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4389` 各一處）。
**20260929 結果**：等 Jimmy 選號（預設 MES16441，查過沒人用），選好就改 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp:46` 與 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4389` 兩處；說明檔文字已給 Jimmy。不用 Steven 決定。
**Jimmy 20260929 09:3x 決定**（main 的 `docs/handoff/TO_STEVEN.md` §4；`docs/RULINGS_20260929.md` 第 2 節）：**MES16441**，說明文字照 St01 給的；St01 改程式那兩處（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp:46`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4389`），ST01-E 做。

### R145. 「畫面開著沒」接上之後直接生效的三個變化（Q-P3＝直接生效；頁面狀態表交件）
**背景**：Steven Q-P3 選直接生效（RULINGS_20260926 S168）。接上後，以前永遠當成「沒開」的判斷開始照 BCB 生效，其中三個操作員會感覺到：(1) Set Up 視窗開著按 START，會跳過「需要重新回原點」的檢查（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ckernel.cpp:877`）；(2) Tower Light 視窗開著時，主迴圈不管蜂鳴器（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ckernel.cpp:1690`）；(3) Set Up／Contact 開著時，遠端 DLL 指令回 -6「Settings Window is Opened」（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Command.cpp:10261`）。另外 Contact 開著時不報低良率、不進省電、不做 auto decay。
**選項**：
- **A** 照 golden 維持（現在，已是 Q-P3＝直接生效）。例：跟 BCB 版一樣。
- **B** 第 (1) 條例外、照舊檢查。例：Set Up 開著也要先確認有回原點。
**St01建議**：A。
**目前狀態**：A（現況，commit `87a625f0`）。
**20260929 結果**：照 Steven Q-P3（直接生效＝跟 BCB 一樣），已做（commit `87a625f0`）。

### R146. 遠端指令「設定視窗開著就拒絕」的清單要不要照 golden 補齊（頁面狀態表交件）
**背景**：golden 查 35 個表單有沒有開著，移植樹縮成 3 個（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Command.cpp:10239-10256` 是 `#if 0`）；現在頁面狀態表每一頁都答得出來。
**選項**：
- **A** 照 golden 打開那段（排進下一批「畫面開著沒」的讀取）。例：Teach 開著時，遠端 SetSiteMap 回 -6。
- **B** 維持 3 個。例：Teach 開著，遠端照樣改 Site Map。
**St01建議**：A。
**目前狀態**：B（現況）；等 Steven 決定。
**20260929 結果**：照 BCB：打開 golden 那段、補齊 35 個表單的檢查，排進下一批頁面狀態讀取。

### 20260929 11:1x Steven 裁決：R122、R144（從 decisions-pending.md 搬來）

### R122. 瀏覽器全部關掉之後，機台運轉中要等幾秒才停下？
**建議：10 秒**（已經這樣做，commit `6273f82f`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebPageTable.cpp`）。
**例**：按 F5 重新整理，畫面 2 秒就回來 ⇒ 不停；夜班把瀏覽器關掉去吃飯 ⇒ 10 秒後機台照 golden PAUSE 停下，回來打開畫面看到告警說明原因。改 5 秒：比較慢的電腦 F5 要 6 秒才連上，機台會被停下；改 15 秒：沒人看的時候機台多跑 15 秒才停。
**為什麼要問**：BCB 沒有「瀏覽器關掉」這種情況，是安全與誤停之間的取捨。
**Steven 的裁決**（20260929 11:17，在 ST01-M 對話裡）：原話「R122.  10秒 ok」⇒ 維持 10 秒（commit `6273f82f` 已是 10 秒），不用改程式。**結案。**

### R144. 沒有畫面時 SECS 主機遠端按 START 被擋，要不要請 EastSun 讓主機收到「現在不能做」？
**建議：要**；在 EastSun 改好之前維持現況（commit `87a625f0`）。
**例**：現況——主機收到「已接受」，機台不動，主機等不到開始事件；改了以後——主機馬上收到「現在不能做」（HCACK=2）。要改的是 EastSun 的檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\SECSGEM\uHGemHT9045.cpp`（START 入口那一行）。
**為什麼要問**：BCB 沒有「沒有畫面不准 START」這條，要不要動 EastSun 的檔由 Steven 決定。
**Steven 的裁決**（20260929 11:17，在 ST01-M 對話裡）：原話「R144.   SECS 的SF code 有ACK 可以回覆, 挑一個正確的ACK進行回覆」⇒ **要回**：沒有畫面時主機遠端 START 被擋，照 SEMI E5 S2F42 的 HCACK 挑正確的碼回給主機——標準碼「2＝Cannot perform now（現在不能做）」；golden 的 HCACK 梯子（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\SECSGEM\uHGemHT9045.cpp` 約 :881-:935，另有 6～17 的機台狀態碼）若已有更貼切的碼，照 golden 的用法挑。**誰做**：ST01-E（頁面狀態表是 St01 的）；改的是 EastSun 的檔，只動 START 入口那一段，改前在交接檔 §3 知會 Jimmy／EastSun（看區段不看檔名）。

### 20260929 17:32 Steven 裁決：Q53～Q57（從 decisions-pending.md 搬來）

### Q53. 原生 MotorView 的「目標位置」在出貨版要顯示什麼？（原生表單第 3 題）
**建議：照 golden，一直顯示 `MOT[].TargetPosition`**（引擎的目標值，不是卡片讀回的值）。
**例**：出貨版機台上，原生 MotorView 的「目標位置」欄現在是「—」（照網頁 Motor Test 的規則：出貨組態不拿 `MOT[]` 位置當卡片值）；改照 golden 後會顯示引擎目前要去的位置，跟 BCB 主畫面一樣。
**為什麼要問**：網頁那條規則是移植時定的，跟 golden 不同；原生畫面是 C++ 主導，照 BCB 的話應該顯示。說明在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ui\native\README.md`（ht9045 MR !7 分支上）。
**Steven 的裁決**（20260929 17:32，在 ST01-M 對話裡）：原話「Q53 照建議值」⇒ 原生 MotorView（以及原生 MotorTest 同一欄）出貨版照 golden 一直顯示 `MOT[].TargetPosition`；ST01-E3 在 ht9045 MR !7 的分支上改。

### Q54. 要不要加「執行期逐頁切換」原生／網頁（原生表單第 4 題）
**建議：先不做**，維持編譯開關 `W906_NATIVE_FORMS`（預設關）；等 Jimmy 審完 MR !7、機台上看過原生視窗，再決定六頁網頁版要不要下架。
**例**：做了的話，`D:\HT9045\system\NativeForms.ini` 每頁一行 0／1，改一行重啟就回網頁版，不用重編；不做的話，要切回網頁版就用關掉開關的版本。
**為什麼要問**：BCB 沒有這種東西，是新功能；Steven 說過「如果可行，會把html的這四頁下架」。
**Steven 的裁決**（20260929 17:32，在 ST01-M 對話裡）：原話「Q54 不用. 只有馬達移動相關的六頁有需要」⇒ 不做執行期逐頁切換（不做 `NativeForms.ini`），維持編譯開關；原生畫面只限跟馬達移動相關的這六頁（IoSetView、MotorTest、MotorView、teach、home、ShuttleMove），其他頁一律網頁。

### Q55. wb_serve 資料端要不要提速到接近 20 ms（讓原生畫面的資料也跟上）
**建議：要**，分兩段：St01（ST01-E3）改 (a) 主迴圈最多等 50 ms（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4566`，加 timeBeginPeriod）與 (c) 馬達覆蓋資料 500 ms 拍子；(b) 1203 每 200 ms 讀一次、驅動裡卡約 140 ms，已寄信請 EastSun 評估（09-29 15:4x）。`wb_serve.cpp` 改之前在交接檔先跟 Jimmy 講好。
**例**：現在原生畫面每 20 ms 重畫一次，但機台上的燈號／位置最快約 200 ms 才變一次；(a)(c) 改完馬達位置約 20～50 ms 更新，IO 仍受 (b) 限制，要等 EastSun。
**為什麼要問**：Steven 要求「IO與馬達的顯示必須是很有效率的, 得快到20ms一次」，但 (a) 改的是整個主迴圈的節拍，影響所有功能，要 Steven 點頭。
**Steven 的裁決**（20260929 17:32，在 ST01-M 對話裡）：原話「Q55 html端可以不用那麼快, 後續有問題才調整」⇒ 現在不改 wb_serve 資料端的節拍（主迴圈 50 ms、馬達覆蓋 500 ms、1203 Poll 200 ms 都維持）；之後有問題再調。寄給 EastSun 的評估請求維持「評估就好、不急」。

### Q56. St01 分支什麼時候請 Jimmy 合進 main
**建議：`10cac033` 兩組態全量 gate 綠了就請 Jimmy 合**（開發版已綠：21 支＝基準；出貨版 09-29 17:17 起跑）。它包含今天的 R144、B10b、D-013、頁面狀態讀取、一條連線（瀏覽器端）、MES16441、D-012 前半。之後的 B10c、D-012 後半下一輪再合。
**例**：合了之後 Jimmy 在機台上驗「一個瀏覽器一條連線」就直接用 main；不合的話 Jimmy 要另外拉 `v906/steven-cbridge-review6`。
**為什麼要問**：合進 main 要 Steven 決定（既有規則）。
**Steven 的裁決**（20260929 17:32，在 ST01-M 對話裡）：原話「Q56 可以」⇒ `10cac033` 兩組態全量 gate 綠了，ST01-M 在交接檔 §2 請 Jimmy 合進 main。

### Q57. Q50 的 20 支網頁探針要不要再對真機檔跑一次
**建議：要，今晚跑**（照 Q50 的做法：先整夾備份、每輪還原、看到清單外的檔就停）。
**例**：上次（09-28 晚）20 輪還原都乾淨，但探針 11 過、20 沒過，原因是探針登出後 130 ms 就重登被防連點擋下，已修（`c4d3463d`）；重跑才知道修好沒有。
**為什麼要問**：會開 wb_serve 讀寫真機檔，St01 規定要 Steven 同意（Q50 那次是「Q50 你可以在晚上的時候做」）。
**要花什麼**：跟上次同一套守門（`wbrun_guard.py` 先備份 → 開模擬版 wb_serve＋探針 → 還原並比對），這次只跑上次失敗的 20 支，加上通過的 11 支當對照，實際排程約 4 小時（`--dry-plan` 估 243 分鐘；上次 21:32～00:43），20260929 19:47 起由 ST01-M 跑；探針修好（`c4d3463d`）之後，預期登入失敗那批會全部消失。
**Steven 的裁決**（20260929 17:32，在 ST01-M 對話裡）：原話「Q57 要」⇒ 今晚由 ST01-M 用 `wbrun_guard.py` 重跑：上次失敗的 20 支＋通過的 11 支當對照，先備份、每輪還原比對，看到清單外的檔就停。

### 20260930 13:4x Steven 裁決：Q58、Q59（從 decisions-pending.md 搬來）

### Q58. Q57 沒過的 10 支網頁探針修好了，要不要再對真機檔跑一次確認
**建議：要，今天晚上跑（20260930 晚上，跟 Q50／Q57 一樣下班後）**。照 Q50 的做法：先整夾備份，每輪還原，看到預期清單外的檔就停。
**發生什麼事**：Q57（09-29 晚上）每一輪還原後檔案都乾淨，但探針 21 支過、10 支沒過。ST01-E 逐一查過（20260930 02:0x），**10 支都是探針自己的問題，不是產品的錯**：
- 7 支是期望值過時。後來加的事件表變大了（B3／B4／B10b／TS）；SYSTEM_TEST_IF 從 788 欄變成 796 欄（筆電 `551c7398`，跟 golden 一樣）；St02 `06f8ef0a` 把登入寫 MES2140／MES2144 到 EventLogTxt 接成真的；golden 每拍都累加開機時間，所以「timeData／lastdata 不變」這種檢查永遠不會成立。
- 3 支是探針邏輯或時間的 bug：countersel 第二次存檔只隔 182 ms，被 400 ms 防連點擋下；A3 的判斷寫反了。另外有 3 個被「第一個失敗」欄位蓋住的，也一起修了。
- 修正在 `329296b4`（St01 的 7 支探針，以及 skill `D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\wbserve-sandbox-run.md` §6.z）。只做了語法檢查和離線自測（33 項 0 失敗），**還沒對伺服器跑過**，要重跑才知道修好沒有。
**為什麼要問**：這次會開 wb_serve 讀寫真機檔，St01 規定要 Steven 同意。
**要花什麼**：守門程式 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\wbrun_guard.py` 跟上次同一套。ST01-E `b1aed08d` 已經把 13 個 golden 開機寫檔加進預期清單，這次不用再手動帶參數。可以只跑上次沒過的 10 支，外加幾支通過的當對照，約 1.5～2 小時；也可以全跑，約 4 小時。由 ST01-M 跑，要避開白天的建置和測試。
**Steven 的裁決**（20260930 13:4x，在 ST01-M 對話裡）：原話「no need」⇒ **不重跑**。Q57 的探針修正 `329296b4` 只做了離線自測（33 項 0 失敗），沒有對伺服器跑過；以後要是有別的原因跑 Q50，就順便確認。

### Q59. 以後 St01 分支每次兩組態全套測試綠了，是不是就直接請 Jimmy 合進 main（不用每次問）
**建議：是**。條件是 ST01-M 跑兩組態全套測試，失敗只有基準那幾項（或已查明不是程式問題，例如測試 exe 被刪），真實檔比對 0 差異，就在交接檔 §2 請 Jimmy 合。例外：碰到加熱、馬達、IO 安全的改動，或 Steven 說過要先看的，仍然先問。
**例**：今天（09-30）St01 分支 `2829694e` 已經通過（SIM 21 項＝基準；SHIP 6 項基準，外加 WB_Crypto 的 exe 在測試前被刪、不是程式問題；真實檔 0 差異）。內容是 B5 重新登入（Q45 甲）、網頁一條連線的兩個小修正、合 Jimmy 的 main。`7e60e445` 再加上 BarCode Exit 和 D-012 A8，03:50 開始跑。
**為什麼要問**：合進 main 要 Steven 決定（既有規則）。Q56 只講到 `10cac033` 那一輪，之後的新功能沒有涵蓋。今晚 `1e5316eb` 那次是照 Q56 那一輪的計畫加上 Jimmy 自己說「M1／M2 修好就合」。
**要花什麼**：不用另外花時間。選「是」的話，Jimmy 早上就能合 `2829694e` 或 `7e60e445`（看哪個先綠）；選「否」的話，每次都要等 Steven 回。
**Steven 的裁決**（20260930 13:4x，在 ST01-M 對話裡）：原話「ok」⇒ **以後 St01 分支兩組態全套 gate 綠了**（失敗只有基準，或是已查明不是程式問題；真實檔 0 差異），**ST01-M 就直接在 FROM_STEVEN §2 請 Jimmy 合**，不用每次問 Steven。例外照建議：碰到加熱、馬達、IO 安全的改動，或是 Steven 說過要先看的，還是先問（例如 B8 會動機台的那幾項、AG-1 E84、form.event 運轉中的例外清單）。

### 20260930 16:0x Steven 裁決：Q60（從 decisions-pending.md 搬來）

### Q60. B8 AG-1（E84）照 golden 讀 AGV.ini，要不要讓它合進 main
**建議：可以合**（照 golden；只有設定成用 E84 的機台才會動作，Steven01 這台不會變）。
**機台上會發生什麼**：機台的 `D:\HT9045\system\Gerneral.ini` [System] AGVModal＝1，**而且** `D:\HT9045\config\AGV.ini` 的「E84 Enable」＝1 時，主流程每一拍都會跑 E84 上下料交握：讀 E84 感測器，切換 LREQ／UREQ／READY／HOAVBL／ES／VA／VS0／VS1 這幾個輸出（POWER 一律不碰），不動任何軸。以前移植版沒有讀 AGV.ini，E84 從來沒跑過。AGVModal＝0 的機台什麼都不變。
**做了什麼**（ST01-E，照 golden V912）：
- (A) `b45d4482`：Setup.AGV 頁照 golden TfAGV 讀／存 AGV.ini（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\AGV.cpp`:1184-1309）；開頁、存檔會照 golden 設定「E84 啟用」。
- (B) `a9f89386`：開機時照 golden 讀一次（main.cpp:9378，在 DoReadLastData 裡，換 recipe 也會讀），另外加上兩顆 Initial Load／Unload 鈕：各自關掉那一側的 6 個 E84 輸出，並重設那一側的工作，跟 golden 一樣。
- 測試：B8_Ag1_AgvIni 28／28、B8_Ag1_Initial 28／28（用假 IO）；相關測試都過，wb_serve 能編。兩組態全套 gate 排在 `ed4716f2` 之後。
**跟 golden 不同的一點**：golden 在運轉中也能按 Initial 鈕，這是操作員救卡住的 AMR 的方法；移植版現在運轉中會擋掉所有頁面事件。ST01-E 接著做一個很窄的「運轉中例外清單」，另外一個 commit，也會先給 Steven 看。
**另外在筆電的檔（Jimmy 的）**：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Automation\AGV_E84.cpp` 的 case 5000 恢復段（:584-594、:947-956）少了 V912 的 Actionflag 清除，也沒有 ATK 5000／5010 的分流；:926-937 的 YES／NO 還是固定回 2。這些已經告訴 Jimmy，由他改。
**為什麼要問**：會動到 IO（E84 輸出），照 Q59 屬於例外，要 Steven 先看。
**Steven 的裁決**（20260930 16:0x，在 ST01-M 對話裡）：原話「Q60 ok」⇒ AG-1 (A) `b45d4482`＋(B) `a9f89386` 可以合進 main。照 Q59，兩組態全套 gate 綠了，就在 FROM_STEVEN §2 請 Jimmy 合。頁面要讀／存，還要等筆電的兩行 §1 認領（wire_engine.js:1053、W906_AGVINI_PATH）落地；開機讀取（B）不受這個影響。

### 20260930 17:1x Steven 裁決：Q61（從 decisions-pending.md 搬來）

### Q61. 運轉中，網頁上 golden 本來按得到的 14 顆鈕要不要放行（頁面事件的「運轉中例外清單」）
**建議：可以放行**（照 BCB；都不動軸）。
**操作員會看到什麼**：今天網頁版只要機台在運轉，所有設定頁的按鈕一律被擋（頁面事件統一拒收）；改完後，只有下面這 14 顆會像 golden 一樣在運轉中按得到，其他照舊擋。
**為什麼（golden）**：Offset 的 Setup Teach 排序鈕 12 顆（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cOffSet.cpp`:3211-3221）和 AGV 的 Initial Load／Unload 2 顆（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\AGV.cpp`:1316-1330）都在非模態 Show() 的表單上，運轉中開著照樣按得到；處理器本身不查 SystemStart／SoftStart（Offset 只查 A30、離線、需要 Setup Teach；A30 時 golden 的 START 會自己打開 Offset）。
**移植樹現在**：頁面事件運轉中一律拒收（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_FormEvent.cpp`:77-81），理由是「golden 運轉中打不開設定畫面」，但漏了非模態表單，所以這 14 顆鈕在網頁上只能停機時按。
**改法**（ST01-E，`46425cbc`，安全層的改動，單獨一個 commit）：運轉中照舊一律拒收，只有例外表上的（表單, 元件, 事件）放行，而且頁面表要說那個視窗真的開著；每一列附 golden 出處；放行後 golden 自己的條件與開頁、點得到的檢查照套。其他所有頁面事件照舊拒收；運轉中不能存設定（editlist.save，RULINGS_20260927 第 2 條第 7 題 A）與 form.save 都不變。
**風險**：AGV Initial 在交握中按下，會關掉 6 顆 E84 交握輸出、交握回到第 1 步（golden 也是這樣，是操作員救卡住的 AMR 用的；HT9050 沒有 E84）。Offset 排序鈕只改運轉中下一顆 IC 放哪一盤（限 A30＋離線）。都不動軸。
**測試**：B8_Os5_SortButtons 34／34、B8_Ag1_Initial 37／37，另外約 20 組相關測試都過；wb_serve 能編。兩組態全套 gate 由 ST01-M 跑。
**為什麼要問**：這是安全層（運轉中擋按鈕）的改動，照 Q59 屬於例外，要 Steven 先看；綠了也不合，等 Steven 回。
**Steven 的裁決**（20260930 17:1x，在 ST01-M 對話裡）：原話「golden應該是有卡權限吧? 依照golden」⇒ **照 golden，包含 golden 的權限檢查**。這 14 顆鈕跟它們所在的 Offset／AGV 視窗，在 golden 裡要什麼權限等級才能開、才能按，網頁版都要一模一樣，而且要在伺服器端（form.event）擋，不能只在頁面上藏起來。ST01-E 先查 golden 的權限出處（開窗、ChangeLevelAttr／LevelSet 的 Enabled、處理器內的檢查），補齊後做成單獨一個 commit，並加低權限、允許權限兩種 ctest。**權限補好、而且兩組態 gate 綠了，才在 §2 請 Jimmy 合 `46425cbc`＋權限修正。**注意：移植版的 ChangeLevelAttr 目前是空函式（M-11 查到的）。

## St02（Steven02，測試通訊）

### Steven 已裁決的項目（W 系列）

以下每一條都用「這是什麼功能」＋「當時的問題」＋「選項」＋「St02建議」＋「Steven 的裁決」的順序整理，方便理解；原本的內部代號（如 P6-Q1、D-1、ELA-8 等）保留在標題括號內，只作對照，不作為說明本身。所有檔案路徑一律使用完整路徑；找不到明確的樹別時會標註（樹待確認）。

### W1. 測試機（Tester）廠牌判斷要用哪個設定值做主（P6-Q1 Auto Retest 的 Tester 廠牌；todo H-009）
**這是什麼功能**：機台在跟 Auto Retest 測試機通訊時，要知道對方是哪種廠牌（0＝Flex、1＝93K），才能用對的通訊格式。Steven 自己先前的裁決 3A 說過「以配方為準」；但 golden（原廠 BCB 程式）的做法是機台自己「學」：看測試機送來的指令是哪一種格式（送 `LOTSTATUS` 代表 Flex、送 `SRQMASK` 代表 93K），學到之後寫回配方檔；V906（移植中的新版程式）目前完全沒有這個「自動學」的動作，開機固定當作 93K。
**當時的問題**：V906 要不要照抄 golden 這套「自動學廠牌、寫回配方」的行為？還是照 Steven 3A「以配方為準」的字面意思，嚴格只看配方檔設定，兩邊都不自動學（風險是：如果現場把測試機從 Flex 換成 93K 或反過來，配方沒跟著改，機台會判斷錯誤而出錯）？
**選項**：(a) 照抄 GPIB 引擎跟 Handler 目前的行為，保留 golden 的自動學機制；(b) 嚴格只看配方設定，兩邊都不自動學（換測試機廠牌時配方沒改就會出錯）
**St02建議**：(a)，最接近 golden 原本的行為
**Steven 的裁決**：「應該要根據工作檔設定, 但是目前FLEX已經幾乎不使用了, 所以預設值使用 93K」（Steven 20260927 10:2x，經 github-02 轉述）
⇒ 意思：以配方（工作檔）裡的設定為準；配方沒設定時，預設值用 93K（不是 Flex）。golden 那套自動學廠牌的機制要不要保留，這一句沒有講；Steven 另外補充「保留並寫回」（見下面現況）。
**現況**：先前暫定做法是 (a)，commit `7aa13ca3`，改了 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Rs232SetupCodes.h、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Gpib\GpibCommands.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\HandlerSettings.h、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_TesterIF.cpp 等 13 個檔。之後 Steven 補充（20260927 10:1x，在 St02 那邊，St02 轉達）：golden「從 Tester 指令（LOTSTATUS／SRQMASK）學廠牌、再寫回配方」這件事**保留並寫回**；配方裡沒有這個鍵時，預設值＝1（93K，這是刻意偏離 golden 原本預設 0 的地方）。St02 目前進行中的做法：照 golden 的 `TfSCKART::AccessFile`（在 906_0625 對照樹 D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\Automation\SCK_ART.cpp:190-209，這是 St02 機台上才有的對照樹，St01 這台機器上沒有）接上筆電（Jimmy）那份 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fSCKART.cpp（記錄在 D:\HT9045_handoff\FROM_STEVEN.md §1，20260927 10:25）；D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp 裡三處相關程式碼（列在 W1b）交給筆電（Jimmy）決定。

### W3. RS232 通訊設定存檔前要不要擋掉 Windows 不支援的組合（P6-Q3(1)；todo H-009）
**這是什麼功能**：機台的 RS232 通訊設定（資料位元數、停止位元數等）可以在畫面上調整並存檔。
**當時的問題**：有幾種組合（5 個資料位元配 2 個停止位元、或 6／7／8 個資料位元配 1.5 個停止位元）是 Windows 系統本身不接受的，Windows 的通訊設定函式（`SetCommState`）呼叫下去會失敗；而且這個失敗目前被程式忽略掉了，結果就是 RS232 port 會默默維持舊的格式，畫面看起來像存成功了，其實沒生效。要不要在存檔那一刻就把這種不合法組合擋下來並提示操作員？
**選項**：A 擋並提示；B 不擋
**St02建議**：A
**Steven 的裁決**：「A」（Steven 20260927 10:2x，經 github-02 轉述）
⇒ 意思：存檔時要擋掉 Windows 不接受的組合，並提示操作員。
**現況**：維持 commit `7aa13ca3`，改的檔案同 W1：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Rs232SetupCodes.h、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Gpib\GpibCommands.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\HandlerSettings.h、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_TesterIF.cpp 等 13 個檔。

### W4. 新版配方檔會不會被拿到舊版（BCB）機台上使用（P6-Q3(2)；todo H-009）
**這是什麼功能**：配方檔記錄機台測試時要用的各項設定，新版（V906）程式加了一些舊版沒有的新選項。
**當時的問題**：如果現場把 V906 存的配方檔拿到用舊版 BCB 程式的機台上開，新選項會不會被舊程式認得？如果不會，舊機台是不是會自動退回成 8 位元／2 個停止位元／不驗證（None）這種預設值？需不需要現在就處理這個相容性問題？
**選項**：A 不會拿去舊機台用；B 會拿去用，需要另外處理相容性
**St02建議**：—（純問題，St02 沒有給建議）
**Steven 的裁決**：「未來舊機台會升級成新版本的程式, 到時候要做一個 old to new 的轉換器 列為待辦」（Steven 20260927 10:2x，經 github-02 轉述）
⇒ 意思：現在不用處理相容性問題（因為舊機台以後都會升級成新版程式）；改成新增一項待辦事項。
**現況**：新增待辦 **G-031 舊版→新版配方轉換器**。

### W5. 機台回覆自己身分編號（Handler ID）時要看哪一份設定（P6-Q4 確認 4A；todo H-009）
**這是什麼功能**：測試機用 `HANDLERID?` 這個指令問機台「你是誰」時，機台要回一組身分編號格式。
**當時的問題**：機台自己的設定檔 D:\HT9045\config\config.ini 裡的 I25 這個項目，跟 GPIB 通訊程式自己另一份設定檔 D:\GPIB9045\system\general.ini 的內容，在某些機台上不一致，這時候 `HANDLERID?` 該回什麼會跟著不一致而改變。例如：HiSilicon 客戶的非 SCC 機型，I25 被強制設成 1，這台機器就會開始回「Machine ID」格式。要照哪一份設定為準？
**選項**：A 照 Handler（機台本身）的 config 設定為準；B 先去處理那些設定不一致的機台
**St02建議**：A
**Steven 的裁決**：「A」（Steven 20260927 10:2x，經 github-02 轉述）
⇒ 意思：照機台本身（Handler）的 config 設定為準。
**現況**：維持 commit `7aa13ca3`，改的檔案同 W1：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Rs232SetupCodes.h、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Gpib\GpibCommands.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\HandlerSettings.h、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_TesterIF.cpp 等 13 個檔。

### W6. RS232 新選項要不要同步改到網頁畫面（P6-Q5；todo H-009）
**這是什麼功能**：機台的通訊設定頁面（網頁版 D:\HT9045\web\page\Setup.TesterIF.html 第 56 行附近的三組選項按鈕）讓操作員切換 GPIB／RS232 等設定。
**當時的問題**：上一次（P2e）Steven 說過只做 C++ 端、不動網頁。而且 golden（原廠程式）在畫面切到 GPIB 模式時，只顯示 GPIB 那一頁，RS232 那組選項在 Setup.TesterIF 頁面上根本看不到；如果要改 GPIB 配方裡的 RS232 相關值，現在只能先暫時切成 RS232 模式存檔、再切回 GPIB，很不方便。這次 RS232 新增的選項，要不要順便一起改網頁？
**選項**：A St02 一起改（新增的選項＋讓 GPIB 模式下也能改到 RS232 那組值）；B 只做 C++ 端，網頁之後再說（結果是新選項在網頁上暫時選不到）
**St02建議**：A
**Steven 的裁決**：「A」（Steven 20260927 10:2x，經 github-02 轉述）
⇒ 意思：St02 一起改網頁 D:\HT9045\web\page\Setup.TesterIF.html（加上新選項，並讓 GPIB 模式下也能修改 RS232 那組設定）。
**現況**：Setup.TesterIF 這個頁面是 St01 負責維護的「C 路」頁面，St02 動手改之前，要先在 D:\HT9045_handoff\FROM_STEVEN.md §1 貼出要改的行號，給 St01 核對。

### W7. 資料庫模組要建立多少筆記錄檔物件（cMyDB P1；todo D-004）
**這是什麼功能**：golden（原廠程式，`TfMain::TfMain`，D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:1552-1720）在開機時會建立 27 個記錄檔物件（`TMyStringList`），分別對應事件記錄、2D 條碼對應、網路記錄、TTL 通訊記錄、加熱器記錄、故障告警記錄、時間資料、生產紀錄、ASM、批次資訊、露點、扭力…等各種 log。V906（移植中的新版程式）原本一個都沒建。
**當時的問題**：V906 要照 golden 一次把 27 個 log 物件都建起來（之後哪個功能要用，就自己去解自己那關的驗收關卡），還是先只建資料庫模組（cMyDB）目前實際會用到的 4 個就好？
**選項**：A 照 golden 27 個全建；B 只建 cMyDB 用得到的 4 個
**St02建議**：A
**Steven 的裁決**：「A」（Steven 20260927 10:3x，經 github-02 轉述）
⇒ 意思：照 golden 27 個 log 物件全部建起來，筆電（Jimmy）先前提議要加的 HeaterLog 也一併接上。
**現況**：先前暫定做法是 B（只建 4 個），commit `80bcd1fb`，改了 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\LogObjects.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cMyDB.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_mydb_csv_eventlog.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.h 等 9 個檔；另有 commit `5368493b` 改了 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp。這些都要照新裁決（27 個全建）調整。

### W8. Qorvo 客戶的「測試機暫停」計時行為要不要改成新版（912）的做法（H-015 確認用；todo H-015）
**這是什麼功能**：`ckernel.cpp` 裡處理 Qorvo 客戶「Tester Pause」（測試機暫停）的計時邏輯。新版（912）的行為是：要超過設定的最大測試時間（MaxTestTime）才會響鈴警示，而且如果按 Reset 之後仍然處於暫停狀態，計時會重新算。
**當時的問題**：Steven 原本說「目前不改沒關係」，後來又說「待辦裡面你可以做的就先做」，St02 因此已經動手把這段改成 912 的行為（commit `1bf262c5`）。現在要確認：這個改動要保留，還是要退回 906 原本「馬上響」的行為？
**選項**：A 保留（照 912 的行為）；B 退回 906「馬上響」（改回三行就好）
**St02建議**：A
**Steven 的裁決**：「已做，A」（Steven 20260927 10:3x，經 github-02 轉述）
⇒ 意思：保留 912 的行為（commit `1bf262c5`），已經做的不用改回去。
**現況**：改動範圍：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ckernel.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Handler\HandlerGpibMsg.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\TESTERCOMM_PORT_LEDGER.md（commit `1bf262c5` 尚未編譯驗證）。

### W9. 遠端下達的溫度偏移量（offset）指令能不能寫進配方檔（D-1；todo H-009／St02）
**這是什麼功能**：測試機或上位系統可以透過 GPIB、SECS 或 FTP 指令，遠端下達一個溫度偏移量（temperature offset），微調機台的測溫。V906 裡 GPIB 的 `SETTJ` 這條指令目前是「活的」（有在運作），但少了新版（912）額外補上的那段處理（golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp:6142-6163）。
**當時的問題**：就算把 912 少的那段補上，要把這個偏移量寫進配方檔（D:\HT9045\IniData\Data\<配方>\Temperature.Data）這一步，目前被一個安全防護閘（S3）擋住（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\uTemp_Set.cpp:6306-6308、:6345-6347），所以現在遠端下達的溫度偏移量**完全不會生效，畫面上也不會有任何提示**。要不要打開這個安全閘，讓它照 golden 的行為真的寫進配方？
**選項**：A 打開 S3 安全閘，照 golden 寫進配方，並把 912 補的那段一起補上；B 維持關閉（遠端下達的溫度偏移量繼續不生效）
**St02建議**：A（照 golden 的行為；但動手前要先跟負責 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\uTemp_Set.cpp 這個檔案的筆電（Jimmy）認領協調）
**Steven 的裁決**：「A」（Steven 20260927 10:3x，經 github-02 轉述）
⇒ 意思：打開 S3 安全閘、照 golden 把偏移量寫進配方，並補上 912 新增的那段處理。
**現況**：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\uTemp_Set.cpp 是筆電（Jimmy）負責的檔案，St02 動手前要先認領、跟筆電對過行號再改。

### W10. 遠端 TCP 指令伺服器（可遠端啟動/暫停機台）要不要現在開放（D-2；todo H-009／St02）
**這是什麼功能**：一個透過 TCP（port 7016／7017）接收遠端指令的伺服器，給 Greatek／TeraPower／TeraProbe 等客戶的上位系統使用，可以遠端下達「開始」「暫停」機台（`HTSET,333`／`HTSET,334`），也可能改到機台的設定檔 D:\HT9045\config\config.ini 和配方檔。
**當時的問題**：V906 收指令的核心程式是活的，但對外開放連線的部分（listener）和回覆機制都還關著（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Command.cpp:13510／:13524、:13553-13618），所以現在外部完全連不上、收不到任何指令。另外發現 golden（原廠程式）本身有一個「接收緩衝區只有 100 bytes，超過就溢位」的既有問題（golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Command.cpp:12776／:12786），V906 目前是照抄這個問題（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Command.cpp:16391／:16401）。要不要現在就開放這個 TCP 伺服器？如果開放，這個緩衝區溢位的老毛病要不要順便修掉？
**選項**：A 現在開放，緩衝區溢位問題照 golden 保留（不修）；B 現在開放，緩衝區溢位問題修掉，並記錄成「刻意偏離 golden」；C 先繼續關著，等之後把整個遠端指令通道的安全設計做好再開
**St02建議**：C；如果現在就要開放，選項就選 B
**Steven 的裁決**：「B」（Steven 20260927 10:3x，經 github-02 轉述；與 St02 建議的 C 不同）
⇒ 意思：現在就開放 port 7016／7017；同時把 100 bytes 接收緩衝區溢位的問題修掉，並記錄成刻意偏離 golden 的地方。
**現況（Steven 20260927 下午在 St02 那邊定案，St02 用 commit `c01e6821` 記錄）**：
- R1＝要修：回覆訊息用實際長度直接送出，不再先複製進一個 500 bytes 的緩衝區（`cBuffer[500]`），這處理方式記成刻意偏離 golden。
- R3＝要處理：程式裡有三個對話框在等待迴圈中（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:537／:806／:6759）也要能照樣回應遠端指令，因為 golden 的 BCB 畫面在跳出視窗（ShowModal）期間也會回應；例如機台在報警（HTGR,801／706）跳窗的當下，遠端問狀態要能回「Down」。
- S-a＝要處理：18 個跟 S2 設定／配方寫入有關的項目，一律照 golden 經過 S3（701）安全閘保護。
- R2＝**不要直接回「NG」**。Steven 原話：「在 rs232 的 bcb 程式裡面，有做把命令接起來的動作，你要參考處理」。St02 要先唯讀研究 golden 版的 RS232 通訊程式（D:\RS232Standard）是怎麼把「分段收到、拼不完整」的命令重新拼接起來的，然後提出方案：（1）7016 這個 TCP 接收路徑要不要照這個方式做（可能取代現有「超過 99 byte 就丟」的粗暴做法）；（2）`HTSET,322`／`HTSET,323` 參數值超出範圍時該怎麼處理才不會直接回 NG。方案要先給 Steven 看過再動手寫程式。
- 還沒定案的部分：`HTSET,702` 指令的字串轉數字（`StrToInt`）失敗時的例外處理方式。
- 完整計畫文件在 St02 分支：D:\HT9045\.claude\skills\ht9045-st02-workflow\references\w10-tcp-command-server-plan.md（St01 這棵樹上沒有這份檔案）。

### W11. 每次測試開始要記錄的「測試座編號 log」程式碼放在哪個檔案（D-3 SocketIDLog；todo H-009／St02）
**這是什麼功能**：golden（原廠程式，`cStartCondition.cpp:1435-1492`，樹待確認）在每次測試開始時，會寫一筆記錄，記下是哪個測試座（socket）在測。
**當時的問題**：V906 目前完全沒有這個函式；呼叫這個功能的地方目前只做到 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\atester.cpp:942-944（這是 St02 負責的驗收關卡 T02，做到這裡就停）。另外還需要一個給自動測試（ctest）用的「測試用切換開關」`W906_SOCKETIDLOG_ROOT`（環境變數，沒設定的話走 golden 原本的路徑，量產機台行為不受影響）。這個功能的程式碼要放在哪一個檔案？
**選項**：A 放進 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fStartCondition（Jimmy 的檔；目前只有 fStartCondition.h，還沒有 .cpp 檔）；B 放進 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\StartCondition.cpp（St01 負責的檔）；C St02 自己另開一個新檔（寫成獨立函式）
**St02建議**：C（因為呼叫這個功能的地方也在 St02 負責的範圍，不必等別人的檔案）；但如果 Jimmy 希望放進 fStartCondition，就選 A
**Steven 的裁決**：「B」（Steven 20260927 10:3x，經 github-02 轉述）
⇒ 意思：放進 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\StartCondition.cpp（St01 負責的檔）。
**現況（分工由 github-70 決定，記在 D:\HT9045_handoff\FROM_STEVEN.md §4，20260927 10:45）**：本體函式 `W906_SC_SocketIDLog()` 和環境變數開關 `W906_SOCKETIDLOG_ROOT` 由 St01 撰寫；St02 要在 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\atester.cpp 定義一個函式指標 `W906_SocketIDLogBody`，並把驗收關卡 T02（:942-944）改成「有裝上函式指標才呼叫」；St01 負責在 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4111 把這個函式指標裝上去。

### W12. 報警事件 log 的三支寫檔／讀檔函式歸誰負責（D-4；todo H-009／St02）
**這是什麼功能**：三支函式（`MySaveFileByFileNameAndType`／`GetLastLine`／`MyInsertToFile`），原本被歸類在 St02 負責的 14 支函式清單裡。
**當時的問題**：進一步稽核後發現，這三支其實是**機台報警的事件記錄（event log）**在用的函式（golden 的 `note.cpp` 裡 `ShowErrorMessage`／`SaveErrEventLog` 會呼叫它們，D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\note.cpp），不是測試通訊相關的功能；呼叫這三支函式的地方分別在 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\canary_support.cpp／D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fNote.cpp／D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp（都是筆電／Jimmy 負責的檔），這也跟 W7 的「4 個或 27 個」log 物件之爭有關聯。這三支函式該不該還算在 St02 名下？
**選項**：A 留給 St02 負責；B 交給負責報警流程的人（筆電／Jimmy）
**St02建議**：B
**Steven 的裁決**：「B 交給報警流程的負責人  st02是有空的時候才幫忙處理」（Steven 20260927 11:3x，在 St01 這邊，github-02 轉述）
⇒ 意思：這三支函式歸報警流程的負責人（筆電／Jimmy）；St02 只在有空的時候幫忙。
**現況**：已在 D:\HT9045_handoff\FROM_STEVEN.md §3／§4（20260927 11:40）通知筆電（Jimmy）與 St02。

### W13. 每天凌晨 1 點自動上傳生產紀錄的功能怎麼接上（D-5 UploadProdLog；todo H-009／St02）
**這是什麼功能**：給 SPIL／Qualcomm 客戶（設定項 N17）用的功能，每天凌晨 01:00 自動把生產紀錄上傳到伺服器。上傳本體的程式在 V906 是活的（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Command.cpp:3608-3654），但負責在凌晨 1 點觸發它的計時器函式 `TFormHS::TimerAutoBackupTimer` 目前只有宣告、沒有實作內容（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fHS.h:691；golden 版本這支函式有 428 行，在 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\HS_Function.cpp:88-515，這是筆電／Jimmy 負責的檔）。
**當時的問題**：要把整支 `TimerAutoBackupTimer`（含自動備份等其他功能）都照 golden 翻譯完整，還是只做「凌晨 1 點觸發上傳」這一小段，其餘先記成「部分翻譯、還沒做完」？
**選項**：A 整支 `TimerAutoBackupTimer` 照 golden 全部翻譯；B 只做凌晨 01:00 這個小觸發點，其餘記成部分翻譯
**St02建議**：A（那支函式其實是「自動備份」的計時器，這裡只需要接上凌晨 01:00 上傳那一段，其他部分仍然缺）
**Steven 的裁決**：「跟eventlog的hub放在一起做」（Steven 20260927 11:4x，在 St01 這邊，github-02 轉述）
⇒ 意思：不整支翻譯 golden 的 `TFormHS::TimerAutoBackupTimer`（D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\HS_Function.cpp:88-515）；凌晨 01:00 的觸發改放進「事件記錄分析排程中心（ElaHub）」一起處理（跟 W21 的原則一樣：跟時間有關的排程都集中進這個 hub），上傳這件事也要套用 W21 決定的「檢查是否上傳成功／依條件分時觸發」規則。
**現況**：本體函式 `TfMain::UploadProdLog`（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Command.cpp:3608-3654）在 V906 目前是活的。記錄在 D:\HT9045_handoff\FROM_STEVEN.md §4（20260927 11:50）。

### W14. 測試逾時、操作員選「Reset 清料」時，要不要真的把配方檔的接觸高度改掉（O07；todo H-009／St02）
**這是什麼功能**：golden（原廠程式，Sam 於 20250820 加的功能 [I49]，D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\atester.cpp:3860-3871）：當操作員選擇「Reset 清料」時，程式會先把配方檔（D:\HT9045\IniData\Data\<配方>\Contact.Data）裡 [Mode] 的 Contact 模式改成 Direct，並把 [Test Arm1]／[Test Arm2] 的 Contact（接觸高度）數值加上 I49 這個補正值（讓接觸點拉高），等清料流程結束後再寫回原本的數值（移植樹對應位置在 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp:3106-3117；golden V912 同一段邏輯在 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\csystem.cpp:15792-15803）。
**當時的問題**：這個「清料前拉高、清料後還原」的原始數值**只存在記憶體裡，沒有另外存檔備份**。如果清料清到一半機台當機或突然斷電，配方檔就會停留在「已經拉高」的狀態，之後開機沒有機制把它還原回來。這個功能要不要照 golden 打開？打開的話要不要順便解決這個「當機就回不去」的風險？
**選項**：A 照 golden 原樣打開（有斷電風險）；B 照 golden 打開，但把原始數值另外存一份檔案備份，重開機時可以用這份檔案還原；C 維持關閉（清料時不拉高接觸高度）
**St02建議**：A（照 golden 做）；如果在意當機風險，就選 B
**Steven 的裁決**：「初始設計中, testif 的變數, 這個算是使用中的, testif_file才是從檔案讀出來的。生產到一半變更變數, 通常沒有存檔, 這個部分你可以看一下, 所以應該只是行為改變而不寫檔。因此應該是傾向於B」（Steven 20260927 11:5x，在 St01 這邊，github-02 轉述；此題方向已定，但細節還要 St02 確認）
⇒ 意思：清料時拉高接觸高度這件事，**只改「使用中」的那份變數（TestIF，機台記憶體裡目前在用的設定），不寫回配方檔**（D:\HT9045\IniData\Data\<配方>\Contact.Data 本身不動）。
**現況**：⚠ 這跟 St02 表格原本寫的選項 B（「照 golden，但先把原值另存一份檔」）字面不同——Steven 的意思其實比較接近「不寫檔、只改記憶體變數」。St02 要先實際確認 golden（D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\atester.cpp:3860-3871、D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\csystem.cpp:15792-15803；原文誤寫成移植樹的 :3106-3117，那其實是 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp 的行號）到底有沒有寫檔，再照「只改變數、不寫檔」這個方向提出實作做法。記錄在 D:\HT9045_handoff\FROM_STEVEN.md §4（20260927 12:05）。

### W15. 事件記錄檔分析程式讀 CSV 檔時，欄位要用什麼符號切開（ELA-1；todo E-005／St02）
**這是什麼功能**：機台的事件記錄分析程式要讀取 CSV 格式的事件記錄檔，並把每一行切成一欄一欄來解析。
**當時的問題**：原廠 BCB6 程式用的切欄函式（`CommaText`）除了逗號之外，也會用「沒加引號的空白」來切欄；本機測試樣本裡有 188 行是像「,16 System,」這樣的格式，用這種切法會被切壞、整行被跳過不分析。機台自己看記錄檔的畫面（cObserver 的 `ParseEventLogLine`，golden 版在 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cObserver.cpp:3848、移植樹版在 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cObserver.cpp:1387）則只用逗號切欄。分析程式要跟哪一種切法一致？
**選項**：A 照 BCB6 原本的切法（跟參考程式的結果一致，但那些被切壞的行本來就不算進分析）；B 只用逗號切欄，讓分析程式跟機台自己的記錄檢視頁共用同一套切欄邏輯（這是刻意偏離 SVN Rev891 版本的做法）
**St02建議**：A（先求跟參考程式的結果對齊）
**Steven 的裁決**：「基本上是使用 逗號 分隔, 同一個字串會用 引號 包含住」（Steven 20260927 12:1x，在 St01 這邊，github-02 轉述）
⇒ 意思：選 **B**——只用逗號切欄，遇到用引號包起來的字串，整段當成一欄（不要照 BCB6 那種連空白也切的做法）。
**現況**：事件記錄分析程式跟機台自己的記錄檢視頁（cObserver 的 `ParseEventLogLine`：golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cObserver.cpp:3848、移植樹 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cObserver.cpp:1387）可以共用同一套切欄邏輯；這處理方式記成刻意偏離 SVN Rev891 版本。

### W16. SPIL 客戶格式的事件記錄檔要不要特別處理（ELA-2；todo E-005／St02）
**這是什麼功能**：SPIL 客戶格式的事件記錄檔，第一欄放的是 UnitName（單元名稱），跟其他客戶格式不同。
**當時的問題**：因為欄位格式不同，golden 分析程式在檢查每一行的日期欄位時全部都會失敗，結果等於這種格式完全不會被分析。要不要新增一套專門處理 SPIL 格式的欄位對應？
**選項**：A 照 golden，不特別分析 SPIL 格式；B 新增 SPIL 專用的欄位對應
**St02建議**：A
**Steven 的裁決**：「SPIL格式與其他人不同, 就是根據golden處理就好」（Steven 20260927 12:1x，在 St01 這邊，github-02 轉述）
⇒ 意思：選 **A**——照 golden 原本的行為，不特別處理 SPIL 格式（不另外做欄位對應）。

### W17. 事件記錄分析程式可不可以寫回機台自己的設定檔（ELA-3；todo E-005／St02）
**這是什麼功能**：golden 分析程式在分析過程中，會把「缺少的預設值」直接寫回機台的檔案：例如在 D:\HT9045\Error\English\JAM0000.dat 裡（每看到一個沒見過的單元或代碼就補一個 key），每次重新讀取設定時也會寫回 D:\HT9045\config\config.ini。
**當時的問題**：網頁版的分析程式要不要照做，也去寫這些機台檔案？還是只單純讀取、不寫回？
**選項**：A 照 golden 寫回；B 網頁版只讀不寫
**St02建議**：A（照 golden 做）；自動測試（ctest）的寫入位置一律導到系統暫存資料夾（%TEMP%），不要真的寫進機台檔案
**Steven 的裁決**：「A」（Steven 20260927 12:1x，在 St01 這邊，github-02 轉述）
⇒ 意思：選 **A**——照 golden 寫回（缺的預設值寫進 D:\HT9045\Error\English\JAM0000.dat、重讀設定時寫回 D:\HT9045\config\config.ini）；自動測試一律導到 %TEMP%。
**現況**：寫入 JAM0000.dat 這個動作，一律要經過「Status.Security」畫面裡的 `security.jam` 這個唯一寫入口（見 W20）。

### W18. 舊版程式裡幾個明顯的錯誤（bug）要不要順便修掉（ELA-4；todo E-005／St02）
**這是什麼功能**：golden 分析程式裡已知有幾個明顯的錯誤：失敗次數（`iFailCount`）從來不會歸零（每次查詢都往上累加，開機時還會自動先查一次）；「By-Filter」清單只會一直增加、不會清理；功能過濾畫面上的兩個輸入框功能對調了（用起來相反）；O06 摘要報表變成每分鐘就存一次檔（可能太頻繁）。
**當時的問題**：這些明顯是原廠程式的錯誤，網頁版要不要照樣複製這些錯誤（跟參考程式行為一致），還是趁機修掉？
**選項**：A 全部照 golden（含錯誤一起照抄）；B 修掉，並記錄成刻意偏離 golden
**St02建議**：先照 A 跟參考程式對齊確認一致，再逐項改成 B
**Steven 的裁決**：「B」（Steven 20260927 12:1x，在 St01 這邊，github-02 轉述）
⇒ 意思：選 **B**——這幾個明顯錯誤（`iFailCount` 不歸零、By-Filter 清單一直長、功能過濾兩個框對調、O06 摘要每分鐘存一次檔）直接修掉，並記錄成刻意偏離 golden。

### W19. 分析器要挑選哪些週期存檔的記錄檔來讀（ELA-5；todo E-005／St02）
**這是什麼功能**：機台的事件記錄檔可以依不同週期存檔（例如每小時、每 12 小時、每月各存一份），另外還有一份彙整檔 AllEventLog。
**當時的問題**：golden 分析程式對「小時檔／12 小時檔／月檔」這幾種存檔週期的檔案會默默跳過不讀；而 AllEventLog 這份彙整檔又會被一起讀進來，造成同一天的記錄被算了兩次（重複計算）。要不要修正這兩個問題？
**選項**：A 照 golden（跳過那些週期檔、AllEventLog 重複算）；B 修正：把重複的資料去重，並且讓程式接受其他存檔週期的檔名
**St02建議**：跟 W18 一樣，先照 A 對齊確認，再改成 B
**Steven 的裁決**：「B」（Steven 20260927 12:1x，在 St01 這邊，github-02 轉述）
⇒ 意思：選 **B**——挑選檔案時要去重，並且要能接受小時／12 小時／月這些不同存檔週期的檔名；AllEventLog 這份不要重複計算。

### W20. 故障代碼（Jam Code）等級設定的編輯畫面，兩套要不要合併成一套（ELA-6；todo E-005／St02）
**這是什麼功能**：事件記錄分析程式自己有一個編輯頁面，可以設定 Jam（故障）等級、`IncludeMTBA`、`IncludeMTBF` 這些跟故障統計相關的參數。
**當時的問題**：機台本身（Handler）也有自己一套類似的設定畫面。這兩套編輯頁面要不要合併？如果要合併，要合併進網頁的 D:\HT9045\web\page\eventlog.html，還是保留機台本身既有的畫面？
**選項**：A 把編輯功能搬進 D:\HT9045\web\page\eventlog.html；B 保留給機台本身（Handler）現有的畫面，先不動
**St02建議**：B（先不做整合）
**Steven 的裁決**：「兩邊的整合成一個就好了」（Steven 20260927 11:0x，在 St01 這邊，github-02 轉述）
⇒ 意思：把分析程式的 Jam Code 編輯器，跟機台本身「Status.Security」畫面裡的 Jam 頁面，**整合成一套**。
**現況**：St01（github-70）建議以「Status.Security」畫面的 Jam 頁面（程式碼裡叫 `security.jam`，在 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebSecurityJam.cpp）作為唯一的編輯入口；如果 eventlog.html 之後要改，就直接開同一個頁面或共用 `security.jam` 這套程式；寫入 D:\HT9045\Error\English\JAM0000.dat 這個檔案一律要經過 `security.jam`（這樣可以讓保護寫入用的具名互斥鎖 Q6 只剩一個寫入口）。St02 在自己的分支 `v906/steven-st02-on-cbridge` 做 Q5、Q6 這兩題時會一併規劃這個整合。

### W21. 各種自動排程（半夜、每週觸發等）程式碼要集中放在哪裡（ELA-7；todo E-005／St02）
**這是什麼功能**：目前排程觸發散落在兩個地方：分析程式自己的計時器（Timer2，對應 O06／N10 這兩個功能開關），以及機台本身（Handler）的半夜／每週觸發（對應 N25／VTEST 這兩個功能開關）。
**當時的問題**：要不要把所有排程都集中搬進一個統一的排程中心（這裡稱為 ElaHub），還是維持原狀、golden 各自留在原本的地方，只是額外都通知一聲（Post 一個指令）給 ElaHub 知道？
**選項**：A 照 golden，排程各自留在原本的地方，只是都會 Post(cmd) 通知 ElaHub 一聲；B 全部搬進 ElaHub 統一管理
**St02建議**：A
**Steven 的裁決**：「看一下觸發的條件, 如果是時間相關的, 就可以進去hub；另外, 曾經遇過多台機台同一時間發出上傳檔案的需求, 導致伺服器無法接收到全部的資料, 所以要有檢查是否上傳成功的動作, 或者是根據某些條件, 做成分時觸發」（Steven 20260927 11:0x，在 St01 這邊，github-02 轉述）
⇒ 意思：跟「時間」有關的排程（定時、每天、每週）要搬進 ElaHub 統一管理；其他不涉及時間的排程照 golden 留在原處，只需要 Post(cmd) 通知一聲。上傳類的功能要加上「檢查是否上傳成功」的機制（失敗要重試或至少記錄下來），或依照某些條件（例如機台的 Machine ID）把上傳時間錯開，避免多台機台同時上傳把伺服器塞爆。
**現況**：St02 要提出具體方案，記錄在 D:\HT9045_handoff\FROM_STEVEN.md §4（20260927 11:05）。

### W22. 哪些客戶專屬報表要先做（ELA-8；todo E-005；照計畫 §5 歸 Jimmy）
**這是什麼功能**：機台會產生給不同客戶專用的統計報表。已知有實際會送出資料的幾種：CC_CYUEAN 客戶（代碼 868，ChipAdvanced）、N25 這個功能旗標（ChipMos 竹北廠會上傳）、CC_VTEST 客戶（代碼 915／919，MTBF 統計）。
**當時的問題**：這麼多種客戶報表，要先做哪幾種？
**選項**：逐項勾選要不要做
**St02建議**：—（這題原本列在整體計畫 §5-4，github-59 說本來是要 Jimmy 決定的）
**Steven 的裁決**：在 St02 那邊指定先做 O06（自動存事件記錄，Auto save event log）、O19（自動記錄報表，Auto Record Report）、N25-3（故障記錄 FTP 上傳，Jam log FTP）、N10（記錄檔上傳伺服器，Log File Upload to Server）、N34（稼動率與失效報表，OEE And Failure Report）五項一起先做（Steven 20260927 10:3x，在 St02 那邊）；在 St01 這邊另外補充「功能有開的都要做」（Steven 20260927 11:1x，在 St01 這邊）
⇒ 意思：先做上述五項；其他客戶專屬報表（ChipAdvanced 868、偉測 MTBF 915／919、南茂 N25 摘要…）只要對應的功能開關有打開，也都要做，只是排在後面做。
**現況**：記錄在 D:\HT9045_handoff\FROM_STEVEN.md §4（20260927 11:10）；對應待辦 todo E-012。
**後續（#22 的 D-a～D-f 六個子決策，Steven 20260927 13:3x～13:4x 在 St02 那邊定案，經 St02-M 透過 CHAT_ST02 轉達）**：
- D-a：照 golden，O10（O06-1）這個開關有勾選才會跑自動報表。
- D-b：三件事都要做——驗證上傳是否成功、每次都重新產生報表（不用舊資料）、開機時補跑錯過的時段。Steven 原話：「是可以的，也是客戶期望的」，並且要依機台 Machine ID 把上傳時間錯開，避免同時間塞爆伺服器。
- D-c：N34 報表在沒有資料時要加保護（golden 原本會發生 underflow 而當機，這裡記成刻意偏離 golden）。
- D-d：產生報表時要重新掃描資料夾一次，不要用上一次查詢留下的舊清單。
- D-e：報表檔案改用 UTF-8 編碼（這是刻意偏離 golden 用的 cp950 編碼；St02 原本建議照 golden 用 cp950，但 Steven 裁決改用 UTF-8）。
- D-f：FTP 上傳要不要連線，照設定檔裡的功能開關決定（例如 N25-3 的 `bN25_3_EnableULJamLog`、N25-2 的主機帳號密碼），不要另外加環境變數控制；自動測試（ctest）一律用注入的假傳輸模組來測，不要連真的 FTP。「模擬組態要不要一律都用假傳輸」這件事還沒定案，另外開了 **W36** 待決。
- 另外：FTP 傳輸要改用 Windows 內建的 WinINet，不要再用 KYECFTP 這套第三方的 MiniFtpEngine（對應 Steven 對另一題 R0 的裁決：「可以改用 VC 有支援的 FTP」）。
- 完整計畫文件在 St02 分支的 `ela-reports-upload-plan.md`（樹待確認，原文未附完整路徑），副本在 D:\AI_TempFile\ela-reports-upload-plan_St02_4186b125.md。

### W23. 驗證用的比對資料與參考程式要用哪一份（ELA-9；todo E-005／St02）
**這是什麼功能**：V906 新寫的事件記錄分析程式，目前還沒辦法產生「帶有停機時間」的完整報警記錄行，需要拿 BCB 原廠 Handler 實際寫出來的真實記錄檔當測試樣本比對。
**當時的問題**：St01 這台機器上現成的樣本檔案，混雜了很多不同台機器的記錄，不適合直接拿來當驗證基準。要指定用哪一台機器、哪幾天的記錄？參考程式要確認是不是 D:\HT9045_Updater_NSIS\EventlogAnalyzer\EventlogAnalyzer.exe（版本 20.25.891.0）這一份？
**選項**：指定要用哪一台機器、哪幾天的記錄；確認參考程式＝D:\HT9045_Updater_NSIS\EventlogAnalyzer\EventlogAnalyzer.exe（20.25.891.0）
**St02建議**：—（純問題）；另外提到有密碼權限的人可以打開公司備份伺服器（backsrv）上的 Rev891.0 壓縮檔，確認跟 SVN 版本庫的 r891 是同一份
**Steven 的裁決**：「`D:\HT9045_Log\EventLogTxt` 這邊有範例檔案可以使用」（Steven 20260927 12:1x，在 St01 這邊，github-02 轉述）
⇒ 意思：驗證用的比對資料，用 D:\HT9045_Log\EventLogTxt 資料夾裡的範例檔案；參考程式維持原題指定的 D:\HT9045_Updater_NSIS\EventlogAnalyzer\EventlogAnalyzer.exe（版本 20.25.891.0）。

### W24. 資料庫函式裡一個「暫代用」的舊介面（兩個參數版本）怎麼處理（cMyDB P4-D1；todo D-004／St02）
**背景說明**：這題出自 St02 在 CHAT_ST02（D:\HT9045_handoff\CHAT_ST02.md）20260926 22:34 提出的「cMyDB P4 要先決定的，技術面為主」那一批問題。
**這是什麼功能**：`MyDBIProcess` 這支資料庫處理函式，golden 原本是 3 個參數的版本；目前程式裡還留著一個只有 2 個參數的「替身」版本（相容用的舊介面），供還沒更新呼叫方式的地方繼續使用。
**當時的問題**：這個 2 參數的替身介面要不要繼續留著？
**選項**：A 先保留成一個轉接器，之後有空再拿掉（St02 建議）；B 其他做法（詳見 St02 說明）
**St02建議**：A
**Steven 的裁決**：「A」（Steven 20260927 10:18，在 St02 那邊，St02 轉達）
⇒ 意思：2 參數版本的 `MyDBIProcess` 先保留成一個轉接器，內部轉呼叫 golden 的 3 參數本體，之後有空再把這個轉接器拿掉。

### W25. 生產紀錄寫檔函式（ProductionLog）的兩個呼叫點要不要翻譯實作（cMyDB P4-D2；todo D-004／St02）
**背景說明**：同樣出自 St02 CHAT_ST02（D:\HT9045_handoff\CHAT_ST02.md）20260926 22:34 那批問題。
**這是什麼功能**：`ProductionLog` 這支函式負責寫生產紀錄，程式裡有兩個地方會呼叫它。
**當時的問題**：這兩處呼叫要先關閉不處理，還是要真的翻譯實作？如果要翻譯，需要做一個 `MemoProductionLog` 的門面（facade）介面，並把資料轉存到 D:\RMS 這個路徑。
**選項**：A 先關著，不處理（St02 建議）；B 翻譯實作（需要 MemoProductionLog 門面＋轉存到 D:\RMS）
**St02建議**：A
**Steven 的裁決**：「B」（Steven 20260927 10:20，在 St02 那邊，St02 轉達）
⇒ 意思：要翻譯實作 `ProductionLog`（需要做 MemoProductionLog 門面，並把資料轉存到 D:\RMS）；動到這種共用檔案之前，St02 要先在 D:\HT9045_handoff\FROM_STEVEN.md §1 貼出行號讓其他人核對。

### W26. 資料庫處理函式要不要加上 `__fastcall` 呼叫慣例（cMyDB P4-D3；todo D-004／St02）
**背景說明**：同樣出自 St02 CHAT_ST02（D:\HT9045_handoff\CHAT_ST02.md）20260926 22:34 那批問題。
**這是什麼功能**：`MyDBIProcessNew` 這支函式的呼叫慣例（calling convention）設定。
**當時的問題**：要不要幫這支函式加上 `__fastcall`（一種 C++ 呼叫慣例修飾詞）？
**選項**：A 維持不加（St02 建議）；B 加上 `__fastcall`
**St02建議**：A
**Steven 的裁決**：「A」（Steven 20260927 10:20，在 St02 那邊，St02 轉達）
⇒ 意思：`MyDBIProcessNew` 維持不加 `__fastcall`。

### W27. Greatek 客戶專用的訊息紀錄（SaveMessageHistroy）分支要不要翻譯（cMyDB P4-D4；todo D-004／St02）
**背景說明**：同樣出自 St02 CHAT_ST02（D:\HT9045_handoff\CHAT_ST02.md）20260926 22:34 那批問題。
**這是什麼功能**：`SaveMessageHistroy` 這支函式裡有一段專門給 Greatek 客戶用的分支（客戶專屬功能，代號 S25）。
**當時的問題**：這段 Greatek 專用分支要不要翻譯實作？
**選項**：A 先關著，不處理（St02 建議，理由是客戶專屬功能 S25）；B 翻譯實作
**St02建議**：A
**Steven 的裁決**：「B」（Steven 20260927 10:20，在 St02 那邊，St02 轉達）
⇒ 意思：要翻譯實作 Greatek 專用的 `SaveMessageHistroy` 分支。

### W28. 記錄檔根目錄要不要能用環境變數指定路徑（cMyDB P4-D5；todo D-004／St02）
**背景說明**：同樣出自 St02 CHAT_ST02（D:\HT9045_handoff\CHAT_ST02.md）20260926 22:34 那批問題。
**這是什麼功能**：記錄檔（log）存放的根目錄路徑，要不要提供一個環境變數當作接點，方便自動測試（ctest）指定到別的路徑，不動到機台真正的記錄檔目錄。
**當時的問題**：環境變數要取什麼名字、怎麼接，這部分已經在 D:\HT9045_handoff\FROM_STEVEN.md §3 問過筆電（Jimmy）的意見。
**選項**：名稱與放法已在 D:\HT9045_handoff\FROM_STEVEN.md §3 問筆電（Jimmy）
**St02建議**：等 Jimmy 回覆
**現況**：**已完成、結案**（St02 20260927 10:18）。這個 log 根目錄轉向的接點已經接上（commit `f5b2d378`，併入 main 的 commit `6d7e8d23`），兩個 commit 改的都是 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\CMakeLists.txt、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\CMYDB_PORT_LEDGER.md、D:\HT9045\.claude\skills\ht9045-mydb\SKILL.md 這三個檔案，並且已經過筆電那邊兩種組態的驗收關卡（gate）。

### W29. 不同通訊執行緒同時寫共用資料時要不要加鎖保護（cMyDB P4-D7；todo D-004／St02）
**背景說明**：同樣出自 St02 CHAT_ST02（D:\HT9045_handoff\CHAT_ST02.md）20260926 22:34 那批問題。
**這是什麼功能**：機台裡處理 socket（網路連線）通訊的執行緒，在呼叫這批資料庫函式時，會寫入一些共用的狀態資料，但目前沒有加鎖（lock）保護，理論上如果兩個執行緒同時寫，可能會互相干擾。
**當時的問題**：這種沒上鎖的寫法要不要接受？還是要加鎖，或者乾脆把這些寫入動作都丟到另一個統一的計時（tick）執行緒去做？
**選項**：A 接受目前沒上鎖的做法；B 加鎖保護；C 改成丟到 tick 執行緒統一處理
**St02建議**：（St02 這題沒有給建議，原本待確認）
**Steven 的裁決**：「不同的通訊方式本來就是開啟不同的 socket 與事件，都是獨立處理的，所以選 A」（Steven 20260927 10:21，Steven 自己直接裁決，St02 轉達）
⇒ 意思：選 **A**——照 golden 原本的做法，不加鎖（因為不同通訊方式本來就是各自獨立的 socket 和事件，不會真的互相衝突）。

### W30. 進入下一階段工作前，要不要先請 Jimmy 那邊跑完前面幾關的驗收測試（cMyDB P4-D8；todo D-004／St02）
**背景說明**：同樣出自 St02 CHAT_ST02（D:\HT9045_handoff\CHAT_ST02.md）20260926 22:34 那批問題。
**這是什麼功能**：cMyDB（資料庫模組）這項工作分成 P1～P4 幾個階段（P 代表 phase／階段），每個階段完成後要跑過驗收關卡（gate）才能繼續下一階段。
**當時的問題**：進入 P4 之前，要不要先請筆電（Jimmy）那邊把 P1～P3 的驗收關卡都跑過一次確認沒問題？
**選項**：A 先跑過 P1～P3 的驗收關卡再做 P4（St02 建議）；B 不跑，直接做 P4
**St02建議**：A
**現況**：**已完成、結案**（St02 20260927 10:18）。P0～P3 都已經併入 main 分支，並且已經過筆電那邊兩種組態的驗收關卡。

> 註：CHAT_ST02 22:34 那批 P4 決策沒有 P4-D6（原始訊息從 D5 直接跳到 D7），St01 照原始訊息這樣登記，不是漏掉。

### W35. QA 重測時回報給測試機的批次狀態值，要用舊版（906）還是新版（912）的定義（P65；todo 無對應列／St02；待 Jimmy／Steven 確認）
**背景說明**：St02 原本列出 9 處「翻成 V912 行為、不是規定的 906」的地方（原因是 St02 這台機器只讀得到 912 版本的原始碼）。跟真正的 906 版本（NB2 R95，真實的 906_20260618 版本）逐項核對後，其中 AMD 執行期判斷、Multi2D、NonTestToRBin、Test Timeout Skip 重設 P65、ATC 3.6 這幾項，都已經被 Steven 20260926 14:3x「以 906 為底、補上 912 的新增」這條裁決涵蓋掉了；AlarmCode 目錄那項則本來就屬於既有的 R68-MYDB 範圍，也不用另外裁決（稽核記錄在 St02 commit `1fdbb90d`：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\ST02_GOLDEN906_AUDIT.md、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\TESTERCOMM_PORT_LEDGER.md、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TesterComm\Handler\HandlerGpibMsg.cpp、D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp 等 5 個檔）。
**這是什麼功能**：機台在跑「ARM-QA 重測」流程時，會回報一個批次狀態值（`iLotStatus`）給測試機知道目前狀態。**只剩這一項還沒定案**：`RunTestProgram` 函式裡的 [P65] ARM QA 重測邏輯——912 版本的做法是在 `bP65QAING && bNeedTest` 條件成立時，送出 `iLotStatus=1`（十六進位 0x42）；906 版本的做法是在 `bP65QAReTest` 條件成立時，送出 `iLotStatus=2`。這兩種是**互相取代**的關係（不是新增功能疊加），而且測試機那邊看得到、可能會依這個值分流處理。V906 目前照抄的是 912 的做法（送 1）。
**當時的問題**：這一項算不算已經包含在 Steven 20260926 14:3x 那次版本基準裁決裡面？還是要另外單獨裁決，改回 906 原本送 2 的行為？
**白話例子**：如果選 A，QA 重測時測試機收到的批次狀態值是 1（跟現在市面上的 912 機台一樣）；如果選 B，改回 906 原本的 2——如果測試機那邊的程式會依這個值做不同分流處理，選 A 跟選 B 機台會走出不一樣的路徑。
**選項**：A 算在裁決範圍內，維持送 912 定義的 1；B 不算在內，改回 906 定義的 2
**St02建議**：A
**Steven 的裁決**：「A」（Steven 20260927 10:2x；記錄在 RULINGS_20260927 第 6 條）
⇒ 意思：維持送 912 定義的 1，不用改程式。
**現況**：筆電（Jimmy）在 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\NIGHT_REPORT.md §0 第 31 題已經結案。

### W36. 開發用的模擬版要不要真的上傳 FTP（代號：★W36）（St02；todo E-012）
（St02 照 Steven 20260927 21:xx 寫法寫的，原檔在交接分支 `docs/handoff/ST02_QUESTIONS_20260927.md`；文中 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\...`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JamRules.h` 是 St02 分支 `v906/steven-gpib-widget`（MR !3）的內容，St01 這台的移植樹要等合進 main 才有。）
（20260928 00:xx 換成 St02 的新寫法：**選項代號改了**——現在 A＝模擬版只寫紀錄、不送；舊寫法的 B 就是這裡的 A。）
**這是什麼功能**：機台會自動把幾份報表傳到客戶的 FTP 伺服器。例如南茂竹北（客戶碼 851）每天午夜的「Jam Rate」報表；每天上傳生產資料到 Server 的那一項也可以走 FTP。這題問的是**模擬版**（開發用的電腦，不是出貨給客戶的版本）要不要真的連出去。

**問題**：開發電腦上常會放從客戶那邊拷來的設定檔（config.ini），裡面的上傳開關、客戶伺服器位址都是真的。

**選項**
- A：只寫 log。開發電腦就算開了上傳開關，也只在 `D:\HT9045_Log\UploadFile\` 下的 FTP_Log 檔寫一行「會送出 Jam Rate.txt」，什麼都不送。
- B：真的傳，但只連本機 127.0.0.1。開發電腦沒有自己架 FTP 伺服器的話，就會記成上傳失敗。
- C：跟出貨版一樣。開發電腦放了客戶的 config.ini，就會把報表真的傳到客戶的伺服器（例如 10.20.50.3）。

**建議**：A（目前就是這樣設的）。

**例子**：開發同事把南茂竹北的 config.ini 拷到自己電腦測試，午夜到了——A 只多一行 log；B 記一筆「連不上」；C 會把開發電腦上的假資料傳進南茂的伺服器。

**裁決前的狀態**：照 A。出貨版不受影響，照設定開關真的上傳。
- 在哪看：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaFtp.h:41`（`#define W906_ELA_SIM_FTP_LOG_ONLY 1`）。
**Steven 的裁決**：「C」，原話「我們會自己建立ftp伺服器做驗證」（Steven 20260928 早上直接對 St02 裁（ST01-M 當時不在）；St02-M 20260928 06:25 轉述，在交接分支 `docs/handoff/FROM_STEVEN.md` §4 與 `docs/handoff/CHAT_ST02.md` 同時間；St02-M 更正：依 commit 時間是 06:2x）
⇒ 意思：模擬版跟出貨版一樣，照設定開關真的連 FTP 上傳；Steven 會自己架 FTP 伺服器驗證。自動測試（ctest）仍一律用假的傳輸、不連網。
**現況**：St02 已做（St02 分支 `v906/steven-gpib-widget`（MR !3，還沒合進 main） `8c3bb48e`）。每天 00:00 的自動上傳模擬版要不要也跑，是新的小題 W36-1（在 `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md`）。

### W36-1. 模擬版要不要也在每天 00:00 自動上傳（W36 已裁 C 之後的小題）（St02）
（St02 照 Steven 20260927 21:xx 寫法寫的，原檔在交接分支 `docs/handoff/ST02_QUESTIONS_20260928.md`；文中 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\...` 底下 St02 新加的檔是 St02 分支 `v906/steven-gpib-widget`（MR !3）的內容，St01 這台的移植樹要等合進 main 才有。）
**W36-1 模擬版要不要也在每天 00:00 自動上傳**

- 功能：南茂竹北等客戶的三個每天上傳（Jam 週報、Summary、前一天事件紀錄），出貨版每天 00:00 自動送。
- 現況：照你的 C，模擬版按「立即執行」已經會真的連 FTP；但 00:00 的自動上傳，舊程式用 `#ifndef SOFT_SIMULTE` 包起來，模擬版不會自動跑。
- **A（建議，St02 目前這樣做）**：照舊程式，模擬版不自動跑，要驗證就按「立即執行」。例：開發電腦放著過夜，不會在 00:00 自己去連你們架的 FTP。
- B：模擬版也跟出貨版一樣每天 00:00 自動上傳。例：驗證用的 FTP 伺服器每天會收到開發電腦送來的檔。只要改一條規則。
- 在哪看：`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\main.cpp:21425-21459`

**Steven 的裁決**：原話「W36-1 模擬版要不要也在每天 00:00 自動上傳 模擬版預設把 FTP或網路連結的選項給取消選取, 除非我手動去打勾存檔, 那就表示該執行階段下, 我想要驗證這個功能」（Steven 20260928 09:0x，在 ST01-M 對話裡）
⇒ 意思（ST01-M 的理解）：模擬版每次啟動，FTP 上傳與網路磁碟連結的選項**預設都是不勾**；Steven 在這次執行中手動打勾並存檔，就表示這一次要驗證這個功能，這時照出貨版的行為走（包含每天 00:00 的自動上傳）；下次重新啟動模擬版又回到不勾。出貨版不受影響。
**現況**：St02 要改：模擬版開機時把這些選項在記憶體裡當成不勾（要不要同時改寫 `D:\HT9045\config\config.ini`、哪幾個選項算「FTP 或網路連結」，St02 做之前回報），00:00 的自動上傳改成看這次執行的勾選狀態，不再用 `#ifndef SOFT_SIMULTE` 一律關掉。

### W37. 清料時自動把接觸高度拉高：只改記憶體、不寫檔的四個細節（St02；todo H-009）
**這是什麼功能**：Configuration 頁有一個選項（代號 O07）：測試逾時要清料時，機台先把 Index 下壓的接觸高度暫時拉高，清完料再改回來。Steven 先前的方向是「只改機台正在用的數值，不寫配方檔」。
**St02 查到的事**：BCB 版（golden V912）拉高時**會寫配方的 `Contact.Data`**（拉高寫一次、改回再寫一次）：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\atester.cpp:3860-3872`、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\csystem.cpp:15792-15804`（906 對照版同一段在 `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\atester.cpp:3830-3842`、`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\csystem.cpp:15020-15032`）。移植樹可以做到「只改記憶體」，但要改的是「配方接觸參數讀進記憶體的那一份」（程式裡叫 DeviceForm_File），不是畫面正在顯示的那一份（DeviceForm）——改錯那一份的話，下一次按 Start 會被檔案內容蓋回去，拉高就沒效。
**要 Steven 確認四點**（括號是 St02 建議）：
- ① 只改記憶體裡的配方副本、完全不寫磁碟（建議：是）。
- ② 改回原值之前又逾時一次，要不要再拉高一次？BCB 版會越拉越高（建議：不再疊加，擋住）。
- ③ 清料失敗、或沒有走到清料那一段時，要不要馬上改回原值？BCB 版會一直維持拉高（建議：馬上改回）。
- ④ 客戶 JSCC 的機台（接觸高度存在 `D:\HT9045\system\ContactInfo.ini`，開關 `bContactHeightSaveToContactIni`）照 BCB 版只切模式、不改高度（建議：照 BCB 版）。
**例子**：接觸高度設 2.00 mm、拉高量 0.5 mm。逾時清料時變成 2.50 mm；清完回到 2.00 mm。照①，全程 `Contact.Data` 裡還是 2.00；照②，清料中又逾時一次時維持 2.50，不會變 3.00。
**裁決前的狀態**：等 Steven 回（St02 20260927 14:16 提出；完整調查在 St02 機台上，St01 這台沒有）。
**Steven 的裁決**：原話「W37 他是特殊功能 //Sam 20250820 : [I49] 清料時 Contact Heigh 要拉高 先不移植, 放到todo」（Steven 20260928 08:5x，在 ST01-M 對話裡）
⇒ 意思：這是客戶特殊功能（程式註解 `//Sam 20250820 : [I49] 清料時 Contact Heigh 要拉高`），現在先不移植，四點細節都不用答；列進待辦 H-110，要做時再回來看這四點。
**現況**：沒有程式要改；待辦 `D:\HT9045\.claude\skills\ht9050-construction\references\todo.md` H-110。

### W38. 事件紀錄分析器：偉測（客戶代碼 915／919）機台要不要也讀一般的每日 log 檔（St02；todo E-005）
**這是什麼功能**：分析器要挑出 `D:\HT9045_Log\EventLogTxt` 裡的 log 檔來算統計。Steven 先前定了：同一筆事件出現在好幾個檔裡只算一次，檔名是別種存檔週期的也收（代號 W19）。
**影響**：這樣做之後，偉測機台的 MTBF 週報也會讀一般的每日 log 檔。BCB 版在偉測機台**只讀檔名含 `_EventLogTxt_` 的檔**（`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:808-810`），所以偉測週報的資料來源會跟舊程式不同：重複的不會重算，但會多讀到舊程式略過的檔。
**選項**：A 偉測機台也照新規則讀每日檔（資料比較完整）／B 偉測機台維持 BCB 版只讀 `_EventLogTxt_` 檔的規則。兩種都做好了，用一個開關切換。
**St02建議**：A。
**例子**：偉測機台某週有 7 個每日檔＋1 個 `_EventLogTxt_` 週檔——A：8 個檔都讀，重複的事件只算一次；B：只讀那 1 個週檔，跟舊程式的週報一模一樣。
**裁決前的狀態**：等 Steven 回（St02 20260927 16:41 提出）。
**Steven 的裁決**：「B」，原話「大部份客戶指定功能，都是固定格式，不可以亂更動」（Steven 20260928 早上直接對 St02 裁（ST01-M 當時不在）；St02-M 20260928 06:30 轉述，在交接分支 `docs/handoff/FROM_STEVEN.md` §4 與 `docs/handoff/CHAT_ST02.md` 同時間；St02-M 更正：依 commit 時間是 06:2x）
⇒ 意思：偉測（客戶代碼 915／919）的 MTBF 報表照 golden 讀固定檔名，不另外讀一般的每日 log。通則：之後做客戶指定功能，一律照 golden 的格式，不自己改。
**現況**：St02 已照 B 做（St02 分支 `v906/steven-gpib-widget`（MR !3，還沒合進 main） `c1129d17`）。

### W39. 溫度設定頁：Boost 最低溫的自動調整，BCB 版的判斷看起來寫反了（St02）
**這是什麼功能**：溫度設定頁有「LB（冷卻風）最低溫度」和「Boost（加速升溫）最低溫度」兩個欄位。BCB 版在「待機時間」欄位按下輸入後，會順便檢查這兩個值：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp:5821-5823` 寫的是「**如果 LB 最低溫 ≤ Boost 最低溫**，就把 Boost 最低溫改成 LB 最低溫＋2°C」。
**看起來的問題**：這個條件像是寫反了。照字面，Boost 本來就比 LB 高的時候反而會被改掉（甚至被改低）；原意比較可能是「Boost 太低（不高於 LB）時才拉高到 LB＋2°C」。而且這個檢查掛在「待機時間」欄位上，不是掛在這兩個溫度欄位上。
**選項**：A 照 BCB 版原樣翻（連同疑似寫反的條件）／B 改成看起來正確的方向（記成刻意偏離）。
**St02建議**：A。
**例子**：LB 最低溫 60°C、Boost 最低溫 70°C，操作員改待機時間——A：Boost 最低溫被改成 62°C（照 BCB 版）；B：70°C 不動（因為本來就比 60 高）。
**裁決前的狀態**：等 Steven 回（St02 20260927 17:21 提出）；St01 沒有另外查證，也沒有在機台上看過 BCB 版的實際行為。
**Steven 的裁決**：原話「W39 edtBoostTempMin 是要啟動功能的下限, 所以比他低都要使用該功能,  所以比edtLBTempMin 高兩度是對的」（Steven 20260928 08:5x，在 ST01-M 對話裡）
⇒ 意思：**選 A，照 BCB 版原樣翻，那段不是寫反**。「Boost 最低溫」（edtBoostTempMin）是 Boost 功能啟動的下限，溫度比它低就要用 Boost；所以 Boost 最低溫要比「LB 最低溫」（edtLBTempMin）高 2°C，BCB 版在 LB ≤ Boost 時把 Boost 設成 LB＋2°C 是對的。
**現況**：St02 本機分支 `v906/steven-w39-ts8` `784d1573` 已照 A 做好（只改 St02 的網頁程式），等這個裁決才推。

### W40. Tester 經 TCP 遠端改設定：還有三處沒打開（St02；todo H-009）
**這是什麼功能**：Tester 可以經由 TCP（port 7016／7017）送指令給機台，遠端改設定。Steven 先前定了：這 18 個「遠端改設定」指令加上「清除 ART 重測批資料」指令都照 BCB 版打開。St02 已經打開 16 個，剩下三處：
- ① **Tester 遠端設定「每支腳的下壓力」**（指令代號 HTSET,354）有兩處寫檔：BCB 版是把值先填進 Contact 設定頁的輸入框，再把輸入框的字寫進配方 `Contact.Data`（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Command.cpp:13436` 起）。移植樹還沒有那個 Contact 設定頁的物件，沒有輸入框可以填。
- ② **Tester 遠端「清除 ART 重測批資料」**（HTSET,701，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Command.cpp:13729` 起）最後要寫 `D:\HT9045\system\lastdata.dat`，需要 ART（自動重測）目前的批次狀態；移植樹的 ART 物件還沒有這些欄位，要補得改資料結構。
**選項**：①：A 直接把 Tester 送來的值寫進 `Contact.Data`（檔案結果跟 BCB 版一樣）／B 先不開，等 Contact 設定頁移植好。②：先不開，機台照實回「沒做」（`HTSR,701,,`），不假裝成功，等 ART 那一塊移植好。
**St02建議**：①選 A；②先不開。
**例子**：Tester 送「每支腳下壓力改成 35 g」——A：配方 `Contact.Data` 的 `Force Per Pin G` 變成 35，跟 BCB 版存出來的檔一樣；B：機台回不支援，檔案不動。
**細節**：這批改動在 St02 的分支（真機上要驗的 13 項寫在 St02 分支的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\TESTERCOMM_PORT_LEDGER.md` W10 節；St01 這台的移植樹還沒有這一節）。
**裁決前的狀態**：等 Steven 回（St02 20260927 17:29 提出）。
**Steven 的裁決**：「A」（轉述只有選項代號，沒有另附原話）（Steven 20260928 早上直接對 St02 裁（ST01-M 當時不在）；St02-M 20260928 06:41 轉述，在交接分支 `docs/handoff/FROM_STEVEN.md` §4 與 `docs/handoff/CHAT_ST02.md` 同時間）
⇒ 意思：① Tester 用網路指令 HTSET,354 改每支腳的下壓力：直接把 Tester 送來的值寫進配方 `Contact.Data`（N 取配方的 ForcePerPinN、G 取 Tester 送來的值），檔案結果跟 BCB 版一樣。② HTSET,701（清除 ART 重測批資料）維持不開：回空值、不假裝成功。
**現況**：St02 已做（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Command.cpp` 同一行，St02 分支 `v906/steven-gpib-widget`（MR !3，還沒合進 main） `e2f1f75b`）。

### W41. Tester 經 TCP 報「這批投入數量」時，數量不是數字怎麼辦（St02）
**這是什麼功能**：Tester 經 TCP 告訴機台這一批要投入多少顆（指令代號 HTSET,702，BCB 版 `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\Command.cpp:13784` 起；V912 在 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Command.cpp:13792`）。BCB 版直接把收到的字轉成數字，如果不是數字，程式會跳出錯誤視窗、而且不回覆 Tester。
**選項**：A 移植樹只在 log 記一筆、不回覆 Tester（對 Tester 來說跟 BCB 版一樣沒回應，但機台不跳錯誤視窗）／B 回 Tester 一個錯誤碼。
**St02建議**：A。
**例子**：Tester 送投入數量「12a」——BCB 版：機台跳錯誤框卡住、Tester 等不到回覆；A：機台不跳框、log 記「投入數量不是數字：12a」，Tester 等不到回覆；B：機台回錯誤碼，Tester 知道被拒。
**裁決前的狀態**：等 Steven 回（St02 20260927 17:29 提出）。
**Steven 的裁決**：「A」，原話「gpib跟rs232的命令並不是一比一對齊都有的,目前沒寫到的其實就是等於不支援該功能,所以有通訊紀錄就好」（Steven 20260928 早上直接對 St02 裁（ST01-M 當時不在）；St02-M 20260928 06:41 轉述，在交接分支 `docs/handoff/FROM_STEVEN.md` §4 與 `docs/handoff/CHAT_ST02.md` 同時間）
⇒ 意思：Tester 報投入數量不是數字時，移植樹只在 log 記一筆、不回覆 Tester。通則：某個介面（GPIB／RS232／TCP）golden 沒寫的指令＝不支援，留通訊紀錄就好，不自己補回覆。
**現況**：St02 已記進註解、帳本與 skill（St02-E 20260928 07:09，St02 分支 `v906/steven-gpib-widget`（MR !3，還沒合進 main））。

### W42. 自動報表檔用什麼文字編碼（代號：#22 D-e 的後續）（St02）
（St02 20260927 21:41 照 Steven 21:xx 寫法重寫，原檔在交接分支 `docs/handoff/ST02_QUESTIONS_20260927.md`；文中 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\...`、`D:\HT9045\.claude\skills\ht9045-st02-workflow\...`、`D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\references\ela-reports-upload-plan.md` 是 St02 分支 `v906/steven-gpib-widget`（MR !3）的內容，St01 這台的移植樹要等合進 main 才有；`D:\HT9045_SVN_TempFile\...` 與 golden 樹這台就有。）
**這是什麼功能**：機台（Event Log 分析）會自動產生幾份統計報表檔。有的只存在機台上，有的會傳到客戶的 FTP 或網路磁碟，讓客戶自己的系統去讀：
- 南茂竹北（客戶碼 851）每天半夜上傳的 Jam 統計；
- 「每天上傳生產資料到 Server」那一項（設定頁 N10）；
- 偉測（客戶碼 915／919）每週的 MTBF 週報，存在 `D:\MTBF_Summary`；
- 客戶碼 868 的 OEE／故障報表；
- 「自動存 event log 統計」（設定頁 O06），可以選存到網路磁碟。

**問題**：舊程式寫出來的報表是 Big5（cp950）。你之前定新程式改用 UTF-8。報表裡只有英文和數字時兩種一樣；**有中文字時**（例如機台名稱、代碼說明），如果客戶那邊的程式只認 Big5，打開會變亂碼。我們不知道各客戶的系統吃不吃 UTF-8。

**選項**
- A：全部用 UTF-8（照你原本的決定）。
- B：會傳給客戶系統讀的那幾份維持 Big5，只在機台上看的用 UTF-8。
- C：做成設定開關，預設 Big5，確認客戶可以之後再切 UTF-8。

**建議**：B（客戶收到的檔跟舊機台一模一樣，最不會出事）。

**例子**
- A：偉測的 MTBF 週報裡如果有中文機台名稱，偉測的系統若用 Big5 讀，那一欄會顯示成亂碼。
- B：客戶收到的檔跟舊機台完全一樣；機台網頁上看的報表是 UTF-8。
- C：一開始跟 B 一樣；哪家客戶確認吃 UTF-8，就在那台把開關打開。

**裁決前的狀態**：程式先照 A 做，編碼集中在一個函式，改成 B 或 C 只要動那一處。
- 在哪看：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaReports.cpp:116-124`（寫報表時轉 UTF-8 的地方）；計畫 `D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\references\ela-reports-upload-plan.md:98`。
**Steven 的裁決**：「A」，原話「報表裡面一般沒中文，訊息有中文的，直接幫我改成多語文版本，根據主畫面的語言設定顯示，但是存檔只存英文」（Steven 20260928 早上直接對 St02 裁（ST01-M 當時不在）；St02-M 20260928 06:25 轉述，在交接分支 `docs/handoff/FROM_STEVEN.md` §4 與 `docs/handoff/CHAT_ST02.md` 同時間；St02-M 更正：依 commit 時間是 06:2x）
⇒ 意思：自動報表檔用 UTF-8；報表檔裡的訊息只存英文，畫面上的訊息依主畫面的語言設定顯示（多語）。
**現況**：編碼 A 的部分 St02 本機分支 `v906/steven-w42-encode` 預設就是 A。「只存英文、畫面多語」的五個細節是新的小題 W42-a～e（在 decisions-pending.md），St02 先照建議做。

### W42-a～e. 報表訊息「只存英文、畫面照主畫面語言顯示」的五個細節（W42 已裁 A 之後的小題）（St02）
（St02 照 Steven 20260927 21:xx 寫法寫的，原檔在交接分支 `docs/handoff/ST02_QUESTIONS_20260928.md`；文中 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\...` 底下 St02 新加的檔是 St02 分支 `v906/steven-gpib-widget`（MR !3）的內容，St01 這台的移植樹要等合進 main 才有。）
**背景**：Steven 0928 裁 ★W42＝A（UTF-8），另說「報表裡面一般沒中文，訊息有中文的，直接幫我改成多語文版本，根據主畫面的語言設定顯示，但是存檔只存英文」。St02 查過：

- **告警那幾列本來就是英文**：舊程式記 log 時，訊息一律從 `D:\HT9045\Error\AlarmCodeList.txt`（3,074 個代碼，全部英文）查；語言設定只影響 Note 視窗顯示的長說明，那段不會寫進 log。
- **中文是從這幾個地方進來的**：告警的「錯誤細節」欄（例：WAR15195 的訊息是 "Hot gun 1 air blow too low."，細節欄是「Hot gun 1 流量過低」）、沒有代碼的自由文字（程式裡寫死的中文、Windows 的錯誤訊息、ChangeLog 標題）、網頁寫的 log。

#### W42-a 哪些檔案算「報表」

- **A（建議）**：只改分析器自己寫的報表（Save Summary、Jam Rate.txt、事件記錄頁的 Excel 下載）。例：`D:\RMS\<機台>-SummaryData_<日期>.txt` 的 Top5 訊息是英文；Handler 的 EventLogTxt 裡「系統找不到指定的檔案。」維持原樣。
- B：A＋南茂竹北每天原樣上傳的 EventLog.txt 也轉成英文。St02 不建議——那是客戶指定的上傳，照 Steven「客戶指定功能是固定格式，不可以亂更動」。
- C：Handler 寫事件紀錄時就只寫英文（Windows 錯誤訊息要改抓英文，繁中 Windows 通常沒有英文語言包）。
- 在哪看：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaChipMos.cpp:452-470`

#### W42-b 找不到英文可換的時候怎麼辦

- **A（建議）**：拿掉中文字、留英文部分；全部都是中文就寫代碼。例：「Hot gun 1 流量過低」→ "Hot gun 1"；全部中文時寫 "[WAR15195]"。
- B：原樣保留（報表就不是純英文）。
- C：整格清空。
- 在哪看：`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\csystem.cpp:4570`、`D:\HT9045\Error\AlarmCodeList.txt:670`

#### W42-c 「主畫面的語言設定」是哪一個（Steven 已說，St02 確認一下理解）

- 現在有三個語言設定，彼此沒連在一起：網頁主畫面的語言選單（英／中／日／韓，每台電腦的瀏覽器各自記住）、C++ 舊主畫面的中英切換、Alarm 視窗自己的語言設定（預設中文）。
- **St02 的理解：跟網頁主畫面的語言選單**。例：選單選「繁體中文」，事件記錄頁的訊息馬上變中文，別台電腦不受影響。
- 另問：**Alarm 視窗要不要也改成跟同一個選單？**（建議要，但那支檔不是 St02 的，要跟檔案主人協調；Steven 沒說要之前 St02 不動它。）
- 在哪看：`D:\HT9045\web\page\main.html:47-52`、`D:\HT9045\web\page\main.html:538-568`、`D:\HT9045\web\page\ht9045_alarm_motionview.js:259-266`

#### W42-d 中文（和日文、韓文）的短訊息從哪裡來

- 英文有 AlarmCodeList.txt；中文沒有「一個代碼一句話」的對照表；日文完全沒有；Korea 資料夾的檔案裡沒有韓文字。
- **A（建議）**：用 `D:\HT9045\Error\Chinese\<代碼>.dat` 的第一行當中文短訊息（600 個檔裡有 520 個第一行是中文）；日文、韓文和沒有這個檔的代碼顯示英文。之後可以再加 B，B 有寫的蓋過 A。例（JAM0109）：「Input Arm 在Hot Plate上吸取異常」。
- B：新開一份對照檔 `D:\HT9045\Error\AlarmCodeList_zh-TW.txt`（格式跟 AlarmCodeList.txt 一樣），由 RD 慢慢補。
- C：先只做英文，等有翻譯再說。
- 在哪看：`D:\HT9045\Error\Chinese\JAM0109.dat:1`

#### W42-e 英文以哪一份清單為準

- 機台上的清單跟 V906 出貨帶的清單，有 1,060 個代碼的文字不一樣。
- **A（建議）**：用機台上那份（Handler 記 log 用的就是它）。例（JAM0109）："Device pick-up error on Hot Plate"。
- B：用出貨帶的那份。例（JAM0109）："In arm device pick-up error on hot plate"。
- 在哪看：`D:\HT9045\Error\AlarmCodeList.txt:6`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ship\Error\AlarmCodeList.txt:8`

**裁決前的狀態**：St02 先照建議（a＝A、b＝A、c＝網頁主畫面選單、Alarm 視窗不動、d＝A、e＝A）做；你選不同的再改。

**Steven 的裁決**（Steven 20260928 09:1x，在 ST01-M 對話裡），原話逐題：
- W42-a：「存檔的檔案都算, 除非後續有特別要求要使用中文存檔 (目前沒遇過這樣的要求)」⇒ **所有寫到磁碟的檔都只存英文**（比 St02 建議的 A 範圍大，包含 Handler 自己寫的事件紀錄；南茂竹北原樣上傳的是 Handler 的事件紀錄，所以上傳出去的也會是英文）；之後有客戶指定要中文存檔再另外處理。
- W42-b：「先針對bcb裡面有的訊息相關找一下, 如果原本是中文的, 幫我翻譯成多國語言版本, 記錄到 i18n 檔案之中」⇒ 找不到英文時，先到 BCB 原始碼找同一則訊息；原本就是中文的，翻成多國語言（英／中／日／韓）寫進 i18n 檔（見 W42-d），存檔用英文那一句。
- W42-c：「全部跟 main 畫面統一選」⇒ 所有畫面（包含 Alarm 視窗）都跟網頁主畫面的語言選單走，不再各自一個設定。
- W42-d：「D:\HT9045\web\page\i18n.js」⇒ 中文、日文、韓文的短訊息來源（與 W42-b 的翻譯）都放在 `D:\HT9045\web\page\i18n.js`。
- W42-e：「D:\HT9045\web\page\Alert.Note.html 這裡面目前是抓哪個檔案?  D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\AlarmChannel.h 這邊又是用哪一個? 反正統一成一個上git」
  - ST01-M 查到的：① 網頁 `D:\HT9045\web\page\Alert.Note.html` 經 `D:\HT9045\web\page\ht9045_alarm_motionview.js:311-318` 讀 `D:\HT9045\web\JSON\AlarmCodeList-index.json`（有進 git；20260909 從機台上的 `D:\HT9045\Error\AlarmCodeList.txt` 轉出來）；② `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\AlarmChannel.h` 本身不讀清單，只把警報代碼轉給網頁、由網頁查表（使用者 20260924 裁定）；C++ 另有 St02 的 AlarmCodeUpdater，從程式內建表寫出 `D:\HT9045\Error\AlarmCodeList.txt`，出貨帶 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ship\Error\AlarmCodeList.txt`（有進 git）；③ 機台上的 `D:\HT9045\Error\AlarmCodeList.txt` 被 `.gitignore:75` 排除、不在 git，跟出貨帶那份有 1,060 個代碼文字不同。
  ⇒ **統一成一份進 git 的正本**，出貨帶 txt、C++ 內建表、網頁 JSON 都從這一份產生；內容以機台上那份為準（Handler 記 log 用的就是它，同 St02 原本建議 A）——這一點是 ST01-M 的理解，Steven 要改用出貨帶那份再說。
**現況**：St02 照做（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis` 的報表、Handler 事件紀錄的英文化、i18n.js 翻譯表、警報清單統一）；Alarm 視窗那支檔不是 St02 的，依「任何畫面的事件都是我們做」由 St01／St02 協調後改。

### W43. 「自動存 event log 統計」沒選時段時，多久存一次（代號：O06-4）（St02）
（St02 20260927 21:41 照 Steven 21:xx 寫法重寫，原檔在交接分支 `docs/handoff/ST02_QUESTIONS_20260927.md`；文中 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\...`、`D:\HT9045\.claude\skills\ht9045-st02-workflow\...`、`D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\references\ela-reports-upload-plan.md` 是 St02 分支 `v906/steven-gpib-widget`（MR !3）的內容，St01 這台的移植樹要等合進 main 才有；`D:\HT9045_SVN_TempFile\...` 與 golden 樹這台就有。）
**這是什麼功能**：設定頁的「Auto save event log」勾了「生產資料」，機台會自動把當天到目前為止的統計存成一個檔。另有一個「依時段存檔」可以選每 10 分鐘或每 30 分鐘；**這題問的是沒勾「依時段存檔」的情況**。

**問題**：舊程式那一行的註解寫「預設一小時一次」，但程式實際寫成**每分鐘**存一次（註解跟程式不一致，看起來是當初寫錯）。

**選項**
- A：每小時存一次（照註解的原意）。
- B：每分鐘存一次（照舊程式實際的行為）。

**建議**：A。

**例子**
- A：一天存 24 次；檔案內容一樣是「當天到目前」的統計，只是更新沒那麼頻繁。
- B：一天存 1440 次，同一個檔每分鐘被覆寫一次；存到網路磁碟時流量比較大。

**目前狀態**：程式照 A 做，有開關可以切回 B。
- 在哪看：舊程式 `D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:3014`（註解「預設一小時一次」）、`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:3036`（實際每分鐘）；新程式 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaReports.cpp:277`。
**Steven 20260928**：待定，先維持目前的每小時（Steven 20260928 早上直接對 St02 裁（ST01-M 當時不在）；St02-M 20260928 06:30 轉述，在交接分支 `docs/handoff/FROM_STEVEN.md` §4 與 `docs/handoff/CHAT_ST02.md` 同時間；St02-M 更正：依 commit 時間是 06:2x）。

**補充（ST01-M 20260928 09:0x 查證；Steven 問「沒勾不是就不做嗎?」「這功能是不是跟「Auto save event log」有衝突?」）**：有衝突。Handler 和舊分析器讀同一組設定（`D:\HT9045\config\config.ini` 的 [Event Log]），同一個勾選框在兩邊意思不一樣：
- Configuration 頁「Auto save event log」這組：[O06-1] Event log、[O06-2] Alarm History、[O06-3] Alarm Statist、[O06-4] Production Data、[O06-8] Update Production Record every 10／30 minutes（這題說的「依時段存檔」）（golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:4046-4058`）。
- **Handler**：O06-1～4 照設定的星期與時間把當天資料匯出成 .xls（例 `<機台>_ProductionData_<日期>.xls`，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cObserver.cpp:3002-3165`）；O06-8 勾了才每 10／30 分鐘寫一筆生產紀錄（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:32021-32024`），**沒勾就完全不寫**（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cpublic.cpp:599`，註解「沒開啟就不記Log」）。
- **舊分析器**：O06-4 勾了就把 `<機台>-SummaryData_<日期>.txt` 存到同一個生產資料夾（預設 `D:\RMS`）；O06-8 沒勾時每分鐘存、勾了才 10／30 分鐘（`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:3010-3099`、`:3142-3151`）。
- 衝突：① O06-8 沒勾，Handler＝不做、分析器＝每分鐘做；② O06-4 一個框管兩件事（Handler 的 .xls 匯出、分析器的 SummaryData.txt，同一資料夾、檔名不同不會互蓋）；③ 另外 Handler 自己只勾 O06-4、其他三個沒勾時會提早 return，生產資料不匯出（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cObserver.cpp:3018-3021`，golden 的怪處，照翻）。
- **新增選項 C**：分析器照 Handler 的意思——O06-8 沒勾就不定時存 SummaryData，勾了才每 10／30 分鐘。**ST01-M 建議 C**：同一個框兩支程式意思一致，也跟畫面文字「Update Production Record every N minutes」相符；St02 現在是 A，改 C 只動一行（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaReports.cpp:277`，St02 分支）。例：O06-4 勾、O06-8 沒勾——A 每小時存一次 SummaryData；B 每分鐘；C 不定時存（Handler 照樣按排程匯出 .xls）。

**Steven 的裁決**：原話「W43  就按照你的建議做吧」（Steven 20260928 09:1x，在 ST01-M 對話裡），前面問過「W43. 沒勾不是就不做嗎?」「這功能是不是跟 「Auto save event log」 有衝突?」
⇒ 意思：**選 C**——分析器照 Handler 的意思：[O06-8]「Update Production Record every 10／30 minutes」沒勾就不定時存 SummaryData，勾了才每 10／30 分鐘存。Handler 自己的 O06-1～4 排程匯出照 golden 不變。
**現況**：St02 把 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaReports.cpp:277`（St02 分支）從「每小時」改成 C。

### W44. 「每天上傳生產資料到 Server」選了「指定時間」（代號：N10-3）（St02）
（St02 20260927 21:41 照 Steven 21:xx 寫法重寫，原檔在交接分支 `docs/handoff/ST02_QUESTIONS_20260927.md`；文中 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\...`、`D:\HT9045\.claude\skills\ht9045-st02-workflow\...`、`D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\references\ela-reports-upload-plan.md` 是 St02 分支 `v906/steven-gpib-widget`（MR !3）的內容，St01 這台的移植樹要等合進 main 才有；`D:\HT9045_SVN_TempFile\...` 與 golden 樹這台就有。）
**這是什麼功能**：設定頁「Log File Upload to Server」可以選上傳時間：每天 00:00、每天 08:00 和 20:00、每小時、指定時間。

**問題**：Handler 的設定可以選「指定時間」，但舊的分析器畫面只有前三個選項；選「指定時間」時，舊分析器實際上是**每小時**上傳。

**選項**
- A：照舊（選「指定時間」＝每小時上傳）。
- B：補做「指定時間」：讀 Handler 設的時間，只在那個時間上傳（跟新的排程功能一起做）。

**建議**：A 先照舊（跟舊機台一樣），有客戶要再做 B。

**例子**
- A：指定 07:30，實際每小時整點上傳一次（跟舊機台一樣）。
- B：只在每天 07:30 上傳一次。

**裁決前的狀態**：照 A 做。
- 在哪看：舊程式 `D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:3044-3067`（只有三個選項的判斷）、`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:3112`（讀設定）；新程式 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaReports.cpp:293`。


**補充（St02 20260927 23:xx）**：
如果選 B（補做「指定時間」），Handler 自己那一邊已經有「每天在指定的分鐘跑一次」的寫法可以照著做：`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\HS_Function.cpp:488-509`；新程式的接點在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaSchedule.cpp:570`。目前照 A（照舊當每小時）。
**Steven 的裁決**：「B」（轉述只有選項代號，沒有另附原話）（Steven 20260928 早上直接對 St02 裁（ST01-M 當時不在）；St02-M 20260928 06:30 轉述，在交接分支 `docs/handoff/FROM_STEVEN.md` §4 與 `docs/handoff/CHAT_ST02.md` 同時間；St02-M 更正：依 commit 時間是 06:2x）
⇒ 意思：補做：照 Handler 設的時間，每天上傳一次生產資料。
**現況**：St02 已照 B 做（St02 分支 `v906/steven-gpib-widget`（MR !3，還沒合進 main） `c1129d17`；時間從 `D:\HT9045\config\config.ini` 讀，沒設時是 00:00）。St02 另外查到舊程式畫面上的「指定時間」從來沒存進設定檔，是新的小題 W44-1；關機錯過時間不補傳（W44-2，St02 照 Steven 的話決定，告知）。

### W44-1. 畫面上的「指定時間」舊程式根本沒存起來，要不要接上（W44 已裁 B 之後查到的漏洞；附 W44-2 告知）（St02）
（St02 照 Steven 20260927 21:xx 寫法寫的，原檔在交接分支 `docs/handoff/ST02_QUESTIONS_20260928.md`；文中 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\...` 底下 St02 新加的檔是 St02 分支 `v906/steven-gpib-widget`（MR !3）的內容，St01 這台的移植樹要等合進 main 才有。）
**W44-1 畫面上的「指定時間」舊程式根本沒存起來，要不要接上**

- 功能：Configuration 的 N10 分頁「每天上傳生產資料」選「指定時間」時，挑一個時間，每天那個時間上傳一次（Jimmy 20250912 為客戶 CYUEAN 加的）。
- 查到的事：那個時間欄位只宣告、只被讀，**從來沒有登記到存檔清單**，所以存檔後永遠是 0——舊程式實際上每天都在 **00:00** 上傳，不管畫面上挑幾點。906、912、899 三版都一樣。
- 現況：St02 已照你的 B 做成「每天在指定時間上傳一次」，時間從 `config.ini` 讀；沒設時是 00:00，跟舊程式實際行為一樣。但畫面挑的時間要真的生效，得把那個欄位登記進 Handler 的 Configuration 存檔清單。
- **A（建議）**：接上，畫面挑幾點就幾點上傳。例：挑 07:30 → 每天 07:30 上傳。要改的是 Handler 的 Configuration（筆電的 `cConfiguration.cpp`，網頁那頁是 St01 的），St02 會先去認領。舊的 BCB 912 也有同樣的漏洞，要不要告訴 Jimmy 修，由你決定。
- B：照舊程式，永遠 00:00。例：挑 07:30 → 還是每天 00:00 上傳。
- 在哪看：`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\Config.h:1030`（只有宣告）、`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\HS_Function.cpp:488-509`（只在這裡讀）

**W44-2（St02 已照你的話決定，告知）機台關著錯過那個時間，開機後要不要補傳**

- St02 照你說的「真的只在指定時間上傳」和舊程式：**不補傳**，FTP_Log 記一筆「錯過」；剛好在那一分鐘內開機的話會傳一次。你要開機後補傳再告訴我。
- 在哪看：`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\HS_Function.cpp:488-509`

**Steven 的裁決**：原話「W44-1 A」「W44-2 要補發」（Steven 20260928 09:1x，在 ST01-M 對話裡）
⇒ 意思：
- **W44-1＝A 接上**：Configuration 的 N10「指定時間」畫面挑幾點，就每天幾點上傳（例：挑 07:30 → 每天 07:30）。要把這個欄位登記進 Handler 的 Configuration 存檔清單（移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cConfiguration.cpp`，St02 先到 FROM_STEVEN §1 認領那幾行；St02 說是筆電的檔，但最近一次改動是 St01 的 S98，認領時先確認檔主人），網頁 `D:\HT9045\web\page\Config.Configuration.html`（St01 的頁）接上時間選擇。
- **W44-2＝要補發**（推翻 St02 原本「不補傳」的做法）：機台關著錯過指定時間，開機後要補傳一次；FTP_Log 照樣記一筆。
- 舊的 BCB 912 也有同一個「指定時間沒存」的漏洞（St02 查到）；要不要告訴 Jimmy 修 BCB，Steven 沒說，先不動。
**現況**：St02 改補傳、認領 cConfiguration.cpp 存檔清單那幾行；St01 接 Configuration 頁的時間欄位。

### W45. 警報代碼（Jam Code）設定畫面合成一個（代號：W20）（St02）
（St02 20260927 21:41 照 Steven 21:xx 寫法重寫，原檔在交接分支 `docs/handoff/ST02_QUESTIONS_20260927.md`；文中 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\...`、`D:\HT9045\.claude\skills\ht9045-st02-workflow\...`、`D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\references\ela-reports-upload-plan.md` 是 St02 分支 `v906/steven-gpib-widget`（MR !3）的內容，St01 這台的移植樹要等合進 main 才有；`D:\HT9045_SVN_TempFile\...` 與 golden 樹這台就有。）
**這是什麼功能**：每一個警報代碼可以設「等級」、「要不要算進 MTBA（平均幾次需要人協助）」、「要不要算進 MTBF（平均故障間隔）」。舊的 Handler 在 Security 頁有一個編輯畫面，舊的分析器也有一個；**兩個改的是同一個檔** `D:\HT9045\Error\English\JAM0000.dat`。你已經定「合成一個」，下面三題是合的方式。調查全文：`D:\HT9045\.claude\skills\ht9045-st02-workflow\references\research-w20-jamcode-editor.md`（St02 分支；交接分支副本 `docs/handoff/ST02_RESEARCH_W20_JAMCODE.md`）。

#### W45-1 合一的畫面放哪一頁

- 選項：A 放在 Security 頁的 Jam 分頁，分析器頁放一顆按鈕打開它；B 只放在分析器頁；C 兩頁都放同一個畫面。
- **建議**：A（Security 頁已經有密碼等級、檔案鎖、匯入匯出）。
- 例子：A 操作員在 Security 頁改，在分析器頁按鈕也會跳到同一頁；B Security 頁就看不到這個設定；C 兩頁都能改，畫面一樣。
- 在哪看：`D:\HT9045\web\page\Status.Security.html:56`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebSecurityJam.cpp:12-48`、`D:\HT9045\web\page\eventlog.html:19`。

#### W45-2 日月光中壢（933）與 TERAPOWER（967）兩家：代碼沒設過時預設算不算

- 背景：這兩家客戶，舊 Handler 把「算進 MTBA」存在檔案裡叫 `IncludeMTBF` 的欄位，沒設過時當「要算」；舊分析器把同一個欄位當「算進 MTBF」，沒設過時大多當「不算」（只有「24 Motor」區和 24 個指定代碼當「要算」）。兩邊誰先讀到沒設過的代碼，就把自己的預設寫進檔案，所以結果看哪一邊先跑。其他客戶兩邊沒有衝突。
- 選項：A 照舊（看誰先讀）；B 統一以 Handler 為準（沒設過＝要算），只改新的分析器；C 統一以分析器為準，Handler 跟著改。
- **建議**：B（舊機台開機時 Handler 本來就先把這些代碼寫成「要算」，推論；而且不用動 Handler 的翻譯）。
- 例子（某代碼 WAR01234 從沒設過）：A 這台先開 Handler 就寫成「要算」、先開分析器就寫成「不算」，每台可能不一樣；B 一律寫成「要算」，MTBF 統計會把它算進去；C 一律「不算」（24 Motor 等除外），Handler 畫面上那一格也會跟著變成沒勾。
- 在哪看：`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cSecurity.cpp:1329-1358`、`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cSecurity.cpp:1136-1147`、`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:2838-2882`。

#### W45-3 分析器多出來的那一格怎麼放

- 背景：分析器畫面比 Handler 多一個「算進 MTBF」勾選框。
- 選項：A 每個欄位一格：Handler 原本那格不動，另加一格給另一個欄位（933／967 的第二格標成「Include MTBA（分析器）」）；B 933／967 只顯示 Handler 那一格，另一個欄位沒有畫面、照預設；C 933／967 兩格連動，一改就兩個欄位寫同一個值（兩個舊程式都沒有這種做法）。
- **建議**：A（兩個舊程式能改的欄位都還改得到，也不會兩格寫同一個欄位）。
- 例子：A 一般客戶看到「Include MTBA」「Include MTBF」兩格；933／967 看到「Include MTBA」「Include MTBA（分析器）」兩格。B 933／967 只有一格。C 933／967 勾一格另一格跟著勾。
- 在哪看：`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.dfm:2015-2042`、`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:2770-2772`、`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cSecurity.cpp:1136-1147`。

#### W45-4 其餘 7 點（建議都是「照舊不變」，你沒意見就照做）

誰能改（照 Handler 的密碼等級）；讀到沒設過的代碼就寫回預設（保留）；等級只用 Handler 那一套；代碼說明文字維持唯讀；匯入匯出 CSV 不加欄位；客戶 KYEC 的 FTP 下載覆蓋這個檔時要加檔案鎖（請 Jimmy 的筆電做）；分析器自己產生的額外區段不列出來。細節在上面那份調查全文的 §6。


**補充（St02 20260927 23:xx，給 W45-2 用）**：
日月光中壢（客戶碼 933）的機台，「03 Index Unit」區的 JAM0305 還沒存過設定時：
- **A 照舊**：分析器先讀到就寫 0——之後 Handler 的 Jam 次數也不算 JAM0305；Handler 先讀到就寫 1——之後分析器把 JAM0305 算成 MTBF 故障。每台看開機順序，結果可能不同。
- **B 以 Handler 為準**（建議）：不管誰先讀都寫 1；Handler 算、分析器也算成 MTBF 故障。只改新的分析器。
- **C 以分析器為準**：不管誰先讀都寫 0，兩邊都不算；要改 Handler 那一端。
- 新程式的開關在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JamRules.h:96`（目前 A）。
**Steven 的裁決**：三小題都照建議（轉述只有選項代號，沒有另附原話）（Steven 20260928 早上直接對 St02 裁（ST01-M 當時不在）；St02-M 20260928 06:41 轉述，在交接分支 `docs/handoff/FROM_STEVEN.md` §4 與 `docs/handoff/CHAT_ST02.md` 同時間）
⇒ 意思：合一的警報代碼設定畫面放 Security 頁，分析器頁放一顆按鈕打開；933／967 沒設過的代碼以 Handler 為準，一律算；分析器多的那一格，每個欄位一格、兩格都留。
**現況**：另外 7 點 Steven 沒另外表示，St02 先照 golden 做。

### W46. FTP 用主動模式還是被動模式（代號：Passive 預設）（St02）
（St02 照 Steven 20260927 21:xx 寫法寫的，原檔在交接分支 `docs/handoff/ST02_QUESTIONS_20260927.md`；文中 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\...`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JamRules.h` 是 St02 分支 `v906/steven-gpib-widget`（MR !3）的內容，St01 這台的移植樹要等合進 main 才有。）
（20260928 00:xx 換成 St02 的新寫法：St02 現在建議 **B 被動**，並補了舊程式明確設被動的出處。另 ST01-M 先前記的補充：Handler 自己的「每天上傳生產資料」本來有一個「用被動模式」的設定 `bN10FtpPassive`（`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\HS_Function.cpp:2134`），舊分析器不看它。）
**這是什麼功能**：上傳檔案時，FTP 除了一條「下指令」的連線，還要一條「傳資料」的連線。主動模式是客戶的伺服器回頭連到機台這台電腦；被動模式是機台這台電腦主動連出去。

**問題**：舊的分析器從來沒設這個，用的是 FTP 元件自己的預設值，這台電腦上查不到那個預設是什麼。設錯的話，隔著防火牆會上傳失敗。

**選項**
- A：主動模式（目前的設定）。
- B：被動模式。

**建議**：**B 被動**，再到現場對客戶的伺服器確認一次。舊程式裡凡是有明確設定的地方，用的都是被動：`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\KYECFTP\FTPClient.cpp:119`（另外 :411、:1273、:3846、:4467、:4561、:4886、:5276 也一樣，註解「Steven 20121020 : 實驗看看」）。

**例子**：機台這台電腦的防火牆擋掉外面連進來的連線時——A 主動模式：指令連得上，但資料那條被擋，上傳失敗；B 被動模式：兩條都是機台連出去，大多數防火牆都過得了。

**裁決前的狀態**：開關先留在主動，等你決定；改被動只要改一行。
- 在哪看：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaFtp.h:57`（`kElaFtpPassive = false`）。
- 更正：St02 在 FROM_STEVEN §4 21:3x 那列說「暫定照 golden 的預設」，改成以這裡為準（建議被動）。
**Steven 的裁決**：原話「預設根據bcb設定值，但是有些客戶是可以選的，所以可以增加選項給客戶選」（Steven 20260928 早上直接對 St02 裁（ST01-M 當時不在）；St02-M 20260928 06:25 轉述，在交接分支 `docs/handoff/FROM_STEVEN.md` §4 與 `docs/handoff/CHAT_ST02.md` 同時間；St02-M 更正：依 commit 時間是 06:2x）
⇒ 意思：FTP 預設照舊程式（BCB）的設定值；另外加一個選項，讓客戶自己選被動模式。
**現況**：St02 的做法（N10 用舊程式已有的勾選框、N25 預設主動＋新加 `config.ini` 設定）請 Steven 看，是新的小題 W46-1（在 decisions-pending.md）。

### W46-1. FTP 主動／被動：St02 照裁決的做法，請 Steven 看有沒有不對（W46 已裁之後）（St02）
（St02 照 Steven 20260927 21:xx 寫法寫的，原檔在交接分支 `docs/handoff/ST02_QUESTIONS_20260928.md`；文中 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\...` 底下 St02 新加的檔是 St02 分支 `v906/steven-gpib-widget`（MR !3）的內容，St01 這台的移植樹要等合進 main 才有。）
- 查到的事實：舊程式已經有一個客戶可選的「Passive mode」勾選框（`config.ini` 的 `[FTPUpLoad] bN10FtpPassive`），在 Configuration 的 N10 FTP 分頁，**只有幾個客戶看得到**（海思、Log 上傳 FTP、HANA MICRON 等），正好是「有些客戶是可以選的」。但它只管 Handler 自己的 N10 上傳，南茂竹北那三個 N25 上傳不看它（舊分析器從沒設被動＝元件預設，St02 理解是主動，待上機確認）。
- **St02 的做法**：
  - N10 上傳：照舊程式的勾選框（`bN10FtpPassive`＋埠號 `iN10FtpPort`），沒勾＝主動。
  - N25 三個上傳：預設照 BCB（主動）；照你說的「增加選項給客戶選」，在 `config.ini` 的 `[FTPUpLoad]` 加一個 N25 專用的被動模式設定，預設 0＝主動＝跟現在一樣。不跟 N10 那個綁在一起（是不同的伺服器、不同的帳號）。
  - 網頁的 Configuration 頁要不要加這個新勾選框：那支是 St01 的檔，之後再請 St01；在那之前要改就直接改 `config.ini`。
- 在哪看：`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\HS_Function.cpp:2134`、`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cConfiguration.cpp:3292-3295`

**Steven 的裁決**：原話「W46-1.  目前按照預設值, 然後把每個ftp都加上可以選擇主動與被動的選項, 後續讓客戶可以自己選」（Steven 20260928 09:3x，在 ST01-M 對話裡）
⇒ 意思：現在一律照預設值（舊程式／BCB 的值）；**每一個 FTP 連線都加上「主動／被動」可選的設定**（不只 N10 和 N25，機台上所有用到 FTP 的地方都算），之後客戶自己選。N10 沿用舊程式已有的 `[FTPUpLoad] bN10FtpPassive`；其他每個 FTP 各加一個設定（預設＝現在的行為）。只有部分客戶看得到的限制要不要拿掉，Steven 沒說，照「後續讓客戶可以自己選」先讓畫面上都看得到，St02／St01 做之前再確認。
**現況**：St02 列出所有 FTP 連線、各加一個被動模式設定（`D:\HT9045\config\config.ini`）；網頁 `D:\HT9045\web\page\Config.Configuration.html` 的勾選框由 St01 加。

### W47. 一批做完時的 OEE／警報報表，檔名裡的批號少一個字（代號：N34，只有客戶碼 868）（St02）
（St02 照 Steven 20260927 21:xx 寫法寫的，原檔在交接分支 `docs/handoff/ST02_QUESTIONS_20260927.md`；文中 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\...`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JamRules.h` 是 St02 分支 `v906/steven-gpib-widget`（MR !3）的內容，St01 這台的移植樹要等合進 main 才有。）
**這是什麼功能**：客戶碼 868 的機台，每做完一批，機台會寫一份 OEE／警報報表，檔名是 `ClipAdv_<批號>_<日期>.csv`。批號是從 event log 裡「Lot Start, Lot ID:…」那一行取出來的。

**問題**：舊分析器取批號時，固定跳過「Lot ID:」後面的一個字元（以為那裡有空格），但 Handler 寫 log 時那裡沒有空格，所以舊工具會把批號的第一個字吃掉（作業員編號、運轉模式也一樣）。

**選項**
- A：修正，取完整的批號（目前的程式）。
- B：照舊，少第一個字。

**建議**：A（開關 `keepGoldenBugs` 仍可切回 B）。

**例子**（log 寫的是「Lot ID:AFYD09N370-D001」）
- A：檔名 `ClipAdv_AFYD09N370-D001_2025331.csv`。
- B：檔名 `ClipAdv_FYD09N370-D001_2025331.csv`；批號只有一個字時會變成 `ClipAdv_(null)_…`。

**裁決前的狀態**：照 A。
- 在哪看：新程式 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaReports.cpp:381`（St02 分支合併後的行號）；Handler 寫 log 的地方 `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\uLotInfo.cpp:1609`（`Lot ID:%s`，冒號後沒有空格）；舊分析器取值 `D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\uChipAdvancedFunc.cpp:219-221`。
- 另外：一批做完時送出這份報表的那一段，在移植樹還閘著（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fLotInfo.cpp:7082-7095`，St01 的檔），St02 已請 St01 打開；打開前只有測試會觸發這份報表。
**Steven 的裁決**：「A」（轉述只有選項代號，沒有另附原話）（Steven 20260928 早上直接對 St02 裁（ST01-M 當時不在）；St02-M 20260928 06:30 轉述，在交接分支 `docs/handoff/FROM_STEVEN.md` §4 與 `docs/handoff/CHAT_ST02.md` 同時間；St02-M 更正：依 commit 時間是 06:2x）
⇒ 意思：修正：檔名裡的批號補回少掉的那一個字。
**現況**：St02 已照 A 做，帳本在 St02 分支 `v906/steven-gpib-widget`（MR !3，還沒合進 main） `c1129d17`。

### W48. 事件記錄頁要不要加每天的 OEE 圖，怎麼算（代號：ELA R6 的 OEE）（St02）
（St02 照 Steven 20260927 21:xx 寫法寫的，原檔在交接分支 `docs/handoff/ST02_QUESTIONS_20260928.md`；文中 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\...`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JamRules.h` 是 St02 分支 `v906/steven-gpib-widget`（MR !3）的內容，St01 這台的移植樹要等合進 main 才有。）
**這是什麼功能**：事件記錄（Event Log 分析）網頁上加一張「每天的 OEE 圖」，讓人看出機台一天裡有多少時間在測試、停機、閒置，或整體設備效率是多少。

**問題**：舊的分析器那張 OEE 圖只畫**假資料**（亂數），並沒有真的算；所以這是新功能，要先定怎麼算。真正的 OEE 要先定三件事：一天的計畫時間算 24 小時還是照班別、理想的 UPH 從哪裡來、哪些算良品。

**選項**
- A：簡單比例。用現有的 log 資料算「測試／停機／閒置」各佔 24 小時的比例。
- B：真正的 OEE＝可用率 × 效能 × 良率，要先定上面那三件事。

**建議**：先做 A；等三個定義定了再做 B。

**例子**
- A：一天測試 18 小時、停機 1 小時、閒置 5 小時 → 圖上顯示 75%／4%／21%。
- B：計畫 24 小時、停機 1 小時、實際 900 UPH 對理想 1000 UPH、良品 98% → 95.8% × 90% × 98% ≈ 84.5%。

**裁決前的狀態**：還沒做，等你決定。
- 在哪看：舊分析器畫假資料的地方 `D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:385-412`（`:400` 用 `random(3)+1` 填狀態）；新程式的說明在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\ELA_PORT_LEDGER.md` 的「R6」一節（St02 分支，推上後才有）。
**Steven 的裁決**：「A」（轉述只有選項代號，沒有另附原話）（Steven 20260928 早上直接對 St02 裁（ST01-M 當時不在）；St02-M 20260928 06:30 轉述，在交接分支 `docs/handoff/FROM_STEVEN.md` §4 與 `docs/handoff/CHAT_ST02.md` 同時間；St02-M 更正：依 commit 時間是 06:2x）
⇒ 意思：事件記錄頁加每天的 OEE 圖，用簡單比例（測試／停機／閒置三段加起來 100 %）。
**現況**：St02 先照建議做；三個計算細節是新的小題 W48-1～3（在 decisions-pending.md）。

### W48-1～3. 每天 OEE 圖的三個計算細節（W48 已裁 A 之後的小題）（St02）
（St02 照 Steven 20260927 21:xx 寫法寫的，原檔在交接分支 `docs/handoff/ST02_QUESTIONS_20260928.md`；文中 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\...` 底下 St02 新加的檔是 St02 分支 `v906/steven-gpib-widget`（MR !3）的內容，St01 這台的移植樹要等合進 main 才有。）
**功能**：事件記錄頁新增一個「OEE」分頁，每一天畫一條 100 % 的長條，分成「測試／停機／閒置」三段，下面附表格。舊分析器的 OEE 圖其實是亂數畫的，而且畫完就藏起來（`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:385-416`），所以這是新功能，不是照搬。

**先知道**：這張圖要的三種紀錄（告警停了多久、每顆 IC 的觸壓紀錄、Handler 每小時的稼動時間）V906 目前都還沒寫。所以這張圖只能看 BCB 舊機台寫出來的 log；V906 寫的日子會顯示「無資料」，要等那三個寫檔移植完。

#### W48-1 每天的「測試」時間從哪裡算

- 問題：log 裡沒有「機台正在跑」的直接紀錄。Handler 自己有算稼動時間，但只每小時寫到另一個檔，V906 還沒寫。分析器現有的「Total Test Time」只算測試機真的在測的秒數，換料、手臂移動都不算。
- A：用每天的 Total Test Time（每次觸壓的測試秒數加總）。例：一天觸壓 36,000 次、每次 1.8 秒，算出測試 18 小時；機台其實跑了 22 小時，換料的那 4 小時會算成閒置。
- **B（建議）**：看觸壓的時間點，兩次觸壓間隔 5 分鐘以內就算「在跑」，再扣掉停機。例：04:00～23:00 每 30 秒觸壓一次，10:00 JAM 停 1 小時 → 測試 18、停機 1、閒置 5 小時＝75／4.17／20.83 %。每台 BCB 機台都有觸壓紀錄；兩支手臂同時測不會重複算；「5 分鐘」之後可以改。
- C：用 Handler 的稼動時間檔（`TimeData_年.csv` 的 ProductionTime）。最準，但 V906 還沒寫、VTEST 機台不寫、偶爾才開的機台一列會橫跨好幾天。
- 在哪看：`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:2401-2437`、`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\main.cpp:8180-8235`；樣本 `D:\HT9045_Log\Production_Log\202604\PPLS555_20260409.csv`、`D:\HT9045_Log\TimeData\2026\TimeData_2026.csv`

#### W48-2 「閒置」跟「關機」要不要分開

- 問題：事件紀錄和觸壓紀錄都看不出機台有沒有開機；只有 Handler 的稼動時間檔（開機時間）有，V906 還沒寫。
- **A（建議）**：不分，閒置＝24 小時－測試－停機（關機也算閒置）。例：75／4／21 %。等 V906 會寫稼動時間檔再加 B。
- B：多一段灰色「關機」，只有有稼動時間檔的機台才有。例：開機 22 小時 → 測試 75、停機 4、閒置 13、關機 8 %。
- 在哪看：`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\main.cpp:8166-8177`、`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cMyDB.cpp:380-392`

#### W48-3 「停機」算哪些告警

- 問題：分析器原本的停機時間，把所有 JAM／WAR／MES 告警框開著的時間都算進去，包括「One cycle finish」這種等人按的提示框（MES1640）。
- **A（建議）**：照舊分析器的 Total Stop Time，全部都算。例：JAM 40 分＋One cycle finish 20 分 → 停機 1 小時。跟分析器 By Day 的 Total Stop Time、Handler 畫面的 Jam Time、日月光中壢結批報表的 DOWN TIME 是同一個定義。
- B：只算警報代碼設定裡有勾「計入 MTBA」的。例：One cycle finish 沒勾 → 停機 40 分，那 20 分算閒置。
- 在哪看：`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:857-899`、`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\main.cpp:8226-8229`、`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\uLotInfo.cpp:16113-16115`；樣本 `D:\HT9045_Log\EventLogTxt\2026\01\EventLogTxt_20260120.csv:63`（One cycle finish 那一列）

**裁決前的狀態**：St02 先照建議（B／A／A）做，這三個都只是計算方式，改選項很便宜；你選不同的再改。

**Steven 的裁決**（Steven 20260928 09:2x～09:3x，在 ST01-M 對話裡，三小題都裁了）：
- **W48-2＝B（閒置和關機分開）**：原話「W48-2  有個poewr on time,或是software on time可以記錄, 所以沒測試,沒systemstart的, 就是idle」⇒ 用開機時間（power on time／software on time）來分：程式開著、但沒在測試也沒有 SystemStart 的時間＝閒置；程式沒開的時間＝關機（灰色那一段）。這要 V906 自己記下開機時間——golden Handler 每小時寫的稼動時間（`TimeData_年.csv`，St02 機台的 906 對照 `main.cpp:8166-8177`、`cMyDB.cpp:380-392`）V906 還沒移植，要一起補。
- **W48-3＝B（停機只算勾了「計入 MTBA」的告警）**：原話「W48-3  B  是不是警告由客戶自己定義」；ST01-M 回：是——由客戶在警報代碼設定（`D:\HT9045\Error\English\JAM0000.dat`，W45 合一的畫面放在 Security 頁 Jam 分頁）自己勾；~~沒設過的代碼預設「要算」（W45-2）~~ **更正（St02 20260928 12:16）：照 golden 的預設，只有代碼含 JAM 而且區域是 01～05 才預設算；W45-2 只改了 933／967**，所以客戶不動時跟舊分析器差不多，把 One cycle finish（MES1640）這類提示框取消勾選，那段時間就改算閒置。
- **W48-1（測試時間的定義）**：原話「W48-1 每天的「測試」時間從哪裡算   gpib發出 0x41 到收到 binon 與 echook的時候是完整的測試時間」（Steven 20260928 09:3x）⇒ 每一次觸壓的測試時間＝從 GPIB 送出 0x41（開始測試）起，到收到 BIN ON 與 ECHO OK 為止；一天的「測試」時間是這些時間加總（最接近 A，但起點終點以這兩個訊號為準）。RS232／TTL 介面用對應的開始與結束訊號。St02 要確認 log 裡現有的測試秒數是不是這樣量的；不是的話，V906 要照這個定義自己記。
**現況**：St02 照做：測試＝0x41 到 BIN ON＋ECHO OK 的加總；停機＝勾了「計入 MTBA」的告警；閒置＝開機中沒測試也沒 SystemStart；關機＝程式沒開。要先補 V906 的開機時間紀錄（稼動時間檔）。

### W49. Tester 由離線切到連線時的強制登出：所有客戶都做，還是只限 SPIL（代號：D4）（St02）
（St02 照 Steven 20260927 21:xx 寫法寫的，原檔在交接分支 `docs/handoff/ST02_QUESTIONS_20260928.md`；文中 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\...` 底下 St02 新加的檔是 St02 分支 `v906/steven-gpib-widget`（MR !3）的內容，St01 這台的移植樹要等合進 main 才有。）
**這是什麼功能**：工程師或主管在「離線」時按 Tester 鈕切到「連線」，Handler 會自動把登入權限降回 Operator（有 5 級權限、而且不是 KYEC 的機台，降成 Open），登入鈕變回「Login」，溫度欄位鎖住。

**問題**：舊程式那一段的註解寫「SPIL Handler」，但程式本身**沒有檢查客戶**，所以所有客戶的機台都會跑。

**選項**
- A：照舊程式，所有客戶都跑。
- B：只限 SPIL 的機台（看 SPIL 功能開關）。

**建議**：A（照舊程式的實際行為）。

**例子**
- A：非 SPIL 客戶的工程師切到連線後，也會被降回 Operator，要再登入一次才能改設定。
- B：非 SPIL 客戶切到連線後，權限不變。

**裁決前的狀態**：照 A 寫好，還沒接上（要改 Jimmy 的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp` 三行，已請筆電同意）。
- 在哪看：舊程式 `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\main.cpp:12104-12131`（前面 :12089-12103 沒有任何客戶判斷）；新程式 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:518-565`（St02 分支）。
**Steven 的裁決**：「A」，原話「主要是怕工程師切了功能，卻忘記登出」（Steven 20260928 早上直接對 St02 裁（ST01-M 當時不在）；St02-M 20260928 06:41 轉述，在交接分支 `docs/handoff/FROM_STEVEN.md` §4 與 `docs/handoff/CHAT_ST02.md` 同時間）
⇒ 意思：Tester 由離線切到連線時強制登出成 Operator，所有客戶都做，不限 SPIL。
**現況**：本體在 St02 的 `WebLogin.cpp`（St02 分支 `v906/steven-gpib-widget`（MR !3，還沒合進 main） `08ee9c9c`）；呼叫點要改 Jimmy 的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp`，等筆電同意（交接分支 `docs/handoff/ST02_FMAIN_CLAIM_20260928.md`）。

### W50. 事件記錄頁四個「存成 Excel」按鈕要寫成什麼格式（代號：ELA P7 XLS）（St02）
（St02 照 Steven 20260927 21:xx 寫法寫的，原檔在交接分支 `docs/handoff/ST02_QUESTIONS_20260928.md`；文中 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\...` 底下 St02 新加的檔是 St02 分支 `v906/steven-gpib-widget`（MR !3）的內容，St01 這台的移植樹要等合進 main 才有。）
**這是什麼功能**：事件記錄頁上的 Alarm、Fail、Top5、By-Filter 幾張表，各有一個存檔按鈕，跟舊分析器一樣，把表格存成 Excel 能開的檔。

**問題**：舊分析器寫的是很舊的 Excel 格式（BIFF2，每一格都當文字、中文用 Big5）。新版 Office 可能會擋掉或只讓唯讀開啟（要上機確認）；而且 Big5 跟「報表用 UTF-8」的規則衝突。舊程式另外有兩個小 bug（超過 255 字會被截斷、覆蓋舊檔時殘留舊資料）。

**選項**
- A：照舊格式寫（約 60 行程式，產出跟舊程式一樣的檔，兩個 bug 修掉，中文用 Big5）。
- B：Excel XML 格式（.xml），UTF-8，Top5 三張表合成一個檔的三個工作表。
- C：真正的 CSV（UTF-8、開頭加 BOM），由網頁用畫面上已有的表格資料產生，瀏覽器直接下載。
- D：CSV 但檔名取 .xls（不建議：Excel 每次都會跳「格式和副檔名不符」）。

**建議**：先做 C（最簡單、不用改伺服器）；客戶一定要 .xls 檔名時再做 A。

**例子**
- A：`AlarmList.xls` 用 Excel 開，跟舊機台一樣，除非 Office 的檔案封鎖設定擋掉舊格式。
- B：`Top5Alarm.xml` 用 Excel 開，中文正常，三張表在三個工作表。
- C：`AlarmList.csv` 直接用 Excel 開，中文正常；舊程式本來每格都是文字，所以內容不會少。
- D：`AlarmList.xls` 開啟時 Excel 先警告，按「是」才開。

**裁決前的狀態**：還沒寫；St02 先照 C 做網頁端（不動伺服器，改成 A 或 B 不會白做太多）。
- 在哪看：舊程式寫 Excel 的地方 `D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\XLSfile.pas:91-272`、四個按鈕 `D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:2245-2297`；新程式的說明在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\ELA_PORT_LEDGER.md`「P7 XLS 匯出」一節（St02 分支）。
**Steven 的裁決**：「D」，原話「你能做成xlsx就太好了」（Steven 20260928 早上直接對 St02 裁（ST01-M 當時不在）；St02-M 20260928 06:32 轉述，在交接分支 `docs/handoff/CHAT_ST02.md`）
⇒ 意思：事件記錄頁四個「存成 Excel」按鈕存成真正的 .xlsx：網頁端產生、每格當文字、Top5 一個檔三個工作表。
**現況**：St02 已做（St02 分支 `v906/steven-gpib-widget`（MR !3，還沒合進 main） `e2f1f75b`）。

### W51. 「Save Summary」能不能自己選存檔資料夾（代號：ELA Save Summary）（St02）
（St02 照 Steven 20260927 21:xx 寫法寫的，原檔在交接分支 `docs/handoff/ST02_QUESTIONS_20260928.md`；文中 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\...` 底下 St02 新加的檔是 St02 分支 `v906/steven-gpib-widget`（MR !3）的內容，St01 這台的移植樹要等合進 main 才有。）
**這是什麼功能**：事件記錄頁的「Save Summary」把畫面上查詢結果的摘要存成一個文字檔。

**問題**：舊分析器的存檔對話框可以選任何資料夾；網頁版為了安全，只讓使用者填檔名，固定存到 `D:\RMS\`（瀏覽器送來的請求不能指定機台上任意的資料夾）。

**選項**
- A：固定存到 `D:\RMS\`。
- B：網頁上提供幾個允許的資料夾讓人選（例如 `D:\RMS\`、`D:\MTBF_Summary\`）。

**建議**：A。

**例子**
- A：名稱填「Line3」→ 存成 `D:\RMS\<Machine ID>-SummaryData_Line3.txt`。
- B：名稱填「Line3」、資料夾選 MTBF → 存成 `D:\MTBF_Summary\<Machine ID>-SummaryData_Line3.txt`。

**裁決前的狀態**：照 A 做好了（St02 分支）。
- 在哪看：舊程式 `D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:2557-2570`（按鈕）、`:2614-2688`（存檔）；新程式 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaService.cpp`（`SummaryNameOk` 與 summary 路由，St02 分支）。
**Steven 的裁決**：「A」（轉述只有選項代號，沒有另附原話）（Steven 20260928 早上直接對 St02 裁（ST01-M 當時不在）；St02-M 20260928 06:41 轉述，在交接分支 `docs/handoff/FROM_STEVEN.md` §4 與 `docs/handoff/CHAT_ST02.md` 同時間）
⇒ 意思：Save Summary 固定存到 `D:\RMS\`，不讓使用者選資料夾（跟 golden 一樣）。
**現況**：St02 已照 A 做（St02 分支 `v906/steven-gpib-widget`（MR !3，還沒合進 main），帳本 `8c3bb48e`）。

### W52. 南茂竹北每天上傳「前一天的事件紀錄」要找哪些檔名（代號：N25-5，只有客戶碼 851）（St02）
（St02 照 Steven 20260927 21:xx 寫法寫的，原檔在交接分支 `docs/handoff/ST02_QUESTIONS_20260928.md`；文中 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\...` 底下 St02 新加的檔是 St02 分支 `v906/steven-gpib-widget`（MR !3）的內容，St01 這台的移植樹要等合進 main 才有。）
**這是什麼功能**：南茂竹北（客戶碼 851）的機台，每天 0 點把「前一天的事件紀錄檔」複製成 EventLog.txt，上傳到客戶的 FTP（設定開關 `bN25_5_EnableUpload`）。

**問題**：舊分析器只找固定檔名 `EventLogTxt_年月日.csv`。如果那台機台的事件紀錄檔名有加機台編號、用了 N10 的檔名規則，或不是一天一個檔（例如一小時一個），就找不到檔，那天不上傳，只在 FTP_Log 記一行。

**選項**
- A：照舊，只找 `EventLogTxt_年月日.csv`。
- B：也認有機台編號／機型的檔名；當天剛好只有一個符合就傳（FTP 上仍叫 EventLog.txt）。
- C：把前一天所有的事件紀錄檔（含小時檔）接成一個再傳。

**建議**：A（跟舊程式一樣）；如果知道南茂現場有開「檔名加機台編號」，就選 B。

**例子**（要傳的是 2026-11-01 的紀錄）
- A：檔名 `EventLogTxt_20261101.csv` 的機台照傳；檔名是 `EventLogTxt_M01_20261101.csv` 的機台，這天不傳。
- B：上面兩種都會傳。
- C：一天 24 個小時檔的機台，也會把完整一天接起來傳。

**裁決前的狀態**：照 A 做好（St02 分支）。
- 在哪看：新程式 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaChipMos.cpp:451-481`（檔名在 :460，St02 分支）；舊分析器 `D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\uChipMosZHUBEI_Func.cpp:134-143`、`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\uAnalysisEventLogText.cpp:27-41`；Handler 的檔名設定 `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cprod.cpp:2399-2404`。
**Steven 的裁決**：「A」（轉述只有選項代號，沒有另附原話）（Steven 20260928 早上直接對 St02 裁（ST01-M 當時不在）；St02-M 20260928 06:41 轉述，在交接分支 `docs/handoff/FROM_STEVEN.md` §4 與 `docs/handoff/CHAT_ST02.md` 同時間）
⇒ 意思：南茂竹北每天上傳前一天的事件紀錄，照舊只找 `EventLogTxt_年月日.csv` 這個檔名。
**現況**：St02 已照 A 做（St02 分支 `v906/steven-gpib-widget`（MR !3，還沒合進 main），帳本 `8c3bb48e`）。

### W53. Tester 鈕切換連線／離線時，要不要照舊程式打開兩道限制（代號：GB P2d 的 D1、D2；確認題）（St02）
（St02 照 Steven 20260927 21:xx 寫法寫的，原檔在交接分支 `docs/handoff/ST02_QUESTIONS_20260928.md`；文中 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\...` 底下 St02 新加的檔是 St02 分支 `v906/steven-gpib-widget`（MR !3）的內容，St01 這台的移植樹要等合進 main 才有。）
**這是什麼功能**：操作員按主畫面的 Tester 鈕，在「連線」和「離線」之間切換。舊程式切換前會先檢查兩件事，新程式當初（9/26 你說「先列待辦」）沒做；9/26 晚上你說「待辦裡面，你可以做的就先做吧」，St02 現在照舊程式做好了，要改 Jimmy 的檔才會生效，所以先跟你確認。

**要打開的兩道限制**
- D1 權限：登入等級不夠（低於權限表第 8 項）而且不是遠端指令時，不能切換；例外是 I40「只能切到連線」的設定（離線 → 連線、操作員等級時仍可切）。
- D2 機台裡還有 IC 或托盤時不能切換，跳 MES1646「請先 Clean out」；**只有出貨版會擋**，模擬版照舊程式不擋。

**選項**
- A：照舊程式，兩道都打開。
- B：先都不開（維持現在：任何人、任何時候都能切）。

**建議**：A（跟舊機台一樣，避免有料時切模式）。

**例子**
- A：操作員（等級不夠）按 Tester 鈕沒有反應；機台還有 IC 時，工程師按也會跳 MES1646，要先清完料。
- B：操作員隨時能切；機台有料時也能切（舊機台不允許）。

**同一批另外三項（照舊程式，不另外問）**：D3 I27 手動整盤（第四題已問）、D6 連線 → 2D_SORT 模式、D7 ASM 機台的連線分支。

**裁決前的狀態**：本體已寫在 St02 的檔（還沒呼叫、不會運作）；要改 Jimmy 的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp` 14 行（行數不變），已請筆電同意。
- 在哪看：舊程式 `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\main.cpp:12072-12087`；改法清單 `docs/handoff/ST02_FMAIN_CLAIM_20260928.md`（交接分支）。
**Steven 的裁決**：「A」（轉述只有選項代號，沒有另附原話）（Steven 20260928 早上直接對 St02 裁（ST01-M 當時不在）；St02-M 20260928 06:41 轉述，在交接分支 `docs/handoff/FROM_STEVEN.md` §4 與 `docs/handoff/CHAT_ST02.md` 同時間）
⇒ 意思：Tester 鈕切換連線／離線時，照舊程式打開兩道限制：權限檢查，以及機台內還有 IC 就擋（MES1646）。
**現況**：本體在 St02 的新檔 `HandlerTesterConnect`（St02 分支 `v906/steven-gpib-widget`（MR !3，還沒合進 main） `8ff2854c`）；要改 Jimmy 的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp` 共 15 行，等筆電同意（交接分支 `docs/handoff/ST02_FMAIN_CLAIM_20260928.md`）。

### W54. HT9050 上要送哪些托盤的狀態（代號：S118）（St02）
（St02 照 Steven 20260927 21:xx 寫法寫的，原檔在交接分支 `docs/handoff/ST02_QUESTIONS_20260928.md`；文中 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\...` 底下 St02 新加的檔是 St02 分支 `v906/steven-gpib-widget`（MR !3）的內容，St01 這台的移植樹要等合進 main 才有。）
**這是什麼功能**：Handler 現在會把主畫面上每個托盤（Tray）的狀態——每一格有沒有料——送給瀏覽器，讓 Motion View 頁把托盤畫出來。

**問題**：舊程式沒有 HT9050 的版本，每種機型都綁同樣的 34 個托盤；沒有資料說 HT9050 實際上有哪些托盤。

**選項**
- A：每台都送 34 個，由網頁依機型設定決定畫哪幾個（目前的程式）。
- B：Handler 只送 HT9050 清單上有的托盤（要先確認那份清單）。

**建議**：A（跟舊程式一樣；網頁本來就會依機型篩選）。

**例子**
- A：HT9050 上 `motionView.trays.fix4` 照樣送出，但網頁不畫它。
- B：如果 HT9050 只有 Loader、Auto1～3、Empty（網頁 HT9050 設定裡寫的那幾個），fix1～6、auto4～6 就完全不送。

**裁決前的狀態**：照 A，而且 Handler 送出的那一段還沒接上（等 wb_serve 兩行的同意），所以現在什麼都不會送。
- 在哪看：舊程式綁托盤的地方 `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cinitial.cpp:6430-6471`；網頁的機型設定 `D:\HT9045\web\JSON\Machine-profile.json`（HT9050 的 caps.tracks）；新程式 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\ChanMvTrays.h:49` 起（St02 分支，推上後才有）。

**Steven 的裁決**：原話「W54. A」（Steven 20260928 09:2x，在 ST01-M 對話裡）
⇒ 意思：每台都送 34 個托盤的狀態，由網頁依機型設定決定畫哪幾個（跟舊程式與目前的程式一樣）。
**現況**：St02 的 producer 照 A（已推，還沒有呼叫端）；接上要筆電同意 wb_serve.cpp 兩行，網頁 Main.MotionView.html／main.html 讀 tag 由 St01 接。

### W55. I27「手動分類」機台按 Tester 鈕（代號：D4／D3）（St02）
（St02 照 Steven 20260927 21:xx 寫法寫的，原檔在交接分支 `docs/handoff/ST02_QUESTIONS_20260928.md`；文中 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\...` 底下 St02 新加的檔是 St02 分支 `v906/steven-gpib-widget`（MR !3）的內容，St01 這台的移植樹要等合進 main 才有。）
**這是什麼功能**：有開 I27「手動分類模式」的機台，第一次按 Tester 鈕時，舊程式會先進「手動分類」，不會切到連線，也就不會跑上面第三題的強制登出；再按一次才切到連線。

**問題**：新程式的「手動分類」這一段還沒做（閘著）。第三題接上之後，I27 機台按 Tester 鈕會**直接**切到連線，而且會被強制登出。

**選項**
- A：先這樣，等「手動分類」做了再一致；把它排進待辦。
- B：現在就先加一個例外：I27 機台先不強制登出（還是同一行，多一個條件）。

**建議**：A，另外把「手動分類」排進待辦。

**例子**
- A：I27 機台按一下 Tester 鈕就切到連線、權限降回 Operator（舊程式要按兩下才會這樣）。
- B：I27 機台按一下就切到連線，但權限不變；等「手動分類」做好才完全跟舊程式一樣。

**裁決前的狀態**：還沒接上。
- 在哪看：舊程式 `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\main.cpp:12093-12097`（I27 先進手動分類）；新程式閘著的那一段 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp:1135-1142`。

**Steven 的裁決**：原話「W55. I27「手動分類」是個功能, 功能執行時,操作者是不能真的按連線按鈕的, 請參照bcb修改」（Steven 20260928 09:2x，在 ST01-M 對話裡）
⇒ 意思：**不選 A 也不選 B，直接照 BCB 做**——把 I27「手動分類」那一段移植過來（golden 906 `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\main.cpp:12093-12097`，St02 機台才有這棵；St01 這台對照 V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp` 同一段）：I27 機台第一次按 Tester 鈕先進手動分類、不切連線；手動分類執行中操作員不能真的切到連線。照「任何畫面的事件都是我們做；沒移植的直接實作」。
**現況**：待辦 H-018 改成要做（St02）；改的是 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp:1135-1142` 閘著的那一段，併在 St02 請筆電同意的 fMain.cpp 行數裡一起認領。

### W56. `aHotPlateSubstrate.cpp／.h` 裡三段過期的註解，St02 可以改嗎（小事；代號：P4 後續）（St02）
（St02 照 Steven 20260927 21:xx 寫法寫的，原檔在交接分支 `docs/handoff/ST02_QUESTIONS_20260928.md`；文中 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\...` 底下 St02 新加的檔是 St02 分支 `v906/steven-gpib-widget`（MR !3）的內容，St01 這台的移植樹要等合進 main 才有。）
**這是什麼**：St02 的 cMyDB P4 把「2 參數的 MyDBIProcess」改成真的會寫 log 的轉接器，放在 `aHotPlateSubstrate.cpp`。這支檔裡有三段註解還寫著「離線模擬用的空本體／保守預設」，已經不對了。St01 說這支檔要先問你。

**選項**
- A：讓 St02 只改這三段註解（同一行改寫、行數不變，不動程式）。
- B：先不動。

**建議**：A。

**例子**
- A：`aHotPlateSubstrate.cpp:2` 的「(offline sim bodies)…」改成「2 參數 MyDBIProcess 在 P4 後是會寫 log 的轉接器」。
- B：註解維持原樣，之後讀程式的人可能以為那一段沒有作用。

**在哪看**：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\aHotPlateSubstrate.cpp:2`、`:16-22`、`:1180-1192`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\aHotPlateSubstrate.h:36-47`（St02 分支）；完整清單 `docs/handoff/ST02_P4_STALE_COMMENTS_20260928.md` §4。

**Steven 的裁決**：原話「W56. A」（Steven 20260928 09:2x，在 ST01-M 對話裡）
⇒ 意思：St02 只改 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\aHotPlateSubstrate.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\aHotPlateSubstrate.h` 那三段過期註解（同一行改寫、行數不變、不動程式）。註解要改成描述 P4 的「2 參數 MyDBIProcess 轉接器」，ST01-M 把這當成 Steven 也接受那個轉接器本身（先前「aHotPlateSubstrate.* 要先問 Steven」那一題）。
**現況**：St02 可以推。

### W57. ★W42 裁決之後：「存檔只存英文、畫面照主畫面語言」的做法細節 6 題（St02）
（St02 照 Steven 20260927 21:xx 寫法寫的，原檔在交接分支 `docs/handoff/ST02_W42_DESIGN_20260928.md`；文中 St02 新加或改的程式在 St02 分支 `v906/steven-gpib-widget`（MR !3）或 St02 本機分支，St01 這台的移植樹要等合進 main 才有。）
**背景（St02 的設計摘要）**：

- **做法**：不去改那 1,055 個記 log 的地方，而是在 Handler 寫 log 的**三個總入口**（St02 的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cMyDB.cpp` :1003、:930、:1865）先轉成英文再寫。這樣 EventLogTxt、HANDLER LOG、EventTracker、`D:\RMS\` 的 .logs、批號歷史、ShowMyMessage、ChangeLog、網頁寫的 log 全部一次涵蓋。
- **怎麼轉**：本來就是英文 → 不動；有警報代碼 → 用正本清單的英文；是舊程式裡的中文句子 → 查翻譯表（放在 `D:\HT9045\web\page\i18n.js`，再產生一份給 C++ 用）；都查不到 → 見 Q1。
- **畫面**：存的是英文，畫面照主畫面選的語言翻譯顯示；Alarm 視窗也跟主畫面選單。
- **警報代碼清單只留一份正本**：用已經在 git 的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ship\Error\AlarmCodeList.txt`，內容換成機台上那份（3,074 個）＋V906 自己加的 5 個；C++ 內建表和網頁的 `web\JSON\AlarmCodeList-index.json` 都從它產生。不用改筆電的 `.gitignore`。
- **南茂竹北每天上傳的 EventLog.txt**：從原樣複製改成逐列轉英文再複製。
- **翻譯量**：舊程式裡有 404 組中英對照可以直接用，另外約 195 句只有中文、要翻成英日韓。

**要 Steven 決定的 6 題**：

**Q1 還沒翻好的中文，存檔時怎麼辦**
- **A（建議）**：照「存檔只存英文」，先拿掉中文、留英文部分；整句都是中文就寫代碼（例：「Hot gun 1 流量過低」→ "Hot gun 1"）。查不到的句子會記在記憶體裡、網頁上看得到，RD 再補翻譯。缺點：補上翻譯之前，那幾行的資訊會變少。
- B：先原樣保留中文，等翻譯補上再變英文。資訊不會少，但那幾行不是純英文。

**Q2 使用者自己輸入的內容要不要例外**（例：ChangeLog 的「舊值 ==> 新值」、批號 Lot ID）
- **A（建議）**：例外、原樣保留——那是資料不是訊息，改了會跟實際設定／批號對不上。
- B：一樣轉英文。

**Q3 翻譯表放哪裡**
- **A（建議）**：照 Steven 指定，全部放 `D:\HT9045\web\page\i18n.js`（會從 36 行變成 1,100 多行，主畫面多載 100～150 KB）。
- B：同一個編輯器另外產生 `D:\HT9045\web\page\i18n_msg.js`，主畫面不用每次都載。

**Q4 警報代碼清單的正本，以哪一台機台上的為準**（每台的 `D:\HT9045\Error\AlarmCodeList.txt` 會被 MDB Updater 改、執行中也會自己加新代碼）
- **A（建議）**：先以 STEVEN-NB3 這台現在這份（3,074 個）＋V906 的 5 個為起點，其他機台多出來的代碼之後用工具合進來。
- B：指定另一台（請說哪一台）。

**Q5 C++ 內建的警報代碼表，改成從正本產生嗎**（現在是照 `D:\MDB Updater\MDB_Updater_Rev902.0_20260410\CreatDatabase.cpp` 產生，跟機台那份有 1,060 個代碼文字不同）
- **A（建議）**：是，照「統一成一個」；MDB Updater 那份改成只拿來比對、印差異。
- B：維持照 MDB Updater 產生。

**Q6 SPIL 格式的事件紀錄也一樣轉英文嗎**
- **A（建議）**：是，照「存檔的檔案都算」。
- B：SPIL 照舊（客戶指定格式）。
**目前狀態**：St02 本機分支 `v906/steven-w42-encode` 照建議做中；回法：「W57 照建議」或逐題寫「W57-Q2＝B」。
**Steven 20260929 07:5x 回 Q1**：原話「Q1 還沒翻好的中文，存檔時怎麼辦 如果是note頁面,  肯定都有英文 如果是MyMessageBox頁面, 你把項目列進去 i18n檔案裏面, 我會找人翻譯」⇒ Note 頁（警報說明）一定有英文，照英文存；MyMessageBox 的中文訊息全部列進 `D:\HT9045\web\page\i18n.js`，由 Steven 找人翻譯。**結案**（ST01-M 20260929 07:5x 依 Steven 20260929「你問題也太多了!」整理：照 BCB 或 Steven 既有裁決就能定的直接結案）：Q1 照上面、Q3＝A（Steven W42-d 指定 i18n.js）、Q4＝A（先用 STEVEN-NB3 那份當起點）、Q5＝A（Steven W42-e「統一成一個」）、Q6＝A（「存檔的檔案都算」）。**Steven 20260929 08:0x 其餘五題**：Q2 原話「使用者輸入的不會有英文, 如果非英文的就用utf8存檔」⇒ 使用者自己輸入的內容照原字、用 UTF-8 存（例外）；Q3 原話「你可以針對不同功能使用不同的i18n檔案」⇒ 翻譯表可依功能分成不同的 i18n 檔；Q4「A」；Q5「A」；Q6「對」⇒ SPIL 也轉英文。**W57 全部結案。**

### W58. ★W36-1 裁決之後：模擬版「一啟動就把網路選項當成沒勾」的 5 個細節（St02）
（St02 照 Steven 20260927 21:xx 寫法寫的，原檔在交接分支 `docs/handoff/ST02_W44_1_W36_1_DESIGN_20260928.md`；文中 St02 新加或改的程式在 St02 分支 `v906/steven-gpib-widget`（MR !3）或 St02 本機分支，St01 這台的移植樹要等合進 main 才有。）
（Steven 20260928 09:0x 裁 W36-1 的原話：「模擬版預設把 FTP或網路連結的選項給取消選取, 除非我手動去打勾存檔, 那就表示該執行階段下, 我想要驗證這個功能」；第 1 題的「該執行階段下」ST01-M 讀成傾向 B。）
**★W36-1（要 Steven 決定 5 題）**
- **做法**：只在模擬版，程式一啟動就在記憶體裡把「FTP／網路連結」那些選項當成沒勾（檔案不動）；Steven 手動勾了存檔，那次執行就照勾的跑。存檔時，這些被遮住的選項會先把檔案原值寫回去，**所以在模擬版存任何設定都不會把出貨版的設定改掉**。
- 哪些選項算：A 確定的 FTP／網路磁碟（約 40 個）、B 確定的網路連線（SECS GEM、RMS／Server、N25-1、N24 RTM）、C 有疑問的、D 不算，清單在下面英文 §1。
- 要動筆電的檔（Steven 回了才認領）：`cprod.cpp` :171／:3217／:3263／:3323 同一行各加一個掛勾、`tools\wb_serve.cpp:4052` 裝上、`CMakeLists.txt:3321`。

**給 Steven 的 5 題**：

1. **模擬版裡勾選並存檔後，config.ini 要不要真的寫成「勾」？**
   - A：要寫，之後開出貨版也是勾。例：模擬版勾 N25-3 存檔 → 下次開出貨版每天自動上傳 Jam log。
   - **B（St02 建議，照 Steven「該執行階段下」）**：不寫，只有這次模擬版有效，檔案維持原值。例：模擬版勾 N25-3 存檔 → 這次會上傳；重開模擬版或開出貨版都照檔案原值。
   - 在哪看：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cprod.cpp` 第 3262～3323 行。
2. **「有疑問」那批要不要也算網路（預設關）？**
   - A：全部算。例：N13 ARMS、N28 JSCK OEE IP、N09 TSV、N14 從 MO 路徑下載，模擬版一開都是關。
   - B：只算上傳，下載照舊。例：N31 從 FTP 下載溫度補償、N32 自動下載更新照檔案；N25-3 上傳則關。
   - **C（St02 建議）**：只算「確定」那批，有疑問的照檔案。
3. **客戶碼強制開啟、畫面灰色不能改的項目，要不要也關？**
   - A：也關。例：KYEC_LEE 的 N10-3 固定開，模擬版這次就完全打不開。
   - **B（St02 建議）**：不動，照客戶碼。
   - 在哪看：`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cConfiguration.cpp` 第 3338 行。
4. **TCP 7016／7017 指令伺服器要不要也在模擬版預設關？**（不是 config 項目，是客戶碼決定：Greatek、TeraPower、TeraProbe 會開）
   - A：關（模擬版這次沒辦法從畫面打開）。
   - **B（St02 建議）**：不動。
   - 在哪看：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\CosFunction.cpp` 第 1645、1962、2010、4399 行。
5. **本機存檔路徑如果設成網路分享，要不要也算網路？**
   - A：算，路徑以 `\\` 開頭就當網路，這次不存。例：O06 生產資料路徑設成 `\\server\share` → 模擬版不寫。
   - **B（St02 建議）**：不算，照常存。
   - 在哪看：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaHub.cpp` 第 410～413 行。
**目前狀態**：等 Steven 回；St02 回了才向筆電認領 `cprod.cpp` 等行。回法：「W58 照建議」或逐題寫。
**Steven 的裁決**（20260929 08:1x，在 ST01-M 對話裡，原話逐題）：1.「要」⇒ 模擬版勾選並存檔後，config.ini 真的寫成勾（**更正**：ST01-M 先前把 W36-1 的「該執行階段下」讀成不寫檔，錯了）；2.「要」⇒「有疑問」那批也算網路、預設關；3.「模擬版時. 關 (改成不反灰 也就是元件要enable)」⇒ 客戶碼強制開啟的項目在模擬版也關，而且畫面元件改成可以按、不反灰；4.「關」⇒ TCP 7016／7017 指令伺服器在模擬版預設關；5.「算, 因為有些電腦沒有打開對應的連結, 功能會失效」⇒ 存檔路徑設成網路分享（`\` 開頭）也算網路、模擬版預設不寫。**W58 全部結案。**
**Steven 20260929 11:06 補一題（St02 問、golden 答不了）**：原話「W58-4 ok」⇒ 模擬版 TCP 7016／7017 指令伺服器預設不開，只有 wb_serve 啟動時設環境變數 `W906_SIM_TCP_SERVERS=1` 才開（筆電的 7016 測試在模擬版還能跑）；config.ini 不加新鍵，客戶格式不變。

### W59. ★W45 裁決之後：合一的警報代碼設定畫面還要確認的 3 件（St02）
（St02 照 Steven 20260927 21:xx 寫法寫的，原檔在交接分支 `docs/handoff/ST02_W45_CLAIMS_20260928.md`；文中 St02 新加或改的程式在 St02 分支 `v906/steven-gpib-widget`（MR !3）或 St02 本機分支，St01 這台的移植樹要等合進 main 才有。）
**這是什麼功能**：每個警報代碼可以設等級、要不要算進 MTBA／MTBF（檔 `D:\HT9045\Error\English\JAM0000.dat`）；Steven 已裁合成一個畫面、放 Security 頁 Jam 分頁（W45）。St02 做的時候還有三件要確認。
**還要 Steven 確認的三件**：

1. **W20-7 警報的長說明（`D:\HT9045\Error\<語言>\<代碼>.dat`）要不要能在畫面上改**：兩個舊程式都能改，V906 網頁現在唯讀。
   - **A（建議）**：先維持唯讀，記成偏離；要開放另外做。
   - B：開放改（要處理 Big5、韓文、RTF 內容）。
   - 在哪看：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebSecurityJam.cpp:43`、`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cSecurity.cpp:1183-1201`、`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:2775-2796`
2. **933／967 第二框的字「Include MTBA (Analyzer)」不是舊程式的原文**（舊分析器那格叫 Include MTBF，但 933／967 它實際存的是 IncludeMTBA），可以嗎？
3. **第二框的位置**：放在 Security 頁 Jam 分頁「Include MTBA」下面的空行（left 244／top 280），可以嗎？
- 另外第 18 題的其他 7 點（W20-2／4／6／8／9／10）照舊程式做，Steven 沒另外表示。
**St02 做了什麼（中文摘要，原本這裡貼的是 St02 認領文件的英文原文，Steven 20260929 要求「W45 說中文」改寫）**：合一的警報代碼設定畫面，St02 自己的檔已做好並合進 St02 分支（`6f276498`）：第 18 題三小題照建議——畫面放 Security 頁 Jam 分頁、事件記錄頁的「Jam Code Setting」鈕打開它；933／967 沒設過的代碼分析器一律算（Handler 不變）；分析器多的那一格另放第二框（933／967 顯示「Include MTBA (Analyzer)」）。其餘 7 點照 BCB。第二框要等 St01 那三支檔（WebSecurityJam.cpp、ht9045_wire_statussecurity.js、Status.Security.html）的同一行修改進來才會出現。
**目前狀態**：St02 照建議做（第二框已在 gpib-widget `6f276498`），等 Steven 確認。回法：「W59 照建議」或逐件寫。
**結案**（ST01-M 20260929 07:5x 依 Steven 20260929「你問題也太多了!」整理：照 BCB 或 Steven 既有裁決就能定的直接結案）：第 2 件（933／967 第二框的字）、第 3 件（第二框位置）照 St02 的做法，不再問。**第 1 件 Steven 20260929 08:1x 原話**：「W20-7 警報的長說明（D:\HT9045\Error\<語言>\<代碼>.dat）要不要能在畫面上改：B 要可以改, 並且使用多國語言的欄位方式」⇒ **B**：警報長說明要能在網頁畫面上改，用多國語言欄位（每種語言一格）呈現；要處理 Big5、韓文、RTF 內容。第 2 件原話「可以統一名稱, 有可能是筆誤」⇒ 933／967 第二框的文字統一成一個名稱（golden 那裡可能是筆誤）；第 3 件原話「你決定就好」⇒ 第二框位置由 St02 決定。**W59 全部結案。**

### W60. ★W46-1 裁決之後：Handler 端的 FTP 程式只會被動模式，「每個 FTP 可選主動／被動」怎麼做到（St02）
（St02 照 Steven 20260927 21:xx 寫法寫的，原檔在交接分支 `docs/handoff/ST02_W46_1_FTP_LIST_20260928.md`；文中 St02 新加或改的程式在 St02 分支 `v906/steven-gpib-widget`（MR !3）或 St02 本機分支，St01 這台的移植樹要等合進 main 才有。）
**背景**：Steven 裁 W46-1「每個 ftp 都加上可以選擇主動與被動的選項」。St02 列出 19 組 FTP 連線（清單在交接分支 `docs/handoff/ST02_W46_1_FTP_LIST_20260928.md`，鍵都放 `[FTPUpLoad]`、叫 `b<代號>FtpPassive`、預設＝今天的行為）。問題是 Handler 端移植樹的 FTP 程式 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\KYECFTP\MiniFtpEngine.cpp`（筆電的檔）**只會被動模式**：設定寫「主動」也沒用，今天是主動的那幾組一移植就會變被動。
**選項**：
- A：請筆電在 MiniFtpEngine 補上主動模式。例：某客戶的 FTP 伺服器只收主動連線，選主動就連得上。
- **B（St02 建議，筆電 20260928 11:2x 也選 B）**：還沒移植的 Handler FTP 改用 Windows 內建的 WinINet（事件記錄分析已經這樣用，一個旗標切主動／被動）；已經用 MiniFtpEngine 的確安（KYEC）那幾組維持被動（golden 本來就是被動）。例：N10 每天上傳選主動 → 走 WinINet 主動；確安的下載照舊被動。
- C：先全部被動，設定先存著，之後再補。例：勾了主動也照被動連，等引擎補好才生效。
**目前狀態**：等 Steven 回（筆電與 St02 都傾向 B）；鍵名清單請 St01 確認。
**Steven 的裁決**：原話「W60. B」（Steven 20260929 07:5x，在 ST01-M 對話裡）。**結案**：選 B——還沒移植的 Handler FTP 改用 WinINet（一個旗標切主動／被動），確安那幾組照 golden 維持被動；Jimmy（20260928 11:2x）與 St02 都選 B，屬技術選擇。

### W61. ★W48 裁決之後的新題：按了 Start 但沒在測試的時間，OEE 圖算「閒置」還是「運轉」（St02）
（St02-M 在交接分支 `docs/handoff/FROM_STEVEN.md` §4 20260928 12:16 提出；程式在 St02 本機分支 `v906/steven-w48-rework` 的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaOee.cpp` 與 `D:\HT9045\web\page\eventlog.html`，還沒推。）
**背景**：Steven 裁 W48：測試＝GPIB 送 0x41 到收到 BIN ON＋ECHO OK 的加總；閒置＝開機中「沒測試、也沒 SystemStart」。但還有一段是「按了 Start（SystemStart 中）但這一刻沒在測」——換盤、浸泡、回原點、兩次觸壓之間。
**選項**：
- **A（St02 建議）**：算閒置（沒在測就是沒在產出）；表格另外列一欄 SystemStart 時間可以對照。例：一天 Start 著但換盤 30 分鐘 → 閒置多 30 分鐘。
- B：SystemStart 期間沒在測也算運轉。例：同上 → 那 30 分鐘不算閒置。
**另外更正（St02 同一列）**：W48-3「停機只算勾了計入 MTBA 的告警」裡，ST01-M 先前說「沒設過的代碼預設要算」**不對**——照 golden 的預設，只有代碼含 JAM 而且區域是 01～05 才預設算；W45-2（18b）只改了 933／967 的預設。所以像 MES1640「One cycle finish」預設算閒置，客戶勾了才算停機。
**目前狀態**：等 Steven 回。回法：「W61 A」。
**Steven 的裁決**：原話「W61. system start都算在生產中, 只是內部會再細分成 test time. contact test, off-line, home 之類的」（Steven 20260929 07:5x，在 ST01-M 對話裡）。⇒ **選 B 並細分**：SystemStart 期間全部算「生產中」（不算閒置）；生產中再細分成測試時間（GPIB 0x41 到 BIN ON＋ECHO OK）、contact test、off-line、home（回原點）等類別。St02 照這個重做 OEE 圖的分類。

### W62. R120 的後續：網頁「FTP 下載工作檔」畫面的 2 個細節（St02）
（St02-M 在交接分支 `docs/handoff/FROM_STEVEN.md` §4 20260928 12:16 提出；R120＝A 要把 FTP 下載畫面「下載成功就解開」接上，St02 出了設計：新網頁＋一個動作，約 3～4 天，等 St01 的新頁面題 N-3。）
**背景**：確安（客戶碼 868）機台結批後要到 FTP 畫面重新下載工作檔才能 START（R120）。移植樹要做一個網頁版的下載畫面。
**題一：按「下載工作檔」時機台主迴圈會停幾秒，可以嗎？**
- **A（St02 建議）**：照 golden 在主迴圈同步做（下載約 5～10 秒，這段時間開門警報會晚幾秒；伺服器沒回應時每一步最多等 20 秒；golden 只准機台閒置、機台內沒有 IC 時下載，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\ckernel.cpp:1606-1687`）。例：操作員按下載，畫面轉 8 秒後顯示成功，這 8 秒內按門開關警報晚一點出來。
- B：傳輸改成背景執行，多約半天，要動筆電的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\KYECFTP\FTPClient_Transfer.cpp`。例：下載中主迴圈照跑，警報照常即時。
**題二：上機驗證下載時，工作檔資料夾要不要導到沙盒？**（確安的設定下載成功會刪掉 `D:\HT9045\IniData\Data` 與 Offset 底下其他所有工作檔，還會寫 `D:\HT9045\system\Gerneral.ini`；`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uLotInfo.cpp:12089-12121`）
- **A（St02 建議）**：導到沙盒（環境變數 `W906_INIDATA_ROOT`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\common.cpp:204-206`）＋測試用 FTP 伺服器。例：驗證時只刪沙盒裡的檔。
- B：用真資料夾，先備份 `D:\HT9045\IniData`，驗完還原。例：驗證時真的刪，事後從備份放回。
**目前狀態**：等 Steven 回。回法：「W62 照建議」或「W62-題一＝B」。
**Steven 的裁決**：原話「W62 題一：按「下載工作檔」時機台主迴圈會停幾秒，可以嗎？ 可以」「題二：上機驗證下載時，工作檔資料夾要不要導到沙盒？ 不需要, 當作在客戶端使用的方式即可」（Steven 20260929 07:5x，在 ST01-M 對話裡）。⇒ 題一＝A：照 golden 在主迴圈同步下載（只准閒置、機台內沒 IC）；題二＝不導沙盒：上機驗證就照客戶端的用法，用真的工作檔資料夾。

## 已裁決、不再問（早期整理；依據）


這些題目原本可能出現在舊版盤點或摘要裡，但已經被後續裁決解決，這次整理時刻意不列進 Q／R／W，
免得 Steven 重複回答：

* W32（~~ESD G5：Handler 要不要自己叫起 ESD 程式、跑 HT IonBar 上電流程~~ **Steven 裁決 B**（20260927 07:2x 當面，RULINGS_20260927 第 2 條第 24 題「906 C++ 全部照建議」）：翻 G5 但拿掉 `WakeupESD()`——不啟動 ESD 程式；ESD 程式開著時照 golden 跑 IonBar 上電。St02 做、筆電知會；**St02 已做 `20829c7a`**＝todo H-028）——不用再回。
* W34（~~St02 那台讀不到 golden 906_20260618~~ **Steven 裁決 A**（20260927 07:2x 當面，RULINGS_20260927 第 2 條第 27 題）：Steven 在 St02 那台用正常管道拿到解開的 906（密碼當面給，不經 git、信件或任何文件）；Steven 自己做；**後續（Steven 20260927 10:1x 在 St02 那邊）：St02 先用本機的 `HT9011UC_Code_V3.33.906.0_20260625_Steven` 當 906 對照**，0618 解開版之後再說；St02 引 906 行號一律寫明樹名）——不用再回。
* W31（~~ELA G8：舊的 BCB 分析器 EventlogAnalyzer.exe 要不要由 V906 自己叫起來~~ **Steven 裁決 A**（20260927 07:2x，經 St02 轉達）：V906 不自動啟動舊 exe，`HandlerBridgeCtl.cpp:255` 的 G8 維持 `#if 0`；舊 exe 負責的 FTP 上傳／六項客戶報表等 E-012 ElaReports；St02 寫進註解與文件 `b84c773f`）——不用再回。
* W33（~~St02 ELA 的 SIM 開機實跑要在哪一台做~~ **筆電 20260927 05:2x 已在自己那台做完**：main `231fffa6` 模擬組態 wb_serve 開機約 30 秒，`[ELA] Event Log Analyzer hub started` 有印、GET `/api/ela` 三次 200、`HT9045_ELA=0`⇒503、ELA 開／關各跑一次真檔逐檔相同＝ELA 沒多寫檔；POST 查詢沒測；照 Jimmy 0918 規則備份→驗證→還原，St01／St02 兩台照舊不用跑；St02 帳本／skill／progress-st02 補證據 `e02c57b6`）——不用 Steven 回。
* W2（~~P6-Q2 GPIB 模式配方裡看不到的 RS232 值~~ 已裁決 (a)（20260926 晚，你在對話裡回的，github-59 轉達））——Steven 已裁決 (a)，不用再回。St02 表的目前狀態：**進行中**（St02）。每份 GPIB 配方只抄一次（Tester.Data 留標記，之後操作員存的值不會被蓋）；寫檔後照一般存檔做配方備份（`fMain->BackupSetupFile()`）並記 NewRecordProcess；配方的 RS232 值之後有改，機台停著時 GPIB 程式自動重開套用；testercomm.html 的 GPIB 分頁顯示這組。Setup.TesterIF.html 照 #6 還沒改　W2 留空號。
* 舊 Q18／Q33（`SaveRmsInfo` 關程式時要不要寫 `config.ini [Server]/[RMS]`）——依 S106（S65
  Q2／S89 Q2 併入 S95）＋ S120-1（S95 生產資料段提前，其餘排後）已裁：排在 S95 其餘項裡，
  之後會做，不是還要 Steven 選 A／B（todo `D-007` 進行中）。
* 舊 Q20（wb_serve 關掉時要不要照 golden 存 `lastdata.dat`）——同上，`WriteLastDataFile(true)`
  就在 S120-1 已裁決要提前的生產資料段裡，不再是開放問題。
* S88 Q1（溫度頁尾段 `MainTempOffsetTail` 何時跑）——已由 S107-1 裁決：維持現行「存檔後就
  啟用」，S88 Q1 結案。
* S57 決策題 3（EP 四鍵舊值蓋回、HSys／ContactForce 互蓋）——已由 S90 裁決：同意 HSys 比照
  ContactForce「沒動過的欄位不把舊值蓋回去」。
* S57 決策題 6（`_integrated.txt` 何時加）——已解決：照 `c79ee4e9` 慣例現在就加，主 session
  已做。
* S86 Q1（＝J1，V912 `OCR.dfm` 缺 `rgOCRTriggerMode`）——Steven 裁示不急，已降級記 todo
  `G-006`（交 Jimmy），不是待 Steven 選項題。
* S65 Q3（開機寫 `lastdata.dat` 那一行何時插）——已解決：`c317ca30` 已接，Steven「直接接
  上去」。
* S62／S63「要不要做網頁」（Magazine／FixAICCD）——已由 Steven 13:5x 裁決「要做，但不重要，
  往後排，先做讀寫檔就好」，跟其餘沒有頁面的表單一起往後排，不再單獨問。
* S70 D1（`AutoCalSuckZ` 只在 OEE 機台讀）——工程師確認這是既有設計、非缺陷，非問題，不算
  待決定題。
* S105（LotInfo Server／RMS 下載上傳）、S67（tech.dat）——Steven 已裁決「晚點做／不重要，
  往後排」，本身就是排程結論，不是選項題，不列進 ★ 節（todo `G-021`／`S67` 見進度表）。
* S107-3（按鈕防連點整件由 St01 做，不等 Jimmy）——已裁決並落地（`WebCmdGuard`＋前端），
  剩下交 Jimmy 的只有引擎頁面 busy 顯示與 `motor.access` 細分白名單，那些是「交給 Jimmy」
  清單裡的項目，不是待 Steven 決定。
