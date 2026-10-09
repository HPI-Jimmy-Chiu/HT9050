# JSON編碼與數值primitive

[current-cpp上層](../../index.md)；來源 `8edc9bdb86d2ece2a83244d9b71f9dfe5837ce86`。
8完整CPP／213原文行；另保存5段context，context不另計函式完成。

| 問題 | 入口 |
|---|---|
| UTF-8長度與錯誤span、逐byte替代、Windows CP950轉換順序 | [編碼](encoding.md) |
| 手工int64、double非有限值、15到17位迴圈與locale界線 | [數值](numbers.md) |
| header policy、Win32 gate、歷史測試、版本／機型與查證界線 | [證據](evidence.md) |
| 8完整函式＋5context、pin／blob／hash與reader分頁 | [manifest](source-manifest.json) |
| 原intake的詞法普查，未重新解出完整caller語意 | [census](symbol-census.json) |

只接續V906共用JSON層；既有登入／容器／quote／value的MR !401已由Jimmy合main，ST02-M回報單批通知已寄。
此入口從最新main獨立建立，!401舊文件沿用已合main內容，新批差異只有本單元，也未宣稱整個JSON議題或S8完成。

## 直接consumer與同名validator

[UTF8文字入口子樹](../utf8-consumers/index.md)補ElaHub、WsDecoder及六個ANSI wrapper的有限直接消費。
固定950／CP_ACP、NUL與失敗分支依函式正文區分；未作實機驗證。
