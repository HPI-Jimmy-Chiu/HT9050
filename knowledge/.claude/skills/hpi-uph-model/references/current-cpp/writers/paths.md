# V912 的每日目錄與 FileInfo

來源 pin `06fb64e540dff625f1f3af002179d56a2e49389e`：
[FileInfo.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/06fb64e540dff625f1f3af002179d56a2e49389e/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/ProductionInfo/FileInfo.cpp)、
[FileInfo.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/06fb64e540dff625f1f3af002179d56a2e49389e/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/ProductionInfo/FileInfo.h)。本頁只查 V912 指定兩個方法，不聲稱 V906 FileInfo 等價。

RecordLotUPH_For_FOREHOPE_NINGBO／RecordLotUPH_For_VTEST 先 DecodeDate(Now())，用 as9045UPH、year／month／day 組 sSaveFolder；資料夾格式為 %d，檔名為 %02d%02d%02d_UPH.csv。%02d 是最小欄寬，不能把年欄誤寫為固定兩位年。

EnsureDirectoriesExist 是 void：

- 入口空字串直接返回；沒有尾端反斜線時，用 GetFileAttributes 判斷已存在檔案才 ExtractFilePath。
- attributes 無效時，只有 .txt／.csv 副檔名會被當檔名剝除；其他含點名稱不由這段直接剝除。
- 去掉尾端反斜線後，按反斜線逐層取 currentPath；attributes 無效則嘗試 CreateDirectory。
- CreateDirectory 失敗就返回；指定 caller 仍繼續組 filename 與呼叫 WriteDataToFile，沒有取得目錄建立成功值。

PathCombin 以 sPath 起始；任一正斜線存在就用正斜線分支，否則依尾端與 sFile 起始反斜線決定是否補反斜線，最後加 sFile 並返回。它是字串拼接，不在本體查檔、建立目錄或回報寫檔結果。

未查 as9045UPH 的所有賦值、磁碟權限／編碼、競態／重試、Now／DecodeDate 的平台與跨午夜結果、FileExists 的實際 dispatch 或所有路徑 consumer。不能由 EnsureDirectoriesExist 的名稱保證目錄存在。
