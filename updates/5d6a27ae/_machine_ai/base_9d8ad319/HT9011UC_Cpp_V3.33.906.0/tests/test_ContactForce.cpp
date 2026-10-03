// tests/test_ContactForce.cpp
// Verification harness for the ContactForce calc-core translation.
//
// Exercises the translated public API (ComputeSlkForce, ComputeEpMaxVoltage)
// with the exact input->expected-output values hand-derived from the BCB6
// formula in the ORIGINAL golden reference
//   HT9011UC_Code_V3.33.906.0_20260618/ContactForce.cpp
//
// LIMITATION (stated explicitly): we CANNOT run the original BCB6 binary
// (no Borland compiler in this environment).  Verification here is therefore:
//   (1) the translation compiles clean under MinGW g++, AND
//   (2) its outputs equal values hand-computed from the BCB6 source ladders +
//       dMaxForce = 3.14 * (d/100)^2 * 500   (source uses literal 3.14, NOT M_PI).
//
// No external test framework: a tiny check harness prints PASS/FAIL per case
// and a final summary, and returns non-zero on ANY failure.

#include "ContactForce.h"
#include <cmath>
#include <cstdio>
#include <string>

static int g_pass = 0;
static int g_fail = 0;

// Floating-point near-equality.  These are integer-seeded analytic formulas,
// so a tight relative epsilon is appropriate (handles 25371.828 magnitude too).
static bool feq(double a, double b, double eps = 1e-6)
{
    return std::fabs(a - b) <= eps * (1.0 + std::fabs(b));
}

static void check_d(const char* name, double got, double expected)
{
    if (feq(got, expected))
    {
        printf("PASS  %-42s got=%.4f  exp=%.4f\n", name, got, expected);
        ++g_pass;
    }
    else
    {
        printf("FAIL  %-42s got=%.4f  exp=%.4f\n", name, got, expected);
        ++g_fail;
    }
}

static void check_s(const char* name, const std::string& got, const std::string& expected)
{
    if (got == expected) { printf("PASS  %-42s got=\"%s\"\n", name, got.c_str()); ++g_pass; }
    else { printf("FAIL  %-42s got=\"%s\" exp=\"%s\"\n", name, got.c_str(), expected.c_str()); ++g_fail; }
}

static void check_i(const char* name, int got, int expected)
{
    if (got == expected) { printf("PASS  %-42s got=%d\n", name, got); ++g_pass; }
    else { printf("FAIL  %-42s got=%d exp=%d\n", name, got, expected); ++g_fail; }
}

static void check_b(const char* name, bool got, bool expected)
{
    if (got == expected) { printf("PASS  %-42s got=%d\n", name, (int)got); ++g_pass; }
    else { printf("FAIL  %-42s got=%d exp=%d\n", name, (int)got, (int)expected); ++g_fail; }
}

int main()
{
    printf("=== ContactForce translation verification ===\n");
    printf("(values hand-derived from BCB6 ContactForce.cpp; original binary NOT run)\n\n");

    // -----------------------------------------------------------------------
    // THTSLKClass (SLKV_STANDARD) -- has 402 & 80_Hi special cases.
    // dMaxForce = 3.14*(d/100)^2*500.
    // -----------------------------------------------------------------------
    printf("-- SLKV_STANDARD (THTSLKClass) --\n");
    {
        // Dia="30"  -> dDiameter=30, dMinForce=0.5, dMaxForce=141.3
        SlkForceData r = ComputeSlkForce("30", 0, true, 0, SLKV_STANDARD);
        check_d("STD Dia=30 dDiameter", r.dDiameter, 30.0);
        check_d("STD Dia=30 dMinForce", r.dMinForce, 0.5);
        check_d("STD Dia=30 dMaxForce", r.dMaxForce, 141.3);   // 3.14*0.09*500
    }
    {
        // Dia="40"  -> dDiameter=40, dMinForce=2.0 (40<=d<50), dMaxForce=251.2
        SlkForceData r = ComputeSlkForce("40", 0, true, 0, SLKV_STANDARD);
        check_d("STD Dia=40 dDiameter", r.dDiameter, 40.0);
        check_d("STD Dia=40 dMinForce", r.dMinForce, 2.0);
        check_d("STD Dia=40 dMaxForce", r.dMaxForce, 251.2);   // 3.14*0.16*500
    }
    {
        // Dia="60"  -> dDiameter=60, dMinForce=8.0 (else), dMaxForce=565.2
        SlkForceData r = ComputeSlkForce("60", 0, true, 0, SLKV_STANDARD);
        check_d("STD Dia=60 dDiameter", r.dDiameter, 60.0);
        check_d("STD Dia=60 dMinForce", r.dMinForce, 8.0);
        check_d("STD Dia=60 dMaxForce", r.dMaxForce, 565.2);   // 3.14*0.36*500
    }
    {
        // Dia="40x2" -> dDiameter=402, dMinForce=4.0 (==402),
        //              dMaxForce=3.14*(4.02)^2*500=25371.828
        SlkForceData r = ComputeSlkForce("40x2", 0, true, 0, SLKV_STANDARD);
        check_d("STD Dia=40x2 dDiameter", r.dDiameter, 402.0);
        check_d("STD Dia=40x2 dMinForce", r.dMinForce, 4.0);
        check_d("STD Dia=40x2 dMaxForce", r.dMaxForce, 25371.828);
    }
    {
        // Dia="80_Hi" -> dDiameter=80, dMinForce=8.0, dMaxForce=1004.8
        SlkForceData r = ComputeSlkForce("80_Hi", 0, true, 0, SLKV_STANDARD);
        check_d("STD Dia=80_Hi dDiameter", r.dDiameter, 80.0);
        check_d("STD Dia=80_Hi dMinForce", r.dMinForce, 8.0);
        check_d("STD Dia=80_Hi dMaxForce", r.dMaxForce, 1004.8); // 3.14*0.64*500
    }
    {
        // EP_Install==5, Dia="Arm1_30": does NOT contain "Arm2_" -> strip first
        // 5 chars ("Arm1_") -> "30" -> dDiameter=30, dMinForce=0.5, dMaxForce=141.3
        //
        // NOTE on the task's prose example ("Arm2_30"): the BCB6 code strips only
        // when Dia.Pos("Arm2_")==0, i.e. when "Arm2_" is ABSENT.  A token that
        // literally contains "Arm2_" is NOT stripped (it then atof's to 0).  The
        // strip-then-parse-to-30 behavior the task describes is produced by a
        // non-"Arm2_" prefix such as "Arm1_30"; we exercise that real path here.
        SlkForceData r = ComputeSlkForce("Arm1_30", 0, true, 5, SLKV_STANDARD);
        check_d("STD EP5 Arm1_30 dDiameter", r.dDiameter, 30.0);
        check_d("STD EP5 Arm1_30 dMinForce", r.dMinForce, 0.5);
        check_d("STD EP5 Arm1_30 dMaxForce", r.dMaxForce, 141.3);
    }
    {
        // Faithful-quirk check: "Arm2_40" DOES contain "Arm2_" -> NOT stripped
        // -> atof("Arm2_40")=0 -> dDiameter=0 -> dMinForce=0.5 (d<=30 branch).
        SlkForceData r = ComputeSlkForce("Arm2_40", 0, true, 5, SLKV_STANDARD);
        check_d("STD EP5 Arm2_40 dDiameter(quirk)", r.dDiameter, 0.0);
        check_d("STD EP5 Arm2_40 dMinForce(quirk)", r.dMinForce, 0.5);
    }
    {
        // Boundary d=35 (30<d<40) -> dMinForce=1.0
        SlkForceData r = ComputeSlkForce("35", 0, true, 0, SLKV_STANDARD);
        check_d("STD boundary d=35 dMinForce", r.dMinForce, 1.0);
    }
    {
        // Boundary d=50 (50<=d<60) -> dMinForce=4.0
        SlkForceData r = ComputeSlkForce("50", 0, true, 0, SLKV_STANDARD);
        check_d("STD boundary d=50 dMinForce", r.dMinForce, 4.0);
    }

    // -----------------------------------------------------------------------
    // THTSLKIndClass / THTDieForceSLKClass / THTDieForceOneByOneSLKClass
    // NO 402 special case.  dMaxForce identical to STANDARD.
    // -----------------------------------------------------------------------
    const SlkVariant noSpecial[3] = { SLKV_IND, SLKV_DIEFORCE, SLKV_DIEFORCE_OBO };
    const char* noSpecialName[3]  = { "IND", "DIEFORCE", "DIEFORCE_OBO" };

    for (int v = 0; v < 3; ++v)
    {
        printf("-- %s (no 402 case) --\n", noSpecialName[v]);
        char nm[96];
        {
            // Dia="30" -> dMinForce=0.5, dMaxForce=141.3
            SlkForceData r = ComputeSlkForce("30", 0, true, 0, noSpecial[v]);
            snprintf(nm, sizeof(nm), "%s Dia=30 dMinForce", noSpecialName[v]);
            check_d(nm, r.dMinForce, 0.5);
            snprintf(nm, sizeof(nm), "%s Dia=30 dMaxForce", noSpecialName[v]);
            check_d(nm, r.dMaxForce, 141.3);
        }
        {
            // Dia="40" -> dMinForce=2.0, dMaxForce=251.2
            SlkForceData r = ComputeSlkForce("40", 0, true, 0, noSpecial[v]);
            snprintf(nm, sizeof(nm), "%s Dia=40 dMinForce", noSpecialName[v]);
            check_d(nm, r.dMinForce, 2.0);
            snprintf(nm, sizeof(nm), "%s Dia=40 dMaxForce", noSpecialName[v]);
            check_d(nm, r.dMaxForce, 251.2);
        }
        {
            // Dia="60" -> dMinForce=8.0, dMaxForce=565.2
            SlkForceData r = ComputeSlkForce("60", 0, true, 0, noSpecial[v]);
            snprintf(nm, sizeof(nm), "%s Dia=60 dMinForce", noSpecialName[v]);
            check_d(nm, r.dMinForce, 8.0);
            snprintf(nm, sizeof(nm), "%s Dia=60 dMaxForce", noSpecialName[v]);
            check_d(nm, r.dMaxForce, 565.2);
        }
    }

    // -----------------------------------------------------------------------
    // DIEFORCE_OBO: int iCount=(Tag%8)+1 affects captions only, not the calc.
    // Varying tag must NOT change dDiameter/dMinForce/dMaxForce.
    // -----------------------------------------------------------------------
    printf("-- DIEFORCE_OBO iCount(Tag) does not affect calc --\n");
    {
        bool allSame = true;
        SlkForceData ref = ComputeSlkForce("40", 0, true, 0, SLKV_DIEFORCE_OBO);
        for (int tag = 0; tag < 16; ++tag)
        {
            SlkForceData r = ComputeSlkForce("40", tag, true, 0, SLKV_DIEFORCE_OBO);
            if (!feq(r.dDiameter, ref.dDiameter) ||
                !feq(r.dMinForce, ref.dMinForce) ||
                !feq(r.dMaxForce, ref.dMaxForce))
                allSame = false;
        }
        check_b("OBO tag-invariant calc (0..15)", allSame, true);
    }

    // -----------------------------------------------------------------------
    // Button1Click EP voltage interpolation.
    // dMaxVol = (((dMidVol-dMinVol)/5)*9)+dMinVol
    // dMinVol=1.0, dMidVol=2.0 -> (1.0/5)*9 + 1.0 = 0.2*9 + 1.0 = 2.8
    // -----------------------------------------------------------------------
    printf("-- ComputeEpMaxVoltage --\n");
    {
        double mv = ComputeEpMaxVoltage(1.0, 2.0);
        check_d("EP interp (1.0,2.0)", mv, 2.8);
    }

    // -----------------------------------------------------------------------
    // Metadata pass-through (sDiameter / iTag / bShow).
    // -----------------------------------------------------------------------
    printf("-- metadata pass-through --\n");
    {
        SlkForceData r = ComputeSlkForce("56", 7, false, 0, SLKV_STANDARD);
        check_s("meta sDiameter", r.sDiameter, "56");
        check_i("meta iTag", r.iTag, 7);
        check_b("meta bShow", r.bShow, false);
    }

    // -----------------------------------------------------------------------
    // Summary
    // -----------------------------------------------------------------------
    printf("\n=== SUMMARY: %d passed, %d failed (total %d) ===\n",
           g_pass, g_fail, g_pass + g_fail);
    if (g_fail != 0)
    {
        printf("RESULT: FAIL\n");
        return 1;
    }
    printf("RESULT: ALL PASS\n");
    return 0;
}
