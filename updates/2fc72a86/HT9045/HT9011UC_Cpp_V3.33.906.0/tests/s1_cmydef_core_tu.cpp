// AI(W906-S1) 20261007 (Ifor01; W-124): S1 standalone-compile gate, part 1 of 3 -- cmydef_core.h on its own.
//   Header-slimming plan s A5 (row "tool": every new sub-header gets a standalone compile test, like w0tail_headers_compile.cpp).
//   This TU includes ONLY cmydef_core.h (twice: the guard must hold), so a declaration that core needs but that now lives in
//   cmydef_io.h / cmydef_rt.h fails right here, not in some includer that happens to include cmydef.h first.
#include "cmydef_core.h"
#include "cmydef_core.h"

// core is the layer S2 moves the ~287 non-IO includers to -- it must not drag the IO or runtime declarations back in.
#ifdef cmydef_ioH
#error "cmydef_core.h pulls in cmydef_io.h"
#endif
#ifdef cmydef_rtH
#error "cmydef_core.h pulls in cmydef_rt.h"
#endif
// plan s A5 step 4: PCI132 stays in core (cinitial.cpp tests it with #ifdef; a missing define would silently change code).
#ifndef PCI132
#error "PCI132 is not defined by cmydef_core.h"
#endif

static_assert(TOTAL_MOTOR == 164, "old cmydef.h :46 TOTAL_MOTOR");
static_assert(PCI1203_IO == 4, "old cmydef.h :5957 PCI1203_IO (was outside the old guard)");

int W906_S1CoreTotalMotor() { return TOTAL_MOTOR; }
