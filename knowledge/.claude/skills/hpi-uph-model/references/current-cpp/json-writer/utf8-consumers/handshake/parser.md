# ParseHttpRequest：有限header parser

[上層](index.md)；[ParseHttpRequest](raw/source-18.md)、[HttpRequest::Clear](raw/source-12.md)；[原header契約](raw/source-28.md)。

每次先out.Clear()，consumed指標若非空先置0；data==0回NeedMore，不繼續讀len或檢cap。
先在caller傳入的byte範圍掃第一個CRLFCRLF；未找到且len>=maxHeaderBytes回TooLarge，否則NeedMore。
找到後blockLen=end+4，只有blockLen>cap才TooLarge，所以含terminator恰等cap可以繼續。
cap預設由header給16KiB。scan發生在未找到的cap判斷之前；沒有為scan配置header副本，
但caller已累積的buffer、read loop成本與全程序記憶體上限不由此函式證明。

通過cap才複製std::string(data,end)，consumed隨即設blockLen，之後解析仍可Bad。
因此consumed非0不等於成功；out也可能已部分填入，caller必須先看回傳值。
只解析第一個header block，後續body／pipelined frame留給caller，沒有讀取Content-Length正文。

以CRLF分行；行內bare CR／LF拒絕。非首行若以SP／HTAB起頭且已有前行，
把TrimOws後的內容以單一空白接到前行；首行起頭OWS拒絕。這是本body的obs-fold處理，
不是全HTTP request smuggling或跨proxy一致性的驗證。
request line以頭兩個SP切METHOD／TARGET／VERSION，method與target不可空，
version不可含SP且須以HTTP/起頭；它不在這裡要求GET、HTTP/1.1或完整method token。
後續upgrade checker才要求特定method／version；靜態檔與network owner要另查。

target在第一個?拆rawPath與query，分別去掉#fragment；path做UrlDecode(rawPath,false)，
所以path中的+保持+。這裡沒有呼叫PathIsSuspicious、檔案canonical confinement或機台互鎖。
header找第一個colon，名字非空且整串IsToken才接受；轉ASCII小寫名字、TrimOws值後保留vector次序。
空白行忽略；缺colon／名字非法則Bad。不驗Host必填、值所有control、body length、Origin或登入權限。
函式完成返回Ok只證明這套header parser條件，不能稱為對所有HTTP規範的完整認證。
