# EP當前main與原移植筆記

20261006唯讀核對main `84233b648`，未運行程式、載入DLL或接觸機台。

- `MachineType.h`仍為註解的`//#define W906_ADAM_EP_LIVE`。`Adam6024Integrate_St02.cpp::W906_AdamEpLive`預設回編譯值，另有`W906_AdamEpLive_SetForTest` override；不能由原資料的OFF結論推每個測試／機台build都OFF。
- 原!114 helper與接入資料保留；把「存在程式」、「gate允許」與「機台有成功讀寫」分三層，不開gate或接手A1b。
- 原資料早期寫「906缺F4／F15防呆」，其後已有更正。現在`adam6024.cpp::TransformFuntion`仍包含[912]筆數檢查與k<8：DieForceOneByOneSLKClass、SLKIndClass的8／16通道迴圈已可看到對應guard。
- 原!135「還沒進main」是20261003筆記。目前`Adam6024Pressure_St02.cpp::ADAM_ReturnValueCheck`的0.8／5.2界線已明確cast成double，檔頭及行內保留A3字面值精度說明；不再把原待合狀態當今日狀態。
- `atester.cpp::CheckAndRecodrEP`的golden完整本體仍在#if0保存，活函式return false。ADAM／EP helper與Index記錄／告警流程接入不能混為已完成。

## 查證來源

- [Integrate helper](../../../../../HT9011UC_Cpp_V3.33.906.0/Adam6024Integrate_St02.cpp)：live／test override與入口。
- [Pressure helper](../../../../../HT9011UC_Cpp_V3.33.906.0/Adam6024Pressure_St02.cpp)：ReturnValueCheck、換算與客戶條件。
- [TransformFuntion](../../../../../HT9011UC_Cpp_V3.33.906.0/adam6024.cpp)：[912] F4／F15與範圍檢查。
- [Index本體](../../../../../HT9011UC_Cpp_V3.33.906.0/atester.cpp)：CheckAndRecodrEP保存／live pair。
- [原移植紀錄與裁決](../adam/original-entry.md)。
