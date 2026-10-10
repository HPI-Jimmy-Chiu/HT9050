# query訂閱與host全域生命期

[上層](index.md)；[watch定位](raw/source-01-part-01.md#watch)完整body在第二頁；原IIFE／event callbacks／暴露debug methods在[evidence](evidence.md)全部保留。

pendingQuery是單一最新query；answered是qid鍵值表。watch要求HT9045Tags.onEvent，缺少則log err並return。
onEvent callback只收m.type==query，寫pendingQuery=m並delete answered[m.qid]；不主動開框，dialog-bridge／page的路由責任未在這段完整讀取。
若T.connect是function就connect().catch空函式，處理的是Promise拒絕；watch沒有總try/catch、沒有unsubscribe或自己避免重複訂閱的flag。
document.readyState==loading時註冊DOMContentLoaded呼watch，否則立即watch。原comment說connect冪等是來源聲明，此單元client本體尚未驗此語義。
HTDialogHost提供verifyAuth、submitResponse；pending返回目前物件引用，_inject(q)覆寫pendingQuery並在q truthy時清該answered entry。
這兩個inline debug方法、IIFE與callback credit0，全文context有保存；不把它們冒計13個選定named／object method函式。

單槽query、answered與authPrompt是此closure的browser狀態；不同視窗／重整／跨tab同步與C++retained query是不同生命期。
沒有從本輪靜態來源推持久化、總記憶體上限、跨視窗一致性或所有框都可關；部署／事件節奏／browser互動未執行。
原20260922-20261003說明、kiosk／fallback／modal-pending／golden裁決及ChangeLog路徑完整保留；現行C++例外與clear責任由[宿主正文](../host-modal/index.md)定位。
