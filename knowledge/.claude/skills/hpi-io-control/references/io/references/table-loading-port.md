> 保存來源：`.claude/skills/ht9045-io-control/references/table-loading-port.md`，main `372b91908`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# V906 移植樹怎麼讀 IO_Table／Mot_Table（S-23 A1，2026-10-04）

> 來源：St01 ST01-E2 的 S-23 回覆（`docs/handoff/S23_FINDINGS_A1A2C5_20261004.md`，在 `v906/steven-handoff`），
> 是 EastSun 派工 7（HT9050 全機 HOME 異常）的一部分。行號取自 GitLab main `247e6c23` 的
> `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`；機台上可能漂，請用函式名找。
> 本檔講的是**移植樹實際的行為**。SKILL.md「IO 表 Enable 技巧」那一節是 golden BCB 的觀點，兩者大致相同，差異在下面標 ⚠。

## 載入順序（開機）

1. `tools\wb_serve.cpp` 的 `main`：`LoadMachineConfig()` → `HSys.ReadGeneralIni()`（`database.cpp:3158`）
   → 讀 `IO_CARD_TYPE`（`database.cpp:1208`）→ `LoadIoData()`（`:1212-1216`）。
2. `InitialHandler()`（`tools\wb_serve.cpp:4066`）→ `InitHontechHardware`（`cinitial.cpp:16989`）：
   依序是 `InitSucker`（`:11052`）、`InitialSwitch`（`:11053`）、`InitialSensor`（`:11064`）、
   `InitialMotorParameter`（`:11066`，裡面呼叫 `LoadMotData`，`:3876`）、`InitCylinder`（`:11067`）。
3. **只有 `IO_CARD_TYPE` 是 2／3／4 才會讀表並綁定**（`database.cpp:1212`；`cinitial.cpp:504`、`:1519`、`:2642`、`:3873`、`:4861`）。
   HT9050 是 4（PCI1203_IO），其他值兩張表都不讀、什麼都綁不到。
4. op log 要到 `tools\wb_serve.cpp:4305`（`W906_OpLogInit`）才開，所以**讀表時的錯誤訊息只印在主控台**
   （`ShowMyMessage`，`canary_support.cpp:157-178`），不會進 op log，也不會上畫面。

## IO_Table（`database.cpp` `LoadIoData` :1697-1763、每列 `TIODATA` :1872-2038）

- 用欄名找欄位（`SetIOTableNo` :2080-2200），表頭要剛好 15 欄才讀；不對就整檔不讀，並出現「data is mistake! (n)」。
- 拆欄照 VCL CommaText：**沒加引號的空白會切斷欄位**（例如名稱 `C Shuttle` 會讓後面每一欄都錯位）。
- 數字一律 `atoi`（ISABase 1／2／4 的 Port 用 `HexStrToInt`）。**非數字會安靜地變 0**，例如「+-」→ 0，沒有任何訊息。
- 空欄位：位址類變 -1、ISABase 變 eMotionNet、InType 變 0。
  - ⚠ **Port 或 Bit 空，或 eMotionNet 的 Lane／IP 空 ⇒ 整列強制 `Enable=0`**（`:1980`）。
  - 只把 Enable 改成 1、卻沒填位址，還是 0，也沒有任何訊息。
- 名稱重複：第一列生效（「alias is duplicated!」）。
- 名稱空白：永遠綁不到。
- IOType 空白：照樣綁定（綁定只看 Alias）。

## 物件怎麼拿到 Enable

- **氣缸**（`InitCylinder`，`cinitial.cpp:4851-5040`）：
  - **`Enable` 只看 `Cylinder` 輸出列**（`iEnable==1` 而且有輸出位址，`:5018-5021`）。
  - `_On`／`_Off` 列只設定感測位址與 `OnSenEnable`／`OffSenEnable`（`:4912-4980`）。
  - ⚠ golden 的 `AUTO_EMPTY_COLOR` 尾段會**蓋掉表格**（`:5242-5278`）：0（四軌）會把 `C_EmptyLoaderZ_Select`／`C_ColorLoaderZ_Select`／`C_Empty_Fix`／`C_Color_Fix`／`C_Color_Up`／`C_Color_Middle`／`C_Empty_Up`／`C_Empty_Middle` 全部設成停用；1（六軌）則全部打開。
    HT9050 現用值是 0（S-23 A1-2）。
- **氣缸停用或缺列時**：
  - `On()`／`Off()` 不動作（`mycylin.cpp:262`、`:304`）。
  - `OnSensor()`／`OffSensor()` 回 **true**（`mycylin.cpp:194-197`）。
  - `OnSenEnable` 是 false 時，原始的 `OnStatus()` 回 **false**（`:180-181`）。
  - 兩個感測都停用、但氣缸本身開著時，`OnSensor()`／`OffSensor()` 也回 true（`:205-227`）：到位判斷會一律立刻成功。
- **感測器 `Sen[]`**（`InitialSensor` :2635-2723）：只有那一列 `iEnable==1` 才開；缺列就是停用、欄位是 0。
  - 例外：`SenBit0..19` 一律強制開（`:2878-2882`）；`bTTLCanUse8Site` 時 `SenBit20..39` 也是；`Enable_PLCSafety_IO`＋ePLCbase 強制開（`:2708-2714`）。
- **開關 `SW[]`**（`InitialSwitch` :1513-1569）：只有 `iEnable==1` 才開；缺列（`:1563`）就是停用。
- **吸嘴**（`InitSucker` :504-619）：`Enable` 看名稱等於吸嘴名的那一列。
  - ⚠ `_On`／`_Off` 列**不看 Enable 欄**，Port 不是 -1 就算開（`:567`、`:580`）；吸嘴本身停用時才全部關掉。

## Mot_Table（`LoadMotData` :1774-1840、每列 `TMOTDATA` :2507-3131）

- 用 `M%02d` 找列（`cinitial.cpp:3881-3882`）。表的 Alias 欄**不用**，別名寫死在 `InitialMotorName`（`:3295-3316`）。
- GearRatio／Acc／Dec 用 `atof`，其他用 `atoi`。
  - ⚠ `0.0.071425` 會讀成 0.0，`cinitial.cpp:4043` 直接抄進 `Motor->GearRatio`，沒有保護。
  - 1203 類別只有 `MotorMove` 擋了 0（`Motor\myEthercatmotor.cpp:1614`）；HOME 的 `:1424`／`:1698`、軟極限 `:1452`、Pitch `:1884` 都會除以 0。
- ⚠ **任何一欄空 ⇒ 整列強制 `Enable=0`**（`database.cpp:3091-3094`）。例外：IP 只有 SYNTEK 才讀，PickLimit 只有 MTestZ1／Z2 才讀。
- 表頭判斷是 `>=28`：缺 SimulateSpeed 也會通過，而且會蓋掉其他缺欄的錯誤；多一欄反而整檔失敗（`:1799`、`:2263-2267`）。
- 沒有那一列的馬達槽：會建立 `TMySMCMotor(-1)`，**指標不是 NULL**，`Enable=false`，表上的值一個都不抄（`cinitial.cpp:4000-4021`、`:4041`）。
  - 不看 Enable 就直接呼叫的路徑，會打到離線的 SMC 樁，每次都回 -1（`Motor\vendor_offline_smc.cpp:31-40`）。
- PCI1203 軸以 `Acm_AxOpenbyID(dev, BoardID, Port)` 開（`Motor\myEthercatmotor.cpp:384`）：**BoardID＝環上站號、Port＝站內軸號**。
  - `D:\HT9045\config\Pci1203Axis.ini` 也用「站號.站內軸號」當 key，開機時值不同才寫（`EtherCAT\Pci1203Control.cpp:2915-2985`）。

## 改表格之前先查

1. 改了 Enable，相關的位址欄有沒有填（不然會被強制成 0）？
2. 數字欄有沒有打錯字（不然會安靜地變 0）？GearRatio 不能是 0。
3. 氣缸：輸出列與 `_On`／`_Off` 列的 Enable 是不是你要的組合？Empty／Color 那 8 顆會被 `AUTO_EMPTY_COLOR` 蓋掉。
4. PCI1203 軸的 (BoardID, Port) 有沒有重複？在不在環上（op log 的 `ROUTE st X ax Y claim`）？

## 開機驗表 TableAudit（E-043，2026-10-04）

> Steven 1004 07:2x Q96＝B，07:4x 定案規則表與原則：「**IO／馬達的裝置有異常 ERROR 的時候，機台就不可以動**」「一開始就開不了，怎麼會還可以 home?」。
> golden 沒有這個功能（新功能）。程式：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\TableAudit.h`（規則表在檔頭）／`TableAudit.cpp`（純函式）／`TableAuditLive.cpp`（讀檔與全域變數）；
> ctest `TableAudit`（`tests\test_tableaudit.cpp`，每條規則各一個壞表樣本）。

- **什麼時候跑**：`tools\wb_serve.cpp:4066`，在 `InitialHandler` 與 `W906_BootSummary` 之後（表已經讀完、也綁定完）。
  - 會重新讀 IO_Table／Mot_Table 的**原始文字**，因為載入程式已經把打錯的字轉成 0，綁定後的表看不出來。
  - 主控台印 `[BOOT] 驗表：ERROR n／WARN n（INFO n）`，每一項再印一行 `TABLEAUDIT <等級> <規則> <哪一列>`。
  - op log 開在 `:4305`，所以這些行先存起來，由 `:8340` 的 `W906_TableAuditSetNote` 補寫成 `AUDIT` 行。
- **規則**（ERROR＝這台一定出事；WARN＝很可能錯，只記錄；INFO＝只記數量）：
  - **ERROR**：
    - T1：HT9050 的 `IO_CARD_TYPE` 不是 4，或表讀不進來。
    - T2a：Enable 1 的列有非數字的值，或 GearRatio ≤ 0。
    - T4：兩個 Enable 1 的 PCI1203 軸 (BoardID, Port) 相同。
    - T5a：兩個 Enable 1 的輸出（Cylinder／Switch／Sucker_On／Sucker_Off）位址相同。
    - T9：HT9050 上 Enable 1 的軸 CardModel 不是 PCI1203。
  - **WARN**：
    - T2b：Enable 0 的列有壞值，或時間欄是非數字。
    - T3：表上 Enable 1，但有空欄位，被強制成 0。
    - T5b：輸入點重複。
    - T6：氣缸輸出列和 `_On`／`_Off` 列的 Enable 不一致。
    - T7：`AUTO_EMPTY_COLOR` 蓋掉表格。
    - T8：名稱重複或空白、列太短、格子裡有空白。
    - T11：Pci1203Axis.ini 對不到軸，或 pel／mel 缺一個。
    - T12：IO 的 1203 站號不在 Pci1203Modules.ini 裡。
  - **INFO**：T10，程式有用到、表上沒有的名稱。
- **擋不擋**：
  - **第一筆（10/05 機台用）只記錄、不擋任何動作**。Steven 07:4x：「明天機台要可以動起來，先以安全能動為主」。
  - **第二筆（10/05 18:00 以後）**：有 ERROR 就擋所有會動的入口。
    - HOME 和 START：`WebStart.cpp:1148`（StartFromWeb）、`csystem.cpp:30460`（面板／SECS／TCP，SoftStart 和 iHome）、`tools\wb_serve.cpp:5713`（網頁全機 HOME）。
    - 手動移動：`WebMotorAccess.cpp:4369` `MotorAccessDispatch` 的 motion 類，包括 Motor Test、JOG、教導、單軸 HOME。
    - 氣缸和開關輸出：`tools\wb_serve.cpp:6215` 的 IO 點擊。
    - **停止／中止（motor.stop、act.home.abort）永遠不擋**。拒絕時的紀錄文字是 `W906 TABLEAUDIT: <動作> refused -- table ERROR <規則> <列>`。
- **2026-10-03 機台現用表的結果**：ERROR 0、WARN 44、INFO 1。
  - WARN 是 T6 23 個、T5b 14 個、T7 2 個、T8 3 個、T2b 1 個（M37）、T11 1 個（station0）。INFO 是 T12，因為沒有附 Pci1203Modules.ini。
  - 所以第二筆上線後，照現在的表不會擋住機台。
- **改表或改規則之前先查**：ERROR 的規則要保守。每多一條 ERROR，就是多一個「機台完全不能動」的理由；新規則先當 WARN，跑一陣子沒有誤報再升級（Steven 決定）。

<!-- preserved-content:end -->
