# GPIB UPH 命令群組與本地 writer

pin `1091472b1ecfafcd9a6c141027fb46abc5e0ffb3`；[manifest](source-manifest.json)保存兩 source、`OnMyCopyMsg`中包含 `MSG_CMD_UPH`的一個完整分支片段，以及 `MyGPIBWrite`一個完整函式。dispatcher 片段不能算完整函式查證。

- [UPH 群組、重試與返回](write.md)。
- [版本、機型與未查界線](limits.md)。

接續[payload 適配](../flow.md)，不以寫入呼叫、log 或 bool true 推設備已收到。
