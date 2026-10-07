# UPH 全域狀態與暫停區段

本層是局部靜態查證；最初狀態證據的來源 pin 0c2eac30b4e56fec64b207f6adcc3ead69711903，六個 source blob、六個完整 body 的保存 hash、24 個定義／extern 宣告、22 個已讀區段見 [manifest](source-manifest.json)。保存 body hash 不代表完整 body 已讀，六個函式的完整語意都仍未核對。

| 問題 | Reference | 已查／仍待查 |
| --- | --- | --- |
| 全域型別、初值與 extern | [狀態定義](globals.md) | 六個 UPH 狀態×兩版×定義／宣告；所有生命週期與 ABI 待查 |
| MainProc 的暫停累加 | [累加區段](pause.md) | bCalculatePauseTime 區段；外層派工／暫停開始寫者未閉合 |
| OneCycle／CleanOut 清除 | [重置區段](reset.md) | 每版兩處＋八處；完整條件／callee／各機型作用仍待查 |
| Kernel暫停開始／刷新與排程 | [kernel子樹](kernel/index.md) | 兩DoSystemMessage body、四ShowRunLabel區段及兩SystemStart接合；完整caller／UI／實際tick待查 |
| 全樹後續線索 | [候選與界線](remaining.md) | 1120 cpp 的六詞原文盤點16候選，不是16個已核對writer |
| Loader取樣旗標／case | [Loader子樹](loader/index.md) | 四已讀區段／兩case邊界；全部Task／caller與取樣鏈待查 |

[Loader caller／HT9050分流](loader/dispatch/index.md) 另補三個上層所選case及9050補盤true的適用界線，仍非完整生命週期。

同一 [UPH 主題](../../../SKILL.md) 保留 HT9050／其他 Handler 機型與 Hot／Ambient／runtime 分流；本層不推定現場作用中配置。入口不增加另一份同題 Skill。
