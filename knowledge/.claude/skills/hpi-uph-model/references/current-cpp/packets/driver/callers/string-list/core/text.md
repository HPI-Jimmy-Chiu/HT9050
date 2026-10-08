# Text 分行與 GetTextStr

定位`GetText`／`SetText`／`SetTextStr`／`GetTextStr`、`TextProxy::operator AnsiString`及兩個文字指派、`items_`／`objects_`／`textCache_`。完整原文與CRLF歷史註解見 [manifest](source-manifest.json)。

## GetText 與 SetText

GetText以std::string累加每項 `.str()`，**每項後追加CRLF，包括最後一項**；空清單回空文字。header宣告旁的bare-LF簡述完整保存，行為判斷依cpp body。
SetText先清兩vectors，再逐byte讀v.str()：LF結束一項；CR後接LF時略過CR、由LF結束；lone CR也結束一項。每個換行都可保存當前空行。迴圈結束只追加非空殘行，最後換行不額外產生尾端空項。每個新項的Objects均為0，最後syncCount。

下表是**逐body靜態推導，沒有執行程式**。JSON樣式只表示字串內容；index與Count依正常完成路徑。

| SetText輸入 | 清單結果 | 再GetText |
| --- | --- | --- |
| `""` | `[]` | `""` |
| `"A"` | `["A"]` | `"A\r\n"` |
| `"A\r\n"` | `["A"]` | `"A\r\n"` |
| `"\n"` | `[""]` | `"\r\n"` |
| `"A\n\nB"` | `["A","","B"]` | `"A\r\n\r\nB\r\n"` |
| `"A\rB\r"` | `["A","B"]` | `"A\r\nB\r\n"` |

內部NUL在這兩個std::string迴圈不作停止符；char*入口另經AnsiString構造、對外c_str consumer如何處理仍要個別查。items中本身含CR／LF時，經Text重新解析可能改變項數，不能承諾任意字串清單完整round-trip。
TextProxy的同型指派先把來源序列化成AnsiString，再SetText，包含上述分行與Objects重設。SetTextStr同樣先包AnsiString(p)；char*與NUL／null的完整契約另沿AnsiString來源續查。

## GetTextStr 快取

body把`GetText().str()`寫入list的textCache_，再回textCache_.c_str。下一次GetTextStr會改寫這份cache；list銷毀也會結束其儲存生命期。
選定Add／Insert／Delete／Clear／SetString／Assign／SetText並沒有主動清或重建textCache_；修改後舊cache可仍是前次快照。header的valid-until-next-mutation註解保留，但本段不提升為每個mutator都更新cache或可跨呼叫保留指標的保證。
精確指標失效、capacity與所有caller的使用生命期需另查或測試；此處沒有實測。

CRLF註解中的20260807寫入失敗與歷史fixture只作保存證據；本輪沒有跑WriteFile、SaveToFile、fixture或機台。
回 [入口](index.md) 與 [代理快取](proxies.md#stringsproxyc_str)。
