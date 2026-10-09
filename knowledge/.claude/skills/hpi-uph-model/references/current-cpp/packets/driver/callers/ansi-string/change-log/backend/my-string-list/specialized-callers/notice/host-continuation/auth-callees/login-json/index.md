# 一般登入、BookLogin與JSON raw writer

[上層 auth callee](../index.md)；[先前 mode／StateJson](../state.md)。
來源 `367d9d85792fa756db6e898c950d6d79931cbf74`；4完整CPP／2來源，只有局部靜態查證。

| 問題 | 入口 |
|---|---|
| 權限清零、SPIL／KYEC、Setup ask與USER欄位 | [一般登入](operator.md) |
| BookCompare結果與Login／Logout旗標 | [BookLogin](book-login.md) |
| StateJson手工正文、RawValue與Key／comma狀態 | [Raw writer](raw-value.md) |
| 機型／客戶／版本／runtime與待續項 | [證據界線](evidence.md) |
| 四正文、hash、來源版本及舊保存範圍 | [保存 manifest](source-manifest.json) |
| 四符號詞法命中、definition／prototype／call分界 | [普查資料](symbol-census.json) |

未讀帳密、權杖、密碼本或環境值；未呼叫登入／INI回寫、JSON writer或機台程式。
完整browser consumer、其他writer成員與外部callee仍待追。

## 接續局部：writer容器與結果狀態

來源 `94ff7c1981d1ac98f01376a574098c39e1c25275`；Key、四容器開關、Ok／Clear與Str inline見[writer子樹](../../../../../../../../../../../../json-writer/containers/index.md)。
原RawValue／BeforeValue正文保持，新增段分開計數；完整caller與內容驗證界線在子頁。
