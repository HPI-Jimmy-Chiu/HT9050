// ===========================================================================
//  FileRW/_FormEventCtx.h -- WS form.event 跑 golden 處理器「期間」的兩樣東西（本體 FileRW/_FormEvent.cpp 檔尾）。
//
//  //AI(W906-D013) 20260929 [W906]  NOT in golden.  todo D-013（Steven 20260929「照 BCB 的邏輯」）：
//    R126 KYEC 刷條碼機台上 Cleaning 頁的「Reset」—— golden 先跳刷條碼框（V912 AutoClean\uCleaning.cpp:2118
//         Barcode_Reader(bcAutoClean) → BarcodeReader.cpp:415-444 → InputBarcodeNumber :122-145 → TFormBarcodeReader::ShowModal），
//         刷完才歸零。網頁做法：頁面送 Reset 的 form.event；golden 開了框 ⇒ ack 多一個 "barcode" 陣列告訴頁面「要刷」；
//         頁面跳框、操作員刷完，再送一次同一個事件並帶 value 的 "barcode":"<刷到的字>"；C++ 照 golden 的框程式
//         （FormShow → edtBarcodeNumber->Text → TimerKeyIn 檢查 → btnEnterClick → FormClose）拿這段字跑 golden 的檢查。
//    R128 Offset 頁還沒選部位就按微調：處理器要讀 value 裡頁面帶的 "noPart"（FileRW/Offset_File.cpp 檔尾）。
//
//  為什麼另開這個檔、不改 FileRW/_FormEvent.h：_FormEvent.h 由 FileRW/_EditPage.h 帶進每一支 FileRW 的產生檔；
//    改它（連註解）會讓約 40 支 .gen.inc 的 TU 全部重編，formevent::Request 加欄位還是 ODR 版面變更。這裡只放函式宣告，
//    Request 不動（"barcode" 在 FileRW/_FormEvent.cpp 裡解析成區域變數）。
//  只 include <string>；不 include BarcodeReader.h（那是 god-stack，本檔也給只連 FileRW 的 ctest 用）。
//    BarcodeReader.cpp 的兩個掛勾指標（g_W906_BarcodeBoxOpenHook／g_W906_BarcodeBoxClosedHook）由 wb_serve 開機時
//    指到下面兩支（FileRW/TestIF_File_Cleaning.cpp FileRW_Cleaning_EvBoot）；沒裝（每一支別的程式、ctest）＝移植樹原本的
//    離線空殼（BarcodeReader.h (D-1)：框馬上關、什麼都沒刷）。
// ===========================================================================
#pragma once

#include <string>

namespace formevent {

// 目前正在跑的 form.event 的 value 原文（W906_FormEvent 持 FormLock 跑處理器期間；其他時候是空字串）。
// 給處理器讀頁面帶的額外鍵（例 Offset 的 "noPart"）；W906_FormEvent 不認得的鍵照舊不理，型別由讀的人自己驗。
const std::string& CurrentValueJson();

// golden 的條碼輸入框開了一次（BarcodeReader.cpp InputBarcodeNumber 的 ShowModal 那一行呼叫）。
//   回 false：現在不是 form.event（開頁、存檔、主迴圈…）⇒ 呼叫端照移植樹原本的離線空殼，什麼都不做。
//   回 true ：這一次 form.event 裡的框；*haveScan＝頁面這一次有沒有帶刷到的字（value 的 "barcode"；一次事件只給第一個框，
//             golden 一個框刷一次），*scan＝那段字。框的標題／種類記下來，ack 的 "barcode" 會列出。
bool BarcodeBoxOpen(const char* caption, const char* inputType, bool* haveScan, std::string* scan);

// 同一個框關掉時 golden 交回呼叫端的字（FormClose 之後的 sBarcodeInfo；空＝沒刷，或被 golden 的檢查清掉）。
void BarcodeBoxClosed(const char* result);

}  // namespace formevent
