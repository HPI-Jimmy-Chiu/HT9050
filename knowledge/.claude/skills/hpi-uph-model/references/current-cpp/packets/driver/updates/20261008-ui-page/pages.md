# 程式頁面狀態到ui.pages

定位`W906_FormProgramShowHook`／`W906_UiPagesJsonHook`、wb_serve的安裝區段、`W906_PageProgramSet`／`PageProgramSet`／`PageTableFind`／`PageTableJson`／`W906_PageTableJson`、`RowState`與TSerialPoll表列。
完整函式／struct及明確標示的選定區段見 [manifest](source-manifest.json)；不是完整wb_serve或WebPageTable安全流程驗證。

1. csystem全域program/page hooks初值0。wb_serve選定安裝區段把它們指向`W906_PageProgramSet`與`W906_PageTableJson`。
2. inline開／關hook的零值條件、bridge成功transition與Close早退條件沿 [前段頁面通知](../20261008-online-pad/pages.md)。安裝語句存在不等於當次程序已走到那段。
3. table列為`TSerialPoll`→`testercomm`、opener=kPgBoth。`PageTableFind`拒空form，按strcmp找列；未知form使ProgramSet呼叫SayUnknown後返回，不更新列。
4. 已知列更新prog、by與since；kPgBoth再把want設1／2，並令wseq=++g_wseqNext。每次呼叫都增加wseq，不因相同open值省略。
5. `PageTableJson`輸出row id／op／state／on；kPgBoth另輸出want=open／close／空與wseq。body字串改變時才增加g_jsonSeq及重建整體JSON。
6. WebBridgeTags選定caller只在page JSON hook非0時stage `ui.pages`。`stageStr`在live=true時以c_str建立字串TagValue，否則null。

ProgramSet的prog／want／wseq是要求與本地狀態；PageTableJson的seq是body變動序號。兩者都不是每個HMI已完成open／close的ack。
RowStateStr／RowAnswer、window registry、PageTableTick停機與完整transport／安全caller尚未在本單元閉合。

回 [入口](index.md)；已查browser如何按wseq消費，見 [瀏覽器](browser.md)。
