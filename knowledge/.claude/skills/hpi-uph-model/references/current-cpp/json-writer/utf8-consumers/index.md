# UTF-8直接consumer與同名validator

[JSON primitive](../primitives/index.md)；[current-cpp](../../index.md)。
來源 `b1bbe408a035595c81615cbc22543a326fba12b2`；10完整CPP／146原文行／9來源檔。
15原文頁另含5段context；相鄰原註解、日期與作者保留。

| 問題 | 入口 |
|---|---|
| ElaHub、WsDecoder與JsonWriter的同名函式如何分辨 | [validator身分](validators.md) |
| 固定950／CP_ACP、失敗fallback與內部NUL | [文字入口轉換](conversion.md) |
| WebSocket close／完整text／分片完成後的拒絕時點 | [WebSocket消費](websocket.md) |
| 版本、機型、平台與有限查證範圍 | [證據](evidence.md) |
| 固定pin、整檔blob／hash、原文分頁與去重 | [manifest](source-manifest.json) |
| 全V906兩symbol遮罩後詞法普查 | [census](symbol-census.json) |

此單元完成有限的直接consumer，不增加canonical主題，也不宣稱完整HTTP、WS或UPH caller已結案。
八個primitive原文沿用上層；!401既有交付與ST02-M單次通知已結案，不重計或重寄。

## 完整decoder接收子樹

[WsDecoder五完整函式](decoder/index.md)補Reset／Fail／Feed與TryOneFrame，分清buffer上限、分片與close責任；既有constructor／validator原文沿用，不重計。

## Encoder輸出子樹

[WebSocket encoder九完整函式](encoder/index.md)補長度／mask與文字、binary、control wrapper；close UTF8截斷與有效性驗證分清，原header／validator沿用不重計。

## 獨立opening-handshake子樹

[WsHandshake完整bytes層](handshake/index.md)保存parser／helpers／response與原header；socket owner另有解析路徑，不能混用驗證保證。

## Socket owner處理子樹

[WebBridgeServer六完整函式](network-owner/index.md)分清自有HTTP／Origin／ready交付與control處理；沿用既有handshake／encoder／decoder，實際flush／drop另追。
