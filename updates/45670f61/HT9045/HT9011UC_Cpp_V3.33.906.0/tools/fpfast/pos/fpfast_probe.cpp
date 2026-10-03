/* AI(W906-FPFAST) 20261003 -- positive control for -fexcess-precision=fast.
 * Same source for BCB6 (bcc32 -Od), the oracle (MinGW.org g++ 6.3.0) and the machine's
 * WinLibs g++ 16.2.0 i686.  Pre-C++11 on purpose (bcc32 must compile it).
 * Prints one line per case: <id> <0|1>.  Shapes copied from the tree:
 *   L*  = `double == <decimal literal>` (the excess-precision class the flag is about)
 *   R*  = Adam6024Pressure_St02.cpp:822 strict range `v<0.8 || v>5.2`
 *   S*  = computed, then stored, then compared (adam6024 KYEC `d = x/10.0; d==2.8`)
 *   X*  = pure 80-bit-intermediate arithmetic BCB6 relies on (must NOT change with the flag)
 */
#include <stdio.h>
#include <stdlib.h>

struct TDeviceForm { int iPad; double dKitDiameter; double dDieForceKitDiameter; };
TDeviceForm DeviceForm_File;            /* global, like cprod.h's DeviceForm_File */

double via_param(double x) { return x; }
double read_float(const char* s) { return strtod(s, 0); }   /* ReadIniData -> ReadFloat -> strtod */

static void out(const char* id, int v) { printf("%s %d\n", id, v ? 1 : 0); }

int main()
{
    /* ---- L: decimal literal equality (FP_ORACLE_FINDINGS s1 / s3) -------------------- */
    double d402 = 40.2;
    out("L1_local_d==40.2", d402 == 40.2);
    double d56 = 5.6;
    out("L2_local_d==5.6", d56 == 5.6);
    volatile double vd = 5.6;
    out("L3_volatile_vd==5.6", vd == 5.6);
    volatile double vd402 = 40.2;
    out("L4_volatile_vd==40.2", vd402 == 40.2);
    out("L5_via_param(5.6)==5.6", via_param(5.6) == 5.6);
    DeviceForm_File.dKitDiameter = read_float("40.2000");       /* recipe text, %0.4f */
    out("L6_global_DeviceForm.dKitDiameter==40.2", DeviceForm_File.dKitDiameter == 40.2);   /* cContact / cinitial / fContact:1671 shape */
    DeviceForm_File.dKitDiameter = read_float("5.6000");
    double fDiameter = DeviceForm_File.dKitDiameter;            /* adam6024 / Adam6024Pressure fDiameter shape */
    out("L7_fDiameter==5.6", fDiameter == 5.6);
    DeviceForm_File.dDieForceKitDiameter = read_float("40.2000");
    fDiameter = DeviceForm_File.dDieForceKitDiameter;
    out("L8_fDiameter==40.2", fDiameter == 40.2);
    double xpitch = read_float("26.67");
    out("L9_XPitch==26.67", xpitch == 26.67);                   /* fHotPlate.cpp:234 shape */

    /* ---- R: Adam6024Pressure_St02.cpp:822 strict range (golden: v<0.8 || v>5.2) -------- */
    double v52 = read_float("5.2");
    out("R1_(5.2<0.8||5.2>5.2)", (v52 < 0.8 || v52 > 5.2));
    double v08 = read_float("0.8");
    out("R2_(0.8<0.8||0.8>5.2)", (v08 < 0.8 || v08 > 5.2));

    /* ---- S: computed, stored to a double, compared (adam6024 KYEC d==2.8) ------------- */
    volatile int i28 = 28;
    double d28 = i28 / 10.0;
    out("S1_d=28/10.0;d==2.8", d28 == 2.8);
    volatile int i58 = 58;
    double d58 = i58 / 10.0;
    out("S2_d=58/10.0;d==5.8", d58 == 5.8);

    /* ---- X: 80-bit intermediates (must be flag-invariant) ----------------------------- */
    volatile double dDia56 = 56.0, f56 = 5.6, dDia40 = 40.0, f40 = 4.0;
    out("X1_56==5.6*10", dDia56 == f56 * 10);                    /* FP_ORACLE_FINDINGS s8.6 adam6024 :450 shape */
    out("X2_40==4.0*10", dDia40 == f40 * 10);
    volatile double a = 1e16, b = 1.0;
    out("X3_(1e16+1)-1e16==1", ((a + b) - a) == 1.0);             /* 64-bit mantissa keeps the 1 */
    volatile double big = 1e308;
    out("X4_(1e308*10)/10==1e308", ((big * 10.0) / 10.0) == big); /* x87 extended exponent range */
    return 0;
}
