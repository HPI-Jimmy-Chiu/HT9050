// =============================================================================
//  halarm.h  --  golden BCB6 component package D:\HT9045\elec\Component\halarm.h
//
//  AI(W906-HALARM) 20260926: RULINGS_20260926 第 16 條（使用者 10B：把舊電腦的
//  HAlarm.cpp／halarm.h 搬到這台，筆電照 golden 接 mycylin SetAlarm／ClearAlarm）。
//  原始碼：D:\HT9045\elec\Component\halarm.h（39 行）＋HAlarm.cpp（307 行），
//  md5 92d183ba…／54a1d78c…；樹內同一份的 UTF-8 副本在
//  docs/nb2_assist/golden_elec/halarm.h.txt、HAlarm.cpp.txt。
//
//  這個元件不在 golden 原始碼樹裡（它是 BCB6 的 component package），所以 golden
//  樹的窮舉掃描找不到它 —— canary_support.h 第 7 節記了這段來歷。golden 用它的地方：
//    main.cpp:202     HAlarm *Alarm;                    （唯一的全域物件）
//    main.cpp:22472   Alarm = new HAlarm(this);  :22473 Alarm->Clear();   （TfMain::SetInitialData）
//    mycylin.cpp:86/:91   SetAlarm／ClearAlarm → Alarm->Set／Clear
//    Motor/myGALILmotor.cpp:632/:651/:703/:721   Alarm->Set(ALM_MOTOR_MOVE)
//    note.cpp:2531    Alarm->Clear();                   （TfNote::FormClose）
//    ckernel.cpp:377/:2507/:2526   ClearAllAlarm／PopUpAlarm
//    acarry.cpp／ckernel.cpp:679／main.cpp:8214/:8237   讀 SystemNG
//
//  與 golden 不同的地方（都是型別，不是行為）：
//    * `TComponent *` → `void *`。golden 只拿這個指標做「是誰發的」比對
//      （ckernel.cpp:2440 `MOT[i].Motor==Comp`），而唯一的物件 Parent 是 fMain，
//      不是任何馬達 —— 所以那個比對在 golden 裡永遠不成立。移植樹的 TfMain 與
//      HTMotor 沒有共同基底，`void *` 是誠實的「只比身分」型別；
//      `HTMotor* == void*` 在標準 C++ 可以直接比，不用轉型。
//    * `TList` of `int*`／`ERR_MSG*` → std::vector 值。順序與 golden 相同
//      （Add 加在尾、PopUp 取 Items[0]）。
//    * `HMutex` + `HALBusy` 自旋 → 一個 CRITICAL_SECTION（見 HAlarm.cpp）。
//    * `operator == (bool t)` golden 沒寫回傳型別（BCB6 當 int），這裡寫 bool。
//    * `PACKAGE` 巨集拿掉（BCB6 package 匯出用）。
// =============================================================================
//---------------------------------------------------------------------------
#ifndef HAlarmH
#define HAlarmH

#include <vector>
//---------------------------------------------------------------------------
//  CLASS DEFINE
//---------------------------------------------------------------------------
class HAlarm
{
public:
    std::vector<int> ErrNoList;         // 錯誤編號串列         golden halarm.h:10 `TList *ErrNoList;`

    HAlarm(void *P);                    // golden halarm.h:12 `HAlarm(TComponent *P);`
    virtual ~HAlarm();
    bool operator == (bool t) {         // 目前是否有 ALARM     golden halarm.h:14-18
        if (ErrNoList.size() > 0 && t)
            return true;
        return false;
    }
    void    Set(int iNo);           // 設定ALARM
    bool    Clear(int iNo);         // 清除ALARM
    void    Clear();                // 清除所有ALARM
    bool    GetStat(int iNo);       // 取得該ALARM編號錯誤是否存在

protected:
    void *Parent;                       // golden halarm.h:26 `TComponent *Parent;`

private:
    HAlarm(const HAlarm &);             // golden 沒有；TList 指標成員在 golden 也不可複製，這裡明講
    HAlarm &operator=(const HAlarm &);
};

//---------------------------------------------------------------------------
extern bool SystemNG;                   // golden halarm.h:31 `extern bool PACKAGE SystemNG;`（定義在 HAlarm.cpp:14）

//  golden main.cpp:202 `HAlarm  *Alarm;` —— main.cpp 沒有移植，這個全域跟它的類別放在一起
//  （HAlarm.cpp）。在 wb_serve 開機時建立（W906_BootCreateAlarm，對應 golden
//  TfMain::SetInitialData main.cpp:22472-22473）；ctest 與其他工具不建立，維持 NULL。
extern HAlarm *Alarm;

//---------------------------------------------------------------------------
//  FUNCTION PROTOTYPE
//---------------------------------------------------------------------------
void  ClearAllAlarm();                                  // 清除所有物件的錯誤碼       golden halarm.h:36
bool  PopUpAlarm(void **Component,int &iErrCode);      // 取出要顯示的錯誤訊息       golden halarm.h:37
bool  PopUpClrAlarm(void **Component,int &iErrCode);   // 取出要清除顯示的錯誤訊息   golden halarm.h:38

//  AI(W906-HALARM) 20260926: golden TfMain::SetInitialData main.cpp:22472-22473
//  `Alarm = new HAlarm(this); Alarm->Clear();`。pMain 傳 fMain（golden 的 this）。
void  W906_BootCreateAlarm(void *pMain);
//  AI(W906-HALARM-CLOSE) 20260926: golden TfNote::FormClose note.cpp:2531 `Alarm->Clear();`（HAlarm.cpp 檔尾）。
void  W906_NoteFormCloseAlarmClear();
#endif
