// ===========================================================================
//  JsonBridge/actions/MainRecordClear.cpp
//
//  AI(W906-S119) 20260927 (St02).  act.main.clearRecord／act.main.meShuttle2Dbl —— 說明在 .h 檔頭。
//
//  R1 906 :30090（912 :31137）  UpdateRecordScreen(true) -- 經函式指標 W906_UpdateRecordScreenBody（.h）：本體是 St01 的 S113
//             `W906_TfMain_UpdateRecordScreen`（FileRW/MainRecord.cpp，只編進 wb_serve），本檔也編進 ctest，直接呼叫會連結失敗；
//             St01 在 S113 進 main 時於自己的 wb_serve 開機那一行裝上（St01 06:00／github-59 §4 0261bae3）。沒裝＝跳過這一行。
//             golden 的本體不看 Attr，true／false 做的事一樣（稼動時間累計），沒裝時只是少一拍累計。
// ===========================================================================
#include "JsonBridge/actions/MainRecordClear.h"

#include <string>

#include "JsonBridge/EventLog.h"   // LogAppend（SKILL §4.7 規則 1：每個 act.* 在本體前記一筆）
#include "WebBridge/JsonWriter.h"
#include "Public/cJSON.h"
#include "vclcompat/vcl_compat.h"
#include "Config.h"                // IniConfig.bShowMainDebugRecord
#include "cSocket.h"               // ArmData[3] / TArm::ClearALLCT
#include "forms/fProductionInfo.h" // fProductionInfo->CalculateNowArmSiteBinQty / UpdateControlBinCount
#include "forms/fMain.h"           // fMain->meShuttle1 / meShuttle2

void (*W906_UpdateRecordScreenBody)(bool) = 0;                                  //AI(W906-S119) 20260927: R1，見 .h
//------------------------------------------------------------------------------
// golden 906 main.cpp:30062（912 :31109；NB2 R92 提供；St02 這台沒有解開的 906_20260618）  void __fastcall TfMain::spbClearRecordClick(TObject *Sender)
W906ClearRecordStop W906_SpbClearRecordClick()
{
    if(IniConfig.bShowMainDebugRecord==false)
        return kCrDebugRecordOff;

//    TSpeedButton *Ptr;
//    int pos;
//    Ptr=(TSpeedButton *) Sender;
//    pos=Ptr->Tag;

//    if(pos<0)
//        pos=0;
//
//    if(pos==0)
//    {
//        for(int i=0; i<2; i++)
//        {
//            for(int j=0; j<4; j++)
//            {
//                LastSet.TestCT[3][i*4+j]=ArmData[0]->ArmSKET[i][j]->GetTotal()+ArmData[1]->ArmSKET[i][j]->GetTotal();
//            }
//        }
//    }
    fProductionInfo->CalculateNowArmSiteBinQty(true);
    ArmData[0]->ClearALLCT();
    ArmData[1]->ClearALLCT();
    ArmData[2]->ClearALLCT();
    fProductionInfo->UpdateControlBinCount(true);                               //Sam 20200525 : Control Bin
    if (W906_UpdateRecordScreenBody) W906_UpdateRecordScreenBody(true);         //AI(W906-S119) 20260927: R1 golden UpdateRecordScreen(true)（St01 S113 裝上之前是 NULL）
    return kCrDone;
}
//------------------------------------------------------------------------------
// golden 906 main.cpp:28427（912 :29471；NB2 R92 提供；St02 這台沒有解開的 906_20260618）  void __fastcall TfMain::meShuttle2DblClick(TObject *Sender)
void W906_MeShuttle2DblClick()
{
    fMain->meShuttle2->Clear();                                                 //AI(W906-S119) 20260927: golden TfMain 成員 -> fMain->（forms/fMain.h:222-223）
    fMain->meShuttle1->Clear();
}

namespace ht9045 {
namespace sjson {

namespace {

const char* const kGoldenClear = "906 main.cpp:30062-30091 spbClearRecordClick（912 :31109-31138）";
const char* const kGoldenDbl   = "906 main.cpp:28427-28431 meShuttle2DblClick（912 :29471-29475；AseRecordMemo OnDblClick，912 main.dfm:16259）";

// 同 ChanAction.cpp ParseDryRun（RULINGS_20260926 第 12 條）：沒帶或明確 false ⇒ 執行；明確 true ⇒ 預覽；
// 型別不對或 payload 壞掉 ⇒ 保守當成預覽。空字串＝沒有 payload ⇒ 執行（golden 的按鈕沒有參數）。
bool DryRun(const std::string& payloadJson)
{
    if (payloadJson.empty()) return false;
    cJSON* root = cJSON_Parse(payloadJson.c_str());
    if (root == 0) return true;
    const cJSON* j = cJSON_GetObjectItemCaseSensitive(root, "dryRun");
    const bool dry = !(j == 0 || (cJSON_IsBool(j) && cJSON_IsFalse(j)));
    cJSON_Delete(root);
    return dry;
}

bool FormsReady()
{
    return fMain != 0 && fMain->meShuttle1 != 0 && fMain->meShuttle2 != 0 && fProductionInfo != 0 &&
           ArmData[0] != 0 && ArmData[1] != 0 && ArmData[2] != 0;
}

}  // namespace

std::string DoClearRecordAction(const std::string& payloadJson)
{
    const bool dry = DryRun(payloadJson);
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("golden").String(kGoldenClear);
    w.Key("dryRun").Bool(dry);
    if (!FormsReady()) {
        w.Key("executed").Bool(false);
        w.Key("guard").String("forms-not-created");
        w.Key("detail").String("fMain／fProductionInfo／ArmData[0..2] 有一個還是 NULL");
        w.EndObject();
        return w.Str();
    }
    if (IniConfig.bShowMainDebugRecord == false) {
        w.Key("executed").Bool(false);
        w.Key("guard").String("debug-record-off");
        w.Key("detail").String("IniConfig.bShowMainDebugRecord==false：golden 直接 return（906 main.cpp:30064，912 :31111）；"
                               "預設 false，只有 CC_SIGURD_HUKOU／CC_RICHTEK 打開（CosFunction.cpp:1074／:2959）");
        w.EndObject();
        return w.Str();
    }
    if (dry) {
        w.Key("executed").Bool(false);
        w.Key("guard").String("");           // 沒有守衛擋，是 dryRun 自己停的
        w.Key("note").String("dryRun：只跑守衛，沒有清任何計數");
        w.EndObject();
        return w.Str();
    }

    LogAppend(kLogProcess, "act.main.clearRecord pressed", "", "", "act");
    W906_SpbClearRecordClick();              // 守衛上面已經過了，這裡一定跑完
    w.Key("executed").Bool(true);
    w.Key("guard").String("");
    w.Key("cleared").BeginArray();
    w.String("fProductionInfo->CalculateNowArmSiteBinQty(true)");
    w.String("ArmData[0..2]->ClearALLCT()");
    w.String("fProductionInfo->UpdateControlBinCount(true)");
    if (W906_UpdateRecordScreenBody) w.String("UpdateRecordScreen(true)");
    w.EndArray();
    w.Key("skipped").BeginArray();
    if (!W906_UpdateRecordScreenBody) w.String("UpdateRecordScreen(true) -- W906_UpdateRecordScreenBody 還沒裝（St01 S113，R1）");
    w.EndArray();
    w.EndObject();
    return w.Str();
}

std::string DoMeShuttle2DblAction(const std::string& payloadJson)
{
    const bool dry = DryRun(payloadJson);
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("golden").String(kGoldenDbl);
    w.Key("dryRun").Bool(dry);
    if (!FormsReady()) {
        w.Key("executed").Bool(false);
        w.Key("guard").String("forms-not-created");
        w.Key("detail").String("fMain／meShuttle1／meShuttle2 有一個還是 NULL");
        w.EndObject();
        return w.Str();
    }
    if (dry) {
        w.Key("executed").Bool(false);
        w.Key("guard").String("");
        w.Key("note").String("dryRun：沒有清");
        w.EndObject();
        return w.Str();
    }
    LogAppend(kLogProcess, "act.main.meShuttle2Dbl pressed", "", "", "act");
    W906_MeShuttle2DblClick();
    w.Key("executed").Bool(true);
    w.Key("guard").String("");
    w.Key("note").String("meShuttle1／meShuttle2 目前是 TfMainMemo 替身（forms/FormWidgets.h，Clear 不做事）；"
                         "改指 vclcompat::TMemo 之後（INBOX 第 75 列）才真的清掉內容");
    w.EndObject();
    return w.Str();
}

void WriteClearRecordActionSchema(webbridge::JsonWriter& w)
{
    w.BeginObject();
    w.Key("cmd").String("act.main.clearRecord");
    w.Key("golden").String(kGoldenClear);
    w.Key("args").BeginObject();
    w.Key("dryRun").String("bool，選填；true＝只跑守衛");
    w.EndObject();
    w.Key("guards").BeginArray();
    w.String("forms-not-created -- 移植樹自己的狀態");
    w.String("debug-record-off -- golden 906 :30064（912 :31111）IniConfig.bShowMainDebugRecord==false");
    w.EndArray();
    w.EndObject();
}

void WriteMeShuttle2DblActionSchema(webbridge::JsonWriter& w)
{
    w.BeginObject();
    w.Key("cmd").String("act.main.meShuttle2Dbl");
    w.Key("golden").String(kGoldenDbl);
    w.Key("args").BeginObject();
    w.Key("dryRun").String("bool，選填；true＝不清");
    w.EndObject();
    w.Key("guards").BeginArray();
    w.String("forms-not-created -- 移植樹自己的狀態");
    w.EndArray();
    w.EndObject();
}

}  // namespace sjson
}  // namespace ht9045
