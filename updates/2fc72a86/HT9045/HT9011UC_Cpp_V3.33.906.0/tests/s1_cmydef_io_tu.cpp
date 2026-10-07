// AI(W906-S1) 20261007 (Ifor01; W-124): S1 standalone-compile gate, part 2 of 3 -- cmydef_io.h on its own (it brings core).
#include <type_traits>
#include "cmydef_io.h"
#include "cmydef_io.h"

#ifndef cmydef_coreH
#error "cmydef_io.h does not include cmydef_core.h"
#endif
#ifdef cmydef_rtH
#error "cmydef_io.h pulls in cmydef_rt.h"
#endif

// one name from each of C (cylinders), D (sensors), E (switches), F (motors), and from the IO lines that sat outside the old guard
static_assert(std::is_same<decltype(C_TrayZ_Selector), const int>::value, "old cmydef.h :302 C_TrayZ_Selector");
static_assert(std::is_same<decltype(MTopAOICCDZ), const int>::value, "old cmydef.h :2522 MTopAOICCDZ");
static_assert(std::is_same<decltype(C_MobileTrayTableSelect), const int>::value, "old cmydef.h (9050 tail) C_MobileTrayTableSelect");
static_assert(std::is_same<decltype(SnC_ALGrip12), const int>::value, "old cmydef.h (9050 tail) SnC_ALGrip12");

int W906_S1IoSeesCore() { return TOTAL_MOTOR; }
