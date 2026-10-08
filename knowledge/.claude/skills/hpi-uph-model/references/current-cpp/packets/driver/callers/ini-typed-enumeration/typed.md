# Typed 讀寫、found 與轉換界線

定位 IniFiles.cpp 的 parseFloatDef、TIniFile::ReadFloat／ReadBool／ReadDateTime、WriteInteger／WriteBool／WriteDateTime；完整 body 見 [manifest](source-manifest.json)。綁定、磁碟 raw 的 2048-byte 限制及 parseIntDef 已沿 [共用讀取](../ini-core/read.md)／[binding](../ini-core/binding.md)，不重複算成本單元新查證。

## 讀取與預設

三個 Read 方法都依 writeThrough_ 選 W906_DiskIniReadRaw(FileName,section,ident,found) 或 store_.ReadRaw；!found 直接回 def。有 key 但值為空、無 key 與讀檔失敗，要依 raw／found 與各 typed parser 分開判讀；不推成相同 UI 提示。

- ReadFloat 呼叫 parseFloatDef(v.str(),def)。parser 兩端 trim unsigned byte<=0x20，全空回 def；substr 後以 strtod 轉換，end==p 或 *end!=NUL 回 def，否則回 v。完整 body 沒有 errno、範圍、finiteness 或 locale 檢查；end 是 C-string 判斷，不能推成嵌入 NUL／溢位／跨平台浮點完整保真。
- ReadBool 呼叫 parseIntDef(v.str(),def?1:0)，再以 iv!=0 回傳。它不是逐字辨識 true／false 的 parser；十六／十進位與失敗預設沿 parseIntDef，int 範圍與 long 寬度仍未驗證。
- ReadDateTime 在 found 後，以 v.Trim().IsEmpty() 決定空值預設；其餘直接 StrToDateTime(v)。本方法沒有 try／catch，也沒再次傳 def 給 converter。尚未讀完 Trim／StrToDateTime／TDateTime 深層鏈，不能聲稱所有無效日期都回 def、會拋何種錯誤或接受何種格式。

## 寫入綁定

WriteInteger 將 value 傳給 AnsiString constructor，再交 WriteString；WriteBool 傳 value?1:0；WriteDateTime 先 DateTimeToStr(value)，再交 WriteString。constructor／日期 formatter 的精度、locale、格式與 round trip 仍待續，不以註解「decimal／0或1」替代本輪完整 callee 查證。

WriteString 的 writeThrough_ 磁碟 writer 或 memory WriteRaw 綁定，沿 [共用 writer](../ini-core/write.md) 與 [store 保存](../ini-writer-store/store.md)。typed 方法的存在不等於磁碟寫入成功，memory mutation 也不等於已 flush。
