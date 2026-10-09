// ===========================================================================
//  Automation/HanaRms_Prepare_St02.cpp -- THanaRmsMembers::PrepareHANARMSConnect only: golden 912
//  Automation/automation.cpp:3049-3074.  Its golden caller is TfMain::WakeupGPIB (main.cpp:18535-18539), here
//  TesterComm/Handler/HandlerBridgeCtl.cpp (gate G9, lifted by this card) -- the one THanaRmsMembers member another
//  library (ht9045_testercomm_handler) calls, so it has a file of its own (St02 1001 lesson: nm shows one definer).
//  AI(W906-ST02-C10) 20261002 (St02-E helper).  See Automation/HanaRms_St02.h.
//  AI(W906-W195) 20261009 (St02-E) H2: the body is golden 913 automation.cpp:3237-3246 (RogerYang 20260916, connect on
//  demand): it only calls HANARMSConnect (HanaRms_St02.cpp), which loads the ini, checks IP / Port and opens.
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
        HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [WARN] ", "A76 on but A10-6(HANA ART) off : first-shot interlock inactive");   // K7: "A76" text verbatim (golden 913 :3242 still says A76)
    //RogerYang 20260916 : 開機先連一次僅為驗證伺服器可達;
    //  實際查詢一律隨需重連(伺服器 30 秒 idle 就會把連線關掉)
    HANARMSConnect();                                                           // AI(W906-W195) 20261009 H2: golden 913 automation.cpp:3243-3245 (912's load + 127.0.0.1 placeholder test + Active test + Open moved into HANARMSConnect, HanaRms_St02.cpp; K1 retired)
}
