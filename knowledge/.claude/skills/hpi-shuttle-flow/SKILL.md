---
name: hpi-shuttle-flow
description: HPI Handler Shuttle 流程、狀態機與安全互鎖，涵蓋 HT9045 SHT1/SHT2 與 HT9050 獨立 In/Out Shuttle。詢問 Do_Auto_SHT1、Do_Auto_SHT2、Do_Auto_InSH、Do_Auto_OutSH、AutoSHT1Task、AutoSHT2Task、左右停位、殘料、floating、sensor broken、2DID/OCR、retry/skip/home、CheckShuttleOutputHasICError、CheckNullICShuttle1_9045、CheckNullICShuttle2_9045、CheckShuttleSensorBroken_1/_2、Rotate Shuttle、Fix3 或 InArm/OutArm/Index 互鎖時使用。先依機型及版本分流，再讀對應 reference。
---

# HPI Shuttle Flow

推送基準與後續裁決：[最新main整合](references/main-integration-20261006.md)。

確認實際執行版本與 MachineTypeChoice，先從分派入口識別當前函式。
讀 [common.md](references/common.md)，再依機型與問題按需讀下表。

| 情境 | 文件 |
|---|---|
| HT9045 SHT1/SHT2、三站雙 Kit、左右流程 | [ht9045.md](references/ht9045.md) |
| HT9050 獨立 In/Out、M18 出料 Y、測區互鎖 | [ht9050.md](references/ht9050.md) |
| 客戶碼造成 retry/skip 差異 | [customers.md](references/customers.md)，核對版本與 Task |
| SHT1/SHT2 舊版完整 case 表 | [ProcessFlow](references/ht9045/Do_Auto_SHT1_SHT2_ProcessFlow.md) |
| OCR/2DID/CCD | [BarcodeFlow](references/ht9045/Shuttle_OCR_2DID_BarcodeFlow.md) |
| Arm 三層互鎖 | [SafetyInterlock](references/ht9045/Shuttle-Arm-Safety-Interlock.md) |
| NN 下壓期預掃 | [PreScan](references/ht9045/NN_2DID_PreScan_DuringIndexDown.md) |
| Out-Kit 置偏與 WAR0495 | [OutKitStuck](references/ht9045/OutKitStuck_Detect_WAR0495.md) |
| 舊根目錄正文、Loader 幾何、timeout／SCK HT9046LS 案例 | [歷史正文](references/ht9045/legacy-body-20261006.md)，依 ht9045.md 的來源版別使用 |
| 20260915 合併前對照 | [歷史參照](references/ht9045/colleague-skill-body-20260915.md) |

其他機型尚未在本樣板逐項抽出差異；先查實際分派與機型旗標，再用歷史文件對照。沒有專屬檔不代表同通用。

必記事項：

- HT9050 的 AutoSHT2Task 是 OutSH 游標；M18 MOutShuttle2 是出料飛梭 Y 軸，不是第二組飛梭。
- 保留每道移動、到位、Z 安全與臂位置互鎖；區分硬體裁決、程式實作及待確認事項。
- HT9050 設定／工單疑點依 [AGENTS.md](../../../AGENTS.md) 機台同步規則先確認快照；文件分析不等於已同步或已上機。
- OK／HOME 會清狀態時先留 State Record；執行期設定依專案既有授權與備份流程。
- 本批僅整理文件；程式變更另行認領、驗證與 MR。
