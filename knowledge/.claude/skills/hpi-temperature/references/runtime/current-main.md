# 目前main與歷史溫控筆記的對照

20261006唯讀核對main `84233b648`；這是程式接入證據，未啟動程式或確認實機溫控成功。原20260927／20261001缺口清單留作歷史原文。

## 今日已核對的更新

- `MachineType.h`預設定義`W906_FASTCLK_HEATER`。`FastClockWbServe.cpp`的`Start`註冊20 ms heater job並設owns flag；`WebBridgeTags.cpp`的PumpTick在fast clock取得heater後略過重複呼叫。
- `FastClockJobs.cpp::W906_FastClockHeaterBeat`在SIM呼叫`W906_HeaterSimTick`，其中沒有`DoThermo`；SHIP在`InitialOK`後依序跑CheckATC6System、DoThermo、HeaterDoorIsOpen、CheckHeater、DoHeaterOn。因此原「DoThermo只有未啟動執行緒caller／SHIP沒有人判斷加熱」已不適用於開啟fast clock的今日程式路徑。
- `THeaterThread::Resume`仍為空；這不表示上述fast clock也沒有執行。啟用flag、實際serve-loop接入、InitialOK及控制器配置要分別確認。
- `rs232.cpp::W906_Comm2DfmBoot`由TCOM2Shim建構子呼叫，設定`g_pCOM2Comm2`與`g_pDTKComm`為Comm2。原「g_pDTKComm初始化null所以永遠沒接」已過時。
- `W906_RS232InitTempCheck/Open`已接入RS232Init；`DoThermo`經`W906_PumpTempComm2`消費收件佇列。開埠失敗的IsSimMode會走記錄／port error；有接線不等於成功開實體埠。

## 尚不能直接推定的事

未重驗所有控制器、EJ1N／DTM逐通道、ATC、fTemperFrom顯示tag、所有gate及原疑點修復狀態。舊「20個#if 0」或null總數不當作今日計數；需對具體路徑重新查程式。Q34／Q71～76的頁面／設定裁決與底層實作仍分開。

## 可重定位來源

- [FastClockJobs](../../../../../HT9011UC_Cpp_V3.33.906.0/FastClockJobs.cpp)：W906_FastClockHeaterBeat。
- [FastClockWbServe](../../../../../HT9011UC_Cpp_V3.33.906.0/FastClockWbServe.cpp)：Start／heater20ms。
- [HeaterSimTick](../../../../../HT9011UC_Cpp_V3.33.906.0/HeaterSimTick.cpp)：SIM本體。
- [rs232](../../../../../HT9011UC_Cpp_V3.33.906.0/rs232.cpp)：TCOM2Shim建構子、W906_Comm2DfmBoot、TempCheck/Open、W906_PumpTempComm2。
- [bthermo](../../../../../HT9011UC_Cpp_V3.33.906.0/bthermo.cpp)：DoThermo／DoThermoReal。
- [原移植現況與方案](../controllers/configuration/references/port-status-and-plans.md)／[原迴圈知識](../core/references/port-thermo-loop.md)。
