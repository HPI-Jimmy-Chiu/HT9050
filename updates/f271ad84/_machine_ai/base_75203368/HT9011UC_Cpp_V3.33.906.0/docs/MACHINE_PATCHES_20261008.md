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
