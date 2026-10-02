# Q41 頁面補件的做法（St02-E 20260927）

> 原本由 Q41 helper 附在 St01 的 skill `D:\HT9045\.claude\skills\ht9045-html-json\references\route-c-golden-bridge.md` 檔尾（§9），
> 因為跟 St01 同一處的新增衝突，而且那是 St01 的檔，20260927 21:0x 改放在這裡；St01 要的話可以在它的檔裡加一行指到這份。

golden 表單的「畫面事件」（改一格就連動別格、按鈕填預設值、開別的視窗）C 路沒有即時入口（只有開頁／存檔；`form.event` 見 §3.0g），
Q41 的 Steven02 列（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md` §3）照下面的做法補在頁面上：

- **檔案**：一頁一支手寫補件 `D:\HT9045\web\page\ht9045_<頁>_q41.js`（Speed 是 `ht9045_speed_c.js`），檔名刻意不叫 `ht9045_wire_<slug>.js`
  （那是 gen_wire.py 的產生檔，重跑會蓋掉）。在頁面 HTML **最後一支接線檔那一行的行尾**加 `<!-- 說明 --><script src="…"></script>`，
  行數不變（例 `D:\HT9045\web\page\Setup.Speed.html:125`、`D:\HT9045\web\page\Config.Configuration.html:128`）。已經有手寫補件的頁
  （`ht9045_setup_sitemap.js`、`ht9045_cleaning_c.js`、`ht9045_barcode_c.js`、`ht9045_offset_wire.js`、`ht9045_hotplate_wire.js`、
  `Setup.Temp_Set.html` 的 inline script）就在認領的那一段裡同一行補（別人的檔行數不變）。
- **時機**：要等引擎套完值才做的（例 Configuration 的顯示段、TrayForm 的格子圖）包 `HT9045Recipe.editlistGet`，在回傳的 promise
  `then` 裡 `setTimeout(…, 0)`（引擎在同一個 then 裡同步套值）。只綁按鈕的（Ld_ULd、Contact、Speed）在 DOMContentLoaded 綁就好。
- **改值**：直接設 `value`／`checked`，再補發 `input`＋`change`（同引擎小鍵盤 onCommit），引擎存檔照 `GB_KIND` 收值。
- **可改判斷**：golden 設 `Enabled=true` 時，上層容器被引擎停用（`aria-disabled="true"`，權限）就不要打開（VCL 父層停用一樣按不到）；
  按鈕停用（`disabled`／`aria-disabled`）就不動作。伺服器存檔仍照規則丟不可改的值，頁面只負責畫面。
- **頁面自己補的元件不要用替身的 id**：引擎會把 `proxies` 裡同 id 的值套到元素上、存檔也會收（例 Cleaning 的 TUpDown 用
  `q41udDeviceCT`，不用 `udDeviceCT`）。
- **radio 一律用容器 id 找**（`#rgXxx input[type=radio]`、TRadioButton 用 label 的 id）；R76 之後引擎會替沒有 name 的 radio 補 name。
- **開別的視窗**（golden `fXxx->Show()`）：`window.parent.postMessage({open:'<background.html WINDOWS id>'}, '*')`。要對方開到某一頁／
  按某顆鈕時，先寫 localStorage 一個請求鍵，對方頁載入時讀、已開著時聽 `storage` 事件，處理完刪掉：
  `ht9045.testercomm.tab`（`D:\HT9045\web\page\testercomm.html`，TesterIF／Contact 的 TCP/IP 鈕）、
  `ht9045.offset.goto`（`D:\HT9045\web\page\ht9045_offset_wire.js` :357，BarCode 的 2DID Offset 鈕；等 editlist.get 資料到了才按）。
- **伺服器那一半**：事件會改別的元件的可見／可改、或改存檔值時（SetUp SU-1～6），在 `FileRW/<結構>.cpp` 的 `BeforeApply` 照 golden 重播
  （頁面值與伺服器不同、替身可改 → 套值＋跑產生器轉的 golden 處理器、列入 handled）；處理器要先加進 `tools/editlist/<結構>.py`
  的 METHODS 再用 `python tools/gen_editlist.py --only <結構>` 重產（先用原本的 .py 重產一次確認 0 行差異）。
- **伺服器事件（WS form.event）放在「伺服器有宣告」後面**（AI(W906-Q41-SPD／TS7) 20260928）：St01 已把 golden 處理器轉成
  form.event 的頁（Configuration、Speed、Temp_Set TS-7），頁面補件照 `D:\HT9045\web\page\ht9045_config_q41.js` 的樣子——包
  `HT9045Recipe.editlistGet` 記下回應；**伺服器有宣告才送，沒宣告一個都不送、照舊在頁面算**（舊伺服器行為不變）。
  宣告怎麼看：IniConfig 回 `eventTag`（`FileRW/IniConfig.cpp:458`，只有它有）；其他 C 路頁不帶 eventTag，看 `events` 有沒有**列齊**
  這一頁要送的控制項（`FileRW/_EditPage.cpp:79`；舊伺服器沒有這個鍵，或只列一部分，例 Temp_Set 舊版只有 TS-1 兩個）。
  tag 用 eventTag，沒有就用頁名（`"Setup.Speed"`，`FindPageForEvent` 收頁名）。同一組按鈕全送或全本地，不混用。
  每個事件帶 `state`（引擎存檔會送的值，**送出當下**才取，不是點的當下）；一次一個、等 ack，照 `ack.changed` 套值（套值補發的
  change 用 APPLYING 擋掉，不再送事件）；busy 等 450 ms 重送、not-operator 續權杖重送一次、錯誤含 "reload page" 就重新開頁。
  state **不帶有自己事件的控制項**（例 Temp_Set 的 rgIndexHeatMode：state 只設 ItemIndex、不重讀補償表，存檔的拒存判斷
  SaveFlow (1) 就擋不到）；面板多的頁（Temp_Set 1420 個面板欄）只帶跟伺服器不同的（WS 單則 64 KB），並把伺服器現在的值寫回
  `proxies`（＝`HT9045Page.golden().page`），inline 存檔包裝才比得對。滑桿這類沒有事件的，照舊在頁面鏡像，存檔時伺服器重播。
  例 `D:\HT9045\web\page\ht9045_speed_c.js`、`D:\HT9045\web\page\ht9045_temp_set_c.js`。
- golden 的怪處照翻並寫在檔頭（例 TrayForm 三組都拿 XCT1 判斷、HotPlate YCT1 只看 XCT1、SetUp 三顆勾選框只看 Arm1PickArm2Test）。
