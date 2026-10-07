# GPIB／RS232 的 payload 適配

pin `ede228b7c370a5f592c6e9b83e81aaa46331bd65`；[manifest](source-manifest.json)保存兩 source、`GpibEngine::OnHandlerMessage`及 `Rs232Engine::OnHandlerMessage`兩完整 body。上游是[EngineSideHandler](../lifecycle.md)。

- [資料複製、局部指標與返回](flow.md)。
- [GPIB UPH 群組與寫入](gpib/index.md)：一個 dispatcher 片段、一個完整 writer body。
- [版本與未查界線](limits.md)。

沒有讀完所有 dispatch、MV 格式或外部設備回覆；0 不作送達確認。
