# HT9045_CONFIG 欄位速查：群組 [B] Report（報表 / 紀錄）

> 來源：
> - `cConfiguration.dfm`（元件名稱與 Caption [Bxx]）
> - `cConfiguration.cpp`（`elConfig->Add` 元件 ↔ 變數綁定）
> - `Config.h`（變數型別與註解）
> - 版本：V3.33.903.0_20260417

> 欄位命名規則：`b` = bool、`i` = int、`d` = double、`as`/`s` = AnsiString

---

## 已綁定 UI 元件的欄位

共 18 個欄位，分屬 9 個區段。

| 區段 | Caption | UI 元件 | 變數名 | 型別 | INI Key | ECID | EC Type | Function Description | 程式註解 |
|------|---------|---------|--------|------|---------|------|---------|----------------------|----------|
| B01 | Use Precaution Record Function | `cbB01` | `bB01_UsePrecautionRecordFunction` | bool | `bB01_UsePrecautionRecordFunction` | -- | -- | Precaution Record Function | Sam 20171120 (Steven) AddPrecautionRecordFunction (form HT7045) |
| B01 | Use Precaution Record Function | `edB01` | `iB01_AutoWakeupPrecautionRecordFormTime` | int | `iB01_AutoWakeupPrecautionRecordFormTime` | -- | -- | Precaution Record Function | Sam 20171120 (Steven) AddPrecautionRecordFunction (form HT7045) |
| B01-1 |  | `edB01_1` | `asB01_PrecautionRecordSavePath` | AnsiString | `asB01_PrecautionRecordSavePath` |  | ECInteger | Auto Wake Up Precaution Record Form Time min | Sam 20171120 (Steven) AddPrecautionRecordFunction (form HT7045) |
| B02 | Use Handler Major Maintenance Record Function | `edB02` | `asB02_HanderMajorMaintenanceRecordSavePath` | AnsiString | `asB02_HanderMajorMaintenanceRecordSavePath` | -- | -- | Handler Major Maintenance Record Function | Sam 20171120 (Steven) AddHanderMajorMaintenanceRecordFunction (form HT7045) |
| B02 | Use Handler Major Maintenance Record Function | `cbB02` | `bB02_HanderMajorMaintenanceRecordFunction` | bool | `bB02_HanderMajorMaintenanceRecordFunction` | -- | -- | Handler Major Maintenance Record Function | Sam 20171120 (Steven) AddHanderMajorMaintenanceRecordFunction (form HT7045) |
| B03 | Use tester report function | `cbB03` | `bB03_TesterReport` | bool | `bB03_TesterReport` | -- | -- | Tester Report | Sam 20231115 : PTI 新增 Tester report |
| B03 | Use tester report function | — | `sB03_Customer` | AnsiString | — | -- | — | Tester Report | Sam 20231115 : PTI 新增 Tester report |
| B03 | Use tester report function | — | `sB03_DeviceID` | AnsiString | — | -- | — | Tester Report | Sam 20231115 : PTI 新增 Tester report |
| B05 | O/S Test Report | `cbB05` | `bB05_OSReport` | bool | `bB05_OSReport` | -- | -- | O/S Test Report |  |
| B05 | O/S Test Report | `edtB05` | `sB05_OSReportPath` | AnsiString | `sB05_OSReportPath` | -- | -- | O/S Test Report | Steven 20250513 : OS Report |
| B11 | Enable | `cbB11Enable` | `bB11UsePATServerFile` | bool | `bB11UsePATServerFile` |  |  | Read PAT Server | Jimmychiu 20241110 : add PAT Class |
| B11 | Enable | `edB11PATServerPath` | `sB11PATServerPath` | AnsiString | `sB11PATServerPath` |  |  | Read PAT Server | Jimmychiu 20241110 : add PAT Class |
| B12 | Enable | `cbB12Enable` | `bB12UsePATSetup` | bool | `bB12UsePATSetup` |  |  | PAT SET UP | Jimmychiu 20241110 : add PAT Class |
| B12 | Enable | `edB12Path` | `sB12PATSetupPath` | AnsiString | `sB12PATSetupPath` |  |  | PAT SET UP | Jimmychiu 20241110 : add PAT Class |
| B13 | Management PAT Job Parameter | `edB13DownloadPath` | `sB13PATJobDownloadPath` | AnsiString | `sB13PATJobDownloadPath` |  |  | Management PAT Job Parameter | Jimmychiu 20241110 : add PAT Class |
| B13 | Management PAT Job Parameter | `edB13UploadPath` | `sB13PATJobUploadPath` | AnsiString | `sB13PATJobUploadPath` |  |  | Management PAT Job Parameter | Jimmychiu 20241110 : add PAT Class |
| B14 | PAT REPORT | `edB14IntervalTime` | `iB14IntervalTime` | int | `iB14IntervalTime` |  |  | PAT REPORT | Jimmychiu 20241110 : add PAT Class |
| B14 | PAT REPORT | `edB14ReportRealTime` | `sB14RealTimePath` | AnsiString | `sB14RealTimePath` |  |  | PAT REPORT | Jimmychiu 20241110 : add PAT Class |

---

## 未綁定 UI 元件的欄位

以下 3 個 Config.h 中以 `B##` 開頭的欄位
在 `cConfiguration.dfm` 中未找到對應 UI 元件（多為內部狀態 / 歷史欄位 / `ReadIniData` 直接讀取）：

| 變數名 | 型別 | ECID | Function Description | 程式註解 |
|--------|------|------|----------------------|----------|
| `b1x2Use4Suck` | bool |  |  | Hung 20110812 : 1x2使用4吸嘴 |
| `b1x4Use8Suck` | bool |  |  | Hung 20110812 : 1x4使用8吸嘴 |
| `b2x2Use8Suck` | bool |  |  | Hung 20110812 : 2x2使用8吸嘴 |
