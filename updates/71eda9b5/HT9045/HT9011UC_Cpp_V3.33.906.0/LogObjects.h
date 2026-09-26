// ===========================================================================
//  LogObjects.h -- create / delete the golden TfMain TMyStringList log objects (cMyDB CSV plan P1).
//  AI(W906-CSVONLY-P1) 20260926.  See LogObjects.cpp.
// ===========================================================================
#ifndef HT9045_LOGOBJECTS_H
#define HT9045_LOGOBJECTS_H

// golden TfMain::TfMain main.cpp:1550-1724 (the log part).  Call once, after the configuration and recipe load.
void W906_CreateLogObjects();
// golden TfMain::FormDestroy main.cpp:12508-12550 (the log part); the TMyStringList destructors flush.
void W906_DestroyLogObjects();

#endif
