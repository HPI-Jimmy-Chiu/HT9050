> 保存來源：`.claude/skills/cpp_build/references/header-slimming-and-motor-io-isolation-20261006.md`，main `2db43115d`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# 熱門標頭瘦身 ＋ Motor／IO 模組化評估（2026-10-06，main `84233b648`）

> Steven 1006 14:0x 要的「標頭瘦身」與「Motor／IO 做成 DLL」兩項詳細評估方案，並要求**搭配新版 skill 整理方案**（hpi- 結構，ST-GPT 執行中）。
> 評估由 ST01-M 的唯讀子代理做；〔實測〕＝量出來的，〔估計〕＝推算。數字只在 St01（Core Ultra 7 155H、NVMe、g++ 6.3）量過，別台請重量。
> **跟 skill 整理的對應**：本檔的 A 部分（標頭）屬 `cpp_build`；B 部分（Motor／IO 分層、HAL 隔離）的結論要落到 **hpi-motor-control／hpi-io-control** 的「通用」節（不分機型），`ht9045-motor-control` 退場時一併搬。ST-GPT 重整這兩支時請在 SKILL.md 放一行指標到本檔，不要複製內容。

## 0. 結論

1. **重編量最大的來源是「修改次數 × fan-out」，不是標頭有多大**〔實測，14 天、不含 merge〕：MachineType.h 28 次×971 obj、cmydef.h 15×835、fMain.h 32×374、cprod.h 5×854、mymotor.h 10×409。
2. **只改註解的 commit 也會觸發全量重編**〔實測〕：MachineType.h 9/28、mymotor.h 5/10、fMain.h 3/32 是純註解。這是零風險、馬上能做的部分（S0）。
3. **cmydef.h 最近新增的 141 行裡有 104 行是 IO 索引**（`extern const int Sn*/C_*/Sw*/M*`）。596 個直接 includer 中只有 309 個引用 IO 名稱。拆出 cmydef_io.h 並遷移 includer 後，這類修改的 fan-out 從 835 降到約 361 obj（−57%）〔估計〕。
4. **mymotor.h 改前置宣告，對 cprod.h 的 fan-out 幾乎沒幫助**：250 個 includer 裡只有 6 個會因此不再拉進 cprod.h（還有 mytray.h→cmydef.h→cprod.h 這條路）。它的價值在 B 部分的隔離。
5. **Motor／IO 做成 DLL 對編譯時間沒有好處**：兩者只占已編譯原始碼 2.8%、archive 共約 1.1 MB（ht9045_sm 是 32.2 MB），而且還有反向連結循環。**建議先做標頭隔離，DLL 不做。**
6. 全新建置的底是 `vclcompat/vcl_compat.h` 這層系統標頭（單獨 include 就 42.7k 行，占每個 TU 前處理量 35–70%），專案標頭瘦身對**全新建置**最多快約 5%〔估計〕；主要效益在**增量重編的 fan-out**。要縮短「改一個 .cpp」的時間，請看 `speedup-ideas.md`（Ninja、thin archive、只建目標、連結）。

## A1. 熱門標頭現況〔實測〕

| 標頭 | 行數 | 直接 includer（總/cpp/h/tests） | 遞移 TU / obj | 本身 include |
|---|---|---|---|---|
| cmydef.h | 6078 | 618/596/22/244 | 673/835 | MachineType.h, myTimer.h, cprod.h, cpublic.h (:8-11) |
| cprod.h | 3330 | 439/432/7/164 | 686/854 | MachineType.h, Config.h, CosFunction.h, `<map>`, myTimer.h (:5-9)；cprod_9050.inc (:490) |
| MachineType.h | 1913 | 408/366/42/152 | 754/971 | vcl_compat.h, `<vector>`, `<windows.h>` |
| Config.h | 1518 | 254/252/2/106 | 695/870 | vcl_compat.h, MachineType.h |
| forms/fMain.h | 1451 | 128/125/3/50 | 318/374 | fMain_Timers.h, FormWidgets.h, `<map>`, `<atomic>` |
| Motor/mymotor.h | 482 | 253/251/2/93 | 372/409 | HTMotor.h, mytray.h, cprod.h, myTimer.h, `<map>` (:66-79) |
| Motor/HTMotor.h | 237 | 28/20/8/11 | 377/415 | vcl_compat.h, `<windows.h>` |
| mycylin.h | 182 | 124/122/2/25 | 126/136 | vcl_compat.h, myTimer.h |
| myio.h | 77 | 3/2/1/0 | 2/2 | `<windows.h>` |
| mytray.h | 97 | 7 | 374/411 | **cmydef.h**, Public/MyProductionRecord.h (:23-24) |

其他：csystem.h 206 個直接 include（遞移 231 obj）；mysensor.h 151（161）；FormsFacade.h 會拉進 fMain.h 和約 25 個表單標頭。

## A2. mymotor.h 用到各個 include 的方式〔實測〕

| include | 怎麼用 | 能否前置宣告 |
|---|---|---|
| HTMotor.h | 只有 `HTMotor *Motor;`（:150） | 可以；要自己補 include vcl_compat.h（AnsiString） |
| mytray.h | 以值持有 `TMyTray Tray;`（:347），inline `FullIC()`（:366） | 不行；但 mytray.h 本身完全沒用到 cmydef.h（:23 可拿掉），`TMyProductionRecord*`（:59）可前置宣告 |
| cprod.h | 只用到 `ARM_CONDITION*`（:176） | 可以，但 cprod.h:2825-2878 是匿名 `typedef struct{}`，要補 tag（C++ linkage 名稱＝typedef 名，mangling 不變，objcompare 可驗）；MAX_ARM_Row/Col、X_PITCH_COUNT 來自 MachineType.h:481-502，要改直接 include |
| myTimer.h | 以值持有 TQPF_Timer | 不行，但只有 43 行 |

試編〔實測〕：擋掉 cprod.h／cmydef.h、前置宣告 ARM_CONDITION 後 `-fsyntax-only` 通過；`-E` 60,093 → 48,696 行（−19%）。
**陷阱**：cmydef.h 的 include guard 在 **:5808 就結束**，:5809-6078（237 個 extern、`#define PCI1203_IO 4`、3 個函式宣告）在 guard 外。拆 cmydef.h 第一步必須先把這段收進 guard。
HTMotor.h 改前置宣告，fan-out 415 → 約 103 obj〔估計〕；但 14 天只改 1 次，優先度低。

## A3. cmydef.h／cprod.h 內容分區〔實測〕

**cmydef.h**：4,925 個 extern（`extern const int` 2,145）、68 個 #define、0 個 #undef、0 個 inline、1 class（uPoint2D）、7 struct。596 個直接 includer 的使用數（token 比對，偏高）：

| 區 | 行 | 內容 | 名稱數 | 使用的 includer |
|---|---|---|---|---|
| A | 1-115 | 系統巨集（TOTAL_MOTOR、InterfaceType_*、卡型） | 52 | 158 |
| B | 116-301 | IC 狀態碼、加熱器型別、基本旗標 | 152 | 436 |
| C | 302-612 | C_* 氣缸索引 | 280 | 63 |
| D | 613-1682 | Sn* 感測器索引 | 927 | 146 |
| E | 1683-2103 | Sw* 開關索引 | 366 | 127 |
| F | 2104-2522 | M* 馬達索引 | 338 | 248 |
| G | 2523-5808 | 執行期全域（bool/int/AnsiString/Timer、SortingBinTray struct :3913-3960、AOA 常數 :5248-5310） | 2,683 | 542 |
| H | 5809-6078 | **guard 外**的新增項（HT9050 的 C_/Sn 等） | 238 | 132 |

- IO 區（C–F）共被 309 個用到；7 個 includer 完全沒用到 cmydef.h 的符號。
- cmydef.h 對其他標頭的實際依賴：cprod.h 只用 `TRAY_TYPE_PARA`（:3986 以值當成員）；cpublic.h 只用三個具名 struct 的 extern（可前置宣告）；myTimer.h 用 TQPF_Timer。

**cprod.h**：36 struct、4 class、3 enum、91 extern、inline getter 在 :198-208、:3146。

| 區 | 行 | 內容 | 使用 TU（611 個中） |
|---|---|---|---|
| P1 | 1-150 | TTLCfg、AutoTeachOffset、ScannerAOIIF | 17 |
| P2 | 151-368 | ARM_OFFSET、Offset、InvisibleOffset | 64 |
| P3 | 369-1139 | PROD_INFO_ST Prod（含 cprod_9050.inc） | 200 |
| P4 | 1140-1375 | LevelSet、DeviceForm、TrayForm、TRAY_TYPE_PARA | 224 |
| P5 | 1376-1648 | Temperature | 128 |
| P6 | 1649-2582 | TestIF（930 行） | 295 |
| P7 | 2583-2822 | TestMode、BinSelect、USER、RunInfo | 118 |
| P8 | 2823-2975 | ArmSpeed、SHSpeed、MGSpeed、MRSpeed | 100 |
| P9 | 2976-3330 | Ld_Uld、AOI、ATC、ATK、AOA、Teach、RT/FT 常數、TAlarm1 | 116 |

- 每個 TU 用到幾個區：0 區 173、1 區 136、2 區 91、3 區 59、4 區以上 152。**cprod.h 本身完全沒用到 Config.h 和 CosFunction.h**，:6-7 只是替下游轉手。

**ODR 與巨集陷阱**〔實測〕：
- 同名不同定義：`TMyKitSuck`（aHotPlateSubstrate.h:371 vs mykitsuck.h:274）、`uPlateInfo`（aHotPlateSubstrate.h:697 vs Public/HTEditList.h:332）。搬宣告時可能讓兩個定義同時出現在一個 TU。
- 熱門標頭全部 0 個 #undef。
- cmydef.h 的巨集只有 `PCI132`（:90）被 `#ifdef` 測到（cinitial.cpp:758）：拆檔時 cinitial.cpp 沒拉到它，**分支會默默消失，編譯不會報錯**。
- 其他：`iSnSocketCnt`（:1015，小寫像變數）、`MAX_DEVICES`（mymotor.h:81）、`_MAX_COL_ITEM`（mytray.h:20）。vendor 標頭 AdvMotErr.h:17 與 mn200.h:641 都定義 `SUCCESS`（值相同）。
- MachineType.h 有 292 個巨集被 1,348 處 `#if` 測到（263 個檔）：功能開關中樞，要動要等 MW-1（EastSun）。

## A4. 傳遞成本 g++ -E〔實測，旗標照 compile_commands，g++ 6.3〕

| TU | 原始碼行數 | 前處理後行數 / 大小 | 標頭數（專案） | -fsyntax-only |
|---|---|---|---|---|
| acarry.cpp | 9,089 | 98,660 / 2.98 MB | 271（74） | 約 6.1 s |
| ainarm2.cpp | 8,052 | 96,817 / 2.88 MB | 283（87） | 約 9.0 s |
| csystem.cpp | 34,049 | 123,845 / 3.78 MB | 335（138） | 約 19 s |
| Motor/mymotor.cpp | 6,744 | 90,510 / 2.72 MB | 239（45） | 約 5.4 s |
| tools/wb_serve.cpp | 9,308 | 116,011 / 3.63 MB | 332（121） | 約 9.1 s |
| mysensor.cpp | 347 | 79,612 | 218（24） | 約 2.4 s |
| Motor/HTMotor.cpp | 127 | 66,401 | 206（12） | 約 2.5 s |

單獨 include 一個標頭的行數：windows.h 18,347；`<string>` 15,195；vcl_compat.h 42,673（WIN32_LEAN_AND_MEAN 時 34,172）；MachineType.h 43,882；cprod.h 52,877；cmydef.h 59,226；mymotor.h 60,093；fMain.h 60,020；FormsFacade.h 72,735。

## A5. 瘦身計畫（依「省下的重編量 ÷ 風險」排序；MachineType.h 一律不動）

基準：14 天約 61k 次 obj 重編〔估計：commit 數 × fan-out；只看比例〕。

| 步驟 | 做法 | 工作量 | 省下（14 天）〔估計〕 | 風險 |
|---|---|---|---|---|
| S0 | 規定：不在熱門標頭（MachineType／fMain／cmydef／mymotor／cprod）加 AI 說明註解，改寫在 .cpp 或 docs | 0.5 天 | 約 −12k（−19%） | 無 |
| S1 | 先把 cmydef.h:5809-6078 收進 guard；再拆成 cmydef_core.h（A＋B，PCI132 留這裡）、cmydef_io.h（C–F＋H 的 IO）、cmydef_rt.h（G＋H 其餘）；cmydef.h 保留為總標頭只 include 三個子檔 | 1–1.5 天 | 0（純結構） | 低：宣告搬家，用 objcompare 驗 |
| S2 | 遷移 includer：約 287 個不用 IO 的改 include core＋rt；總標頭最後不再含 io | 2–4 天（可分批） | 約 −5k | 中：會和平行分支在 include 區塊衝突 |
| S3 | mytray.h 拿掉 cmydef.h；mymotor.h 改前置宣告 HTMotor 與 ARM_CONDITION（補 tag），直接 include MachineType.h、vcl_compat.h、myTimer.h | 1 天 | 約 −0.3k（主要是 B 部分的前置） | 低：漏了會編譯失敗，不會默默出錯 |
| S4 | FormsFacade.h 不再 include fMain.h；需要的 105 個 TU 自己 include；16 個 include 了 fMain.h 卻沒用 `fMain->` 的拿掉 | 1 天 | 約 −1.8k（374→312） | 低 |
| S5 | TRAY_TYPE_PARA（cprod.h:1257-1299）搬到小標頭，cmydef.h 不再 include cprod.h；43 個 TU 補 include；cmydef.h→cpublic.h 改前置宣告 | 1 天 | 約 −0.7k（854→714） | 低 |
| S6 | cprod.h 拿掉 Config.h 和 CosFunction.h；135 和 115 個 TU 補 include | 1–1.5 天 | 約 −0.8k（Config 870→534、Cos 857→398） | 低 |
| 工具 | include 圖與 fan-out 報表腳本；每個新子標頭的獨立編譯測試（比照 tests/w0tail_headers_compile.cpp） | 0.5–1 天 | — | — |

- 合計約 8–11 人天，省約 −20k（約 −33%）〔估計〕。
- **fMain.h**：125 個 includer 中 92 個只用 1–3 個 `fMain->` 成員（cbSetupFileName 32、BackupSetupFile 23）。做窄介面要改 golden 翻譯過來的呼叫點，跟「照 golden 逐字翻譯」衝突，**這次不做**。
- **給 MW-1 裁決用的資料（不提案動它）**：MachineType.h 14 天 34 個 hunk 中 25 個落在 :1600-1913 的 WB_／W906_ 開關區，這 13 個巨集只被 31 個檔引用。若 EastSun 同意把這區獨立出去，這類修改的重編量從約 971 obj 降到約 31 TU。

**驗證沒改到行為**：
1. 同一套工具鏈建 before／after 兩個目錄，sim 與 ship（W906_NO_SOFT_SIMULTE）兩組態。
2. 每個 .obj 先 `objcopy --strip-debug`（標頭行號變了 debug 資訊一定不同），再比 `objdump -d --no-show-raw-insn`、`nm -C`、`.data`／`.rdata`。
3. 已知雜訊：.rdata 字串位移（St02-E 1004 比 fMain.cpp.obj 見過）；`__DATE__`／`__TIME__`（cObserver.cpp、WebBridgeTags.cpp、TempCtrl/TriTemp.cpp）。
4. 兩組態 ctest 都要過；再對巨集做 census，確保 PCI132 在 cinitial 這個 TU 一定有定義。
5. 樹裡**沒有現成的 objcompare 工具**，要另外寫一支。

## B1. 現有的靜態庫〔實測〕

| 函式庫 | 原始檔 | 宣告的連結 | nm 實際查到的對外未解析符號 |
|---|---|---|---|
| ht9045_motor（CMakeLists.txt:1402-1477，另加 :3561） | HTMotor、mySimMotor、mymotor、mytray、Hontech_M4、mySMCmotor、EtherCAT/MyEtherCAT、EtherCAT/MyNUEC1、myMN200motor、mySYNTEKmotor、myEthercatmotor、myGALILmotor、3 個 vendor_offline、EcatMotorRoute；約 24.9k 行 | PUBLIC vclcompat、ht9045_globals（:1495） | 335 個：globals 212、**sm 32、io 11、core 10、db 5**、vclcompat 6、CRT/vendor 59 |
| ht9045_io（:1542-1560） | IOBackend、MyLaneIo、myswitch、mysensor、mycylin、myio；約 3.2k 行 | 同上（:1573） | 56 個：globals 19、**forms 3**（fSmartDiagnostic）、**sm 2**（IdleCheckSafeDoorByCylinder）、vclcompat 7、CRT 25 |

依賴往上指的 obj：mymotor→core/io/sm；myMN200、myGALIL→core/db/io/sm；myEthercatmotor→core/io/sm；MyEtherCAT→db/sm；mySMC→core/db；mySYNTEK、mytray→sm；MyLaneIo、myio→sm；mycylin→forms。
**反向循環**：globals 的 cprod.cpp 用到 `MOT`、`Cylinder`、`Sen`、`TTrayMotor::SetTraySingleData`、`TMyTray::FullIC`、`TMySensor::IsOn`。現在連得起來是因為 wb_serve（:3444）和測試都用 `$<LINK_GROUP:RESCAN,…>` 重掃。

## B2. 跨邊界的全域狀態〔實測，去註解〕

- **往外**（motor .cpp／io .cpp 引用 app 層）：`MOT[` 459/0、`Prod.` 67/0、`IniConfig.` 45/0、`fMain->` 29/0、`ArmSpeed[` 22/0、`TestIF.` 9/0、`CosFunction.` 9/0、`HSys` 6/0、`Cylinder[` 42/1、`SystemStart` 5/4、`MachineTypeChoice` 10/11、`*_CARD_TYPE` 6/19、`Show*Message` 147/17、`W906_*` 122/8。用到的 cmydef.h extern：motor 184 個、io 13 個。
- **往內**（motor/io 以外引用）：`MOT[` **9,898 次／253 檔**（測試 73 檔）、`.Motor->`（直接碰 HAL）477／59、`Sen[` 2,952／141、`Cylinder[` 2,224／79、`SW[` 1,950／138、`MyLaneIO` 176／23、TMyMotor／TTrayMotor 型別 90／22、HTMotor 72／29。
- nm 實際用到的介面：motor 161 個符號（TMyMotor 55、自由函式／全域 56、TTrayMotor 22、TMyTray 13、HTMotor 7）；io 51 個（TMyCylinder 18、自由函式／全域 16、TLaneIO 10）。

## B3. 要做成 DLL 需要什麼

- **公開介面**：HTMotor 64 個 public 方法（54 virtual）；TMyMotor 106 方法＋約 40 public 欄位；TTrayMotor 28＋9；TMyTray 24 方法＋33 欄位；TMyCylinder 31＋約 64；TMySensor 5＋11；TMySwitch 5＋12；TLaneIO 20。
- **全域要變存取函式**：MOT[300]、Cylinder[]、Sen[]、SW[]、MyLaneIO 等，上萬處直接存取欄位要改寫；否則得匯出「有建構子的物件陣列」，靠 MinGW auto-import，還有 exe／DLL 靜態初始化順序問題。
- **CRT 與例外**：CMakeLists.txt:150 `-static-libgcc -static-libstdc++`，每個 DLL 自帶一份 libstdc++；i686 DW2 例外不能跨 DLL 拋（motor/io 有 5 處 try/throw/catch）；介面上有 std::string、std::map、AnsiString ⇒ exe 與 DLL 必須同一版編譯器，但現在 gate 6.3、機台 WinLibs 16.2 兩套。
- **測試執行檔**：373 個 add_executable，tests/CMakeLists.txt 277 行提到 ht9045_motor、267 行提到 ht9045_io；改 DLL 後每支測試都要找得到 DLL（正是當初 `-static` 要避免的，:140-148），F-Secure 也會掃新 DLL。
- **編譯時間**：motor＋io 占已編譯原始碼 47.7k／1,676k 行（2.8%），archive 0.95＋0.16 MB ⇒ 影響接近 0〔估計〕。

## B4. 離「標頭隔離」還差多遠〔實測〕

已經乾淨（只 include vcl_compat、windows、自己、vendor、myTimer.h）：HTMotor.h、mySimMotor.h、mySMCmotor.h、mySYNTEKmotor.h、myEthercatmotor.h、myGALILmotor.h、Hontech_M4.h、EcatMotorRoute.h、GaliRoute.h、MyEtherCAT.h、MyNUEC1.h、IOBackend.h、MyLaneIo.h、myswitch.h、mycylin.h、myio.h。
**還不乾淨的只有 4 條邊**：mymotor.h→cprod.h（:70）、mytray.h→cmydef.h（:23）、myMN200motor.h→database.h（:98）、mysensor.h→MachineType.h（:22，為了 MAX_SENSOR_ITEM／MAX_TTL_BIT）。
.cpp 層：13 個非 vendor 的 motor .cpp 有 10 個 include cmydef.h；6 個 io .cpp 有 5 個。往上 include app 層的例子：myGALILmotor.cpp（fMain.h、fOffSet.h、csystem.h、cinitial.h、asendic.h、cprod.h、Config.h、CosFunction.h、atester_shims.h、mycylin.h、mysensor.h）；mymotor.cpp（cContact.h、aHotPlateSubstrate.h、fMotorTest.h、canary_support.h）；myMN200motor.cpp（fNote.h、MyBinDisp.h、acarry_shims.h、atester_shims.h）；mycylin.cpp（fSmartDiagnostic.h）；MyLaneIo.cpp（Vc8Route.h）；mySMCmotor.cpp（cMyDB.h）。

## B5. 建議（分階段；DLL 不做）

1. **B-1，1–2 天，低風險**：修掉上面 4 條標頭邊（和 A5 的 S3 一起做）。加一個 ctest 檢查分層：Motor/ 與 IO 的公開標頭只能 include 允許清單裡的標頭。
2. **B-2，1 天**：加一個 nm 棘輪檢查：motor→sm/io/core/db 與 io→forms/sm 的未解析符號數（目前 32/11/10/5 和 3/2）列為上限，只能減不能增。
3. **B-3，1–3 週，中風險**：往上的呼叫改成掛勾（樹裡已有做法：`W906_Ht9050OrgHomeHook`，mymotor.h:96），motor 內放同名轉發 shim，不改 golden 翻譯過來的呼叫點。要處理：UI 訊息（ShowErrorMessage／ShowMotorErrorMessage、fMotorTest、fSmartDiagnostic）、狀態機全域（InArmSuck、IndexZCanMove…）、db 紀錄（MyDBIProcess、RecordProcess、HSys）、IdleCheckSafeDoorByCylinder。之後把 motor 拆成 `motor_hal`（HTMotor＋驅動＋vendor_offline＋MyNUEC1）和 `motor_app`（mymotor／TTrayMotor／手臂例程）。**風險**：某支 exe 沒裝掛勾會默默什麼都不做，預設行為要大聲失敗。
4. **編譯時間上得到什麼**〔估計〕：很少。真正的好處是架構——分層可檢查、HAL 能單獨測、不再依賴 RESCAN，也符合 Steven「馬達是 class 概念、先看技術」的原則。要縮短編譯時間，依序做 A5 的 S0→S1/S2→S4，加上 `speedup-ideas.md` 已實測的 Ninja、thin archive、只建目標。

## C. 下一步（給 Steven 決定後派工）

| 項目 | 誰 | 前置 |
|---|---|---|
| S0 註解規則 | ST01-M 寫進 ht9045-v906 agent 與 cpp_build skill；筆電同意後進 CLAUDE.md | 無 |
| S1 cmydef.h 收 guard＋三拆（總標頭不變） | St01（ST01-E） | objcompare 工具（0.5–1 天） |
| B-1 四條標頭邊＋分層 ctest | St01（ST01-E） | 與 S3 同做 |
| S2 includer 遷移 | 分批、各 owner 自己的檔（St01／St02／Frank／Ifor），避開平行分支 | S1 合進 main |
| S4／S5／S6 | St01 | S1 |
| MW-1 MachineType.h 開關區獨立 | EastSun 裁決後筆電做 | 本檔 A5 的資料 |
| B-3 掛勾化、motor_hal／motor_app | 翻譯穩定後再排 | B-1、B-2 |

<!-- preserved-content:end -->
