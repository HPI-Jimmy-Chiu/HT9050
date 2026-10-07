# 機台 patch 收件紀錄 20261008

> 接 `MACHINE_PATCHES_20261007.md`（第 21 節）。來源照舊是 GitHub `HPI-Jimmy-Chiu/HT9050` 分支 `machine/integ-ioweb`；衝突以機台為準（RULINGS_20260930 第 11 條）；鏈尾＝機台歷史重建鏈（`mach_chain.py`）的最後一顆。

## 1. 1007 23:41 收到的 cpp 0309／0310 上 main（第 99 批）

- **cpp 0309 TRAYSAFE9050**（EastSun 1007「Loader 不會將 Tray 移動到吸取位置／Empty 不會將空 Tray 補到 Auto 1」→「寫 HT9050 自己的判斷：Loader 在最左邊，Empty 在最右邊，Auto 在中間」）：golden 的 Tray Arm 教導防撞線 (Empty+Color)/2+6500（這台＝6866）是 9046 的版面，HT9050 往 Empty（7532）與 Loader（191213）的移動全被拒（22:23:45「TrayArm moves 7532 to the left error」）⇒ `Motor/mymotor.cpp` 改照 HT9050 站序；`tests/test_flow9050_tray.cpp` 新 [TS] 區（5 項）。
- **cpp 0310 TRAYFLOW9050**（EastSun「Empty 不會將空 Tray 補到 Auto 1」→「我現在修」）：`acatchtray.cpp` DoCatchTray case 500／600 在 Type_HT9050 改走 `W906_CatchFromEmpty9050`（檔尾：Empty 疊數不明時往上探到有盤、算出 L+1 盤；最深層已有盤報 MES1022；只有抽屜感測報 MES1024；SIM＝5），DoCatchFromEmpty_9050 拿最上面那盤並把數量減一；400／1000／2000 也重設 9050 子游標；`cmydef.cpp` Auto1～3 層數從 0 起算（原本 -1 讓第一盤高一格）；`asendic_Loader.cpp` Loader 數到 0 改報 MES0920（原本 1→800→900→1 靜默打轉）；`csystem.cpp` HOME／START 也重設 9050 Tray 子游標。
- **合併衝突（1 處，以機台為準）**：0310 跟筆電第 97 批的 `c755aa7a` 改到 `tests/test_fp9050_index.cpp` 同一行（FP9050_Pure 的 [F2] 普查讓 FLOWTRACE 讀 `iTestYFinePitchTask`）——筆電版只放行 `W906_FlowTraceTick` 函式內，機台版放行整支 `csystem.cpp`。照機台版（`git checkout --theirs`），筆電那兩行函式範圍追蹤一起退回，檔案＝機台的版本。
- 收進**第 99 批**（b18 `v906/jimmy-b99`：main `90348048`＋0309＋0310（cherry-pick）＋St02 W-149 MR 4（!319，只改測試）＋NB2-1 LINK-22 文件（!318），`868caf4c`）。
- 第 99 批上 main `cfd361b8`＝**第 182 包**（GitHub `14c39ef7`）；gate b99a：兩組態 31 分鐘：只有固定失敗（兩組態都是 4 支；模擬版 W-149 那 3 支已過）。
- **鏈尾**：C++ `b0b3c5ab`（cpp 0308）→ **`c79de0ba`**（cpp 0310，第 99 批）。下一次從 **cpp 0311／web 0140／tools 0171** 開始。
