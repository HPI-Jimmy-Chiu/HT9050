# W3 硬體物件層 9 支檔：被引用的函式（nm 盤點）—— 20260924 夜間

> 產生：`python tools/w3_caller_census.py build_b1dbg`（AI(W906-W3-CENSUS)）。對**實際編出來的** .obj 問 nm：目標檔定義的函式 × 其他 .obj 的未定義符號。
> ⚠ 這是「符號存在」第一級（能編能連）：被引用 ≠ 執行期會走到；也看不到「被 stub 先滿足」。W3 解閘時每一個仍要量第三級（讀寫真的有效果）。
> ⚠ `mykitsuck.cpp` **刻意沒註冊**（`CMakeLists.txt:2723`）：它與 `aHotPlateSubstrate.h:106/:371` 各有一份 `TMySucker`／`TMyKitSuck`、版面不同，
>   同時編進來是 ODR（memory「兩個TMyKitSuck ODR」）。在用的是 aHotPlateSubstrate 那一份（A4-6 的真空／吸嘴格數空殼在那裡）⇒ W3 的 mykitsuck = 把 golden 本體併進在用的那一份，要單獨一波做。

```
==============================================================================
MyLaneIo（CMakeFiles\ht9045_io.dir\MyLaneIo.cpp.obj）: 定義 30 個函式，被其他檔引用 8 個
  [  2] TLaneIO::IOInputBit(int, int, int, int, int, vclcompat::AnsiString)    mycylin, mysensor
  [  2] TLaneIO::IOOutBitStatus(int, int, int, int, int, vclcompat::AnsiString mycylin, myswitch
  [  2] TLaneIO::IOBitOn(int, int, int, int, int, vclcompat::AnsiString)       mycylin, myswitch
  [  2] TLaneIO::IOBitOff(int, int, int, int, int, vclcompat::AnsiString)      mycylin, myswitch
  [  1] TLaneIO::GetIOValue(int, int, int, int, int, vclcompat::AnsiString)    MyVacuumPanel
  [  1] TLaneIO::GetIOValueThread(int, int, int, int, int, vclcompat::AnsiStri MyVacuumPanel
  [  1] TLaneIO::SetIOValueThread(double, int, int, int, int, int, vclcompat:: MyVacuumPanel
  [  1] TLaneIO::SetUseIP(int, int, int, vclcompat::AnsiString, bool)          cinitial
==============================================================================
myio（CMakeFiles\ht9045_io.dir\myio.cpp.obj）: 定義 16 個函式，被其他檔引用 0 個
==============================================================================
mysensor（CMakeFiles\ht9045_io.dir\mysensor.cpp.obj）: 定義 17 個函式，被其他檔引用 4 個
  [ 55] TMySensor::IsOff()                                                     AGV_E84, AGV_PortScan, AutoRetest, Command, OCRInsp …
  [ 46] TMySensor::IsOn()                                                      AGV_E84, AGV_PortScan, AutoClean, AutoRetest, BarCode_Bottom2DID …
  [  1] CopySensor(TMySensor*, TMySensor*)                                     cinitial
  [  1] TMySensor::Status()                                                    cSensorScan
==============================================================================
myswitch（CMakeFiles\ht9045_io.dir\myswitch.cpp.obj）: 定義 19 個函式，被其他檔引用 5 個
  [ 47] TMySwitch::On()                                                        AGV_E84, AutoClean, TriTemp, aTester_Front, aTester_Rear …
  [ 24] TMySwitch::Off()                                                       AGV_E84, AutoClean, PowerSavingMode, TriTemp, WebStart …
  [ 11] TMySwitch::Status()                                                    AGV_E84, AGV_PortScan, TriTemp, acatchtray, asendic …
  [  3] TMySwitch::OnOff(bool)                                                 cinitial, ckernel, csystem
  [  1] CopySwitch(TMySwitch*, TMySwitch*)                                     cinitial
==============================================================================
mycylin（CMakeFiles\ht9045_io.dir\mycylin.cpp.obj）: 定義 99 個函式，被其他檔引用 18 個
  [ 31] TMyCylinder::Off()                                                     AutoClean, AutoRetest, OCRInsp, TriTemp, aRotateKIT_In …
  [ 26] TMyCylinder::On()                                                      AutoRetest, OCRInsp, TriTemp, aRotateKIT_In, aRotateKIT_Out …
  [ 15] TMyCylinder::Pop()                                                     AutoRetest, OCRInsp, TriTemp, aRotateKIT_In, aRotateKIT_Out …
  [ 15] TMyCylinder::Push()                                                    AutoRetest, aRotateKIT_In, aRotateKIT_Out, acatchtray, ainarm9045 …
  [ 13] TMyCylinder::OffStatus()                                               AGV_E84, aTester_Front, aTester_Rear, acatchtray, asendic …
  [ 10] TMyCylinder::OnSensor()                                                OCRInsp, WebStart, aTester_Front, aTester_Rear, acatchtray …
  [  7] TMyCylinder::OnStatus()                                                asendic, asendic_Color, asendic_Empty, asendic_Loader_RT, cInArmPlacement …
  [  7] TMyCylinder::OffSensor()                                               OCRInsp, acatchtray, aoutarm9045, csystem, fTeach …
  [  5] TMyCylinder::GetOutBit()                                               OCRInsp, TriTemp, acatchtray, ainarm2, csystem
  [  1] TMyCylinder::AddOffTime(double)                                        cStartCondition
  [  1] TMyCylinder::GetOffTime()                                              cStartCondition
  [  1] TMyCylinder::GetOnTimeAvg()                                            cStartCondition
  [  1] TMyCylinder::GetOffTimeAvg()                                           cStartCondition
  [  1] TMyCylinder::GetOnTimeAlarm()                                          cStartCondition
  [  1] TMyCylinder::GetOffTimeAlarm()                                         cStartCondition
  [  1] TMyCylinder::Reset()                                                   acatchtray
  [  1] TMyCylinder::AddOnTime(double)                                         cStartCondition
  [  1] TMyCylinder::GetOnTime()                                               cStartCondition
==============================================================================
mykitsuck: NOT COMPILED in build_b1dbg（沒有 .obj —— 這支檔不在任何建置目標裡）　⚠ **20260926 已過期**：A4-6（8ff6c754）合一後 mykitsuck 已編進 ht9045_sm，重量結果（被引用 68 個、0 個閘）在 `W3_PROGRESS.md` §15
==============================================================================
mytray（CMakeFiles\ht9045_motor.dir\mytray.cpp.obj）: 定義 31 個函式，被其他檔引用 17 個
  [ 40] TMyTray::FullIC()                                                      AGV_PortScan, SortingBinTray, aRotateKIT_In, aRotateKIT_Out, acatchtray …
  [ 38] TMyTray::HasIC()                                                       aRotateKIT_In, aRotateKIT_Out, acatchtray, ainarm9045, ainarm9045S_1x4_4 …
  [  7] TMyTray::HowManyIC()                                                   acatchtray, ainarm9045, ainarm_SearchPickPlate, ainarm_SearchPlacePlate, asendic_Loader …
  [  5] TMyTray::HasRealIC()                                                   AutoRetest, SortingBinTray, aoutarm9045, csystem, mymotor
  [  4] TMyTray::SetXYItem(int, int)                                           AutoClean, cinitial, csystem, mymotor
  [  3] TMyTray::ClearData()                                                   AutoClean, csystem, mymotor
  [  2] TMyTray::HasDataIC(int)                                                AutoClean, asendic_Auto
  [  1] TMyTray::SetBlockXYItem(int, int)                                      cinitial
  [  1] TMyTray::ReadUnloaderInfo(vclcompat::AnsiString)                       cinitial
  [  1] TMyTray::SaveUnloaderInfo(vclcompat::AnsiString)                       cinitial
  [  1] TMyTray::HowManyICInBuffer(int)                                        acatchtray
  [  1] TMyTray::HowManyBinICInTray(int)                                       asendic_Auto
  [  1] TMyTray::HasOnlyDataICAndNullIC(int)                                   mymotor
  [  1] TMyTray::SetData(int)                                                  mymotor
  [  1] TMyTray::HasOCRIC()                                                    OCRInsp
  [  1] TMyTray::TMyTray()                                                     mymotor
  [  1] TMyTray::~TMyTray()                                                    mymotor
==============================================================================
myTimer（CMakeFiles\ht9045_globals.dir\myTimer.cpp.obj）: 定義 16 個函式，被其他檔引用 9 個
  [116] TQPF_Timer::Off()                                                      AGV_E84, AGV_PortScan, ATCSystem, AutoClean, AutoRetest …
  [103] TQPF_Timer::SetSecAndOn(double)                                        AGV_E84, AGV_PortScan, ATCSystem, AutoClean, AutoRetest …
  [ 72] @_ZN10TQPF_TimerC1Ev@4                                                 AGV_E84, AGV_PortScan, ATCSystem, AutoClean, AutoRetest …
  [ 38] TQPF_Timer::LatchCycleTime(bool)                                       TesterTCP_Socket, WebStart, aTester_Front, aTester_Rear, ainarm9045 …
  [ 34] TQPF_Timer::SetMSAndOn(unsigned long)                                  AGV_PortScan, AutoClean, BarCode_Bottom2DID, BarCode_Bottom2DID8CCD, BarCode_Shuttle1_CCDScan …
  [ 16] TQPF_Timer::Set0_1SecAndOn(double)                                     AutoClean, MyCCLink, MyCCLinkBoard, WebStart, aTester_Front …
  [  6] TQPF_Timer::LatchCycleTimeSec(bool)                                    TriTemp, acatchtray, asendic, atester_ProcessCount, cMyDB …
  [  2] TQPF_Timer::SetSec(unsigned long)                                      aTester_Front, aTester_Rear
  [  1] TQPF_Timer::On()                                                       asendic_Auto
==============================================================================
MyTempPanel（CMakeFiles\ht9045_sm.dir\MyTempPanel.cpp.obj）: 定義 45 個函式，被其他檔引用 6 個
  [  1] TMyTempPanel::SetCaption(vclcompat::AnsiString)                        uTemp_Set
  [  1] TMyTempPanel::SetIndexTag(int)                                         uTemp_Set
  [  1] TMyTempPanel::SetEnable(bool)                                          uTemp_Set
  [  1] TMyTempPanel::SetParent(vclcompat::TTabSheet*)                         uTemp_Set
  [  1] TMyTempPanel::TMyTempPanel(vclcompat::AnsiString, int)                 uTemp_Set
  [  1] TMyTempPanel::~TMyTempPanel()                                          uTemp_Set
```
