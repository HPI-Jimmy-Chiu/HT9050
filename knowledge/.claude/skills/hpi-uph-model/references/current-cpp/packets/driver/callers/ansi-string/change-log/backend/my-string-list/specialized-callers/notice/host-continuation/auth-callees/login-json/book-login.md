# BookLogin：比較結果與按鈕狀態分工

[上層](index.md)；[已保存BookCompare](../state.md)；[原文](raw/source-02.md)。

`WebLogin_BookLogin` 先看s_btLoginIsLogout；為true時寫msg並回WEBLOGIN_ALREADY_IN，
這條路沒有呼叫WebLogin_BookCompare。源碼comment對應golden按鈕顯示Logout時應先登出，
本callee本身沒有執行登出。

其他路呼叫BookCompare，只有r==WEBLOGIN_OK才寫s_btLoginIsLogout=true，最後直接回r。
因此BookCompare比對並更新權限，不等於Login按鈕已進Logout狀態；
舊正文沒有寫這旗標，這份wrapper補上成功時的狀態轉換。
此函式沒有在各失敗分支寫false，也沒有在already-in早退重設AccessLevel。

早退直接解參照msg，未在此函式guard空指標；有效生命週期由caller負責，完整caller未查完。
二進位／文字本parser、使用者名稱／caption、登出回復與browser consumer另續，
不讀實際password book或執行任何登入／登出。
