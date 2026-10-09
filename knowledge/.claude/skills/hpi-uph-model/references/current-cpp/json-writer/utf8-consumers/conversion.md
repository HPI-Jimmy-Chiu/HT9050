# 固定950、process ACP與失敗路徑

[上層](index.md)；[八primitive的CP950／repair契約](../primitives/encoding.md)。

| wrapper完整原文 | 首次wide長度<=0 | 轉碼來源 |
|---|---|---|
| [ElaHub ToUtf8](raw/source-02.md) | 空字串 | 明確950、flags=0 |
| [WebBuilder U8](raw/source-04.md) | SanitizeToUtf8(s) | CP_ACP、flags=0 |
| [WebLogin U8Text](raw/source-05.md) | SanitizeToUtf8(s) | CP_ACP、flags=0 |
| [LotInfoFtp U8](raw/source-06.md) | 空字串 | CP_ACP、flags=0 |
| [RecipeChange U8](raw/source-08.md) | 空字串 | CP_ACP、flags=0 |
| [SmartDiag U8](raw/source-09.md) | SanitizeToUtf8(s) | CP_ACP、flags=0 |
| [宿主MbU8](raw/source-10.md) | SanitizeToUtf8(s) | CP_ACP、flags=0 |

先通過validator就回原字串：ElaHub用自己的本地body，其餘六個wrapper明確用webbridge共用body。
WebBuilder／MbU8原註解提到ANSI(cp950)，實際API參數是CP_ACP；
只在部署process ACP恰為950時才可套固定CP950說法，本輪未讀或修改機台的ACP設定。

上表Sanitize fallback只在第一次MultiByteToWideChar量wide長度wn<=0時呼叫。
Sanitize本身在Windows另試固定950／MB_ERR_INVALID_CHARS，通過完整有效檢查才使用，否則逐byterepair。
六個ACP wrapper的WideCharToMultiByte量UTF8長度<=0仍返回長度0的字串，沒有再次fallback。
ElaHub兩次量長度<=0也各自直接回空字串。
不能把「有Sanitize分支」擴大成所有API失敗都會修復。

這七個wrapper都沒有檢查實際wide／UTF8填寫API回傳值是否等於量出的長度，
也沒有在轉碼回傳前再IsValidUtf8。JsonWriter Cp950ToUtf8的完整長度檢查是另一份契約，不能套過來。
helper body未呈現_WIN32條件式，不推定這些consumer有JsonWriter相同的非Windowsrepair gate；
平台、shim、實際link與Win32錯誤行為另查，不由原API名稱宣稱跨平台實測成功。

AnsiString入口均先以a.c_str()構造std::string，所以內部NUL後byte不進這些body的轉換範圍。
[U8s](raw/source-07.md)又以s.c_str()建AnsiString後呼叫LotInfoFtp本地U8；內部NUL在此入口先截斷。
MbU8接受const char*，NULL先變空字串，非NULL也是C字串長度。
ElaHub ToUtf8直接接std::string，以data()及s.size()交API；body處理範圍包含內部NUL，
但能否供後續UI／JSON完整保留還須核其caller，不能只從此helper推出。
