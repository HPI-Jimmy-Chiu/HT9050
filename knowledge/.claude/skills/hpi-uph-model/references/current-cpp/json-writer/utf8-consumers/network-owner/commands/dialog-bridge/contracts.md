# 版本、機型、客戶與歷史裁決

[上層](index.md)／[證據](evidence.md)；活定位以來源樹／檔名＋function／變數，固定pin offsets只用於原文保存。

本單元只查 `web/page/dialog-bridge.js` 一份UTF-8網頁來源；它是V906宿主／Recipe傳輸的頁面bridge證據，不是V912 BCB6／Big5程式修正。
HT9050與其他Handler可以共用本頁傳輸mechanism，但實際channel／page／宿主／服務root及C++機種分派須核對部署與當台版本；本輪沒有現場載入證據。
七body沒有以CUSTOMER_CODE／MachineTypeChoice完成機種或客戶對照；未核對V912、V899與各客戶頁面，不能由本檔沒有該欄位推論全機台無差異。
same displayKind、requestId、channel、seq是不同責任欄位，不能拿頁面接受某訊息取代後端授權、IO互鎖或正在等待的query。
全文保留20260922不停機、20260924debug及傳輸fallback、20261003close recent、20261008recent[]註解；其中歷史實機驗證與golden行號皆為原文史料，不改稱本輪驗證。
HTDialogBridge.build標記、檔頭規格、channels／frame設定及API object逐字保留；build字串本身不能證明最新source pin、頁面部署或所有caller已同步。
僅文件／保存／引用驗證，沒有執行JS、browser、build、機台或runtime；沒有修改snapshot／system設定。
