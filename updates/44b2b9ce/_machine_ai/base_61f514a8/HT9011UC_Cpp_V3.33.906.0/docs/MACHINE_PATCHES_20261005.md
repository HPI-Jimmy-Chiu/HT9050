# 機台端 patch 整合 1005（cpp 0202～0209、web 0103～0108）

> 接在 `docs/MACHINE_PATCHES_20261002.md`（第三輪，cpp 0047～0129、web 0045～0082）之後。中間兩輪沒有另寫這份檔：
> cpp 0130～0179 在第 53 批（1004 00:08 `5d6a27ae`，第 135 包）收進 main；cpp 0180～0201 在第 65 批（第 145 包）收進 main，逐顆帳本是 `docs/handoff/MACH0180_LEDGER_20261004.md`。
> 裁決照舊：RULINGS_20260930 第 11 條（衝突以機台為準）；機台的暫時繞道留在機台。
> ⚠ 筆電 1004 晚上到 1005 04:5x 只巡了 GitLab，**漏讀了機台 10/04 的兩次推送**（GitHub `machine/integ-ioweb` `d8abb7c` 20:58、`04048c0` 22:17）——
> 原因：這個 session 載入的 night-loop 技能是主 checkout 的舊版，沒有「§1.0 機台端優先：每輪先抓 GitHub」那一節（main 版有）。技能自己要求的「開工先比對主 checkout 與 main 的技能」那一步沒做。

## 0. 這一輪收到什麼

| 機台 patch | 是什麼 | 處置 |
|---|---|---|
| cpp 0202 `13eca55` | PKG-134：機台合筆電第 134 包 | 不收（是我們的包；機台的合併解法照它的 WORKLOG §3） |
| cpp 0203 `ecfa1a4` | PKG-135..140：機台合筆電第 135～140 包；**裡面夾帶機台自己的 TRAYSAFE-3**（照 St01 S-26：`mymotor` 的 `TrayArmMotorMove` 防呆改回原版，只有 HOME 的 `W906_HomeTrayArm` 在 HT9050 傳 `bCheckPos=false`，自動運轉照原版檢查） | 不收：包的部分是我們的；TRAYSAFE-3 收窄 golden 檢查（HOME 時不檢查 Tray 臂位置）＝機台繞道，留機台 |
| cpp 0204 `0540c57` | WORKLOG：135～140 包 ctest 失敗清單 | 沒收（見 §1 WORKLOG） |
| cpp 0205 `e820eea` | HT9050-TEACH-KB-LOADERY：Teach 的 Loader Y 小鍵盤在 HT9050 不夾上下限 | **留在機台**：改的是機台自己的 `FileRW/TeachKb.cpp`（機台 10/02 的 TEACH-KB，裁決 C），main 沒有這個檔；main 照 #51＝A 本來就不夾 |
| cpp 0206 `3800d02` | HT9050-TEACH-KB-ALL：Teach 全部欄位在 HT9050 不夾（開關 `W906_HT9050_TEACH_KB_CHECK` 預設 0） | **留在機台**（同上）；= NIGHT_REPORT §0 #99 機台自己解了 |
| cpp 0207 `fa29db7` | WORKLOG 10-04：§2 第 120～123 列、§4 空跑前必處理、HT9050 Out Shuttle 規則 9／10、Frank 待回 | 沒收（見 §1）；內容已轉 St01（W-62）、NB2-1（W-63） |
| cpp 0208 `33880da` | PKG-141..144：機台合筆電第 141～144 包；**`WebBridge/WebBridgeServer.cpp` 沒套**（!176 把 `act.observerSG.state` 加進免權杖，等 EastSun） | 不收（是我們的包）；main 維持 !176，等 EastSun |
| cpp 0209 `fef1f41` | WORKLOG 10-04：§2 第 124～125 列（第 125 列＝三個動作的唯讀查證＋約 14 項待 EastSun） | 沒收（見 §1）；內容已轉 St01（W-62）、NB2-1（W-63） |
| web 0103 `94b238e`、0104 `f6f923a`、0108 `af09ed6` | PKG：機台合筆電第 134、135～140、141～144 包的網頁那一半 | 不收（是我們的包） |
| web 0105 `9a0cc07` | HT9050-TEACH-KB-ALL 網頁那一半 | **留在機台**：改的是機台自己的 `page/ht9045_teach_kb_c.js`，main 沒有 |
| web 0106 `1554472` | TEACH-1PICKZ：單吸嘴機台 Teach 的 In／Out Arm Pick Up／Place Z 點重新顯示（EastSun 1004「我的ZA需要校正」） | **收**：第 66 批 `e55b4748`（git am，作者照機台） |
| web 0107 `1e95562` | TEACH-1PICKZ-2：HMI 外框裡也顯示（等整頁載完再判斷吸嘴數） | **收**：第 66 批 `84da524e` |

## 1. WORKLOG 鏡像（1005 05:4x 補上）

用 `resume_20261001/mach_chain.py` 把 C++ 歷史鏈從 `a64c46ff`（cpp 0129）接到 0209：0130～0187 → `9792ea5b`；**0188 跳過**（0187 重複匯出，MACH-0180 帳本已記）；0189～0209 → 鏈尾 **`2f49d2ca`**。每一顆都不帶模糊比對地套上。
鏈尾的 `docs/WORKLOG_MACHINE.md` 跟 main 比只多 20 行（main 沒有任何一行是機台版沒有的）⇒ 整份照鏈尾鏡像（第 66 批 `73971487`）＝等於收了 WORKLOG 0204／0207／0209 加上機台合包 commit（0202／0203／0208）寫的 §3 列。
（先前用 `git am -3` 單收 0204 會失敗，就是因為它的上下文是那些合包列。）

## 2. 下一次從哪裡開始

- 下一次從 **cpp 0210／web 0109／tools 0162** 開始。
- C++ 歷史鏈尾 **`2f49d2ca`**（cpp 0209）；web 鏈尾仍是 `78ee7510`（web 0084，0085～0108 還沒接——這一輪收的 web 0106／0107 是直接 `git am --directory=web` 套上的）。
