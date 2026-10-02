# Recipe 下載 / 上傳詳細流程

## 1. 數據流架構

```
[Server / RMS / FTP]
       ↓ *.zip 檔案
btDownloadClick / PP_DL_REQUEST / SECS/GEM / GPIB
       ↓
DownloadFromServer(sDLFileName, bFromFTP)
       ├─ 連線驗證
       ├─ 舊有工作檔判斷（bHasSetUpFile）
       ├─ DoBackupSetupFile()   ← 備份本機關鍵參數
       ├─ 下載 zip + 解壓（7z.exe）
       ├─ MD5 校驗（若 IniConfig.bN20_CheckMD5）
       └─ DoOverWriteSetupFile()  ← 依 Security_new.def 還原本機參數
              ↓
       btSaveSetupFileClick（若為 _NET 檔）
       ├─ XCOPY 到 IniData\Data 目錄
       └─ EventReport(SwitchSetupFile)
```

---

## 2. DownloadFromServer 主流程

**函式簽名**：
```cpp
bool __fastcall TfLotInfo::DownloadFromServer(AnsiString sDLFileName, bool bFromFTP);
```

### 2.1 流程詳解

```
Step 1: 連線驗證
  IniConfig.bEnableRms? → 檢查 RMS / RmsDownPath 是否存在
  Connection Fail → return false (WAR1683)

Step 2: 工作檔是否已存在
  bHasSetUpFile = DirectoryExists(DataPath + sDLFileName)
  不存在 → 建立目錄 + 加入 cbSetupFileName 清單
  └─ MyForceDirectories(sDLOffsetPath)

Step 3: Offset 檔同步
  sDLOffsetPath + "Position Offset.Data" 不存在 → 從目前工作檔複製

Step 4: 下載 zip（若 bEnableRms==true）
  str2 = IniConfig.sRmsPath + sDLFileName + ".zip"
  CopyFile(str2, DataPath+sDLFileName+".zip")
  失敗 → WAR1684 Retry/Skip

  [若 ATC Recipe 需下載]
  bUseFTPDownLoadATCRecipe && ATC_SYSTEM==eNewATCSystem && bATCActiveCooling
  → 下載 sDLFileName+"ATC_Recipe.zip" → 解壓到 sATCPath

Step 5: 備份本機參數
  bHasSetUpFile==true → DoBackupSetupFile(DataPath, sDLFileName1)

Step 6: 解壓 zip
  7z.exe e "DataPath\sDLFileName.zip" -o"DataPath\sDLFileName\" -y
  等待解壓完成（最多 100*500ms = 50秒）

Step 7: MD5 校驗
  IniConfig.bN20_CheckMD5 && CompareMD5ByFolder() == 0 → 失敗（WAR16118）

Step 8: 驗證 8 個必要檔案
  ArmCondition.Data, Binasgn.Data, Contact.Data, HandlerCondition.Data,
  HotPlate.Data, Temperature.Data, Tester.Data, Tray.Data

Step 9: 清除 zip
  DeleteFile(DataPath+sDLFileName+".zip")

Step 10: 還原本機參數
  bHasSetUpFile==true → DoOverWriteSetupFile(DataPath, sDLFileName)

Step 11: 成功回傳 true
  → ShowErrorMessage("MES1687") 提示解壓成功
```

---

## 3. DoBackupSetupFile — 備份本機關鍵參數

**目的**：在下載新 Recipe 前，先把本機不希望被伺服器覆蓋的參數記錄到內部變數。

**備份的關鍵參數（存入成員變數）**：

| 成員變數 | 來源 INI | 來源欄位 |
|---------|---------|---------|
| `fTempUserOffset[tcTotalCount]` | `Temperature.Data` → `[User OffSet]` | 各 CH 溫度偏移 |
| `fTempATCOffset[tcTotalCount]` | `Temperature.Data` → `[ATC]` | ATC 溫度偏移（ATC≥eATC30+） |
| `iIndexHeatingMode` | `Temperature.Data` → `[Index]` | Index 加熱模式 |
| `fContactHeight[0..24]` | `Contact.Data` | 各 Contact 高度 / 力量 / 模式 |
| `iShuttleMode[2]` | `HandlerCondition.Data` → `[Configuration]` | Shuttle Mode / Cancel |
| `iTrayMapDate[12]` | `HandlerCondition.Data` | Tray Mapping 相關設定 |
| `bART[3]`, `iART` | `Tester.Data` → `[AutoRetest]` | ART 設定（若啟用） |
| `iAutoClean[0..23]` | `HandlerCondition.Data` | Auto Clean 各項參數 |
| `strCleanCnt[X][Y]` | `HandlerCondition.Data` | Auto Clean 次數（各格） |

---

## 4. DoOverWriteSetupFile — 還原本機參數

**目的**：解壓新 Recipe 後，根據 `Security_new.def` 判斷哪些參數要「保留本機」（不被伺服器覆蓋）。

### 4.1 參數保護旗標（AuthPath + "Security_new.def" → [Network]）

| INI Key | 預設值 | 說明 |
|---------|-------|------|
| `Temp Offset` | **true** | true=不保留，false=保留本機溫度偏移 |
| `Index Heat Mode` | **true** | true=不保留，false=保留本機 Index 加熱模式 |
| `Contact High` | **false** | true=不保留，false=**保留本機 Contact Height** |
| `Contact Force` | **true** | true=不保留，false=保留本機 Contact Force |
| `Contact Mode` | **true** | true=不保留，false=保留本機 Contact Mode |
| `HotPlate` | **false** | true=不保留，false=**保留本機 HotPlate 資料** |
| `Load Unload` | **false** | true=不保留，false=**保留本機 ArmCondition** |
| `Speed Setting` | **true** | true=不保留，false=保留本機速度設定 |
| `Shuttle Mode` | **false** | true=不保留，false=**保留本機 Shuttle Mode** |
| `Test Mode` | **false** | true=不保留，false=**保留本機 Test Mode** |
| `Binasgn` | **false** | true=不保留，false=**保留本機 Bin 分配** |
| `BinasgnOff` | **false** | true=不保留，false=**保留本機 BinasgnOff** |
| `Auto Clean` | **true** | true=不保留，false=保留本機 Auto Clean 參數 |
| `Cleaning Count` | 同 Auto Clean | 是否保留清潔次數計數 |
| `Auto Retest` | **true** | (SCK ART 專用) |
| `Bottom 2D Offset` | **false** | 保留本機 Bottom 2D Offset |

> **關鍵規則**：`bNeedCover=false` → **保留本機**（還原）；`bNeedCover=true` → **接受伺服器**（不還原）

### 4.2 Greatek 強制

`CUSTOMER_CODE==CC_Greatek` 時，所有 `bNeedCover` 強制為 `true`（全接受伺服器）。

### 4.3 CC_SCC 保留清單

`CC_SCC` 時，Auto Clean 只保留尺寸相關參數，不保留 Function/SelectArm。

---

## 5. SPIL 下載流程

**觸發條件**：`IniConfig.bSPILFunction==true`

```
1. 清除 DataPath\ 下的舊 Recipe 檔案
2. 執行 HandlerAutoDownload.bat（D:\HT9045\IniData\SPIL\）
3. 讀取 SPIL.txt 解析 Error Code
   - 0 → 下載成功
   - 非 0 → 對應錯誤訊息（1003、1031、1054、1071-1074 等）
4. 驗證 FileListBox1（必須恰好 1 個檔案）
```

---

## 6. UploadToServer 流程

```
btUploadClick → UploadToServer()

Step 1: 刪除舊 zip（DataPath\sULFileName.zip）
Step 2: 建立新 zip
  7z.exe a -tzip "DataPath\sULFileName.zip" "DataPath\sULFileName\*.*"
Step 3: 複製到 Server
  CopyFile(DataPath\sULFileName.zip → IniConfig.sRmsPath\sULFileName.zip)
Step 4: ATC Recipe 上傳（若啟用）
  7z.exe a → 打包 sATCPath 下含 sDLFileName_ 的檔案
  複製到 IniConfig.sRmsPath
Step 5: 清除 zip
Step 6: 顯示結果（MES1689 成功 / WAR1685 失敗）
```

---

## 7. btDownloadClick 前置驗證

```cpp
if(SystemStart || HasICUnderMachine())  return;   // 機台運行中禁止下載

// VTEST：有 IC 在機台 → 設 LastSet.bHasDownloadFile=true 延後下載
if(IniConfig.bVTESTFunction && HasAutoICInMachine())
{
    LastSet.bHasDownloadFile=true;
    return;
}
```

**AMKOR_China / QUALCOMM 特殊處理**：下載後強制開啟全部 Socket Sites。

---

## 8. 溫度 / Device Name 驗證（IniConfig.bCheckFile）

```
edDeviceName 解析溫度 → CutTempToEdit()
  SCC / SCK → 格式如 "XX_125" → 取最後一個 '_' 後的數字
  其他 → 直接解析前 3 字 (ROO=Ambient, 數字=熱測溫度)

驗證邏輯：
  ROO → 需要 LastSet.iTemperature == Tempture_Ambient
  數字 ≤ IniConfig.fAmbientTemp → 需要 Ambient mode
  數字 > fAmbientTemp → 需要 Hot mode + 溫度匹配
```

---

## 9. Recipe 相關目錄結構

```
D:\HT9045\
├── IniData\
│   ├── Data\           ← 主要 Recipe 目錄（各 Setup File 子目錄）
│   ├── DataFTP\        ← TSMC FTP 暫存目錄
│   └── SPIL\           ← SPIL BAT 腳本目錄
├── IniData\Data\[SetupFileName]\
│   ├── ArmCondition.Data
│   ├── Binasgn.Data
│   ├── BinasgnOff.Data
│   ├── Contact.Data
│   ├── HandlerCondition.Data
│   ├── HotPlate.Data
│   ├── Temperature.Data
│   ├── Tester.Data
│   ├── Tray.Data
│   ├── TestMode.Data
│   └── Position Offset.Data  ← 在 OffsetPath 子目錄
└── 7z.exe              ← 壓縮解壓工具
```

---

## 10. ATC Recipe 下載（eNewATCSystem）

**觸發條件**：
```cpp
CosFunction.bUseFTPDownLoadATCRecipe == true &&
ATC_SYSTEM == eNewATCSystem &&
Temperature.bATCActiveCooling == true &&
CosFunction.bHiSiliconFunction == false
```

**下載流程**：
1. `sDLFileName + "ATC_Recipe.zip"` 從 RMS Server 複製
2. 解壓到 `sATCPath`（ATC Recipe 目錄）
3. 若解壓失敗 → `return false`（整個下載失敗）
