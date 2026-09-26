# 給機台端 Claude：更新包 15（GitLab main `c65ddd85`，相對更新包 14 `21323505`）

> 筆電端 Claude 20260926 19:5x 產生。**先套更新包 3～14，再套這一包。要不要套由 Jimmy 決定。**
> 3 檔（`tools/wb_serve.cpp` 與兩份文件），底稿 `base_21323505\`。一樣排除 OBJROOT 那 4 個檔。

## 這一包是什麼：NB2 R70 覆核第 9 條（阻塞框等待）抓到的四件，已修

`tools/wb_serve.cpp`，全部同一行改寫（行數不變）：

| 件 | 改了什麼 | 你們機台上看得到的差別 |
|---|---|---|
| **MW-A**（🟠，最重要） | 警報框／是否框開著時，框裡的 1203 `Poll()` 前後把「輸出優先」的 yield hook 拿掉 | **以前：框開著時在網頁 IO 頁點輸出，約一半會真的動。現在：一律回 `modal-pending`、輸出不動**（golden 框開著時其他畫面按不到）。Motor Test 的 `motor.stop` 照舊放行 |
| **A1**（🟠） | 關框時把 `bLampStart`／`bLampPause` 設回 false | 以前關框後面板 START／PAUSE 燈約一半機會停在常亮；現在回到第 9 條之前的全暗（照 golden 的 ProcessKeyFlush 燈號還沒翻） |
| A5 | 告警框開、關時照 golden 清 `bAlarmReset`（note.cpp:1363／:2522） | 之前框裡按過 Alarm Reset 之後，下一個警報的 `GetHandlerStatusByDll` 不再回 4、主畫面不顯示 Alarm；現在照 golden |
| YN-3 | 是／否框也收 `sim.di.set` | 只影響 SOFT_SIMULTE |

## 在機台上要看的（NB2 R70 §1 的驗法，這台筆電沒有 1203 做不了）

1. 叫出一個阻塞警報（`sys.echoErrorModal` 或真的警報）。**確認機台周圍有沒有人**。
2. 框開著時，在 `HW.IoSetView` 對同一個輸出點點 10～20 次。
3. 修正後：ack 全部是 `modal-pending`，console 不再出現 `dispatch(output-first): io.btnPanelClick`，輸出不變。
4. 答掉警報後，面板 START／PAUSE 燈不會卡在常亮。

## 步驟

同前幾包：Check → EastSun 同意 → 備份 → Apply → **重新建置** → ctest 與筆電比 → commit 回報。
`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`

筆電驗證：兩組態語法檢查 0 錯、全量 gate＝基準（出貨 3＋5 Disabled、模擬 18＋5 Disabled），子檢查 0 差異；system＋config＋IniData 0 變動。
