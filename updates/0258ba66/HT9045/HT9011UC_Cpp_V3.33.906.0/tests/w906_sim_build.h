// =============================================================================
//  tests/w906_sim_build.h -- one expectation per build configuration, on the SAME line.
//
//  AI(W906-W149) 20261007 (St02-E), card W-149 SIM-AWARE.  The simulation build defines SOFT_SIMULTE (MachineType.h:63-65); the
//  ship build does not (cmake -DW906_NO_SOFT_SIMULTE=ON).  Golden's `#ifdef SOFT_SIMULTE` arms make some behaviour differ by build
//  (MachineType.h:56-62 lists them).  A test that checks such behaviour gives the SIM value from that arm and keeps the ship
//  condition as it was:
//
//      CHECK(W906_SIM_BUILD ? (c1 == 0) : (c1 == 1), "code 1" W906_SIM_NOTE(" -- SIM: golden 0618 MyLaneIo.cpp:582 skips it"));
//
//  W906_SIM_BUILD  1 when SOFT_SIMULTE is defined, else 0 -- the same macro as the product arm the check mirrors.
//  W906_SIM_NOTE   the text in the SIM build, "" in the ship build (so the ship output stays byte for byte as it was).
//  Same-line form so the other owners' test files keep their line numbers (failure output and reverse lists cite them).
//  Product code is never changed to make a test pass; the SIM value always cites the golden arm that produces it.
// =============================================================================
#ifndef W906_TESTS_SIM_BUILD_H
#define W906_TESTS_SIM_BUILD_H

#include "MachineType.h"   // SOFT_SIMULTE (unless W906_NO_SOFT_SIMULTE)

#ifdef SOFT_SIMULTE
#define W906_SIM_BUILD 1
#define W906_SIM_NOTE(s) s
#else
#define W906_SIM_BUILD 0
#define W906_SIM_NOTE(s) ""
#endif

#endif // W906_TESTS_SIM_BUILD_H
