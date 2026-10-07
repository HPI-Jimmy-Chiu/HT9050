# BCB6路由

先讀[原bcb_build正文](../bcb/original-entry.md)：BCB_ROOT、bpr2mak／make／ilink32、.bpr輸出／Obj／PCH路徑、增量／clean／rebuild與缺import lib等原處置。範例中的歷史目錄原樣保存，不是目前預設修改版本。

目前BCB6修正目標為V912、Big5／pre-C++11；V899與其他歷史碼依write boundary只讀。HT9050相關BCB對照版也不自動變成可寫目標；機型／runtime分流見[機型](../machines/index.md)。

工具留在原位，沒有改script或複製新鏡像：

- [build_bcb.bat](../../../bcb_build/scripts/build_bcb.bat)
- [build_bcb_safe.ps1](../../../bcb_build/scripts/build_bcb_safe.ps1)
- [redirect_mak.ps1](../../../bcb_build/scripts/redirect_mak.ps1)

本次只比對byte與文檔路由，未執行工具。bpr2mak可能寫回.bpr、.bpr路徑可能寫至共享Obj／EXE；原文相關處置保留，執行前仍以本次專案／寫入邊界為準。[原PowerShell清理策略](../bcb/references/powershell-cleanup.md)含廣域Stop-Process，不能從保留原文推得可關閉其他session的授權。
