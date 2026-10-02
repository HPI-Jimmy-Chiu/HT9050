# 評估：設定頁的分頁標籤——「一共幾頁」表、開畫面時停在哪一頁、切頁時 BCB 自動跑的程式（R105、R106；連 Q46）

> 讀者：Steven。撰寫：ST01-E（Steven01 工程線）派的工程師，20260927 22:xx。**只讀研究，沒有改任何程式、沒有 build、沒有在移植樹跑轉檔工具。**
> 基準：`D:\HT9045` 分支 `v906/steven-cbridge-review6` 的 commit `c20bdec2`（那顆 commit 改的主要檔是 `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md`）。移植樹的檔一律用 `git -C D:\HT9045 show c20bdec2:<路徑>` 讀（今晚有人在改 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\IniConfig.py`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\ArmSpeed_File.py`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\Temperature.py`）。
> 行數量測：把 `c20bdec2` 的 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\` 與 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\` 複製到暫存區 `C:\Users\steven\AppData\Local\Temp\claude\d---github\99781e6b-2c4e-4591-81e8-fc879566566c\scratchpad\eval4\`，在複本上跑「原版」與「評估版」轉檔工具比對（第 4 節）。移植樹一個檔都沒動。
> 三棵樹的寫法：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\` 開頭＝**移植樹**（C++，UTF-8）；`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\` 開頭＝**golden V912**（BCB6 原始碼，Big5／cp950 編碼，VS Code 要「Reopen with Encoding」選 Big5）；`D:\HT9045\web\` 開頭＝**網頁**。行號是 20260927 當下的檔案。

**名詞（先白話，括號裡是程式裡的名字）**

| 白話 | 程式裡的名字 | 說明 |
|---|---|---|
| 一排分頁標籤 | TPageControl（每一頁是 TTabSheet） | 設定畫面上可以點來切換的一排頁籤。一個畫面可以有好幾排，也可以一排裡面再套一排。 |
| 第幾頁 | activePageIndex（網頁頁籤的 `data-t`） | 從 0 起算（第 0 頁＝畫面上最左邊那一頁），藏起來的頁也算一格。本文說「第 N 頁」一律是這個 0 起算的號碼。 |
| 「一共幾頁」表 | ELSetPageOrder | C++ 記住「這一排一共幾頁、依序叫什麼」，用來判斷網頁送來的「第幾頁」合不合理。 |
| 設計時停的頁 | .dfm 的 ActivePage | BCB 畫面設計檔（.dfm）裡存著設計者最後停在哪一頁；BCB 程式開機建畫面時套一次。常常不是第 0 頁。 |
| 切頁事件 | OnChange | **操作員點頁籤**時 BCB 自動跑的一段程式；程式自己切頁時不會跑。 |
| 網頁即時事件 | form.event（Q40＝A） | 網頁操作的當下請 C++ 跑一次 BCB 的事件程式，再把改了什麼回給網頁。C++ 已做，網頁送出點還沒有（Jimmy 的引擎）。 |
| C 路頁 | C 路（C 形狀） | 由 C++ 照 golden 的開畫面程式（FormShow）與存檔程式跑的設定頁；網頁只顯示與送出。 |
| 轉檔工具／產生檔 | `gen_editlist.py`／`*.gen.inc` | 把 golden 的 BCB 表單程式轉成 C++ 的工具 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\gen_editlist.py`；每個結構的設定在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\<結構>.py`；輸出是 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\<結構>.gen.inc`（不手改，改工具後重跑）。 |
| 替身 | 具名替身 EL&lt;T&gt; | C++ 端代替 BCB 畫面元件、只存值的物件（C++ 沒有畫面，元件在網頁）。 |

---

## 0. 一句話結論

- **R105（「一共幾頁」表）**：安全、操作員看不出差別、約半天、不用等 Jimmy。轉檔工具替 64 排分頁各產生一張完整頁序表，28 個產生檔只**加**行（共 +184 行、0 行刪），消掉 10 排分頁「後面幾頁被當成不合理、記一筆假待辦」的問題（有網頁的 4 排：Handler System「Heater」、Temp_Set「Multi Sensor」、Tester IF「SLT Setting」、Ground Man「Log」）。**建議現在做。**
- **R106（開畫面時停在哪一頁照 BCB）**：C++ 端可以先做，**做完畫面不會變**（網頁引擎還不照 C++ 切頁）。操作員真正看到差別要等 Jimmy 改網頁引擎。逐頁比對有分頁的 17 個 C 路網頁、50 排分頁：**18 排一定跟 BCB 不同**（11 排在 Configuration，幾乎都是「設計時停的頁」），**5 排看機種或模式**，27 排相同。要先講好三件事：①存檔後網頁會重讀，重讀時要不要跳頁（題 3）；②存檔時把目前頁送給 C++ 之後，Tray Assignment 的「FT 存檔時 RT 跟著一樣」會開始生效（題 4，是行為改變）；③Temp_Set 的溫控設定分頁必須連 BCB 的「建立畫面」程式一起翻，否則會變成開在錯的頁。
- **切頁事件（OnChange）**：有網頁的設定頁上還有 6 支沒翻（Configuration 外層＋內層、Temp_Set、Handler System、BarCode、Tray Form）。建議只翻 Temp_Set 那一支；Configuration 兩支跟著 Q46 的選擇走；Handler System（網頁碰不到那一頁）、BarCode（純畫面）、Tray Form（複製功能網頁已做）三支不翻、寫明理由（題 5）。

---

## 1. 背景（白話）

### 1.1 BCB 版怎麼決定「打開設定畫面時停在哪一頁」

1. **程式開機**建所有設定畫面（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\HT9045.cpp` 第 167-246 行的 CreateForm），每一排分頁先停在**設計時停的頁**。設計者存檔時停在哪就是哪，例：Configuration 的 A～M 大分頁停在「N [ Network ]」（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 457 行）。
2. 有的畫面在**建立時**再切一次（例：Temp_Set 的溫控設定那一排切回第 0 頁，golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp` 第 428-431 行 FormCreate）。
3. **每次打開畫面**（開畫面程式 FormShow），有的明寫「切到某頁」，例：Yield Monitoring 依 Run Mode 切到 Normal 或 Re-Test 頁（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uYieldMonitoring.cpp` 第 2646-2674 行）；沒寫的，**停在操作員上次離開的那一頁**（畫面物件開機建一次、一直留著）。
4. 程式把某一頁**藏起來**時，如果藏的正好是目前這一頁，BCB 會自動跳到同一位置旁邊看得見的那一頁（BCB 元件庫的規則；移植樹 Start Condition 已照這條做，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\StartCondition.py` 第 26 行說明、第 297-305 行做法）。
5. **切頁事件只在操作員點頁籤時跑**；程式自己切頁不跑（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\StartCondition.cpp` 第 262-263 行的註解是同一個認定）。

### 1.2 網頁版現在怎麼做（現況與 commit）

**C++（伺服器）**
- commit `c9d3c932`（主要改 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditList.cpp`）起，每排分頁的替身在開頁資料裡帶「目前第幾頁」，也收網頁在存檔（editlist.save）與即時事件裡送回的值；不合理（不是整數、超出頁數、這一排被停用或藏起來）就只丟那一筆、記待辦（R102～R104）。規則在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditList.h` 第 131-133 行、第 159-163 行、第 178-189 行；判斷在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditList.cpp` 第 312-349 行（PageCountOf、PageIndexRefused）；存檔時先套分頁在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp` 第 118-150 行。自動測試 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_editlist_pageindex.cpp`（ctest 名 EditList_PageIndex）。
- **「一共幾頁」怎麼算**：登記了「一共幾頁」表就照表；沒登記（**目前全部都沒登記**，呼叫端 0 個）就數「父子表裡掛在這一排底下的頁」——但轉檔工具只把「程式有用到的元件＋它們的上層」放進父子表（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\gen_editlist.py` 第 410-416 行），沒用到的頁不在表裡，所以只是**下限**，下限以上的頁被當成不合理。
- **開畫面時停的頁**：轉檔工具不讀設計時停的頁，替身一律從第 0 頁開始（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\vclcompat\Controls.h` 第 497-501 行預設 0）；golden「切到某頁」的寫法 `->ActivePage=某頁` 被當成純畫面、換成空敘述（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\gen_editlist.py` 第 303-311 行）——目前正在執行的程式裡有 20 處被這樣丟掉。另一種寫法 `->ActivePageIndex=數字`（直接給號碼）本來就照跑。
- **已經自己處理分頁的頁**（這次不用動）：Tray Assignment（開機設第 1 頁 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TrayForm.cpp` 第 454-456 行；存檔與事件改讀函式 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\TrayForm.py` 第 172-185 行）、Start Condition（自己的頁指標＋網頁補件：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\StartCondition.cpp` 第 150-224 行、第 266-291 行，`D:\HT9045\web\page\ht9045_startcondition_c.js`）、Cleaning（`D:\HT9045\web\page\ht9045_cleaning_c.js` 第 77-85 行、第 220-221 行照 C++ 切頁）、Tester IF 介面那一排（`D:\HT9045\web\page\ht9045_testerif_c_wire.js` 第 138-142 行）、Bin Select（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\BinSelect.py` 第 145-179 行已手寫成號碼）、OffSet（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\Offset_File.py` 第 157 行、第 223 行）。

**網頁（Jimmy 的引擎 `D:\HT9045\web\page\ht9045_wire_engine.js`）**
- C 路頁載入時套值（`D:\HT9045\web\page\ht9045_wire_engine.js` 第 1131-1187 行 gbApply、第 1188-1223 行 gbLoad）**不看**「第幾頁」，存檔（同一個檔第 1249 行起 gbSave）也**不送**。頁面檔一律把第 0 頁標成目前頁（Configuration 外層例外，標第 2 頁 Config）。只有第二型頁（HotPlate）的 formOverlay 會照伺服器點頁籤（`D:\HT9045\web\page\ht9045_wire_engine.js` 第 1591-1594 行）——這段可以直接拿來用。
- 頁籤的點擊是每一頁 HTML 裡的小程式（例 `D:\HT9045\web\page\Config.Configuration.html` 第 62-80 行）：點第 N 個頁籤就顯示第 N 塊內容。
- 網頁在**開站**時就把設定頁在背景載好（`D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\window-open-edge.md` 第 1.2 節）⇒ 「開頁時的頁」其實是開站那一刻算的；那份評估的選項 D（打開視窗才載入）做了之後才會對準操作員按開窗的那一刻。

### 1.3 操作員會遇到的情況（由程式推得；網頁引擎接上之後才會出現）

- **假待辦（R105）**：在 Handler System 切到最後一頁「Heater」改完存檔 → 存檔結果多一行「第 10 頁超出 0～9，伺服器保留原頁」，而且 C++ 記成還停在原來那頁。
- **開頁停在第 0 頁、BCB 不是（R106）**：Run Mode 是 Retest 時打開 Yield Monitoring，BCB 停在「Re-Test」頁，網頁停在「Normal」；Configuration 開機後第一次打開，BCB 停在「N [ Network ]」，網頁停在「A [ Function ]」。
- **存檔結果跟 BCB 不同（R106 延伸）**：Tray Assignment 開了「FT 存檔時 RT 跟著一樣」（Configuration 的 bFTBin2RTBin），操作員在 Normal Test 頁存檔，BCB 會把 FT 的 bin 放置複製到 RT（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp` 第 1353-1372 行）；網頁不送目前頁，C++ 以為停在 Retest 頁，永遠不複製。
- **切頁事件沒跑（OnChange）**：Temp_Set 切到 ATC 補償頁，BCB 把「基準溫度」那一組格子藏起來、改不到；網頁照樣顯示、照樣可改。

---

## 2. 逐頁表

欄位說明：「頁數與順序」＝第幾頁：操作員看到的頁名（程式名）；「設計時停的頁」＝golden .dfm 的 ActivePage（括號是第幾頁）；「golden 讀／設分頁的行」＝golden 程式裡看或改這一排目前頁的地方，標「未翻」的是移植樹沒轉的方法、「丟掉」的是目前被當純畫面的；「現況」＝網頁今天停的頁 vs BCB 第一次打開停的頁；「一共幾頁」＝C++ 父子表目前數得到幾頁／實際幾頁。

### 2.1 有網頁的 C 路頁（17 頁、50 排）

**Configuration（`D:\HT9045\web\page\Config.Configuration.html`，結構 IniConfig，golden 表單 TfConfiguration）**

| 分頁標籤排 | 頁數與順序 | 設計時停的頁 | golden 讀／設分頁的行 | 切頁事件 | 現況（網頁／BCB） | 一共幾頁 |
|---|---|---|---|---|---|---|
| 外層 PageControl1 | 5 頁：0 Soft、1 Comm（tsTempComm）、2 Config、3 Tray、4 Hot Plate | Config（2），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 27 行 | 開頁 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp` 第 4636-4646 行：ASE 高雄若在 Config 頁就改回 Soft（照跑），其他客戶切到 Config（丟掉）；`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp` 第 5609、5663 行 UpdateUT150Comm 讀（未翻）；`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp` 第 6101-6114 行 PageControl1Change（未翻） | PageControl1Change（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 43 行）：權限檔鎖頁＋ASE 高雄密碼＝Q46／Q45 | 網頁 Config（頁面檔寫死第 2 頁）；BCB Config；**ASE 高雄 BCB 是 Soft**（看機種） | 5／5 |
| 內層 pcConfig | 14 頁：0 A、1 B、2 C、3 D、4 E、5 F、6 G、7 I、8 L、9 O、10 N、11 P、12 M、13 Search Function | N（10），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 457 行 | 開頁 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp` 第 4619-4623 行逐頁打開（已等價改寫）；`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp` 第 6112 行、第 6120-6121 行（切頁事件內，未翻） | pcConfigChange（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 470 行）＝Q46 | 網頁 A；BCB 第一次 N → **不同** | 14／14 |
| pcA00（A 群組） | 6 頁：A01-A10、A11-A20、A21-A30、A31-A40、A51-A70、A71-A80 | A71-A80（5），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 494 行 | 無 | 無 | 網頁 A01-A10；BCB A71-A80 → **不同** | 6／6 |
| pcB00 | 2 頁：B01-B10、B11-B15 | 0，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 2247 行 | 無 | 無 | 相同 | 2／2 |
| pgC00 | 3 頁：C01-C10、C11-C20、C21 | C11-C20（1），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 2622 行 | 無 | 無 | **不同** | 3／3 |
| PageControl2（C21 裡） | 3 頁：Small、Mid、Big | 0，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 3577 行 | 無 | 無 | 相同 | 3／3 |
| pcD00 | 9 頁：Preasure、High Cal、Contact、Mode、D40、D50、D60、D70、D80 | D80（8），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 3916 行 | 無 | 無 | **不同** | 9／9 |
| pcE00 | 4 頁：X/Y Scale、Other、E50、E70 | E70（3），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 5832 行 | 無 | 無 | **不同** | 4／4 |
| pgcScale | 3 頁：E30 In Arm、E31 Out Arm、E32 Shuttle | E32（2），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 5857 行 | 開頁切 E30（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp` 第 4820 行，丟掉；結果碰巧是 0） | 無 | 相同 | 3／3 |
| pgcE31 | 3 頁：E31、E31 Hot、E31 Cold | 0，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 6394 行 | 開頁切 E31（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp` 第 4821 行，丟掉；結果 0） | 無 | 相同 | 3／3 |
| pcF00 | 4 頁：F01-F10、F11-F20、F21-F30、F31-F40 | F31-F40（3），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 9766 行 | 無 | 無 | **不同** | 4／4 |
| pgI00 | 5 頁：I01-I22、I20-I30、I31-I40、I41-I50、I51-I60 | 0，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 10901 行 | 無 | 無 | 相同 | 5／5 |
| pcL00 | 6 頁：L01-L10、L11-L20、L21-L30、L31-L32、L33-L35、L36-L42 | L36-L42（5），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 11997 行 | 無 | 無 | **不同** | 6／6 |
| pcO00 | 3 頁：O01-O10、O11-O20、O21-O30 | O11-O20（1），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 13532 行 | 無 | 無 | **不同** | 3／3 |
| pcN00 | 29 頁：N05、N06 … N35、N40 | N05（0），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 14447 行 | 無；但 N05 頁多數機台被藏（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp` 第 5023 行），依 1.1 第 4 條 BCB 會跳到下一個看得見的頁 | 無 | 名目相同；**N05 被藏的機台 BCB 停在下一頁**（第 5 節風險 7） | 29／29 |
| pgcN10_1 | 2 頁：N1-N10、N11-N20 | 0，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 15376 行 | 無 | 無 | 相同 | 2／2 |
| pgcN14_1 | 6 頁：N01-N06、N07-N13、N14-N19、IPSC、N20、N23 | N23（5），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 15793 行 | 無 | 無 | **不同** | 6／6 |
| pgcN23 | 4 頁：N23、N23-1、N23-2、N23-3 | 0，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 17934 行 | 無 | 無 | 相同 | 4／4 |
| pcN25 | 2 頁：N25-1～3、N25-4 | N25-4（1），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 18283 行 | 無 | 無 | **不同** | 2／2 |
| pcP00 | 5 頁：P01-P10、P11-P20、P21-P30、P31-P50、P51-P65 | P21-P30（2），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 20041 行 | 無 | 無 | **不同** | 5／5 |

**其他 16 頁**

| 結構（網頁） | 分頁標籤排 | 頁數與順序 | 設計時停的頁 | golden 讀／設分頁的行 | 切頁事件 | 現況（網頁／BCB） | 一共幾頁 |
|---|---|---|---|---|---|---|---|
| ArmSpeed_File（`D:\HT9045\web\page\Setup.Speed.html`） | PageControl1 | 8 頁：0 Speed & Acc/Dec、1 Input Arm、2 Shuttle、3 Index Arm、4 Output Arm、5 Tray Arm、6 Tray Loader、7 Magazine | Input Arm（1），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSpeed.dfm` 第 24 行 | 開頁切第 0 頁（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSpeed.cpp` 第 49 行，丟掉；結果碰巧 0） | 無 | 相同 | 8／8 |
| GroundMan（`D:\HT9045\web\page\Status.GroundMan.html`） | PageControl1 | 2 頁：0 Main、1 Log | 0 | 開頁切 0（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\GroundMan\GroundMan.cpp` 第 224 行，照跑） | 無 | 相同 | **1／2**（Log 頁被當不合理） |
| HSys（`D:\HT9045\web\page\HW.HandlerSys.html`） | pcSetting | 11 頁：0 Handler、1 Loader/Unloader、2 In & Out Arm、3 Index Items、4 Shuttle、5 Other Items、6 Com Port、7 Customer Code、8 ION Fan、9 Search Function、10 Heater | Index Items（3），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\HandlerSys.dfm` 第 46 行 | 開頁切 0（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\HandlerSys.cpp` 第 154 行，照跑）、第 155 行藏 Customer Code；`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\HandlerSys.cpp` 第 1204-1212 行右鍵 Exit 顯示並切到 Customer Code（未翻）；`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\HandlerSys.cpp` 第 1288 行（切頁事件內，未翻） | pcSettingChange（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\HandlerSys.dfm` 第 52 行） | 相同 | **10／11**（缺 Search Function ⇒ 第 10 頁 Heater 被當不合理） |
| Ld_UldDelayTime（`D:\HT9045\web\page\Setup.Ld_ULd.html`） | PageControl1 | 2 頁：Load / Unload、Knocker | 0 | 無 | 無 | 相同 | 2／2 |
| Offset_File（`D:\HT9045\web\page\Setup.OffSet.html`） | PageControl1 | 7 頁：0 In Out Arm Offset、1 Index and Tray Arm、2 Arm Offset List、3 Setup Teaching、4 Hot Temp Shift、5 In/Out Scale、6 Z cal | 0 | 建構子先 1 後 0（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cOffSet.cpp` 第 189-196 行，號碼照跑、頁名丟掉；第 192-193 行藏第 1、2 頁）；存檔讀（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cOffSet.cpp` 第 2812 行，已改讀手寫值）；按鈕切頁（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cOffSet.cpp` 第 3370、3376、3383、3389、3552 行，網頁按鈕自己切）；`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cOffSet.cpp` 第 2700 行讀（未翻） | 無 | 相同 | 7／7 |
| StartCondition（`D:\HT9045\web\page\Data.StartCondition.html`） | 外層 PageControl1 | 3 頁：0 Start Mode、1 Life Time、2 Function | Life Time（1），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cStartCondition.dfm` 第 42 行 | 無（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cStartCondition.cpp` 第 542-571 行只管 Function 頁顯示） | 無 | 網頁 Start Mode；BCB Life Time → **不同** | 3／3 |
| 同上 | pgLifeTime | 9 頁：HeadCondition1～3、Socket ID、Vibrator、Smart Diagnostic、Socket Count、In/out arm picker、Cylinder | Socket Count（6），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cStartCondition.dfm` 第 682 行 | 開頁依目前頁切（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cStartCondition.cpp` 第 391-455 行）、清除鈕看目前頁（第 778-792 行）、存檔看目前頁（第 915-991 行）——全部已由手寫頁指標處理 | pgLifeTimeChange（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cStartCondition.dfm` 第 686 行；網頁補件已照做） | 相同（已處理） | 9／9 |
| Temperature（`D:\HT9045\web\page\Setup.Temp_Set.html`） | pgcTempSetting | 5 頁：Hot、Ambient、Temperature Control、Others、Tri-Temp | Others（3），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.dfm` 第 259 行 | 建立畫面時切 0（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp` 第 428-431 行 FormCreate，**未翻**） | 無 | 相同（碰巧：C++ 預設 0） | 5／5 |
| 同上 | pgcOtherFunc | 5 頁：Initial Temp Offset、Enhanced Offset、L/B Control、Sigma For Temp、Others | Others（4），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.dfm` 第 1630 行 | 無（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp` 第 594-609、945 行只管顯示） | 無 | **不同** | 5／5 |
| 同上 | pgcTempOffset | 11 頁：0 Normal、1 Arm 1、2 Arm 2、3 Heat Gun、4 ATC、5 ATC7.0、6 ATC_PID、7 DUT Heat、8（未使用）、9 ATC FFC、10 Multi Sensor | ATC（4），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.dfm` 第 4461 行 | 開頁切 0（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp` 第 950 行，照跑）；`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp` 第 5226-5259 行（切頁事件內，未翻） | pgcTempOffsetChange（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.dfm` 第 4472 行） | 相同 | **10／11**（缺 Normal ⇒ 第 10 頁 Multi Sensor 被當不合理） |
| 同上 | PageControl1（加熱模式裡） | 2 頁：Standard、Tri Temp | 0 | 無 | 無 | 相同 | 2／2 |
| TestIF_File_BarCode（`D:\HT9045\web\page\Setup.BarCode.html`） | pgc2DID | 6 頁：2DID Setting、Floating Detection、CCD Setting、Cognex Setting、2D Mapping、XML Result | 0 | 開頁切 0（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\BarCode\BarCode.cpp` 第 351 行）、沒裝條碼但有 Shuttle 浮料檢查切 1（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\BarCode\BarCode.cpp` 第 491-499 行）——都照跑；`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\BarCode\BarCode.cpp` 第 645 行沒裝條碼時藏 2DID 頁 | pgc2DIDChange（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\BarCode\BarCode.dfm` 第 54 行） | 網頁 0；**浮料檢查機台 BCB 是 1**（看機種；C++ 已經是 1，網頁沒照） | 6／6 |
| 同上 | pc2DID | 5 頁：Result、Log Data、Setting、Multi 2DID、OCR | Setting（2），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\BarCode\BarCode.dfm` 第 62 行 | 無 | pgc2DIDChange（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\BarCode\BarCode.dfm` 第 73 行） | **不同** | 5／5 |
| 同上 | pgcFunction | 2 頁：Function I、Funtion II | 0 | 開頁切 0（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\BarCode\BarCode.cpp` 第 345 行） | 無 | 相同 | 2／2 |
| 同上 | pcShtFloat | 2 頁：Result、Setting | Setting（1），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\BarCode\BarCode.dfm` 第 2402 行 | 無 | 無 | **不同** | 2／2 |
| 同上 | pgcCCDSetting | 3 頁：4CCD、8CCD、Unloader Clip | Unloader Clip（2），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\BarCode\BarCode.dfm` 第 2615 行 | 下 8CCD 切 1、否則 0（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\BarCode\BarCode.cpp` 第 514-517 行，照跑）；從 Unloader 讀夾子碼時切 Unloader Clip（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\BarCode\BarCode.cpp` 第 521-524 行，丟掉） | 無 | 網頁 0；**看機種**：8CCD 機台 1、讀夾子碼機台 2 | 3／3 |
| 同上 | pgc2DMap | 7 頁：Auto 1~3、Auto 4~6、Fix 1~3、Fix4~6、Bin Box、Magazine1~8、Magazine9~14 | 0 | 開頁切 0（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\BarCode\BarCode.cpp` 第 343-344 行） | 無 | 相同 | 7／7 |
| TestIF_File_Cleaning（`D:\HT9045\web\page\Setup.Cleaning.html`） | pcCleanYield | 5 頁：Offset、Yield、Failure、Smart、Time | 0 | 無 | 無 | 相同（網頁補件照 C++） | 5／5 |
| 同上 | pgCleanType | 2 頁：Kit、Tray | 0 | 依 Kit／Tray 類型切（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\AutoClean\uCleaning.cpp` 第 779-786、2252-2259、2385 行，照跑）；`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\AutoClean\uCleaning.cpp` 第 2145 行 XCT1Change 讀（照跑） | 無 | 相同（網頁補件照 C++） | 2／2 |
| TestIF_File_QAMode（`D:\HT9045\web\page\Setup.QAMode.html`） | pgcQAMode | 2 頁：QA Mode、QA Sampling | 0 | 無 | 無 | 相同 | 2／2 |
| TestIF_File_SetUp（`D:\HT9045\web\page\Setup.SetUp.html`） | pgcASE | 2 頁：Check Torque、Tray Map | 0 | 無 | 無 | 相同 | 2／2 |
| TestIF_File_TesterIF（`D:\HT9045\web\page\Setup.TesterIF.html`，St02 的結構） | PageControl2（介面） | 4 頁：DIO、GPIB、RS232、TCP/IP | GPIB（1），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTesterIF.dfm` 第 63 行 | 只顯示選到的介面頁並切過去（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTesterIF.cpp` 第 1192-1199 行；第 1198 行頁名丟掉、第 1199 行號碼照跑） | 無 | 相同（網頁補件切到唯一看得見的頁） | 4／4 |
| 同上 | pgcRS232 | 2 頁：Setting、SLT Setting | 0 | 無 | 無 | 相同 | **1／2**（SLT Setting 被當不合理） |
| 同上 | PageControl1（時間設定） | 5 頁：Time1、Time2、PurgeAir、RT、EQC | 0 | 開頁切 0（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTesterIF.cpp` 第 115、195 行，照跑）；SPIL：RT 模式切 RT、QC 站切 EQC、否則 Time1（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTesterIF.cpp` 第 263-274 行，丟掉） | 無 | 網頁 0；**只有 SPIL 不同**（看客戶） | 5／5 |
| TestIF_File_VacuumUnit（`D:\HT9045\web\page\HW.VacuumUnit.html`） | pgcVacuumUnit | 4 頁：Manual、InArm、Index、OutArm | 0 | 無 | 無 | 相同 | 4／4 |
| TestIF_File_YieldMonitoring（`D:\HT9045\web\page\Setup.YieldMonitoring.html`） | pgcMode | 7 頁：0 Normal、1 Re-Test、2 AutoRetest、3 AutoRetest（KYEC）、4 Alarm 4 / 5、5 Yield、6 Auto Site Off | Yield（5），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uYieldMonitoring.dfm` 第 78 行 | 依 Run Mode 切（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uYieldMonitoring.cpp` 第 2646-2674 行，全部丟掉）：一般／續跑／初始／Auto Site Map／QA→Normal；ART 系列：KYEC→第 3 頁、ASE 高雄→第 2 頁、其他→Normal；其餘（Retest 系列）→Re-Test | 無 | 網頁 0；**看模式**：Retest 系列 BCB 是 Re-Test | 7／7 |
| 同上 | pgcBySiteByBinPercentCompare_FT | 4 頁：Bin Alarm1～4 | Bin Alarm3（2），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uYieldMonitoring.dfm` 第 96 行 | 無 | 無 | **不同** | 4／4 |
| 同上 | pgcBySiteByBinPercentCompare_RT | 4 頁：Bin Alarm1～4 | Bin Alarm3（2），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uYieldMonitoring.dfm` 第 2475 行 | 無 | 無 | **不同** | 4／4 |
| TrayForm（`D:\HT9045\web\page\Setup.TrayAssignment.html`） | pgRunMode | 4 頁：0 Normal Test（群組）、1 Retest（群組）、2 Normal Test（圖）、3 Re-Test（圖） | Retest 群組（1），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.dfm` 第 253 行 | 讀：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp` 第 1033-1034 行（ShowCompnet）、第 1235-1236 行、第 1277-1278 行、第 1355-1356 行（存檔 FT→RT 複製）、第 1687 行——全部已改讀手寫函式；伺服器開機已設 1 | 無 | 網頁 0；BCB 與 C++ 都是 1 → **不同（網頁跟 C++ 也不一致）** | 4／4 |
| UserDefForm_File（`D:\HT9045\web\page\Setup.TrayForm.html`） | PageControl1 | 6 頁：Type1、Type2、Type3、Bin Box、Color Sensor、Boat Carrier | 0 | 開頁切 Type1（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayForm.cpp` 第 149 行，丟掉；結果 0）；`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayForm.cpp` 第 465 行 UpDateType、第 703 行 spbCopyClick 讀（都未翻，複製功能網頁自己做） | PageControl1Change（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayForm.dfm` 第 241 行） | 相同 | 6／6 |

合計 50 排：**18 排一定不同**（Configuration 11、Start Condition 1、Temp_Set 1、BarCode 2、Yield Monitoring 2、Tray Assignment 1）、**5 排看機種或模式**（Configuration 外層、BarCode pgc2DID、BarCode pgcCCDSetting、Tester IF 時間設定、Yield Monitoring pgcMode）、27 排相同。

另外兩個有網頁、但這一排不在頁面上：Bin Select 的 PageControl1（7 頁，`D:\HT9045\web\page\Setup.BinSel.html` 自己的版面，golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cBinSel.cpp` 第 2161-2227 行依測試模式切頁、第 2386-2432 行存檔看頁——已手寫成號碼並改讀移植樹物件）；AOAOffset 的 pgMotionView（主畫面的 11 頁 Motion View，`D:\HT9045\web\page\Main.AOAInfo.html` 沒有這一排）。Contact、CounterSel、DIO、ShuttleMove 四頁沒有分頁。

### 2.2 沒有網頁的 C 路結構（16 排；R105 也會補表，但今天沒有人送「第幾頁」）

| 結構 | 分頁標籤排 | 頁數 | 設計時停的頁 | golden 讀／設分頁的行 | 切頁事件 | 一共幾頁 |
|---|---|---|---|---|---|---|
| ACTForm | pcACT | 2（Temperature、Setup） | 0 | 開頁切 0（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\AutoTemperature.cpp` 第 527 行，丟掉） | 無 | 2／2 |
| AOAOffset | pgMotionView | 11（Motor View … AOA Info） | Motion View（1），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.dfm` 第 11932 行 | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp` 第 8720-8721、9834、26854-26869、29566 行（主畫面，都未翻） | 無 | **1／11**（只登記第 10 頁 AOA Info ⇒ 只收第 0 頁，真正用到的第 10 頁反而被擋） |
| AOISetup | PageControl1 | 8 | TopBottom Mode（6），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\fAOI.dfm` 第 246 行 | 開頁依機種切 2／4／0 並藏頁（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\fAOI.cpp` 第 3779-3821 行，FormShow 未翻）；第 4790-4792 行（未翻） | 無 | 7／8 |
| AOISetup | PageControl4 | 2 | 0 | 無 | 無 | 1／2 |
| AOISetup | pcLog、PageControl2、PageControl3 | 2、2、1 | Hex（1）／0／0 | 無 | 無 | 不是替身（C++ 沒有這三排） |
| AutoCalSuckZ | PageControl1 | 6 | Calibrate Suck Z Height（5），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\ProductionInfo\ProductionInfo.dfm` 第 24 行 | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\ProductionInfo\ProductionInfo.cpp` 第 183、188、1889、3072-3078、5831-5840、6147-6179 行（都未翻） | PageControl1Change（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\ProductionInfo\ProductionInfo.dfm` 第 36 行） | **1／6**（只登記第 5 頁 ⇒ 只收第 0 頁，真正有資料的第 5 頁被擋） |
| AutoCalSuckZ | PageControl2、pcInOutArmSuckZ | 5、2 | 4／0 | 無 | 無 | 不是替身／2／2 |
| ContactForce | PageControl1 | 8 | Die Force Dynamic Kit（5），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\ContactForce.dfm` 第 24 行 | 無（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\ContactForce.cpp` 第 693-713 行只管顯示） | 無 | 8／8 |
| IniConfig_OCR | PageControl1 | 3 | 2 | 開頁切 2（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\OCR.cpp` 第 215 行，照跑） | 無 | 3／3 |
| Monitor | MVPageControl | 3（Main、Setup、Specific） | 0 | 建構子切 0（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Monitor\MonitorInterface.cpp` 第 29 行，未翻） | 無 | 2／3 |
| Rotate | pgcRotate | 6 | M8 In Rotate（2），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\RotateKit\fRotate.dfm` 第 351 行 | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\RotateKit\fRotate.cpp` 第 858-931 行讀（未翻；其中 TabIndex＝「看得見的頁」的位置，不是頁號） | 無 | 4／6 |
| TestIF_File_AutoAlignment | PageControl19、PageControl18 | 2、2 | 1／0 | 無 | 無 | 2／2、2／2 |
| TestIF_File_FixAICCD | PageControl1 | 2 | 0 | 無 | 無 | 2／2 |

34 個結構全部的分頁共 68 排（AOAOffset 只算它用到的那一排；golden 主畫面 .dfm 另有 4 排不屬於這個結構）；其中 64 排在 C++ 有替身。**Golden .dfm 裡沒有任何一頁設計成「藏起來」**（34 個 .dfm 找 `TabVisible = False` 共 0 筆）——藏頁全部是程式在跑的時候做的。

### 2.3 切頁事件總表（8 支）

> ⛔ 20260929 更新（B10b，commit `f14484e1`）：**已翻**＝Temp_Set `pgcTempOffsetChange`、Configuration `PageControl1Change`、`pcConfigChange`（頁面在使用者點分頁時送 form.event＋activePageIndex；程式設 ActivePage 不送，同 VCL 不觸發 OnChange）。**還沒翻**＝HSys `pcSettingChange`、BarCode `pgc2DIDChange`、UserDefForm `PageControl1Change`（照下表的建議排後面）。

| 結構 | 事件（golden） | 做什麼（白話） | 移植樹現況 | 建議 |
|---|---|---|---|---|
| IniConfig | PageControl1Change，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp` 第 6101-6114 行 | ASE 高雄切到 Config 頁要輸入密碼（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp` 第 6887-6904 行 bPassWord，會跳 BCB 密碼視窗），錯了切回 Soft；依權限檔 `D:\HT9045\config\Security_new.def` 把整頁鎖住 | 未翻；Configuration 頁收不到網頁即時事件（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp` 沒有登記成 C 路頁） | 跟 Q46（題 5） |
| IniConfig | pcConfigChange，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp` 第 6116-6123 行 | 登入等級 ≥ 權限表 42 號時，依權限檔把 A～M 某頁整頁鎖住 | 同上 | 跟 Q46 |
| Temperature | pgcTempOffsetChange，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp` 第 5224-5262 行 | 在 ATC 頁藏起「基準溫度」那一組（7 個輸入格＋標籤＋清除鈕）；Arm 1／Arm 2 頁才顯示排序鈕；TSMC 台南另切兩塊面板 | 未翻 | **翻** |
| HSys | pcSettingChange，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\HandlerSys.cpp` 第 1286-1289 行 | 只在 Customer Code 頁顯示「搜尋客戶代碼」框 | 未翻；那一頁平常藏著，要右鍵點 Exit 才出現（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\HandlerSys.cpp` 第 1204-1212 行，未翻；屬 R92＝Handler System 隱藏手勢那一題） | 不翻（網頁碰不到那一頁） |
| TestIF_File_BarCode | pgc2DIDChange（兩排共用），`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\BarCode\BarCode.cpp` 第 2571-2580 行 | 重畫條碼統計表頭（主畫面 Lot 資訊表，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\BarCode\BarCode.cpp` 第 1475 行起）與盤面小圖（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\BarCode\BarCode.cpp` 第 8378-8397 行） | 未翻；同一支畫面程式在開頁已當純畫面處理（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\TestIF_File_BarCode.py` 第 105-106、126-127 行） | 不翻（純畫面） |
| UserDefForm_File | PageControl1Change，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayForm.cpp` 第 455-467 行 | 重排「從 Type? 複製」下拉（排除目前這一型） | 未翻；複製功能在網頁做（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\UserDefForm_File.py` 的 replace「FormShow 150」那一條） | 不翻（除非複製改成 C++ 做） |
| StartCondition | pgLifeTimeChange，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cStartCondition.cpp` 第 1199-1221 行 | 把該頁的接觸次數警報值放進輸入框 | 已翻，C++ 開頁時先算好每一頁的值（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\StartCondition.cpp` 第 266-291 行），網頁補件切頁時照放 | 不用動 |
| AutoCalSuckZ | PageControl1Change，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\ProductionInfo\ProductionInfo.cpp` 第 3070 行 | Halt／Pause 狀態頁的顯示 | 未翻；沒有網頁 | 等有網頁再說 |

（ST01-E 交代的「五支」是 IniConfig pcConfigChange、Temperature、HSys、BarCode、UserDefForm；golden 另外還有 IniConfig 外層 PageControl1Change（Q46 也提到）、Start Condition、AutoCalSuckZ 三支，共 8 支。有網頁的設定頁上還沒翻的是 6 支。）

---

## 3. 設計

### 3.1 設計 (1)：轉檔工具替每一排分頁產生「一共幾頁」表（R105）

**要改的只有轉檔工具一個檔** `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\gen_editlist.py`（St01 的共用工具），約 12 行 Python：

1. 在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\gen_editlist.py` 產生父子表的地方（第 417-421 行之後、第 422 行之前）：從「程式用到的元件＋上層」（同一個檔第 410-416 行已算好的 names_used 與 anc）挑出型別是分頁控制的，依 golden .dfm 的順序（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\gen_editlist.py` 第 190 行起的 dfm_parents 已照 .dfm 由上往下走，回傳的父子表本身就是 .dfm 順序）列出**每一排底下全部的頁（包含沒用到的）**，每排產生一行表：
   `static const char* const k<前綴>_Pages_<分頁控制>[] = {"頁0", "頁1", …};`
2. 在開機建容器替身的函式 `<前綴>_CreateContainerProxies()`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\gen_editlist.py` 第 422-427 行產生；34 個結構的手寫入口開機都會呼叫它）裡、`ELSetParents` 之後，每排加一行：
   `filerw::ELSetPageOrder("<golden 表單類別>", "<分頁控制>", k<前綴>_Pages_<分頁控制>, <頁數>);`
3. C++ 共用層**不用改**（接收端 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditList.cpp` 第 221-225 行已經有，現在沒有呼叫端）；手寫入口不用改；ctest EditList_PageIndex 不用改（它只測共用層）。
4. 實測（評估版工具在暫存區複本上跑）：64 排、28 個產生檔各多 4～42 行，**只加不改**；範例（Tester IF，St02 的產生檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_TesterIF.gen.inc` 多的 8 行）：
   ```
   // golden cTesterIF.dfm 的分頁頁序（DFM 順序＝VCL PageIndex，0 起算；沒用到的分頁也列）
   static const char* const kTIF_Pages_PageControl1[] = {"rgTime1", "rgTime2", "rgPurgeAir", "tsRT", "tsEQC"};
   static const char* const kTIF_Pages_PageControl2[] = {"tsDio", "tsGpib", "tsRs232", "tsTCPIP"};
   static const char* const kTIF_Pages_pgcRS232[] = {"tsRS232Setting", "tsSLTSetting"};

       filerw::ELSetPageOrder("TFTestIF", "PageControl1", kTIF_Pages_PageControl1, 5);
       filerw::ELSetPageOrder("TFTestIF", "PageControl2", kTIF_Pages_PageControl2, 4);
       filerw::ELSetPageOrder("TFTestIF", "pgcRS232", kTIF_Pages_pgcRS232, 2);
   ```
5. 效果：10 排「一共幾頁」變正確（2.1、2.2 表「一共幾頁」欄粗體的那些），其餘 54 排數字不變但改成「精確」而非「下限」；操作員畫面、存檔結果都不變。

### 3.2 設計 (2)：開畫面時的分頁照 golden（R106）

分四塊，(2a)～(2c) 是 C++（St01），(2e) 是網頁引擎（Jimmy）。

**(2a) 開機套「設計時停的頁」**：轉檔工具多讀 .dfm 的 `ActivePage = 頁名`（目前的屬性讀取 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\gen_editlist.py` 第 206-207 行不收它，另寫 10 行小函式），在開機套設計期狀態的 `<前綴>_DfmState()`（第 429 行起產生）裡，對「設計時停的頁不是第 0 頁」的每一排加一行：
`EL<TPageControl>("<表單類別>", "<分頁控制>")->ActivePageIndex = <N>;   // golden DFM ActivePage = <頁名>`
實測 35 行、19 個產生檔。順序對：VCL 也是先載入 .dfm、再跑建構子與建立畫面程式，手寫入口也是先呼叫 DfmState 再呼叫建構子。

**(2b) 把 golden「切到某頁」翻成真的切頁**：轉檔工具在「純畫面」規則之前（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\gen_editlist.py` 第 303 行之前）加兩條改寫，只改「替身分頁控制」的寫法（別的接收者照舊當純畫面）：
- 切頁 `EL<TPageControl>(…)->ActivePage = <頁>;` → `filerw::ELSetActivePage(<分頁控制替身>, <頁替身>);`
- 讀目前頁 `EL<TPageControl>(…)->ActivePage` → `filerw::ELActivePage(<分頁控制替身>)`（回傳目前那一頁的替身，可以拿來比較，例 `== EL<TTabSheet>(…, "tsATC")`，或讀 `->Tag`）
實測：正在執行的程式 20 處改成真的切頁（ACTForm 1、ArmSpeed 1、IniConfig 3、Offset 3、BarCode 2、Tester IF 4、Yield Monitoring 5、Tray Form 1）；讀目前頁 0 處在執行的程式裡（30 處都在已停用的 `#if 0` 原文裡，只是文字跟著變）。
C++ 共用層 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditList.h`／`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditList.cpp` 要新增兩支函式（約 40 行）：
- `ELSetActivePage(分頁控制, 頁)`：用 (1) 的「一共幾頁」表查出這一頁是第幾頁，設進去；查不到（頁是 nullptr、不是這一排的）⇒ 不動、記一筆待辦。要查「替身的名字」，替身登記函式 ELKeep（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditList.cpp` 第 75-86 行）多記一張「物件 → 名字」對照。Tester IF 的 `tsTemp[Index]`（陣列取頁，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTesterIF.cpp` 第 1198 行）也靠這個查。
- `ELActivePage(分頁控制)`：回傳表上第 N 頁的替身；N 超出範圍回 nullptr。
- ctest：在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_editlist_pageindex.cpp` 加案例（查得到、查不到、陣列取頁、藏起來的頁照收）。改共用層要跑全量 ctest 並比對失敗清單。
- 另一條保險（建議一起做）：轉檔工具在收「存檔流程讀的替身」（mustSend，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\gen_editlist.py` 第 319-333 行）時**排除分頁控制**——網頁送不送「第幾頁」都可以（R102），不能因為存檔程式看了目前頁就把整頁存檔擋住（Tray Assignment、Bin Select 今天就是為了躲這個才手寫，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TrayForm.cpp` 第 37-42 行註解）。今天 34 個產生檔的 mustSend 裡沒有任何分頁控制（查過），所以這條目前不改變任何輸出。

**(2c) 補翻 golden 的「建立畫面」程式**：Temp_Set 的溫控設定排設計時停在 Others（3），golden 建立畫面時切回 0（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp` 第 428-431 行 FormCreate），移植樹沒翻。只做 (2a) 不做這一步，C++ 會記成 Others、跟 BCB 不同。做法：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\Temperature.py` 的 methods 加 FormCreate，手寫入口 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Temperature.cpp` 開機在建構子之後呼叫一次（照 VCL：建構子→OnCreate）。全部 34 個結構裡只有這一處（Monitor 的建構子也切頁，但設計時停的本來就是 0，結果一樣）。今晚有人在改 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\Temperature.py`，要等它合進來。

**(2d) 不在這一步做的：「藏起目前頁就跳到旁邊」規則**（1.1 第 4 條）。要做就得把所有「設定頁看不看得見」的程式行改成呼叫函式：34 個產生檔裡正在執行的有 240 行、18 個檔。影響例：Configuration N 那一排，N05 頁多數機台被藏（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp` 第 5023 行），BCB 會停在下一頁；C++ 會記成第 0 頁（藏起來的 N05）。多數畫面的開畫面程式藏完頁會再明寫切頁，所以影響只剩零星幾排。建議另開一題，等 (2) 上線後再看。

**(2e) Jimmy 的網頁引擎要做的**（`D:\HT9045\web\page\ht9045_wire_engine.js`）
1. **載入時照 C++ 切頁**：套值函式（`D:\HT9045\web\page\ht9045_wire_engine.js` 第 1131-1187 行）遇到分頁控制替身帶「第幾頁」，先記下；整頁套完（第 1188-1223 行最後）再點 `:scope > .pcTabs > .tab[data-t="N"]`，外層先、內層後。點的寫法第 1591-1594 行已經有。**程式切頁不能觸發網頁即時事件**（照 BCB：程式切頁不跑切頁事件）。藏起來的頁照點（R103：BCB 允許停在藏起來的頁；點擊是程式呼叫，頁籤看不見也點得動）。
2. **什麼時候切**：建議只在第一次載入時切；存檔成功後的重讀（`D:\HT9045\web\page\ht9045_wire_engine.js` 第 1279-1290 行「寫完一定重讀」）保留操作員目前的頁（題 3）。
3. **存檔時送目前頁**：存檔（`D:\HT9045\web\page\ht9045_wire_engine.js` 第 1249 行起）對每個分頁控制替身送 `{"<分頁控制>":{"activePageIndex":<目前頁籤的 data-t>}}`；C++ 已經會收、不合理的只丟那一筆（R102）。停用的分頁控制（editable＝false）建議不送，免得每次存檔多一行待辦（題 4）。
4. 已自己切頁的網頁補件（Cleaning、Start Condition、Tester IF）不用改：它們在引擎載入完之後才跑，最後以它們為準，值也跟 C++ 一致。

### 3.3 設計 (3)：切頁事件翻成網頁即時事件（OnChange → form.event）

**C++ 端（St01）**
- 做法跟現有五頁的事件一樣（`D:\HT9045\.claude\skills\ht9045-html-json\references\route-c-golden-bridge.md` 第 3.0g-4 節）：在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\<結構>.py` 的 events 加 `('<分頁控制>', 'change', '<事件程式>')`，事件程式加進 methods（參數拿掉 Sender），重跑 `--only <結構>`，手寫入口加一行事件登記。
- **新頁數怎麼送**：網頁把「剛點的那一頁」放在即時事件的「畫面其他格目前的值」欄（state）裡（`{"<分頁控制>":{"activePageIndex":N}}`）。commit c9d3c932（主要改 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditList.cpp`）已經支援：C++ 先照 state 設好分頁、再跑事件程式（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp` 第 421-452 行），所以**即時事件的 C++ 本體不用改**。可選的加強：把分頁當成「這個元件自己的值」（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp` 第 399-417 行驗值、第 454-470 行套值，解析在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_FormEvent.cpp` 第 74-102 行），順便擋「操作員點得到的一定是看得見的頁」。
- 事件程式讀目前頁要 (2b) 的 `ELActivePage`；Temp_Set 還要兩支小函式：頁的「看得見位置」（golden `tsATC->TabIndex`）與「頁號」（golden `tsArm1->PageIndex`），都查 (1) 的表。
- 逐支：
  - **Temp_Set pgcTempOffsetChange**：照翻。golden 怪處照留並加註：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp` 第 5226 行拿「頁號」（藏起來的頁也算）跟 ATC 頁的「看得見位置」比；V912 只會藏 ATC 後面的頁（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp` 第 883-901、1326 行），ATC 前面四頁從不藏，所以實際上兩個數字相同。
  - **Configuration 兩支**：Q46 選 A 才做。先要讓 Configuration 收得到網頁即時事件——它不是用共用開頁描述登記的（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp` 沒有 PageRegistrar），即時事件找不到這一頁、也查不到「這個等級開過頁」；要替它補事件入口。（⛔ ST01-E 20260927 22:xx 更正：這個入口今晚已經補上——commit `64ade3b7`（主要檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp` 檔尾），Configuration 另登記一個只給即時事件用的別名頁「Config.Configuration」，開頁時記「這個等級開過頁」；所以 Configuration 兩支只剩「翻切頁程式」本身要做）。ASE 高雄的密碼（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp` 第 6887-6904 行，跳 BCB 密碼視窗）伺服器做不到，照現有規則當「密碼錯」⇒ 切回 Soft 頁、記待辦（連 Q45）。
  - 其他三支不翻（2.3 表的理由）。
- 事件回給網頁的「改了什麼」會帶分頁（例 ASE 高雄被切回 Soft），也會帶格子的看得見／可不可改。

**網頁端（Jimmy）**
1. 操作員**點**頁籤時，若開頁資料的 events 裡有這一排（C++ 開頁多帶，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditPage.cpp` 第 325 行起），送網頁即時事件：control＝分頁控制、event＝change、state 放「新頁」**以及畫面上其他格目前的值**。
2. 為什麼 state 要帶其他格：C++ 存檔時會丟掉「當下看不見」的格子的網頁值（`D:\HT9045\.claude\skills\ht9045-html-json\references\route-c-golden-bridge.md` 第 3.0b 節第 4 步）。如果操作員先在 Normal 頁改了基準溫度、再切到 ATC 頁（格子被藏）才按存檔，沒有 state 的話改的值會被丟掉；BCB 會照存。帶了 state，C++ 在藏起來之前就收下新值，存檔結果跟 BCB 一樣。
3. 照回覆的 changed 局部套用；changed 裡有分頁就**靜默**切過去（不再送事件）。
4. 網頁即時事件的送出點（Q40）目前在引擎裡還沒有（`D:\HT9045\.claude\skills\ht9045-html-json\references\route-c-golden-bridge.md` 第 3.0g-1 節），這一步跟它一起做。

---

## 4. 每一步影響哪些產生檔（實測）

量測方法：暫存區複本（commit `c20bdec2` 的內容；那顆 commit 主要改 `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md`，程式與它的上一顆相同）先跑原版工具——34 個產生檔與 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_editlist_sources.cmake` 跟 `c20bdec2` 的內容**位元組完全相同**（換行 LF），證明重跑本身不帶進別的差異（Teach 用自己的工具 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\gen_teach_editlist.py`，不受影響）；再跑評估版（(1)，以及 (1)＋(2a)(2b)），用 Python difflib 算行數。

| 產生檔（都在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\`） | 行數 | (1) 一共幾頁表 | (2a)(2b)（疊在 (1) 上）：執行中的程式 | (2b)：停用的 #if 0 原文 |
|---|---|---|---|---|
| ACTForm.gen.inc | 901 | +4 | +1 −1 | — |
| AOAOffset.gen.inc | 579 | +4 | +1 | — |
| AOISetup.gen.inc | 1286 | +6 | +1 | — |
| ArmSpeed_File.gen.inc（今晚有人改設定） | 2742 | +4 | +2 −1 | — |
| AutoCalSuckZ.gen.inc | 307 | +6 | +1 | — |
| BinSelect.gen.inc | 2145 | +4 | — | ±7 |
| ContactForce.gen.inc | 1479 | +4 | +1 | — |
| GroundMan.gen.inc | 508 | +4 | — | — |
| HSys.gen.inc | 4085 | +4 | +1 | — |
| IniConfig.gen.inc（今晚有人改設定） | 9496 | +42 | +16 −3 | — |
| IniConfig_OCR.gen.inc | 348 | +4 | +1 | — |
| Ld_UldDelayTime.gen.inc | 344 | +4 | — | — |
| Monitor.gen.inc | 172 | +4 | — | — |
| Offset_File.gen.inc | 2891 | +4 | +3 −3 | — |
| Rotate.gen.inc | 804 | +4 | +1 | — |
| StartCondition.gen.inc | 2136 | +6 | +2 | ±33 |
| Temperature.gen.inc（今晚有人改設定） | 6382 | +10 | +3 | — |
| TestIF_File_AutoAlignment.gen.inc | 586 | +6 | +1 | — |
| TestIF_File_BarCode.gen.inc | 2327 | +14 | +5 −2 | — |
| TestIF_File_Cleaning.gen.inc | 3311 | +6 | — | — |
| TestIF_File_FixAICCD.gen.inc | 175 | +4 | — | — |
| TestIF_File_QAMode.gen.inc | 441 | +4 | — | — |
| TestIF_File_SetUp.gen.inc | 4324 | +4 | — | — |
| **TestIF_File_TesterIF.gen.inc（St02 的）** | 1939 | +8（3 張表＋3 行登記＋註解與空行） | +5 −4（開機設介面頁＝GPIB 1 行；開頁 3 處與介面切換 1 處變成真的切頁） | — |
| TestIF_File_VacuumUnit.gen.inc | 502 | +4 | — | — |
| TestIF_File_YieldMonitoring.gen.inc | 3929 | +8 | +8 −5 | — |
| TrayForm.gen.inc | 2705 | +4 | +1（開機設第 1 頁，跟手寫入口現有那一行重複、值相同） | ±3 |
| UserDefForm_File.gen.inc | 897 | +4 | +1 −1 | — |
| **合計** | | **28 檔 +184／−0** | **19 檔約 +55／−20** | **3 檔約 ±43** |

不變的產生檔：DeviceForm_File、IniConfig_CounterSel、ShuttleMove、TTLCfg、TestIF_File_Magazine、Winway（沒有分頁）、Teach（另一支工具）。`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_editlist_sources.cmake` 不變。

(1)(2) 以外要手改的檔：
- (2)：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditList.h`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditList.cpp`（兩支函式＋名字對照）、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_editlist_pageindex.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\Temperature.py`＋`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Temperature.cpp`（FormCreate）。
- (3)：每支事件一個結構設定檔＋手寫入口一行；Configuration 另要補事件入口（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp`）。
- 網頁：`D:\HT9045\web\page\ht9045_wire_engine.js`（Jimmy 的檔）。

重產方式：一律 `--only <結構>` 一個一個重產（多人同時作業的規矩，`D:\HT9045\.claude\skills\ht9045-html-json\references\route-c-golden-bridge.md` 第 3.0g-4 節第 1 步的冪等確認照做）。IniConfig、ArmSpeed_File、Temperature 等今晚的改動合進來之後再重產那三個；St02 的 Tester IF 產生檔要在交接檔（FROM_STEVEN）寫明「只多兩段新行、開頁與介面切換四行變成真的切頁」。

---

## 5. 風險（逐頁）

1. **操作員看到的「開頁那一頁」會變**（(2)＋網頁引擎第 1 點之後才發生；只有 C++ 做完不會）：
   - Configuration：開機後第一次打開停在「N [ Network ]」，A、C、D、E、F、L、O、N14、N25、P 各群組開在設計殘留的那一頁（例 A71-A80、D80、L36-L42）。BCB 使用者一樣會看到，但習慣網頁第 0 頁的人會覺得「跑掉了」。ASE 高雄外層停在 Soft。
   - Yield Monitoring：Retest 系列模式停在 Re-Test；兩排 Bin Alarm 停在 Bin Alarm3。
   - Start Condition：停在 Life Time。Temp_Set：Others 群組停在 Others。BarCode：pc2DID 停在 Setting、浮料檢查停在 Setting，8CCD／讀夾子碼機台停在 8CCD／Unloader Clip。Tester IF：只有 SPIL 會停在 RT／EQC。
   - Tray Assignment：網頁改停在 Retest 群組（跟 C++ 一致了）。
   - 「上次離開的那一頁」：BCB 下次打開停在上次離開的頁；網頁只有「存檔時送過的頁」C++ 才記得，沒存檔就離開的不記。
   - 開站時就決定（1.2 最後一條）：Yield Monitoring、Tester IF（SPIL）這種看當下模式決定的，開站後才換模式要重新整理頁面才對。
2. **存檔後重讀會跳頁**（網頁引擎每次載入都切時）：Speed 存完跳回第 0 頁（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSpeed.cpp` 第 49 行）、Yield Monitoring 跳回模式那一頁、Tester IF 跳回 Time1、Handler System／Ground Man 跳回第 0 頁。BCB 按存檔不會重跑開畫面程式，不會跳。⇒ 題 3。
3. **Tray Assignment 的存檔結果會變**（網頁引擎第 3 點「存檔送目前頁」之後）：開了 bFTBin2RTBin 的機台，在 Normal Test 頁存檔會把 FT 的 bin 放置複製到 RT（跟 BCB 一樣，但跟網頁今天不同）；在 Retest 頁存檔不複製。ShowCompnet 的 Auto 類型文字（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp` 第 1033-1040 行）也會照目前頁。⇒ 題 4。
4. **Temp_Set 溫控設定排開錯頁**：只做 (2a) 沒做 (2c)，C++ 記成 Others（3），BCB 是 Hot（0）。(2a) 與 (2c) 要同一批上。
5. **Start Condition 兩份「目前頁」**：它自己的頁指標（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\StartCondition.py` 第 238-239 行、第 276 行）跟 commit c9d3c932（主要改 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditList.cpp`）加的通用值是兩份；(2a) 開機兩份都是 Socket Count，但開頁程式只改自己那份。今天網頁補件以自己那份為準、存檔也送自己那份，行為正確；通用那份只是「報出去的數字可能不準」。之後可以把 Start Condition 改用 (2b) 的通用寫法、退掉自己那份（另開清理題，不急）。
6. **沒有網頁的結構**：AOISetup 的開畫面程式沒翻，(2a) 會讓 C++ 記成設計時停的第 6 頁，BCB 依機種是 2／4／0；AutoCalSuckZ、Rotate、ContactForce 同類。今天沒有網頁、沒人送，沒有影響；將來接網頁時要一起翻開畫面程式。
7. **藏起目前頁不跳頁**（(2d) 不做）：Configuration N 排在 N05 被藏的機台，BCB 停在下一頁，C++ 記第 0 頁；網頁的頁籤小程式只負責點擊，第 0 頁的內容可能在頁籤藏起來後仍然顯示（推論，沒在瀏覽器驗）。
8. **待辦變多**：網頁若對停用中的分頁控制也送「第幾頁」，C++ 每次存檔記一行「點不到、不收」。網頁引擎第 3 點建議不送。
9. **即時事件漏帶其他格的值**（(3)）：見 3.3 網頁端第 2 點；漏帶會讓「先改、再切到藏格子的頁、再存」的值被丟掉。
10. **重產衝突**：IniConfig、ArmSpeed_File、Temperature 的設定檔今晚有人在改；Tester IF 是 St02 的。一律 `--only`、等對方推上去再重產，重產前後做冪等確認。
11. **頁名對不上（既有，不是這次造成）**：Yield Monitoring 網頁兩排 Bin Alarm 的第 4 頁叫 tsBinAlarm4_FT／tsBinAlarm4_RT，golden V912 .dfm 叫 TabSheet1／TabSheet2（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uYieldMonitoring.dfm` 第 304、2655 行）。「第幾頁」用號碼，不受影響；但用頁名套「看不看得見」的地方對不上（網頁可能是用別版 .dfm 產生的，沒查）。

---

## 6. 工作量

| 步驟 | 誰 | 估計 | 內容 |
|---|---|---|---|
| (1) 一共幾頁表 | St01 | 0.5 人天 | 工具約 12 行；`--only` 重產 28 檔；兩組態（模擬／出貨）語法檢查；探針：Handler System 送第 10 頁不再記待辦 |
| (2a)(2b)(2c) C++ | St01 | 1～1.5 人天 | 工具約 25 行＋兩支共用函式＋ctest 案例＋Temperature FormCreate；重產 20 檔；全量 ctest 比對失敗清單；探針：開頁資料的「第幾頁」逐頁對 2.1 表 |
| (2e) 網頁引擎 | Jimmy | 0.5 人天＋上機 | 載入切頁（第一次）、存檔送頁；17 頁各開一次看停的頁；Tray Assignment 上機存 FT／RT 各一次 |
| (3) Temp_Set 事件 | St01＋Jimmy | 0.5 人天 C++；網頁跟 Q40 送出點一起 | 事件設定＋兩支小函式＋探針 |
| (3) Configuration 兩支 | St01＋Jimmy | 1～1.5 人天 C++ | 先補 Configuration 的事件入口；Q46 選 A 才做 |
| (2d) 藏頁跳頁規則 | St01 | 1 人天以上 | 240 行改寫；不建議現在做 |

---

## 7. 建議順序

1. **(1) 一共幾頁表**——現在做。今晚改動中的三個結構等合進來再 `--only` 重產，其他 25 檔先做。操作員看不到差別。
2. **(2a)(2b)(2c) C++**——接著做。網頁還不照 C++ 切頁，所以做完畫面不變；唯一的行為面影響（開頁資料裡的「第幾頁」數字）沒有人讀。(2a) 與 (2c) 同一批。
3. **(2e) 網頁引擎：載入時切頁**（Jimmy）——題 2、題 3 決定後。這一步起操作員看得到差別（第 5 節第 1 點）。
4. **(2e) 網頁引擎：存檔送目前頁**（Jimmy）——題 4 決定後。Tray Assignment 行為改變，上機驗、通知操作員。
5. **(3) Temp_Set 切頁事件**——等網頁即時事件的送出點（Q40）。
6. **(3) Configuration 兩支**——Q46 選 A 才做；選 B 就改在開頁時鎖（Q46 自己的範圍），不做事件。
7. 以後再說：(2d) 藏頁跳頁規則；Start Condition 退掉自己那份頁指標（第 5 節第 5 點）。

---

## 8. 要 Steven 決定的題目全文（decisions-pending 的 Q51；是 R105、R106 的細化，題 5 連 Q46）

（本文其他地方寫的「題 1」～「題 5」，就是下面的 Q51-1～Q51-5。）

### Q51-1（R105 細化）：C++ 要不要記住每一排分頁「一共幾頁、依序是哪幾頁」
**背景**：設定畫面上有一排排可以切換的分頁標籤（BCB 叫分頁控制 TPageControl）。20260927 起，網頁存檔時可以順便告訴 C++「操作員現在停在第幾頁」（commit c9d3c932，主要改 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditList.cpp`）；C++ 要知道這一排一共幾頁，才能判斷送來的頁數合不合理。現在 C++ 只數得到「程式有用到的那幾頁」，所以有 10 排少算了頁，排在後面的頁會被當成不合理：存檔結果多一行假的待辦，而且 C++ 記成還停在原來那頁。有網頁的是 4 排：Handler System 的「Heater」頁、Temp_Set 溫度補償的「Multi Sensor」頁、Tester IF 的「SLT Setting」頁、Ground Man 的「Log」頁。網頁引擎還不會送頁數，所以今天操作員看不到；Jimmy 接上之後就會出現。
**選項**：A 補：轉檔工具（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\gen_editlist.py`）照 BCB 畫面設計檔替 64 排分頁各產生一張完整頁序表（28 個產生檔各多 4～42 行、只加不改，畫面與存檔結果都不變）／B 維持：少算的頁照舊被擋、照舊記假待辦。
**St01 建議**：A（約半天，不用等 Jimmy）。
**例子**：操作員在 Handler System 切到最後一頁「Heater」改完按存檔——現在存檔結果會多一行「第 10 頁超出 0～9，伺服器保留原頁」（假的）；選 A 之後照收，沒有這一行。

### Q51-2（R106 細化一）：C++ 記的「打開畫面時停在哪一頁」要照 BCB 的哪一種
**背景**：BCB 打開設定畫面時停在哪一頁，由三件事決定：①畫面設計檔裡設計者最後停的那一頁（程式開機時套一次，常常不是第一頁）；②打開畫面的程式明寫「切到某頁」（例 Yield Monitoring 依 Run Mode 切到 Normal 或 Re-Test 頁，golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uYieldMonitoring.cpp` 第 2646-2674 行）；③沒寫的，停在操作員上次離開的那一頁。移植樹兩種都沒照：一律從最左邊那頁開始，並且把②當成純畫面丟掉（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\gen_editlist.py` 第 303-311 行）。逐頁比對有網頁的 17 頁、50 排分頁：18 排一定跟 BCB 不同，其中 11 排在 Configuration、幾乎都是①（例 Configuration 開機後第一次打開停在「N [ Network ]」，golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.dfm` 第 457 行）；另外 5 排看機種或模式（ASE 高雄、SPIL、Shuttle 浮料檢查、下 8CCD、Retest 模式）。對照表：`D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\page-control-tabs.md` 第 2.1 節。
**選項**：A ①②都照（最像 BCB；Configuration 會開在 N 頁）／B 只照②，①一律最左邊那頁（偏離 BCB，程式裡標 [W906]＝註明這是移植樹刻意跟 BCB 不同的地方；Tray Assignment 例外仍照①停在 Retest 群組，因為 BCB 存檔會看目前在哪一頁）／C 維持現狀：一律最左邊那頁，②也不照。
**St01 建議**：A（照 golden）。這一步只改 C++ 記的值，做完畫面不會變（網頁要不要照著切是題 3），可以先做；約 1～1.5 人天。
**例子**：Run Mode 是 Retest 時打開 Yield Monitoring——BCB 停在「Re-Test」頁；A、B 都是 Re-Test；C 停在「Normal」。開機後第一次打開 Configuration——BCB 停在 N 頁；A 是 N 頁；B、C 是 A 頁。

### Q51-3（R106 細化二，Jimmy 的網頁引擎）：網頁什麼時候照 C++ 切到那一頁
**背景**：題 2 只改 C++ 記的值；要操作員看得到，Jimmy 的網頁引擎（`D:\HT9045\web\page\ht9045_wire_engine.js` 第 1131-1223 行，載入設定頁時套值的地方）要照 C++ 給的頁數去點頁籤。網頁每次存檔後會重讀一次（會重跑 BCB 打開畫面的程式），而有的打開畫面程式寫死「切回第一頁」（例 Speed，golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSpeed.cpp` 第 49 行）；BCB 按存檔不會重跑打開畫面的程式，畫面不會跳。另外網頁在開站時就把設定頁載好（`D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\window-open-edge.md`），所以「打開時停的頁」其實是開站那一刻算的。
**選項**：A 只有第一次載入時照 C++ 切；存檔後重讀保留操作員目前的頁／B 每次載入都照 C++ 切（存完可能跳回第一頁）／C 網頁不切（C++ 記它的，畫面照舊停最左邊那頁）。
**St01 建議**：A（Jimmy 約半天，改完 17 頁各開一次看停的頁）。
**例子**：在 Speed 的「Index Arm Condition」頁改完按存檔——A 停在 Index Arm Condition（跟 BCB 一樣）；B 跳回「Speed & Acc/Dec Setting」；C 不會跳，但 Yield Monitoring 等頁打開時不會照 BCB 停在 Re-Test。

### Q51-4（R106 細化三）：存檔時網頁要不要把「目前停在哪一頁」送給 C++
**背景**：有些 BCB 存檔程式會看操作員現在在哪一頁。Tray Assignment：Configuration 開了「FT 存檔時把 RT 設定跟 FT 一樣」（bFTBin2RTBin）時，在 Normal Test（FT）頁按存檔，BCB 會把 FT 的 bin 放置複製到 RT（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayAssignment.cpp` 第 1353-1372 行）。網頁現在不送，C++ 以為一直停在 Retest 頁（設計檔停的那頁，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TrayForm.cpp` 第 454-456 行），所以網頁存檔永遠不複製。
**選項**：A 送（Jimmy 改引擎的存檔；C++ 已經會收，不合理的只丟那一筆，R102）：在 FT 頁存才複製，跟 BCB 一樣——對網頁使用者是「行為改變」／B 不送：維持網頁永遠不複製（跟 BCB 不同）。
**St01 建議**：A；Jimmy 改完先在 Tray Assignment 上機各存一次 FT 頁、RT 頁確認，並告訴操作員這個差別。停用中的分頁排不送，免得每次存檔多一行待辦。
**例子**：開了該選項的機台，操作員在 Normal Test 頁把 Auto1 改成 Bin 2 後存檔——BCB：RT 的 Auto1 也變成 Bin 2；網頁現在：RT 不變；選 A 之後：RT 也變成 Bin 2。

### Q51-5（切頁事件；Configuration 兩支連 Q46）：操作員點分頁時 BCB 自動跑的程式，網頁要照做哪幾支
**背景**：BCB 有些分頁在操作員點頁籤時會自動跑一段程式（切頁事件；程式自己切頁時不跑）。有網頁的設定頁還沒翻的有 6 支、分成 5 項（全文與出處：`D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\page-control-tabs.md` 第 2.3 節）：
1. Configuration 外層＋內層（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp` 第 6101-6123 行）：依權限檔整頁鎖住，ASE 高雄切到 Config 頁要輸入密碼——就是 Q46（密碼另連 Q45）。Configuration 頁收網頁即時事件的入口今晚已補上（commit `64ade3b7`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp` 檔尾的別名頁「Config.Configuration」），只剩翻切頁程式本身。
2. Temp_Set 溫度補償（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp` 第 5224-5262 行）：切到 ATC 頁時把「基準溫度」那一組（7 個輸入格＋標籤＋清除鈕）藏起來；Arm 1／Arm 2 頁才顯示排序鈕；TSMC 台南另切兩塊面板。
3. Handler System（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\HandlerSys.cpp` 第 1286-1289 行）：只在「Customer Code」頁顯示搜尋框；這一頁平常藏著，要右鍵點 Exit 才出現（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\HandlerSys.cpp` 第 1204-1212 行），網頁沒有這個動作，所以碰不到。
4. BarCode（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\BarCode\BarCode.cpp` 第 2571-2580 行）：重畫條碼統計表頭與盤面小圖，純畫面。
5. Tray Form（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTrayForm.cpp` 第 455-467 行）：重排「從 Type? 複製」下拉；複製本身現在是網頁自己做。
（Start Condition 那支已由網頁照做；ProductionInfo 那支還沒有網頁。）
**選項**：A 只翻 2（Temp_Set）；1 跟著 Q46（Q46 選 A 才翻，選 B 就改成開頁時鎖、不翻事件）；3、4、5 不翻並在程式註明理由／B 5 項全翻（3 要先做右鍵手勢，4、5 伺服器做的是畫面，網頁看不出差別）／C 都不翻。
**St01 建議**：A。Temp_Set 約半天 C++（要 Q51-1、Q51-2 先做）；網頁的送出點由 Jimmy 跟 Q40 的送出點一起加，送的時候要把畫面上其他格目前的值一起帶上。
**例子**：Temp_Set 切到 ATC 頁——BCB 的「基準溫度」幾格消失、改不到；網頁現在還看得到、改了也會存；選 A 之後切頁時 C++ 跑這支程式，格子跟著藏起來，存檔不收那幾格。如果操作員是先在 Normal 頁改了基準溫度才切到 ATC 頁存檔，BCB 照存，選 A 也照存（因為網頁切頁時已經把新值帶給 C++）。

---

## 9. 沒查證／推論的地方

1. **沒有 build、沒有編譯**：3.2 的兩支共用函式（`ELSetActivePage`、`ELActivePage`）與 Temp_Set 要的兩支小函式都還不存在；評估版工具只產生文字、量行數，產生的 C++ 沒有編譯過。
2. 行數用 Python difflib 算，逐檔的增減可能因對齊方式差 ±3 行；「執行中／#if 0 裡」的分類是依產生檔的 `#if 0 // GATE`／`#endif // GATE` 標記判斷。評估版工具在 `C:\Users\steven\AppData\Local\Temp\claude\d---github\99781e6b-2c4e-4591-81e8-fc879566566c\scratchpad\eval4\HT9011UC_Cpp_V3.33.906.0\tools\gen_editlist_eval.py`（`--eval1`＝(1)、`--eval2`＝(1)＋(2a)(2b)），由 `C:\Users\steven\AppData\Local\Temp\claude\d---github\99781e6b-2c4e-4591-81e8-fc879566566c\scratchpad\eval4\make_eval_gen.py` 從原版工具加上評估修改產生（暫存區是這個工作階段專用，之後可能被清掉）。
3. 三條 BCB 元件庫行為是依既有認定與 BCB 元件庫的已知寫法，這次沒有再對 BCB 原始碼驗：.dfm 裡頁的先後＝頁號（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_EditList.h` 第 159-162 行同一個認定）；程式切頁不跑切頁事件（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\StartCondition.cpp` 第 262-263 行）；藏起目前頁會跳到旁邊（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\StartCondition.py` 第 26 行）。
4. **瀏覽器沒有開**：網頁點藏起來的頁籤、第 0 頁內容在頁籤藏起來後是否還顯示，是讀頁面檔裡的小程式推的（`D:\HT9045\web\page\Config.Configuration.html` 第 62-80 行），沒有實測。
5. 「BCB 第一次打開停的頁」假設畫面物件開機建一次、一直留著（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\HT9045.cpp` 第 167-246 行），沒有追「上次離開的那一頁」的情況；看機種的 5 排沒有對真機設定檔查這台是哪一種。
6. golden 讀／設分頁的行是用規則式找「分頁控制名->ActivePage／ActivePageIndex／TabIndex／Pages／PageCount」與「頁名->PageIndex／TabVisible」，註解後面的不算；用別的變數間接拿到分頁控制（例 `TPageControl *p = pcConfig; p->ActivePage`）的寫法找不到。所屬方法是「這一行之前最後一個方法開頭」，放在方法外的行會算錯方法（這次列出的都有人工看過上下文）。
7. 今晚 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\IniConfig.py`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\ArmSpeed_File.py`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\Temperature.py` 的改動會讓這三個產生檔的行號、行數變；表裡是 commit `c20bdec2`（主要改 `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md`）當下的數字。
8. Yield Monitoring 網頁第 4 頁頁名跟 V912 .dfm 不同（第 5 節第 11 點），網頁是從哪一版 .dfm 產生的沒查。
9. 指令紀錄（20260927 21:5x～22:xx，全部唯讀）：`git -C D:\HT9045 show c20bdec2:<路徑>`、`git archive c20bdec2` 匯出到暫存區；分析程式都在 `C:\Users\steven\AppData\Local\Temp\claude\d---github\99781e6b-2c4e-4591-81e8-fc879566566c\scratchpad\eval4\` 資料夾：analyze.py（逐排盤點）、pages.py（網頁頁籤比對）、diffcount.py／gatecount.py／livecount.py（重產行數）、tabvis.py（藏頁行數）、savereads.py（存檔讀的替身有沒有分頁控制）；golden 用 `iconv -f cp950` 讀。
