// =============================================================================
//  AOI/TopBottomInspect.cpp -- see TopBottomInspect.h.  AI(W906-Q32-S69) 20260927 (St02).
//  Every key, section and default below is golden 906_0625_Steven fAOI.cpp:182-263 (TFrmAOI::IntialParameter), in the
//  same order.  AI(W906-ST02-C912) 20261003 (St02-E helper): back to golden 906_0625 fAOI.cpp:182-263 (RULINGS_20261002 #20 / #23-6) -- the 912 block (912 fAOI.cpp:237-238 iAutoRetryCount / sRecipeName, :316-319 FailStop_<code> x15, Ifor 20260812 / 0827 / 0820) and the with912 flag removed.
// =============================================================================
#include "AOI/TopBottomInspect.h"

#include <cstdio>
#include <cstdlib>

namespace aoi {

namespace {

void AddI(std::vector<TbKey>* v, const char* sec, const std::string& key, int* p, const char* def, char kind = 'i')
{
    TbKey k;
    k.section = sec; k.key = key; k.kind = kind; k.pi = p; k.pb = 0; k.ps = 0; k.def = def;
    v->push_back(k);
}
void AddB(std::vector<TbKey>* v, const char* sec, const std::string& key, bool* p)
{
    TbKey k;
    k.section = sec; k.key = key; k.kind = 'b'; k.pi = 0; k.pb = p; k.ps = 0; k.def = "";   // golden check boxes: default ""
    v->push_back(k);
}
void AddS(std::vector<TbKey>* v, const char* sec, const std::string& key, AnsiString* p, const char* def)
{
    TbKey k;
    k.section = sec; k.key = key; k.kind = 's'; k.pi = 0; k.pb = 0; k.ps = p; k.def = def;
    v->push_back(k);
}
std::string Idx(const char* fmt, int i)
{
    char b[96];
    std::snprintf(b, sizeof(b), fmt, i);           // golden AnsiString().sprintf(fmt, i); no std::to_string on MinGW.org 6.3
    return b;
}
void AddPhoto(std::vector<TbKey>* v, const char* prefix, int i, TbPhotoInfo& t)   // :207-212 / :216-221
{
    char f[32];
    std::snprintf(f, sizeof(f), "%s_%%d_PosX", prefix); AddI(v, "TopBottomInspect", Idx(f, i), &t.ppPosition.X, "0");
    std::snprintf(f, sizeof(f), "%s_%%d_PosY", prefix); AddI(v, "TopBottomInspect", Idx(f, i), &t.ppPosition.Y, "0");
    std::snprintf(f, sizeof(f), "%s_%%d_LT", prefix);   AddB(v, "TopBottomInspect", Idx(f, i), &t.bClampLT);
    std::snprintf(f, sizeof(f), "%s_%%d_LB", prefix);   AddB(v, "TopBottomInspect", Idx(f, i), &t.bClampLB);
    std::snprintf(f, sizeof(f), "%s_%%d_RT", prefix);   AddB(v, "TopBottomInspect", Idx(f, i), &t.bClampRT);
    std::snprintf(f, sizeof(f), "%s_%%d_RB", prefix);   AddB(v, "TopBottomInspect", Idx(f, i), &t.bClampRB);
}

}  // namespace

std::vector<TbKey> TopBottomInspectKeys(TopBottomInspectParams& p)
{
    std::vector<TbKey> v;
    const char* T = "TopBottomInspect";
    const char* F = "Function Setting";
    AddI(&v, T, "iEnable",                 &p.iEnable,                 "0", 'r');           // :182
    AddI(&v, T, "iAction",                 &p.iAction,                 "0", 'r');           // :183
    AddS(&v, T, "SocketAddress",           &p.sSocketAddress,          "172.16.8.200");     // :185
    AddS(&v, T, "SocketPort",              &p.sSocketPort,             "5109");             // :186
    AddS(&v, T, "sCamaName",               &p.sCamaName,               "CM1");              // :187
    AddI(&v, T, "iStartDelayTime",         &p.iStartDelayTime,         "5");                // :188
    AddI(&v, T, "iGetResultDelay",         &p.iGetResultDelay,         "200");              // :189
    AddI(&v, T, "iTimeout",                &p.iTimeout,                "10");               // :190
    AddI(&v, T, "iRotate0Pos",             &p.iRotate0Pos,             "-20");              // :191
    AddI(&v, T, "iRotate180Pos",           &p.iRotate180Pos,           "3230");             // :192
    AddI(&v, T, "iTopBtnCenterX",          &p.tbCenter.ppPosition.X,   "977");              // :193
    AddI(&v, T, "iTopBtnCenterY",          &p.tbCenter.ppPosition.Y,   "80");               // :194
    AddI(&v, T, "iLight_Up_Z_Top",         &p.iLight_Up_Z_Top,         "0");                // :195
    AddI(&v, T, "iLight_Up_Z_Btm",         &p.iLight_Up_Z_Btm,         "0");                // :196
    AddI(&v, T, "iCCD_Up_Z_Top",           &p.iCCD_Up_Z_Top,           "0");                // :197
    AddI(&v, T, "iCCD_Up_Z_Btm",           &p.iCCD_Up_Z_Btm,           "0");                // :198
    AddI(&v, T, "iRotateKitAngOffset_In",  &p.iRotateKitAngOffset_In,  "0");                // :200
    AddI(&v, T, "iRotateKitAngOffset_Out", &p.iRotateKitAngOffset_Out, "0");                // :201
    AddI(&v, "ZPickOffset", "iZPickOffset", &p.iZPickOffset,           "0");                // :203
    for (int i = 0; i < 2; ++i) AddPhoto(&v, "Small", i, p.tbpiLessThanOrEqual65mm[i]);     // :205-213
    for (int i = 0; i < 4; ++i) AddPhoto(&v, "Large", i, p.tbpiMoreThan65mm[i]);            // :214-222
    AddB(&v, F, "Enable Consecutive Fail Check",          &p.bConsecutiveFailCheck);                // :224
    AddI(&v, F, "Consecutive Fail Count",                 &p.iConsecutiveFailCount, "0");           // :225
    AddB(&v, F, "Enable Consecutive Fail Check(Picture)", &p.bConsecutiveFailPictureCheck);         // :227
    AddI(&v, F, "Consecutive Fail Count(Picture)",        &p.iConsecutiveFailPictureCount, "0");    // :228
    AddB(&v, F, "Enable Accumulated Fail Check",          &p.bAccumulatedFailCheck);                // :230
    AddI(&v, F, "Accumulated Fail Count",                 &p.iAccumulatedFailCount, "0");           // :231
    AddB(&v, F, "Enable Interval Check",                  &p.bIntervalCheck);                       // :233
    AddI(&v, F, "Interval Count",                         &p.iIntervalCount, "0");                  // :234
    AddI(&v, F, "AOI Fail Set Bin",                       &p.iAOIFailSetBin, "15");                 // :236
    AddB(&v, F, "Enable AOI Fail Bin",                    &p.bAOIFailBin);                          // :237
    for (int i = 0; i < kAOIFailCheckMax; ++i) {                                                    // :240-264
        AddB(&v, F, Idx("Enable Consecutive Fail Check_%d", i),          &p.bConsecutiveFailCheck_List[i]);
        AddI(&v, F, Idx("Consecutive Fail Count_%d", i),                 &p.iConsecutiveFailCount_List[i], "0");
        AddB(&v, F, Idx("Enable Consecutive Fail Check(Picture)_%d", i), &p.bConsecutiveFailPictureCheck_List[i]);
        AddI(&v, F, Idx("Consecutive Fail Count(Picture)_%d", i),        &p.iConsecutiveFailPictureCount_List[i], "0");
        AddB(&v, F, Idx("Enable Accumulated Fail Check_%d", i),          &p.bAccumulatedFailCheck_List[i]);
        AddI(&v, F, Idx("Accumulated Fail Count_%d", i),                 &p.iAccumulatedFailCount_List[i], "0");
        AddB(&v, F, Idx("Enable Interval Check_%d", i),                  &p.bIntervalCheck_List[i]);
        AddI(&v, F, Idx("Interval Count_%d", i),                         &p.iIntervalCount_List[i], "0");
    }
    return v;
}

void TopBottomInspectParams::SetGoldenDefaults()
{
    tbCenter.bClampLT = tbCenter.bClampLB = tbCenter.bClampRT = tbCenter.bClampRB = false;   // fAOI.cpp:4611 tbCenter.Clear()
    std::vector<TbKey> keys = TopBottomInspectKeys(*this);
    for (std::size_t i = 0; i < keys.size(); ++i) {
        const TbKey& k = keys[i];
        if (k.pi) *k.pi = std::atoi(k.def);
        else if (k.pb) *k.pb = (std::atoi(k.def) == 1);        // HTEditList.cpp:1127-1129: checked only when exactly 1
        else if (k.ps) *k.ps = AnsiString(k.def);
    }
}

}  // namespace aoi
