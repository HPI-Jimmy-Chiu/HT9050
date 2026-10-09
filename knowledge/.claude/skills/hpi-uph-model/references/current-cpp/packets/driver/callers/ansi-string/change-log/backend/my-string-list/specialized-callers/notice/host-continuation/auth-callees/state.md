# 密碼本與狀態JSON使用不同的判斷

[上層](index.md)；[DoPassword](password.md)；[原文](source-manifest.json)。

## BookCompare讀取與結果

WebLogin_BookCompare用非空bookOverride，否則pwPath。缺檔或binary fopen失敗
回NO_BOOK與msg，未在這兩個早退重設AccessLevel。
binary條件含bUseLoginDatToSetLevel與override helper；fread到PASS_WORD後
掃1000項，正文未檢查fread結果。text用TStringList::LoadFromFile，
每行以512buffer截斷、SplitStrByDotSpaceOnly抽ID／level／password，依旗標DecodeStr。
parser、binary override與DecodeStr完整callee仍待查，不宣稱完整格式或錯誤處理已驗。

逐項以user及password的UpperCase比較，JCET_FOR_EVAN==1可略ID相等條件；
level用atoi，只接受0～3。match後設AccessLevel／itemIndex、姓名、caption，
ChangeLevelAttr與NewRecordProcess，再回OK。這個callee不負責設Login按鈕為Logout，
原BookLogin與BookCompare分工comment保留。
全無有效match則重設Operator各狀態，回BAD_CREDENTIALS與WAR1677；
此函式沒有直接ShowErrorMessage，頁面實際如何顯示仍待查。
沒有讀取任何實際密碼本、帳密、解密資料或呼叫這些副作用。

## 三個模式判斷不可互換

reauth::BookPath優先W906_PWBOOK_PATH，否則pwPath；BookOverride優先同一override，
否則空字串。只保存getter程式正文，本輪沒有讀取環境值。
Typed先pw::Acp轉成AnsiString；失敗時用u.c_str()直接建字串，完整編碼callee未核。

| 來源位置 | 條件 |
|---|---|
| DoPassword mode | FileExists(reauth::BookPath()) |
| WebLogin_UsesBook／StateJson mode | FileExists(pwPath)或bUseLoginDatToSetLevel |
| BookCompare選檔／binary模式 | override或pwPath，再依binary flags／helper |

因此reply的login.mode未必等於本次DoPassword所走分支；尤其override或binary旗標
與檔案存在性不同時，需以各函式條件核對，不能只依UI的mode文字推定權限流程。

## StateJson是手工字串拼接

WebLogin_StateJson輸出mode、live AccessLevel、itemIndex、fMain user-select Text、
s_userCaption、固定四個items、btLogin文字及SystemStart。
沒有把password或credential參數直接放入，但userCaption可依BookCompare設成UseName，
所以AuthVerify自己userId=null不代表login物件完全不含使用者識別資料。

兩個動態文字直接接在引號內，這份函式沒有escape函式呼叫；
實際允許字元、JsonWriter::RawValue如何處理不合法JSON與reader端效果待續，
不宣稱本輪已重現quote／control-character的runtime錯誤。
fMain／cbUserSelect在此直接解參照，完整初始化與NULL責任未追完。
