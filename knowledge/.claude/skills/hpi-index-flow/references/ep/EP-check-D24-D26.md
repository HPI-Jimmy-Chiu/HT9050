> 保存來源：`.claude/skills/ht9045-contact-force/references/EP-check-D24-D26.md`，main `e184ef205`。以下保留原文；原文中的機型／版本與「裁決、提案、已實作」仍依原標註。當前實作狀態先看 [共同與差異](../common.md)。

<!-- preserved-content:start -->
# EP 壓力回授監控：[D24] vs [D26]（含 Hana Micron / HT9046LS 案）

> 來源：V3.33.908.16 實碼追查（2026-08-31）。案由：Hana Micron 端客戶要求
> "prevent over contact force on HT9046LS"，Teratech Korea (S.E.Hong) 詢問
> D24 與 D26 的差異與該用哪一個。

---

## 1. 一句話結論

- **[D26] = 判定容差（數值）**，單位 **kPa**，是 over / under force 監控的**主角**。
- **[D24] = 一個加壓自檢動作**，用來抓 EP 漏氣，**沒有自己的容差欄位，一律吃 D26 的數字**。
- 兩者的 alarm 都是 **WAR1605**（`The air of electronic air regulator (EP) is not enough!`）。

---

## 2. 對照表

| 項目 | **[D24] Enable EP check function** | **[D26] Enable EP encoder range +,-** |
|---|---|---|
| 性質 | 動作：主動加壓自檢 | 判定：容差視窗（數值） |
| 顯示條件 | Gerneral.ini `IndexEveryTimeCheckEP=1` | `EP_Install==3 \|\| EP_Install==5` |
| 時機 | `DoTestHeadMotor` case 9（Z1 移安全位置）→ case 30000 | 每次下壓、EP 已充到設定值，送測試命令前 |
| 動作 | EP 輸出最大值（`EP_MAXKPA<=500 ? 4095 : 2275`）→ 保壓 → 讀回比對 → 回寫 `DeviceForm.dPress` → 等洩氣 | 不額外動作，只讀回當下實際壓力 |
| 容差來源 | **借用 `IniConfig.iD26EPEncoderRange`** | 自己的 `iD26EPEncoderRange`（10~100，預設 100） |
| 判定 | `ADAM_Alarm()` | `ADAM_Alarm()` |
| 目的 | EP 氣囊 / 調壓閥漏氣健檢（保養面） | 每顆接觸壓力 over / under 監控（品質面） |
| UPH | **會拉長 index cycle**（詳見 §5） | 幾乎無影響 |

---

## 3. 程式碼錨點（V908.16）

| 位置 | 內容 |
|---|---|
| `cConfiguration.cpp:1359-1380` | D24 / D26 / D26_1 / D26_2 註冊；D24 只在 `bIndexEveryTimeCheckEP==true` 才 `bShow`；D26 群組只在 `EP_Install==3\|\|5` 才 `bShow`。Range 上下限 `10~100`，預設 100（ASE_KH+HiSilicon 例外：鎖定 5） |
| `adam6024.cpp:539-542` | `ADAM_Rang(int v){ iADAMRange=v; }` — 只是把 D26 數值塞進全域 |
| `adam6024.cpp:545-601` | `ADAM_Alarm()`：`PA=ADAM_ReadPA()`（**kPa**）、`iAdamOutValue=AdamOutputToPA(iWritePA)`（**kPa**）；`PA > out+iADAMRange \|\| PA < out-iADAMRange` → true。JCET / KYEC_LEE 且 `iD26_3FixValueOrPercentage==1` 時改用百分比 |
| `adam6024.cpp:503-522` | `ADAM_ReadPA()` 內差回傳 **kPa** |
| `adam6024.cpp:964-967` | `KpaTransferKG()` 末段：`Kg = kPa × 10.197 × (D²×π/4 × LoadRate) / 1000`（D 單位 cm） |
| `atester.cpp:6143-6156` | D24 觸發點：`bD24EnableEPCheckFuntion && bIndexEveryTimeCheckEP` → `InitIndexEveryTimeCheckEP(); Task=30000;`（`CC_VTEST_Shanghai` 排除） |
| `atester.cpp:8331-8336` | case 30000 → `IndexEveryTimeCheckEP()` 回 true 才 `Task=10` 續行 |
| `atester.cpp:9159-9264` | `IndexEveryTimeCheckEP()` 狀態機本體 |
| `atester.cpp:9267-9412` | `CheckAndRecodrEP()` — 生產中每次測試的 EP 讀回 + log + alarm |
| `atester.cpp:1418-1422` | `CheckAndRecodrEP(Type)` 呼叫點：`DoTester` 內、送測試命令前 |
| `main.cpp:21318-21325` | Contact 畫面開啟 / D24 自檢進行中 → `fAirForce=-1` 暫停一般 EP 輸出 |
| `adam6024.cpp:2659-2673` | `ADAM_ReturnValueCheck()`：D26_2 / D24 / D26 任一開啟就做週期性讀回顯示 |

---

## 4. ⚠️ 最容易踩的坑：D26_1 是隱藏的總開關

`CheckAndRecodrEP()` **整個函式體包在 `if(IniConfig.bD26EnableEPLog==true)` 之內**
（`atester.cpp:9271`），而生產時的 alarm 判斷在它裡面：

```cpp
bool CheckAndRecodrEP(int iArm)
{
    if(IniConfig.bD26EnableEPLog==true)          // ← D26_1，沒開整段不跑
    {
        ... 建立 CSV ...
        if(shuttle 條件成立)
        {
            ADAM_Rang(IniConfig.iD26EPEncoderRange);   // 容差永遠取自 D26
            bAdamAlarm = ADAM_Alarm(...);
            ... 寫 CSV ...
            if(IniConfig.bD24EnableEPCheckFuntion ||   // D24 或 D26 任一開 → 發 alarm
               IniConfig.bD26EnableEPEncoderRange)
            {
                if(bAdamAlarm && SystemStart)
                    ShowErrorMessage("WAR1605", K_RETRY, ...);
            }
        }
    }
}
```

**後果**：只勾 D26、不勾 D26_1 時，量產中**完全不會**發 EP alarm，
D26 只剩 auto-get-height（`cContact.cpp:5839`、`8578`）的 `bEPLeakage` 還會用到。
這是實作耦合（EP log 與 EP 檢查綁在一起），不是設計意圖，但現行版本就是如此。

→ **任何要求「EP 壓力異常要報警」的客戶，D26_1 必須一起勾。**

---

## 5. D24 的 UPH 代價

`IndexEveryTimeCheckEP()`（`atester.cpp:9159-9264`）是**插在每一次 index 循環裡**的
（原碼註解：「Index每一次都確認EP是否有充飽氣」），流程：

| case | 動作 | 耗時 |
|---|---|---|
| 100 | Z1 移 `Prod.TestZ1_Safe`，`ADAM_DirectWriteData(max)`，`CheckEPTimer.SetSecAndOn(5)` | 馬達移動 |
| 200 | 等 `CheckEPTimer.Off()` → 讀回比對；OK 則回寫 `DeviceForm.dPress`，`ReleaseEPTimer.SetSecAndOn(1)` | **5 s** |
| 300 | 等 `ReleaseEPTimer.Off()`（等洩氣）→ return true | **1 s** |

合計固定等待 6 s（`//Steven 20240618 : 3 --> 5`，舊版為 3+1=4 s），再加 Z1 移動時間。

> **對客戶的措辭**：只說「會拉長 index cycle time / 影響 UPH」，
> **不要報 6 秒這個數字**（RogerYang 20260831 指示）。內部評估時才用上表。

**建議定位**：D24 不適合量產常開，應作為**換 kit 後、定期保養、或懷疑 EP 漏氣時**的
臨時驗證手段，驗完關閉。日常 over-force 監控用 D26 + D26_1。

---

## 6. 單位與換算（客戶最常誤解）

- 欄位名稱寫 "encoder range"，但 `ADAM_Rang()` 存的值是直接和 `ADAM_ReadPA()` 回傳的
  **kPa** 相減 → **Range 的單位是 kPa**，不是 encoder count。命名是 2011 年 ChungHung 留下的。
- kPa → Kg：

  ```
  Kg = kPa × 10.197 × (D² × π/4 × LoadRate) / 1000     // D = kit 缸徑 (cm)
  ```

  以 D=6.0 cm、LoadRate≈1 估算，**±10 kPa ≈ ±2.9 Kg**（實際須代入該機台的 LoadRate）。
- 客戶若給的是「可接受 ±X Kg」，要用上式反推 kPa 後填入 D26。

---

## 7. D26 子選項

| 選項 | 變數 | 作用 |
|---|---|---|
| D26_1 Enable EP log | `bD26EnableEPLog` | **生產中 EP 檢查與 alarm 的實際總開關**；產生 `D:\HT9045_Log\EP\YYYYMM\YYYY-MM-DD.csv`（Time / Index / Setting / Kpa / Kg / Alarm） |
| D26_2 Show EP encoder | `bD26EnableEncodeShow` | Contact 畫面顯示 EP 讀回值（`lblReadEP` / `lblReadEP2`），調機決定 Range 用 |
| D26_3 Dual EP encoder range | `bD26_3EnableDualEPEncoderRange` / `iD26_3DualEPEncoderRange` | 第二顆 EP（Die Force）；需 `INSTALL_DOUBLE_EP==DOUBLE_EP_NORMAL\|\|DOUBLE_EP_MULTI`；alarm 走 `ADAM_DualAlarm()` → WAR1610 |
| （JCET/KYEC 專用）| `iD26_3FixValueOrPercentage` | 1=Range 當百分比、0=當固定值；僅 `CC_JCET` 顯示，`CC_KYEC_LEE` 也吃這個判斷 |

---

## 8. 建議設定（over-force 監控需求通用）

| 項目 | 設定 | 理由 |
|---|---|---|
| D26 | ✅ ON，數值由可接受 Kg 公差反推 | over / under force 判定核心（上下限同時管） |
| **D26_1** | ✅ **必開** | 不開 alarm 完全失效；另產生 CSV 追溯 |
| D24 | ⚠️ 量產不建議常開（拉長 index cycle） | 僅作漏氣臨時驗證 |
| D26_2 | ⭕ 調機時開 | 決定 Range 值 |
| D26_3 | ❌ 關（無 dual EP 時） | 僅 Die Force 機型 |

---

## 9. 客戶回覆要點（英文，可直接引用）

- D26 = the tolerance window; compares the **actual EP pressure read back on every press-down**
  against the commanded value; alarms on **both over and under** force (WAR1605).
- The "Range" unit is **kPa**, not encoder counts; settable 10–100.
- **D26_1 (Enable EP log) must also be checked** — otherwise no alarm is raised in production.
  It also produces a traceability CSV for the end customer.
- D24 = a **leak self-check** that inflates the EP to maximum and verifies it; it uses the
  **same tolerance value entered in D26** and **increases the index cycle time**, so it is
  recommended as a temporary check (after kit change / maintenance / suspected leak),
  not as a permanent production setting.
- D26_3 is for dual-EP (Die Force) machines only.

---

## 10. 順手記下的碼瑕疵

`cConfiguration.cpp:1361` 的 `else` 後面殘留沒有 `//` 的 `ECBool,`（註解符號被刪掉）。
因 `ECBool` 是 enum 常數，被當成逗號運算式 → **編譯會過、行為正確**，僅影響可讀性。
下次動到該區時順手補回 `//`。

<!-- preserved-content:end -->
