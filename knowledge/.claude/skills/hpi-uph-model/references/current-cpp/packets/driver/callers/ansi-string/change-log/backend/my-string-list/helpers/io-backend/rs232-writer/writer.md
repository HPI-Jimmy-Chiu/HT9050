# rs232std::WriteDataToFile的寫檔契約

以 `TesterComm/Rs232/Rs232Globals.cpp` 的
`WriteDataToFile(char* cFilePath, char* cData, int iSize)`、`Fp`、`wtfz`定位。
它回傳void，使用Win32 API；與共用全域writer的bool或AnsiString參數是不同介面。

## 開檔參數必須依位置判斷

原文為 `CreateFile(cFilePath, FILE_SHARE_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)`。
第二個參數是desired access，第三個才是share mode；因此這裡的share mode實際為0。
Windows列 `FILE_SHARE_WRITE` 為0x2、`FILE_WRITE_DATA` 為2，該bit放在access位置可表示寫資料權限；
不可依常數名字寫成「已允許其他writer共用」。share mode 0的排他限制以API成功開檔為前提。
見 [CreateFileA參數](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea)
與 [File Access Rights](https://learn.microsoft.com/en-us/windows/win32/fileio/file-access-rights-constants)。

`CREATE_ALWAYS` 在API規定條件下建立或截短既有檔案；本體沒有seek到尾端、append選項、
備份或transaction。因此不能套用共用writer的append契約。
此為原文與 [CreateFileA建立模式](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea)
的靜態推導，未執行覆寫或共用鎖測試。

## 長度由iSize直接給API

| 本體觀察 | 契約界線 |
| --- | --- |
| `WriteFile(Fp, cData, iSize, &wtfz, NULL)` | 傳入要求byte數與buffer，沒有strlen、字串拼接或換行追加 |
| API參數型別為DWORD；這裡來源為int | 本地沒有負值／上限／buffer容量guard；不能把任意iSize視為安全 |
| `wtfz`接收實際寫入數 | 本體沒有比較它與iSize，也沒有重試或補寫 |
| `cData`為char* | 這次call shape不以NUL找長度，但有效buffer及實際binary caller尚未核實 |

[WriteFile規格](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile)
將要求byte數、實際寫入數與BOOL結果分列；本函式只傳出參數，沒有利用結果建立成功契約。
不把管線的部分寫入案例套成這個普通檔案的已觀察行為。

## 可見失敗分支與呼叫端資訊

只檢查 `Fp!=INVALID_HANDLE_VALUE`；開檔失敗即走到void函式尾端。
開檔成功後執行WriteFile再CloseHandle，兩者回傳值都沒有檢查，
沒有GetLastError、錯誤notice或成功值傳給caller。
由於截短發生在寫入前，不能由「函式返回」推論舊檔已保留或新資料已完整寫入；
若後續寫入失敗，這個本體也沒有復原路徑。這是條件式靜態限制，並非機台故障量測。
本體沒有FlushFileBuffers或原子替換，不能宣稱已提供耐久落盤或crash一致性。

完整原文與註解保持在 [manifest](source-manifest.json)；未修正任何原始碼。
