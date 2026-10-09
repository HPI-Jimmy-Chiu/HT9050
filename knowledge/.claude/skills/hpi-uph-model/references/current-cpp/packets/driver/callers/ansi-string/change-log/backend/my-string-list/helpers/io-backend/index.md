# 檔案讀取與 OS／CRT 寫入契約

承接 [IO 呼叫形狀](../io-boundary.md)。這層新增共用 reader 的完整正文，將既有 writer 呼叫、平台文件規格及部署／實機證據分開。

| 問題 | 文件 |
| --- | --- |
| CheckFileIsEmpty 的 bool、EOF 與關閉 | [讀取及 ownership](readers.md) |
| CreateFile sharing／定位／WriteFile／flush | [Win32 契約](win32.md) |
| CRT mode／回傳值／文字定位與緩衝 | [CRT 條件式契約](crt.md) |
| 版本、機型、編譯器與待續 | [範圍](limits.md) |
| 來源 pin／正文／官方來源 | [保存清單](source-manifest.json) |

新增 2 完整 cpp／5 原始 region，共 7 新摘錄、5 來源；5 個既有 writer／probe 正文只重核 context。原文及來源 byte、Git blob、body hash、原始 banner／宣告／建置入口皆保存。42 歷史 manifest、Skill metadata、資源與相容入口不改寫；canonical 已整合主題維持 22。

函式範圍限於 common.cpp 的 CheckFileIsEmpty／ReadDataFromFile；不是整個 common.cpp、RS232、upload 或 UPH 公式完成。外部規格以官方文件 URL 與查閱日定位，未保存大段文件原文；它們不是本機 C++、CRT 或機台驗證。

## PlateInfo caller與JSON ownership（20261009）

[完整選定reader caller／JSON入口](callers/index.md)：23完整CPP、11完整C與12region共46原文；2既有common reader僅context。LoadFile先Clear、parser／group／team與buffer／tree釋放、schema與靜態test_common覆蓋已分層；cinitial選定兩呼叫在#if 0，完整startup／ABI／部署CRT／實際IO與機台另續。

## RS232Standard獨立writer（20261009）

[rs232std writer契約](rs232-writer/index.md)：1完整CPP與6region共7新原文；desired access／share mode、CREATE_ALWAYS、iSize與void回報限制，歷史902版本及有限15檔spelling scan分開。CMake註冊非實際link，完整RS232／upload、部署IO與機型仍待查。
