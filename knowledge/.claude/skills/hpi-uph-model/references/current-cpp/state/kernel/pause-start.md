# ShowRunLabel 的暫停開始寫入

來源：[V906 ckernel.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/938ebc37e95314ad496b03c7c257e1abc5fb9799/HT9011UC_Cpp_V3.33.906.0/ckernel.cpp)、[V912 ckernel.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/938ebc37e95314ad496b03c7c257e1abc5fb9799/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/ckernel.cpp)；pin `938ebc37e95314ad496b03c7c257e1abc5fb9799`。定位ShowRunLabel的SystemStart、CheckCanChangeRealDummy、tUPH_PauseStartTime與bCalculatePauseTime。

已讀SystemStart的if／else接合並以所選body的brace配對核對：這處writer在SystemStart為false的else內。SystemStart真分支的完整內容、其所有writer及UI helper語意尚未讀完。

此else依序先判斷RUN CHECK（SECS Remote Start／ChipMos_ZHUBEI Start Control／SECS Pause）、Decay Test、Auto Clean Ion Fan與MAGAZINE Remove The Tray；前面任一分支命中，本輪不進入後面的這處PAUSE分支。條件式仍以來源中的IniConfig、CUSTOMER_CODE、bPhysicalStart、bSECSPause與iMagazineStatus為準，未推定現場設定。

PAUSE分支的局部條件為：

```cpp
fMain->CheckCanChangeRealDummy()==false ||
(HasICUnderMachine() && HasAnyICInMachine() &&
 CUSTOMER_CODE==CC_ASE_KaohSiung)
```

第一項不限這個ASE客戶碼；第二項需要三項共同成立。這是所選條件式的讀法，CheckCanChangeRealDummy／兩個HasIC helper的完整判斷與作用中客戶仍未驗。

進入此分支後，只有bCalculatePauseTime為false，才執行tUPH_PauseStartTime=Now()並設bCalculatePauseTime=true；若已為true，本處不覆寫開始值。PAUSE狀態顯示在這個旗標判斷之後，不能以顯示字串當作本次已更新開始時間的證明。

本處沒有累加tUPH_PauseTime或重置UPH全部狀態。累加與清旗標的已讀區段另見 [MainProc](../pause.md)，elapsed扣除另見 [CalculateUPH](../../calculate.md)。串接順序、所有寫者／caller／thread、clock有效性與每次pause入帳次數仍未閉合。
