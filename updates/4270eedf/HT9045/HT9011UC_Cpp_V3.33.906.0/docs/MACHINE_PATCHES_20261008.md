# 機台 patch 收件紀錄 20261008

> 接 `MACHINE_PATCHES_20261007.md`（第 21 節）。來源照舊是 GitHub `HPI-Jimmy-Chiu/HT9050` 分支 `machine/integ-ioweb`；衝突以機台為準（RULINGS_20260930 第 11 條）；鏈尾＝機台歷史重建鏈（`mach_chain.py`）的最後一顆。

## 1. 1007 23:41 收到的 cpp 0309／0310 上 main（第 99 批）

- **cpp 0309 TRAYSAFE9050**（EastSun 1007「Loader 不會將 Tray 移動到吸取位置／Empty 不會將空 Tray 補到 Auto 1」→「寫 HT9050 自己的判斷：Loader 在最左邊，Empty 在最右邊，Auto 在中間」）：golden 的 Tray Arm 教導防撞線 (Empty+Color)/2+6500（這台＝6866）是 9046 的版面，HT9050 往 Empty（7532）與 Loader（191213）的移動全被拒（22:23:45「TrayArm moves 7532 to the left error」）⇒ `Motor/mymotor.cpp` 改照 HT9050 站序；`tests/test_flow9050_tray.cpp` 新 [TS] 區（5 項）。
- **cpp 0310 TRAYFLOW9050**（EastSun「Empty 不會將空 Tray 補到 Auto 1」→「我現在修」）：`acatchtray.cpp` DoCatchTray case 500／600 在 Type_HT9050 改走 `W906_CatchFromEmpty9050`（檔尾：Empty 疊數不明時往上探到有盤、算出 L+1 盤；最深層已有盤報 MES1022；只有抽屜感測報 MES1024；SIM＝5），DoCatchFromEmpty_9050 拿最上面那盤並把數量減一；400／1000／2000 也重設 9050 子游標；`cmydef.cpp` Auto1～3 層數從 0 起算（原本 -1 讓第一盤高一格）；`asendic_Loader.cpp` Loader 數到 0 改報 MES0920（原本 1→800→900→1 靜默打轉）；`csystem.cpp` HOME／START 也重設 9050 Tray 子游標。
- **合併衝突（1 處，以機台為準）**：0310 跟筆電第 97 批的 `c755aa7a` 改到 `tests/test_fp9050_index.cpp` 同一行（FP9050_Pure 的 [F2] 普查讓 FLOWTRACE 讀 `iTestYFinePitchTask`）——筆電版只放行 `W906_FlowTraceTick` 函式內，機台版放行整支 `csystem.cpp`。照機台版（`git checkout --theirs`），筆電那兩行函式範圍追蹤一起退回，檔案＝機台的版本。
- 收進**第 99 批**（b18 `v906/jimmy-b99`：main `90348048`＋0309＋0310（cherry-pick）＋St02 W-149 MR 4（!319，只改測試）＋NB2-1 LINK-22 文件（!318），`868caf4c`）。
- 第 99 批上 main `cfd361b8`＝**第 182 包**（GitHub `14c39ef7`）；gate b99a：兩組態 31 分鐘：只有固定失敗（兩組態都是 4 支；模擬版 W-149 那 3 支已過）。
- **鏈尾**：C++ `b0b3c5ab`（cpp 0308）→ **`c79de0ba`**（cpp 0310，第 99 批）。下一次從 **cpp 0311／web 0140／tools 0171** 開始。

## 2. 1007 23:57～1008 00:26 收到的 cpp 0311～0313 上 main（第 100 批）

- **cpp 0311 UPCYLOFF**（EastSun 1007：C_Load_Up／C_Empty_Up／C_Auto1～3_Up 在 HOME 一開始送一次 Off）：HT9050 這幾個是雙線圈閥（C_xxx_Up＋C_xxx_UpOff），`Cylinder[]` 只管 On 線圈；`uhome.cpp` HOME 第 1 步呼叫 `g_W906UpCylOffHook(0)`（On 線圈關、Off 線圈通電），之後每一輪 `(1)` 在 500 ms 後放掉 Off 線圈；掛勾在 `JsonBridge/IoBtnPanelClick.cpp` 檔尾（只有 wb_serve 會掛，ctest 是 0）。
- **cpp 0312 TRAYNOPICKUP**（EastSun 1008「TrayArm moves 6800 out of the HT9050 order」→「用 HT9050 旗標，Tray Arm 位置不要加 6800」）：`cinitial.cpp` HT9050 的 Tray Arm 各站位置＝教導點＋Offset（不再加 Load／Empty／Auto 的 iPickUp），iXTraySafty＝Empty 教導點 -2000；Color 照 Empty 的規則（`Motor/mymotor.cpp`）。
- **cpp 0313 FIXFULL9050**（1008 00:19 `WAR240198 MOutArmX Motor Alarm`）：Out Arm case 300 讓位走 `MoveOutArmXY_ToFix_Tray_Full`，golden X＝`PSoftLimitN + iOutArmXBase*2000 + 100`，這台 SoftLimitN＝-999999 ⇒ 目標約 -999899；HT9050 改用 Auto3 X/Y（`aoutarm.cpp`）。⇒ 筆電派 MainNB-GPT 普查還有哪些地方拿軟體極限當目標（W-164，HT9050 48 軸有 45 軸是 ±999999）。
- **合併衝突（1 處）**：0311 跟 St02 W-155 A（!315，第 94 批）都在 `JsonBridge/IoBtnPanelClick.cpp` 檔尾加了一段（機台還沒整合第 177 包，所以它的版本沒有 St02 那段）——兩段都留：main 的在前、機台的接在後（`resolve_single_append.py` 驗過兩段各自逐字出自兩邊）。
- cherry-pick 時 git 在背景自動整理 packfile（00:37 新 pack＋multi-pack-index），印了幾行「packfile index unavailable」；三顆 commit 與 21 個新物件都查過存在，沒有東西壞。
- 收進**第 100 批**（b19 `v906/jimmy-b100`：main `5ee41624`＋0311／0312／0313（cherry-pick）＋ST-GPT !321（技能文件），`295b897d`）。
- 第 100 批上 main `77887765`＝**第 183 包**（GitHub `ffbeeee2`）；gate b100a：兩組態 30 分鐘：只有固定失敗（兩組態都是 4 支）。
- **鏈尾**：C++ `c79de0ba`（cpp 0310）→ **`b8dfbe89`**（cpp 0313，第 100 批）。00:47 又到 **cpp 0314 CCDYHOME9050**（HT9050 的 CCD Y（M108）「在原點」改成「已回原點＋位置接近 0」，不看原點燈——00:31 燈在開關邊緣熄掉，910 的 MoveIndexZ 互鎖（剛好 0 且燈亮）擋掉每一次 Index Z 下降；`acarry.cpp`、`atester_FinePitch.cpp`＋測試）⇒ 排**第 101 批**；web 0140／tools 0171 還沒有。

## 3. 00:47 收到的 cpp 0314 上 main（第 101 批）

- **cpp 0314 CCDYHOME9050**（EastSun 1008「CCD Y home check stops my Index Z」→「用 HT9050 旗標分開；已回原點＋位置接近 0」）：00:31 MCCDY 回到 0，但原點燈在開關邊緣（DS402 回原點）熄掉，910 的 MoveIndexZ 互鎖（剛好 0 且燈亮）擋掉每一次 Index Z 下降；HT9050 改成「已回原點＋位置接近 0」，不看燈（`atester_FinePitch.cpp` MoveIndexZ、`acarry.cpp`；測試 `test_fp9050_index.cpp` [F5]、`test_ht9050_shtsafe.cpp`）。
- 收進**第 101 批**（b18 `v906/jimmy-b101`：main `e5078330`＋0314，乾淨 cherry-pick，`979e2947`）。
- 第 101 批上 main `c13c0d3e`＝**第 184 包**（GitHub `78e796d4`）；gate b101a：兩組態 30 分鐘：只有固定失敗（兩組態都是 4 支）。
- **鏈尾**：C++ `b8dfbe89`（cpp 0313）→ **`90e67dde`**（cpp 0314，第 101 批）。下一次從 **cpp 0315／web 0140／tools 0171** 開始。00:56／01:14 又到 cpp 0315 TORQUEBYPASS9050（HT9050 先跳過 Index 扭力讀取）＋0316 PROBESENSOR9050（Loader 層數探測看抽屜感測）⇒ 第 102 批（b19，gate b102a 跑中）。

## 4. 00:56／01:14 收到的 cpp 0315／0316 上 main（第 102 批）；快照 01:14

- **cpp 0315 TORQUEBYPASS9050**（EastSun 1008 WAR0361「Index Z1 torque not read in 5 s, E-044」→「先跳過，扭力讀取還沒驗證」）：`atester_FinePitch.cpp` FP case 12110（910 :2415）在 HT9050 且 `g_W906Ht9050TorqueReadBypass`（檔尾，**預設 true**）時不讀、不比扭力，直接 InitWriteAndCheckMotorTorqueTask 再到 12200（扭力上限照樣寫回）；`tests/test_fp9050_index.cpp` 新 [F6]「bypass 開 → 12200、沒有 WAR0361」，原本的 F6／F8 扭力讀取檢查改在 bypass 關的狀態下跑。⚠ 標記是 `AI(W906-TORQUEBYPASS9050)`、不是 `AI(W906-TEMP-*)`，而且做成旗標、兩種狀態都有測試 ⇒ 照第 10 條的判準收進 main；**等扭力讀取驗證過，請機台把預設改回 false 再推**（TO_ES02 同時間那列）。
- **cpp 0316 PROBESENSOR9050**（EastSun 1008 01:03 MES0924 DoLoad_9050_1110：層 2～4 的 SnLoaderTrayHasTray 都亮、到上限 5）：`asendic_Loader.cpp` DoLoad_9050 case 1100／1115／1125 改問 `W906_LoaderProbeSeesTray`（檔尾）——HT9050＝`Sen[SnLoaderDrawerHasTray].IsOff()`（Off＝有盤），其他機型照 910 的 `Sen[SnLoaderTrayHasTray].IsOn()`；`tests/test_flow9050_tray.cpp`＋14 行。同時機台把 `teach.ini` 的 `setEditTZ9050LoaderProbeLimit` 5→20（快照 01:14，main `f84de6c7`，已叮嚀）。
- 收進**第 102 批**（b19 `v906/jimmy-b102`：main `c13c0d3e`＋0315＋0316（cherry-pick），`0d8b290c`）。
- 第 102 批上 main `7a40ab61`＝**第 185 包**（GitHub `c3f47871`）；gate b102a：兩組態 30 分鐘：只有固定失敗（兩組態都是 4 支）。
- **鏈尾**：C++ `90e67dde`（cpp 0314）→ **`8257e310`**（cpp 0316，第 102 批）。下一次從 **cpp 0317／web 0140／tools 0171** 開始。02:00 又到 cpp 0317 DUALCOIL9050（HT9050 五口三位氣缸雙線圈）⇒ 第 103 批（b18，gate b103a 跑中）。

## 5. 02:00 收到的 cpp 0317 上 main（第 103 批）

- **cpp 0317 DUALCOIL9050**（EastSun 1008「`Cylinder[C_LoaderEdgePush].Pop()` ＝ `Cylinder[C_LoaderEdgePush].Off()` 且 `Cylinder[C_LoaderEdgePushOff].On()`，Push 反過來」）：HT9050 的五口三位氣缸 Pop／Off 時通電「<名稱>Off」那一個線圈、Push／On 時放掉。`mycylin.cpp` 的 OnSwitch／OffSwitch 同一行呼叫 `g_W906PairedCoilHook`（檔尾，預設 0，所以只編 mycylin.cpp 的測試與其他機型照 golden）；新檔 `JsonBridge/W906DualCoil9050.cpp`（直接編進 wb_serve，靜態註冊）只在 HT9050、IO 表有那一列 Off、IOType 是 Cylinder、Enable=1、而且不是引擎本身的 Cylinder[] 時才寫（機台表 27 對）；IO 頁的手動點擊照舊（golden 單點寫）。`JsonBridge/IoBtnPanelClick.cpp` 0311 UPCYLOFF 的 Off 線圈改成保持（不再 500 ms 後放掉）。新測試 `test_dualcoil9050.cpp`（HT9050_DualCoil 26/26）。
- **合併衝突（1 處）**：`tests/CMakeLists.txt` 檔尾——main 的 St02 W-150 等區塊在前、機台的 HT9050_DualCoil 區塊接在後（`resolve_single_append.py`；add_executable／add_test 名稱沒有重複）。
- 收進**第 103 批**（b18 `v906/jimmy-b103`：main `7a40ab61`＋0317（cherry-pick），`14465c4a`）。
- 第 103 批上 main `75203368`＝**第 186 包**（GitHub `3fee2657`）；gate b103a：兩組態 31 分鐘：只有固定失敗（兩組態都是 4 支；新測試 HT9050_DualCoil 過）。
- **鏈尾**：C++ `8257e310`（cpp 0316）→ **`c29f9a1c`**（cpp 0317，第 103 批）。下一次從 **cpp 0318／web 0140／tools 0171** 開始。
- 筆電工具：每一包的 helper 改由 `derive_next_pkg.py <包號> <BASE> <TIP> <spec>` 產生（同一件事手寫第五次、其中一次寫錯 gate log 名之後改的；第一版自己也抓錯了 mkpkg 的舊值，靠它印出的「literal 舊→新」當場發現，已加交叉檢查）。

## 6. 08:33 收到：cpp 0318（WORKLOG）、tools 0171（HTDESIGNER）；快照 08:33（08:5x）

- **cpp 0318＝WORKLOG §3 補三列**（只有文件）：收進 main `87dd2299`（作者／日期照 patch）。cherry-pick 在 `docs/WORKLOG_MACHINE.md` 衝突：main 少機台的 tools 0170 列與「167～172」包那一列（PKG-* 照規矩不 cherry-pick，所以那列從沒進 main）→ 照 RULINGS_20260930 第 11 條以機台為準，保留機台的 3 列（`resolve_single_append.py`：ours 0 行＋theirs 3 行）。只動文件，不出包。
- **tools 0171＝HTDESIGNER：方案總管的 Debug／Release 鈕跟 F5 連動**（EastSun 1008「我這邊按鈕都不能按」→「連動F5 release 和debug」；基底 0.234.0、版本號不變；只動 `tools/vscode-htdesigner` 的 extension.js／CHANGELOG／兩支測試）：照 tools 0164／0165 的做法**交 ES02 併進下一版 htdesigner**（ES02 是那條線的主人；main 仍是 0.234.0，ES02 的 0.235～0.250 照 Jimmy #147＝A 等 !310 轉 Ready 一起合）。⚠ 跟 !310（F5 與啟動按鈕共用模擬及 Debug／Release 組態，Draft）是同一件事，合的時候行為以機台這顆為準。TO_ES02 §4、W-171。tools 不接鏈。
- **快照 08:33**（main `3cbca261`）：`teach.ini` Tray Z 間距 `setEditTZ9050LoaderPitch／EmptyPitch／Auto1Pitch／Auto2Pitch／Auto3Pitch` 1900 → **2300**（真的參數變更，不是 `.bak`）⇒ CHAT_JIMMY 已叮嚀全體先同步再驗證（RULINGS_20261005 第 6 條）。其餘只有 log 指標與 lastdata。
- **鏈尾**：C++ `c29f9a1c`（cpp 0317）→ **`1907c79d`**（cpp 0318）。下一次從 **cpp 0319／web 0140／tools 0172** 開始。

## 7. 08:52 收到：cpp 0319 TRAYZTHICK9050；快照 08:51（09:1x）

- **cpp 0319＝TRAYZTHICK9050**（EastSun 1008「第一張圖紅框處的設定值請使用第二張圖黃框處的資訊帶入」：Teach 的 9050 Tray Z 間距用 Tray Form 的厚度）：`cinitial.cpp` `W906_TrayZ9050` 同一行改呼叫檔尾新的 `W906_TrayZ9050Pitch`＝該站 Tray Form 厚度（`UserDefForm_File[type].ZDepth`）×100（golden dbTrayThick 的換算）；厚度 0 或型別不對照用教導值；`csystem.cpp` DoAllProcess（HT9050 才做）把用到的值寫回 `Tech.iTrayZ9050Pitch[]` 給 Teach 頁顯示；測試 `test_flow9050_tray.cpp` [TT] 11 項。機台說明：EastSun 08:46 自己改的 `asendic_Loader.cpp`（DoLoad_9050 case 1100 的 `C_Load_Up.Push()`）**不在這顆**，留在他的樹。
- **收進第 108 批**（b19 `v906/jimmy-b108`＝main `2b672667`＋這顆，cherry-pick 乾淨，tip `4857dd6c`），gate b108a 跑中；綠了推 main、出第 191 包。
- **快照 08:51**（main `2b672667`）：工單 `IOWEB_TEST_R003` 的 `Tray.Data` 厚度 `Think` 7.62 → **23**（配 0319 後間距＝2300，跟 08:33 Teach 改的 2300 一致）；`Gerneral.ini` 只有 `Program Close` 0→1（程式關閉狀態）。工單真的變了 ⇒ CHAT_JIMMY 再叮嚀一次。
- **鏈尾**：C++ `1907c79d`（cpp 0318）→ **`7f1b12a0`**（cpp 0319）。下一次從 **cpp 0320／web 0140／tools 0172** 開始。
- **09:4x 上 main**：第 108 批 `21aa8a46`＝第 191 包（GitHub `e6d7e53d`）；gate b108a：兩組態 33 分鐘：只有固定失敗（兩組態都是 4 支，471 支測試；出貨的 EcatAlarmScanLive 有一次沒跑起來＝執行檔被鎖，單獨重跑通過）。

## 8. 10:26 收到：cpp 0320（WORKLOG）、cpp 0321 BOOTCLEAR9050；快照 10:26（10:4x）

- **cpp 0320**：WORKLOG 第 200 列補一句（Loader 間距＝Tray Form Type1 厚度 23.000 mm＝2300，EastSun「keep it」），只有文件。
- **cpp 0321＝BOOTCLEAR9050**（EastSun 1008「每次開機，如果機台有料，就詢問是否清料：是就清，否就照舊」）：`cinitial.cpp` 檔尾 `W906_BootAskClearMachine9050`，wb_serve 裝好是／否網頁對話框之後的第一圈 DoAllProcess 問一次（開機序列本身問不了）；只有 HT9050、只在閒置時；「有料」＝SaveMachineRecord 自己的清單或 HasICUnderMachine()；是＝golden InitialMachine＋Loader／Empty 層數 −1（重新探層）＋Auto1～3 計數 0＋SaveMachineRecord；否＝照舊；回 3（已有框開著）＝下一圈再問。起因：重開後 golden 會把 Tray Y「有盤」還原，但 `iLoaderLayerCount_9050` 從 −1 起算，DoLoad_9050 case 1 把 Loader Z 停在 Down[−1]＝22300 而不探層（10:14 操作記錄）。測試 `test_flow9050_tray.cpp` [BC] 7 項。機台說明：EastSun 09:48／09:56 自己改的 `acatchtray.cpp`／`asendic_Loader.cpp` 探層**不在這顆**。
- **收法**：第 110 批（b19）10:25 已開跑 gate（!329＋!306＋ES02 HTDESIGNER），只建了 4 分鐘——**停掉、把 0320／0321 疊上去（cherry-pick 乾淨，tip `a21be254`）、重開 gate**，機台這兩顆不用再等一整批。
- **快照 10:26**（main `b031066f`）：工單 `IOWEB_TEST_R003` 的 `Binasgn.Data` 改成**只有 Bin1→Auto1**，原本送到 Auto2、Fix1～3 的對應都拿掉——就是 W-101 問的那件事，EastSun 改工單解決了。工單真的變了 ⇒ CHAT_JIMMY 已叮嚀。
- **鏈尾**：C++ `7f1b12a0`（cpp 0319）→ **`adb3a720`**（cpp 0321）。下一次從 **cpp 0322／web 0140／tools 0172** 開始。
- **11:1x 更正：cpp 0321 退回，沒進第 110 批**。gate b110a（含 0321）出貨綠、**模擬紅**：`Flow9050_Tray` [BC] 2 項（測試第 1102、1107 行）。只拿 main（第 109 批）＋0320／0321 在模擬組態單獨編這支測試，同樣 2 項紅 ⇒ 是 0321 自己的新測試在模擬組態不過，不是同批的 MR（出貨組態會過；機台只跑過出貨）。原因（讀碼）：選「是」跑 golden `InitialMachine()`，它清 Tray Y 與類型是 Auto 的 Auto 盤，**不清 Tray Z／手臂吸嘴／Shuttle／Index**（`HasICUnderMachine()` 會看這些）；同一支測試前面幾段在模擬組態留下了料，所以清完還判定「有料」、框又跳一次。處置：第 110 批改成 MR 三件＋cpp 0320＋Ifor01 !330（原第 111 批併進來，tip `0ebccfc5`）重跑 gate；0321 經 EastSun 退回機台，在模擬組態修好再送（W-173）。鏈尾照舊 `adb3a720`（0321 已接鏈），下一次從 cpp 0322 開始；0321 修好後的版本照新編號收。

## 9. 10:56 收到：cpp 0322（BOOTCLEAR9050 修正）——跟 0321 一起先不收（11:2x）

- **cpp 0322**：開機詢問與 TRAYZTHICK 的 Teach 頁間距同步，原本掛在 DoAllProcess（只有 SystemStart 時才跑）⇒ 開機根本沒問（EastSun「沒出現視窗詢問」）；兩個呼叫搬到 MainProc（閒置也跑）；`test_mainproc_guard.cpp` PART 3 用真的 MainProc 驗。
- **先不收**：0322 的 `csystem.cpp` 改動接在 0321 那一行上（0321 沒進 main 就套不上），而且沒動到模擬組態紅的 `Flow9050_Tray` [BC]。⇒ 0321＋0322 一起等機台在模擬組態修好、整組再送（W-173 已補）。⚠ 0322 也把已在 main 的 0319 間距同步從 DoAllProcess 搬到 MainProc——main 上這個同步目前仍在 DoAllProcess（只在運轉中跑），等整組收進來才會一起改。
- **鏈尾**：C++ `adb3a720`（cpp 0321）→ **`dc3d4247`**（cpp 0322）。下一次從 **cpp 0323／web 0140／tools 0172** 開始。
- **11:5x**：cpp 0320（WORKLOG）隨第 110 批進 main（`dfbd4696`＝第 193 包，GitHub `9689472f`）；0321／0322 仍等機台在模擬組態修好整組再送（W-173），鏈尾照舊 `dc3d4247`。

## 10. 11:31／11:45 收到：cpp 0323（WORKLOG 203）、cpp 0324 FLOWTRACE2（12:5x）

- **cpp 0323**：WORKLOG 第 203 列——EastSun 在機台上改 `IO_Table.csv`：Load／Empty／Auto1～3 五個 `_UpOff_On` 從 Port 16 Bit 0（＝`_Up_On`）改成 Port 17 Bit 1（＝`_Up_Off`），只差 10 bytes；這是機台設定，程式沒改（快照 11:45 帶上來）。
- **cpp 0324＝FLOWTRACE2**（EastSun 1008「InArmSuckUse[2][4] 目前的狀態」）：操作記錄的 FLOW 行尾端多記入料手臂取料（`iPickFromLoadStageTask`）／Tray Arm（`CatchTrayTask`）／Empty（`iCatchFromEmpty9050Task`）的步驟、Loader／Empty／Auto1～3 層數、`InArmSuckUse[2][4]` 與入料手臂吸嘴有沒有 IC；只寫記錄（`tools/wb_serve.cpp` 寫 FLOW 行，沒人解析）。測試 `Flow9050_Tray` [FT]。
- **收法**：第 111 批（b18，St02 !331）11:50 開的 gate 跑了約 15 分鐘——**停掉、把 0323／0324 疊上去（tip `c73779d5`）、重開**，機台這兩顆不用多等一批。
- **衝突**：①`docs/WORKLOG_MACHINE.md` 照機台版本（第 203 列；機台那邊第 201／202 列——就是還沒進 main 的 0321／0322——也一起帶進來了，那兩列記的是機台的工作，程式要等 W-173）。②`tests/test_flow9050_tray.cpp`：機台把 [FT] 接在 0321 的 [BC] `TrayBootClearAsk()` 結尾，main 沒有 [BC] ⇒ 只取 [FT]、改從 [TT] `TrayZThicknessPitch()` 的結尾呼叫（`resolve_ft_without_bc.py`；0324 的 commit 訊息有註明）。`csystem.cpp` 乾淨套上。
- **鏈尾**：C++ `dc3d4247`（cpp 0322）→ `98fb3564`（cpp 0323）→ **`60ffcac2`**（cpp 0324）。下一次從 **cpp 0325／web 0140／tools 0172** 開始。0321／0322 仍等 W-173。
- **GitLab**：12:0x～12:50 連不上：公司的反向代理（openresty）把 https 請求 301 導到同一網址加 :443，形成無限轉址，網頁、fetch、push 全部失敗；12:50 自己恢復（不是筆電這邊的設定）。這段時間第 111 批 gate 照跑，恢復後才推 main 與第 194 包。
- **12:5x**：cpp 0323／0324 隨第 111 批進 main（`214f680d`＝第 194 包，GitHub `f13d767d`；gate b111a：兩組態 43 分鐘：只有固定失敗（兩組態都是 4 支，476 支測試）；出貨組態的 Jam_Rules 在 gate 中失敗一次（當時負載高），單獨重跑通過、實檔未變）。快照 11:45 鏡像到 main `ec96bfa2`（`IO_Table.csv` 五列；ini／lastdata 是執行中狀態），CHAT_JIMMY 已叮嚀。

## 11. 13:22 收到：cpp 0325（WORKLOG 205）；快照 13:21（13:3x）

- **cpp 0325**：WORKLOG 第 205 列——EastSun「In Arm 的吸嘴是由三個 Suck 去共同吸取才算吸取完成（IC 尺寸太大）」「吸取後的流程動作好像沒正確判斷」。機台查證：HT9050 的 In Arm 綁成 A／C／E／G 一組（`mykitsuck.cpp` `W906_InitVc4Groups`，`AI(W906-VC4-HT9050)`），「吸到」＝組內每一顆 Enable=1 的真空都亮；IO 表 G 也是 Enable=1，只吸 A／C／E 時永遠不成立 ⇒ 重試用完 ⇒ MES0101（11:17）。處置：機台 `IO_Table.csv` 的 `InArmSuckG`／`_On`／`_Off` 改成 Enable 0（**只改機台設定，程式沒改**）。
- **收法**：只有文件 ⇒ 照 cpp 0318 的做法直接 cherry-pick 到 main（乾淨），main `1956e18b`，不等 gate。
- **快照 13:21** 鏡像到 main `0b55e181`：`IO_Table.csv` 三列 Enable 1→0；ini／lastdata／machinerecord 是執行中狀態。CHAT_JIMMY 已叮嚀。
- **鏈尾**：C++ `60ffcac2`（cpp 0324）→ **`068df856`**（cpp 0325）。下一次從 **cpp 0326／web 0140／tools 0172** 開始。0321／0322 仍等 W-173。

## 12. 15:05 收到：cpp 0326（WORKLOG 206）；快照 15:05（15:1x）

- **cpp 0326**：WORKLOG 第 206 列——EastSun「可是剛剛掉 IC」：14:57 入料手臂吸取時 A／C／E 一組在時限內沒有全部 ON ⇒ `Suck()` 重試後關真空＋Error ⇒ Z 上升、IC 掉（`mykitsuck.cpp` :2304-2333）。EastSun「我現在用 VC4，要用 VC4 的偵測」→ 機台查證：In Arm 的感測列（160／64～66）就是 VC4 站的 VC0～VC2「真空 OK」位元，引擎與 Vacuum Unit 頁讀同一個位元，**不用改讀法**；處置：機台 `IO_Table.csv` 的 `InArmSuckA` OnAlarmTime 5→20（單位 100 ms，等真空最多 2 秒）。**只改機台設定，程式沒改。**
- **收法**：只有文件 ⇒ 直接 cherry-pick 到 main（乾淨），main `63c3fee5`，不等 gate。
- **快照 15:05** 鏡像到 main `323e0262`：`IO_Table.csv` 一列（InArmSuckA 5→20）；ini／lastdata／machinerecord 是執行中狀態。CHAT_JIMMY 已叮嚀。
- **鏈尾**：C++ `068df856`（cpp 0325）→ **`d6572e39`**（cpp 0326）。下一次從 **cpp 0327／web 0140／tools 0172** 開始。0321／0322 仍等 W-173（15:1x 追問第 1 次）。

## 13. 15:2x～15:3x 收到：cpp 0327（WORKLOG 207）、cpp 0328 IORESUME；快照 15:37（15:5x）

- **cpp 0327**：WORKLOG 第 207 列——EastSun「loader 與 unloader 的 edgepush 和 clip IO_TABLE delay time 設成 1.5 秒」：機台 `IO_Table.csv` Loader／Empty／Auto1～3 的 EdgePush、EdgeClip 開／關延遲改 15（只改機台設定）。
- **cpp 0328＝IORESUME**：gdb 在 Windows 停住所有執行緒，繼續後主執行緒先到 `Pci1203IoThreadAdopt`、IO 執行緒還沒做完新的一輪，看到 21 秒前的影像就判「卡沒開」⇒ LINK lost、EMG／Servo 錯誤（14:32）。改成：主執行緒自己也停了那麼久（整個程式被停）時，先等一輪新的 IO 再判斷。**程式改在 IO 執行緒段，照 RULINGS_20261005 第 115 條（＝C）留在機台**——main 沒有那一段（cherry-pick 時整段 293 行衝突），跟 cpp 0234 IOFIX 同樣只收 WORKLOG 第 208 列（commit 標題寫明「WORKLOG row 208 only」）。
- **收法**：0327＋0328（只有 WORKLOG）＋ES02 HTDESIGNER 0.285～0.286＝第 116 批 `fc380d7c`（第 199 包，GitHub `777a582f`）；**沒有 C++／CMake 改動，沒跑全量 gate**（C++ 跟第 115 批相同）。
- **快照 15:37** 鏡像到 main `88c16ce4`：`IO_Table.csv` 10 列（EdgePush／EdgeClip 延遲 15）；CHAT_JIMMY 已叮嚀。
- **鏈尾**：C++ `d6572e39`（cpp 0326）→ `104d564c`（0327）→ **`0488121a`**（0328）。下一次從 **cpp 0329／web 0140／tools 0172** 開始。0321／0322 仍等 W-173。

## 14. 18:2x 收到：cpp 0329 CYLNOREDELAY、cpp 0330（WORKLOG 209）；快照 16:41（18:4x）

- **到達時間**：兩顆在機台 `e8e58499`（commit 時間 16:42），但 GitHub 上 18:08 之後才出現——筆電 16:48／17:10／17:28／17:51／18:08 五輪 fetch 到的最新都還是 `e60bff84`（15:38）。不是漏讀，是機台晚推。
- **cpp 0329＝CYLNOREDELAY**（EastSun 1008「目前的狀況是等待太久，若同一個判斷式導致每次呼叫都要等待確認的時間，有什麼辦法可以解決」→ 在機台的提問選 B：改氣缸、只有 HT9050）：golden 的 Push／Pop 做完把 Task 設回 1，下一次呼叫（例如 `A.Pop() && B.Pop() && C_Empty_Up.Push()` 每一次掃描）又重等一次開關延遲——機台量到 MEmptyZ 停下到下一個動作 78 次，最短 1015 ms、中位數 1594 ms。改成：Push／Pop 完成就記「已確認開／關」，反方向的指令清掉另一個標記；有標記、而且**那個氣缸有該方向的感測器、感測器還在位**時立刻回 true。沒有感測器、感測器掉了、直接 On()、其他機種＝照 golden。由 `JsonBridge/W906DualCoil9050.cpp` 的註冊器掛 `g_W906CylNoReDelayHook`（只有 HT9050）。測試 `tests/test_dualcoil9050.cpp` [6]。**跟 golden 不同，是 EastSun 本人的選擇（RULINGS_20261006 第 21 條＝定案）**。
- **cpp 0330**：WORKLOG 第 209 列（同一件事；沒有感測器的 EdgeClip／EdgePush 不在範圍內）。
- **收法**：0329＋0330＋St02-E MR !344（W-178 第 1 部分）＋ES02 HTDESIGNER 0.294～0.297＝第 119 批（b18 `8d8e7343`），兩組態全量 gate 跑中。
- **快照 16:41** 鏡像到 main `c4d425fd`：只有執行中狀態（`general.ini` LastFile、`Gerneral.ini` Program Close、`config.ini` EventLog 日期格式、lastdata／machinerecord），沒有參數或工單變動，所以沒有在 CHAT_JIMMY 叮嚀。
- **鏈尾**：C++ `0488121a`（cpp 0328）→ `57b2a810`（0329）→ **`dc801732`**（0330）。下一次從 **cpp 0331／web 0140／tools 0172** 開始。0321／0322 仍等 W-173。

## 15. 18:4x～19:0x 收到：cpp 0331 PROBECYL9050、tools 0172、cpp 0332＋web 0140 CARDDO；快照 18:44（19:2x）

- **cpp 0329／0330 已進 main**：第 119 批 `68fc0e65`（第 202 包，GitHub `07dbeae3`），gate b119a：兩組態 35 分鐘：只有固定失敗（兩組態都是 4 支，486 支測試）。
- **cpp 0331＝PROBECYL9050**（EastSun 1008「Empty 為什麼那麼慢」→「繼續」）：15:57～16:00 Empty 探層每一層 27～38 秒——case 100 每一層都重跑 `EdgeClip.Pop() && EdgePush.Pop() && LoaderZ_Select.Pop() && C_Empty_Up.Push()`，沒有感測器的 EdgeClip／EdgePush（1.5 秒延遲，WORKLOG 207）每呼叫一次就重等一次。改成：`acatchtray.cpp` `W906_ProbeEmpty9050`（HT9050 專用，不是 golden 的碼）探層開始時清 `s_iEmptyProbeCylOk9050`，第一次那串條件成立時設起來，之後每一層直接動 Z。測試 `tests/test_flow9050_tray.cpp` [PC]（含對照組）。WORKLOG 第 210 列（含 209 的更正）。
- **tools 0172**：機台的 HTDESIGNER 合了筆電第 193／196～201 包（0.234→0.293；EastSun「請更新版本」），衝突取筆電那邊、機台 tools 0171 的 pairedConfig 拿掉（ES02 0.286 已併）。**跟 `PKG-*` 一樣是機台合筆電的包，不 cherry-pick**；機台的 C++ 仍是第 172 包（pkg_uptake）。
- **19:06 又收到 cpp 0332＋web 0140＝CARDDO**（EastSun 1008「1203 頁加 DO0 DO1 按鈕」→「先寫按鈕，我來測」）：新命令 `pci1203.carddo.setBit`（PCIE-1203 卡本身的 DO 通道 0／1，`Acm_DaqDoSetBit`，已在允許清單、沒有新的廠商呼叫），1203 頁加 ON／OFF 按鈕；機台自己寫明「還沒確認是卡本身的 DO 還是 ring 同號的通道」，EastSun 用按鈕測。WORKLOG 第 211 列。
- **收法**：0331＋0332＋web 0140 排第 120 批（跟 St02 MR !345、ES02 W-161、HTDESIGNER 0.298／0.299 一起）；試合過 0331 乾淨；web 0140 照慣例 `git am --directory=web`。
- **快照 18:44** 鏡像到 main `574ae7d6`：只有執行中狀態（`Gerneral.ini` Program Close、machinerecord），沒有叮嚀。
- **鏈尾**：C++ `dc801732`（cpp 0330）→ `5fe1bc7e`（cpp 0331）→ **`74294f66`**（cpp 0332）。下一次從 **cpp 0333／web 0141／tools 0173** 開始。0321／0322 仍等 W-173。

## 16. 20:5x：cpp 0331 退回；tools 0173 不收

- **cpp 0331 退回，沒進第 120 批**。gate b120a（含 0331）出貨綠、**模擬紅**：`Flow9050_Tray` [PC] 3 項（測試第 1126、1131、1137 行），在模擬組態單獨重跑也是 3 項紅（132 過）⇒ 是 0331 自己的新測試在模擬組態不過（機台只跑出貨組態），跟 0321 [BC] 同一類。處置：第 120 批改成 0332＋web 0140＋St02 !345＋ES02 W-161＋HTDESIGNER 0.305 重跑 gate（b120b，b19 `e8bd68cf`）；0332 的 WORKLOG 那段只收第 211 列（0331 的第 210 列與 209 的更正跟著 0331 等）；0331 經 EastSun 退回機台，在模擬組態修好再送（W-182，同時抄 St01）。
- **tools 0173**（19:32）：機台的 HTDESIGNER 合了筆電第 202 包，跟 0172 一樣是機台合筆電的包，不 cherry-pick。
- **鏈尾**：C++ 照舊 **`74294f66`**（cpp 0332；0331 `5fe1bc7e` 已接鏈但沒進 main）。下一次從 **cpp 0333／web 0141／tools 0174** 開始；0321／0322（W-173）、0331（W-182）修好後照新編號收。

## 17. 20:4x 收到：cpp 0333 IOWAIT、cpp 0334 TASKSJOBS、tools 0174（21:2x）

- **cpp 0332＋web 0140 已進 main**：第 120 批 `39c6388c`（第 203 包，GitHub `63b1aa25`），gate b120b：兩組態 33 分鐘：只有固定失敗（兩組態都是 4 支，488 支測試）。
- **cpp 0333＝IOWAIT**（EastSun 1008「請你繼續優化執行續」）：主執行緒每次等 1203 IO 執行緒都計時（`Pci1203IoThreadRunSync`），在 wb_serve 的 SLOW 行按階段顯示；PumpTick／MotorAccessPollTick 拆成更細的階段。只量測、不改行為。**程式掛在 IO 執行緒段，照 RULINGS_20261005 第 115 條（＝C）留在機台**——試合時 `Pci1203Monitor.cpp`／`wb_serve.cpp` 都衝突（main 沒有那一段），跟 0328 IORESUME 一樣只收 WORKLOG 第 212 列（排第 121 批）。
- **cpp 0334＝TASKSJOBS**：機台 `.vscode/tasks.json` 的 IOWEB 建置工作（ship／DEBUG／-O2）改問設計外掛要 `-j`；main 沒有 IOWEB 那幾個工作（試合衝突）⇒ 不收，跟 **tools 0174～0176**（設計外掛：`-j` 看可用記憶體、F5 模式）一起交 ES02（W-183，跟 W-171 一樣）。
- **鏈尾**：C++ `74294f66`（0332）→ `c77ea9ef`（0333）→ **`6650b5b1`**（0334）。下一次從 **cpp 0335／web 0141／tools 0177** 開始。0321／0322（W-173）、0331（W-182）仍等機台。
- **22:1x**：cpp 0333 的 WORKLOG 第 212 列（＋設計外掛 tools 0172／0173 兩列）進 main（第 121 批 `e6c54331`，第 204 包 GitHub `351291d5`）；程式照 #115＝C 留在機台。tools 0174～0176 由 ES02 0.307 合進 HTDESIGNER（W-183 結案）。

## 18. 22:4x 收到：cpp 0335 SAYTHROTTLE、cpp 0336 PKG204、web 0141 PKG204、tools 0177／0178；快照 22:47（23:0x）

- **cpp 0336＋web 0141＝PKG204**：**機台套了筆電第 173～204 包**（EastSun 要它做的）——舊檔 108 支（105 換新；`.vscode/tasks.json`、`acatchtray.cpp` 照機台）、新檔 35、兩邊都改 22（合 16、設計外掛／WORKLOG 6 支照機台）；衝突：cinitial／test_flow9050_tray／csystem 照機台，test_scankey_golden／IoBtnPanelClick／tests/CMakeLists／WebBridgeTags 照筆電；EastSun 沒 commit 的改動先擱著、合完再放回去（沒 commit）。ctest Debug 426/441，15 支是它已知的清單。**包本身，不 cherry-pick**；WORKLOG（173～204 那列、第 213 列、tools 0174～0178 的第 3 節）照鏈尾收。
- **cpp 0335＝SAYTHROTTLE**（EastSun「這邊在做什麼 也太誇張」「請一直優化執行續」）：20:51～20:59 一模一樣的「StopDec／ExtDrive FAILED ret=0x800000D3」寫了 29,690 行（Common Motion Utility 佔著卡）；`EtherCAT/Pci1203MotorRoute.cpp` 改成 5 秒內同站／同軸／同呼叫／同結果只計數不重寫，並寫出是哪個程式佔著卡。排第 123 批。
- **tools 0177／0178**：機台的 HTDESIGNER 合了筆電第 203／204 包，跟 0172 一樣不 cherry-pick。
- **快照 22:47** 鏡像到 main：只有執行中狀態，沒叮嚀。
- **鏈尾**：C++ `6650b5b1`（0334）→ `ce5735b1`（0335）→ **`2f629ec4`**（0336）。下一次從 **cpp 0337／web 0142／tools 0179** 開始。0321／0322（W-173）、0331（W-182）照舊在機台。
- 筆電的 `pkg_uptake.py` 只認 `NNNN-PKG-<n>`，沒認出 `0336-PKG204-…`，一直報「機台停在 172」；改成 `PKG-?<n>` 之後印「機台已套第 204 包」。
- **23:4x**：cpp 0335 SAYTHROTTLE＋WORKLOG（鏈尾 `2f629ec4` 整份）進 main（第 123 批 `d02fa0e9`，第 206 包 GitHub `6c604e44`）。tools 0179（機台合第 205 包的 HTDESIGNER）不收。
