---
name: bu-wave-loop
description: HT9045 V906 BU-C 戰役（把 PCIE-1203 成果導入 A 樹）的自動波次政策。定義授權三層級、波次佇列、A 樹 gate 集、硬停止線（做完 BU-C3 就停）、以及「回傳 SUCCESS 不等於機台做了事」那一族陷阱。Use when：執行 /bu-wave、規劃 1203 導入波次、判斷某個 1203 動作能不能自動做、要把同事的 1203 成果合併進 A。關鍵字：BU-C, BU-W, bring-up, 1203, PCIE-1203, Pci1203Monitor, Pci1203Control, 機邊, 授權三層級, Mot_Table, IO_CARD_TYPE
---

# BU 波次迴圈政策（HT9045 V906 · 1203 導入）

**權威計畫書：`HT9011UC_Cpp_V3.33.906.0/docs/BU_C_CAMPAIGN_PLAN.md`**
（§1 現狀實測、§2 授權三層級、§3 波次佇列、§5 gate、§6 停止條件、§7 已付代價的坑）。
本 skill 是執行摘要；**兩邊不一致時以計畫書為準，並回頭修這裡。**

## 先讀 pt-wave-loop（單一出處，不複寫）

`pt-wave-loop` skill 的以下段落**全部適用**，動工前載入它：
六個已付代價的陷阱／硬邊界（Big5、EOL、唯讀樹）／「不准自行停下」三規則／
冷啟動協議／額度中斷實測表／「agent 論證比程式碼更常錯」。

---

## ⛔ 本戰役最重要的一條

**做完 BU-C3 硬停。** BU-C4（write path）、BU-C5（網頁落地）、BU-C6（機邊）
**全部是 🟡/🔴，`/loop /bu-wave` 一律不做。**
撞到就寫進計畫書 §3 對應波次的「先決/缺料」欄，換下一個 🟢，**不停不等不問**。

---

## 授權三層級（每一波開工第一件事就是判這個）

| 級 | 定義 | loop 可不可以做 |
|---|---|---|
| 🟢 | 只動 A 樹原始碼 / 測試 / 文件。**不碰資料檔、不武裝旗標、不開卡** | ✅ 可以 |
| 🟡 | 動 `Mot_Table*.csv` / `IO_Table.csv` / `Gerneral.ini` 的 `IO_CARD_TYPE`、動驅動器參數、在**生產 target** 上武裝 `HAVE_PCI1203`、開 inbound command 通道 | ❌ 佇列 |
| 🔴 | 軸會動、線圈會通電、`WB_PUMP_1203_CONTROL_LIVE` | ❌ 佇列，且要使用者在機台旁 |

**判不出來就當 🟡。**

---

## 每波必答的四個安全問題（一題都不能省，答案寫進 DEVLOG）

1. 我碰到 `Mot_Table*.csv` / `Gerneral.ini` / `IO_Table.csv` 了嗎？ → **必須是「沒有」**
2. `MyLaneIo.cpp:49` 的安全門檢查還是我找到時的樣子嗎？ → **必須是「是」**
3. `tools/production_audit.ps1` 九根逐檔 MD5 零變更嗎？ → **必須是「是」**
4. 我有把「模擬跑起來」講成「機台跑過了」嗎？ → **必須是「沒有」**

---

## 波次佇列（權威在計畫書 §3，這裡只列可自動的部分）

| 波 | 級 | 內容 | 驗收 |
|---|---|---|---|
| BU-C0 | 🟢 | 戰役開張（備份、計畫書、指令、政策） | 文件齊備 |
| BU-C1 | 🟢 | 唯讀層落地：`Pci1203Gear.h` / `YaskawaSigmaXAlarms.h` / `Pci1203Monitor.{h,cpp}` 編進 `ht9045_pci1203_probe`（STATIC、**不武裝**） | dualgate 兩側 GREEN ＋ `pci1203_readonly_gate.ps1` 從紅轉綠 |
| BU-C2 | 🟢 | `test_pci1203_seam`（不開 `HAVE_PCI1203`）：斷言 A 馬達工廠只靠資料就選到 1203 且它是惰性的；斷言 B IO facade 把 `ePCI1203` 路由到注入的 backend | 新測試綠 ＋ dualgate 兩側 GREEN |
| BU-C3 | 🟢 | `WebBridgeTags.{cpp,h}` 三方合併 ＋ pci1203 tag 家族 | dualgate 兩側 GREEN |
| **BU-C4 起** | 🟡🔴 | **⛔ 停** | — |

---

## A 樹的 gate 集（**不是 B 樹那一套**）

```bash
cd /d/HT9045/HT9011UC_Cpp_V3.33.906.0
nohup bash tools/dualgate.sh <tag> > /dev/null 2>&1 &
# 收工：
bash tools/gateverdict.sh <tag>
```

- **判定看 `*_VERDICT` / `*_EXTRA` / `*_ABSENT`，不看 exit code。**
  ctest 對任何失敗數都回 exit 8 —— 5 個失敗和 7 個失敗的 sentinel **逐位元組相同**。
- 失敗集合逐項必須等於常駐五項：
  `config_db` / `IniFiles` / `ini_helpers` / `config_loaders` / `GA1_ReadGeneralIni`。
- **一律用序列版 `dualgate.sh`，不要用 `dualgate2.sh`**（後者把 Debug ctest 疊在
  Release build 上，實測把 `dfm2rc_fidelity` 推過 600 秒逾時 → 兩側假紅，
  換到的加速只有 6%）。
- **gate 執行期間不准改任何進建置的檔**（原始碼與 `CMakeLists.txt`）。
  收工用 `build_<tag>g/cfg.log` 的 mtime（**起跑**錨點，不是 sentinel 寫入時刻）
  比對交付檔 mtime，任何一個較新就作廢重跑。
- 1203 專屬：BU-C1 之後 `tools/pci1203_readonly_gate.ps1` **必須綠**；
  BU-C4 之後加 `tools/pci1203_control_gate.ps1`。

⚠ **`.claude/skills/bu-wave/SKILL.md` 第 4 步寫的 gate 集是 B 樹形狀**
（`build_nonoracle` / `build_x64` / `pe_truncation_check.ps1` / `macro_order_gate.ps1`）。
那些腳本隨 BA 確實搬過來了，但 **A 樹沒有 `build_nonoracle` / `build_x64` 目錄**。
A 的主 gate 是 `dualgate.sh`。

---

## 這個戰役特有的五個陷阱

1. **先問「這段文件講的是哪一棵樹」。** 9 份 1203/BU 文件裡 **7 份描述的是 `D:\HT9050` 的碼**
   （commit `707b175` 只搬了文件）。`docs/BU_CAMPAIGN_PLAN.md` 的 19 顆 RESUME commit
   在 A 全部是 unknown revision；`MON1203_WEB_CAMPAIGN_PLAN.md` 與 `mw-wave-loop` skill
   宣稱的「MW-1/MW-2 ✅ 已完成」「`Pci1203Monitor.{h,cpp}` ✅」**對 A 是假的**。
   三個 10 秒判準：
   `git grep -c "pci1203\." -- WebBridgeTags.cpp`（A=0、B=72）、
   `ls EtherCAT/ | grep 1203`（A 無）、`git log --grep="BU-" | wc -l`（A=0）。
2. **`Acm_*` 回傳 SUCCESS 不是「機台做了事」的證據。** 實測：沒有 cyclic exchange 時
   `Acm_AxJog` 回 SUCCESS 而軸不動，且 DI 影像在開卡那一刻就凍結。
   **不要在 `Acm_*` 回傳碼上面蓋互鎖或交握。**
3. **同事的成果包不是超集，是分岔。** `WebBridgeTags.cpp` 我們 1066 行、他 1782，
   `+979 / −262` —— **整檔覆蓋會丟掉我們 262 行**（含 20260915 的 `AI(W906-FW-1T)` 系列）。
   一律逐段 splice，不要 copy 覆蓋。同理 `pci1203_readonly_gate.ps1`
   （他有 20260914 的 anchor 修正、我們有 BOM，兩邊各有對方沒有的東西）。
4. **`probes/fixpot.cpp` 會寫 `Pn50A` 關掉驅動器超程保護，而且從來沒被執行過。**
   **不要讓它進到任何 build glob 掃得到的目錄。**
   同族：`kCmdAxAbsEncoderReset` 不可逆（多圈歸零、原點消失、要重 HOME 且 SERVOPACK
   重新上電），它靠「打字輸入站號」保護且 C++ 會比對該軸自己的站號 —— 改 UI 不要拿掉。
5. **`ht9045_pci1203_probe` 的安全性完全建立在「沒有人連結它」**
   （`CMakeLists.txt:2874-2880` 自己寫的）。它把 7 個 TU 用 `HAVE_PCI1203=1` 再編一次，
   而那 7 個檔已經在 `ht9045_motor` / `ht9045_io` / `ht9045_sm` 裡。
   **把這個 archive 加進任何連結線 = 7 個 TU 重複符號。**

---

## 額度中斷協議

照 `pt-wave-loop`。中斷前務必：
(1) 寫完 `docs/DEVLOG.md` 一節 ＋ 更新 RESUME，下一步具體到檔名行號；
(2) 把「缺料 / 撞到 🟡」的項目寫進 `docs/BU_C_CAMPAIGN_PLAN.md` §3 對應波次；
(3) 明講哪些數字已驗證、哪些沒有。
