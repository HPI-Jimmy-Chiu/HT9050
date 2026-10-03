// ===========================================================================
//  Automation/HanaRmsMembers_St02.cpp -- THanaRmsMembers ctor / dtor and the real-socket policy flag.
//  AI(W906-ST02-C10) 20261002 (St02-E helper).  See Automation/HanaRms_St02.h.
//
//  Deliberately free of socket code: atester_shims.cpp:370 `new TfAutomationShim()` references this ctor / dtor, so
//  every program that links atester_shims.cpp.obj extracts THIS object.  It must not name TClientSocket (that would
//  pull vclcompat/ClientSocket.cpp and its WinSock imports into programs that never touch HANA RMS).  The dtor frees
//  the socket through W906_pfnHANARMSClientFree, set by the code that made it (HanaRms_St02.cpp).
// ===========================================================================
#include "Automation/HanaRms_St02.h"
#include "Automation/HanaRms_Internal_St02.h"

#include <cstddef>   // NULL

namespace {
bool g_bW906HanaRmsRealSocket = false;   // [W906] 1: default Sim; only the composition root sets it (TesterCommWiring.cpp)
}

void W906_HanaRmsSetRealSocket(bool bReal) { g_bW906HanaRmsRealSocket = bReal; }   // [W906] 1: read when the socket is made (W906_HANARMSClientCreate)
bool w906hanarms::RealSocket() { return g_bW906HanaRmsRealSocket; }

// golden 912 automation.cpp:58-150 TfAutomation::TfAutomation -- only its HANA RMS lines (:148-149) and what the dfm gives
// the RMS widgets (automation.dfm:471-610 / :765-775).  The rest of golden's ctor is the OLP form (fAutomationEngine).
THanaRmsMembers::THanaRmsMembers()
{
    mmoHANARMS       = new TMemo();                   // dfm :471 (no Lines)
    edtHANARMSIP     = new TEdit();
    edtHANARMSIP->Text = "127.0.0.1";                 // dfm :555
    edtHANARMSPort   = new TEdit();
    edtHANARMSPort->Text = "6670";                    // dfm :563
    edtHANARMSManCmd = new TEdit();                   // dfm :598-610: no Text
    lblRMS_Status    = new TLabel();
    lblRMS_Status->Caption = "Disconnected";          // dfm :535
    HANARMSClient    = NULL;                          // [W906] 1: made on first use (W906_HANARMSClientCreate), dfm :765-775
    W906_pfnHANARMSClientFree = NULL;
    bW906HANARMSConnecting = false;                   // [W906] 5
    sHANARMSLot = "";
    sHANARMSJobInfoLine = "";
    aSystemHour = aSystemMin = aSystemSec = aSystemMSec = 0;
    aSystemYear = aSystemMonth = aSystemDate = 0;

    bHANARMSNeedRunCheck=false;                                             //RogerYang 20260902 : RMS 互鎖旗標初始化   (golden 912 automation.cpp:148)
#if 0 // GATE(W906-ST02-C10) [W906] 2: fAutomation is made during static initialisation (atester_shims.cpp:370); an ini read here is the BA-SIOF1 hazard (Automation/automation.h:354-361).  PrepareHANARMSConnect loads it before use (golden :3057) -- golden 912 automation.cpp:149
    LoadHANARMSSetting();     //RogerYang : 開機把 IP/Port 載入欄位
#endif
}

THanaRmsMembers::~THanaRmsMembers()      // [W906] golden: the TfAutomation form owns these dfm components and frees them with itself
{
    if(HANARMSClient != NULL && W906_pfnHANARMSClientFree != NULL)   // [W906] 1: see the file banner
        W906_pfnHANARMSClientFree(HANARMSClient);
    HANARMSClient = NULL;
    delete mmoHANARMS;       mmoHANARMS = NULL;
    delete edtHANARMSIP;     edtHANARMSIP = NULL;
    delete edtHANARMSPort;   edtHANARMSPort = NULL;
    delete edtHANARMSManCmd; edtHANARMSManCmd = NULL;
    delete lblRMS_Status;    lblRMS_Status = NULL;
}
