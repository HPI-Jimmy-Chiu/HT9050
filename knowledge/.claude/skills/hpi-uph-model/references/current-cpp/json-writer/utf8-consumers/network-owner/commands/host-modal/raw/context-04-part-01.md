# 原文：complete YES/NO rationale and adjacent metadata（1）

[上層](../index.md)／[manifest](../source-manifest.json)。來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp`，固定pin `b187b85fbbdc5167338ae886c8845ac775d97b37`。
以function／變數定位；offset與SHA僅固定pin保存證據。本頁payload 35行；context credit0，重疊內容不重計。

```cpp
<!-- preserved-content:start -->
// ===========================================================================
//  AI(W906-YESNO) 20260925：ShowMyMessageBox_YES_NO -> 網頁是／否對話框，答案回流。
//
//  使用者 20260925 裁決（docs/RULINGS_20260925.md 第 10 條，3A）：「YES/NO 對話框被
//  替身自動回答 → 照 golden 跳網頁對話框，等操作員回答」。與同日第 2 條（教導頁的阻塞
//  告警框照 golden 跳框並等待，即使單執行緒 tick 迴圈停住）同方向。
//
//  ## 沒有發明新傳輸 —— 與 ForwardShowErrorMessage 同一條路
//    請求：WebSocket query 訊框（PostQueryOptions，options=["YES","NO"]）
//          ＋ 檔案信箱 Message-dialog-request（同事 dialog-bridge.js 的 show-my-message
//            通道，畫在 page/Alert.MyMessageBox.html —— 那頁本來就有 pnlYes/pnlNo）
//    回應：modal.answer／dialog.response（tag=qid，value="YES"|"NO"；帶 ":<按鍵>" 也接受），
//          WebBridgeServer 的 socket 執行緒把它放進 CommandQueue（權杖豁免，
//          WebBridgeServer.cpp 的 AI(W906-ALARM-ANSWER-TOKEN)），這裡的迴圈自己 drain。
//    ⇒ 瀏覽器端只多了 dialog-page.js 的兩顆鍵（pnlYes/pnlNo）綁定；
//      ht9045_dialog_host.js、dialog-bridge.js、WebBridgeServer 的應答路徑都沒改。
//
//  ## golden 的 YES/NO **會停機** —— 照做
//    golden 本體 mymessbox.cpp:1020 `StopAllMotor();`，ShowModal 觸發的 FormShow
//    :302-310 `if(!iUnLoaderCount){ SystemStart=false; SoftStart=false; ... StopAllMotor(); }`。
//    這一點與告警（fNote）一樣。golden PowerSavingMode.cpp:156-158 的註解也佐證：
//    「不能用ShowErrorMessage，因為retry->Start，機台就跑起來了」—— YES/NO 答完**不會**
//    自己重新啟動，操作員要再按 START。見檔尾 W906_YesNoShowLikeGolden／W906_YesNoCloseLikeGolden。
//
//  ## 等待期間答案進得來嗎（死鎖）
//    與 ForwardShowErrorMessage 完全同構：socket 執行緒獨立於本執行緒收 WebSocket 並
//    push 進 CommandQueue（有鎖），本迴圈每 100 ms drain 一次；modal.answer／dialog.response
//    不需要權杖。其他指令一律回 "modal-pending"（cfg.resync 與 motor.stop 例外，理由同上面那支）。
//    沒有逾時 —— golden 的 ShowModal 也是無限等。
// ===========================================================================

// AI(W906-YESNO) 20260925: 寫一份 YES/NO 請求進 show-my-message 信箱。
//   JSON 由 tools/wb_dialog_mailbox.h 的 w906dlg::YesNoRequestJson 組（欄位與理由寫在那裡；
//   放在標頭是為了讓 tests/test_yesno_dialog.cpp 用 cJSON 驗它是合法 JSON）。
//   S2 以第一個 ';' 切成 lblChineseMsg／lblSubMsg（golden mymessbox.cpp:1032-1047）也在那裡。

<!-- preserved-content:end -->
```
