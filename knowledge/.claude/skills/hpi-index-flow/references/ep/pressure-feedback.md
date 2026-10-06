> 保存來源：`.claude/skills/ht9045-contact-force/SKILL.md`，main `e184ef205`。以下保留原文；原文中的機型／版本與「裁決、提案、已實作」仍依原標註。當前實作狀態先看 [共同與差異](../common.md)。

<!-- preserved-content:start -->
## 6. EP 壓力回授監控：[D24] vs [D26]

> 完整版（程式碼錨點、狀態機、單位推導、客戶回覆英文稿）見
> [references/EP-check-D24-D26.md](EP-check-D24-D26.md)。

**一句話**：**D26 = 判定容差（數值），是 over/under force 監控的主角；
D24 = 一個加壓自檢動作，沒有自己的容差欄位，一律吃 D26 的數字。** 兩者 alarm 都是 WAR1605。

| 項目 | **[D24] Enable EP check function** | **[D26] Enable EP encoder range +,-** |
|---|---|---|
| 性質 | 動作：主動加壓自檢 | 判定：± 容差視窗（數值） |
| 顯示條件 | Gerneral.ini `IndexEveryTimeCheckEP=1` | `EP_Install==3 \|\| 5` |
| 時機 | `DoTestHeadMotor` case 9 → case 30000（每個 index 循環） | 每次下壓、EP 充到設定值，送測試命令前 |
| 動作 | EP 輸出**最大壓力** → 保壓 → 讀回比對 → 回寫設定值 → 等洩氣 | 不額外動作，只讀回當下實際壓力比對命令值 |
| 容差來源 | **借用 `iD26EPEncoderRange`** | 自己的 `iD26EPEncoderRange`（10~100） |
| 目的 | EP 氣囊 / 調壓閥**漏氣健檢**（保養面） | 每顆**接觸壓力 over/under 監控**（品質面） |
| UPH | **會拉長 index cycle**，量產不建議常開 | 幾乎無影響 |

### ⚠️ 三個必記的坑

1. **`iD26EPEncoderRange` 的單位是 kPa，不是 encoder count。**
   `ADAM_Rang()` 存的值直接和 `ADAM_ReadPA()`（回傳 kPa）相減。欄位名是 2011 年舊命名。
   換算：`Kg = kPa × 10.197 × (D²×π/4 × LoadRate) / 1000`（D = kit 缸徑 cm）；
   D=6.0、LoadRate≈1 時 ±10 kPa ≈ ±2.9 Kg。
2. **D26_1「Enable EP log」是隱藏的總開關。**
   `CheckAndRecodrEP()` 整個函式體包在 `if(bD26EnableEPLog==true)` 內，alarm 判斷在其中
   （`atester.cpp:9271` / `9399`）。**只勾 D26 不勾 D26_1 → 量產中完全不會發 EP alarm。**
   任何「EP 壓力異常要報警」需求，D26_1 必須一起勾。
3. **D24 沒有自己的公差欄位**，`ADAM_Rang(IniConfig.iD26EPEncoderRange)` 永遠取 D26 的數字。

### 判定式（`adam6024.cpp:545-601`）

```cpp
PA            = ADAM_ReadPA(&dValue);          // 讀回實際壓力 (kPa)
iAdamOutValue = AdamOutputToPA(iWritePA);      // 命令壓力   (kPa)
if(PA > iAdamOutValue + iADAMRange ||
   PA < iAdamOutValue - iADAMRange)  return true;   // → WAR1605，over 與 under 同時管
```
（`CC_JCET` / `CC_KYEC_LEE` 且 `iD26_3FixValueOrPercentage==1` 時改判百分比。）

### 建議設定（over-force 監控需求通用）

| 項目 | 設定 | 理由 |
|---|---|---|
| D26 | ✅ ON，數值由可接受 Kg 公差反推 | 判定核心 |
| **D26_1** | ✅ **必開** | 不開 alarm 完全失效；另產生 `D:\HT9045_Log\EP\YYYYMM\*.csv` |
| D24 | ⚠️ 量產不建議常開 | 僅換 kit / 保養 / 懷疑漏氣時臨時驗證 |
| D26_2 | ⭕ 調機時開 | 畫面顯示 EP 讀回值，決定 Range |
| D26_3 | ❌ 關（無 dual EP 時） | 僅 Die Force 機型，alarm 走 WAR1610 |

> **對客戶措辭**：D24 的代價只說「會拉長 index cycle time / 影響 UPH」，
> **不要報具體秒數**（RogerYang 20260831 指示）。

---


<!-- preserved-content:end -->
