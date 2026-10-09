# CRT：來源實參與條件式平台對照

[上層](index.md)；common 的 2 個 WriteDataToFile overload 已完成，本輪只重核；[reader](readers.md) 是新正文。下列 Microsoft Learn 規格適用其 CRT，不當成已確認的 MinGW、BCB6 或部署機台行為。

來源 writer 用 fopen("w"／"a")，readers 用 "r"，都沒有明寫 "b"／"t"／"ccs="。Microsoft CRT 中 w 截斷／建立，a 追加，r 讀取；未指定 t/b 時由全域 _fmode 決定翻譯模式。若是 text mode，讀入 CRLF 轉 LF、輸出 LF 轉 CRLF，輸入 CTRL+Z 也可能作 EOF。因此本輪不直接宣稱來源的 "r"／"a" 必定是 text mode，也不宣稱所有輸出 bytes 是 UTF-8 或 Big5。[fopen](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fopen-wfopen?view=msvc-170)。

| 檢查點 | 已保存來源與 Microsoft CRT 規格 |
| --- | --- |
| writer 開檔 | 只測 pFile!=NULL；失敗沒有回傳值通知 caller。 |
| fputs(cData) 與 fputs("\n") | 兩次呼叫的結果未核；規格以非負／EOF 表示成功／錯誤，不寫字串末端 NUL。它不是 caller 任意 byte buffer 的 length 介面。 |
| fclose | 結果未核；規格是 0／EOF。stream buffer 會在關閉前 flush，但不能據此等同全套 Win32／硬體持久性證據。 |
| ftell | source long file_size 未核；Microsoft 文件指出 text mode 位置未必反映實體 byte offset，宜作 fseek 的位置值；不能僅用此值當已解碼字串長度。 |
| fread(file_buf,file_size,1,pFile) | 回傳完整 item 數，不是 byte 數；來源未看回傳，也未以 feof／ferror 分辨。size=0 時規格回傳 0 且不改 buffer；部分讀取 item 的值也不能當已建立完整內容。 |

writer 的兩次 fputs 未取得一次整筆 record 成功保證；其 void 介面不能表示錯誤或 durability。既有 TStringList::GetText 已知 CRLF 與 CRT 的條件式翻譯不能混成同一層。[fputs](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fputs-fputws?view=msvc-170)、[fclose](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fclose-fcloseall?view=msvc-170)。

文字模式下，ftell 的位置與 fread 的 CRLF 轉換可能不等於來源計算的 file_size 個字元；source 把 NUL 放在預計位置，沒有核實際讀取內容。此為規格與源碼的靜態推論，未用任何測試檔重現，也沒有把 text／binary 差異當成目前客戶 log 的確診原因。[ftell](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/ftell-ftelli64?view=msvc-170)、[fread](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fread?view=msvc-170)。

兩次 fseek 結果也未核；Microsoft 規格有成功／錯誤回傳與 text mode 限制。文件即使有各 CRT 函式內部鎖，也不提供這一整串 seek／tell／malloc／read 的交易鎖或檔案不變保證。[fseek](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fseek-fseeki64?view=msvc-170)。

UTF-8 C++ 原始碼、narrow AnsiString 內容、pathname 解碼、stream 翻譯與機台最終 log encoding 是不同證據；本輪沒有改 locale／_fmode、取得實際 DLL link、執行 reader／writer 或比較 log bytes。
