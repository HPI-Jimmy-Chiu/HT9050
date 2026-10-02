---
description: "HT9045 V906 BU-C 戰役：把同事的 PCIE-1203 成果導入 A 樹（執行一個完整波次 → 整併 → 驗收 → commit → 更新 RESUME）。搭配 /loop 可連續推進，但做完 BU-C3 硬停。關鍵字：BU-C, 1203, bring-up, Pci1203Monitor, Pci1203Control, 導入"
argument-hint: "[目標（BU-C1/BU-C2/BU-C3/auto）]"
---

先載入 **bu-wave-loop** 與 **pt-wave-loop** 兩個 skill（`Skill` 工具）取得完整政策，
再照下面執行。權威計畫書：`HT9011UC_Cpp_V3.33.906.0/docs/BU_C_CAMPAIGN_PLAN.md`。
使用者輸入：$ARGUMENTS

工作目錄：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0`。

---

## ⛔ 先讀這一段，每一波都讀

**做完 BU-C3 硬停，回報並等使用者。**
BU-C4（write path）／BU-C5（網頁落地）／BU-C6（機邊）全部是 🟡/🔴，**loop 一律不做**。

**授權三層級**（判不出來就當 🟡）：
🟢 只動原始碼/測試/文件，不碰資料檔、不武裝旗標、不開卡 → 可以做
🟡 動 `Mot_Table*.csv`／`IO_Table.csv`／`Gerneral.ini` 的 `IO_CARD_TYPE`、動驅動器參數、
   在生產 target 上武裝 `HAVE_PCI1203`、開 inbound command 通道 → **佇列**
🔴 軸會動、線圈會通電 → **佇列，要使用者在機台旁**

撞到 🟡/🔴 → 寫進計畫書 §3 對應波次的「先決/缺料」欄，**換下一個 🟢，不停不等不問**。

---

## 步驟 0 — `git status` 先跑

**第一件事，不是先讀 RESUME**（這條規則在本工作區已四次撿到樹上未 commit 的在製工作）。

```bash
cd /d/HT9045/HT9011UC_Cpp_V3.33.906.0
git log --oneline -5
git status --porcelain -- .
tasklist | grep -i -E "wb_serve|cmake|ctest"
```

1. 有未 commit 的在製工作 → **先收完，不要疊新波**。
2. 有背景 gate／workflow 在跑 → **不要改任何進建置的檔**（政策 5c，違反就作廢重跑）。
3. `build\wb_serve.exe` 在跑 → 先確認它沒鎖住連結目標，否則 gate 會假紅。
4. 讀 `docs/DEVLOG.md` 最上方那一節與 `docs/BU_C_CAMPAIGN_PLAN.md` §3 佇列。
   RESUME 與 `git log` 對不上 → **相信 `git log`**。

## 步驟 1 — 選標的

- `$1` 指名（`BU-C1`/`BU-C2`/`BU-C3`）→ 就做那個。
- `$1` 空或 `auto` → 照計畫書 §3 由上往下第一個未完成的 🟢。
- **開工前先判授權等級**，🟡/🔴 直接佇列。

## 步驟 2 — 做

**落地波（BU-C1）**：主迴圈序列做，不委派。
先判來源（B 樹的 `Pci1203Monitor.cpp` 845 行 vs 備份包的 2,565 行不是同一版）。
從 `D:\HT9045\backup\HT9050_PCI1203_20260916` 讀，**不要動 `D:\HT9050`**。
UTF-8 + CRLF、無 BOM 照原樣；兩支 `.ps1` 要**補回 BOM**（ACP=950 的筆電會 ParserError）。

**測試波（BU-C2）**：新增 ctest，**不開 `HAVE_PCI1203`**。
scratch CSV 只寫暫存目錄；`DataPath`/`asGeneralPath` 兩層都要導向 scratch
（blanket ENVIRONMENT ＋ 各寫入點的 getenv 縫，**只做第一層沒用**）。

**合併波（BU-C3）**：逐段 splice，**絕不整檔覆蓋** ——
`WebBridgeTags.cpp` 我們有 262 行他沒有。
每個 tag 守 liveness predicate 慣例：`stage*(snap,"name",<isSourceLoaded>,value)`，
未載入發 `null` 而不是看起來合理的 0。

**唯讀盤點**：絕對宣稱（「全樹沒有」「零消費者」）**預設為偽，自己用 `git grep` 重跑**。
⚠ 這棵樹裡遞迴 `grep -rn` 會回「查無」且 exit 0 而符號其實存在 —— 只用 `git grep`。

## 步驟 3 — 整併（主迴圈序列做，不委派）

CMakeLists 落點用 `nm` 量。
⚠ **`ht9045_pci1203_probe` 是 STATIC 且刻意連結進 nothing**
（`CMakeLists.txt:2874-2880` 自己寫安全性完全建立在這件事上）。
把它加進任何連結線 = 7 個 TU 重複符號。**加進去 ≠ 武裝。**

## 步驟 4 — 驗收 gate

```bash
nohup bash tools/dualgate.sh <tag> > /dev/null 2>&1 &
# 收工：
bash tools/gateverdict.sh <tag>
```

- **判定看 `*_VERDICT` / `*_EXTRA` / `*_ABSENT`，不看 exit code**
  （ctest 對任何失敗數都回 8，零鑑別力）。
- 失敗集合逐項 = 常駐五項：`config_db`/`IniFiles`/`ini_helpers`/`config_loaders`/`GA1_ReadGeneralIni`。
- 收工比對：交付檔 mtime 必須**早於** `build_<tag>g/cfg.log`（起跑錨點），否則作廢重跑。
- BU-C1 之後追加 `tools/pci1203_readonly_gate.ps1`（**必須從紅轉綠**）。
- 超出驗收線 → 停、找根因、不 commit；同根因紅兩次 → 換標的並記錄。

## 步驟 5 — commit

一顆 commit 一件事。行為變更單獨一顆。
`git add` **逐檔點名**（多 session 共用 repo，嚴禁寬 glob，嚴禁用 `git checkout` 還原）。
註解格式：`//AI(W906-BU-C<波號>) YYYYMMDD: 描述`。

## 步驟 6 — DEVLOG + 計畫書

`docs/DEVLOG.md` **最上方**加一節（做了什麼、量到什麼數字、坑、**刻意沒做的事**）。
更新 `docs/BU_C_CAMPAIGN_PLAN.md` §3 該波的狀態與「先決/缺料」。
下一步要具體到**檔名行號**。被否決的替代方案也寫進去，讓事後可以審。

## 步驟 7 — 四個安全問題 + 回報

每波結尾把這四題的答案寫進 DEVLOG：

1. 我碰到 `Mot_Table*.csv`／`Gerneral.ini`／`IO_Table.csv` 了嗎？ → **必須是「沒有」**
2. `MyLaneIo.cpp:49` 的安全門檢查還是我找到時的樣子嗎？ → **必須是「是」**
3. `tools/production_audit.ps1` 九根逐檔 MD5 零變更嗎？ → **必須是「是」**
4. 我有把「模擬跑起來」講成「機台跑過了」嗎？ → **必須是「沒有」**

回報：本波交付、兩組 ctest 數字＋**失敗集合**、下一個標的。
引用完成度百分比**必附分母與單位**。

**回合結束前不准閒著** —— 沒有背景工作在跑就開下一波（除非已經做完 BU-C3）。

---

## 停止條件（僅此五個）

1. **做完 BU-C3 → 硬停**，回報等使用者。
2. 撞到 🟡/🔴 → 佇列，換下一個 🟢，不停不等不問。
3. 同根因紅兩次 → 換標的並記錄根因。
4. 缺料（同事的 `wb_publish.cpp` / `TcpTagLink` diff）→ 記進計畫書 §3 BU-C4，換標的。
5. 額度耗盡 → 寫完 DEVLOG + 計畫書再停。
