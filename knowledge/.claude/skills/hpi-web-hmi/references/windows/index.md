# 視窗狀態與停止分工

- [完整頁面表原文](original-entry.md)：PageRowDef／opener、回答規則、START與全關行為、原裁決和owner。
- [WebSocket hub原文](../json/references/ws-link-hub.md)：每瀏覽器外框共用一條連線、id／ack／tag／權杖、jog放開與重送規則。
- [原路C開頁／關頁契約](../json/references/route-c-golden-bridge.md)、[exit／save](../pages/references/exit-save-unified.md)。
- [目前main](../runtime/index.md)：registry保守fShow、頁面表回答與實際screen在場分別查，不拿同一答案取代三者。

定位以WebPageTable.cpp的PageFormAnswer／PageScreenPresent／PageStartAllowed／PageTableTick、WebWindowRegistry.cpp的WebWindowRegistryQuery／WebWindowRegistryFShowConservative及csystem.cpp的W906_FormShowing。頁面／WINDOWS表與已被認領程式仍按既有owner處理，本批只改知識入口。

原90／69列與500ms是20261002文件基準；本次沒有重算全表或跑FShow_Audit，不宣稱數字仍成立。現在看kRows／kWebRowCount／opener與實際tagcaller。hub只放開所持jog；全關寬限與STOP在伺服器頁面表，這兩條分開。
