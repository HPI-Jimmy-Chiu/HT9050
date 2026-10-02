# 個人出差報告（Personal Travel Report）

## 適用時機
- 使用目前登入者自己的 AD 身分，自動查詢員工資料與近 7 天 CRM 出差/客戶服務回覆紀錄
- 將 API 回傳內容整理成 Markdown 表格
- 使用者明確要求「個人出差報告」、「出差報告」、「CRM 出差資料」、「依 AD 查自己的出差紀錄」時使用

## 強制規則
- **一定要使用自己的AD認證**
- **一定要使用自己的AD認證**
- **一定要使用自己的AD認證**
- 執行時**優先呼叫腳本**：`../../scripts/generate_personal_travel_report.ps1`
- 為了跨環境穩定性，建議固定使用 `powershell.exe -NoProfile -ExecutionPolicy Bypass -File` 方式執行
- 這支腳本採用 ASCII-only 實作，避免 Windows PowerShell 5.1 在不同編碼環境下解析失敗
- 預設流程一律先讀取本機目前登入者的 AD 帳號，不可直接猜測、硬編碼、沿用他人的 `eid`
- 正常模式不可直接指定 `eid` 查詢；必須先用 AD 帳號呼叫 HR API 取得自己的員工工號後，再呼叫 CRM API
- 僅能查詢近 7 天資料；若使用者要求超過近 7 天，必須明確告知 API 會擋住，並要求縮小日期範圍
- JSON request body 不可包含註解

## 例外：測試模式
- 只有在使用者**明確要求測試特定 `eid`** 時，才允許暫時覆寫正常模式，直接使用指定 `eid` 呼叫 CRM API
- 測試模式必須在回覆中明確標示「這是測試，不是個人 AD 流程」
- 測試完成後，若使用者要正式產出個人出差報告，必須回到正常模式

## 腳本入口

優先直接執行腳本：

`../../scripts/generate_personal_travel_report.ps1`

腳本職責：
- 自動讀取目前登入者 AD 帳號
- 正常模式下自動查 HR API 取得自己的 `員工工號`
- 驗證 `startdate` 是否落在近 7 天內
- 呼叫 CRM API
- 直接輸出 Markdown 內容，可顯示於聊天回覆或寫入檔案

## 正常流程

### Step 1. 取得目前登入者 AD 帳號
- 使用本機環境變數或 Windows Identity 取得目前登入者帳號
- 建議使用：`$env:USERDOMAIN`、`$env:USERNAME`
- 範例：`HONPREC\\Maurice`

### Step 2. 呼叫腳本，由腳本用 AD 帳號查 HR API 取得員工資料

API:
`POST https://aiapim.honprec.com/api/hpi/hr/getEmployeeInfo`

Header:
- `apikey: 9c12e022845e6f432f56b308638d59ff`
- `content-type: application/json`

Request body:
```json
{
  "uid": "Maurice",
  "uname": "",
  "uorg": "",
  "udial": "",
  "uextension": ""
}
```

取值規則:
- 從回應 `data[0]` 讀取 `員工工號`
- 若 `data` 為空，流程立即停止，回報「查無員工資料」

### Step 3. 呼叫腳本，由腳本用近 7 天日期查 CRM API

API:
`POST https://aiapim.honprec.com/api/hpi/crm/getCRMDataById`

Header:
- `apikey: 48dfdba900b30d6e9446399b39ecba9d`
- `content-type: application/json`

Request body:
```json
{
  "startdate": "20260414",
  "eid": "21061"
}
```

日期規則:
- `startdate` 格式固定為 `yyyyMMdd`
- `startdate` 必須落在今天往前推 7 天內
- 若今天是 `2026-04-20`，可接受範圍為 `20260414` 到 `20260420`
- 若使用者只說「近 7 天」，預設用今天往前推 6 天作為 `startdate`

### Step 4. 直接顯示腳本輸出的 Markdown 表格

預設輸出欄位:
```markdown
| 序號 | 單據類型 | 回覆單號 | 問題說明 | 問題處理 | 處理人員 | 更換料件 | 客戶需求 | 建立日期 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
```

整理規則:
- 不需要 AI 摘要，除非使用者另外要求
- `問題處理` 內的換行要保留，可轉成 `<br>` 以便 Markdown 表格顯示
- 若 `data` 為空，仍要明確回報「API 成功，但近 7 天內查無資料」

## PowerShell 使用方式

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File '..\..\scripts\generate_personal_travel_report.ps1'
```

指定近 7 天內起始日：

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File '..\..\scripts\generate_personal_travel_report.ps1' -StartDate '20260414'
```

輸出到檔案：

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File '..\..\scripts\generate_personal_travel_report.ps1' -OutputPath '<repo>\public\Docs\weekly\2026\04\20260420\personal-travel-report.md'
```

測試模式（僅使用者明確要求時）：

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File '..\..\scripts\generate_personal_travel_report.ps1' -UseTestEid -TestEid '21098' -StartDate '20260414'
```

## 顯示規則
- reference 本身不直接手寫 API 呼叫結果
- 一律顯示腳本輸出的 Markdown
- 腳本為了跨環境穩定性，Markdown 標題與欄位名稱使用 ASCII；API 回傳的實際內容仍保留原始資料
- 若需要回覆聊天內容，直接貼上腳本輸出的 Markdown 區塊
- 若需要存檔，使用 `-OutputPath` 讓腳本直接寫出 `.md`

## 完成檢查
- 是否優先使用 `../../scripts/generate_personal_travel_report.ps1`
- 是否先取得本機目前登入者的 AD 帳號
- 是否先打 HR API 再取 `員工工號`
- 是否只查近 7 天
- 是否避免使用他人的 `eid` 作為正式流程輸入
- 是否將 CRM 結果整理成完整 Markdown 表格