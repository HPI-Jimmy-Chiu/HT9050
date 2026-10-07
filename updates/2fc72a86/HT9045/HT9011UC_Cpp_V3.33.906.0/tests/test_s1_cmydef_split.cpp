// AI(W906-S1) 20261007 (Ifor01; W-124): S1 standalone-compile gate, part 3 of 3 -- cmydef_rt.h on its own, then the umbrella.
//   s1_cmydef_core_tu.cpp / s1_cmydef_io_tu.cpp are the other two TUs of this executable; each includes only its own
//   sub-header.  Passing = all three TUs compiled and linked; the checks are compile-time (static_assert / #error).
#include <cstdio>
#include <type_traits>
#include "cmydef_rt.h"
#include "cmydef_rt.h"

#if !defined(cmydef_coreH) || !defined(cmydef_ioH)
#error "cmydef_rt.h does not include cmydef_core.h and cmydef_io.h"
#endif

static_assert(std::is_same<decltype(iTrayZMotor), int[MAX_TRACK]>::value, "old cmydef.h :2524 iTrayZMotor");
static_assert(std::is_same<decltype(iLoaderLayerCount_9050), int>::value, "old cmydef.h (9050 tail) iLoaderLayerCount_9050");
static_assert(std::is_same<decltype(bRunArmSuckZAuto), bool>::value, "old cmydef.h :5812 bRunArmSuckZAuto (was outside the old guard)");

// the umbrella every includer still uses: after the three sub-headers it must add nothing that clashes, and a second
// inclusion must be a no-op (its old tail :5809-6078 used to sit outside the guard).
#include "cmydef.h"
#include "cmydef.h"

int W906_S1CoreTotalMotor();
int W906_S1IoSeesCore();

int main()
{
    const int a = W906_S1CoreTotalMotor(), b = W906_S1IoSeesCore();
    std::printf("[S1] cmydef_core.h / cmydef_io.h / cmydef_rt.h each compile on their own; umbrella cmydef.h twice OK (TOTAL_MOTOR %d / %d)\n", a, b);
    return (a == 164 && b == 164) ? 0 : 1;
}
