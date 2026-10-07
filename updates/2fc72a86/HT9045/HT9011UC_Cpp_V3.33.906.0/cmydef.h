//==============================================================================
#ifndef cmydefH
#define cmydefH

//AI(W0-TAIL) 20260626: de-VCL cmydef.h. Dropped handlerlog.h / MyStringList.h /
//  MyBinDisp.h / HTEditList.h (VCL-form/UI; cmydef references no types from
//  MyBinDisp/HTEditList, and uses TMyLog by value + TMyStringList* only).
#include "MachineType.h"        //Steven 20130809 : OK
#include "myTimer.h"
#include "cprod.h"              //JerryYang 20150910 Auto Sorting BinTray by Out Arm when Clean Out
#include "cpublic.h"
//AI(W906-S1) 20261007 (Ifor01; W-124): S1 -- the content now lives in three sub-headers (and :5809-6078, formerly outside the
//  guard, is inside it); this file stays the umbrella every includer uses.
#include "cmydef_core.h"
#include "cmydef_io.h"
#include "cmydef_rt.h"
#endif
