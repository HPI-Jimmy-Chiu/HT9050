# 給 NB2 輔助 session 的需求單

> **這個檔歸新電腦。** NB2 只讀、不改（建立這份空白格式之後，NB2 就不會再動它）。
> NB2 每輪 `git pull` 後會先處理這裡「未處理」的項目，結果寫在 `docs/nb2_assist/README.md` 的最新條目，
> 並在那裡註明是回應第幾號需求。

## 格式

```
### Q<編號> — YYYYMMDD HH:MM
要什麼：（分析哪個函式／哪個 commit／做什麼工具）
為什麼：（卡在哪一波、要拿去做什麼）
完成條件：（怎樣算做完）
```

## 需求

### Q1 — 20260924 20:20
要什麼：vclcompat `TStringGrid` 越界語意的影響盤點。列出全樹（移植樹，排除 tests/）每一處 `->Cells[..][..]` 讀／寫，
  分成三類：(a) 迴圈上界用的是別的東西（不是同一張表的 RowCount／ColCount），可能越界；(b) 保證在界內；(c) 依賴「越界會丟例外」
  （例：`SECSGEM/uHGemEquipment.cpp` 的 GetCEIDContent／GetReportIDContent 呼叫點註解、`tests/test_uHGemEquipment.cpp:616`）。
  每筆附 golden 對應行（用你的 UTF-8 鏡像）。
為什麼：W3 子項 6b。`vclcompat/StringGrid.h:75` 說越界丟 `std::out_of_range`「模仿真 VCL 的 ERangeError」，但本機 BCB6
  `grids.pas` 查過是回空字串（memory bcb6-stringgrid-cells-oob-read-benign）。mycylin 的氣缸計數（`fSmartDiagnostic->GetCyliderOnCount`）
  就卡在這裡：golden 迴圈讀到 i=294 不管 RowCount，開了會在氣缸動作時丟 golden 不會丟的例外。要改 vclcompat 之前得先知道誰依賴現在的行為。
完成條件：三類清單＋每筆 golden 行號；結論一句「改成回空字串會改變哪幾個測試／哪幾條 SECS 行為」。

### Q2 — 20260924 20:20
要什麼：`uPadInterface`（golden `uPadInterface.cpp` 962 行、`.h`、`.dfm`）翻譯範圍預勘：哪些是 RS232 協定與按鍵狀態（該歸 C++），
  哪些是畫面（歸網頁）；`TPadRS232Thread` 與 SPComm `TComm` 在移植樹有沒有可用的替身（`vclcompat/Comm.*`？）；
  mysensor.cpp:72/:119/:167、myswitch.cpp:78/:128/:178 那六個閘各自需要它的哪個方法（IsPadKey／ProcessScanKey／IsPadButton／SendSwitchStatus）。
為什麼：W3 子項 4b。筆電 `Gerneral.ini` 是 `ControlPanelMode=1`，這種機台上移植樹的實體面板鍵（Start／Pause／Reset…）兩邊都讀不到。
完成條件：一張「golden 函式 → 歸 C++／歸網頁 → 相依是否存在」的表，加一段建議的翻譯順序。

### Q3 — 20260924 20:20
要什麼：absence 哨兵抓到的過期宣稱「TfOffSet has no GetOffsetPath」（`SECSGEM/uHGemHT9045.cpp:823` A3、`ainarm2.cpp:1323-1358`）。
  JerryYang `8bfbab2f` 已新增 `TfOffSet::GetOffsetPath`。請列出這兩個檔裡**因為這個宣稱而閘住**的每一段：行號、golden 原文對應、
  打開後會走到什麼（檔案讀寫？定位補償？），以及你判斷的安全等級。
為什麼：W2 候選。ainarm2 那段是入料臂位置補償檔路徑，碰定位，我要逐條重問「為什麼它當初該閘」。
完成條件：每段一列的表，附建議（可開／要人在機台旁驗／維持閘住）。

### Q4 — 20260924 21:40
要什麼：W3 第 10 項預勘 —— `LoadMotData`（database.cpp）、`InitialMotorName`／`InitialMotorParameter`（cinitial.cpp）、五個 `InitMotor`
  （`Motor/mymotor.cpp`、`myMN200motor.cpp`、`mySMCmotor.cpp`、`mySYNTEKmotor.cpp`、`myEthercatmotor.cpp` 各自的 `InitMotor`）。
  (1) 每個函式在移植樹的閘位（`#if 0`／樁）與擋住的相依，對 golden 逐段對照（同 R1 吸嘴那份的格式）；
  (2) **只看 `IO_CARD_TYPE==NewIO_MN200 || PCI_P64C64` 那半**（使用者 20260924 裁決：else／BDE 那半不做）；
  (3) `TMyEtherCatMotor::InitMotor` 寫進卡的參數與順序，對照 EastSun 的 `Pci1203Axis.ini`（`machines/HT9050/`）與他 `Pci1203Control`
  的開卡／設參數順序，列出不一致處；
  (4) HT9050 `machines/HT9050/Mot_Table.csv` 45 軸的手算對照值（仿 R1 §7，挑 3 軸：一軸 1203、一軸非 1203、一軸有特殊欄位）。
為什麼：W3 第 10 項緊接在吸嘴之後，然後就是 W4（MotorTest 真的驅動 1203 馬達）。第 (3) 點決定 W4 開卡要不要改 `InitMotor`。
完成條件：閘位表＋EastSun 對照表＋3 軸手算值；「待 Jimmy」的另外列。

### Q5 — 20260924 21:40
要什麼：`cinitial.cpp` ChangeSite 的 N1-G4 還原端地圖（檔頭 GATE 註解 `:16536` 起的說明；閘在 `:12082-13449`，47 個閘、669 個
  CopySuck／CopyKitSuck 敘述）。逐閘列：行範圍、golden 對應行、屬於哪個機種／站數組合（`MachineTypeChoice`／`USE_PICKER_COUNT`…）、
  除了 `CopySuck`／`CopyKitSuck`／`*Backup` 之外還缺什麼相依。
為什麼：A4-6 之後吸嘴只剩一套、`CopyKitSuck`／`CopySuck`／`*Backup` 都活了；W3 第 9 項會打開**備份端**（InitSucker 的 `CopyKitSuck`），
  還原端就是這 47 個閘。它們決定「選了哪幾站，Index 真空要怎麼換位」—— 沒開等於每種模式都是恆等對應（檔頭註解寫的
  「the wrong nozzle…」那類症狀）。要一次開完還是分機種開，先要這張地圖。
完成條件：47 列的表＋「只缺吸嘴四件、可以直接開」的有幾個／「還缺別的」的有哪幾個。

---

## 📣 20260925 15:5x 新電腦：**先讀這段**（使用者：「依照你現況使用量，很快就會使用完，如果有需要舊電腦協助，可分工給它」）

### Q0 — 對齊 main（最優先，影響你之後每一輪）
- **feat/v912-port 已於 9/25 09:00 鎖定，main 是主線**。feat 現在落後 `origin/main` **151 顆**（main 頭 `c8858192`，之後還會推）。
  你在 feat 上量的東西已經是舊碼：W4 MotorTest、W5-a 教導頁、Steven review6、IOWEB-P4、IO 燈號、HT9050 家族分派、InitDIOStstus 都只在 main。
- ⇒ 從下一輪起：`D:\HT9045\_wt_assist` 對齊 **`origin/main`**；你的產出（只動 `docs/nb2_assist/**`、`tools/nb2_assist/**`）**推到 main**
  （路徑不重疊，被拒就 pull 再推）。需求單之後我寫在 **main** 的這個檔；這一段是最後一次寫在 feat。
- 使用者的收尾時間 T＝**2026-09-29（二）09:00**（RULINGS_20260925 第 1 條，main 上的 `docs/RULINGS_20260925.md`）。

### Q6 — Index Z 扭力（r28torq）要的 vclcompat 串列埠語意對照（決策用）
要什麼：`vclcompat/Comm.h/.cpp`（`Spcomm::TComm`）對 golden SPComm 的差異表，以及全樹 **TComm 使用端** 的清單（檔:行、是不是活的、
  開哪個埠、dfm 上設了哪些屬性：Outx_CtsFlow／Outx_DsrFlow／DtrControl／RtsControl／ReadIntervalTimeout／各 timeout）。
  重點三件：(1) `ReaderProc_` 在 ReadIntervalTimeout=MAXDWORD、total timeout=0 時是否空轉（吃滿一顆 CPU）；(2) `WriteCommData` 在呼叫端
  執行緒同步 WriteFile、WriteTotalTimeout=0 ⇒ 硬體流控卡住時會不會卡死單執行緒節拍；(3) `ApplyCommState_` 沒套 dfm 的流控／DTR／RTS。
  對每個使用端說明：補上「SPComm 預設＋WriteTotalTimeoutConstant＋讀取閒置 Sleep(1)」之後行為會怎麼變。
為什麼：新電腦的 r28torq（`D:\HT9045\.claude\worktrees\r28torq`，未 commit）照 BCB6 把 Index Z 扭力改走真的 RS232，但審查發現上面三件
  會擋輪詢，**要使用者裁決**（這類「會擋住 tick 迴圈的整合」一律先問）。我要一張讓他 5 分鐘看得懂的表。
完成條件：差異表＋使用端表＋「建議修法與每個使用端的影響」一段；標「待 Jimmy」。

### Q7 — 只列 `Type_HT9046`（沒有 LS／1032）的 14 處分派：HT9050 要不要算（決策用）
要什麼：用 `tools/nb2_assist/machine_family_dispatch.py --min-family 1` 在 **main** 上重量（我 15:4x 量到活 12、`#if 0` 2），逐處列：
  golden 行號、那段在做什麼、背後的**硬體假設**（例：`Motor/mymotor.cpp:3522` TrayArmMotorMove 的 `iSafePos=49750` 是什麼位置、單位；
  `acarry.cpp:279/482/675/834` 四支飛梭 IC 檢查為什麼 HT9046 直接 return —— 是不是因為沒有那組 sensor；對照 `machines/HT9050/IO_Table.csv`
  看 HT9050 有沒有那些 sensor）、HT9050 不加時實際走哪一臂。
為什麼：RULINGS 第 5 條「HT9050 是 HT9046 家族」只涵蓋 R18 的 `--min-family 2` 清單；這 14 處不在裁決範圍，其中兩類碰運動／感測，要使用者／EastSun 決定。
完成條件：14 列的表，分「非運動類（可直接照家族加）」與「運動／感測類（要 EastSun 確認）」兩組，每列附建議。

### Q8 — R29 新工作的預勘（三件，照 R1 那種格式）
(a) **M108 MCCDY MotorID 1080 > `m_Axishand[999]`**（使用者「依據建議」＝陣列開大）：列出所有以 MotorID／軸號為下標、上限是 999／1000 的陣列與
  迴圈（檔:行、宣告、使用點數），哪些要一起開大才不會變成另一個越界；golden 對應行。
(b) **TfMain 建構子沒讀的機台鍵**（使用者「要翻」）：golden `main.cpp` TfMain 建構子讀 Gerneral.ini 的每一個鍵（INDEX_SUCKER_TYPE、ZSafePos…），
  逐鍵標：移植樹讀了沒（Steven review6 已翻 EP_Install／INOUT_ARM_PICKER_USE_MOTOR／ION_FAN_TYPE／EP_MAX* 那兩段）、誰用它、缺鍵時 golden
  的 MessageBox／強制寫值、和 D44 泵（R14）的先後關係（否則 WAR1604 會被 `bIndexCheck1` 永久關掉）。
(c) **EP 電控比例閥子系統**（adam6024／APAX，使用者「要」）：翻譯範圍——檔案、函式、golden 行數、相依、已翻／未翻，建議切幾波。
完成條件：三份表＋各自的「建議順序」。

### Q9 — W3 剩下的機械工作清單（給我照表施工）
要什麼：(1) ~~`database.cpp` 裡被改成英文的 **38 處**訊息~~（**0926 新電腦已自己做完，不用做**）：逐處「移植樹行號／目前英文／golden 行號／golden 原文中文（UTF-8）」；
  (2) ~~`mykitsuck.cpp`／`myTimer.cpp`／`MyTempPanel.cpp` 三支的逐函式表（golden 函式 → 移植樹位置 → 活／閘／樁）~~（0926 新電腦已做，W3_PROGRESS §15，不用做）；
  (3) ~~`myio` 那 4 支被引用但沒翻的函式：呼叫點、golden 本體行數、相依~~（0926 新電腦已量完：其實都有翻，見 W3_PROGRESS §15 末，不用做）。
為什麼：W3 稽核（c507adea 追加範圍）完成條件缺的就是這幾項；表做好我就能機械式改，不必再逐檔讀 golden。
完成條件：三張表；(1) 要逐字、零 U+FFFD。

### Q10 — 合併後哨兵（常駐那一條，這次指定範圍）
要什麼：覆核 main 上 **`f45f6235`（HT9050 家族分派）**、**`0ca03ee6`（InitDIOStstus；開機那處刻意閘住）**、以及之後會推的 W5-b（教導頁運動鈕）
  與 yesno（YES/NO 網頁對話框，RULINGS 第 10 條）。照你 R27 的做法：ODR／COMDAT／替身遮蔽／引用行號是否造假。
完成條件：每顆 commit 一段；有問題的標嚴重度與檔:行。

### Q11 — 20260925 16:4x（使用者：「周末任務幫我評估一件事，排在最後」）
要什麼：Index Z 扭力上限在 golden 走 RS232（`rs232.cpp` TCOM2：Panasonic A5 / 三菱封包，iWriteAndCheckMotorTorque 寫入→讀回→比對，ReadTorque_* 讀扭力回饋）。
  HT9050 的 Index Z（MTestZ1／MTestZ2）是 1203 EtherCAT 軸 ⇒ 評估**改走 1203 SDO** 是否可行：(1) HT9050 Z 軸驅動器型號（`machines/HT9050/Mot_Table.csv`、
  `Pci1203Axis.ini`）與它的 CiA402 物件字典：扭力上限（0x6072 Max torque、0x60E0／0x60E1 正負扭力限制，或廠商 Pn 參數對應的物件）、扭力實際值（0x6077）；
  (2) EastSun 的 Pci1203Monitor／Pci1203Control 現有 SDO 讀寫介面（哪個函式、同步還是排程、單位、錯誤碼），能不能直接用、要不要新 kind；
  (3) golden 扭力流程每個呼叫點（atester DoTestHeadMotor case 120／12000／12110、uhome 320／322、ShowMainScreenPresure、ReadTorque）逐一對應到 1203 版的做法，
  單位換算（golden Prod.iMaxPreasure 是什麼單位 → 0.1% 額定扭力？）；(4) 用 `MachineTypeChoice==Type_HT9050` 分流時要改的點位清單。
為什麼：使用者要「如果技術上可行，開始評估9050都是透過1203來獲取，你用machine type=9050來區隔功能使用」；排在週末最後，新電腦先要一份量過的預勘。
完成條件：可行性結論（可行／有條件／不可行＋理由）、物件對照表、呼叫點對照表、建議的實作切法；需要 EastSun 確認或上機量的另列「待 Jimmy／EastSun」。

### Q12 — 20260925 18:4x（新電腦：氣缸逾時警報要照 golden 接上，缺 HAlarm 原始碼）
要什麼：舊筆電上 `D:\HT9045\elec\Component\HAlarm.cpp`、`halarm.h` 的 **UTF-8 鏡像**（放 `docs/nb2_assist/golden_elec/`，原檔 Big5 用 cp950 讀），
  加一段說明：(1) `HAlarm::Set(iCode)`（約 :110-130）完整語意 —— `GetStat` 去重是以什麼為單位、`SetStat` 何時設、`Parent` 是什麼；
  (2) `HAlarm::Clear(iCode)`／`Clear()`／`ClearAllAlarm`（:234）各清什麼；(3) golden 氣缸（mycylin.cpp:84-92 的 SetAlarm／ClearAlarm，
  :343／:400／:492／:549 四個呼叫點）送進佇列後，ckernel `ProcessAlarm`（golden ckernel.cpp:2501-2527）怎麼把它變成畫面上的告警、會不會停機。
為什麼：W3 稽核第 ⑺ 項「氣缸逾時警報被樁吞掉，生產中不會停機」。移植樹已有佇列接縫（canary_support.cpp 第 7 節 W906_PopUpAlarm_Push／PopUpAlarm），
  但 HAlarm::Set 的去重規則不在這台筆電上（0922 換機沒搬 elec\Component），照「≥90% 才修」不能憑空設計去重。
完成條件：鏡像檔＋上述三點的語意說明；新電腦據此把 mycylin 的 SetAlarm／ClearAlarm 照 golden 接到佇列。

### Q13 — 20260927 03:5x（新電腦：ChangeLog 輸入端要照 golden 接上，有兩個 BCB6 語意要先量）
要什麼：
  (1) golden `common.cpp:891` 的 `if(ret!=Str && InitialOK==true)`：`ret` 是 double、`Str` 是 AnsiString（`Str.sprintf("%0.4f", Value)`）。
  BCB6 怎麼解析這個比較？是把 `ret` 轉成 AnsiString（等於 FloatToStr，"1.5" 對 "1.5000" ⇒ 幾乎一定不相等 ⇒ 每寫一次 double 就記一筆）還是別的？
  NB2 那台若有 BCB6：請貼 `Include\Vcl\dstring.h` 裡 `operator!=`／`operator==` 的宣告（成員還是非成員）與 `AnsiString(double)` 建構子；
  有 bcc32 的話編一支 5 行的小程式（`double r=1.5; AnsiString s="1.5000"; printf("%d", r!=s);`）印結果最準。
  (2) 那台若有客戶的 HANDLER LOG（EventLog csv）：找「change Value」的紀錄，看 double 欄位沒改值時會不會出現「1.5000==>1.5000」這種同值紀錄。
  (3) golden `Items->Strings[ret]` 超出範圍時（ini 裡的值比選項數大），BCB6 是不是丟 EStringListError ⇒ 整個 WriteIniData 在寫檔之前就中斷（`try` 只包 Write）。
為什麼：golden common.cpp:622-1099 五個 `WriteIniData` 多載的 change-log 區塊與 `TempChangeLog`（:1802-2037）要照 golden 接上（St02 0927 03:06 確認現在可以做）。
  移植樹的 vclcompat 會把 `double != AnsiString` 解成字串比較、`Strings[]` 超出範圍回空字串；BCB6 若不一樣，就會多記或少記一整批紀錄。
完成條件：(1) 的結論附證據（標頭原文或實測輸出）；(2)(3) 有就附，沒有就寫「查不到」。

### Q14 — 20260927 09:0x（新電腦：工具 #31 認檔頭的 golden 版本宣告；R100 覆核）
要什麼：
  (1) 工具 #31 `golden_cite_version.py`：檔頭寫明 golden＝V912 的檔（例 `tools/editlist/TestIF_File_BarCode.py`「golden V912 TfBarCode」、
  `FileRW/TestIF_File_TesterIF.cpp`「golden V912 TFTestIF」、`forms/fSortCT.cpp`、`WebRecipeChange.cpp`），整支跳過「應引 906」的檢查。
  理由：那些是照 RULINGS_20250925 S37／RULINGS_20260926 第 26 條「畫面讀寫照 912」翻的，引 V912 行號是對的（St01 07:00 抽查 `WebRecipeChange.cpp:652`
  `main.cpp:29354` 在 V912 正是 `TfMain::EnabledSetupFile`）。筆電已回 St01 選 A（TO_STEVEN §4 09:0x）。
  (2) 覆核筆電的 TASKLIST（你 R100 提的同一件，筆電 08:5x 做了、還在 gate）：`cStateRecord.cpp` 檔尾 `W906_BootRegisterTaskList`。
  跟 R100 建議不同的地方：**不補宣告**，13 列閘著（R100 的 10 個，加 `fBarCode->iBottom2DIDTask`＝`BarCode_Bottom2DID.cpp:84` 匿名 namespace、
  `fBarCode->i2DIDCheckTask`＝移植樹只有 `forms/fContact.h:1531` 那個同名的 TfContact 成員（另一個變數）、`iDoAllPassVerifyTask`＝只在
  `atester.cpp` 的 golden 原文閘裡）；`&fContact->X` 對到 `fContactForm`、`&fBarCode->X` 的 8 個對到 `BarCode_Shuttle{1,2}_*.cpp` 的全域。
  請逐列看這 13 列與這兩種對映對不對（尤其「移植樹真正推這個 task 的變數是哪一個」）。
完成條件：(1) 工具改好＋selftest；(2) 對映表的每一列「對／不對＋證據」。

### Q15 — 20261001 01:3x（量測工具的守門：量測中「新增」的檔沒有移走）
要什麼：你 `v906/nb2-assist` 的 `tools/nb2_assist/webhmi_runtime_measure.py` 還原那一步只蓋回「被改過的」檔，新增的檔只列出來（`ADDED ... (left in place, listed for review)`）。筆電 0930 23:49～1001 00:05 用它的副本量了三次串流（改前／改後），wb_serve 開機時 `SetMD5ByFolder` 在作用中工單 `IniData\Data\FT005054\` 寫了新的 `<md5>.MD5`，還原又把舊的放回去 ⇒ 資料夾裡有兩個 `*.MD5`；golden `CompareMD5ByFolder`（cpublic.cpp:955-989）看到超過一個會**全部刪掉**，這張工單就沒有檢查碼了。筆電已把那個檔搬到 `D:\HT9045\backup\night_quarantine\20260930\`（MANIFEST 有記），副本也改成：新增的檔（`web/` 除外）搬到 `<out>/added_moved/<rel>`，`still_differing` 把沒搬走的新增檔也算進去，所以 `backup_deleted` 只在全部回到原狀時才會是 true。請照這個思路改原版。
完成條件：原版改好＋selftest 驗三件事——新增的檔被搬走（原位置不存在、`added_moved` 裡有）、guard_report 寫明搬到哪、有沒搬走的新增檔時 `backup_deleted` 是 false。

### Q16 — 20261001 07:0x（問題 A 修了，請在新 main 上重量停止延遲；問題 B 還要你）
要什麼：在 main 第十一批（GitHub 第 96 包）上重跑你的 GATE-1 與 S13（`webhmi_cmd_latency.mjs`／`webhmi_firstload_stall.py`，同樣帶種子檔＝8.6 MB 的 `Production-update.json`）。筆電這批做了兩件事：(1) `/JSON` 清密碼路由加檔案快取（大小／最後寫入／建立時間沒變就不重讀不重掃；只收 ≥256 KB、最後寫入 ≥2 秒前的檔；輸出位元組不變）＋開機時在背景執行緒先讀好 `JSON\` 與 `JSON\js\` 的大檔（主控台會印一行 `[SCRUBCACHE] prewarm …`）；(2) `settings.js` refreshProduction 先發 HEAD，ETag 沒變就不 GET。
為什麼：你 3fb68638 量到閒置停止 p95 155～285 ms，只擋這支輪詢是 27 ms；筆電實測：新舊兩版模擬 wb_serve 對同一支 8.6 MB 檔：回應位元組 MD5 相同（清過密碼的 General-config.json 也相同），伺服器首位元組 GET 98→19 ms、HEAD 86→6 ms；開機背景預讀 10 個大檔 21.7 MB 用 265 ms；兩次單獨跑 wb_serve 改到的 6 個 system／config 檔已從快照還原（MD5 核對）、新增的 lastdata_backup2.dat 搬到隔離區。問題 B（伺服器啟動後第一次載入卡 4～8 秒、低 CPU）筆電沒有重現環境，預讀只是把第一次大檔讀取移出 socket 執行緒，**沒有證明它治好 B**。
完成條件：(1) 閒置停止 p95 與 `sys.ping` 最大值（新 main vs 3fb68638 那一輪）；(2) 第一次載入卡住還在不在、多久；還在的話，卡住時對 socket 執行緒取堆疊（你 S13 寫的下一步）。
