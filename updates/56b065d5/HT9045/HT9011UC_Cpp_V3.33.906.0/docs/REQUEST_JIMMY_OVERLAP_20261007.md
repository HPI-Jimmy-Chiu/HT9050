# 派工給 Jimmy：全部畫面檢查「有沒有東西擋到」（2026-10-07）

EastSun 1007：「請派工jimmy 偵測所有畫面是否有擋到」。

## 這次看到的例子

Setup → Contact 頁按 Save 之後，右下跳出存檔結果框（黑底：「✓ 已寫入（golden DeviceForm_File）／ⓘ 65 個欄位目前權限不能改，沿用原值／⚠ golden 還有沒做到的步驟：…」），
蓋住 Save／Exit 按鈕、「Contact Force Calibration」、Contact Height 區，而且內容很長（golden 未做步驟的開發用說明）。

- 產生處：`web/page/ht9045_wire_engine.js:1325` 附近（存檔 ack → 訊息列）；同型的還有 `ht9045_offset_ev.js:179／:222`、`ht9045_offset_wire.js:290／:296`。
- 機台規則（EastSun）：**不要蓋住畫面、不要多加確認框**。

## 要做的

1. **全部頁面掃一遍**（每個 HW.*／Setup.*／Status.*／Main.* 頁，含各分頁、對話框、浮動訊息、toast、下拉、tooltip）：
   - 任何浮動元素（position:absolute／fixed、z-index 高的）蓋到可操作的控制項（按鈕、輸入框、下拉、勾選）就算「擋到」。
   - 也檢查控制項互相重疊、超出所屬框、被截斷（文字顯示不全）。
   - 建議做成自動檢查（headless Edge：對每個可點控制項中心點做 `document.elementFromPoint`，回傳的不是它自己或它的子元素＝被擋），各頁跑出清單。
2. **存檔結果訊息**：不要蓋住按鈕；開發用的「golden 還有沒做到的步驟」不要顯示給操作員（放 log 或收合）；訊息自動消失或放在不擋操作的位置。
3. 機種差異：HT9050 有隱藏／收合的區塊（Teach 頁 W906Teach9050Layout、gbW906TrayZ9050），兩種機種都要掃（`?machine=HT9050`）。
4. 清單（頁面、元素、被誰擋、截圖）連同修正一起交回。

## 參考

- 機台之前用過的量法：headless Edge 量 `elementFromPoint`（Teach 頁 Out Shuttle Z 控制項定位時用過）。
- headless Edge 的 profile 會記住視窗大小，測試要固定 `--window-size`。
