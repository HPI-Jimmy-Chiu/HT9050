# DFM 畫面解析度規範（Batch D）

> 適用時機：當本次掃描包含任何 `.dfm` 修改時，平行啟動 Batch D。

---

## 兩種解析度

機台螢幕有兩種解析度，設計 BCB `.dfm` 畫面時必須確保在兩種解析度下皆能正確顯示、不超出框架：

| 解析度 | 備註 |
|--------|------|
| **1280×1024** | 較小螢幕，需特別注意控件是否被裁切 |
| **1920×1080** | 較大螢幕，確認控件不會錯位或空白過多 |

---

## 設計原則

- 設計時以 **1280×1024** 為最小基準，確保所有控件在此解析度下完整可見。
- 避免使用絕對座標定位超過 1280×1024 可視範圍。
- 若使用 `Anchors` 或 `Align`，需驗證在兩種解析度下展開行為是否符合預期。

---

## 檢查重點

> **C：自動掃描只看 Form 物件本身**（每個 `.dfm` 的第一個 `object Xxx: TXxx`）的 `Left` / `Top`。
> 子元件（Panel / Button / Edit…）的 `Left`/`Top` 為相對父層座標，**一律不檢查**，避免大量誤報。
> （V3.33.906.0 全量掃描：原始 28 筆 → 只看 Form 後 4 筆。）

| 項目 | 檢查方式 |
|------|----------|
| **Form** Left 是否落在可視範圍外（1280 < Left < 60000） | 解析 dfm 第一個 object 的 Left（排除 IDE 佔位值如 65532） |
| **Form** Top 是否落在可視範圍外（1024 < Top < 60000） | 解析 dfm 第一個 object 的 Top |
| 控件 Left + Width 是否 ≤ 1280 | 解析 dfm 中 Left/Width 屬性 |
| 控件 Top + Height 是否 ≤ 1024 | 解析 dfm 中 Top/Height 屬性 |
| `Anchors` 設定 | 確認 `[akLeft, akTop, akRight, akBottom]` 組合正確 |
| `Align` 設定 | `alClient` / `alLeft` / `alRight` 等是否會在大解析度下浪費空間 |

---

## 修復約定：Form 開在畫面外（Left/Top 超出可視範圍）

當發現 **Form**（`object Xxx: TXxx`）的 `Left`/`Top` 設計值 >1280 / >1024（且 <60000，排除 IDE 佔位值如 65532）：

- **修法**：僅將該 **Form 物件本身** 的 `Left` / `Top` 改為 `10` / `10`。
- **禁止**：變動 Form 內任何子元件（Panel / Button / Edit…）的座標——子元件座標為相對父層，Form 移動後整體版面不變。
- **命中非 Form（如 `TPanel`）**：屬內部元件，**不修改**，僅標記待人工確認母 Form 寬度 / `Anchors`。

案例：V3.33.906.0 — `frmDefrost`(Left=2038) / `fBinAOISel`(1959) / `fAdam6024`(1611) 三個 Form 改為 10/10；`Magazine.dfm` 的 `pnlMagazineTrayOut`(TPanel) 屬內部元件未動。

---

*最後更新：2026-06-18*
