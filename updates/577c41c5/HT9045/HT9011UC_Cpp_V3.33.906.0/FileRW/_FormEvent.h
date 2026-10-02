// ===========================================================================
//  FileRW/_FormEvent.h -- WS form.event：golden 表單的「控制項事件」（例：下拉 OnChange 選一筆就自動填欄位）。
//
//  AI(W906-FRW-S157) 20260927 [W906]  NOT in golden.
//  裁決：Steven ★ Q40＝A（RULINGS_20260926 S157）——HotPlate、TrayForm、Cleaning 三頁共用一個 WS 指令。
//  格式：St01 FROM_STEVEN 20260927 10:15（Jimmy TO_STEVEN 20260927 10:2x 同意；頁面送出點在 Jimmy 的
//        web/page/ht9045_wire_engine.js，只有標 data-ht-event 的控制項才送）。
//
//  送（WS）  cmd:"form.event"  tag:"<頁名>"（"Setup.HotPlate"；也收 "Setup.HotPlate.html" 與 C 路結構名）
//            value:JSON 字串 {"form":"TfHotPlate","control":"cbSelectHPFromDB","event":"change",
//                             "itemIndex":3,"text":"QFN2X2","checked":null,"state":{…選用，格式同 form.save 的 widgets…}}
//  回（ack） 成功 ok:true ＋ {"form","control","event","golden","changed":{名稱:{text?,itemIndex?,checked?,items?,
//            enabled?,visible?…}},"messages":[{en,zh}],"todo":[…]}（changed 的鍵名同 /api/form 第二型 display 的
//            widget；沒變的鍵不出現，不送 null）
//            失敗 ok:false ＋ error:"<碼>: <說明>"，碼：unknown-page、unknown-control、no-handler、bad-payload、
//            running（SystemStart||SoftStart；例外：檔尾 formevent::runexc 的表，AI(W906-FE-RUNEXC) 20260930）、handler-failed；busy: 由 tools/wb_serve.cpp 分派迴圈頭的
//            WebCmdGuard 回（同 cmd＋tag＋value 400 ms 內重複）。
//
//  分派：A 形狀（JsonBridge/FormBridge.h BridgeDesc::events，tools/gen_formbridge.py）→ formbridge::RunEvent；
//        C 路（FileRW/_EditPage.h PageEvent 表，tools/gen_editlist.py）→ filerw::RunPageEvent。
//
//  //AI(W906-EVB1) 20260928 [W906] X-2（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md B1）：value 多兩個選用鍵，
//    都是「控制項自己的新值」，只給 C 路（A 形狀 Setup.HotPlate 沒有這兩種元件的事件 ⇒ 帶了回 bad-payload）：
//      "position":<整數>        TTrackBar／TScrollBar／TUpDown（C 路三種都是 filerw::ELTrackBar 替身）的 OnChange（event "change"）。
//                               RunPageEvent 先照 VCL 設 Position（夾 Min..Max、更新 Associate、不另外觸發 OnChange）再跑表上的
//                               golden 處理器（＝那個 OnChange）一次。被夾過 ⇒ changed 帶 {"<control>":{"position":<實際值>}}、todo 記一筆。
//      "activePageIndex":<整數> TPageControl 的 OnChange（event "change"）：新分頁（golden DFM 頁序、0 起算＝VCL PageIndex）。
//                               範圍／點得到照 editlist.save 的同一種值（FileRW/_EditList.h ELPageIndexRefused，c9d3c932）；
//                               不合法 ⇒ bad-payload（使用者點不到不存在的分頁；editlist.save 那邊是只丟那一筆，不同）。
//                               先設 ActivePageIndex 再跑處理器（VCL：換頁之後才 OnChange）。
//    null＝沒帶。帶在別種元件、或事件不是 "change"（例 IniConfig udD46 的 btNext／btPrev：伺服器自己照 Increment 走一格）⇒ bad-payload。
//    沒帶這兩個鍵＝舊行為（處理器看到伺服器端目前的 Position／分頁），既有請求完全不變。
// ===========================================================================
#pragma once

#include <string>

namespace formevent {

// 頁面送來的一次事件（W906_FormEvent 解析好、驗過型別）。
struct Request {
    std::string form;          // golden 表單類別（頁面帶的；和分派到的表單不同 ⇒ bad-payload）
    std::string control;       // golden 元件名（＝頁面元件 id）
    std::string event;         // "change"／"click"
    bool hasIndex = false;     int itemIndex = -1;
    bool hasText = false;      std::string text;
    bool hasChecked = false;   bool checked = false;
    std::string stateJson;     // 頁面其他控制項目前的值 {名稱:{text?,itemIndex?,checked?}}；沒送＝空字串
    //AI(W906-EVB1) 20260928 [W906] X-2：控制項自己的新值（見檔頭）；放在最後，既有欄位的位置不動
    bool hasPosition = false;  int position = 0;          // TTrackBar／TScrollBar／TUpDown（ELTrackBar）的 OnChange
    bool hasPageIndex = false; int activePageIndex = -1;  // TPageControl 的 OnChange
};

// 一次事件的結果。code 空＝成功。
struct Result {
    std::string code;          // unknown-control／no-handler／bad-payload／handler-failed
    std::string why;
    std::string golden;        // 跑到的 golden 處理器位置（成功時回給頁面）
    std::string changedJson = "{}";
    std::string messagesJson = "[]";
    std::string todoJson = "[]";  bool closed = false;   //AI(W906-EVB10A) 20260929 [W906]：golden 處理器呼叫了 Close()（C 路 RunPageEvent 的 "closed" 記號）⇒ ack.closed；接在同一行
};

}  // namespace formevent

// tools/wb_serve.cpp 的 form.event 臂呼叫（主迴圈；本體自己持 FormLock）。回 true＝成功，*ack 是 JSON 物件
// （不含 ok —— WebBridgeServer AckJson 會加）；回 false＝*err 是 "<碼>: <說明>"。
bool W906_FormEvent(const std::string& tag, const std::string& valueJson, std::string* ack, std::string* err);

//AI(W906-FE-RUNEXC) 20260930 [W906]：運轉中例外表（本體與每一列的 golden 出處在 FileRW/_FormEvent.cpp 檔尾；
//  D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md P-3）。W906_FormEvent 第 2 步：SystemStart||SoftStart 時
//  只有 Allowed 回 true 的事件往下走（之後的檢查照舊），其他照舊回 running。
namespace formevent {
namespace runexc {
struct RowInfo {
    const char* form;       // golden 表單類別（W906_FormEvent 分派到的 formClass，例 "TfOffSet"）
    const char* control;    // golden 元件名
    const char* event;      // "click"／"change"
    const char* shownObj;   // golden 表單物件名（W906_FormShowing 的鍵＝D:\HT9045\web\background.html WINDOWS 的 form:）
    bool systemStart;       // SystemStart 時放行
    bool softStart;         // SoftStart 時放行
    const char* golden;     // golden 出處（V912）
};
// true＝這一次事件運轉中照 golden 放行；false＝照舊拒收，*why 非空＝表上有這一列、但狀態格或頁面表不成立的理由
bool Allowed(const std::string& formClass, const std::string& valueJson, std::string* why);
int RowCount();                 // 測試用：表有幾列
const RowInfo* RowAt(int i);    // 測試用：第 i 列（超出範圍回 nullptr）
}  // namespace runexc
}  // namespace formevent

//AI(W906-B8-OS1B) 20261001 [W906]：form.event 的 after-ack 動作（本體與理由在 FileRW/_FormEvent.cpp 檔尾；
//  D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「OS-1b」）。golden 處理器最後叫 fMain->Start 那種會開等待框的動作，
//  不能在 W906_FormEvent 持 FormLock 時做：處理器只登記，wb_serve 的 form.event 臂在回覆送出之後（鎖外、同一條主迴圈執行緒）才跑。
namespace formevent {
namespace afterack {
// 處理器期間登記一個動作：fn(arg)；what＝給頁面看的一行說明（ack "afterAck" 陣列的一項）。fn 是 0 ⇒ 不登記。
void Defer(void (*fn)(const std::string& arg), const std::string& arg, const std::string& what);
void Reset();     // 作廢目前登記的（W906_FormEvent 開頭與處理器失敗時呼叫）
int Pending();    // 測試用：目前登記了幾個
}  // namespace afterack
}  // namespace formevent
// tools/wb_serve.cpp 的 form.event 臂在 CompleteCommand 之後呼叫（ok＝W906_FormEvent 的回傳）：ok 才照登記順序跑，否則丟掉；跑完清空。
void W906_FormEventRunAfterAck(bool ok);
