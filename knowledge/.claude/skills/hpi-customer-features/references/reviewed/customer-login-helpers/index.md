# 密碼本路徑、格式判斷與轉碼 helper

接續[登入結果／MBox](../customer-login/index.md)，只補 V906 六個指定函式 body／namespace 與 Image 型別；來源、blob、編譯條件及日期見[manifest](source-manifest.json)。同題供 HT9050／其他 Handler 查閱，作用中機型、客戶、版本、建置與 runtime 仍分開核對。

| 路由 | 已讀內容 | 仍未閉合 |
| --- | --- | --- |
| [路徑與模式](paths.md) | pw／reauth BookPath、WebLogin_UsesBook、前層 caller 的模式表達式 | 全部 pwPath／環境／旗標寫者、其他 caller 與 UI 同步 |
| [override 格式判斷](binary.md) | W906_PwBookBinaryOverride、logindat::Read、Image／kSize | 解碼／結構與語意、預設 binary 讀取、例外及完整consumer |
| [轉碼與 fallback](encoding.md) | pw::Acp 及前層 Typed caller 的分工 | 平台API／CP_ACP組態、其他caller、字串constructor與完整登入效果 |

本層是 V906 靜態 helper；V912 的 UI／密碼本路徑仍讀前層已釘住的指定 body，不把這些 C++ helper 套給 BCB6。沒有讀實際環境、密碼本或帳密，未執行API、build、runtime或機台；原候選與人工客戶列不增加。

20261007 後續局部證據見[讀取／解碼／分欄與字串](../customer-login-readers/index.md)：補 V906 名稱清單 reader、TStringList 與 narrow 字串操作，以及 V906／V912 兩個算法 body 對照。此處原來未閉合的完整 reader／consumer／認證與磁碟效果仍未宣稱完成。
