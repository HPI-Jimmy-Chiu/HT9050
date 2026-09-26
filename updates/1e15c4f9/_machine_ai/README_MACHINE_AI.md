# 給機台端 Claude：更新包 6（GitLab main `1e15c4f9`，相對更新包 5 `b5fb53be`）

> 筆電端 Claude 20260926 14:4x 產生。**先套更新包 3、4、5，再套這一包。要不要套由 Jimmy 決定**（推上 GitHub 不代表要馬上 pull）。
> 11 檔，底稿 `base_b5fb53be\`。一樣排除 OBJROOT 那 4 個檔。

## 這一包是什麼：第 9 條（RULINGS_20260926 第 9 條＋第 26 條 Q3＋第 27 條 3）

阻塞框（告警框、是／否框、ShowMyMessage 框）在等回答的時候，照 golden 框自己的 Timer1Timer：

| 行為 | 以前（移植樹） | 現在 | golden 出處 |
|---|---|---|---|
| 塔燈／蜂鳴器 | 停在框出現前的樣子，蜂鳴器不叫 | 照 `LastSet.MessageLight`／`MusicSelect` 的「警報／訊息」狀態動；**告警框期間蜂鳴器一律叫**（Jimmy 裁決） | note.cpp:3382、mymessbox.cpp:587 `DoSystemMessage` |
| 面板鍵燈 | 不閃 | 告警框：給的鍵（Retry／Skip…）閃、選了之後 Start／Pause 閃；是／否與 ShowMyMessage 框：Pause 燈閃 | note.cpp:3084-3139 FlushLabel、mymessbox.cpp:585 |
| 面板 Alarm Reset | 是／否框按了沒反應 | 消音（不關框） | mymessbox.cpp:567-579 |
| 沒有網頁 | 一直等，沒人知道 | 30 秒沒有網頁（沒有 WebSocket、3 秒內也沒有 HTTP 請求）就自動開 Edge 正式版畫面（`--new-window`，profile `%LOCALAPPDATA%\HT9045_Edge_Web`，同 HT9045_Web.cmd），之後每 2 分鐘最多一次、一個框最多 3 次 | 新功能（Jimmy 第 9 條） |

### ⚠ 會碰到你們（EastSun）那邊的一件事

**等待迴圈裡現在也會呼叫 `ht9045::Pci1203Monitor()->Poll()`**，節奏同主迴圈（`kIoTickMs`），只呼叫 Poll，不呼叫 `W906_MotorAccessPollTick`。
理由：HT9050 的 `Sen[]` 讀的是監看器的樣本（`Pci1203IoRoute.cpp` FillDi），主迴圈停在等待裡時樣本凍住 ——
面板 Alarm Reset 消音、告警框的面板 Retry／Skip＋Start、ShowMyMessage 的面板 Pause，以及 `DoAvoidIndexMotorFallDown` 的 EMG／斷電判斷都看不到新狀態。
你們 `wb_serve.cpp:537` 那行註解自己也寫了「Poll() does not run inside this wait」。Poll 在 `Pci1203Monitor.h` 的唯讀清單內；若你們有不能在等待中 Poll 的理由，請告訴 Jimmy。

### 在機台上要看的（筆電 ctest 測不到）

1. 跳一個告警框、不開網頁：塔燈照「警報」設定、蜂鳴器叫；面板 Retry／Skip 燈閃；按面板 Alarm Reset 停叫；選 Retry 再按 Start 會重新啟動。
2. 同樣不開網頁等 30 秒：Edge 自己開起來、顯示那個框（log 有 `[modal-wake] ... 第 1／3 次自動開瀏覽器 OK`）。
3. 答完框：塔燈與蜂鳴器在 1 秒內回到正常狀態。

## 同時知會：Jimmy 0926 14:0x 的裁決（RULINGS_20260926 第 27 條，給機台端）

* **R66-GALI＝A**：在 `Motor/myGALILmotor.cpp` 的 `Gali_*` 層依軸的 CardType（`PCI1203`，和第 6 條同一個條件）分流到這一軸的 `Motor->`，**由你們和第 2 條一起做**。同時要滿足：第 6 條在 `HSys.LoadMotData()`（cinitial.cpp:3874）之前生效；Z1 的伺服燈等要從 1203 監看器取（`ht9045_motor` 沒有 `HAVE_PCI1203`）。
* **R66-D13＝B**：照 golden、不改程式 ⇒ **HT9050 的 `bCheckIndexHomeSensor` 必須維持 0**（開了會一直重新歸零）。
* 更新包 5 的請求還在：請把 `machines/HT9050/IO_Table.csv` 現場表推上來。

## 步驟

同前幾包：Check（唯讀）→ EastSun 同意 → 備份 → Apply → **重新 configure＋全量建置** → ctest 與筆電比 → commit 回報。
`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`

筆電驗證（main `66c2480e`，兩組態全量 gate，3 個審查 agent 11 條修正之後）：出貨 188 項＝基準 3＋5 Disabled；模擬＝基準 18＋5 Disabled；
新 ctest `ModalWake` 通過；子檢查與修正前那輪 0 差異；system\ config\ 586 檔 0 變動。
