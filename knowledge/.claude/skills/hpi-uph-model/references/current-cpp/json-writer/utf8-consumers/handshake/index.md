# 獨立 HTTP／WebSocket opening-handshake bytes 層

[上層](../index.md)；[encoder](../encoder/index.md)；[decoder](../decoder/index.md)。
pin `7bf6ff28e23bbfeb6f94c1751d63ae62533313f4`；26完整CPP／419函式原文行、28原文頁含2 context。

| 問題 | 入口 |
|---|---|
| header cap、request line、obs-fold、partial out與consumed | [parser](parser.md) |
| ASCII token／OWS、URL／path、header／query讀法 | [helpers](helpers.md) |
| upgrade分類、key形狀與caller責任 | [upgrade](upgrade.md) |
| Accept key、101／400／405／426／431 | [responses](responses.md) |
| 來源保存、版本／機型與有限caller證據 | [證據](evidence.md) |
| 完整正文、header契約、去重與hash | [manifest](source-manifest.json) |
| 三檔26 spellings遮罩詞法查讀 | [census](symbol-census.json) |

這是獨立helper契約；WebBridgeServer的socket解析與upgrade路徑另有正文，不能移植其保證。
沒有增加canonical主題，也不宣稱UPH、HTTP、WS、browser或機台驗證已完整結案。
