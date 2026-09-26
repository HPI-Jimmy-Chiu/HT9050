# W3 進度表 —— 硬體物件層 9 支檔＋cinitial 初始化＋五個 InitMotor

> 週末計畫 `docs/WEEKEND_PLAN_20260925.md` W3（含 c507adea 追加範圍）。完成條件：每支檔一張表，
> 被引用的函式逐一標「已開＋測試名」或「仍閘住＋理由」；沒有一個被引用的函式停在「沒寫理由的 `#if 0`」或「回 0／false 的樁」。
> 「被引用」用 `tools/w3_caller_census.py`（nm）量，結果在 `docs/W3_CALLER_CENSUS_20260924.md`。
> 驗證照使用者 20260924 晚的規則：輕量驗證（增量建置＋相關 ctest）＋出貨組態閘門；START／PAUSE 留到清單最後。

| 順序 | 檔／函式群 | 狀態 |
|---|---|---|
| 1 | `MyLaneIo` | ✅ 本輪（見下） |
| 2 | `myio` | ✅ 本輪（見下）：沒有被引用的函式，3 個閘維持並寫明理由 |
| 3 | `mysensor` | ✅ 本輪（見下）：4 個被引用函式都已開；舊式讀取改接 myio 真函式；面板閘 → 4b（原寫 3b，0926 統一） |
| 4 | `myswitch` | ✅ 本輪（見下）：5 個被引用函式都已開；舊式寫入改接 myio 真函式；面板閘 → 4b（原寫 3b，0926 統一） |
| 4b | **`uPadInterface`（新增子項）**：RS232 通訊式實體操作面板 | ⏳ 相依不存在，要翻（golden 962 行＋讀取執行緒）。mysensor／myswitch 的面板閘等它 |
| 5 | `LoadIoData`＋`InitialSwitchName`／`InitialSwitch`＋`InitialSensorName`／`InitialSensor` | ✅ 本輪（見下）：if 那半逐列逐欄 0 不符；else（BDE）閘改寫為使用者裁決；`SetIOTableByNUEC1` 量過是 no-op |
| 6 | `mycylin` | ✅ 本輪（見下）：18 個被引用函式都已開；舊式路徑改接 myio；計數閘 → 6b、警報 → W2 |
| 6b | **vclcompat `TStringGrid` 越界語意（新增子項）** | ✅ 本輪（見下 §6b）：改成 BCB6 grids.pas 語意（稀疏存放、越界讀回 ""、越界寫存起來、縮小不刪資料、負索引丟例外）；mycylin 氣缸計數 4 個閘解開 |
| 7 | `InitialCylinderName`／`InitCylinder` | ✅ 本輪（見下）：if 那半三組位址逐列逐欄 0 不符；else 閘改寫為使用者裁決 |
| 8 | `mykitsuck` | ✅ 本輪（見下；逐函式表在 §15，0926 補）：A4-6 兩套 TMySucker／TMyKitSuck 合一為 golden 佈局；NB2 R1 §A 抓到的活 ODR（`cOffSet.cpp:63`）隨之消失 |
| 9 | `InitialSuckerName`／`InitSucker` | ✅ 本輪（見下）：if 那半 49 個吸嘴 × 三組 7 欄 0 不符＋手算值；else（BDE）閘改寫為使用者裁決；ChangeSite 還原端（N1-G4）→ 第 12 項 |
| 9b／9c | **GATE n4-1**（`SetMyKitSuckItemAmount`）＋ChangeSite **47 個 N1-G4 還原閘**＋`SetInOutArmParameter` 的 **COPYBACKUP** | ✅ 本輪（見下 §9b）：NB2 R3 抓到所有吸嘴格線都停在 1×1；照 golden 解開，兩組態 0 個新失敗 |
| 10 | `LoadMotData`＋`InitialMotorName`／`InitialMotorParameter`＋五個 `InitMotor` | ✅ 本輪（見下）：if 那半本來就活著，補第三級測試（48 軸 0 不符）；BDE 兩臂閘改寫為使用者裁決；五個 InitMotor 往下呼叫量過；⚠ HT9050 要 `INDEX_MOTION_CARD=1` |
| 11 | `mytray` → `myTimer` → `MyTempPanel` | ✅ 本輪（見下；myTimer／MyTempPanel 逐函式表在 §15，0926 補）：mytray 整支照 golden 重翻（原本 11 個樁＋6 處偏離）；myTimer 0 差異；MyTempPanel 被引用 6 個都活、其餘閘是版面／滑鼠事件歸網頁 |
| 12 | `cinitial.cpp` 其餘被引用的函式 | ✅ 本輪（見下 §12）：21 個被引用函式逐一列；A4-6 後理由失效的 8 個閘解開；其餘閘都有真理由（UI 歸網頁／BDE 裁決／真的缺相依） |
| 13 | `database.cpp` 的 `ShowMyMessage` 標題（稽核第 ⑸ 項） | ✅ 20260926 03:4x：IO 表／馬達表載入與逐欄檢查的 **38 處**標題照 golden 原文改回中文（例：`"IO file data error!"` → `"注意！IO檔案的資料錯誤!"`、`"ACC is NULL!"` → `"注意！馬達ACC為空!"`）。移植樹與 golden 的 38 個呼叫依序一對一，前 12 行組訊息的其他字串逐一比過都相同（只有標題不同）；同行換字＋行尾 `AI(W906-DBMSG)` 註解，行數不變（其他文件引用的 `database.cpp` 行號不動）；測試與網頁都沒有比對這些英文字 |
| 14 | 稽核的過期／不一致註解（第 ⑵ 項＋覆核補的幾則） | ✅ 20260926 04:0x，全部同行改寫、行數不變：`cinitial.cpp:656`（InitSucker）與 `:4136`（GATE 2）的 else 閘改成與另外 4 個同一句「使用者裁決…else 可以不用」並指向本計畫 W3；GATE 2 橫幅 `:4113` 註明理由已換成使用者裁決；`:2629` 的 `EtherCAT/MyNUEC1.cpp` 更正為 golden `CCLink/MyCCLinkSensor.cpp:2603`；`mycylin.cpp` 檔頭兩則（舊式點「no-op stubs」→ 已改接 myio、SmartDiagnostic「gated」→ 6b 已解開）；`mysensor`／`myswitch` 的「子項 3b」統一成 4b（各 4 處）；`myio.cpp:162/168/174` 的「myio 自由函式 0 個被引用」更正（nm 量 build_ship：`IOInputBit／IOBitOn／IOBitOff／IOOutBitStatus` 被 mycylin／mykitsuck／mysensor／myswitch 引用；閘照舊，x64 上那些舊式點讀回 0）。驗證：5 支檔改前改後用出貨組態同一組旗標前處理，**逐行相同**（cinitial 79,285 行…） |

---

## 1. `MyLaneIo`（20260924 夜）

**被其他檔引用的 8 個函式**（nm 盤點）

| 函式 | 引用者 | 狀態 | 測試 |
|---|---|---|---|
| `IOBitOn` | mycylin、myswitch | ✅ 已開。**A4-7 修正**：依點位分派三路後端（見下） | `LaneIORoute`、`LaneIOSim` |
| `IOBitOff` | mycylin、myswitch | ✅ 同上 | `LaneIORoute`、`LaneIOSim` |
| `IOInputBit` | mycylin、mysensor | ✅ 同上；PLC 分支照 golden（只在 `ePLCbase` 觸發） | `LaneIORoute`、`LaneIOSim` |
| `IOOutBitStatus` | mycylin、myswitch | ✅ 已開（golden 本來就只讀 `OutPortData` 快取，不碰卡） | `LaneIOSim` |
| `GetIOValue`／`GetIOValueThread`／`SetIOValueThread` | MyVacuumPanel | ✅ 已開：`#if HAVE_PCI1203` 內照 golden 呼叫 `Acm_Daq*Ex(uiDevhand,…)`；沒有 SDK 的建置回 golden 的失敗值 999.0／false | ⚠ 無 ctest：這三支只對 1203 類比模組（ECAT-VC8）有意義，要有卡的機台才量得到 |
| `SetUseIP` | cinitial | ✅ golden 本體整段是註解（`/* … */`），照翻為空函式；`:191` 的 `#if 0` 只是存放 golden 原文 | —— |

**A4-7：依點位分派三路後端**（本輪的主要修正）

| | 修正前 | 修正後 |
|---|---|---|
| golden 的做法 | 每個 IO 方法直接呼叫廠商函式：`iISABase==ePCI1203` → `Acm_Daq*Ex(uiDevhand,…)`；`IO_CARD_TYPE` 是 MN200（1／2）→ `mn_*`；其他 → `_mnet_*`（`IOInputByte` 那支的 MN200 條件是 `IO_CARD_TYPE==1`） | —— |
| 移植樹 | 三路都送進**同一個** `pIO`，預設 `TSimIOBackend`，而 `SetBackend` 在測試以外 0 個呼叫者 ⇒ 出貨組態的 `SW[]`／`Sen[]` 讀寫的是模擬值 | 三個後端 `pIO1203`／`pIOMN200`／`pIOMnet`，每個呼叫點照 golden 條件選（同一行改寫，不位移行號） |
| 何時換成真後端 | 從來沒有 | `InitHontechHardware()` 開頭呼叫 `MyLaneIO.SelectVendorBackends()`（golden 開卡的地方；放在 `InitSucker`／`InitialSwitch` 之前，因為它們可能已經會寫輸出） |
| 模擬建置 | —— | 不換，三路都是模擬（使用者 20260918：模擬 vs 真機只看 `SOFT_SIMULTE`） |
| 測試 | —— | 建構後預設仍是模擬（= 硬體尚未初始化）；測試都不呼叫 `InitHontechHardware`，所以既有測試依賴的「預設模擬」不變 |

⚠ **今天在 HT9050 上 1203 點仍不會真的驅動**：`TPci1203Backend` 用 golden 的全域 `uiDevhand`，而它只有 `OpenEtherCatMastCard` 開卡後才有值；
開卡條件 `INSTALL_ETHETCAT()`（Sam 2023，Shuttle 感測／真空模組用）不涵蓋 HT9050 ⇒ 讀寫回錯誤碼、走 golden 的 `MNetLog` 錯誤分支。
開卡與 handle 照 EastSun 的做法補 = 週末計畫 **W4**。參數本身兩邊一致：EastSun 送 `Acm_DaqDoSetBitEx(dev, ring, station, stationChan*8+bit)`，
golden 送 `(uiDevhand, Ring, IP, Port)`；HT9050 IO 表的 IP＝站號、Port＝站內通道（SnMotorPower 30＝3×8＋6）。

**樁與閘**

| 項目 | 狀態 |
|---|---|
| `MNetLog`（原本是檔內 `static` 空函式） | ✅ 改接真的 `bool MNetLog()`（`Motor/myMN200motor.cpp`，golden :2146）。⚠ 它的本體因 `fMain->slMNetLog` 未移植而閘住 ⇒ 今天仍不落檔。golden `FormCreate` 那一批日誌物件（`slEventLog`、`sl2DMappingLog`、`slMNetLog`、`slUploadFile`）**整批沒有在移植樹建立**（`slEventLog` 只有宣告，`git grep "slEventLog\s*=\s*new"` 0 處）⇒ 另列一項，不在這裡單補一個 |
| `ShowMyMessage`（原本是檔內 `static` 空函式） | ✅ 改接真的（`canary_support.cpp:146`：記錄、印出、轉給網頁顯示，不阻塞 tick）。只在非 `SOFT_SIMULTE` 的 IO 位址錯誤分支呼叫，與 golden 相同 |
| `fiosetview_fShow()` | 刻意保留：golden 的「IO 表單開著嗎」在 web 架構下歸瀏覽器（pt-wave-loop 陷阱 #6）。回 false = 走 golden「沒開 → 跳位址錯誤訊息」那一支（保守，寧可多報） |
| `bPLCIO`／`bPLCInData` 定義在本檔 | 既有的已揭露偏離（AI(W906-GA1-B2-integrate)），連結拓撲理由，行為相同 |

**驗證**：`LaneIORoute`（新，35 項：三路探針逐方法 × 點位 × `IO_CARD_TYPE`、1203 回傳值換算、`SelectVendorBackends` 兩組態）
模擬／出貨兩組態都 35/35；IO 相關既有測試（LaneIOSim、SimIO、Pci1203Seam、W6_4*、W7_L1_*、W7_HotplateLoaderVibrate）出貨組態全過。
⚠ 模擬組態的 SimIO 有 5 項失敗是既有的：那 5 項在驗 `CheckPortRangeErr` 的錯誤碼，而它在 `SOFT_SIMULTE` 下本來就整段跳過（本輪沒動它）。

⚠ 附帶踩到的坑：新測試原名 `test_lane_io_dis`**`patch`**，Windows 的安裝程式偵測把檔名含 patch 的 exe 當安裝程式要求提權 ⇒ bash「Permission denied」、ctest「Not Run」。改名 `test_lane_io_route`。

---

## 2. `myio`（20260924 夜）

**被其他檔引用的函式：0 個**（nm 盤點：16 個定義、0 個被引用；`myio.cpp.obj` 從未被抽出連結）。⚠ **20260926 更正：這句是 A4-6／W3-6 之前量的，已過期** —— 現在 `IOInputBit／IOBitOn／IOBitOff／IOOutBitStatus`（兩參數）被 mycylin／mykitsuck／mysensor／myswitch 的舊式路徑引用（nm，build_ship），見第 14 列。golden 的舊式 IO（ISA／PCI 原始 port 卡）介面。

| 閘 | golden | 狀態 |
|---|---|---|
| GATE (1) `outportb` | :137／:170／:199 | 維持閘住：x64 沒有原始 port I/O（BCB6 的 `<conio.h>` 內建）；HT9050 的 IO 走 1203（MyLaneIo 的 `pIO1203`） |
| GATE (2) `inportb` | :280／:295 | 同上 |
| GATE (3) `EnableNTPort` | :79／:100 | 同上（它只是讓 (1)(2) 能碰硬體的權限呼叫） |
| GATE (4) 兩參數 `IdleCheckSafeDoorByCylinder` | :112／:146／:179 | 已開（BU-D7 20260916），但因為沒人呼叫 myio 而不會執行（⚠ 20260926 更正：「沒人呼叫 myio」已過期 —— nm 量 build_ship：`myio.cpp.obj` 引用 `IdleCheckSafeDoorByCylinder(int,int)`（定義在 `csystem.cpp.obj`），wb_serve.exe 有連進來；也就是舊式 eISABase／ePCI1735U／ePLCbase 點的 IOBitOn／IOBitOff 現在會走到這個安全門檢查） |

理由屬於 W3 規則的「x64 上不存在的硬體路徑 ⇒ 照翻保留、閘住，註明 HT9050 走 1203」。閘註解已由 `TODO(W4-IO-part2)` 改寫成理由（同一行，不位移行號）。

---

## 3. `mysensor`（20260924 夜）

| 函式 | 引用者 | 狀態 | 測試 |
|---|---|---|---|
| `IsOff` | 55 支檔 | ✅ 已開：`eMotionNet`／`ePCI1203`／`ePLCbase` → `MyLaneIO.IOInputBit`（A4-7 後依點位分派）；`eISABase`／`ePCI1735U` → **myio 的真 `IOInputBit`**（本輪由樁改接） | `LaneIOSim`、W7_L1_* |
| `IsOn` | 46 支檔 | ✅ 同上 | 同上 |
| `Status` | cSensorScan | ✅ 同上 | 同上 |
| `CopySensor` | cinitial | ✅ 已開（純欄位複製） | —— |

| 閘／樁 | 狀態 |
|---|---|
| 檔內樁 `IOInputBit(Port,Bit)`（回 false） | ✅ 改接 myio 的真函式（含 golden 的 PLC 分支與 `TTL_CARD_TYPE` 保護）。原始 port 讀取在 x64 仍是 myio GATE (2) 讀 0。nm 查過 myio.obj 的全域與其他 obj 無重複定義；wb_serve 現在確實連進 `IOInputBit(int,int)` 等四支 |
| 面板閘 ×3（`iControlPanelMode==1 && fPadInterface->IsPadKey(Name)`） | 維持閘住，**理由更正**：原註「VCL GOD-STACK」不對。`fPadInterface` 是 RS232 通訊式實體操作面板（golden `uPadInterface.cpp` 962 行、`TPadRS232Thread`；Start／Pause／Reset／Home／AlarmReset／SafeLock… 共 17 鍵），按鍵狀態來自硬體、歸 C++ ⇒ 相依不存在 ⇒ **子項 4b** |
| `CheckSenPortRangeErr` | golden 本身就是整段註解（mysensor.cpp:180-231），照翻為註解 |

⚠ **機台端要知道**：筆電 `Gerneral.ini` 是 `ControlPanelMode=1`（`database.cpp:1624` 讀入），而 `cinitial.cpp:1678`／`:2897` 在這個模式會停用 IO 版的面板鍵 ⇒
在 ControlPanelMode=1 的機台上，移植樹的實體面板鍵**兩邊都讀不到**（通訊面板沒翻、IO 版被停用）。HT9050 的值要看機台的 `Gerneral.ini`。

## 4. `myswitch`（20260924 夜）

| 函式 | 引用者 | 狀態 | 測試 |
|---|---|---|---|
| `On` | 47 支檔 | ✅ 已開：`eMotionNet`／`ePCI1203` → `MyLaneIO.IOBitOn/Off`（A4-7）；`eISABase`／`ePCI1735U`／`ePLCbase` → **myio 的真 `IOBitOn/Off`**（本輪由樁改接） | `LaneIOSim`、W7_L1_*、W7_HotplateLoaderVibrate |
| `Off` | 24 支檔 | ✅ 同上 | 同上 |
| `Status` | 11 支檔 | ✅ 同上（`IOOutBitStatus`：讀輸出快取，不碰硬體） | 同上 |
| `OnOff` | cinitial、ckernel、csystem | ✅ 已開（呼叫 On／Off） | —— |
| `CopySwitch` | cinitial | ✅ 已開（純欄位複製） | —— |

| 閘／樁 | 狀態 |
|---|---|
| 檔內樁 `IOBitOn`／`IOBitOff`／`IOOutBitStatus`（兩參數，回 void／false） | ✅ 改接 myio 的真函式。⚠ 連帶啟用 golden 的兩參數 `IdleCheckSafeDoorByCylinder` 互鎖（csystem.cpp，BU-D7 已開）：機台閒置時，若輸出點屬於「要檢查安全門」的氣缸，門開著就擋下輸出。這是 golden 的安全行為；HT9050 的點全是 1203，不走這條舊式路徑 |
| 面板閘 ×3（`fPadInterface->IsPadButton／SendSwitchStatus／ProcessScanKey`） | 維持閘住，理由同 mysensor ⇒ **子項 4b** |

## 5. `LoadIoData`＋`InitialSwitchName`／`InitialSwitch`＋`InitialSensorName`／`InitialSensor`（20260924 夜）

| 函式 | 狀態 | 證據 |
|---|---|---|
| `LoadIoData` | ✅ 已開（67 行對 67 行、0 閘） | `MachineIoTable_HT9050`：1,062 列讀入、欄位索引對 |
| `InitialSwitchName`／`InitialSensorName` | ✅ 無分支、0 閘 | —— |
| `InitialSwitch`（if＝NewIO_MN200 那半） | ✅ 已開。**第三級**：`machines/HT9050/IO_Table.csv` 餵進去，132 個查得到的 Switch 逐列比對 7 個欄位（ISABase／Ring／IP／Port／Bit／Type／Using）**0 不符** | `MachineIoTable_HT9050`（本輪擴充） |
| `InitialSensor`（if 那半） | ✅ 已開。289 個 Sensor 逐列逐欄 **0 不符**（ePLCbase 的 Ring／IP 固定 0 照 golden） | 同上 |
| else 那半（BDE `TTable`／`DataModule1->SwitchTable／SensorTable`） | 閘留著，註解改寫成「使用者 20260924：else 不用」並指向 W3（`cinitial.cpp:1568`、`:2722`） | —— |
| `SetIOTableByNUEC1()`（`InitialSensor` 分支外，golden :2762 無條件呼叫） | 維持閘住，**理由更正**：函式在 golden `CCLink/MyCCLinkSensor.cpp:2603`（舊註解寫 `MyNUEC1.cpp` 是錯的），本體整段包在 `if(SHUTTLE_SENSOR_TYPE==6||7)` ⇒ **非 NU-EC1 機台上是 no-op**（c507adea 要求量的就是這個）。HT9050 不用它 | golden 原文 |

## 6. `mycylin`（20260924 夜）

**被其他檔引用的 18 個函式**：`Off`(31)、`On`(26)、`Pop`(15)、`Push`(15)、`OffStatus`(13)、`OnSensor`(10)、`OnStatus`(7)、`OffSensor`(7)、
`GetOutBit`(5)，以及 cStartCondition 用的 `AddOnTime／AddOffTime／GetOnTime／GetOffTime／GetOnTimeAvg／GetOffTimeAvg／GetOnTimeAlarm／GetOffTimeAlarm`、acatchtray 用的 `Reset`。
全部已開：`eMotionNet`／`ePCI1203` → `MyLaneIO`（A4-7 依點位分派）；`eISABase`／`ePCI1735U`／`ePLCbase` → **myio 的真函式**（本輪由樁改接）。
測試：`W7_L1_*`、`W7_HotplateLoaderVibrate`、`W6_4*`、`GA1_cprod`（出貨組態全過）。

| 閘／樁 | 狀態 |
|---|---|
| 檔內樁 `IOBitOn`／`IOBitOff`／`IOOutBitStatus`／`IOInputBit`（兩參數） | ✅ 改接 myio 的真函式（同 myswitch／mysensor） |
| `fSmartDiagnostic->GetCyliderOnCount/OffCount` ×4 | **暫時維持閘住，理由更正**：相依已存在（`forms/fSmartDiagnostic.cpp` ACTIVE、實例靜態初始化時就 new），但 golden 的計數迴圈讀 `Cells[][i+1]` 到 i=294、不管 RowCount；`vclcompat::TStringGrid` 越界讀會丟 `std::out_of_range`，而 BCB6 的 grids.pas 是回空字串（本機 BCB6 原始碼查證過）⇒ 開了會在氣缸動作時丟 golden 不會丟的例外，wb_serve 會掛。⇒ **子項 6b** 先修 vclcompat |
| `SetAlarm`／`ClearAlarm` | 維持：golden 是 `Alarm->Set/Clear`，`HAlarm`（golden main.cpp:202 的全域，PT §8 的 NULL 全域之一）移植樹仍沒有 ⇒ 相依不存在，列 **W2（警報子系統）**。⚠ 影響：氣缸預警（Eastsun 20260521 整合）在運轉中升級成警報的那一步目前不會發生 |
| `UpdateSimulateCompomentPosition`／`SetSimulateCompoment`（TControl 動畫） | 維持閘住：模擬模式下移動畫面元件的純動畫，歸瀏覽器（陷阱 #6） |
| `InitialCylinderName` 的替身 | 已退役（PT-W7a），真本體在 cinitial.cpp |

## 7. `InitialCylinderName`／`InitCylinder`（20260924 夜）

| 函式 | 狀態 | 證據 |
|---|---|---|
| `InitialCylinderName` | ✅ 無分支、0 閘（mycylin.cpp 的替身已退役） | `InitCylinder` 內部先呼叫它 |
| `InitCylinder`（if＝NewIO_MN200 那半） | ✅ 已開。**第三級**：`machines/HT9050/IO_Table.csv` 餵進去，258 個有名稱的氣缸，IO 表查得到的 **輸出 72 組、On 感測 54 組、Off 感測 54 組**，每組 7 欄（RingUse／Ring／IP／Port／Bit／Type／ISABase）逐列比對 **0 不符**；`OnSensorName`／`OffSensorName` 照 golden 是 `<名稱>_On`／`_Off` | 新 ctest `MachineCylinders_HT9050`（521 項） |
| else 那半（BDE，N3-G1） | 閘留著，註解改寫成「使用者 20260924：else 不用」（`cinitial.cpp:5065`） | —— |

Enable 系列不在這支測試比：golden 另有規則（位址全 0 或 Port／Bit 為 0 就關、尾段對 Empty／Color／Auto 的升降氣缸強制開），那是 golden 行為本身、不是「位址有沒有寫進去」。

## 8. `mykitsuck`（A4-6，20260924 夜：分析完，下一步動手）

**問題**：移植樹有兩份同名、佈局不同的 `TMySucker`／`TMyKitSuck`：

| | `aHotPlateSubstrate.h`（精簡版，186 支檔在用） | `mykitsuck.h`（完整版，= golden 佈局） |
|---|---|---|
| `TMySucker` 資料成員 | 15 | 71 |
| `TMySucker` 方法 | 18 | 35 |
| IO 接線欄位（On／Off／Sen 的 Ring／IP／Port／Bit／ISABase／Type／PortName） | **全部沒有** | 有 |
| `DoOnIO`／`DoOffIO`／`OffSuck`／`IsSuckFinish`／`IsDestroyFinish`… | **沒有** | 有 |
| 實作 | `aHotPlateSubstrate.cpp`，真空靠 `W906_SimVacuumOnBit` 等模擬欄位 | `mykitsuck.cpp`（golden 94 個符號全部忠實翻譯、啟用），**刻意沒註冊**（`CMakeLists.txt:2723`，避免 ODR） |

golden 只有一份：`mykitsuck.h`（`acarry`、`acatchtray`、`ainarm*`… 都 include 它）。
⇒ **後果**：真機上移植樹的真空吸嘴（入料臂、出料臂、測試頭、Carry Kit）不會真的開關真空；`InitSucker` 的 IO 接線整段被閘住（`cinitial.cpp:493-878`）也是因為精簡版沒有那些欄位。

**精簡版獨有、合一時要處理的**（`git grep` 量）：

| 成員 | 誰在用 | 處置 |
|---|---|---|
| `W906_SimVacuumOnBit`／`W906_OffDestroyRaw`／`W906_OffDestroyStamp`／`W906_ResetOffDestroyCount` | 只有 aHotPlateSubstrate 自己 | 隨精簡版退役 |
| `W906_GetOffDestroyCount` | aHotPlateSubstrate＋`tests/test_w7_l2_ckernel.cpp` | 測試改用完整版的等價狀態 |
| `IsPickFinish` | 沒有別人 | 退役 |
| `SetType1ToType2ByPickCol` | `ainarm9045_2x4_16.cpp` 6 處 | **golden 沒有這個定義**（只有呼叫點）⇒ 以「移植樹獨有、讓該檔編得起來」保留並註明，不改呼叫點 |
| `OnTask` | —— | golden `mykitsuck.h:17` 有（`TMySucker` private），完整版要對照補齊 |

**步驟**：(1) `aHotPlateSubstrate.h` 改 include `mykitsuck.h`、刪精簡版定義；(2) 註冊 `mykitsuck.cpp`、刪 `aHotPlateSubstrate.cpp` 的精簡版方法本體；
(3) 上表逐項處理；(4) 模擬／出貨兩組態全量建置（186 支檔重編）；(5) 解 `InitSucker` 的 if 那半（第 9 項），寫吸嘴第三級測試；(6) 出貨組態閘門。
⚠ `sizeof(TMyKitSuck)` 會從 14,792 變 48,488（x64），全域陣列與 memcpy 類的程式要一併檢查。

### 8 實作紀錄（20260924 20:3x 起）

| 步 | 做了什麼 | 行號策略 |
|---|---|---|
| 1 | `aHotPlateSubstrate.h`：`:97` 分隔線 → `#include "mykitsuck.h"`；`:105` → `#if 0`；`:647` → `#endif`（精簡鏡像 `TMySucker`／`TMyKitSuck` 退役） | 原地替換，不位移 |
| 1 | `aHotPlateSubstrate.cpp`：`:79`／`:929` 包住全域物件與 66 個方法本體；`:1261`／`:1262`／`:1264` 三個單行全域註解掉；`:1290`／`:1642` 包住 13 個方法與重複的 `CopyInitSuck` | 原地替換 |
| 2 | `mykitsuck.h`／`.cpp`：附加觀測接縫（per-nozzle OffDestroy 計數）與 `SetType1ToType2ByPickCol`；`OffDestroy()` 同行加計數呼叫；接縫全域宣告放檔尾 | 同行附加＋檔尾 |
| 3 | CMake：`mykitsuck.cpp` 註冊到 `ht9045_sm`（`:1888`，跟 `aHotPlateSubstrate.cpp` 同一行）；`:2723`「刻意不註冊」改寫為史料 | 同行 |
| 修 | `mykitsuck.h` 的全域 `using vclcompat::TList`／`cl*` 移到 `mykitsuck.cpp`（全域 using 跟 `HTEditList.h`／`aHotPlateSubstrate.h` 的 TList、`uHGemEquipment.h`／`acatchtray_shims.h` 的顏色撞名）；`clAqua` 改成 mykitsuck.cpp 內 static（跟 `cObserver.cpp:179` 撞） | 同行 |
| 修 | `cinitial.cpp:123-124` 的 `pSuck`／`pTempSuck` 檔內 static 替身退役（改用 golden 全域）；`:450` `new TList` → `new vclcompat::TList` | 同行 |
| 修 | `test_ga1_cprod`：補完整版 `TMySucker` ctor／dtor、`TMyKitSuck` dtor 的空替身 | —— |
| 修 | 連結期重複定義 3 個：`ptrOutSHT`（`acarry.cpp:99`）、`OutSht3Kit`（`acarry_shims.cpp:67`）、`SortArmPordRec`（`asortarm.cpp:563`）—— 都是當初 mykitsuck 沒連進來時各檔自補的，初值與 golden 相同（NULL／預設建構）⇒ 退役，改用 mykitsuck.cpp 的 golden 定義 | 同行註解 |
| 驗 | 出貨組態全量建置 0 錯誤。nm：wb_serve 裡 `TMySucker::DoOnIO`／`DoOffIO`／`OffSuck`／`IsSuckFinish` 都在（精簡版沒有），`TMyKitSuck` 75 個方法；反組譯 `TMySucker::OffDestroy()` 依序呼叫 `DoOffIO`（真的關破壞閥）與 `W906_OffDestroySeamHit`（計數） | —— |

**ctest 第一輪：5 支 SEGFAULT，全部是「測試校準在精簡鏡像上」**（gdb 帶 ctest 的圍堵環境變數逐支取 backtrace，不猜）：

| 測試 | backtrace | 原因 | 處置 |
|---|---|---|---|
| `AutoClean` | `TMyKitSuck::ClearAll → TMyProductionRecord::InitialRecord → Strings[]` | 測試把 `PordRec[0][0].asBuffer->CommaText` 設成 `"1,2,3"`（3 行）；golden `InitialRecord` 遇到行數 != `eDataTotal` 會 Delete 後再解參考（golden 自己的 bug，照翻保留，`Public/MyProductionRecord.cpp:326-345`）。精簡鏡像的 `ClearAll` 不碰 `PordRec`，所以以前沒事。（先懷疑 vclcompat 的 CommaText 會丟結尾空欄位，**量過是錯的**：1／2／5／30 個空欄位來回都保持行數） | 種子改成完整長度的紀錄（前三欄 1/2/3），比對意思不變，另加「行數仍是 eDataTotal」的斷言 |
| `W6_2_InArmCanary` | `TMyKitSuck::CopyToTray ← DoPlaceToHPSwapData` | golden `CopyToTray` 解參考 `Mot.Tray.PordRec[c][r]->asBuffer`；golden 在 `InitialMotorParameter` 配置（golden cinitial.cpp:4068，移植 :4585），測試沒走那條路。精簡鏡像的 `CopyToTray` 是只寫 `Item` 的樁 | 測試開頭照 golden 那段替兩個加熱盤配置紀錄。**產品程式沒問題**：wb_serve 開機的 `InitialMotorParameter` 會配置 |
| `W6_2c_InArmVariants`／`Batch2`／`Batch3` | `TMyKitSuck::ArmUpSideAllTypeIC ← GetShuttleState_*` | `ptrInSHT` 回到 golden 預設 NULL；精簡鏡像原本預設 `&FLCarryKit` 就是為了這裡。golden 的 `GetShuttleState_*` 只由 AutoClean 呼叫（12 處），且呼叫前一定先指派 `ptrInSHT`（AutoClean.cpp:1085／:1375／:2287／:3985） | 測試照呼叫端的做法先指派 `ptrInSHT=&FLCarryKit`。產品程式保留 golden 的 NULL |
| `ObserverCore`／`TemperFromCore` | —— | `BAD_COMMAND`，單獨重跑兩支都通過（暫時性，memory `v906-ctest-bad-command-is-not-a-regression`） | 不處理 |

修完 5 支＋相鄰的 `W6_2c_InArmVariantsBatch4`、`W906_DoIndexAutoClean` 全過。

⚠ **自己犯的錯（未提交前發現並修正）**：第 2 步的同行附加把程式碼接在既有行尾註解後面 —— `mykitsuck.cpp:2162` 的計數呼叫、`mykitsuck.h:271`／`:449`
的宣告都被 `//` 吃掉，編得過但不生效。改成「程式碼在第一個 `//` 之前」。另一次差點把 `using` 接在 `#include "cprod.h"` 後面（前置處理器會忽略）。
已記成 memory `same-line-append-lands-inside-trailing-comment`。

**收尾（21:0x～21:3x）**

| 項 | 內容 |
|---|---|
| 出貨組態閘門 | 21:01：169 項，失敗集合 = `GA1_ReadGeneralIni config_db config_loaders dfm2rc_idempotent`，與合併前基準逐項相同（`G_EXIT=8`、ctest.log 時間戳 21:01，確認不是讀到舊 log） |
| 模擬組態 | `build_b1dbg` 全量建置 exit 0 —— `mykitsuck.cpp` 的 `#ifdef SOFT_SIMULTE` 分支第一次被編譯。ctest 169 項 19 失敗。**對照 W3 之前（`08c78bbb`）的模擬組態基準**（detached sparse worktree 全新建置，7 分 34 秒）：166 項 20 失敗 —— 19 項逐項相同（4 項出貨基準＋15 項模擬組態語意：SimIO、W6_Canary、W6_4_TesterAnchor、HanaART、BarCodeHelpers、BarCode8CCDGlue、AGV_E84、Automation、W7_L1_Auto2／Color／Loader／AutoRT、GA2_C1_cinitial、WB_SimPump、mainproc_guard），基準多出的 `WB_DialogMailbox` 是它跟我的建置同時跑（現在的樹上通過）。⇒ **W3 第 1～8 項＋A4-6 在模擬組態 0 個新失敗** |
| 第 4 個重複定義 | NB2 R1 §5a 列的 `InArmPlaceSuck`（`cInArmPlacement.cpp:166`）。**連結沒報錯只是運氣**：nm 顯示它跟 `mykitsuck.cpp` 在同一個 archive（`libht9045_sm.a`），目前沒有任何執行檔抽出 `cInArmPlacement.cpp.obj`（wb_serve 0 個符號），一旦有人用它就是 multiple definition。退役，改用 golden 的 `mykitsuck.cpp:219`。另外用 nm 對 `mykitsuck.cpp.obj` 的 164 個已定義符號掃全部 archive：扣掉 inline／template（COMDAT，`TMySucker::SetNeedSuck`、`ChangeToFloatNonPcnt<>` 等）之後，重複的**只有這一個** |
| NB2 R1 §A 的活 ODR | `cOffSet.cpp:63`（JerryYang `8bfbab2f`）include 完整版、物件卻是精簡版佈局 ⇒ 讀 `iMotRow` 時超出物件尾端約 19.8 KB。A4-6 之後樹上只有一套佈局（`aHotPlateSubstrate.h` 本身改 include `mykitsuck.h`），**不需要改 JerryYang 那一行**；NB2 建議的「改 include `aHotPlateSubstrate.h`」在 A4-6 之後也等價 |
---

## 9. `InitialSuckerName`／`InitSucker`（20260924 夜，接在 A4-6 之後）

| 閘 | golden | 處置 |
|---|---|---|
| `InitialSuckerName` 7 個（`:329`／`:344`／`:357`／`:385`／`:403`／`:419`／`:426`） | :267-353 | ✅ 全開：`SuckerName`、`F/BTestSuckBackup`、`In/OutArmSuckBackup`、`CheckKitSuck` A4-6 後都活了 |
| `pSuck->Add(&CheckKitSuck…)`（`:488`） | :398-399 | ✅ 開 |
| InitSucker 大閘 `:493-878` | :401-774 | **拆開**：if（`IO_CARD_TYPE==NewIO_MN200 \|\| PCI_P64C64`，`:504-654`）變活碼；else（BDE `TTable`）在 `:656`／`:877` 另外閘住，理由寫使用者 20260924 裁決。`:655` 的 `else {` 保留空殼，控制流與 golden 相同。`TTable *T;`（`:502`）註解掉（型別不存在，只有 else 用） |
| `OffEnable=false` ×4（`:902`／`:907`／`:921`／`:926`） | :798-816 | ✅ 開 |
| `CopyKitSuck` 備份端 ×2（`:933`／`:1113`） | :821-822、:998-999 | ✅ 開。**還原端**（ChangeSite 的 47 個 N1-G4 閘）不在本項：ChangeSite 的呼叫者今天被兩個 TU 內巨集接縫攔下（`ckernel_shims.cpp:111`、`auto9045.cpp:139`）加 GATE N3-G6，要一起開 ⇒ 第 12 項。47 個閘的理由已原地改寫（「mykitsuck 刻意不註冊」過期）；只開備份端是安全的一半（備份只是複製） |

**第三級測試** `MachineSuckers_HT9050`（新，`tests/test_machine_suckers.cpp`，登記在 `tests/CMakeLists.txt` 檔尾並自己補圍堵環境變數）：
全表 49 個吸嘴 × Sen／On／Off 各 7 欄 **0 不符**；pSuck 內 53 個有名稱，表裡找不到的 4 個（`OutArm2SuckAA`／`AB`、`CheckKitSuck_1`／`_2`）照 golden 只 sprintf 不跳訊息；
三顆手算（`InArmSuckA` 第 571／583／582 列、`FTestSuckAA` 第 304／340 列、`CatchSuck` 第 12／29／32 列，數字從 CSV 直接讀，不經程式解析）；
負壓 TestSuck 的 OnAlarmTime ÷10 封頂 50；`ISABase` 最後等於 `SenISABase`；備份格同步命名、`CopyKitSuck` 後備份格帶 IO 接線；
Enable：模擬組態 golden 強制全關＋DelayTime 歸 0，出貨組態 `SenUsing!="" && 表 Enable==1`（HT9050 49 列全 0 ⇒ 另在 cwd 產生只改 InArmSuckA 一列的表驗 true 臂，不改版控的機台表）。
出貨組態 193/193；`GA2_C1_cinitial` 65/65。

⚠ **機台端要知道**：HT9050 的 IO 表 49 列吸嘴 `Enable` 全是 0 ⇒ 出貨組態下所有吸嘴 `Enable=false`、`DoOnIO` 不動作（golden 規則，NB2 R1 §0-3 也量到）。
要讓吸嘴真的動，要在機台的 `IO_Table.csv` 把對應列的 Enable 設 1。另外 `InArmSuckB`（第 575 列）`OnDelayTime=1`，其餘 7 顆是 10 —— 可能是筆誤，照表翻不改。
---

## 10. `LoadMotData`＋`InitialMotorName`／`InitialMotorParameter`＋五個 `InitMotor`（20260925 凌晨）

| 函式 | 狀態 |
|---|---|
| `LoadMotData`（database.cpp:1774） | ✅ 已活（第 5 項已驗 67 行對 67 行）；`W906_MOTTABLE_PATH` 接縫讓測試讀版控的機台表 |
| `InitialMotorName` | ✅ 無分支、0 閘 |
| `InitialMotorParameter` if 那半（:3870-4108） | ✅ 本來就是活碼（依 CardModel 建 1203／MN200／SYNTEK／SMC／Galil 物件、寫 16 個欄位、Enable 軸呼叫 InitMotor）。**本項補的是第三級測試** |
| GATE 1（:3854，`TTable *PT;`）／GATE 2（:4136-4517，motor.db／motor_SMC.db 兩臂） | 閘留著，理由原地改寫成「使用者 20260924 W3 裁決：只做 2/3 那半，BDE 不翻」 |
| GATE 3 的註解 | **過期更正**：原文說「InitialHandler 全樹 0 個呼叫者 ⇒ 安全門回呼是死的」，但 A4-2（20260924）起 `tools/wb_serve.cpp:3084` 就呼叫 `InitialHandler()`，`InitialOK` 也有活的寫入者（`WebBridgeTags.cpp:563`）⇒ **閒置安全門檢查在 wb_serve 是活的**（golden 行為，保留），註解原地改寫 |
| `:3703` 的宣告註解 | 過期更正：寫「本體是 acatchtray_shims 的空樁」，實際那個樁早就 `#if 0` 退役（PT-W7a），nm 只有 `cinitial.cpp.obj` 定義 `SetMotorAccelSpeed` |

**第三級測試** `MachineMotors_HT9050`（新，`tests/test_machine_motors.cpp`，登記在 `tests/CMakeLists.txt` 檔尾並自己補圍堵環境變數）：
餵 `machines/HT9050/Mot_Table.csv`、`IO_CARD_TYPE=2`、`INDEX_MOTION_CARD=1` 跑 `InitialMotorParameter()`：
表內 48 軸（awk 量 CSV：PCI1203 19、MN200 16、SMC 13）驅動類型與 CardModel **0 不符**、16 個欄位（齒輪比／方向／原點方向／四個速度／初速／伺服警報／1P2P／感測器型態／極限邏輯／In1 邏輯／兩個軟體極限／模擬速度／加速度）**0 不符**、
1203／MN200 位址碼往返 **0 不符**、尾段補齊後 **0 個 NULL 軸**（MainProc 每拍直接讀 `MOT[i].Motor->Enable`）；四軸手算（M00／M02／M14／M140，CSV 逐列直接讀）；
Enable 出貨組態 19 軸（剛好是表裡 19 個 1203 軸）、模擬組態 0 軸；`INDEX_MOTION_CARD=0` 時 Index 四軸改成 `TMyGALILMotor`（golden :3923-3948）。
出貨 86/86、模擬 83/83。

⚠ **量到的 golden 怪處（照翻，不改）**：`TMyEtherCatMotor` 自己宣告 protected 的 `short iBoardID/iPortID`，遮蔽 `HTMotor` 的 `unsigned int` 同名成員，
ctor 只寫自己那份、`HTMotor` 的 ctor 兩份都不寫 ⇒ **透過 `HTMotor*` 讀 1203 軸的 `iBoardID/iPortID` 是未定義值**（golden 相同：`HTMotor` 不是 TObject，BCB6 的 new 不清零）。
測試第一版就是這樣讀到垃圾值（看起來像 ASCII）才發現的。讀它的地方只有 MN200 Pitch 群組的 `SetGroup`（`cinitial.cpp:4637`、`mymotor.cpp:3748`／`:4663`），HT9050 那幾軸是 MN200／SMC／補位軸，不是 1203 ⇒ 今天無影響。

**五個 `InitMotor` 往下呼叫什麼**（nm `build_nosimg/wb_serve.exe`＋各 archive，這台筆電的出貨組態）：

| 類別 | 往下呼叫 | 這台（無 SDK） | 有 1203 卡的機台 |
|---|---|---|---|
| `TMyEtherCatMotor`（PCI1203） | `Open_Axis` → `Acm_AxOpenbyID`；`Acm_AxResetError`／`Acm_SetU32Property`…（`#if HAVE_PCI1203`） | `uiDevhand==0` ⇒ `Open_Axis` 直接返回、`InitMotor` 回 false，不碰硬體 | 開卡後**真的寫軸參數**；⚠ golden 在 ResetError 失敗時 `goto ResetMotorError` 無上限重試（照翻）⇒ W4 開卡時要看 |
| `TMyMN200Motor` | `mn_set_motion_cfg` ×11 | `vendor_offline_motionnet.cpp` 離線樁 | 同左（V906 沒連 MN200 DLL）⇒ HT9050 的 MN200 軸不會動（表內也全是 Enable=0） |
| `TMySMCMotor` | `SmcWSetCtrlTypeIn/Out`、`SmcWSetOrgLog`… | `vendor_offline_smc.cpp` 離線樁 | 同左 |
| `TMySYNTEKMotor` | `_Hon_m4_initial`／`_load_motion_file`／`_set_feedback_src`（翻譯碼）→ `_mnet_*` | 翻譯碼＋離線 motionnet | 同左 |
| `TMyGALILMotor` | 無（golden 本體就是 `return true`） | —— | —— |
wb_serve 匯入的 DLL 只有 KERNEL32／msvcrt／PSAPI／SHELL32／USER32／VERSION／WS2_32 —— 沒有任何廠商 DLL。

⚠ **機台設定（W4 要一起交代，18:50 那封信沒寫）**：HT9050 的 Index Z（M14 `MTestZ1`）在 Mot_Table 是 PCI1203，但 golden 看 `INDEX_MOTION_CARD==0` 就把
Index 四軸（13-16）改建成 Galil 物件、完全跳過表 —— 筆電 `system/Gerneral.ini:141` 就是 0。**HT9050 機台的 `[System] INDEX_MOTION_CARD` 要設 1**（全樹只比 `==0`，任何非 0 值都走表）。
C++ 目前還不能驅動 1203 馬達（W4），所以不急著寄；W4 交付時跟開卡設定一起寫進信。
---

## 9b／9c. 吸嘴格數與 ChangeSite 還原端（20260925 凌晨，NB2 R3 回覆 Q5／RB-3）

**NB2 抓到、我在 W3-9 漏掉的**：全樹 49 個 `SetItemAmount(` 全部在 **GATE n4-1**（`cinitial.cpp:10885-10992`，`SetMyKitSuckItemAmount` 整個本體）裡 ⇒
每個 `TMyKitSuck` 都停在 ctor 的 1×1，所有 `iMaxRow/iMaxCol` 迴圈只跑 `[0][0]`；W3-9 打開的備份端也只帶到 `[0][0]`。

| 顆 | 做了什麼 | 驗證 |
|---|---|---|
| 9b（8445ed2f） | 解 GATE n4-1：88 個敘述與 golden :5506-5611 比對 0 差異；閘理由在 A4-6 後全失效；wb_serve 在 golden ctor 位置（:3042／main.cpp:2121）先呼叫，比 InitSucker 早 | 出貨 171 項＝基準；模擬 0 個新失敗（MainCalcCore 是暫時性 Not Run，重跑 3/3） |
| RJ-03（93042c72） | NB2 R3：開機與存配方後的重讀，`fQAMode`／`fMagazine` 在 `fTrayAssignment` 之前（golden 相反）⇒ QAMode 缺鍵時把還沒讀進來的 `TrayForm.Loader.Direction` 寫進真實 `Tester.Data`。照 golden 對調（兩條鏈）。筆電 64 份配方都已有該鍵、3 天內 0 份被改 ⇒ 沒有要還原的 | 出貨＝基準；模擬 0 個新失敗 |
| 9c（c22dcb12） | 47 個 N1-G4 還原閘＋COPYBACKUP（golden ainarm9045 :1503-1513）解閘。ChangeSite 呼叫者是活的（`SetWorkParameter` :7126，N3-G6 已於 0921 退役）—— 我在 W3-9 寫「要等第 12 項接呼叫者」是錯的，原地更正 | 出貨 171 項＝基準；模擬 171 項 19 失敗，與 A4-6 當時逐項相同 |
| 測試 | `MachineSuckers_HT9050` 加 (2b)：格數沒設時備份只到 `[0][0]`；`SetMyKitSuckItemAmount` 後 F/BTestSuck 與 In/OutArmSuck 2×4、TestSocket 4×8，備份帶到 `[1][3]`（FTestSuckBD／InArmSuckH，第 305／572 列 Port 135 Bit 7） | 出貨 201/201、模擬 199/199 |

仍在的洞（W2，照順序排在 W4／W5 之後）：`ckernel_shims.cpp:111`／`auto9045.cpp:144` 的 TU 內 no-op `ChangeSite`、
`aTester_Front/Rear`／`atester_32Site` 的 no-op `MoveSuckData` 替身（NB2 R7：前臂、後臂、32-site 的 destroy 流程都被吃掉，要人在機台旁驗）。
---

## 11. `mytray` → `myTimer` → `MyTempPanel`（20260925 凌晨）

**`mytray`（被 40 支檔引用的 `FullIC`／38 支的 `HasIC`…）—— 整支照 golden 重翻**（golden 344 行，`AI(W906-W3-11)`）。舊檔頭自稱「PARTIAL W4 stub」，逐函式對 golden 量到：

| 項目 | golden | 舊移植 |
|---|---|---|
| 建構子 | `XItem=_MAX_COL_ITEM`、`YItem=_MAX_ROW_ITEM`，不呼叫 ClearData | `XItem=YItem=1`＋呼叫 ClearData |
| `ClearData` | 只清 `XItem×YItem`；既有生產紀錄呼叫 `PordRec->InitialRecord()`；`iBinData=-1` | 清整個 `_MAX`，**`PordRec` 全設 NULL**（開機配置好的紀錄遺失）；`iBinData=0` |
| `HasIC` | `Data` 非 0 就算 | 只認 `HAS_IC`／`HAS_NULL_IC`（清潔片、OCR、卡匣狀態都漏） |
| `HasRealIC` | 非 0 且非 `HAS_NULL_IC` | 只認 `HAS_IC` |
| `HasOnlyDataICAndNullIC` | 至少要有一顆 DataType 才 true | 全空也 true |
| `SetXYItem`／`SetBlockXYItem` | 鉗制到 `_MAX`、P06 CarrierTray 分支、`XBWidth/YBWidth` 規則 | 簡化版 |
| 11 個樁 | `HowManyIC`（7 支檔用）、`HowManyUpper/LowerHalfIC`、`HowManyICInBuffer`、`HowManyBinICInTray`、`HasDataIC`、`HasOCRIC`、`HasICCassette`、`HasEmptyCassette`、`HasCleanPad`、`CleanPlate2HasIC`、`Save/ReadUnloaderInfo` | `return false/0`／空函式 |

* **唯一的移植決定**：ctor 先把 POD 陣列清 0、`PordRec` 設 NULL，再套 golden 值。golden 的 `TMyTray` 全在靜態儲存（`MOT[].Tray`，本來就是 0），所以對它們是 no-op；只是讓堆疊／heap 上的實例（測試）確定，否則第一次 ClearData 會對垃圾指標呼叫 `InitialRecord()`。
* **golden 怪處照翻**：`Save/ReadUnloaderInfo` 從 `&iWhichSite` 起寫 `sizeof(UnloaderData)`（3 個陣列），但類別佈局裡 `iNeedRotAng`／`iCurrRotAng`（Steven 20170425）插在 `iWhichSite` 與 `iWhichIndex` 之間 ⇒ 檔案裡實際是 iWhichSite＋兩個旋轉角度陣列。改它等於改檔案格式，不動。
* ⚠ **寫到樹外**：`cinitial.cpp` 的 `SaveUnloaderInfo(int)` 寫 golden 寫死的 `D:\UnloaderInfo\…`（今天就已經在寫 `LastTrayInfo.txt`，09-24 18:06），現在 `TMyTray::SaveUnloaderInfo` 也會真的寫二進位紀錄。ctest 的圍堵環境變數沒蓋到這個路徑 ⇒ 後續要補一個 `W906_UNLOADERINFO_ROOT` 接縫（比照其他 `W906_*_ROOT`），寫進夜間報告 §2。
* 引用舊行為的註解同步更正：`mykitsuck.cpp:163`（「ClearData 把 PordRec 設 NULL」）。`ckernel_shims.cpp:264` 寫的「ctor 給 1×1 所以回 true」—— golden 的 30×70 一樣回 true，結論不變。

**`myTimer`**：逐敘述對 golden 0 差異（只有 include 不同）；被引用的 9 個函式都活 ⇒ ✅，不用動。

**`MyTempPanel`**：被引用的 6 個（ctor／dtor／SetParent／SetEnable／SetCaption／SetIndexTag）都是活的，編輯框的 `Text`（溫度值）也是活的。
67 個 `#if 0` 裡 62 個在 ctor（`Left/Top/Width/Font/Parent/Bevel…` 版面屬性，vclcompat 沒有這些成員），1 個 dtor、1 個 SetParent（`Parent/Align`），3 個是 MouseDown 叫小鍵盤＋溫度上下限檢查 ⇒ **全部是 UI，歸網頁**（pt-wave-loop 陷阱 #6：不補樁）。

驗證：出貨組態閘門（01:31）171 項，失敗集合＝基準；模擬組態（01:42）171 項 19 失敗，與前一輪逐項相同 ⇒ 0 個新失敗。新 ctest `MyTray_Golden`（鎖住上表每一條偏離＋11 個原本是樁的函式＋UnloaderInfo 往返）出貨／模擬都 32/32；這些斷言對舊樁一定失敗（例如 `HowManyIC()==3`，舊樁恆 0）
---

## 6b. vclcompat `TStringGrid` → BCB6 grids.pas 語意（20260925 凌晨，NB2 R2 Q1）

NB2 逐行讀了 BCB6 `grids.pas`：單元是 `{$R-}`（:12），`Cells` 是**稀疏**的列→欄結構（GetCells :5154-5160、SetCells :5180-5186），
**從不丟 ERangeError**；`SetRowCount/SetColCount` 只鉗制 >=1 然後 `ChangeSize`，**不碰資料**（:3783-3791、:2306-2374）。
舊的 vclcompat 用 `vector::at`，任何越界都丟 `std::out_of_range`、縮小會截斷 —— 兩點都不是 BCB6，而樹上好幾處註解還寫「跟真 VCL 的 ERangeError 一樣」。

| 項 | 新行為（= BCB6） |
|---|---|
| 儲存 | `std::map<列, std::map<欄, AnsiString>>`（map 節點在插入時參考不失效 ⇒ NB2 擔心的「同一運算式兩個格子參考懸空」不會發生） |
| 讀沒寫過的格子（界內或界外，非負） | `""` |
| 寫任何非負位置 | 存起來；`RowCount/ColCount` 不變（以它們為界的迴圈看不到） |
| 縮小 RowCount／ColCount | 資料保留，再放大看得到 |
| 負索引 | 丟例外（BCB6 的 EListError）。唯一沒照做的邊角：BCB6 讀「沒寫過的列、欄 < 0」回 `""`，這裡也丟 —— 全樹沒有這種呼叫 |

`StringGrid.h` 被 26 處程式碼、42 份文件用行號引用 ⇒ 標頭只做原地替換（`#include <map>` 放在原本的空行），`:75` 仍在原位。

連帶：
* `mycylin.cpp` 的氣缸計數（`fSmartDiagnostic->GetCyliderOn/OffCount`）4 個閘解開 —— 6b 本來就是為它開的子項；補 `forms/fSmartDiagnostic.h` include（放在原本的分隔線上）。
  計數本身仍受 `CosFunction.bCylinderOnOffTimeLog` 與記錄檔存在與否控制（golden 相同）。
* `test_uHGemEquipment.cpp:305-308` 重新校準：原本斷言「越界會丟 std::out_of_range」是移植樹自己發明的；改成驗 BCB6 的四條（越界讀 ""、縮小再放大看得到、越界寫存起來但計數不變、負索引丟）。
* `test_ga1_cprod` 直接編 mycylin.cpp、不連 forms ⇒ 補 `fSmartDiagnostic` 空替身（照這支測試既有的做法）。

驗證：出貨組態閘門（02:1x）172 項，失敗集合＝基準；模擬組態（02:28）172 項 19 失敗，與前一輪逐項相同 ⇒ 0 個新失敗。
（01:4x～01:57 那幾輪作廢：我的背景閘門子殼在建置失敗後沒停、重開時三組建置疊跑，輸出互相覆蓋；殺掉、刪掉殺之前之後產生的 .obj/.exe/.a 後乾淨重跑。已記 memory。）
---

## 12. `cinitial.cpp` 其餘被引用的函式（20260925 凌晨）

**盤點方法**：`python tools/w3_caller_census.py build_nosimg cinitial`（nm）⇒ 定義 321 個函式，**被其他檔引用 21 個**；再用 NB2 的
`tools/nb2_assist/stale_gate_reasons.py` 找理由屬於「X 不存在」的閘（cinitial 內 36 個），逐一在樹上重驗 —— 不信工具的分類。

**解開的 8 個**（golden 原文照舊，只換 `#if 0`／`#endif`，行號不變）：

| 閘 | 函式 | 內容 | 為什麼可以開 |
|---|---|---|---|
| N3-G13（:7363） | `SetSuckRetryCount` 前段 | In/Out 臂每顆吸嘴 `SetRetryCount(ArmSpeed[].iRetryCT)` | `TMySucker::SetRetryCount` 在 mykitsuck.cpp:2696（A4-6） |
| N3-G14（:7386）／N3-G15（:7402） | `SetSuckRetryCount`（ckernel 在用） | F/B 測試頭重試清 0、TrayArm 重試次數 | 同上 |
| n4-5a／b／c（:11194／:11242／:11268） | `UpdateMyKitSuckDelayTimeToProd` | 破壞延時 `OffDelayTime`＋重吹 `DestroyAgainCount/Time`（In/Out 臂、F/B 頭、TrayArm） | 欄位在 golden 佈局都有；這就是「放不掉的 IC」那組參數。⚠ **W3-12 當時其實沒接上**：這支函式 0 個活呼叫者（DoSetupSystemToProd 的 GATE n2-2 還閘著；AutoClean 打到檔內 static 空殼）—— NB2 R17 RW-01 量到，**W3-12c 才接上**（見下） |
| n4-7（:11354） | `InitialMachine` | `CheckKitSuck.ClearAll()` | `CheckKitSuck` 在 mykitsuck.cpp:217。⚠ 同上：唯一呼叫點 LoadMachineRecord 的 GATE n2-13 還閘著，**W3-12c 才接上** |
| W7a-I2（:13629） | `SetMotorSpeed`（56 支檔在用） | `SetGaliRate(ArmSpeed[IndexArm].iACDCBodySP)` | 唯一理由是 48 個 Gali 樁撞名；樁已退（P0-5），nm 只有 myGALILmotor.cpp 一份且已在 wb_serve |

**21 個被引用函式的現況**：

| 函式 | 引用 | 閘 | 判定 |
|---|---|---|---|
| `SetMotorSpeed` | 56 | n5-G1b（`fShuttleMove->fShow`）、n5-G1（`fShowMessage->sgdSpeedView`）、n5-G1c（`FrmRotate->SetIn/OutRotateSpeed` 0 定義） | 前兩個是 UI 狀態歸網頁（陷阱 #6）；第三個真的缺 |
| `IsNNMode`、`SetInArmSpeed`、`CompareTechData`、`SetHangupMaxTime`、`InitialHeaterDoor`、`SetMyKitSuckItemAmount`、`SetTechDataToProd_Yield`、`CheckFix3FullPlaceTechData` | 各 1～20 | 0 | ✅（`SetInArmSpeed` 的 csystem 呼叫被 TU 內替身攔下，那是 W2 的 NB2 R7 清單） |
| `SetWorkParameter` | 6 | N3-G12（`fMain->ShowFunctions`） | UI 歸網頁 |
| `SetMotorScaleSpeed`／`SetUnloaderInfoFile` | 6／1 | 0（工具把 :5065 的 BDE 閘誤算進來，那是 InitCylinder 的） | ✅ |
| `DoSetupSystemToProd` | 1 | ~~0~~ ⚠ **更正：當時 9 個**（n2-2～n2-10；n2-1、n2-11 先前已解）。W3-12c 解 n2-2／n2-4，剩 7 個：n2-3 `fAutoTeach` 沒 facade、n2-5 `fMain->SetOpenBin` facade 沒成員、n2-6～9 `tRotateShim` 缺欄位、n2-10 `CheckEPRange` 本體在 fHS 閘裡 | 原本那格是我照工具輸出抄的，沒逐條重驗 |
| `SaveMachineRecord` | 2 | N1-G3c（全域 `iHotInArmOrder` 不存在，golden 在 ainarm2）、N1-G3b（`sCleanPlaceRec` 綁在第二份 `uPlateInfo` 標頭） | 真的缺；N1-G3b 要等 `uPlateInfo` 像 A4-6 那樣合一（NB2 的重複類別普查還列著它） |
| `SetMotorAccelSpeed` | 2 | 0（工具把 InitialMotorParameter 的 BDE 閘誤算） | ✅ |
| `ShowMainScreenPresure` | 2 | N1-G6 ×6（fMain 扭力元件） | UI 歸網頁 |
| `InitShuttleThreadParameter` | 2 | n4-11（`fiosetview` 標籤） | UI 歸網頁 |
| `InitialHandler` | 1（wb_serve） | N1-G2b（`InitNTPort` 0 定義）、N1-G2c（COM2 `RS232Init`）、N1-G2e（`fStartCondition` 表單沒翻，A4-5）、N1-G2g（`dmTrayMotor`，跟 4b 面板共用 COM 埠） | 真的缺；N1-G2c／g 隨 4b 一起翻 |
| `SetTechDataToProd` | 1（wb_serve） | n5-G4／G6（`SetAOATrayTeachPoint` 0 定義）、n5-G9／G13（`tRotate.DutNum`）、其餘 n5-G5～G15（fMain 元件） | 真的缺＋UI |
| `SetSuckRetryCount` | 1（ckernel） | —— | ✅ 本輪全開（N3-G14／G15） |

**理由改寫（仍閘住）**：n2-10（`CheckEPRange` 已宣告但本體在 fHS 的 Cat E 閘裡）、N1-G5（`InitialAddrToATC` 已存在，擋住的是順序：
wb_serve 在 :3244 才 new `fTemp_Set`，比 `InitialHandler`（:3084）→ ChangeSite 晚 ⇒ 開了會在開機時解參考 NULL）。

### 12c. 更正：第一批解開的 n4-5a/b/c、n4-7 在執行期到不了（20260925 清晨，NB2 R17）

NB2 R17 RW-01 覆核 bda2d6fc：`UpdateMyKitSuckDelayTimeToProd` 與 `InitialMachine` 兩支函式**外部 0、本檔 0 個活呼叫者**。
我自己用 nm 重量（build_nosimg，W4-a 之後）：`__Z30UpdateMyKitSuckDelayTimeToProdv`、`__Z14InitialMachinev` 只有 cinitial.cpp.obj 定義（T），
**沒有任何 .obj 引用（U）**；同檔的呼叫點都在 `#if 0` 裡：

| 呼叫點 | 閘 | 理由（寫的） | 事實 |
|---|---|---|---|
| `DoSetupSystemToProd` :8308 | n2-2 | `UpdateMyKitSuckDelayTimeToProd` 全樹 0 定義 | 本體在同檔 :11087；閘上方的註解自己寫「EXPIRED -- OPEN THIS GATE AT INTEGRATION」 |
| `LoadMachineRecord` :9612 | n2-13 | `InitialMachine` 全樹 0 定義 | 本體在 :11308，同上 |
| `AutoClean.cpp:888` | —— | —— | 打到檔內 `static void UpdateMyKitSuckDelayTimeToProd() {}`（:188，「not yet translated」）把真本體遮掉（R17 RW-03） |

同一類過期的還有 n2-4（`SetHangupMaxTime`，本體 :11282）與 n2-15／n2-16／n2-22（`SaveMachineRecord`，本體 :16965）（R17 RW-02）。

**W3-12c 做的**：六個閘照同一模式解開（行號不變）；AutoClean 的 static 空殼原地改成 `extern` 宣告真本體。
`SetSuckRetryCount`（N3-G13～15）與 `SetMotorSpeed`（W7a-I2）**是活的**（nm：ckernel.cpp.obj／56 支檔引用），那三格當時沒說錯。
⚠ n2-15／16／22 開了之後，wb_serve 開機會照 golden 寫 `d:\HT9045\system\machinerecord.dat`（ctest 走不到 LoadMachineRecord）—— 真機驗證前先備份那個檔。
測試：`MachineSuckers_HT9050` 加第 (4) 段，直接呼叫 `UpdateMyKitSuckDelayTimeToProd()` 鎖破壞延時／重吹次數／間隔的手算值（In/Out 臂＋Index 第 8 欄）。
⚠ 共用區 35881445 那一版 README 第十一節第 4 點（「重吹次數／間隔現在會照參數設定」）**在那一版是錯的**，下一次更新共用區時更正。

驗證：第一批（bda2d6fc）與第二批各跑一次兩組態閘門 —— 出貨組態 172 項，失敗集合＝基準；模擬組態 172 項 19 失敗，與前一輪逐項相同（03:09、03:28）。


## 15. `mykitsuck`／`myTimer`／`MyTempPanel` 逐函式表（20260926 凌晨，W3 稽核第 ⑴ 項）

稽核缺口：mykitsuck 在 A4-6 合一之後沒重跑引用盤點（`W3_CALLER_CENSUS_20260924.md` 還寫「NOT COMPILED」）；myTimer、MyTempPanel 只有散文、沒有表也沒有測試名。
產生方式：`python tools/w3_function_tables.py ../Obj/V906/build_ship <輸出.md>`（nm 量 worktree 的出貨組態建置 `f4cddea6` 之後；規則見工具檔頭）。

結果：
* **mykitsuck**：定義 160 個函式、被引用 **68** 個，**本體內 0 個 `#if 0`**（整支檔 0 個）；17 個有測試 TU 直接呼叫。
* **myTimer**：定義 16 個、被引用 **9** 個，0 個閘（整支檔 0 個）；token 對 golden 只差 include（稽核覆核量過）。
* **MyTempPanel**：定義 45 個、被引用 **6** 個（呼叫端只有 `Temperature`、`uTemp_Set`）。建構子本體內 62 個 `#if 0`、`SetParent`／解構子各 1 個 —— 全是 VCL 版面／事件（`Parent`／`Align`／`Left`／`Top`／`OnMouseDown`…，vclcompat 的控制項沒有這些成員），**版面歸網頁**，不是缺相依；數值成員（`SetCaption`／`SetEnable`／`SetIndexTag`）0 個閘。沒有任何測試 TU 直接呼叫它。
* 「測試」欄是「—」不等於沒測到：只代表沒有測試 TU **直接**叫它；經產品碼間接走到的（例如 ArmFlow 類測試跑手臂流程時呼叫的吸嘴方法）這張表沒量。

### mykitsuck（68 個被引用）

| # | 函式 | 移植樹 | 本體內 `#if 0` | 引用它的產品檔數 | 測試 TU 直接呼叫它的 ctest（經產品碼間接走到的不算） |
|---|---|---|---|---|---|
| 1 | `TMyKitSuck::HasIC()` | `mykitsuck.cpp:774` | 0 | 82 | test_kitsuck_predicates、test_w7_c1_cleanout_finish、test_w7_c2_onecycle_finish |
| 2 | `TMyKitSuck::ResetAll()` | `mykitsuck.cpp:2739` | 0 | 64 | — |
| 3 | `TMySucker::Suck()` | `mykitsuck.cpp:2207` | 0 | 63 | — |
| 4 | `TMyKitSuck::SetItemData(int, int, int, int)` | `mykitsuck.cpp:338` | 0 | 59 | test_AutoClean、test_SCK_ART_Remainder |
| 5 | `TMyKitSuck::HasRealIC()` | `mykitsuck.cpp:621` | 0 | 49 | test_barcode_shuttle1_scan、test_barcode_shuttle2_scanremainder2、test_kitsuck_predicates、test_w7_grid_occupancy |
| 6 | `TMySucker::Destroy()` | `mykitsuck.cpp:2408` | 0 | 39 | — |
| 7 | `TMySucker::GetStatus()` | `mykitsuck.cpp:1964` | 0 | 34 | — |
| 8 | `TMyKitSuck::NoIC()` | `mykitsuck.cpp:416` | 0 | 33 | test_kitsuck_predicates |
| 9 | `TMyKitSuck::UseSiteHasIC()` | `mykitsuck.cpp:477` | 0 | 33 | test_atester_front_suckic、test_atester_rear_suckic、test_kitsuck_predicates、test_w7_a1_inarm_floating_latch…（5） |
| 10 | `TMyKitSuck::HasType(int)` | `mykitsuck.cpp:1769` | 0 | 30 | — |
| 11 | `TMyKitSuck::SetAllToNullIC()` | `mykitsuck.cpp:588` | 0 | 29 | test_atester_32site、test_atester_front_suckic、test_atester_rear_suckic、test_w6_2_inarm_core…（17） |
| 12 | `TMyKitSuck::UseSiteFullIC()` | `mykitsuck.cpp:462` | 0 | 28 | — |
| 13 | `TMyKitSuck::UseSiteNoIC()` | `mykitsuck.cpp:486` | 0 | 28 | test_w7_grid_occupancy |
| 14 | `TMyKitSuck::SetAll(int)` | `mykitsuck.cpp:1051` | 0 | 24 | — |
| 15 | `TMyKitSuck::RightSideNoIC(int)` | `mykitsuck.cpp:806` | 0 | 22 | — |
| 16 | `TMyKitSuck::SetPickerCount(int, int, int, int, int, int, int)` | `mykitsuck.cpp:395` | 0 | 22 | — |
| 17 | `TMyKitSuck::LeftSideNoIC(int)` | `mykitsuck.cpp:779` | 0 | 21 | — |
| 18 | `TMyKitSuck::SetUnuseAndHasNullICToNullIC()` | `mykitsuck.cpp:558` | 0 | 19 | — |
| 19 | `TMyKitSuck::ArmUpSideAllTypeIC(int, int, int)` | `mykitsuck.cpp:939` | 0 | 17 | — |
| 20 | `CopyInitSuck(TMyKitSuck*, TMyKitSuck*, int, int, int, int)` | `mykitsuck.cpp:1157` | 0 | 16 | — |
| 21 | `TMyKitSuck::ArmDownSideNoIC()` | `mykitsuck.cpp:881` | 0 | 15 | — |
| 22 | `TMyKitSuck::ArmUpSideNoIC()` | `mykitsuck.cpp:871` | 0 | 14 | — |
| 23 | `TMySucker::Normal()` | `mykitsuck.cpp:2189` | 0 | 14 | — |
| 24 | `TMyKitSuck::ArmDownSideAllTypeIC(int, int, int)` | `mykitsuck.cpp:1029` | 0 | 10 | — |
| 25 | `TMySucker::On()` | `mykitsuck.cpp:2165` | 0 | 10 | — |
| 26 | `TMyKitSuck::CountRealIC()` | `mykitsuck.cpp:753` | 0 | 9 | test_w7_grid_occupancy |
| 27 | `TMySucker::Off()` | `mykitsuck.cpp:2180` | 0 | 9 | — |
| 28 | `TMyKitSuck::AlreadyTest()` | `mykitsuck.cpp:1709` | 0 | 8 | — |
| 29 | `TMyKitSuck::RowHasDefineIC(int, int)` | `mykitsuck.cpp:703` | 0 | 7 | — |
| 30 | `TMyKitSuck::ClearAll()` | `mykitsuck.cpp:1039` | 0 | 6 | test_AutoClean、test_barcode_helpers、test_kitsuck_predicates、test_w6_2_inarm_core…（15） |
| 31 | `TMyKitSuck::SetAllRealIC2InterfaceBin()` | `mykitsuck.cpp:2783` | 0 | 6 | — |
| 32 | `TMySucker::OffDestroy()` | `mykitsuck.cpp:2159` | 0 | 5 | — |
| 33 | `TMySucker::Reset()` | `mykitsuck.cpp:1901` | 0 | 5 | — |
| 34 | `TMyKitSuck::CopyFromTray(int, int, int, TTrayMotor&, int, int, int, int, bool)` | `mykitsuck.cpp:1609` | 0 | 4 | — |
| 35 | `TMyKitSuck::HasRealIC_Left(int, int)` | `mykitsuck.cpp:647` | 0 | 4 | — |
| 36 | `TMyKitSuck::HasRealIC_Right(int, int)` | `mykitsuck.cpp:675` | 0 | 4 | — |
| 37 | `TMyKitSuck::IsPickDestroyFinish()` | `mykitsuck.cpp:988` | 0 | 4 | — |
| 38 | `TMyKitSuck::MoveSuckData(TMyKitSuck&, int, int, int, int)` | `mykitsuck.cpp:1489` | 0 | 4 | — |
| 39 | `TMyKitSuck::MoveSuckDataDiff(TMyKitSuck&, int, int, int, int)` | `mykitsuck.cpp:1549` | 0 | 4 | test_AutoClean |
| 40 | `TMyKitSuck::CopyToTray(int, int, int, TTrayMotor&, int, int, int, int)` | `mykitsuck.cpp:1660` | 0 | 3 | — |
| 41 | `TMyKitSuck::IsPickSuckFinish()` | `mykitsuck.cpp:975` | 0 | 3 | — |
| 42 | `CopyKitSuck(TMyKitSuck*, TMyKitSuck*)` | `mykitsuck.cpp:2818` | 0 | 2 | — |
| 43 | `TMyKitSuck::ArmDownSideHaveRealIC(bool)` | `mykitsuck.cpp:1001` | 0 | 2 | test_w6_2c_inarm_variants |
| 44 | `TMyKitSuck::ArmLeftSideHaveRealIC(int)` | `mykitsuck.cpp:822` | 0 | 2 | test_w6_2c_inarm_variants |
| 45 | `TMyKitSuck::ArmRightSideHaveRealIC(int)` | `mykitsuck.cpp:846` | 0 | 2 | test_w6_2c_inarm_variants |
| 46 | `TMyKitSuck::ArmUpSideHaveRealIC(bool)` | `mykitsuck.cpp:911` | 0 | 2 | test_w6_2c_inarm_variants |
| 47 | `TMyKitSuck::HasDefineIC(int)` | `mykitsuck.cpp:727` | 0 | 2 | — |
| 48 | `TMyKitSuck::IsShtDestroyFinish()` | `mykitsuck.cpp:962` | 0 | 2 | — |
| 49 | `TMyKitSuck::IsShtSuckFinish()` | `mykitsuck.cpp:949` | 0 | 2 | — |
| 50 | `TMyKitSuck::SetNullIcToHasNullIc()` | `mykitsuck.cpp:529` | 0 | 2 | — |
| 51 | `CopySuck(TMySucker*, TMySucker*)` | `mykitsuck.cpp:2853` | 0 | 1 | — |
| 52 | `TMyKitSuck::All_HasIC()` | `mykitsuck.cpp:445` | 0 | 1 | — |
| 53 | `TMyKitSuck::ArmAll_HasICType(int, int)` | `mykitsuck.cpp:495` | 0 | 1 | test_AutoClean |
| 54 | `TMyKitSuck::FindNoIC()` | `mykitsuck.cpp:427` | 0 | 1 | test_AutoClean |
| 55 | `TMyKitSuck::HasNotTestYet()` | `mykitsuck.cpp:1077` | 0 | 1 | — |
| 56 | `TMyKitSuck::PartAlreadyTest()` | `mykitsuck.cpp:1747` | 0 | 1 | — |
| 57 | `TMyKitSuck::SetAllHASIC2ErrorBin()` | `mykitsuck.cpp:2800` | 0 | 1 | — |
| 58 | `TMyKitSuck::SetItemAmount(int, int)` | `mykitsuck.cpp:378` | 0 | 1 | — |
| 59 | `TMyKitSuck::SetMotorCount(int, int)` | `mykitsuck.cpp:389` | 0 | 1 | — |
| 60 | `TMyKitSuck::SetPickerCount(int, int)` | `mykitsuck.cpp:407` | 0 | 1 | — |
| 61 | `TMyKitSuck::SetType1ToType2ByPickCol(int, int)` | `mykitsuck.cpp:2947` | 0 | 1 | — |
| 62 | `TMyKitSuck::ShtAll_HasICType(int, int)` | `mykitsuck.cpp:512` | 0 | 1 | test_AutoClean |
| 63 | `TMySucker::GetOnBit()` | `mykitsuck.cpp:1908` | 0 | 1 | — |
| 64 | `TMySucker::OnDestroy()` | `mykitsuck.cpp:2153` | 0 | 1 | — |
| 65 | `TMySucker::OnSuck()` | `mykitsuck.cpp:2135` | 0 | 1 | — |
| 66 | `TMySucker::ReStart()` | `mykitsuck.cpp:1895` | 0 | 1 | — |
| 67 | `TMySucker::Sensor()` | `mykitsuck.cpp:1989` | 0 | 1 | — |
| 68 | `TMySucker::SetRetryCount(int)` | `mykitsuck.cpp:2696` | 0 | 1 | — |

### myTimer（9 個被引用）

| # | 函式 | 移植樹 | 本體內 `#if 0` | 引用它的產品檔數 | 測試 TU 直接呼叫它的 ctest（經產品碼間接走到的不算） |
|---|---|---|---|---|---|
| 1 | `TQPF_Timer::Off()` | `myTimer.cpp:40` | 0 | 119 | test_barcode_8ccd_glue、test_globals、test_w0tail_headers、test_w7_l1_auto…（8） |
| 2 | `TQPF_Timer::SetSecAndOn(double)` | `myTimer.cpp:88` | 0 | 106 | — |
| 3 | `TQPF_Timer::TQPF_Timer()` | `myTimer.cpp:14` | 0 | 74 | test_ga1_cmydb、test_ga1_cprod、test_globals、test_w0tail_headers…（6） |
| 4 | `TQPF_Timer::LatchCycleTime(bool)` | `myTimer.cpp:123` | 0 | 41 | — |
| 5 | `TQPF_Timer::SetMSAndOn(unsigned long)` | `myTimer.cpp:94` | 0 | 36 | test_w6_1_empty_canary、test_w6_canary、test_w7_l1_auto、test_w7_l1_auto2…（7） |
| 6 | `TQPF_Timer::Set0_1SecAndOn(double)` | `myTimer.cpp:82` | 0 | 17 | — |
| 7 | `TQPF_Timer::LatchCycleTimeSec(bool)` | `myTimer.cpp:106` | 0 | 7 | — |
| 8 | `TQPF_Timer::On()` | `myTimer.cpp:32` | 0 | 2 | test_globals、test_w0tail_headers |
| 9 | `TQPF_Timer::SetSec(unsigned long)` | `myTimer.cpp:46` | 0 | 2 | — |

### MyTempPanel（6 個被引用）

| # | 函式 | 移植樹 | 本體內 `#if 0` | 引用它的產品檔數 | 測試 TU 直接呼叫它的 ctest（經產品碼間接走到的不算） |
|---|---|---|---|---|---|
| 1 | `TMyTempPanel::SetCaption(AnsiString)` | `MyTempPanel.cpp:683` | 0 | 2 | — |
| 2 | `TMyTempPanel::SetEnable(bool)` | `MyTempPanel.cpp:651` | 0 | 2 | — |
| 3 | `TMyTempPanel::SetIndexTag(int)` | `MyTempPanel.cpp:688` | 0 | 1 | — |
| 4 | `TMyTempPanel::SetParent(TTabSheet*)` | `MyTempPanel.cpp:639` | 1 | 1 | — |
| 5 | `TMyTempPanel::TMyTempPanel(AnsiString, int)` | `MyTempPanel.cpp:77` | 62 | 1 | — |
| 6 | `TMyTempPanel::~TMyTempPanel()` | `MyTempPanel.cpp:598` | 1 | 1 | — |

### myio（4 個被引用）

| # | 函式 | 移植樹 | 本體內 `#if 0` | 引用它的產品檔數 | 測試 TU 直接呼叫它的 ctest（經產品碼間接走到的不算） |
|---|---|---|---|---|---|
| 1 | `IOBitOff(int, int)` | `myio.cpp:316` | 0 | 3 | — |
| 2 | `IOBitOn(int, int)` | `myio.cpp:282` | 0 | 3 | — |
| 3 | `IOInputBit(int, int)` | `myio.cpp:423` | 0 | 3 | — |
| 4 | `IOOutBitStatus(int, int)` | `myio.cpp:378` | 0 | 3 | — |

myio（W3 稽核第 ⑹ 項，NB2 需求單 Q9(3)）：4 支被引用的函式**都有翻、本體內 0 個 `#if 0`**（需求單寫「沒翻」不對，nm 在 wb_serve.exe 看得到四個 T 符號）。
呼叫端是 mycylin／mysensor／myswitch／mykitsuck 的舊式 `eISABase`／`ePCI1735U`／`ePLCbase` 分支。本體裡實際會做的事：
* `IOBitOn`／`IOBitOff`：先跑 `MYIO_IDLECHECKSAFEDOOR`（GATE 4 已開，BU-D7；→ `csystem.cpp` 的 `IdleCheckSafeDoorByCylinder(int,int)`），`TTL_CARD_TYPE>0` 直接 return；
  更新軟體影子 `lOutPortData[]`／`OutPortData[]`；真正寫卡的 `MYIO_OUTPORTB` 在 x64 上是 `((void)0)`（`myio.cpp:162`，GATE 1）。
* `IOOutBitStatus`：只讀軟體影子，0 個閘。
* `IOInputBit`：安全 PLC 點（`bPLCIO[port][bit]`）讀 `bPLCInData`（活的）；其餘原始 port 走 `MYIO_INPORTB`，x64 上回 0（GATE 2）。
* ⚠ `OutPortData[ret]` 的索引檢查是 golden 的 `ret>127`，陣列越界問題是 RULINGS_20260925 第 18 條（「OutPortData 越界要修」），排在整合 P17 時一起做（INBOX 第 16 列），這裡不動。
