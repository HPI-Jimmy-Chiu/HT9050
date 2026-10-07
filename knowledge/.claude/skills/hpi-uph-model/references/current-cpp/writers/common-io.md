# 共用 WriteDataToFile

來源 pin `06fb64e540dff625f1f3af002179d56a2e49389e`：
[V906 common.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/06fb64e540dff625f1f3af002179d56a2e49389e/HT9011UC_Cpp_V3.33.906.0/common.cpp)、
[V906 common.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/06fb64e540dff625f1f3af002179d56a2e49389e/HT9011UC_Cpp_V3.33.906.0/common.h)、
[V912 common.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/06fb64e540dff625f1f3af002179d56a2e49389e/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/common.cpp)、
[V912 common.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/06fb64e540dff625f1f3af002179d56a2e49389e/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/common.h)。定位 WriteDataToFile、bOverWrite、pFile。

| 項目 | V906 | V912 |
| --- | --- | --- |
| 指標 overload | const char* 路徑／資料 | char* 路徑／資料 |
| AnsiString overload | 用 c_str() 轉呼叫指標 overload | 同樣轉呼叫 |
| common.h 預設值 | bOverWrite=false | bOverWrite=false |
| bOverWrite=true／false | fopen 的 w／a 模式 | 同樣模式 |
| pFile!=NULL | fputs(cData)、fputs 換行、fclose | 同樣步驟 |
| 開檔失敗 | 不進寫入區，void 返回 | 同樣返回 |

指定 UPH writer 傳 filename／t 兩個 AnsiString 引數，未明傳第三參數；相符宣告預設為追加模式。此處是來源 signature／呼叫表達式證據，沒有建置來證明實際連結版本。

兩版都沒有檢查 fputs／fclose 結果，也沒有在本體回傳成功值、flush／readback 核對、備份、原子替換或鎖定。void 返回與 caller 繼續執行，不能當保存成功證據。

helper 無 CSV 引號／分隔符 escaping。KYEC 的 t 已包含換行，helper 又寫一次換行；原碼有兩次換行輸出。text-mode 的最終磁碟位元組與檔案讀取結果未量測；不能把這份證據稱為 CSV consumer 已驗證。

只查本頁指定兩個 overload。EventLogAnalysis 的同名函式、RS232 的三參數 iSize overload 不是本頁的保存成功證據，未將它們混成同一個 writer。
