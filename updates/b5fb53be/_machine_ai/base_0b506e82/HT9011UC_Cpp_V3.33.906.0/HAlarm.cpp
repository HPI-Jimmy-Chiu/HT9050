// =============================================================================
//  HAlarm.cpp  --  golden BCB6 component package D:\HT9045\elec\Component\HAlarm.cpp
//
//  AI(W906-HALARM) 20260926: RULINGS_20260926 第 16 條。逐函式照 golden 翻；
//  行號引用都是 golden HAlarm.cpp 的行號（樹內 UTF-8 副本
//  docs/nb2_assist/golden_elec/HAlarm.cpp.txt 行號相同）。型別上的差異見 halarm.h 檔頭。
//
//  取代的東西：canary_support.cpp 第 7 節的「警報佇列接縫」（W906-W7-L2 20260803）
//  只翻了 PopUpAlarm／ClearAllAlarm 兩個出口，沒有 HAlarm 物件本身 —— 沒有 Set 的去重、
//  沒有 Clear(code)、沒有 UpdateSystemNG，所以氣缸（mycylin.cpp SetAlarm／ClearAlarm）
//  與 Galil 馬達（GATE W4G-4）都只能是空殼。那一節已退役，由本檔取代。
//
//  鎖：golden 用 `HMutex *ThMutex`（HThread.h）加一個 `static bool HALBusy` 自旋
//  （`while(HALBusy); HALBusy=true;`）。這裡用一個 CRITICAL_SECTION 取代 ThMutex，
//  HALBusy 不翻：在 CS 之內它不提供任何額外的互斥，而在標準 C++ 的記憶體模型下
//  對一般 bool 自旋是資料競爭（編譯器可以把 `while(HALBusy);` 變成無窮迴圈）。
//  CRITICAL_SECTION 可重入，golden Clear() → Clear(code)、Set() → GetStat() 的
//  巢狀取用行為不變。
//
//  NULL：golden 的三個串列在第一個 HAlarm 建構時才配置（:37-47），而 golden 在
//  TfMain::SetInitialData（main.cpp:22472）就建好唯一的 Alarm，所以 golden 的
//  ClearAllAlarm／PopUpAlarm／UpdateSystemNG 從來不會遇到 NULL 串列。移植樹的 ctest
//  與其他工具不建 Alarm，而 ckernel ProcessAlarm（csystem.cpp:16303 每個 DoSystem 週期）
//  照樣會叫 PopUpAlarm／ClearAllAlarm —— 所以那三個函式在串列尚未配置時照
//  「沒有任何警報」處理（等於 golden 空串列的那一臂），不是新行為。
// =============================================================================
#include "halarm.h"

#include <windows.h>
#include <cstdio>
#include <cstdlib>

//---------------------------------------------------------------------------
bool    SystemNG = false;       // 系統故障                         golden :14 `bool SystemNG;`

//  golden main.cpp:202 `HAlarm  *Alarm;`（見 halarm.h）
HAlarm *Alarm = NULL;

namespace {
struct ERR_MSG {                // 錯誤訊息結構                     golden :16-19
    void        *ObjPtr;        // 物件指標
    int         iErrCode;       // 錯誤碼
};
} // namespace

static std::vector<HAlarm *>  *HAlarmList     = NULL;   // 系統錯誤串列   golden :21
static std::vector<ERR_MSG>   *ShowAlarmList  = NULL;   // 顯示錯誤串列   golden :22
static std::vector<ERR_MSG>   *ClearAlarmList = NULL;   // 清除錯誤串列   golden :23
static CRITICAL_SECTION       *ThMutex        = NULL;   //               golden :24 `static HMutex *ThMutex`
//  golden :25 `static bool HALBusy = false;` —— 不翻，理由見檔頭「鎖」。
//---------------------------------------------------------------------------
static void UpdateSystemNG();   // 更新系統錯誤狀態                 golden :27

static void MutexUse()      { EnterCriticalSection(ThMutex); }   // golden ThMutex->Use()     // 申請使用Mutex
static void MutexRelease()  { LeaveCriticalSection(ThMutex); }   // golden ThMutex->Release() // Free Mutex

//---------------------------------------------------------------------------
//  ALARM建構子                                                     golden :29-51
//  Parent:本Alarm的擁有物件
//---------------------------------------------------------------------------
HAlarm::HAlarm(void *_Parent)
{
    // golden :35 `ErrNoList = new TList;` —— ErrNoList 是成員 vector，建構即空。

    if (ThMutex == NULL)
    {
        ThMutex = new CRITICAL_SECTION;
        InitializeCriticalSection(ThMutex);
    }

    if (HAlarmList == NULL)
        HAlarmList = new std::vector<HAlarm *>;

    if (ShowAlarmList == NULL)
        ShowAlarmList = new std::vector<ERR_MSG>;

    if (ClearAlarmList == NULL)
        ClearAlarmList = new std::vector<ERR_MSG>;

    HAlarmList->push_back(this);                                    // golden :49 HAlarmList->Add(this)
    Parent = _Parent;
}

//---------------------------------------------------------------------------
//  解構子(清除配置區域)                                            golden :53-105
//  GOLDEN QUIRK PRESERVED（:60-69、:72-81）：刪掉一筆之後把 iP 設成 0，接著 for 的
//  iP++ 讓它從 1 開始 —— 第 0 筆若也符合就會被跳過。只影響解構，而 golden 唯一的
//  Alarm 活到程式結束，從來不解構；照原樣保留，不修。
//---------------------------------------------------------------------------
HAlarm::~HAlarm()
{
    // 先移除顯示串列上的相同ALARM .........................................
    for (int iP = 0;iP < (int)ShowAlarmList->size();iP ++)
    {  // 找顯示串列
        if ((*ShowAlarmList)[iP].ObjPtr == Parent)
        {
            ShowAlarmList->erase(ShowAlarmList->begin() + iP);
            iP = 0;
        }
    }

    // 再移除清除串列上的相同ALARM .........................................
    for(int iP=0; iP<(int)ClearAlarmList->size(); iP++)
    {
        if ((*ClearAlarmList)[iP].ObjPtr == Parent)
        {
            ClearAlarmList->erase(ClearAlarmList->begin() + iP);
            iP = 0;
        }
    }

    // 從ALARM串列中移除
    for(int iP=0; iP<(int)HAlarmList->size(); iP++)
    {
        if((*HAlarmList)[iP]==this)
        {
            HAlarmList->erase(HAlarmList->begin() + iP);
            break;
        }
    }

    Clear();                            // 清掉錯誤碼串列
    // golden :94 `delete ErrNoList;` —— 成員 vector，隨物件釋放。

    if(HAlarmList->size()==0)           // 如果己經沒有ALARM物作存在
    {
        delete HAlarmList;
        delete ShowAlarmList;
        delete ClearAlarmList;
        DeleteCriticalSection(ThMutex);
        delete ThMutex;
        HAlarmList      = NULL;
        ShowAlarmList   = NULL;
        ClearAlarmList  = NULL;
        ThMutex         = NULL;
    }
}
//---------------------------------------------------------------------------
//  設定ALARM                                                       golden :107-137
//---------------------------------------------------------------------------
void HAlarm::Set(int iCode)
{
    if(GetStat(iCode))                  // 如果此一訊息己存在
        return ;

    try
    {
        MutexUse();                     // 申請使用Mutex
        ErrNoList.push_back(iCode);     // 在串列上加上錯誤碼

        ERR_MSG Msg;
        Msg.ObjPtr = Parent;            // 錯誤發生的物件指標
        Msg.iErrCode = iCode;           // 錯誤碼
        ShowAlarmList->push_back(Msg);  // 在顯示的錯誤串列中加入訊息
        MutexRelease();                 // Free Mutex
    }
    catch (...)
    {
        std::fprintf(stderr, "無法配置系統錯誤訊息\n");   // golden :133 ShowMessage("無法配置系統錯誤訊息");
        std::abort();
    }
    UpdateSystemNG();
}

//---------------------------------------------------------------------------
//  清除ALARM                                                       golden :139-200
//---------------------------------------------------------------------------
bool HAlarm::Clear(int iECode)
{
    bool Flag=false;

    MutexUse();                                     // 申請使用Mutex
    for(int iP=0; iP<(int)ErrNoList.size(); iP++)
    {
        if(ErrNoList[iP]==iECode)                   // 如果串列中的錯誤碼相同
        {
            ErrNoList.erase(ErrNoList.begin() + iP);    // 刪除該節點
            Flag=true;
            break;
        }
    }
    if(!Flag)
    {
        MutexRelease();
        return false;
    }

    // 先移除顯示串列上的相同ALARM .........................................
    for(int iP=(int)ShowAlarmList->size()-1; iP>=0; iP--)
    {
        const ERR_MSG &P=(*ShowAlarmList)[iP];
        if(P.ObjPtr  ==Parent &&                    // 如果是本錯誤物件
           P.iErrCode==iECode)                      // 且錯誤編號相同
        {
            ShowAlarmList->erase(ShowAlarmList->begin() + iP);  // 將顯示串列中的移除
            break;
        }
    }

    // 將本ALARM加到清除串列上..........................................
    //  golden :190 註：golden 裡沒有任何人呼叫 PopUpClrAlarm（全樹 0 處），所以這個串列
    //  每清一次警報就長一筆、永不消化。照原樣保留。
    try
    {
        ERR_MSG Msg;
        Msg.ObjPtr  =Parent;         // 錯誤發生的物件指標
        Msg.iErrCode=iECode;         // 錯誤碼
        ClearAlarmList->push_back(Msg);  // 在顯示清除的串列中加入訊息
    }
    catch (...)
    {
        std::fprintf(stderr, "無法配置清除系統錯誤訊息\n");   // golden :193 ShowMessage("無法配置清除系統錯誤訊息");
        MutexRelease();              // Release Mutex
        std::abort();
    }

    MutexRelease();
    UpdateSystemNG();

    return true;
}
//---------------------------------------------------------------------------
//  清除所有ALARM                                                   golden :201-211
//---------------------------------------------------------------------------
void HAlarm::Clear()
{
    for(int iP=0; iP<(int)ErrNoList.size(); iP=0)
    {
        int P=ErrNoList[iP];
        Clear(P);
    }
}
//---------------------------------------------------------------------------
//  取得該ALARM編號錯誤是否存在                                     golden :212-230
//---------------------------------------------------------------------------
bool HAlarm::GetStat(int iECode)
{
    MutexUse();
    bool bResult=false;

    for(int iP=0; iP<(int)ErrNoList.size(); iP++)
    {
        if (ErrNoList[iP]==iECode)
        {
            bResult=true;
            break;
        }
    }
    MutexRelease();
    return bResult;
}
//---------------------------------------------------------------------------
//  取得該ALARM編號錯誤是否存在（golden :231-233 的標題就是這樣寫的；函式本體是清除全部）
//                                                                  golden :234-242
//---------------------------------------------------------------------------
void ClearAllAlarm()
{
    if(HAlarmList!=NULL)            // 移植樹：沒建過 HAlarm（ctest）＝沒有任何警報，見檔頭「NULL」
    {
        for(int iP=0; iP<(int)HAlarmList->size(); iP++)
        {
            HAlarm *P=(*HAlarmList)[iP];
            P->Clear();
        }
    }
    SystemNG=false;
}
//---------------------------------------------------------------------------
//  更新系統錯誤狀態                                                golden :243-258
//---------------------------------------------------------------------------
void UpdateSystemNG()
{
    bool Flag=false;

    for(int iP=0; iP<(int)HAlarmList->size(); iP++)     // 只從 Set／Clear 進來，那時 HAlarmList 一定已配置
    {
        HAlarm *P=(*HAlarmList)[iP];
        if(*P==true)                // 如果有機構上有錯誤碼
        {
            Flag=true;
            break;
        }
    }
    SystemNG=Flag;
}
//---------------------------------------------------------------------------
//  取出要顯示的錯誤訊息                                            golden :259-284
//---------------------------------------------------------------------------
bool PopUpAlarm(void **Component, int &iErrCode)
{
    if(ShowAlarmList!=NULL && ShowAlarmList->size()>0)  // 移植樹多一個 NULL 判斷，見檔頭「NULL」
    {
        MutexUse();                          // 申請使用Mutex
        ERR_MSG P = (*ShowAlarmList)[0];

        *Component = P.ObjPtr;              // 指向錯誤物件
        iErrCode   = P.iErrCode;            // 傳回錯誤碼
        ShowAlarmList->erase(ShowAlarmList->begin());
        MutexRelease();                      // Release Mutex
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
//  取出要清除顯示的錯誤訊息                                        golden :285-307
//---------------------------------------------------------------------------
bool PopUpClrAlarm(void **Component,int &iErrCode)
{
    if (ClearAlarmList!=NULL && ClearAlarmList->size() > 0)  // 移植樹多一個 NULL 判斷，見檔頭「NULL」
    {
        MutexUse();                          // Use Mutex
        ERR_MSG P = (*ClearAlarmList)[0];

        *Component = P.ObjPtr;              // 指向錯誤物件
        iErrCode   = P.iErrCode;            // 傳回錯誤碼
        ClearAlarmList->erase(ClearAlarmList->begin());
        MutexRelease();                      // Release Mutex
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
//  AI(W906-HALARM) 20260926: golden TfMain::SetInitialData main.cpp:22472-22473。
//  golden 在 TfMain 建構子（main.cpp:2120 SetInitialData()）裡做，早於
//  SetMyKitSuckItemAmount（:2121）與任何機構動作；wb_serve 在同一個相對位置呼叫
//  （tools/wb_serve.cpp，SetMyKitSuckItemAmount 之前）。
//---------------------------------------------------------------------------
void W906_BootCreateAlarm(void *pMain)
{
    Alarm = new HAlarm(pMain);
    Alarm->Clear();
}
//---------------------------------------------------------------------------
