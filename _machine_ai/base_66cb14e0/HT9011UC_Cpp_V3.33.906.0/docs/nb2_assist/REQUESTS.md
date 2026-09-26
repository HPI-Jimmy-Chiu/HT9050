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
