# S-10 Tray Edit（網頁視窗 HW.TrayEdit）怎麼接的 —— St02，20260930

> golden：906_0625_Steven `uTrayEditForm.cpp`（TTrayEditForm）、入口在 `cSortCT.cpp`（右鍵盤面）。
> commit（都在 gw，MR !6）：
>
> * `08f8aec9`：我們的檔；
> * `505c33b6`：E2 審查的第 1、2 點；
> * `d393df85`：St01 的 e6e537c2，claim 3.7／3.8；
> * `3dc2f7c9`：接線 3.3～3.6；
> * `8bebcf9d`、`a6003d06`：FShow_Audit。

## 檔案（誰的）

| 檔 | 擁有者 | 內容 |
|---|---|---|
| `HT9011UC_Cpp_V3.33.906.0/TrayEditForm.cpp`、`.h` | St02 | golden uTrayEditForm 逐行翻譯。VCL 的 mtLoaderBuffer（TTMyTray）改成一般狀態（格子顏色、數字、色表）送給頁面；偏離 golden 的地方都在檔頭列出 |
| `HT9011UC_Cpp_V3.33.906.0/WebTrayEdit.cpp` | St02 | WS 指令 `act.trayEdit`（op：`state`、`rightClick`、`contactEditTray`、`select`、`update`、`cancel`、`noSave`、`fill`、`binCount`、`snapshot`），回傳 executed 或 refused 與 guard |
| `web/page/HW.TrayEdit.html`、`web/page/ht9045_trayedit.js` | St02 | 網頁視窗 |
| `WebPageTable.cpp:463`、`:630` | St01（claim 3.7／3.8，e6e537c2） | 操作員從 HMI 關窗 → `W906_TrayEditWindowClosedHook` → `WindowClosed` |
| `acatchtray_shims.cpp` | 筆電（claim） | `TTrayEditForm_Facade::Close()` → `W906_TrayEditCloseHook` → 這支表單的 FormClose |
| `tools/wb_serve.cpp` | 筆電（claim） | 指令分派多一行：`act.trayEdit` → `W906_TrayEditCmd` |
| `web/page/ht9045_sortct_wire.js` | 筆電（claim） | cSortCT 盤面右鍵 → `act.trayEdit {op:'rightClick', panel}`（golden cSortCT 的入口） |
| Contact 頁（St01，B8 CT-3） | St01 | Edit Tray 鈕 → `act.trayEdit {op:'contactEditTray'}` → TrayEditForm.cpp `ContactEditTray()`＝golden cContact.cpp:16853-16859（CC_ASE_KaohSiung → `EditTray(MMTrayY, 0)`，其他 → `EditTray(MMTrayY, 1)`）。golden 的鈕 DFM 隱藏，只有 DoStepContactLoadDevice（:16002）會顯示；顯示由頁面照 golden 做。20260930 St02 加的 op |
| `CMakeLists.txt` | 筆電（claim） | wb_serve 的來源清單加上 TrayEditForm.cpp、WebTrayEdit.cpp |

## 行為（跟 golden 不同的地方）

* golden 用 ShowModal；wb_serve 的 tick 執行緒不能卡住，所以改成：
  * 頁面表開窗（`W906_FormProgramShow`）；
  * 表單一直開著，直到 Update、Cancel、fill 或 Close。
* 操作員直接關掉網頁視窗＝golden 的 Cancel（SpeedButton2Click :614-617，E2 第 2 點）。
* ShowMyMessage 排隊，等 `act.trayEdit` 放開 FormLock 之後才顯示（E2 第 1 點）。
* SaveJPG：wb_serve 沒有畫面可以截，改成在同一個資料夾、用同一個檔名，存一份盤面的文字圖。這是預設，Steven 還沒回覆。
  * 它前面那個 MySleep(100)（FormClose :238）拿掉了，原本只是等截圖完成。

## fShow 的規則（St01 20260930 04:58 同意）

* `TrayEditForm->fShow` 是 **TrayEditForm 自己的 modal 狀態，由 St02 的門面照 golden 維護**：
  * 只在 FormShow :120 設（golden :76）；
  * 只在 FormClose :221 清（golden :239），HMI 關窗也走這條路。
* TrayEditForm.cpp 裡的讀取點保持直接讀，同一行寫了原因。原本 12 個；20260930 加 ContactEditTray 的守門後是 13 個，`tools/fshow_audit_baseline.json` 記成 `"TrayEditForm.cpp": 13`。
* TrayEditForm.cpp **以外**要讀，一律包 `W906_FShow` 或 `W906_FormShowing`。例如 WebStart.cpp:1626 = golden main.cpp:4716。
* golden note.cpp:3451（每秒 BringToFront）移植樹沒有對應，不用做。

## 還沒做

* **不要在表單程式（例如 forms/fContact.cpp）直接呼叫 `EditTray`**：TrayEditForm.cpp 只編進 wb_serve，ht9045_forms 的測試會連結失敗。要開窗一律走 `act.trayEdit` 的 op（在 WebTrayEdit.cpp 加）。
* 其他入口還沒接：golden asendic_Auto／asendic_Loader 也會呼叫 EditTray（移植樹 asendic_Auto.cpp:2833 等）。
  * 這兩支檔用的還是 TU 裡的空替身：`W7L1A_EditTray`（asendic_Auto.cpp:328）、`W7L1L_EditTray`（asendic_Loader.cpp:279）。
  * 真的 `EditTray` 已經在 TrayEditForm.cpp。要接的話，把這兩個替身退役（是筆電的檔，要先認領）。
* 沒有上機驗過。上機要看：
  * 右鍵開窗、Update、Cancel、直接關窗，這四種都要回到主畫面；
  * 重開 wb_serve 之後，不能殘留開著的窗。
