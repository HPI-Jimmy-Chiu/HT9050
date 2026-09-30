# 機台端 patch 整合 第二輪（cpp 0045～0046、web 0034～0044、tools 0001～0109）

> 接在第一輪（`docs/MACHINE_PATCHES_20260930.md`，cpp 0022～0044、web 0012～0033＝第六批）之後。
> 裁決照舊：RULINGS_20260930 第 11 條「一切按照機台建議」，衝突以機台為準；TOKEN-OFF（0018）、TEMP-DOORS（0014／0015）照舊不收。
> 分支 `v906/jimmy-mach1001`（worktree `D:\HT9045\.claude\worktrees\mach1001`），疊在第七批 `v906/jimmy-b7` 上。

## 0. 來源與做法

| 項目 | 內容 |
|---|---|
| 來源 | GitHub `machine/integ-ioweb`：0930 21:09 `10474c9` cpp 0045～0046＋web 0034（VACUNIT-1203）；21:13 `5bde367` web 0035～0044（NOOVERLAP-A／B／C，EastSun 當天喊暫停，收的是已完成的頁面）；tools 0001～0109（HTDESIGNER，到 1001 00:10 `8042541`） |
| C++／web 的套法 | 同第六批：機台的 blob 不在筆電，`git am -3` 找不到基底 ⇒ 延伸前一輪留下的**機台歷史重建鏈**（C++ `6fc40277`＝機台 C++ 0001～0044、web `76e99d73`＝web 0001～0033）。機台 patch 的內容行是 CRLF、repo 存 LF ⇒ 先轉 LF，用暫時 index＋`commit-tree` 接在鏈上（不 checkout，作者／日期／說明照 patch）：C++ `ec34f75e`（0045）→ `e5dd8d6c`（0046）；web `79f9123d`（0034）…`dee34e8d`（0044）。再逐顆 cherry-pick 到本分支＝真正的三方合併 |
| tools 的套法 | 109 顆只碰 `tools/vscode-htdesigner/`（筆電樹裡原本沒有這個資料夾；位置跟機台一樣在移植樹 `HT9011UC_Cpp_V3.33.906.0/tools/` 底下），轉 LF 後 `git am --directory=HT9011UC_Cpp_V3.33.906.0` 照編號套 |
| 核對 | 13 顆 C++／web 逐顆比對「加的行、刪的行」（多重集合，逐檔）＝機台 patch：**12 顆完全相同**；0045 只差下面手動合的 4 個檔 |

## 1. 衝突（4 處，0045）

| 檔 | 兩邊各做了什麼 | 怎麼合 |
|---|---|---|
| `VacuumUnit/VacuumUnit.cpp` | 筆電 INBOX 115 B（`a5d6fb67`，RULINGS 第 4 條「全部開」）已把 `SetIOTableByECAT_VC8_Sucker`／`SetSuckISABase`／`VaccumCopyFormSuck`／`VaccumCopyToSuck` 的 golden 本體打開、替身退役；機台 0045 用另一種寫法打開前三個：包在 `MachineType.h` 的 `W906_VC8_SUCKER_REMAP` 底下，外加 VC8 吸嘴閘（`VacuumUnit/Vc8Route.h`） | **照機台**：前三個用機台版本。原因不只是裁決——115 B 那版**沒有閘**，而機台量到 `BTestSuck` 在 0x50／0x51 的 DO 16～19 跟上料／Auto1 的氣缸（`C_Load_Up`、`C_LoaderDrawerLock`、`C_Auto1_Up`、`C_Auto1DrawerLock`）同一條通道：吸嘴列只要有一列被改成 Enable=1，115 B 那版就會打到氣缸線圈。`VaccumCopyToSuck` 機台沒動 ⇒ 保留 115 B 的（golden 裡只有一段被 `/* */` 註解掉的程式呼叫它，沒有活的呼叫點；`test_i115b_vacuum` 的 V3 靠它）。檔頭那句同一行補註。跟機台版本只差這兩處 |
| `WebBridge/WebBridgeServer.cpp` | 免 token 的指令清單：筆電 INBOX 119 加了 `dialog.notifyAck`；機台加了 `vacuum.get`／`vacuum.open`／`vacuum.close`（Vacuum Unit 頁每秒讀一次） | 兩個都留；機台那邊當上下文帶進來的 TOKEN-OFF 兩行照第六批**不收** |
| `tools/wb_serve.cpp`（2 行） | 操作記錄：筆電的 `[STREAM]`（`W906_OpStreamNote`）；機台的 `VAC`（`W906_OpVacuumNote`，每一次 ECAT-VC8 寫入） | 同一行並存，插在第一個 `//` 之前（行號不動） |
| `tests/CMakeLists.txt` | 兩邊都在檔尾加測試（筆電 SysinitBoot／Stream2E_MvMotor 等；機台 `VacuumVc8`） | 都留，沒有重名 |

另外 `JsonBridge/IoBtnPanelClick.cpp` 檔頭 (h)（第七批剛更正過）同一行補上 0045 的 `W906_Vc8SuckerGate`。

## 2. 這一輪收進來的東西（白話）

| patch | 內容 |
|---|---|
| cpp 0045 VACUNIT-1203 | Vacuum Unit 視窗在 PCIE-1203 上接好（ECAT-VC8 路徑，**一律安全失敗**）：讀寫前確認 ring 1 那一站存在、是 ECAT-VC8、在 OP 狀態、位址在卡片對應表；寫還要 VacuUnitType=1、沒有在運轉。吸嘴照 golden 交給 VC8（`W906_VC8_SUCKER_REMAP`，EastSun R3），144 個吸嘴別名只有確認是 VC8 才送。**這台 ring 上目前沒有 VC8**：全部 999.0／Error5、按鈕鎖住並寫原因。新 ctest `VacuumVc8` |
| cpp 0046 | VC8 拒絕訊息「每個物件每分鐘一行」的表 32 → 256 格（原本 48 個物件把表擠爆，變成每秒都印） |
| web 0034 | Vacuum Unit 頁：每秒輪詢、顯示 C++ 算的值、只有 available 的鈕才解鎖、寫之前確認框 |
| web 0035～0044 NOOVERLAP | EastSun「重疊是不被允許的」：只動版面（先修 golden 執行時會藏／會對齊卻被畫在設計位置的元件）。已完成：Teach、Contact、Contact Force、CC-Link、Offset、Configure、Handler System、Speed、Security、Motor Test、Home、Observer、Setup、Ld/ULd、Tray Form、主畫面與小狀態窗等；**還沒做**：IO、Omron、Yield Monitor、Auto Clean、Barcode、Temp Set、Tray Assign、Vacuum Unit |
| tools 0001～0109 HTDESIGNER | VS Code 的「HTML 視覺設計工具」外掛（0.111），不進 wb_serve 建置；EastSun：「編譯沒問題就能 commit and push」 |

## 3. INBOX 132（IO 畫面吸嘴沒作用）的結論更正

筆電 0930 23:5x 查的時候還沒讀到 0045，寫了 A（改 IO 表 Enable=1）／B（網頁不擋）兩條路——**兩條都作廢**：B 會打到氣缸線圈，A 也不是正確的路。真正的原因是**這台 ring 1 上目前沒有 ECAT-VC8 真空模組**；0045 已讓 IO 頁與 Vacuum Unit 頁把原因寫出來。GitHub 第 91 包 README 已在 1001 00:3x 更正（`46fb729`）。剩下的問題只有一個：VC8 實際有沒有裝、有沒有接上 ring 1。

## 4. gate

（見本批 commit 與 NIGHT_REPORT §5）
