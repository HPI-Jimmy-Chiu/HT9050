// ===========================================================================
//  FileRW/DeviceForm_KbExtra.cpp -- TfContact（Setup.Contact.html）editlist.get 的 "extra"：golden 小鍵盤的執行期上下限，
//  以及 edDropWaitTimeMouseDown 輸入後那一段修正要用到的伺服器端值。
//
//  //AI(W906-SETUPA-KB) 20261002 新檔（手寫）。EastSun 1001「請檢查每個頁面元件…用枚舉 每個東西都檢查」：
//    Contact 頁有一批欄位 golden 的 ShowQwertyKey 上下限是執行期變數（InputLimit.*、DeviceForm_File.dDieForceKitDiameter、
//    IniConfig.bChangeKitNoHardStop、CUSTOMER_CODE 分支），網頁拿不到（web/page/ht9045_contact_wire.js:114 的註記），
//    所以小鍵盤只有 golden 的旗標與小數位、沒有範圍；edDropWaitTimeMouseDown（golden cContact.cpp:2089-2239）輸入之後
//    還有一整段修正（Release>=Pick+1、Contact 高度、Drop offset、IC 大小≤Tray pitch、速度、等待時間、扭力），
//    其中幾個值也只在伺服器上（fIndexDownPos、Loader Tray 的 X/Y pitch、LoadForm->XDivision、DeviceForm.ContactMode…）。
//    這裡在開頁（golden FormShow 跑完之後，filerw::PageJson 呼叫 extraJson）照 golden 的式子算好送給頁面；
//    頁面那一半在 web/page/ht9045_golden_kb_unwired.js（Setup.Contact.html 那一段）。
//  golden（906 cContact.cpp，cp950）：
//    edForcePerPinGMouseDown :19029-19050  N_DOUBLE 4 卡範圍（JCET／AMKOR_China：dForcePerpinHigh；其他 +10）～dForcePerpinLow
//    edForcePerPinNMouseDown :18542-18563  同上，dForcePerpinNHigh／NLow（edDieForcePerPinG／N 用同兩支，dfm）
//    edContactOffsetArm1MouseDown :15164-15168  N_DOUBLE 2 dContactHigh～dContactLow（Arm2 同一支）
//    edShtPickOffset1MouseDown :16867-16874  bChangeKitNoHardStop ? dShuttleHigh～Low : iOffsetZHigh～Low（N_DOUBLE 2；Shuttle 2 同一支）
//    edDoubleForceMouseDown :18357-18369  dKitDiameter=DeviceForm_File.dDieForceKitDiameter*10、(d²π/4)*5.0*0.0101972 與 *0.4*，
//                                         MyFormatFloat(…,2)、N_DOUBLE 2 min～max（之後 CountDieForceKg(false)，頁面做）
//  只讀記憶體、不寫檔、不碰機台。⚠ 值是「開頁那一刻」的：開頁之後若有 form.event 改了 dDieForceKitDiameter
//    （rgDieForceKitDiameterClick），要重新開頁才會更新 edDoubleForce 的範圍（頁面的 C 路本來就在每次開窗重送 editlist.get）。
// ===========================================================================
#include <string>

#include "cprod.h"
#include "Config.h"
#include "CosFunction.h"
#include "cmydef.h"
#include "MachineType.h"
#include "EJ1N/TextProcess.h"   // MyFormatFloat（golden :18364）
#include "WebBridge/JsonWriter.h"

namespace {
void Kb(webbridge::JsonWriter& w, const char* id, const char* flags, int dp, double a, double b)
{
    w.Key(id).BeginArray();
    w.String(flags);
    w.Number((wb_int64)dp);
    w.Bool(true);
    w.Number(a);
    w.Number(b);
    w.EndArray();
}
}  // namespace

std::string FileRW_Contact_KbExtraJson()
{
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("kb").BeginObject();
    {
        const bool bPlain = (CUSTOMER_CODE == CC_JCET || CUSTOMER_CODE == CC_AMKOR_China);   // golden :19042／:18555
        const double gHi = bPlain ? InputLimit.dForcePerpinHigh : InputLimit.dForcePerpinHigh + 10;     // :19044／:19048
        const double nHi = bPlain ? InputLimit.dForcePerpinNHigh : InputLimit.dForcePerpinNHigh + 10;   // :18557／:18561
        Kb(w, "edForcePerPinG",    "DOUBLE", 4, gHi, InputLimit.dForcePerpinLow);
        Kb(w, "edDieForcePerPinG", "DOUBLE", 4, gHi, InputLimit.dForcePerpinLow);    // dfm OnMouseDown = edForcePerPinGMouseDown
        Kb(w, "edForcePerPinN",    "DOUBLE", 4, nHi, InputLimit.dForcePerpinNLow);
        Kb(w, "edDieForcePerPinN", "DOUBLE", 4, nHi, InputLimit.dForcePerpinNLow);   // dfm OnMouseDown = edForcePerPinNMouseDown（dfm Enabled=False）
        Kb(w, "edContactOffsetArm1", "DOUBLE", 2, InputLimit.dContactHigh, InputLimit.dContactLow);   // :15167
        Kb(w, "edContactOffsetArm2", "DOUBLE", 2, InputLimit.dContactHigh, InputLimit.dContactLow);
        const double zHi = IniConfig.bChangeKitNoHardStop ? InputLimit.dShuttleHigh : (double)InputLimit.iOffsetZHigh;   // :16870-16873
        const double zLo = IniConfig.bChangeKitNoHardStop ? InputLimit.dShuttleLow  : (double)InputLimit.iOffsetZLow;
        Kb(w, "edShtPickOffset1", "DOUBLE", 2, zHi, zLo);
        Kb(w, "edShtPickOffset2", "DOUBLE", 2, zHi, zLo);
        const double dKitDiameter = DeviceForm_File.dDieForceKitDiameter * 10.0;                       // :18360
        double dDualForceMax = (((dKitDiameter * dKitDiameter * 3.14) / 4.0) * 5.0 * 0.0101972);         // :18361
        double dDualForceMin = (((dKitDiameter * dKitDiameter * 3.14) / 4.0) * 0.4 * 0.0101972);         // :18362
        dDualForceMax = MyFormatFloat(dDualForceMax, 2);                                                 // :18364
        dDualForceMin = MyFormatFloat(dDualForceMin, 2);                                                 // :18365
        Kb(w, "edDoubleForce", "DOUBLE", 2, dDualForceMin, dDualForceMax);                               // :18367
    }
    w.EndObject();
    // golden edDropWaitTimeMouseDown :2103-2238 要的伺服器端值（頁面自己的欄位值它自己讀）
    w.Key("clamp").BeginObject();
    w.Key("hanaMicron").Bool(CUSTOMER_CODE == CC_HANA_MICRON);                                           // :2103
    w.Key("indexDownPos").Number(fIndexDownPos);                                                         // :2122-2125
    w.Key("directContactModeDiffentSpeed").Number((wb_int64)DirectContactModeDiffentSpeed);   // :2127（cbContactMode 的 ItemIndex 頁面讀）
    w.Key("tmoveSlowContact").Bool(DeviceForm.ContactMode == TMoveSlowContact);                          // :2128
    const int t = TrayForm.Loader.iTrayType;
    const bool tOk = (t >= 0 && t < 4);                                                                  // UserDefForm_File[4]（cprod.h）
    w.Key("loaderYPitch").Number(tOk ? UserDefForm_File[t].YPitch : 0.0);                                // :2159-2160
    w.Key("loaderXPitch").Number(tOk ? UserDefForm_File[t].XPitch : 0.0);                                // :2169／:2174-2176
    w.Key("loadFormXDivision").Number((wb_int64)(LoadForm ? LoadForm->XDivision : 0));        // :2168
    w.Key("fixedDropSpeed").Bool(CosFunction.bFixedDropSpeed);                                           // :2186
    w.Key("iFixedDropSpeed").Number((wb_int64)CosFunction.iFixedDropSpeed);                   // :2188-2190
    w.Key("koreaFunction").Bool(IniConfig.bKoreaFunction);                                               // :2219
    w.EndObject();
    w.EndObject();
    return w.Str();
}
