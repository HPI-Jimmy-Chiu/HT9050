# StrToDateTime 與 caller 預設

定位 TDateTime.cpp 的 StrToDateTime；[manifest](source-manifest.json) 保存完整 body。前一層 TIniFile::ReadDateTime 的 found／Trim／IsEmpty 分支，沿 [typed API](../ini-typed-enumeration/typed.md)，不修改其既有查證正文。

本體先將 Y=1899、Mo=12、D=30、h=m=sec=0，再以 s.Trim().str() 複製為 std::string。第一式是 `%d%*[-/]%d%*[-/]%d %d:%d:%d`；兩個 suppressed scanset 可消耗一個以上 `-`／`/`，沒有另做來源註解所稱的 separator 轉空格操作。

| 分支 | 本體條件與輸出 | 證據界線 |
|---|---|---|
| 日期式 | sscanf 賦值數 n>=3，交 serialFromTm(Y,Mo,D,h,m,sec,0) | 日期完整賦值即接受；時間可不完整，未賦值項維持 0 |
| 時間式 | 第一式 n<3，另以 `%d:%d:%d` 賦值至少 2 項，轉 Word 交 EncodeTime | 秒可以未讀到；沒有要求完整消耗輸入 |
| 皆未達條件 | 回 TDateTime(0.0) | 沒有傳 caller def 或獨立錯誤狀態 |

兩式都沒有日期／時間範圍、尾隨字元、finiteness 或 scanf integer 範圍檢查。時間式的負值／過大值經 unsigned short Word 轉型，以及底層 scanf 超出 int 可表示範圍的行為，仍須逐平台契約查證，不能稱為合法日期。

ReadDateTime 在 !found 時回 def；found 後按 Trim().IsEmpty() 的結果決定 def，其餘交 StrToDateTime。現在可補出這個差異：到達 converter 的非空輸入若解析失敗，回 serial 0，並非直接回 caller def。這仍不是 IsEmpty、所有 constructor、完整 UI caller 或 BCB6 行為都已驗證。

本體沒有明示 throw／try-catch，不代表 std::string 分配或其他標準函式庫路徑不會拋例外。serialFromTm、EncodeTime 的數值運算與 Word 界線見 [serial](serial.md)；沒有執行有效／無效日期或檔案讀寫測試。
