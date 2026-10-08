# FormatDateTime 與 DateTimeToStr

定位 TDateTime.cpp 的 FormatDateTime／DateTimeToStr；[manifest](source-manifest.json) 保存完整 body。FormatDateTime 先 DecodeDate／DecodeTime，取 fmt.str()，用 std::string out 與 char tmp[16] 產生文字；數值界線沿 [serial](serial.md)。

| token run（大小寫不分） | 本體輸出 |
|---|---|
| y | run>=4：%04d 年；其他 run：year%100，以 %02d |
| d／h／n／s | run>=2：%02d；其他：%d。n 一律 minute |
| z | run>=3：%03d 毫秒；其他：%d |
| m 判成 minute | run>=2：%02d 分；其他：%d |
| m 判成 month | run>=3：固定英文 Jan..Dec，month 不在 1..12 用空字串；run2：%02d，run1：%d |

run 是連續相同 token 的計數，不是截斷長度；mmmm 仍走固定英文縮寫，不能描述為完整月份名稱。標準函式庫格式化、locale 與整數提升的全部平台契約未測。

## month/minute 判斷

m 從目前位置往前掃：遇 h 判 minute；遇 y/d/s/n/z/m 停止。尚未判 minute 才從 run 後方往後掃：遇 s 判 minute；遇 y/d/h/n/z/m 停止。其餘字元跳過。兩個 scan 對原格式字串逐 char 判定，沒有解析 escape 或字面區塊，不能把註解的「相鄰 token」當成完整格式文法。

## escape 與 AM/PM

反斜線讓下一 char 按字面輸出；最後單獨反斜線被捨棄。未匹配 token 的其他 char 直接輸出。大寫後的整份原格式只要含 AM/PM，use12h 即 true，所有 h token 以 hour%12、0→12；此先行搜尋不考慮 escape。

迭代遇到未 escape 的 A/a 且接續五個 char 大寫為 AM/PM 時，hour<12 輸出 AM，否則 PM。渲染 marker 與先行決定 12 小時是兩段邏輯；不能推為 escape 後的 marker 一律不影響 h。

DateTimeToStr 固定傳 `yyyy/mm/dd hh:nn:ss`，minute 明用 nn；此固定格式不走 AM/PM、escape 或月份縮寫分支，只輸出到秒。DecodeTime 仍先四捨五入至毫秒並夾上限，字串不保留全部 double 精度；沒有宣稱任意 TDateTime round trip、BCB6 格式相同或實機精度已驗證。
