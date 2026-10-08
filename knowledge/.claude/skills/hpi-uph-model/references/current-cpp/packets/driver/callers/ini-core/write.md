# 寫入路徑與失敗界線

定位 IniFiles.cpp 的 WriteString、W906_Win32WriteProfileString、W906_IniProbe；關鍵變數 writeThrough_、probe、had、buf、out。完整 body 見 [manifest](source-manifest.json)，沒有實際呼叫保存函式。

writeThrough_=true 不更新 store_，直接呼叫磁碟 writer。writer 先 probe；probe==0 才嘗試 rb，讀到的 chunks 加到 buf；had 表示 rb 開成功，並非完整讀檔成功確認。

接著呼叫 W906_IniApplyFast 得到 out。若 had、out==buf 且 W906_IniTouchSame 成功，就直接返回；其餘情況嘗試 wb，唯 probe==2 時跳過 wb。probe==1 的新檔仍可嘗試寫入，這個 body 不建立缺少的目錄。

fread 沒有用 ferror 分清 EOF／錯誤；fwrite 的實際 byte 數、fclose 回傳值與 wb 失敗未送回 caller。writer、WriteString 都回 void，沒有在此處回讀比對、交易、backup／restore 或 concurrent writer 鎖。不能將「呼叫了 writer」或 Aux SaveSetupData 回 true 說成「磁碟已保存成功」。

W906_IniApplyFast 的完整修改演算法、W906_IniTouchSame 的實作、memory store_、typed writes 與全部 caller 還待下一單元。原註解所稱格式／時間戳／golden 一致保留為歷史內容，不當成本輪測試結果；未聲稱其他 key、byte、時間戳或 crash recovery 已驗證。
