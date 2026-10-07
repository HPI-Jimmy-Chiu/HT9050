# 派工給 Jimmy：IO 頁「面板」分頁（Front／Rear）程式碼補齊（2026-10-07）

EastSun 1007：「請派工給 jimmy 把這個頁面 程式碼 補齊」（附圖：IO 頁 Front／Rear 兩塊面板，Power Off／On、Front、Reset、Pause、Home、Start、One Cycle、Retry、Skip、Clean Out、Tray Feed、Tray End、Alarm Reset、Front/Rear、Step、T. Start，左下 Pad 按鈕）。

## 頁面在哪

- 網頁：`web/page/HW.IoSetView.html`（面板分頁；按鈕是 `TBtnPanelLane`，Alias＝`SwFK*`（前）／`SwRK*`（後）燈號，上方小方塊＝`SnFK*`／`SnRK*` 按鍵輸入，`SwFrontActiveLed`／`SwRearActiveLed`、`SwRKManualStep`、`SwRKManualTStart`）。
- golden：`Tfiosetview::BtnPanelClick`（iosetview.cpp:1030-1206），其中 `fPadInterface->SendSwitchStatus(Ptr)`（:1095）；Pad 按鈕＝開 `TfPadInterface`（uPadInterface）。

## 目前缺什麼（機台樹）

1. **按燈號沒有送到實體面板**：`JsonBridge/IoBtnPanelClick.cpp` 檔頭 DEVIATION (a)「SendSwitchStatus: fPadInterface is not ported in this tree」，該處（:429）沒有呼叫。機台樹的 `PadInterface_St02.cpp` 已有 `SendSwitchStatus(...)`（RS-232 面板，`iControlPanelMode==1`），請接上 golden :1095 的那一行。
2. **面板燈號／按鍵列不是 1203 輸出**：`io.btnPanelClick` 走的是 ISABase==ePCI1203 分支；`SwFK*`／`SwRK*` 是通訊面板的燈（PadItem，`PadInterface_St02.h:155-186` 對照表），需要 golden 的面板分支（Down 切換 → SW[] → SendSwitchStatus），不要走 MyLaneIO 寫卡。
3. **按鍵小方塊的狀態**：請確認 `SnFK*`／`SnRK*` 在網頁上顯示的是 `DoScanPanelLed` 設的 `PadItem[i].mlEvent->Value`（golden `TMySensor::Status` 在 `iControlPanelMode==1` 讀 `ProcessScanKey`，`mysensor.cpp:73-77`）。
4. **Pad 按鈕**：golden 開 `TfPadInterface`（通訊紀錄／版本／前後面板切換）；網頁目前沒有對應畫面，請補或給一個等效頁。
5. 註：`tools/wb_serve.cpp:7256` 還留著「uPadInterface 未移植（mysensor.cpp:119 同一個閘）」的舊註解，補完時一併更正。

## 實機狀況（給你參考，2026-10-07 18:20 實測）

- 面板通訊是好的：18:20:14～18:21:21 按實體 HOME，oplog 收到 `PADKEY t050400002020`（HOME 0x20）／`t050400002000`（放開）。
- 每一筆都帶 **0x2000（ALARM RESET 一直按著）**＝ALARM RESET 鈕卡住或短路（硬體待查）。機台樹 72b2f1a（RSTHELD）已改成：卡住的 ALARM RESET 不再蓋掉其他鍵。
- 開機時後面板送 `t051400004000`（SafeLock 0x4000）。
- 機台樹 1f92a63（PADNOTE）：面板送來的每一筆非燈號封包都即時寫 oplog「PADKEY」（面板紀錄要滿 1000 行才落地）。

## 規則

- 照 golden；偏離要在檔頭寫 DEVIATION。
- 不要碰安全 PLC 的寫入（只讀 FC4）。
- 測試：tests/test_st02_pad_interface.cpp、tests/test_scankey_golden.cpp 要照過。
