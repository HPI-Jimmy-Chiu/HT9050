---
name: gpib-program-manual
description: >
  HT9xxx GPIB Program Manual 程式設計手冊知識庫。
  涵蓋 GPIB 軟體架構、H9046_32GPIB.exe 功能、GPIB 初始化流程、
  RS232 通訊程式架構、GPIB 與 Handler 通訊介面設計。
  關鍵字：GPIB Program Manual, H9046_32GPIB, GPIB 架構, GPIB 初始化,
  RS232 通訊, 32-site GPIB, HT9046, Logic Handler, SRQ flow
---

# SKILL: gpib-program-manual

## 描述

HT9xxx GPIB Program Manual 程式設計手冊知識庫（基於 `GPIB_Program Manual_V12.04.doc`）。
當使用者詢問 GPIB 軟體架構、GPIB 程式設計方法、H9046_32GPIB.exe 功能說明、
RS232 通訊程式架構、GPIB 初始化流程、GPIB 與 Handler 的通訊介面設計，
或需要理解 GPIB9045 程式設計指引時，應載入此 SKILL。

關鍵字：GPIB Program Manual, H9046_32GPIB, GPIB 架構, GPIB 初始化,
RS232 通訊, GPIB 程式設計, Logic Handler, HT9046, 32-site GPIB,
GPIB interface, SRQ flow, test handler GPIB program

## 原始文件

- **路徑**：`d:\GPIB9045\.github\skills\gpib-program-manual\references\GPIB_Program Manual_V12.04.doc`（二進位規格，不在 git：`D:\GPIB9045` 不是 git repo，只在 GPIB9045 工作區）
- **版本**：V12.04
- **格式**：Microsoft Word 97-2003 (.doc)
- **說明**：GPIB 程式設計手冊，描述 H9046_32GPIB.exe 軟體架構與 GPIB/RS232 通訊介面

## 適用範圍

- HT-9xxx 系列 Logic Handler（HT9045, HT9046）
- GPIB 32 Site 通訊程式 (`H9046_32GPIB.exe`)
- BCB6（Borland C++ Builder 6）開發環境
- GPIB 介面卡初始化與驅動程式設定
- RS232 串列通訊設定與流程

## 完整參考文件

> **MD 版本（已轉換 + 程式碼補充）**：
> `.claude/skills/hpi-gpib/references/gpib-program-manual/references/GPIB_Program_Manual_V12.04.md`
>
> 原始 Word 文件：
> `d:\GPIB9045\.github\skills\gpib-program-manual\references\GPIB_Program Manual_V12.04.doc`（二進位規格，不在 git：`D:\GPIB9045` 不是 git repo，只在 GPIB9045 工作區）

## 主要主題索引

> 詳細規格請參閱：`.claude/skills/hpi-gpib/references/gpib-program-manual/references/GPIB_Program_Manual_V12.04.md`

### 資料結構

| 結構 | 方向 | 說明 |
|------|------|------|
| `VM` | GPIB → Handler | `iCommand`, `Result[32]`, `bError`, `bEchoStop`, `cReturn[256]`, `GpibData[256]`, `GPIBBin`, `bOneCycle` |
| `MV` | Handler → GPIB | `iSendCommand`, `Site[32]`, `bSimulate`, `bSupport32Bin`, `bCloseGpib`, `bTimeOutProcess`, `GpibAddress`, `MachineISRun`, `IsTest`, `bGpibMode`, `iLotStatus`, `HandlerHwnd`, `GpibHwnd`, `GPIBBin`, `iStatus[17]`, `Message[2048]`, `UseSiteMapData[256]`, `asATC_TYPE[32]`, `MultiMessage[4096]` |

> **重要**：VM 與 MV 結構只能在末端新增成員，Handler / GPIB / RS232 三端須同步修改並同步版號。

### 使用函式對照

| 函式 | 方向 | 說明 |
|------|------|------|
| 函式一：`RunTestProgram(true, flag2)` | H→G | 啟動測試，傳 MV（MSG_CMD_NONE）|
| 函式二：`SendMSG_CMD(int CMD)` | H→G | 傳送無附加資料的 Handler→GPIB 指令 |
| 函式三：`SendMSG_CMD(int CMD, AnsiString Msg)` | H→G | 傳送含字串資料（填入 `Message[2048]`）|
| 函式四：`SendMSG_CMD_DeviceMapSRQ(int iStatus)` | H→G | DeviceMap SRQ 狀態，附帶 `iLotStatus` |
| 函式五：`SendMSG_TestMode()` | H→G | 傳送 GPIB 模式（GPIBBin = iGpibMode）|
| 函式六：`SendCaptureFinish()` | G→H | BIN 分類完成通知（MSG_CMD_NONE + Result[32]）|
| 函式七：`SendECHO` | G→H | ECHO 響應（Result[0..3]='E','C','H','O'）|
| 函式八：`SendMSG_CMD(int CMD)` | G→H | 傳送無附加資料的 GPIB→Handler 通知 |
| 函式九：`SendMSG_CMD(int CMD, AnsiString Msg)` | G→H | 傳送含字串（填入 `cReturn[256]`）|

### MSG_CMD 代碼分類

| 分類 | 代表指令 |
|------|---------|
| 基礎測試 | MSG_CMD_NONE(0)、MSG_CMD_ECHONG(9)、MSG_CMD_ESC(96)、MSG_CMD_RESUME(97) |
| 機台動作 | MSG_CMD_Arm1Down(16)、MSG_CMD_Arm2Down(17)、MSG_CMD_ContactTestArm1/2(18/19)、MSG_CMD_TimeOut*(11-13)、MSG_CMD_HandlerHome*(14/15) |
| 取得狀態 | MSG_CMD_GetTestArmPos(71)、MSG_CMD_GetSiteOnOff(83)、MSG_CMD_GetNowAllTemp(69) |
| 控制 GPIB | MSG_CMD_CloseGpib(21)、MSG_CMD_Version(24)、MSG_CMD_HandlerID(62)、MSG_CMD_EnableBarCode(31) |
| 設定參數 | MSG_CMD_SetTemp(73)、MSG_CMD_SetSoakTime(74)、MSG_CMD_BinMap(67)、MSG_CMD_OverDrive(35) |
| ART 專用 | MSG_CMD_SCKART_*(43-50)、MSG_CMD_LotStatus(22)、MSG_CMD_RetestFlag(28)、MSG_CMD_RCMD(25) |
| 2D Barcode | MSG_CMD_2DIDFormat(93)、MSG_CMD_EnableBarCode(31)、MSG_CMD_READYNEXTSHOT(165) |
| SIGURD | MSG_CMD_SIGURD_CHKSTATUS(133)、MSG_CMD_SGSETUP(139)、MSG_CMD_CHECKLIST(141)、MSG_CMD_SetBINCOUNT(146) |
| 三星格式 | MSG_CMD_SamSung_Tmp(89)、MSG_CMD_SamSung_Map(90)、MSG_CMD_SamSung_Soak(91) |

### eTestMode 測試模式

SingleSite(0) / DualSite(1) / TriSite1X3(2) / QualSite1X4(3) / QualSite2X2(5) / _6Site2X3(7) / _8Site2X4(9) / _16Site2X8(12) / _32Site4X8N(14)

## 與 gpib-command-list SKILL 的關係

| 面向 | gpib-program-manual | gpib-command-list |
|------|--------------------|--------------------|
| 內容 | 程式架構、IPC 結構、MSG_CMD 實作 | GPIB 指令格式與通訊規格 |
| 使用時機 | 修改 Handler↔GPIB IPC 邏輯、新增 MSG_CMD | 查詢 GPIB 指令語法、BIN 排列 |
| 層次 | 實作層（How to program）| 規格層（What commands）|

---

## 搬移說明（St02 20260926）

本 skill 是從 `D:\GPIB9045\.github\skills\gpib-program-manual` 複製進 `D:\HT9045\.claude\skills` 的（原處保留）。
下列二進位檔（客戶／廠商文件、手冊、截圖）**沒有一起搬進版控**：main 會同步到 GitHub。要看原檔請到原處：

- `references/GPIB_Program Manual_V12.04.doc`
