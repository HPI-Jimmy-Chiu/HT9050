# 物件綁定、memory 與保存時機

定位 IniFiles.cpp 的 TIniFile constructors、destructor、ReadString、ReadInteger、WriteString、UpdateFile、flush；宣告與歷史註解保留在 [manifest](source-manifest.json) 的完整 IniFiles.h。

| 路徑 | 完整 body 可確認的行為 | 尚未確認 |
|---|---|---|
| TIniFile(fileName) | FileName 綁路徑、writeThrough_=true；constructor 不預先載入 store_ | caller 是否重用／改 FileName，完整 ownership |
| TIniFile(fileName, MemTag) | writeThrough_=false；呼叫 store_.LoadFromFile | memory store 的全部解析／保存實作 |
| ReadString／ReadInteger | writeThrough_ 選磁碟或 store_.ReadRaw；found 分開處理缺值 | 全部 typed read 與 caller |
| WriteString | false 選 store_.WriteRaw；true 呼叫 W906_Win32WriteProfileString | Fast writer 與 store_ 的完整 callee |
| UpdateFile | 只有 !writeThrough_ 時呼叫 flush | caller 是否執行此方法、磁碟是否成功 |
| flush | store_.SaveToFile(FileName, !writeThrough_) | SaveToFile 的完整 body |
| ~TIniFile | 完整 body 沒有 flush | TMemIniFile 自己的 destructor 尚未在本單元選讀 |

Aux 的 SaveSetupData 用 TIniFile 保存後 delete；連到本層時，write-through 的 WriteString 已呼叫磁碟 writer，base destructor 不另行重試或回報。表單完整來源見 [既有映射](../form-settings/mapping.md)。

IniFiles.h 的「Default VERBATIM」「不重現 cap」「destructor safety net」等是保存的歷史說明。當前 ReadString 的磁碟缺值走 W906_Win32ProfileDefault，與 memory 路徑直接回 def 不同；實作證據見 [讀取](read.md)。不能把 header 的歷史宣稱當成本輪 BCB6、V912 或實機保真度結果。
