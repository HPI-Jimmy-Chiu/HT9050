// ===========================================================================
//  Automation/HanaRms_Prepare_St02.cpp -- THanaRmsMembers::PrepareHANARMSConnect only: golden 912
//  Automation/automation.cpp:3049-3074.  Its golden caller is TfMain::WakeupGPIB (main.cpp:18535-18539), here
//  TesterComm/Handler/HandlerBridgeCtl.cpp (gate G9, lifted by this card) -- the one THanaRmsMembers member another
//  library (ht9045_testercomm_handler) calls, so it has a file of its own (St02 1001 lesson: nm shows one definer).
//  AI(W906-ST02-C10) 20261002 (St02-E helper).  See Automation/HanaRms_St02.h.
// ===========================================================================
#include "MachineDefine.h"      // the tree's include hub (golden automation.cpp:1)
#include "Automation/HanaRms_St02.h"
#include "Automation/HanaRms_Internal_St02.h"
#include "Config.h"             // IniConfig.bA77_EnableHanaRMSInterlock / bA10_6_HANA_ART_TestMode_Enable

#include <cstdlib>              // atoi

using w906hanarms::HANARMSAddLog;

//RogerYang : ready RMS connection before the GPIB program is launched.
//  gated by A76 (HANA RMS interlock enable) ; idempotent (skip if already active).
//  [W906] the comment's "A76 (HANA RMS interlock enable)" is golden's A77 flag below: in 912 the HANA RMS enable moved
//  to [A77] (cConfiguration.cpp, "RogerYang 20260725 : HANA RMS Interlock enable"; A76 became SPIL's
//  bA76AfterHomeNeedAlm) -- comment and code mean the same flag, kept as the code has it.
void THanaRmsMembers::PrepareHANARMSConnect()
{
    if(IniConfig.bA77_EnableHanaRMSInterlock==false)
        return;
    if(IniConfig.bA10_6_HANA_ART_TestMode_Enable==false)                       //RogerYang 20260902 : First Shot 互鎖靠 HANA ART SRQ,沒開先警告
        HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [WARN] ", "A76 on but A10-6(HANA ART) off : first-shot interlock inactive");   // [912 known defect] K7: "A76" text verbatim
    LoadHANARMSSetting();

    if(edtHANARMSIP->Text=="127.0.0.1")
        return;   //still placeholder -> don't auto-connect yet

    W906_HANARMSClientCreate();                                                 // [W906] 1: golden's dfm component, made on first use
    if(HANARMSClient->Active) return;            // 已連/連線中 -> 不重連
    if(bW906HANARMSConnecting) return;           // [W906] 5: RogerYang's comment above ("已連/連線中" = connected OR connecting) -- the polled socket's Active is false while a connect is pending
    try
    {
        HANARMSClient->Address = edtHANARMSIP->Text;
        HANARMSClient->Port    = atoi(edtHANARMSPort->Text.c_str());
        HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [INFO] ", "connecting to "+edtHANARMSIP->Text+":"+edtHANARMSPort->Text);
        bW906HANARMSConnecting = true;                                          // [W906] 5: before Open -- a Sim Open connects inside the call and OnConnect clears it
        HANARMSClient->Open();                                                  // [912 known defect] K1: the only connect (WakeupGPIB); the server drops an idle link after 30 s
    }
    catch(...)
    {
        HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [ERR] ", "connect exception");
    }
}
