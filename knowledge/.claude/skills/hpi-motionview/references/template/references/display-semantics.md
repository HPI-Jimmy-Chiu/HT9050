> 保存來源：`.claude/skills/ht9045-motionview-html-ui/references/display-semantics.md`，main `84233b648`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# 顯示語意（與實機 Motion View 對齊）

要取代 Motion View，格盤的顏色語意必須跟實機一致，否則現場看不懂。

## 1. HotPlate 格子的三種狀態

| 實機顯示 | 意義 | 模板做法 |
|---|---|---|
| **白色** | `HAS_NULL_IC` —— 空位（含「料已被取走」與「從未放過料」） | 淡灰／虛線框，並在標籤顯示 `null N` |
| **淡色（青／紫）** | 有料、**加熱中**（未達 soak time） | `--s0l` / `--s1l` |
| **深色（青／紫）** | 有料、**已達 soak time**（可取） | `--s0` / `--s1`＝原色加深 |

⚠ 兩個實測踩過的誤讀：

1. **橘色不是「已取走」**。實機用橘色代表「已達 soak time 的 IC」，料還在盤上。
   我曾用 FIFO 順序去對，因為「先放的先加熱完」剛好也吻合，所以誤判成「已取走」。
   真正的取料是 **橘 → 白** 的轉變。
2. **`HAS_NULL_IC` 一般設定成白色**，不要另外配一個顯眼顏色。
   加深原色來表示 soak 完成即可，另加第三個色系反而不好分辨。

## 2. 兩個色系（青／紫）代表什麼

**這顆料已被指定要送哪一條 Shuttle**：青 → SHT1（Index Arm 1）、紫 → SHT2（Index Arm 2）。

為什麼放上盤就要決定：一個 Carry Kit 只有「吸嘴數」個 Site 槽位，
取料時整組必須進同一個 Kit，所以 InArm 放上 HotPlate 的那一刻就分配好了。
實機逐格記錄在 `HP2_WhichShuttle.xls`；格內數字是 Site（`HP2_Site.xls`）。

## 3. 料盤（Loader / Bin）

| 狀態 | 顏色 |
|---|---|
| 未測料件（Loader、吸嘴上） | 藍 `--ic` |
| 已測料件（Out-Kit、OutArm、Bin 盤） | 綠 `--ic2` |
| 空格 | 淡灰 `--line2` |

「已測」不必再細分 Bin 別；要顯示 Bin 分類時再加色階。

## 4. 立體圖上的逐格在籍點

只顯示總數不夠直覺，現場習慣看格盤。做法：

- 位置**固定**（`dotGrid()` 建立時算好 iso 座標），每幀只改 `fill`
- 用 `setDot()` 做 fill/stroke 快取，避免每幀 setAttribute 全量寫入
- 每格約 2.4~3.4 px；`Loader 250 + HotPlate 128 + Bin 250 ≈ 630` 個點更新無感

### ⚠ 圖層順序（踩過的坑）

**不透明的盤面必須畫在逐格點之前**，否則會把點蓋掉。
模板的 SVG 分層順序固定為：

```
gS（靜態盤面、軌道、機構外框）→ gDot（逐格在籍點）→ gD（會動的手臂/托板）→ gT（文字標籤）
```

Auto1 盤面原本畫在 `gD`，結果把 Bin 的逐格點全蓋掉，看起來像「Auto1 被遮住」。

## 5. 手臂與機構

| 元素 | 表現 |
|---|---|
| 吸嘴頭 | 開放框（`fill:none`），間距**真的隨變距開合**，附尺寸線與行程尺 |
| 吸嘴 Z | 該支有動作時顯示 `ZA▼`／`ZA▲`；八支各自獨立（`MInArmZA~ZH`） |
| 手臂閒置 | 虛線細框 |
| Index 兩臂 | 各自配色（琥珀／紫），Socket 占用另開一軌 |
| Tray Arm | 平時停在 Empty 軌（干涉區外），只在退盤時進 Auto 區；閒置畫虛線 |
| 互鎖區 | 俯視圖上用紅框標當下作用中的 Zone |

## 6. 標籤要寫得能一眼判斷

- 數量標籤寫 `料 32　null 80` 或 `120/128`，**不要只寫一個數字** ——
  只寫 `128` 時看不出跟 `120` 的差別（實測被質疑過）
- ⚠ **數量標籤不可壓在盤面中央** —— 會蓋掉一整片逐格在籍點（實測被退回過）。
  做法：錨在盤的**前緣（最後一列）**，再往畫面下方推開。
  且**必須用螢幕座標偏移**，不能只改 model Y ——
  iso 的 Y 係數只有 `0.315`，盤面在畫面上很扁，
  往 model Y 推一整個盤深也只換到 ~20 px，擠不出標籤自己的高度。

  ```js
  /* badge(name,n,x,y,z,color,label,wide,dx,dy) —— dx/dy 是螢幕座標 */
  badge("TRAY",cnt(s.tray),G.LOADER,LANE.loader.y0-TRAY_D+TRAY.yp,0,null,null,false,26,20);
  ```

  移完要驗兩件事：① 換算後的 svg 座標仍在 `viewBox` 內（含標籤自己的高度）；
  ② 沒有被 bottom-left / top-right 的浮動吸嘴頭小圖（`.inset`）蓋住 ——
  那是 HTML overlay，不受 SVG 圖層順序管。
- 時間軸標題註明「示意，非實測」；不要放 cycle time / UPH 數字，
  避免與客戶 log 的 `Index Cycle Time` 定義混淆
- 變距讀數同時顯示 mm 與 pulse，並在踩到機構上下限時標出來

## 7. 本機檔案的快取陷阱

改完 HTML 用瀏覽器開本機檔案時，**常會抓到舊快取**。
交付時直接告知使用者按 `Ctrl + F5`（或 `Ctrl+Shift+R`）強制重載，
並在回覆裡附上「檔案時間 + 關鍵字串是否存在」的驗證結果，避免來回確認。

<!-- preserved-content:end -->
