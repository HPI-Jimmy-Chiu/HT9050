// =============================================================================
//  forms/fMain_TestSeconds.cpp  --  golden TfMain::Timer2Timer 的測試秒數
//
//  AI(W906-J12) 20260926: new file. 從 906 golden 逐行搬（cp950 解碼成 UTF-8，零 U+FFFD，依行號機械複製，縮排少一層）：
//    HT9011UC_Code_V3.33.906.0_20260618/main.cpp:21157-21205（TfMain::Timer2Timer 裡；V912 是 :21855-21903）
//
//  St01 AUDIT_PROD_20260926 J12（狀態列 [5] 測試秒數「寫死 "0"」，發布歸 St01、產生端歸 Jimmy）：
//  移植樹的 iCurrentTime 只有歸零（atester.cpp:1684、cObserver.cpp:3069），沒有人加 ⇒ 測試中永遠是 0，
//  「Test Time Out Not Alarm Error!!」那一筆 log 也從來不會出現。
//
//  呼叫點：tools/wb_serve.cpp 主迴圈，同一行緊接在 W906_TesterCommTick()（golden Timer2 的 ProcessHVisionConnect，:21155）之後 ——
//    golden 的順序就是 ProcessHVisionConnect → 這一段。
//  計時基準：SystemSec 每拍由 WebBridgeTags.cpp:600 的 GetTimeInfo() 更新（AI(W906-CLOCK)），跟 golden 一樣只看「秒有沒有變」。
//  閘：兩句 StatusBar1 是畫面（fMain 沒有 StatusBar1）；數字本身（iCurrentTime）照 golden 維護，網頁讀它。
//  已知差異：golden 的 Timer2 在 ShowModal 期間也會觸發；移植樹的阻塞框等待迴圈不呼叫這一支 ⇒ 框開著的那幾秒不計。
//
//  ctest：tests/test_timer2_testseconds.cpp（Timer2TestSeconds）。
// =============================================================================
#include "cmydef.h"                  // IsTest / SystemSec / iCurrentTime / bInitialMaxTime
#include "cprod.h"                   // TestIF.iInitialMaxTime / iMaxTime
#include "cMyDB.h"                   // RecordProcess
#include "vclcompat/vcl_compat.h"

void W906_Timer2TestSecondsTick();

//------------------------------------------------------------------------------
void W906_Timer2TestSecondsTick()                                               // golden main.cpp:21157-21205（Timer2Timer 內）
{
static int iOldTime=0;                                                      //Steven 20130703 : 紀錄測試時間
static bool bHasError=false;
if(IsTest)
{
    if(iOldTime!=SystemSec)
    {
        iOldTime=SystemSec;
        iCurrentTime++;
        #if 0 // GATE(W906-J12) web-display: fMain has no StatusBar1 -- the page shows iCurrentTime (status [5], St01 publishes) -- golden main.cpp:21165
        fMain->StatusBar1->Panels->Items[5]->Text=AnsiString(iCurrentTime);
        #endif

        if(bInitialMaxTime==true)
        {
            if(iCurrentTime>TestIF.iInitialMaxTime)
            {
                if(bHasError==false)
                {
                    RecordProcess("Test Time Out Not Alarm Error!!");
                }
                bHasError=true;
            }
            else
            {
                bHasError=false;
            }
        }
        else
        {
            if(iCurrentTime>TestIF.iMaxTime)
            {
                if(bHasError==false)
                {
                    RecordProcess("Test Time Out Not Alarm Error!!");
                }
                bHasError=true;
            }
            else
            {
                bHasError=false;
            }
        }
    }
}
else
{
    bHasError=false;
    iCurrentTime=0;
    iOldTime=SystemSec;
    #if 0 // GATE(W906-J12) web-display: fMain has no StatusBar1 -- the page shows iCurrentTime (status [5], St01 publishes) -- golden main.cpp:21204
    fMain->StatusBar1->Panels->Items[5]->Text="0";
    #endif
}
}
