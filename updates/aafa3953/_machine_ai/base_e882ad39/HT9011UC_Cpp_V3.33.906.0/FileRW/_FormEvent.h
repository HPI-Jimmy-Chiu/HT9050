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
//            running（SystemStart||SoftStart）、handler-failed；busy: 由 tools/wb_serve.cpp 分派迴圈頭的
//            WebCmdGuard 回（同 cmd＋tag＋value 400 ms 內重複）。
//
//  分派：A 形狀（JsonBridge/FormBridge.h BridgeDesc::events，tools/gen_formbridge.py）→ formbridge::RunEvent；
//        C 路（FileRW/_EditPage.h PageEvent 表，tools/gen_editlist.py）→ filerw::RunPageEvent。
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
};

// 一次事件的結果。code 空＝成功。
struct Result {
    std::string code;          // unknown-control／no-handler／bad-payload／handler-failed
    std::string why;
    std::string golden;        // 跑到的 golden 處理器位置（成功時回給頁面）
    std::string changedJson = "{}";
    std::string messagesJson = "[]";
    std::string todoJson = "[]";
};

}  // namespace formevent

// tools/wb_serve.cpp 的 form.event 臂呼叫（主迴圈；本體自己持 FormLock）。回 true＝成功，*ack 是 JSON 物件
// （不含 ok —— WebBridgeServer AckJson 會加）；回 false＝*err 是 "<碼>: <說明>"。
bool W906_FormEvent(const std::string& tag, const std::string& valueJson, std::string* ack, std::string* err);
