//------------------------------------------------------------------------------
// AI(W906-FUSELIMIT) 20261001: golden TfMain::FormShow main.cpp:9535-9551 -- the heater temperature-fuse limit
//   TempFuseLimitType, line for line (RULINGS_20261001 #4, Jimmy 1001 08:4x: "照建議，一切都要翻").
//   Nothing in the port assigned it before, so it stayed at its cmydef.cpp:5604 initialiser 0.0 and its three
//   readers all compared against 0:
//     cprod.cpp:3033 SetCustomerLimitationForConfig (run by every ReadLastSetIni) -- WorkTemperBase+range+10 >= 0
//        always held, so IniConfig.iSocketTemptureRangeOver was rewritten to 0-WorkTemperBase-11 (negative);
//     forms/fHS.cpp:857 -- outside the tri-temp machines every set temperature above 0 was flagged bOverSet;
//     uHeaterThread.cpp:1294 CheckHeater -- the "over the fuse" branch (iAlarmOver165 counter) needed only
//        iSec!=SystemSec to fire.
//   wb_serve calls it where golden has it: after the MyForceDirectories run and right before bHasTrayCSV /
//   bHasPlateCSV (:9553-9554) and InitialHandler (:9559), so the boot ReadLastSetIni (:9564; wb_serve.cpp
//   W906_DoReadLastData) clamps with the real limit.  CUSTOMER_CODE / MachineTypeChoice / iTempLimitation are
//   loaded before that by LoadMachineConfig (database.cpp), as golden loads them before FormShow.
//   ⚠ golden: CC_HONPREC_QC is 0 (MachineType.h:224), the same value as a CUSTOMER_CODE that was never set, and the
//   ReadLastSetIni inside LoadMachineConfig still runs before this with 0.0 (golden database.cpp:336 does the same);
//   both kept verbatim -- the FormShow ReadLastSetIni after this is the one whose result stays.
//------------------------------------------------------------------------------
#include "cmydef.h"

void W906_TfMain_FormShow_TempFuseLimit()
{
    if(CUSTOMER_CODE==CC_HONPREC_QC ||
       CUSTOMER_CODE==CC_EMemory ||                                             //Steven 20250701 : 力旺要求溫度200度
       iTempLimitation==tTemp200)
    {
        TempFuseLimitType=TemperatureFuseLimit250;
    }
    else if(CUSTOMER_CODE==CC_SCC ||                                            //Steven 20140424 : SCC加熱150度 會一直Alarm修正
            MachineTypeChoice==Type_HT9046_LS ||
            iTempLimitation==tTemp150 ||                                        //wei 20150617 改機最高溫150度
            iTempLimitation==tTemp155 ||                                        //Sam 20240118 新增 155度 模式
            iTempLimitation==tTemp175 )                                         //Steven 20210927 : add for 175度
    {
        TempFuseLimitType=TemperatureFuseLimit200;
    }
    else
    {
        TempFuseLimitType=TemperatureFuseLimit170;
    }
}
