// =============================================================================
//  PadInterfaceClock_St02.cpp  --  the RS-232 operator pad's serve-loop job (golden TPadRS232Thread)
//
//  AI(W906-ST02-P1) 20261005 (St02-E): card ST02-P1.  Read PadInterface_St02.cpp's header first (adaptation (2)).
//  Its own TU on purpose (techniques: a new global symbol that another library calls lives in a .cpp of its own):
//  cinitial.cpp / rs232.cpp pull PadInterface_St02.cpp.obj into every executable that links ht9045_sm, and most of
//  them do not compile FastClock.cpp.  Only FastClockWbServe.cpp calls this, and every target that compiles it
//  (wb_serve, test_fastclk_jobs) also compiles FastClock.cpp.
// =============================================================================
#include "PadInterface_St02.h"
#include "cmydef.h"                 // iControlPanelMode
#include "FastClock.h"
#include <string>

//==============================================================================  golden main.cpp:22479 / :10142 + uPadInterface.cpp:31-49
void W906_PadFastClockAdd(ht9045::fastclock::FastClock* clock, std::string& jobs)
{
    if(clock==0 || iControlPanelMode!=1)                                        // golden main.cpp:22479 / :10142: thread only for mode 1
        return;
    if(clock->Add("pad", 1, ht9045::fastclock::kDelay, &W906_PadThreadTick)>=0)  // golden TPadRS232Thread SleepEx(1)
        jobs += ", pad 1 ms (golden TPadRS232Thread::Main232, ControlPanelMode=1)";
}
