// =============================================================================
//  GearRatioLive.cpp  --  the Motor Test "Gear Ratio" tab: the machine half of IGearRatioBackend (wb_serve only)
//
//  AI(W906-GEARRATIO) 20261002 [W906]: RULINGS_20261002 #22 (NB2 spec §5; the interface: GearRatioBackend.h; the dispatcher and
//  the save transaction: WebMotorAccess.cpp EOF; the pure part: GearCalc.cpp). Forwarded by WebMotorAccessLive.cpp's
//  LiveBackend. Runs on the tick thread only (the motor.access dispatch and OverlaySnapshot), the thread that owns MOT[] /
//  Tech / fTeach -- like ArmCellLive.cpp.
//    GearLiveArmOf          MOT[mi].Mot_Name is MInArmX / MInArmY (1) or MOutArmX / MOutArmY (2).
//    GearLiveEmgStop        golden TfMotorTest::IsMotorCanRun(true)'s sensors (uMotorTest.cpp:1280-1298; the port's copy is
//                           WebMotorAccessLive.cpp GoldenMotorCanRun) WITHOUT its ShowMyMessage("EMG Stop") -- in wb_serve that
//                           box holds the tick thread until it is answered; this feature refuses with the reason instead.
//    GearLiveTeachWindowOpen W906_FormShowing("fTeach", fTeach->fShow) -- the page table (W906FormShowing.h).
//    GearLiveTeachRefs      fTeach->TechPara / TechTwoPara / TechSuckPara[0..1] (+[2] with the sort arm) and elTeach's integer slots.
//    GearLiveSetMotor       MOT[mi].Motor->GearRatio / PSoftLimitP / PSoftLimitN (memory; the file is the dispatcher's).
//    GearLiveSetTeach       writes through the registry pointers.
//    GearLiveTeachSaveReload golden TfTeach::btnSaveClick's tail without its question (FileRW/Teach.cpp IC_btnSaveClick :193-197):
//                           FileRW_Teach_SaveFile(true) (= fTeach->SaveFile(true): TECH_* + elTeach + the two backlash keys; tech.dat
//                           not written, ruling 14 -- reported in the note), fTeach->ReadFile(), the proxies mirrored,
//                           InitShuttleThreadParameter(), fAllMotorHome=false. The Gerneral.ini keys of btnSaveClick are NOT written.
// =============================================================================
#include "WebMotorAccess.h"
#include "GearRatioBackend.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "vclcompat/vcl_compat.h"
#include "MachineType.h"
#include "cmydef.h"           // MInArmX .., fAllMotorHome, Enable_PLCSafety_IO, USE_OUT_SORT_ARM, Sn*EMG
#include "mysensor.h"         // Sen[]
#include "common.h"           // asTeachPath
#include "cinitial.h"         // InitShuttleThreadParameter
#include "Motor/mymotor.h"    // MOT[]
#include "forms/fTeach.h"     // fTeach, TechPara / TechTwoPara / TechSuckPara
#include "forms/fTeachPara.h"
#include "Public/HTEditList.h"   // elTeach, THTEdit
#include "FileRW/_EditList.h"    // filerw::SessionBegin / SessionJson
#include "W906FormShowing.h"
#include "WebLogin.h"
#include "Public/cJSON.h"

void FileRW_Teach_SaveFile(bool bSaveByTeach);   // FileRW/Teach.cpp (golden TfTeach::SaveFile, uteach.cpp:4924)
void FileRW_Teach_MirrorProxies();               // FileRW/Teach.cpp EOF (golden ReadFile's SetEdit->Text=*Parameter mirror)

namespace ht9045 {

int GearLiveArmOf(int mi)
{
    if (mi < 0 || mi >= MAX_TRAY_MOTOR || MOT[mi].Motor == 0) return -1;
    const int n = MOT[mi].Mot_Name;
    if (n == MInArmX || n == MInArmY) return 1;
    if (n == MOutArmX || n == MOutArmY) return 2;
    return 0;
}

bool GearLiveEmgStop(std::string& why)
{
    std::string w;
    if (Sen[SnFrontLeftEMG].IsOff())  w += "前左 ";
    if (Sen[SnFrontRightEMG].IsOff()) w += "前右 ";
    if (Sen[SnRearLeftEMG].IsOff())   w += "後左 ";
    if (Sen[SnRearRightEMG].IsOff())  w += "後右 ";
    if (Enable_PLCSafety_IO && Sen[SnAllEMG].IsOff()) w += "PLC 安全迴路 ";      // KenHsieh 20250212 (golden :1290)
    if (Sen[SnServo].Enable && Sen[SnServo].IsOff()) w += "Servo 電源 ";        // kevin 20140121 (golden :1292)
    if (w.empty()) { why.clear(); return false; }
    why = "EMG Stop：" + w + "（golden TfMotorTest::IsMotorCanRun 的感測器）—— 不移動";
    return true;
}

bool GearLiveTeachWindowOpen(std::string& why)
{
    const bool open = W906_FormShowing("fTeach", fTeach != 0 && fTeach->fShow);
    why = open ? "頁面表說教導頁（fTeach）開著" : std::string();
    return open;
}

bool GearLiveTeachRefs(std::vector<GearTeachRef>& out, std::string& why)
{
    out.clear();
    if (!fTeach) { why = "教導頁的 facade（fTeach）沒建（wb_serve 開機會建）"; return false; }
    if (!elTeach) { why = "elTeach 沒建（FileRW_Teach_Boot）"; return false; }
    for (std::size_t i = 0; i < fTeach->TechPara.size(); ++i) {
        const TECH_PARA* t = fTeach->TechPara[i];
        if (!t || !t->Parameter) continue;
        GearTeachRef r;
        r.ptr = t->Parameter; r.list = "TechPara"; r.index = (int)i; r.side = 0; r.rawMotor = t->MotorSelect;
        r.section = (t->MotorSelect >= 0 && t->MotorSelect < MAX_TRAY_MOTOR) ? std::string(MOT[t->MotorSelect].Alias.c_str()) : std::string();
        r.key = t->Key.c_str(); r.value = *t->Parameter;
        out.push_back(r);
    }
    for (std::size_t i = 0; i < fTeach->TechTwoPara.size(); ++i) {
        const TECH_TWOPARA* t = fTeach->TechTwoPara[i];
        if (!t) continue;
        for (int j = 0; j < 2; ++j) {
            if (!t->Parameter[j]) continue;
            GearTeachRef r;
            r.ptr = t->Parameter[j]; r.list = "TechTwoPara"; r.index = (int)i; r.side = j; r.rawMotor = t->MotorSelect[j];
            r.section = (t->MotorSelect[j] >= 0 && t->MotorSelect[j] < MAX_TRAY_MOTOR) ? std::string(MOT[t->MotorSelect[j]].Alias.c_str()) : std::string();
            r.key = t->Key[j].c_str(); r.value = *t->Parameter[j];
            out.push_back(r);
        }
    }
    const int nSuck = (USE_OUT_SORT_ARM != eartUninstall) ? 3 : 2;               // golden ReadFile / SaveFile (RogerYang 20250416)
    for (int s = 0; s < nSuck; ++s)
        for (int i = 0; i < 2; ++i)
            for (int j = 0; j < 8; ++j) {
                const TECH_SUCKPARA& p = fTeach->TechSuckPara[s];
                if (!p.Parameter[i][j]) continue;
                GearTeachRef r;
                r.ptr = p.Parameter[i][j]; r.list = "TechSuckPara"; r.index = s; r.side = i * 8 + j; r.rawMotor = p.MotorSelect[i][j];
                r.section = p.Group.c_str(); r.key = p.Key[i][j].c_str(); r.value = *p.Parameter[i][j];
                out.push_back(r);
            }
    for (int k = 0; k < elTeach->FEditList->Count; ++k) {
        const THTEdit* it = static_cast<const THTEdit*>(elTeach->FEditList->Items[k]);
        if (!it || !it->iParameter) continue;
        if (it->Content != ECInteger && it->Content != ECPosInt && it->Content != ECNegInt) continue;
        GearTeachRef r;
        r.ptr = it->iParameter; r.list = "elTeach"; r.index = k; r.side = 0; r.rawMotor = -1;
        r.section = it->IniGroupName.c_str(); r.key = it->IniKeyName.c_str(); r.value = *it->iParameter;
        out.push_back(r);
    }
    return true;
}

std::string GearLiveTeachIniPath() { return std::string(asTeachPath.c_str()); }

bool GearLiveSetMotor(int mi, double ratio, int softP, int softN, std::string& why)
{
    if (mi < 0 || mi >= MAX_TRAY_MOTOR || MOT[mi].Motor == 0) { why = "MOT[" + std::to_string(mi) + "] 沒有馬達物件"; return false; }
    MOT[mi].Motor->GearRatio   = ratio;
    MOT[mi].Motor->PSoftLimitP = softP;
    MOT[mi].Motor->PSoftLimitN = softN;
    std::printf("motor.access gear: MOT[%d] GearRatio=%.9g PSoftLimitP=%d PSoftLimitN=%d\n", mi, ratio, softP, softN);
    return true;
}

bool GearLiveSetTeach(const std::vector<std::pair<const int*, int> >& v, std::string& why)
{
    for (std::size_t i = 0; i < v.size(); ++i) {
        if (!v[i].first) { why = "空的教導點指標"; return false; }
        *const_cast<int*>(v[i].first) = v[i].second;                            // a registry pointer GearLiveTeachRefs returned in this dispatch
    }
    return true;
}

bool GearLiveTeachSaveReload(bool save, std::string& why, std::string& note)
{
    note.clear();
    if (!fTeach) { why = "教導頁的 facade（fTeach）沒建"; return false; }
    filerw::SessionBegin(std::string());
    if (save) FileRW_Teach_SaveFile(true);
    fTeach->ReadFile();
    FileRW_Teach_MirrorProxies();
    InitShuttleThreadParameter();                                               // golden btnSaveClick :2280 (Steven 20120921)
    fAllMotorHome = false;                                                      // golden btnSaveClick :2281
    note = filerw::SessionJson();
    return true;
}

std::string GearLiveCalLogDir()
{
    const char* d = std::getenv("W906_OPLOG_DIR");
    return (d && *d) ? std::string(d) : std::string();
}

std::string GearLiveOperator()
{
    const std::string j = WebLogin_StateJson();
    cJSON* root = cJSON_Parse(j.c_str());
    std::string s;
    if (root) {
        const cJSON* lv = cJSON_GetObjectItemCaseSensitive(root, "levelName");
        const cJSON* uc = cJSON_GetObjectItemCaseSensitive(root, "userCaption");
        const cJSON* le = cJSON_GetObjectItemCaseSensitive(root, "level");
        if (cJSON_IsString(lv) && lv->valuestring) s = lv->valuestring;
        if (cJSON_IsString(uc) && uc->valuestring && *uc->valuestring) s += std::string(s.empty() ? "" : " ") + uc->valuestring;
        if (cJSON_IsNumber(le)) s += " (level " + std::to_string((int)le->valuedouble) + ")";
        cJSON_Delete(root);
    }
    return s;
}

}  // namespace ht9045
