# 取值之後的本地轉送

入口仍是[GetUPH／UPHStrings](../flow.md)：`MSG_CMD_UPH`與格子字串傳給兩參數 `SendMSG_CMD`；本段不重新宣稱producer／表格生命週期已驗。七完整文字與宣告見[manifest](source-manifest.json)。

## V906 註冊與清除

`forms/fMain.cpp`所選轉送表定義以0初始化；`TfMain::SendMSG_CMD`只有在 `W906_TesterForward.SendMSG_CMD_Msg != 0`才呼叫callback。`FwdSendCmdMsg`又以 `fTesterSide`非空才轉給Handler helper；這兩個本地gate通過仍不代表bridge或接收端可用。

`W906_TesterCommInit`先呼叫 `W906_TesterConnectRulesInstall`，`g_inited`為true時提前return。之後 `W906_CmdServersEnsure`在 `HT9045_TESTERCOMM`字串恰為"0"的opt-out判斷之前；opt-out會提前return，不走本函式後面的callback assignment。此處只讀環境條件，沒有更改環境、建立server或啟動engine。

通過後設 `g_inited=true`、註冊hub factories、必要時配置 `fTesterSide`並Attach；再把 `SendMSG_CMD_Msg`設為 `&FwdSendCmdMsg`。全部init caller／其他installer、helper語意、作用中機台配置與thread時序仍待查，不據函式存在推一定已安裝。

`W906_TesterCommShutdown`以 `!g_inited`提前return；所選後續先以 `W906_TesterForwardTable()`覆寫轉送表，清三個Wnd／bridge hook，再呼叫aux／server／hub shutdown，將 `fTesterSide=NULL`後delete，最後 `g_inited=false`。只記本地順序，不證全部退出路徑或並行生命週期。

## 共用包裝與版本差異

V906 `THandlerTesterSide::SendMSG_CMD`與V912 `TfMain::SendMSG_CMD`共同寫 `HHandler2Gpib.iSendCommand`、清Message，再按 `Message.Length()`拷貝字串。所選body沒有在這一步核對長度上界；資料結構大小／編碼／完整下游仍未驗。

兩版以 `TestIF.iGpibMode==InterfaceType_Delta_Castle || TestIF_File.i2DIDFormat==eAMD`選擇刷新SiteMap／ATCType；它是所選body的runtime欄位條件，不能換稱全部客戶或機型gate。之後配置 `COPYDATASTRUCT`，`dwData=0`、`cbData=sizeof(HHandler2Gpib)`、`lpData`指向iSendCommand，最後delete pcp。

| 版本／function | 本地出口 | 仍待查 |
| --- | --- | --- |
| V906 Handler SendMSG_CMD | `SendToBridge(pcp)` | callback註冊生效、hub／engine／收件端 |
| V912 TfMain SendMSG_CMD | `SendMessage(fMain->HVisionWnd, WM_COPYDATA, ...)` | 視窗／handle、收件端與返回結果 |

V906 `SendToBridge`遇hub／pcp／lpData為0就return；否則依cbData建payload、設 `result=0`後呼叫 `hub->SendToEngine(payload,&result)`，所選body沒有使用回填result。它不直接向呼叫者回傳送達成功；[SendToEngine／mailbox局部](mailbox/index.md)已補本地狀態；engine及接收端仍未讀完，不能推丟失或成功。

回[界線](limits.md)與[consumer樹](../../index.md)。
