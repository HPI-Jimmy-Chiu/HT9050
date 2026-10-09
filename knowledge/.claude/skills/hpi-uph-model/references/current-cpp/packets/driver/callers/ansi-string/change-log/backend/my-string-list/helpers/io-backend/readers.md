# CheckFileIsEmpty／ReadDataFromFile

[上層](index.md)；定位 `common.cpp` 的兩個定義及 `common.h` 宣告；完整原文見 [清單](source-manifest.json)。

| CheckFileIsEmpty 路徑 | 正文 |
| --- | --- |
| 進入 | bResult=true；fopen(cFilePath.c_str(),"r")。 |
| 開檔且第一字元不是 EOF | 維持 true；沒有掃完整檔案。 |
| 第一字元是 EOF | 設 false；沒有呼叫 feof／ferror 分辨空檔與讀取錯誤。 |
| 開檔失敗 | 沒有分支內讀取，仍在分支外 fclose(pFile)，此時 pFile 為 NULL；不能把初值 true 當成保證返回。 |

函式名稱不代表「true=空」。它也不是 FileExists 或檔案成功寫入判定；沒有取得能覆蓋後續 writer 的 handle。原始 translator banner 稱此為 preserved golden bug，只作歷史註記，不是新 BCB6／913 實測。

Microsoft CRT 規格中 fgetc 的 EOF 可來自串流結束或錯誤，需其他旗標分辨；fclose(NULL) 會進入 invalid-parameter 處理。這是 Microsoft CRT 條件式對照；目前 MinGW／部署 CRT 的失敗路徑未執行，不能斷言一定 crash 或一定正常返回。[fgetc](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fgetc-fgetwc?view=msvc-170)、[fclose](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fclose-fcloseall?view=msvc-170)。

## ReadDataFromFile 的狀態順序

1. fopen(...,"r")；只有 NULL 時直接 return NULL。
2. fseek 到 SEEK_END，再以 long file_size 接 ftell，fseek 回 SEEK_SET；兩次 seek 與 tell 結果未核。
3. malloc(file_size+1)；沒有判負值、加法範圍、配置失敗或 NULL。
4. fread(file_buf,file_size,1,pFile)，大小是單一 item=file_size、count=1，未使用回傳的 item 數。
5. 在預計的 file_size 索引放 NUL，fclose 後 return file_buf；並非按實際已讀字元位置終止。

來源沒有把 buffer 初始化，也沒有用實際讀取量重設終止位置。遇到錯誤、較短讀取或文字模式轉換時，不能由非 NULL 回傳推定內容完整且都是已初始化字元；空檔的 size=0 也是獨立邊界。這是靜態缺口，未觸發失敗。

buffer 由 malloc 配置、與已關閉的 FILE 分離；caller 的釋放與所有權處理仍須逐一查證。Microsoft malloc 文件說明其配置及 free 配對；本輪沒有保存完整 caller，因此不宣稱目前程式沒有 leak，也不推定可用 delete[] 釋放。[malloc](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/malloc?view=msvc-170)。

ftell、fread 的文字模式位置與結果界線見 [CRT](crt.md)；來源使用 long 而非文件專用的 64-bit tell 入口，但未依名稱推定部署 ABI 的 long 寬度。本輪沒有建立檔案、呼叫 reader 或修正來源。
