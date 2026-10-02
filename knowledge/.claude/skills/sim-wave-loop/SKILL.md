---
name: sim-wave-loop
description: HT9045 V906 SIM 戰役（用 golden 自己的 SOFT_SIMULTE 路徑跑模擬）的自動波次政策。定義最終目標四門 G1-G4、SIM-0~SIM-5 波次佇列、偏離佇列、真機邊界、停止條件。Use when：執行 /sim-wave、規劃 SOFT_SIMULTE 波次、解閘 MainProc 模式階梯、HOME/START 准入路徑、判斷某個改動是翻譯還是偏離。關鍵字：SIM-W, SOFT_SIMULTE, 模擬, 沒有機台, MainProc 階梯, ScanSystemSensor, SoftStart, fAllMotorHome, SystemStart, DoHomeProcess, 主守衛, 4239, 解閘, un-gate
---

# SIM 波次迴圈政策（HT9045 V906 — golden 的模擬路徑）

**權威計畫書：`HT9011UC_Cpp_V3.33.906.0/docs/SIM_CAMPAIGN_PLAN.md`**
（§1 最終目標四門、§3 波次佇列、§4 gate、§5 偏離佇列、§6 真機邊界、§7 停止條件）。
本 skill 是執行摘要；細節以計畫書為準，**兩邊不一致時以計畫書為準並回頭修這裡**。

## 先讀 pt-wave-loop（單一出處，不複寫）

`pt-wave-loop` skill 的以下段落**全部適用**，動工前載入它：
五個已付代價的陷阱／硬邊界（Big5、EOL、唯讀樹）／「不准自行停下」三規則／
冷啟動協議／額度中斷實測表／驗收 gate 4a-4b／「agent 論證比程式碼更常錯」。

## SIM 專屬要點

0. **⚠️ 開 `SOFT_SIMULTE` 不會讓機器啟動。** 20260825 盤點（16 agents）的結論：
   **全樹沒有任何 `SOFT_SIMULTE` 分支會把 `SystemStart` 或 `fAllMotorHome` 設成 true。**
   所有觸及這兩個旗標的分支都是 `#ifndef` 方向 —— 定義它是「**跳過**會清除它們的硬體 abort」。
   真正的關鍵路徑是 **`uhome.cpp` 未翻譯**（golden 4,876 行，`ProcessMotorHome` body 3,663 行）。
   計畫書 §0 有完整證據；**不要重新發明「開個 flag 就好」這個預期**。

1. **目標是翻譯忠實度，不是新功能。** golden 已有 `SOFT_SIMULTE` 模擬入口（971 命中／155 檔）；
   移植樹已翻進 911 命中／170 檔。缺的是讓 golden 自己的 HOME→START 路徑能走。

2. **這個戰役不是「讓 tag 數字動」。** 那件事已達成且證據等級很低：
   pump 每 tick 進 `MainProc`，但 `DoAllProcess` 在**主守衛**就 return
   （實測 `SystemStart=false`、`fAllMotorHome=false`），引擎一個都沒派工。
   ⚠ **原文寫 `csystem.cpp:4239`，那個行號已死** —— GL-4p 20260830 用編譯器重量：
   活體守衛在 **`csystem.cpp:4475`**，而 `:4239` 落在 `DoAllProcess` 的**另一份死定義**裡
   （該檔有兩份定義，`:3309` 死／`:4451` 活）。要用就跑
   `tools\live_lines.ps1 -Source csystem.cpp -Lines <n>`，不要抄。
   ⚠ 另外 `--pump` 這個旗標本身已於 20260908（MW-1）移除，pump 現在是無條件行為。

2b. **範圍只到「准入」，不含「准入後跑得對不對」。** 後者今天不可能回答：沒有 golden 的
   tick/UPH/bin 基準，而且 `Motor/mymotor.cpp:713-732` 的 live body 是 golden 的**非**
   `SOFT_SIMULTE`「瞬間到位」臂（golden 的多 tick ramp 在 `:658-696` 的 `#if 0` 內），
   時序語意本來就不同。混為一談會做出無法驗收的承諾。

3. **四門缺一不可**（計畫書 §1）：
   G1 兩組建置 exit 0 ／ G2 守衛項由**活體**設 true ／ G3 引擎真的被派工 ／
   G4 不開卡不下馬達命令。
   **G2/G3 的證據必須是 gdb 的 stack，不是 tag 數字。**

4. **每一波跑兩組建置**：開 `SOFT_SIMULTE` 與不開。只驗一組會讓另一組靜默壞掉 ——
   這是本戰役最容易犯的錯。

5. ⚠️ **這一條已被推翻，20260826 —— 不要照舊版做。**
   舊文：「`SOFT_SIMULTE` 用命令列開，不改 `MachineType.h:48`」。
   **現況：`MachineType.h:48` 就是 `#define SOFT_SIMULTE`**（使用者 20260825 親手取消註解，
   SIM-0 commit `3e45194`），而 GL 戰役 20260826 裁決**保持這樣**。
   理由是安全：依賴命令列旗標時，只要任何一條建置線漏帶，就會在真機上產出**非模擬**建置。
   ⚠️ 動 `:48` 前必須先開 `GATE G9`（`csystem.cpp:16643`），否則會變成
   「電源給了、煞車還咬著」。完整論述：`docs/GL_CAMPAIGN_PLAN.md` §4.4。
   **連帶影響**：`build_nonoracle` 與 `build_sim_nonoracle` 現在**都是** SIMULTE 建置；
   「不開 SIMULTE」那一組要另外用 `-USOFT_SIMULTE`，而且**只驗編譯、建出來的 exe
   絕不在這台執行**（見 `GL_CAMPAIGN_PLAN.md` §4.5）。

6. **命中數不等於活著。** `csystem.cpp` 22.5% 是 `#if 0` 死參考文字（162 塊／7,183 行）。
   引用 911 當覆蓋率之前，先用預處理器區間掃描量出活/死比例（SIM-1 的功課）。

7. **忠實 vs 偏離的分界線**（計畫書 §5 有完整表）：
   解閘 `MainProc` 階梯 = 翻譯，可做；
   從 `PumpTick` 直呼 `ScanSystemSensor` = **偏離**，佇列不做；
   `PumpInit` 重新強制守衛 = 使用者 20260817 已否決；
   放寬 `csystem.cpp:4239` = 動主安全守衛，絕不。

8. **階梯不准零碎解閘。** 閘門橫幅原話：那條 if/else-if chain 的各臂是互斥構成的，
   抽一部分會讓兩個模式在同一 tick 跑。先找出 un-gate 阻擋清單再評估。

9. **波次順序（20260825 盤點後重排，第一版是錯的）**：
   SIM-0 連結門（3 個 facade 成員，證據鏈完整，可立即做）→ SIM-1 基線門 →
   SIM-2 階梯門（小～中，~10 + 60-80 行，只缺 `fSpeed`）→
   SIM-3 START/HOME 寫入者（小～中，~190 行）→
   **SIM-4 HOME 門＝`uhome.cpp` 翻譯，大，多日，真正的關鍵路徑** → SIM-5 守衛門。
   循環門已移出範圍（見 2b）。

10. ⚠️ **這一條也過期了，20260826。** 舊文：「這台沒有 git」。**現在有** ——
   MinGit 2.55.0 裝在 `%LOCALAPPDATA%\Programs\git-mingit`（使用者層 PATH，
   `core.autocrlf=false`，因為這棵樹 EOL 混合、硬統一會造出上千行假 churn）。
   所以 `/pt-wave` 的 `git status` / commit 步驟**現在可以正常用**，
   不需要 `_noBuild` MD5 比對或整樹快照那些替代做法。
   ⚠️ 但 `git` **不一定在新 shell 的 PATH 裡**，波次腳本要自己 prepend：
   `$env:PATH = "$env:LOCALAPPDATA\Programs\git-mingit\cmd;$env:PATH"`
   ⚠️ `web/`（`d:\HT9045\web\.git`）是**另一個 repo**，跨 repo 改動需要兩顆 commit。
   ⚠️ **`d:\HT9045\.claude\` 仍然沒有版本控制**（29 個 skill ＋ 16 個 command，
   含這個檔案本身）。根目錄要不要 `git init` **是使用者的決定，記入佇列不要自己做。**

11. **`WebBridgeTags` 有 5 處失效引用**（計畫書 §9），其中
   `WebBridgeTags.cpp:317-321` 的「one flag away」**直接誤導了 20260825 的除錯方向**。
   修它們是 SIM-1 的交付項。看到那些行號不要照抄，先複驗。

## 真機邊界（這台是實際機台，不是開發機）

`pt-wave-loop` 硬邊界全部適用，追加：專屬 `build_sim*` 目錄、`build\` 永不碰、
**跑 ctest 前備份 `system\`+`config\`+`IniData\` 收工 MD5 比對**（常駐失敗五項正好是碰設定檔的）、
`wb_*` 不加 `--real`／`--allow-cmd`、`--dry` 不覆蓋 `AuthPath`
（⚠ **20260908 MW-1**：`wb_publish`／`wb_gateway` 已完全不收參數，給了 exit 2；
這三個旗標現在**只**存在於 `wb_serve`。測試旋鈕改 `WB_PUBLISH_*`／`WB_GATEWAY_*` 環境變數）、
不拿 `pci1203_linkprobe --enumerate` 當煙霧測試。

**絕不把「模擬跑起來」講成「機台跑過了」。**

## 夜間 loop 與額度

- 每波收工＝commit + DEVLOG + 🔖RESUME，然後**立即開下一波，不准閒著結束回合**。
- 需要偏離才能推進 → 記入佇列，**改推下一個里程碑或改推 PT 波次**，不停。
- 同一里程碑連續 2 波零進展 → 記錄阻擋、改標的；連續 4 波才回報等裁決。
- 額度耗盡 → 寫完 DEVLOG + RESUME 再停。
- session 死了：磁碟上的 commit + RESUME 就是全部狀態，使用者重開後
  `/loop /sim-wave` 冷啟動即無損接續。
