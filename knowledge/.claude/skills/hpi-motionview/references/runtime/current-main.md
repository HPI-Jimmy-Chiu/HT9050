# 當前MotionView與保存架構的界線

20261006唯讀核對main `84233b648`。原稿包含多個時期，以下是目前檔案證據，未開頁或測實機。

## 正式資料

- 當前頁面位於`web/page/`，靜態layout／能力資料在`web/JSON/`；原稿的`D:\HT9045\page`與`.github/skills`記作歷史位置。
- `Main.MotionView.html`的Steven 20260918 A→B註記與`liveSettings`：teach／Gerneral／recipe快照換成`/api/system/teach`、`/api/system/gerneral`、`/api/recipe/<doc>`。`Production-update.json.state.motionView`沒有這條producer，不接舊快照頂替；語系字典仍可保留JSON。
- `Main.MotionView9050.html`由`HT9045Live.load()`與`liveSettings9050()`取即時資料，model讀不到就gate而不猜機型。頁末`ht9045_mv9050_axes.js`填`/api/struct/motor/runtime`軸表，註記「只填表、不擺圖」與整機motionView尚無producer。
- `web/JSON/Machine-profile.json`的HT9050與layout source仍標runtimeSupported:false；那是頁面能力／歷史HTML-only metadata，不能據此推定當前V906 source沒有Type_HT9050或整台軟體不支援9050。

## 生成與模式

Main9050頁末明標：Steven 0918 B路之後是手改，`build-main-motionview9050.py`重產會蓋掉B路與軸接線，重生前需先把兩者搬進腳本。原「不得手改Main、概念頁改完必跑build」僅作原時期記錄，不作今日無條件指令。
原StateRecord／SIM模板不等同目前正式LIVE，概念動畫也不能證明機台在籍、運動或安全互鎖生效。

## 查證來源

- [9045正式頁](../../../../../web/page/Main.MotionView.html)：liveSettings／A→B註記。
- [9050正式頁](../../../../../web/page/Main.MotionView9050.html)：HT9045Live、liveSettings9050與頁末軸接線註記。
- [能力集](../../../../../web/JSON/Machine-profile.json)／[layout](../../../../../web/JSON/MotionView9050-layout.json)。
- [原LIVE模板](../template/references/live-mode.md)／[原JSON runtime接線](../layout/references/json-runtime-wiring.md)。
