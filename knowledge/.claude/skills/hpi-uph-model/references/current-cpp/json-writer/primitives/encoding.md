# UTF-8檢查、CP950轉換與逐byte修復

[上層](index.md)；[原policy](raw/source-09.md)；[Win32 gate](raw/source-11.md)。

`IsValidUtf8(s)`與`RepairUtf8(s)`均以i<s.size()迴圈呼叫`Utf8SequenceLen(s,i)`。
後者直接讀s[i]，正文沒有先驗i<n；以上兩個選定caller有界線，不推定其他caller條件。

| Utf8SequenceLen正文 | 結果 |
|---|---|
| c0<0x80 | 1，ASCII |
| c0<0xC2 | 0，continuation或C0／C1 overlong開頭 |
| C2..DF／E0..EF／F0..F4 | need=1／2／3，逐個檢查continuation並組cp |
| F5..FF、i+need>=n、continuation格式不符 | 0 |
| 依need檢查overlong、cp>0x10FFFF、D800..DFFF | 0 |
| 通過 | need+1 |

[完整Utf8SequenceLen](raw/source-01.md)；[IsValidUtf8](raw/source-04.md)。
IsValidUtf8遇len=0回false，否則i+=len；空字串跳過迴圈回true。

[RepairUtf8](raw/source-02.md)在無效span附加[kReplacement](raw/source-10.md)的三個byte，
然後只++i；有效span則append(s,i,len)並i+=len。
因此是逐個無效byte替代，不是一次替代整個原錯誤多byte片段，也不保證輸出長度等於輸入。

[SanitizeToUtf8](raw/source-05.md)的順序：空字串原樣回傳；全字串有效UTF-8原樣回傳；
只在_WIN32下嘗試Cp950ToUtf8(raw,&converted)，且轉換結果還要通過IsValidUtf8；
其餘走RepairUtf8。CP950是固定950，不讀GetACP或process ACP，並非偵測所有來源編碼。
混合資料也先按全字串契約判斷，正文沒有逐個欄位或混合編碼探測。

[Cp950ToUtf8](raw/source-03.md)在Win32 gate內，先用MultiByteToWideChar(950,
MB_ERR_INVALID_CHARS)量長度，再用明確s.size()轉wide；任何wide長度<=0或結果不相等回false。
之後WideCharToMultiByte(CP_UTF8,0)先量長度，再轉換；亦要求正長度與結果相等。
只有最後成功才out->swap(utf8)並回true。這個helper不自行驗out指標；選定Sanitize caller傳本地物件地址。
明確長度含輸入內部NUL的byte範圍，但Windows API／配置失敗與平台實作未於本輪執行。
不要把來源註解的「Never emits invalid UTF-8」改稱本輪browser、機台或跨平台實測。
