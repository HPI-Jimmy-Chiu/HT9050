// =============================================================================
//  W906FormShowing.h  --  「這個畫面現在開著嗎」的單一函式（取代 golden 的 fXxx->fShow）
//
//  //AI(W906-PAGETAB-Q51) 20260928 [W906] St01（Steven 團隊）。設計：
//    D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\page-state-array.md §3.4
//
//  給不 include csystem.h 的檔（atester、ainarm、AutoClean…）在**同一行**附加 include 用；
//  兩個宣告跟 csystem.h:421／:440 同一行的宣告是同一個東西（重複宣告合法）。本體在 csystem.cpp:30049（ht9045_sm）。
//
//  讀取點一律這樣換（同一行，成員值照傳，C++ 自己開的畫面不會因此失真）：
//      golden:  if(fContact->fShow==false)
//      移植:    if(W906_FormShowing("fContact", fContact->fShow)==false)
//  表單名 ＝ golden 表單名 ＝ D:\HT9045\web\background.html WINDOWS 表的 form:（**不是** id:）。
//
//  回答：member || W906_FormFShowHook(form)。hook 由 wb_serve 經 WebMotorAccessLive.cpp:1142 接到頁面表
//    （WebPageTable.cpp PageFormAnswer，規則 1～7）；ctest／沒有 wb_serve ⇒ hook 是 0 ⇒ 回傳成員值（與改之前相同）。
//
//  程式自己開／關畫面（golden fXxx->Show()／Close() 的移植位置旁，同一行）：
//      W906_FormProgramShow("fHome", true, "uhome.cpp:646 TfHome::Show");
//    hook 是 0（ctest）⇒ 什麼都不做。
// =============================================================================
#ifndef W906FormShowingH
#define W906FormShowingH

bool W906_FormShowing(const char* goldenForm, bool member);
extern void (*W906_FormProgramShowHook)(const char* goldenForm, bool open, const char* where);

inline void W906_FormProgramShow(const char* goldenForm, bool open, const char* where)
{
    if (W906_FormProgramShowHook) W906_FormProgramShowHook(goldenForm, open, where);
}

#endif // W906FormShowingH
