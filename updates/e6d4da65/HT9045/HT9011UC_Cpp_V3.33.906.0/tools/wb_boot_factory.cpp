// ===========================================================================
//  tools/wb_boot_factory.cpp -- census 129 (e) E-BOOT-005: golden TfMain::FormShow main.cpp:10615-10616 (golden
//  906_0618; V912 main.cpp:11056-11057) once at wb_serve's boot.  AI(W906-B23-EBOOT005) 20261001 (laptop, batch 23).
//      RunInfo.Factory=HandlerSystem->GetCustomerName();
//      fObserver->labFactory->Caption=RunInfo.Factory;                             //Steven 20120524 : 只讀一次就夠了
//  The port has no HandlerSystem global (forms/fHandlerSys.h INTEGRATION-PENDING); St01's FileRW_HSys_CustomerName()
//  (end of FileRW/HSys.cpp, D-037) is golden THandlerSystem::GetCustomerName over the rgCustomerList proxy, so this runs
//  after FileRW_HSys_Boot() (tools/wb_serve.cpp:4062).  Called on tools/wb_serve.cpp:4166 before the :10655 log objects,
//  i.e. in golden's order.  A file of its own so tools/wb_serve.cpp needs no new #include (its line count stays).
//  Programs other than wb_serve do not link FileRW (St01's "HonPrec" fallback, FileRW/_fallback.cpp) and do not call this.
// ===========================================================================
#include "cprod.h"               // RunInfo (TRunInfo::Factory, cprod.h:2731)
#include "forms/fObserver.h"     // fObserver (cObserver.cpp:3289) -> labFactory (forms/fObserver.h:845, a TPanel)

AnsiString FileRW_HSys_CustomerName();   // FileRW/HSys.cpp (St01, D-037)

void W906_Boot_RunInfoFactory()
{
    RunInfo.Factory=FileRW_HSys_CustomerName();                                 // golden main.cpp:10615
    fObserver->labFactory->Caption=RunInfo.Factory;                             //Steven 20120524 : 只讀一次就夠了   (golden :10616)
}
