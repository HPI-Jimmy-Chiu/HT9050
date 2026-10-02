---
description: 推進一個 ST 波次（把 golden TfMain::Start() 分段翻進 TfMainWeb::StartFromWeb）。做完 ST-W7 硬停，絕不做 S1／S3 武裝。
---

先載入 **st-wave-loop** 與 **pt-wave-loop** 兩個 skill（`Skill` 工具）取得完整政策，
再照下面執行。權威計畫書：`HT9011UC_Cpp_V3.33.906.0/docs/START_CAMPAIGN_PLAN.md`。

使用者輸入（可選，指名波次如 `W3`）：$ARGUMENTS

---

## 步驟 0 — `git status` 先跑，不是先讀計畫書

```
cd /d/HT9045/HT9011UC_Cpp_V3.33.906.0
git log --oneline -5
git status --porcelain -- . | grep -v '^??'
tasklist | grep -i -E "cmake|ctest|cc1plus"
```

有 build／ctest 在跑 → 不介入，只回報就結束回合。

## 步驟 0.5 — ★ 交棒檢查（每一次觸發都要做，在選波次之前）

```
grep -n "kWave7State" HT9011UC_Cpp_V3.33.906.0/WebStart.cpp
```

| 值 | 意思 | 動作 |
|---|---|---|
| `0` / `1` | 未開始 / 進行中 | 往下做步驟 1 |
| **`2`** | **卡住** —— §5.1 剩下的 gate 全都要使用者裁決 | **交棒** |
| **`3`** | **完成** —— §5.1 的 🔴 全部補完或明確裁決過 | **交棒** |

### 交棒動作（2 和 3 都做，但回報的話不一樣）

1. `CronList` 找到 prompt 是 `/st-wave` 的那個 job，`CronDelete` 它。
   **用 prompt 內容找，不要寫死 job id** —— session 重開後 id 會變。
2. `CronCreate({cron: "13,43 * * * *", prompt: "/fw-wave", recurring: true})`。
   **交棒對象是 `/fw-wave`，不是 `/night-loop`**（使用者 20260915 17:00 裁決）。
   理由：ST 卡住時通常還早（fShow 那組要等裁決），而 fw-wave 的佇列很長、
   方向唯讀、夜間最安全，正好接手 st-wave 讓出來的時間。
   `/night-loop` 由它自己的時間鏈（`7,27,47 4-7 * * *`）在清晨奪權，不需要這裡掛。
3. `ScheduleWakeup({stop: true})`（如果這一輪是被 ScheduleWakeup 叫醒的）。
4. 回報，然後**結束回合**。不要順手開始跑 night-loop 的第一輪 ——
   讓新掛的 cron 自己觸發，這樣「交棒發生了」是可觀察的。

### ⚠ 回報時 2 和 3 不可以混為一談

- `3` = **做完了**。ST-W7 的還債清單清空，S2 結束。
- ⚠ 依 20260915 的實際狀態，**最可能的結果是 `2`** —— `fShow` 那 10 個
  需要「web 層怎麼回報當前開啟頁面」的設計裁決，而做它要改 `wb_serve.cpp`
  （S1/S3 地盤）。回報時要照實說「做不下去」，不要粉飾成做完。
- `2` = **做不下去了**。剩下的每一個 gate 都需要使用者裁決
  （例如 §5.2 的 `fShow` 那 10 個，要先決定 web 層怎麼回報當前開啟頁面）。
  **回報必須逐條列出卡在哪、等什麼決定**，不可以只說「ST 戰役結束」。

寫成 `2` 就是承認自動迴圈做不動了。**不要為了讓它看起來完成而寫 `3`。**

### ⚠ session-scoped，這件事要對使用者講清楚

cron 與 `ScheduleWakeup` **都活在這個 session 裡**。Claude Code 視窗關掉、
或機器重開，**兩個都會死，交棒也不會發生**。磁碟上唯一的狀態是
commit + `kWave7State` + 計畫書 —— 使用者隔天重開後 `/night-loop` 冷啟動即可接續。

---

## 步驟 1 — 下一波是哪一波（量，不要記）

```
grep -n "kTranslatedLines" WebStart.cpp
```

`0`→W1、`309`→W2、`660`→W3、`961`→W4、`1262`→W5、`1574`→W6、`1875`→**W7（小相依）**。
`$ARGUMENTS` 指名就做那一波。

| 波 | golden 範圍 | 行數 |
|---|---|---|
| ST-W1 | `main.cpp:4385-4693` | 309 |
| ST-W2 | `main.cpp:4694-5044` | 351 |
| ST-W3 | `main.cpp:5045-5345` | 301 |
| ST-W4 | `main.cpp:5346-5646` | 301 |
| ST-W5 | `main.cpp:5647-5958` | 312 |
| ST-W6 | `main.cpp:5959-6259` | 301 |
| ST-W7 | 6 個小相依（149 行）＋ 補完 §5.1 的 🔴 | — |

## 步驟 2 — 抽 golden、查相依、翻譯

golden 是 Big5（`cp950` 讀），抽到 scratchpad，不要動 SVN 工作副本。
`tools/start_wave_deps.py` 先列缺什麼 —— 但它看不到「定義被 `#if 0` 閘住」，
那個只有連結器抓得到。

每一行帶 `// golden :NNNN`。相依不存在就 `SAFETY-GATE(W906-ST-Wn-X)` 閘掉，
**用機台的話寫清楚後果**，並在計畫書 §5.1 加一列、標 🔴（golden 用它擋啟動）
或 🟡（純顯示）。**能部分閘就不要整段閘。**

## 步驟 3 — 四關驗收，缺一不可

1. `g++ -fsyntax-only -Wall -Wextra` → exit 0 且 **WebStart.cpp 自己 0 個診斷**
2. 全量 `cmake --build build` → rc=0（**先導檔再讀，管線會吃掉 exit code**）
3. 動到既有檔時做 preprocessed 比對 → 每一行差異都必須是預期中的那一行
4. `nm --defined-only` 看 `StartFromWeb` 在 exe；
   `nm --undefined-only` 看它引用**活的機台符號** ← 這關是「build 綠不等於接上了」的解藥

## 步驟 4 — 收工

更新 `kTranslatedLines` → 計畫書 §5 打勾 + §5.1 補 gate →
`git add` 逐檔點名 → commit（寫量到什麼，含自己的錯與更正）→ push →
wb_serve 起回來並跑 `tools/pagewire/verify_engine_live.py`。

---

## ⛔ 硬停止（沒有例外）

- **做完 ST-W7 就停、回報、照步驟 0.5 交棒給 `/fw-wave`。**
  **不准碰 S1（操作權 token）或 S3（掛分派）** —— 交棒不是授權，
  fw-wave 自己的硬邊界（唯讀方向、write path 佇列等使用者）照樣把 S1/S3 擋在外面。
- 絕不 override `TfMain::Start()` —— 全樹 19 個呼叫點只有 2 個被閘住。
- 絕不寫 `SystemStart = true` 或任何等價捷徑。
- 未翻完時 `StartFromWeb()` 尾端一律 `iStartIn = 0; return false;`。
- 撞到需要改 `tools/wb_serve.cpp` → 停。那是 S1／S3 的地盤。
