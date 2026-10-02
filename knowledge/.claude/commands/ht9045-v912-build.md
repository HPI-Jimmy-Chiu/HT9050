---
description: "HT9045 V912 BCB6 編譯檢查模板（目前的量產維護目標）。Use when: 驗證 V912 程式碼修改後是否可編譯、需要 build/rebuild、檢查 HT9045.bpr。關鍵字：V912 build, 912 build, BCB6 build, HT9045.bpr, rebuild, compile check, Obj912, Out912"
argument-hint: "[build|rebuild|clean] [變更焦點] [期望輸出]"
---

> 本指令的執行請交給 **ht9045-v912** 子代理（用 Task 工具啟動）。

請針對固定版本目錄 `HT9011UC_Code_V3.33.912.0_20260908_Jimmy` 執行編譯檢查。

使用者輸入：$ARGUMENTS

> 若未提供，預設 build 模式並向使用者確認：
> - 建構模式：build / rebuild / clean
> - 變更焦點：例如 acatchtray.cpp JAM0610 防抖補搬後
> - 期望輸出：例如 回報可編譯性 + 失敗點 + 最小回歸建議

## 固定目標

- 版本根目錄：`HT9011UC_Code_V3.33.912.0_20260908_Jimmy`
- 專案檔：`HT9011UC_Code_V3.33.912.0_20260908_Jimmy/HT9045.bpr`
- 參考技能：bcb_build（`.claude/skills/bcb_build/SKILL.md`）

## 輸出導向（本指令最重要的一條）

V912 的 `.bpr` 把 `PROJECT` 指向 `D:\HT9045\EXE\HT9045.exe`、301 個 obj 與 PCH 指向
`D:\HT9045\Obj` —— **與 V899 出貨路徑相同**。直接建置會覆蓋出貨中的 HT9045.exe。

`build_bcb.bat` 自 20260909 起**預設**導向樹內私有目錄：

```
.claude\skills\bcb_build\scripts\build_bcb.bat "D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy" "HT9045.bpr" rebuild
```

- 產出 → `Obj912\`（中間產物、PCH）與 `Out912\HT9045.exe`
- 導向由 `scripts\redirect_mak.ps1` 在 bpr2mak 之後、make 之前執行，含驗證 gate：
  只要 .mak 還殘留任何 `D:\HT9045\Obj` / `D:\HT9045\EXE` 就直接失敗，不會開始編譯
- `clean` / `rebuild` 也先導向再解析 OBJ_DIR（否則 clean 會刪掉共用的出貨中間產物）
- **只有真的要出貨才傳第 4 個參數 `shared`**

## 執行要求

1. 先確認 BCB6 工具存在：`bpr2mak.exe`、`make.exe`、`bcc32.exe`、`ilink32.exe`。
2. 先確認**沒有 BCB IDE 實例在跑**（IDE 會搶同一組中間檔）。
3. 建置後必須驗證共用目錄未被觸碰，並在回覆中附上證據：
   - `D:\HT9045\EXE\HT9045.exe` 的 md5 與 mtime 建置前後相同
   - `D:\HT9045\Obj\*.obj` 數量不變
4. 若環境不足導致無法編譯，不可假設成功，必須直接列出阻塞原因。
5. 若編譯成功，補上與本次變更焦點相符的最小回歸建議。

## 已知基線（20260909 首次全量建置，vendor drop 未修改）

- 結果：**BUILD SUCCEEDED**，0 errors，294 個 .obj，`Out912\HT9045.exe` 30,446,080 bytes
- 警告：W8080 ×23、W8004 ×20、W8066 ×16、W8008 ×2、W8070 ×1
- **W8070 是真缺陷**：`aoutarm9045_1x2_1.cpp:61` `GetNowShuttleMode_1x2_1(int)`
  在「shuttle 有 IC 但 Item[0][0] 與 Item[0][1] 都是空」時沒有 return，回傳暫存器殘值。
  這是 V912 新單元自帶的，不是我們改壞的 —— 回報公司，不要自行修掉後忘了說。
- 先前擔心的 `cmydef.h` 三個非 inline 檔案範圍函式定義（被 214 個 build unit include）
  **並未造成 link 失敗**，ilink32 通過，map 檔裡 `IsSafePLCIO` 正常。這條疑慮已排除。
- 注意 `MachineType.h:43` 的 `SOFT_SIMULTE` 在 vendor drop 裡是開的；上述基線是模擬版。
  出貨前必須關掉並重新全量建置，屆時警告數會變。

## 輸出格式

1. 前置條件確認（含 BCB IDE 是否在跑）
2. 實際執行的建構模式與目標專案
3. Build 結果（成功 / 失敗 / 未執行）
4. 關鍵錯誤或警告摘要（與上面的基線比對，只講**新增**的）
5. 共用 `D:\HT9045\EXE` / `Obj` 未被觸碰的證據
6. 針對本次變更焦點的最小回歸建議
