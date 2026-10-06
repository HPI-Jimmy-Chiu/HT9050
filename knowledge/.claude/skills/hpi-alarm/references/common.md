# Alarm 共同項與差異

| 項目 | 共通原則 | 路徑／配置差異 |
|---|---|---|
| 解除 | 選取動作與確認分開，ReturnCode 是所選動作 | 原生 Note 二段式、Message 實體鍵一段式；V906 畫面 START 還受客戶／SIM 條件限制 |
| 停機 | 停不停由 C++ 決定，HTML 不能拿視窗層級當停機證明 | V906 ShowErrorMessage hook host 中的 kcode==0 通知仍可先停機，唯讀 seam 無宿主時是另一條路 |
| 權限 | 各類密碼與按鍵 gate 分清 | 原生 Message 實體鍵／滑鼠有非對稱；Note、NonStop、SpecialPanel 不共用一個開關 |
| 告警位置 | arguments.position 對 unit／panel／mv | HT9050 與其他機型用不同 mv9045／mv9050；Code 文本不可由 position 假推 |
| Web 信箱 | requestId／tag、C++ 等待者與畫面 queue 同時追 | kcode==0 通知、阻塞 note、modeless message、被取代的框各走不同 reply |

ESD、安全門、溫度、EMG 的必停機裁決與 stop／nonstop 顯示界線保存於原文，不能藉整理 Skill 刪除或放寬。UI 的 queueStop 永不丟、queueNS 上限8，與 C++ 單槽不可混為同一個queue。

先讀 [目前main查證](runtime/current-main.md)，再按 [解除](dismissal/index.md)、[Dialog](dialog/index.md)、[顯示](display/index.md)、[機型](machines/index.md) 分層；原生／歷史原文不直接當 V906 全量現況。

## Yield 告警也屬同一主題

共同順序是 CalculateSiteYield → 按 site／picker／total／interval／special 分群檢查 → DoLowYieldAlarm 的 Retry／OneCycle／Smart Auto Clean 分流。實際觸發仍取決於客戶旗標、測試模式、窗口與門檻，不能由機型名稱推定。

[Yield 分層入口](yield/index.md)保存完整九函式說明及 E-034 沿革；[目前 Yield](yield/current-main.md)核對 Timer3 呼叫、清計數、RT 自動關 site 與尚有 gate 的 UI 記錄，不把舊程式註解當目前未移植的證據。
