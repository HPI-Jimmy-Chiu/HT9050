# override 的二進位探測與固定長度讀取

來源：[WebLogin.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/76dd45f370a180f90917a13d4ceea3d41d7ad71e/HT9011UC_Cpp_V3.33.906.0/WebLogin.cpp)的 `W906_PwBookBinaryOverride` 及[LoginDatBook.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/76dd45f370a180f90917a13d4ceea3d41d7ad71e/HT9011UC_Cpp_V3.33.906.0/LoginDatBook.h)的 `logindat::Read`／`Image`／`kSize`；只讀這個override探測路徑，完整解碼／檔案consumer尚未查完。

BinaryOverride先拒絕空ov，再以logindat::Read讀Image；Read失敗回false，成功後只要Image任一byte等於0就回true，沒有0則false。Image是vector<unsigned char>；副檔名不是這個body的分類條件，true也不表示帳密或所有record內容正確。

Read先clear輸出img，fopen(path, rb)失敗回false；建立kSize+1的本機buf，fread最多該長度、fclose，只有n==kSize才resize及swap入img並回true。其他讀取數量直接false，輸出仍是起初清空的img。這是固定長度讀取／搬移body，不等於完整資料結構或解碼驗證；未讀實際檔案、製造樣本或執行此helper。

前層BookCompare的binary條件另有CosFunction.bUseLoginDatToSetLevel且bookOverride為空的分支。其預設binary reader直接fread(USER.RecordCT)，不是此Read；不能把override的kSize檢查推給所有密碼本讀取路徑。[前層讀取界線](../customer-login/book.md)仍保留原查證日期與pin。

後續需實際解碼／結構、所有讀取caller、檔案與例外狀態及最終登入／consumer效果；版本／客戶／機型／runtime分開，沒有改程式或設定。
