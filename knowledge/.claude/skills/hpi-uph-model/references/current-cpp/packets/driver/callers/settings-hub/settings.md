# 設定發布與 packed framing

定位 HandlerTesterSide.cpp 的 PublishSettings 與 HandlerSettings.h；原文見 [manifest](source-manifest.json)。header 的 20260926 裁決註解是保留來源，本次未重新裁決。

| function／變數 | 選讀行為 | 口徑 |
| --- | --- | --- |
| PublishSettings／HsArtTesterType | 發布 fSCKART 存在時的 iTesterType，否則 -1 | 品牌欄位，不是 site／容量／UPH |
| PublishSettings／HsUseGPIBFormat | 發布 IniConfig.iI25UseGPIBFormat | 與 ART 是兩個 store，沒有共同 epoch／交易 |
| 三個 Hs* atomic | 各自 function-local static atomic<int>，初始 -1 | 單欄位 atomic 不代表整批快照同一時刻；consumer 的回退待續 |
| HsPackAuxFraming | baud 1..0x7fffff、byteSize 0..3、stopBits 0..2、parity 0..4；超出返回 -1 | framing 數值 pack，不是 MessageDef ABI |
| pack 位元 | baud 左移 8、byteSize 左移 5、stopBits 左移 3，與 parity OR | W906_Rs232* ordinal 映射未在本層完整選讀 |
| HsUnpackAuxFraming | v<0 返回 false，其餘解位元並 true | body 沒有 pointer null guard 或完整 ordinal 重驗 |

[StartBridgeProgram](../init-selection/selection.md) 先 PublishSettings，再判斷是否 Start／Restart。原註解提及每 tick／品牌學習後亦呼叫；本層未核對全部 caller 或 engine 讀取時序，不能因 store 成功便認定外部 Tester 就緒。

回 [索引](index.md)、[Aux](recipe.md)、[界線](limits.md)。
