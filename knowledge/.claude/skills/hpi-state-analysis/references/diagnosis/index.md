# 卡住、延遲與deadlock診斷

先讀[共同證據](../common.md)，再按需要讀：

- [完整SOP](../source/references/analysis-sop.md)與[讀取筆記](../source/references/state-record-reading-notes.md)：版本／參數→三時間→最後正常與第一個異常Task。
- [troubleshooting](../source/references/troubleshooting.md)：區分感測器／到位、通訊等待、測試端／資料庫與流程條件。
- [deadlock模式](../source/references/deadlock-patterns.md)：Synchronize／ThreadProcess／MainProc與GUI依賴，原判斷有版本界線。
- [決策變數](../source/references/decision-variables-registry.md)與[互鎖](../source/references/module-interlock-map.md)：以實際Task／case與允許條件核對。

重複state、單次耗時增加或多模組同時停更是證據線索，不能單憑它們確診硬體或參數錯誤。根因依證據排序，缺檔與未核對caller明列，驗證步驟需符合當次現場條件與授權。
