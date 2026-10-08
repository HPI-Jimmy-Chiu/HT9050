# Bridge 設定同步與 close request

定位 `TesterComm/Handler/HandlerBridgeCtl.cpp` 的 `SyncBridgeSettings`、`CloseGpibProgram`；兩個完整定義見 [manifest](source-manifest.json)。

## SyncBridgeSettings

此 body 以 `TestIF_File`、`LastSet.iTester`、先前值及 socket 狀態分流；caller 的 InitialOK／SystemStart guard 見 [tick](tick.md)，不是本函式自帶的通用保護。

| 分流 | 已讀 body 的行為 | 查證界線 |
|---|---|---|
| `iGpibMode == InterfaceType_SPEA_Type` | SPEA address／mode／pass-bin 舊段放在 `#if 0`，目前只消除 unused 並清 sOSRecipe／b2DID | 被編譯排除的段不能當已實作 |
| `iTestType == TCP_IP_MODE && iTester == ON_LINE` | recipe 名或 barcode 狀態改變時複製 recipe，依序送 WORKFILE、GETOSSETUP、SET2DID；四次 `MySleep(100)` | TCP callees、睡眠實作與實際耗時未驗 |
| 其他分流，address／tester／bin 數／HANA ART flag 變化 | bin 數變化先更新 bin UI；`UseSiteHasIC()==false` 才走 bridge message、OCR/barcode/pin、2DID、HANA ART 與 FTP 命令 | UI 與傳輸 callees 未全部讀完 |
| RS232 mode 改變 | `!OffLineGpibWay()` 且 RS232 mode 不同，送 TestMode，再讀 RS232 Setup.ini 的 iTesterMode | 此處 helper 與 INI 是否補寫預設仍待查 |
| TTL mode 改變 | 同樣排除 OffLineGpibWay，TTL_CARD_TYPE 2／3，mode 不同且 `W906_FormFShow` 為 false | web 視窗狀態與 golden 註解分開 |
| GPIB mode 改變 | GPIB 或 OffLineGpibWay 且 mode 不同，送 TestMode，再讀 GPIB general.ini 的 iTesterMode | 不代表全部模式已閉合 |

其他分流中，bin UI 更新在 socket 無 IC 的 guard 之前；不能說 socket 有 IC 時整段完全沒有作用。OCR／BAR_CODE_INSTALL、CosFunction.bFTPFunction 等還有 feature guard。

2DID body 選 eAMD／eIntel／eStandard 後直接送 MSG_CMD_2DIDFormat；原註解提到移除 Multi2D，是保存的移植史料，並非本輪重驗 V912 的結論。

## CloseGpibProgram

- `bFind==false` 早退；`LastSet.iRunStartMode==rsmQAMode && bQAModeFinishCleanOut` 也早退。
- 通過後組 COPYDATASTRUCT，暫設 bCloseGpib，MTI／PTI 客戶另將 iLotStatus 設 0，送 MSG_CMD_CloseGpib；cbData 使用 `sizeof(HHandler2Gpib)`，lpData 指 iSendCommand。
- 呼叫 `SendToBridge` 後刪 pcp、清 bCloseGpib、記 MyDBIProcess，將 WakeupGPIBdelay 設為 5 秒並清 bFind。

本 body 未檢查 SendToBridge 的成功結果，也沒有在此呼叫 Hub::Restart 或 Aux::CloseTesterComm。它建立 close request 與後續 delay 狀態；實際送達、關閉、喚醒與 relaunch 仍需 caller／callee 證據。

`sizeof` 與欄位地址只是已讀語句，不能據此宣稱各版本 struct layout、容量或 ABI 已相容。
