# 分隔文字解析與 cells

來源：[TStringList.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/42323e80ed44da6cb62b7482227364aae221e72c/HT9011UC_Cpp_V3.33.906.0/vclcompat/TStringList.cpp)；[TStringList.h 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/42323e80ed44da6cb62b7482227364aae221e72c/HT9011UC_Cpp_V3.33.906.0/vclcompat/TStringList.h)。讀 `CtCharNext`、`CtIsBlank1`、`CtScan`、`CtStrEnd`、`CtMove`、`CtExtractQuoted`、`CtSetDelimited`、`SetCommaText`、`SetDelimitedText`、`CommaTextCells`／`CommaTextCell`。完整原文見 [manifest](source-manifest.json)。

## 解析順序

`CtSetDelimited` 由 `Value.c_str()` 起步，到第一個 NUL 停止。先跳過非 NUL 且 unsigned byte ≤空白的字元；未加引號項到空白或 Delimiter 截止。加引號項走 `CtExtractQuoted`，相鄰雙引號解成一個引號。項目後跳過空白、吃一個分隔符、再跳空白，然後重複。

helper 對 `out`／可選 `cells` 做 append，本身沒有 Clear。`SetDelimitedText` 用新 vector 先解析，再呼叫 qualified `TStringList::Clear()`，逐項虛擬 `Add`。基底 Add 會放 null Objects，派生 Add 的效果需另讀 caller。qualified Clear 的歷史理由是避免 TRadioGroupItems 在最終 Count 尚未建立前 clamp；vclcompat 沒有完整 OnChange／Sorted／Duplicates／EndUpdate 模型。

`SetCommaText` 先永久指定 Delimiter=逗號、QuoteChar=雙引號，再呼叫 `SetDelimitedText`。這不是暫存欄位後還原的 setter。

## 正文逐分支的靜態例子

以下用 ASCII 逗號與雙引號；`[]` 表零項，`[""]` 表一個空項。未跑 fixture／BCB oracle。

| 輸入文字 | 清單值 | 判斷位置 |
| --- | --- | --- |
| 空字串或只有空白 | `[]` | 第一個 while 跳過空白後已到 NUL |
| `07 Tester I/F` | `["07","Tester","I/F"]` | 未加引號項也在空白分界 |
| `a,b,` | `["a","b",""]` | 分隔符下一 byte 立即為 NUL |
| `a, ` | `["a"]` | 分隔符後有空白，沒有建立尾空項 |
| `a,,b` | `["a","","b"]` | 下一輪從分隔符本身建立空項 |
| `"ab"cd,e` | `["ab","cd","e"]` | 關閉引號後的文字成下一項 |
| `"a""b",c` | `["a\"b","c"]` | adjacent quote pair 折成一個雙引號 |
| 未關閉的 `"abc` | `["ab"]` | Src 到 StrEnd，再少取一個 byte |
| 只有 `"` 或 `"a` | `[""]` | Src−P ≤1，回傳空值 |

未關閉引號少掉的是 **byte**，UTF-8 尾字元可能因此截斷。`CtMove` 只寫入目的字串有效範圍；原始註解所述 BCB R01／R02 越界或例外是歷史對照，不能據此宣稱任意 malformed input 與 BCB 完全一致。

## CommaTextCells 是 port-only API

`CommaTextCells` 先 `out.clear()`，以新 items vector 呼叫同一 helper，固定逗號／雙引號。`b`／`e` 為原值 byte 半開區間 `[b,e)`；`v` 是解碼後項值，`quoted` 取項首 byte 是否等於 QuoteChar。空白與分隔符本身通常在區間外；引號項包含開、關引號。

| 原值 | cell 靜態結果 |
| --- | --- |
| `a,,b,` | `a:[0,1)`；空項 `[2,2)`；`b:[3,4)`；尾空項 `[5,5)` |
| `"a,b",c` | quoted `a,b:[0,5)`；未引號 `c:[6,7)` |

offset 不可直接當 UTF-16／Unicode code point 位置。header 所列 `WebMotorAccess.cpp MtSplitCells`／Motor Test 存檔是歷史 caller 提示；本單元沒有新增 caller 閉環或存檔實測。

回 [入口](index.md)、[輸出](rendering.md)、[界線](limits.md)。
