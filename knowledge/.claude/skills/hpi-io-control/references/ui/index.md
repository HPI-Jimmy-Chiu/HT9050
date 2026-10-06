# IO畫面與退出reference樹

- [VCL IO畫面modal／Timer／FormClose案例](../io/original-entry.md)：錄影判讀、GUI是否活著與重複例外去重分開。
- [Alias Map工具原說明](../io/references/io-alias-map-doc.md)：原gen_io_alias_doc.py不搬、不改、不執行；五碼地址顯示不能取代目前1203路由。
- [Exit／Ctrl-C／X／登出／關機完整時間線](../io/references/exit-shutdown.md)。
- [目前IO按鈕caller](../runtime/current-main.md)。

exit原文件含先後互相推翻的歷史方案：以Q44最小版與後續A3W裁決／實作核對，不把舊「SIM接真卡仍加熱」例子或「全DO清零、等讀回」方案當現行已驗證行為。查FileRW/MainClose.cpp的Evaluate／Q44Issue／ShutdownSequence／SessionEndWindowExits；指令已送、讀回、確認馬達停止三層分開，未量測不假稱實機驗證。
