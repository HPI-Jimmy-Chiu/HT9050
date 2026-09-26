# 給機台端 Claude：更新包 14（GitLab main `21323505`，相對更新包 13 `19844f8e`）

> 筆電端 Claude 20260926 19:3x 產生。**先套更新包 3～13，再套這一包。要不要套由 Jimmy 決定。**
> 10 檔（3 個新檔），底稿 `base_19844f8e\`。一樣排除 OBJROOT 那 4 個檔。

## 這一包是什麼：`UpdateMainOperateMode` 整支照 golden 翻了（Jimmy 0922 裁決「都要，全部動作都要執行」）

golden `TfMain::UpdateMainOperateMode`（main.cpp:12803-13127）＋它呼叫的 `ChangeATCSiteUse`（:13129-13942）、`TemperatureEditDisable`、`SetNormalOrPrime`，約 1,200 行。
在這之前 wb_serve 裡它只是一個計數樁。

### ⚠⚠ 機台上的行為變化（一定要知道）

**wb_serve 開機（`InitialHandler` → `LoadMachineRecord` → `SetMainRunStartMode`）、讀配方、切運轉模式、切 On-Line／Off-Line 時都會真的跑它**，照 golden：

* **加熱器繼電器 `SW[SwHeaterRelay]`**：常溫模式 → Off；ATC 主動冷卻時照 golden 看新 ATC 型號（6.0／5.1／7.0 且沒有 EMG → On，否則 Off）—— ⚠ 移植樹的 `ATC_InterfaceForm` shim 的 `iATC_MODE_TYPE` 目前固定是 0（沒人維護，INBOX 第 60 列），所以這一支實際上只會走到 Off；其他溫度模式照 golden 的分支。
  你們的 IO 表若有 `SwHeaterRelay` 那一列，**開機就可能切它**。
* **ATC 命令**：`ATCInterfaceForm->SendCommToATC7(ATC_STOP／ATC_SET_TEMP／ATC_RUN)`（有 ATC7.0 時）、`ChangeATCSiteUse` 的站點通道。
  移植樹沒有 ATC 連線，新 ATC 那一大段照 golden「沒連線就跳過」。
* **寫 `system\lastdata.dat`／`lastdata_backup.dat` 與 `config\config.ini` 的四節**（Vibrate_Time、P65_QAMode、SocketContact、O_Count 接觸壽命計數）—— golden 每次都寫。
  筆電實測：lastdata 只多不少（339 個原本是 0 的位元組變成有值）、config.ini 只多空行（值不變，第二次開機完全相同）。

主畫面的溫度顯示元件（圖示、標籤、Prime／Normal 面板）屬於網頁，這一包不動它們。

### 架構（為什麼要經 hook）

本體在 `forms/fMain_OperateMode.cpp`／`fMain_ATCSiteUse.cpp`（**ht9045_sm**，`CMakeLists.txt:2748`）；虛擬的 `TfMain::UpdateMainOperateMode` 留在 `forms/fMain.cpp:507`（ht9045_forms，不可依賴 sm），
計數＋經 `W906_UpdateMainOperateModeHook` 呼叫本體；hook 由 wb_serve 在 `InitialHandler` 之前裝（`tools/wb_serve.cpp:4064`，開機 log 會印 `UpdateMainOperateMode body installed = yes`）。
沒裝 hook 的程式（每一支 ctest）＝原本的計數樁。

## 在機台上要看的

1. **先確認機台周圍有沒有人**（加熱器繼電器會動）。全量重編、開機：log 有 `UpdateMainOperateMode body installed = yes` 與 `InitialHandler ... done -> true`。
2. 開機後看加熱器繼電器是否照工單的溫度模式（常溫→關）。
3. ctest：新的 `OpModeBody`（15 項）通過；ctest 前後 `system\lastdata*.dat`、`config\config.ini` 的 MD5 不變（ctest 不裝 hook）。
4. 開機前後比對 `system\lastdata.dat`／`config\config.ini`：值的變化應該只有「多寫進去」，不應該有原本有值的欄位變成 0 —— 若有，回報給筆電。

## 步驟

同前幾包：Check → EastSun 同意 → 備份 → Apply → **重新 configure＋全量建置** → ctest 與筆電比 → commit 回報。
`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`

筆電驗證：兩組態全量 gate＝基準（出貨 3＋5 Disabled、模擬 18＋5 Disabled），`OpModeBody` 兩組態通過；wb_serve 開機煙霧測試（sim、ship 各一次）都正常起來、正常結束，
寫到的真實檔照「備份→驗證→還原」量過並從快照還原。兩個唯讀審查 agent 對 golden 逐句比對，抓到的兩個 major（ATC 沒連線被當成已連線、forms→sm 分層）已修。
