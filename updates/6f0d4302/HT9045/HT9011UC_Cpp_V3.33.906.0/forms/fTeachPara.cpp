// ===========================================================================
//  forms/fTeachPara.cpp  --  TECH_PARA / TECH_TWOPARA / TECH_MotorAxle /
//                            TECH_SUCKPARA 的本體，以及 TfTeach::ReadFile()。
//
//  翻譯波次：AI(W906-TEACH-W1) 20260919
//  golden：`uteach.cpp:61-252`（四個類別）、`uteach.cpp:4781-4920`（ReadFile）
//
//  ⚠ 設計、偏離、以及每一處閘的理由**全部寫在 forms/fTeachPara.h 的檔頭**，
//    不在這裡重複。動這個檔之前先讀那份。
//
//  一句話版本：移植樹的 `TfTeach` 沒有 widget 實例（661 個識別字，
//  `forms/fTeach.h:330-343` 明文 DEFERRED），所以 golden 碰 widget 的每一個
//  deref 都加了 `!=NULL` 保護並就地註明；讀取路徑（`Parameter` /
//  `MotorSelect` / `Key`）**一字未改**。三支 `SaveToFile` 以 widget 為
//  **資料來源**（寫的是 `SetEdit->Text`），沒有 widget 就沒有要寫的字串，
//  所以本體閘住 —— 那是「相依不存在」，不是「怕寫檔」。
//
//  ---------------------------------------------------------------------------
//  ⚠ 第二處偏離（被編譯器逼的，不是選擇）：`(int)(*Parameter)=` -> `(*Parameter)=`
//  ---------------------------------------------------------------------------
//  golden 全檔一律寫成
//
//      (int)(*Parameter)=CheckAndReadIniData(...);
//
//  也就是把左值 C-style 轉型成 `int` **之後再賦值**。BCB6 接受這個寫法
//  （它把 C-style 轉型的結果當左值）；**C++17 / g++ 不接受** ——
//  轉型結果是純右值，`error: lvalue required as left operand of assignment`。
//
//  ⇒ 本檔一律寫成 `(*Parameter)=`。這是**零語意差異**的機械替換：
//    `Parameter` 宣告就是 `int*`（golden uteach.h:25），所以 `*Parameter`
//    本來就是 `int`，那個轉型從頭到尾是 no-op。右側的
//    `CheckAndReadIniData` 也已經是 `int` 多載（common.h:223）。
//  ⓘ 一共 9 處：TECH_PARA::ReadFromFile 6（含 else 的 =0）、TECH_TWOPARA::ReadFromFile 2、
//    TECH_SUCKPARA::ReadFromFile 1，加上兩支被閘住的 SaveToFile 裡的
//    （閘住的那些**保留 golden 原樣**，因為它們不編譯，留著才好對帳）。
// ===========================================================================
#include "forms/fTeachPara.h"
#include "forms/fTeach.h"          // TfTeach（ReadFile 是它的方法）

#include "common.h"                // asTeachPath / CheckAndReadIniData /
                                   // CheckSectionExist / CheckKeyExist / WriteIniData
#include "cmydef.h"                // MACHINE_HAS_AUTO_ALIGNMENT_CCD / USE_OUT_SORT_ARM / eartUninstall
#include "cprod.h"                 // Teach（Teach_Pos，:3236）、ReadData（:3240）
#include "LastSet.h"               // Tech（TECH，:1154）
#include "Motor/mymotor.h"         // MOT[] / .Alias
#include "Public/HTEditList.h"     // elTeach（:256，**裸指標，從未建構**）

#include "vclcompat/SysUtils.h"    // FileExists / ExtractFileName / ExtractFilePath

#include <cstdlib>                 // atoi

// ---------------------------------------------------------------------------
//  golden uteach.cpp:43 —— `int ... TECH_MAX_ITEM=0, TechTwoItem=0;`
//  （同一行的 SelMotSpeed / iAuto_TeachPitch / Tech_Part 不屬於本波，
//    等它們的消費者落地時由那一波帶進來 —— 提前定義等於替別人的波次
//    先佔名字，而那個名字的語意還沒被翻譯過。）
// ---------------------------------------------------------------------------
int TECH_MAX_ITEM = 0;
int TechTwoItem   = 0;

//----------------------------------------------------------------------------
//  golden uteach.cpp:61-75
//----------------------------------------------------------------------------
TECH_PARA::TECH_PARA(int *Par1, int MotorNo, TEdit *EdtSet, AnsiString _Key, TSpeedButton *SPBFun, TSpeedButton *SPBGo, bool Visible)
{
    Parameter       =Par1;
    MotorSelect     =MotorNo;
    SetEdit         =EdtSet;
    funButton       =SPBFun;
    btGo            =SPBGo;
    Key             =_Key;

    //AI(W906-TEACH-W1) 20260919: golden :70 是 `SetEdit->Visible=Visible;`，
    // 沒有 NULL 檢查 —— 在 golden 那邊 SetEdit 一定是表單上的一顆 TEdit。
    // 本樹沒有 widget 實例（見 fTeachPara.h 檔頭），登錄表傳進來的是 0，
    // 所以這裡加保護。golden 自己對 funButton / btGo **就有**同樣的檢查
    // （:71-74），所以這個形狀不是我發明的，是把 golden 的第一行補齊成
    // 它對另外兩個成員已經採用的寫法。
    if(SetEdit!=NULL)
        SetEdit->Visible=Visible;
    if(funButton!=NULL)
        funButton->Visible=Visible;
    if(btGo!=NULL)
        btGo->Visible=Visible;
}

//----------------------------------------------------------------------------
//  golden uteach.cpp:77-116  —— **讀取路徑的核心**，一字未改。
//
//  四個 Shuttle 區段拼法變體的特例是 golden 自己的（Steven 20250926）：
//  `Mot_Table.csv` 把 in-shuttle 的 Alias 寫成舊拼法 `MInShutte1/2`，
//  而 teach.ini 兩種拼法都可能存在。實測這台機器：四種區段
//  `[MInShuttle1]` `[MInShuttle2]` `[MInShutte1]` `[MInShutte2]` **各 1 個**，
//  且 `MOT[].Alias` 是舊拼法 —— 所以走的是 :79 那一支。
//----------------------------------------------------------------------------
void TECH_PARA::ReadFromFile()                                                  //Steven 20240501 : Teach改存成ini
{
    if(MOT[MotorSelect].Alias=="MInShutte1")                                    //Steven 20250926 : Fixed for Shuttle Teach
    {
        if(CheckSectionExist(asTeachPath, "MInShutte1"))
            (*Parameter)=CheckAndReadIniData(asTeachPath, "MInShutte1", Key, (int)(*Parameter));
        else
            (*Parameter)=CheckAndReadIniData(asTeachPath, "MInShuttle1", Key, (int)(*Parameter));
    }
    else if(MOT[MotorSelect].Alias=="MInShuttle1")
    {
        if(CheckSectionExist(asTeachPath, "MInShuttle1"))
            (*Parameter)=CheckAndReadIniData(asTeachPath, "MInShuttle1", Key, (int)(*Parameter));
        else
            (*Parameter)=CheckAndReadIniData(asTeachPath, "MInShutte1", Key, (int)(*Parameter));
    }
    else if(MOT[MotorSelect].Alias=="MInShutte2")
    {
        if(CheckSectionExist(asTeachPath, "MInShutte2"))
            (*Parameter)=CheckAndReadIniData(asTeachPath, "MInShutte2", Key, (int)(*Parameter));
        else
            (*Parameter)=CheckAndReadIniData(asTeachPath, "MInShuttle2", Key, (int)(*Parameter));
    }
    else if(MOT[MotorSelect].Alias=="MInShuttle2")
    {
        if(CheckSectionExist(asTeachPath, "MInShuttle2"))
            (*Parameter)=CheckAndReadIniData(asTeachPath, "MInShuttle2", Key, (int)(*Parameter));
        else
            (*Parameter)=CheckAndReadIniData(asTeachPath, "MInShutte2", Key, (int)(*Parameter));
    }
    else if(MOT[MotorSelect].Alias!="")
    {
        (*Parameter)=CheckAndReadIniData(asTeachPath, MOT[MotorSelect].Alias, Key, (int)(*Parameter));
    }
    else
    {
        (*Parameter)=0;
    }
    //AI(W906-TEACH-W1) 20260919: golden :115 `SetEdit->Text=(int)(*Parameter);`
    // —— 純 UI 鏡像，讀取路徑不經過它。保護見檔頭。
    if(SetEdit!=NULL)
        SetEdit->Text=(int)(*Parameter);
}

//----------------------------------------------------------------------------
//  ⚠⚠ 使用者裁決 20260919（選項 b）—— 三支 `SaveToFile` 的**明確偏離**
//----------------------------------------------------------------------------
//  使用者原話：「你裁決一個明確偏離：SaveToFile 直接寫 *Parameter 的值，
//  不經過輸入框」。
//
//  golden 寫進 ini 的是 **widget 的文字**，不是變數：
//      WriteIniData(asTeachPath, ..., Key, SetEdit->Text);
//  移植樹沒有 widget 實例，所以改寫成 `AnsiString((int)(*Parameter))`。
//
//  兩條路的等價性，逐條講清楚（這是這個偏離唯一需要被審的地方）：
//
//  * `bSaveByTeach==true`（golden :120-121 那一支）
//      golden：先 `SetEdit->Text=(int)(*Parameter)`，再寫 `SetEdit->Text`。
//      淨效果就是「把 *Parameter 的十進位字串寫進 ini」。
//      ⇒ **完全等價**，連寫進去的位元組都一樣（AnsiString(int) 產生同一個
//         十進位字串，vclcompat/AnsiString.h:67）。
//
//  * `bSaveByTeach==false`（golden :122-123 那一支）
//      golden：先 `(int)(*Parameter)=atoi(SetEdit->Text)` —— 也就是**把操作員
//      在畫面上打的字讀回變數**，再把那個字寫進 ini。
//      移植樹**沒有操作員輸入的通道**（那顆輸入框不存在），所以沒有「操作員
//      打的字」這個東西可讀。
//      ⇒ 這一支**行為不同**：移植樹寫的是 `*Parameter` 現值，而且**不會**
//         反向更新 `*Parameter`。
//      ⇒ 實務後果：從網頁存教導值時，存下去的是 C++ 端目前持有的值。
//         等 widget 層或 web 寫入通道落地，這裡要改回讀那個通道。
//
//  ⓘ 呼叫點現況（量過）：`bSaveByTeach==false` 在本樹**沒有任何呼叫點** ——
//    `TfTeach::ReadFile()` 的三處全部傳 `true`（golden :4823/:4826/:4828/
//    :4829/:4832/:4849）。所以今天這個差異是**不可達**的；它會在
//    `TfTeach::SaveFile()`（golden uteach.h:2378，尚未移植）落地那天才變成
//    可觀察的行為。到時候要重讀這一段。
//----------------------------------------------------------------------------
//  golden uteach.cpp:118-157
//----------------------------------------------------------------------------
void TECH_PARA::SaveToFile(bool bSaveByTeach)                                   //Steven 20240501 : Teach改存成ini
{
    //AI(W906-TEACH-W1) 20260919: golden :120-123 的兩支都以 SetEdit->Text 為中介。
    // 使用者裁決 A4=(b)：直接用 *Parameter。等價性分析見本函式上方那則。
    // （golden 的 else 支會把畫面文字讀回 *Parameter —— 沒有畫面就沒有那個來源，
    //   所以這裡不做反向更新，並在上方寫明。）

    if(MOT[MotorSelect].Alias=="MInShutte1")                                    //Steven 20250926 : Fixed for Shuttle Teach
    {
        if(CheckSectionExist(asTeachPath, "MInShutte1"))
            WriteIniData(asTeachPath, "MInShutte1", Key, AnsiString((int)(*Parameter)));
        else
            WriteIniData(asTeachPath, "MInShuttle1", Key, AnsiString((int)(*Parameter)));
    }
    else if(MOT[MotorSelect].Alias=="MInShuttle1")
    {
        if(CheckSectionExist(asTeachPath, "MInShuttle1"))
            WriteIniData(asTeachPath, "MInShuttle1", Key, AnsiString((int)(*Parameter)));
        else
            WriteIniData(asTeachPath, "MInShutte1", Key, AnsiString((int)(*Parameter)));
    }
    else if(MOT[MotorSelect].Alias=="MInShutte2")
    {
        if(CheckSectionExist(asTeachPath, "MInShutte2"))
            WriteIniData(asTeachPath, "MInShutte2", Key, AnsiString((int)(*Parameter)));
        else
            WriteIniData(asTeachPath, "MInShuttle2", Key, AnsiString((int)(*Parameter)));
    }
    else if(MOT[MotorSelect].Alias=="MInShuttle2")
    {
        if(CheckSectionExist(asTeachPath, "MInShuttle2"))
            WriteIniData(asTeachPath, "MInShuttle2", Key, AnsiString((int)(*Parameter)));
        else
            WriteIniData(asTeachPath, "MInShutte2", Key, AnsiString((int)(*Parameter)));
    }
    else if(MOT[MotorSelect].Alias!="")
    {
        WriteIniData(asTeachPath, MOT[MotorSelect].Alias, Key, AnsiString((int)(*Parameter)));
    }
}

//----------------------------------------------------------------------------
//  golden uteach.cpp:159-178
//----------------------------------------------------------------------------
TECH_TWOPARA::TECH_TWOPARA(int *Par1, int *Par2, int Mot1, int Mot2, TEdit *Edt1, TEdit *Edt2, AnsiString _Key1, AnsiString _Key2, TSpeedButton *SPB1, TSpeedButton *SPB2, bool Visible)
{
    Parameter   [0]=Par1;
    Parameter   [1]=Par2;
    MotorSelect [0]=Mot1;
    MotorSelect [1]=Mot2;
    SetEdit     [0]=Edt1;
    SetEdit     [1]=Edt2;
    Key         [0]=_Key1;
    Key         [1]=_Key2;
    funButton      =SPB1;
    btGo           =SPB2;

    //AI(W906-TEACH-W1) 20260919: golden :172-173 無 NULL 檢查，見上面 TECH_PARA 同一則。
    if(SetEdit[0]!=NULL)
        SetEdit     [0]->Visible=Visible;
    if(SetEdit[1]!=NULL)
        SetEdit     [1]->Visible=Visible;
    if(funButton!=NULL)
        funButton->Visible=Visible;
    if(btGo!=NULL)
        btGo->Visible=Visible;
}

//----------------------------------------------------------------------------
//  golden uteach.cpp:180-189
//
//  ⓘ 注意 golden 這一支**沒有** TECH_PARA 的四個 Shuttle 拼法特例 ——
//    它只有 `Alias!=""` 一條。照翻，不要「補齊」。
//----------------------------------------------------------------------------
void TECH_TWOPARA::ReadFromFile()                                               //Steven 20240501 : Teach改存成ini
{
    if(MOT[MotorSelect[0]].Alias!="")
        (*Parameter[0])=CheckAndReadIniData(asTeachPath, MOT[MotorSelect[0]].Alias, Key[0], (int)(*Parameter[0]));

    if(MOT[MotorSelect[1]].Alias!="")
        (*Parameter[1])=CheckAndReadIniData(asTeachPath, MOT[MotorSelect[1]].Alias, Key[1], (int)(*Parameter[1]));
    //AI(W906-TEACH-W1) 20260919: golden :187-188 —— 純 UI 鏡像。
    if(SetEdit[0]!=NULL)
        SetEdit[0]->Text=(int)(*Parameter[0]);
    if(SetEdit[1]!=NULL)
        SetEdit[1]->Text=(int)(*Parameter[1]);
}

//----------------------------------------------------------------------------
//  golden uteach.cpp:191-208  —— 偏離同 TECH_PARA::SaveToFile（見上面那則）。
//----------------------------------------------------------------------------
void TECH_TWOPARA::SaveToFile(bool bSaveByTeach)                                //Steven 20240501 : Teach改存成ini
{
    //AI(W906-TEACH-W1) 20260919: 同 TECH_PARA::SaveToFile 的偏離。

    if(MOT[MotorSelect[0]].Alias!="")
        WriteIniData(asTeachPath, MOT[MotorSelect[0]].Alias, Key[0], AnsiString((int)(*Parameter[0])));
    if(MOT[MotorSelect[1]].Alias!="")
        WriteIniData(asTeachPath, MOT[MotorSelect[1]].Alias, Key[1], AnsiString((int)(*Parameter[1])));
}

//----------------------------------------------------------------------------
//  golden uteach.cpp:210-218
//
//  ⓘ golden 這個 ctor 對 SelButton **完全沒有** NULL 檢查，而且四個敘述
//    全部是 widget 屬性寫入（GroupIndex / Tag / AllowAllUp / Visible）。
//    讀取路徑不經過 TECH_MotorAxle（它連 ReadFromFile 都沒有）。
//----------------------------------------------------------------------------
TECH_MotorAxle::TECH_MotorAxle(int MotorNo, TSpeedButton *SPB, bool Visible)
{
    MotorSelect             =MotorNo;
    SelButton               =SPB;
    //AI(W906-TEACH-W1) 20260919: golden :214-217 的四行全是 widget 屬性寫入。
    if(SelButton!=NULL)
    {
        SelButton->GroupIndex   =1;
        SelButton->Tag          =MotorNo;
        //AI(W906-TEACH-W1) 20260919: GATE —— 缺相依：`vclcompat::TSpeedButton`
        // 沒有 `AllowAllUp` 成員（vclcompat/Controls.h:484-491 只有 Caption /
        // Down / GroupIndex，基底 TControl 再給 Visible / Enabled / Tag）。
        // ⚠ 語意：golden 用它讓「同一組按鈕可以全部彈起」。那是**純 UI 行為**，
        //   而且本樹連按鈕實例都沒有（SelButton 永遠是 0，這個 if 進不來）。
        // UN-GATE：等 vclcompat 的 TSpeedButton 補上這個成員。
#if 0 // GATE (W906-TEACH-W1-ALLOWALLUP): 缺相依 vclcompat::TSpeedButton::AllowAllUp
        SelButton->AllowAllUp   =true;
#endif
        SelButton->Visible      =Visible;
    }
}

//----------------------------------------------------------------------------
//  golden uteach.cpp:220-233
//
//  ⓘ golden 的 `Parameter[i][j]!=NULL` 檢查是它**自己**寫的
//    （RogerYang 20250423），不是我加的 —— TechSuckPara 是固定大小的
//    2x8 陣列，但只有被登錄過的格子才有 Parameter。
//----------------------------------------------------------------------------
void TECH_SUCKPARA::ReadFromFile()                                              //Steven 20240523 : Teach SUCKPARA改存成ini
{
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<8; j++)
        {
            if(MOT[MotorSelect[i][j]].Alias!="" && Parameter[i][j]!=NULL)       //RogerYang 20250423 MotorSelect[i][j] may be zero so that Alias="MInArmX"
            {
                (*Parameter[i][j])=CheckAndReadIniData(asTeachPath, Group, Key[i][j], (int)(*Parameter[i][j]));
                //AI(W906-TEACH-W1) 20260919: golden :229 —— 純 UI 鏡像。
                if(SetEdit[i][j]!=NULL)
                    SetEdit[i][j]->Text=(int)(*Parameter[i][j]);
            }
        }
    }
}

//----------------------------------------------------------------------------
//  golden uteach.cpp:235-252  —— 偏離同 TECH_PARA::SaveToFile（見上面那則）。
//----------------------------------------------------------------------------
void TECH_SUCKPARA::SaveToFile(bool bSaveByTeach)                               //Steven 20240523 : Teach SUCKPARA改存成ini
{
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<8; j++)
        {
            if(MOT[MotorSelect[i][j]].Alias!="" && Parameter[i][j]!=NULL)       //RogerYang 20250423 MotorSelect[i][j] may be zero so that Alias="MInArmX"
            {
                //AI(W906-TEACH-W1) 20260919: 同 TECH_PARA::SaveToFile 的偏離。
                // ⓘ golden 這一支的 else 不是讀 widget，是**從 ini 讀回來再寫回去**
                //   （:246），淨效果等於不改值；移植樹直接寫 *Parameter，等價。
                WriteIniData(asTeachPath, Group, Key[i][j], AnsiString((int)(*Parameter[i][j])));
            }
        }
    }
}

// ===========================================================================
//  TfTeach::ReadFile  --  golden uteach.cpp:4781-4920（宣告 golden uteach.h:2379）
//
//  這是整條 teach 資料鏈的入口：
//    StartFromWeb -> CompareTechData（要 Tech.* 有值）
//    cinitial.cpp SetWorkParameter -> ReadTechData -> **fTeach->ReadFile()**
//
//  ---------------------------------------------------------------------------
//  這台機器實際會走哪一條（20260919 實測，寫下來免得日後靠猜）
//  ---------------------------------------------------------------------------
//    `system\teach.ini` 的 `[Teach INI] Update2=1`  -> bUseIniFile = true
//    -> 走 :4838 的 else（逐項 ReadFromFile），**不是** :4803 的二進位那條
//    區段 `[MInShuttle1]` `[MInShuttle2]` `[MInShutte1]` `[MInShutte2]`
//    -> 四個都存在（各 1）-> :4840 的 if **不成立** -> 跳過 ReadData + SaveToFile
//    `[InArm] AutoCleanPick=-1640`（非 0）-> :4915 的補寫**不成立**
//
//  ⇒ 0919 量的那台機器沒有走到上面兩個寫檔分支——但本函式**不是純讀取**：ReadFromFile 用的 CheckAndReadIniData 遇到缺的鍵會**把預設值補寫進 teach.ini**（照 golden common.cpp:432-447，int 版；unsigned long 版不寫）。St01 1001 04:51（todo D-027）：他那台的 teach.ini 缺鍵，MInShuttle1/2 那幾支讀取就把它改寫了；每次 ReadTechData（含 10 個網頁關窗尾端的 SetWorkParameter）都一樣（筆電 1001 05:4x 量：四個 Shuttle 區段各 9 個鍵都在）。行為照 golden，不改；AI(W906-D027) 20261001 更正本句（原文「這台機器上，本函式是**純讀取**：不寫任何檔。」）
//  ⚠ 這是**這台機器**的量測，不是全稱命題。換一台 teach.ini 不同的機器，
//    會撞到下面那幾個閘，而閘會**誠實地編不過或連不起來**，不會靜靜寫錯東西。
// ===========================================================================
void TfTeach::ReadFile()
{
    AnsiString sUpdateKey="Update2";
    //AI(W906-TEACH-W1) 20260919: golden :4784 `int iFileHandle;` 只被下面那個
    // 被閘住的 FileCreate/FileClose 區塊用到，一起閘掉（留著會是 -Wunused）。
    AnsiString szDir="";

    szDir.sprintf("%s", asTeachPath);                                           //Steven 20100927 Start : Offset的資料夾
    //AI(W906-TEACH-W1) 20260919: GATE —— 缺相依：`FileCreate` / `FileClose`
    // 未移植（vclcompat/SysUtils 只有 FileExists，:266）。golden 用它們在
    // teach.ini 不存在時建一個空檔。
    // ⓘ 這台機器 `FileExists(asTeachPath)` 為真，所以這一段本來就到不了。
    // UN-GATE：等 vclcompat 落地 FileCreate/FileClose，或由整合者決定用
    //   std::ofstream 取代（**那是改寫不是翻譯**，要留紀錄）。
    // ⚠ 後果（缺了它會怎樣）：teach.ini 不存在的機器上，後面的
    //   CheckKeyExist/CheckAndReadIniData 會對著不存在的檔問，全部拿到預設值
    //   -> Tech.* 全 0 -> CompareTechData 否決 -> START 被擋。
    //   那是**擋住**而不是**放行**，所以缺這一段不會讓機台亂動。
#if 0 // GATE (W906-TEACH-W1-FILECREATE): 缺相依 FileCreate / FileClose
    if(FileExists(szDir)==false)
    {
        iFileHandle=FileCreate(szDir);
        FileClose(iFileHandle);
    }
#endif // GATE (W906-TEACH-W1-FILECREATE)

    if(CheckKeyExist(asTeachPath, "Teach INI", sUpdateKey))                     //Steven 20240501 : Teach改存成ini
    {
        bUseIniFile=CheckAndReadIniData(asTeachPath, "Teach INI", sUpdateKey, false);
    }
    else
    {
        bUseIniFile=false;
    }

    if(bUseIniFile==false)
    {
        ReadData("d:\\HT9045\\system\\tech.dat", (char *)&Tech.iZLoad, sizeof(TECH));
        Tech.iInArmPickX =(Tech.iInArmPickX ==0)?Tech.iInArmLoadStageX      :Tech.iInArmPickX;
        Tech.iInArmPickY =(Tech.iInArmPickY ==0)?Tech.iInArmLoadStageY-2000 :Tech.iInArmPickY;
        Tech.iOutArmPickX=(Tech.iOutArmPickX==0)?Tech.iOutArmAuto1X         :Tech.iOutArmPickX;
        Tech.iOutArmPickY=(Tech.iOutArmPickY==0)?Tech.iOutArmAuto1Y-2000    :Tech.iOutArmPickY;
        if(CosFunction.bOutShuttleSensorCanNotDisable)                          //Steven 20151202 : Out Shuttle Sensor不能取消檢查
        {
            if(Tech.OutSH1ZDetectPos==0)
                Tech.OutSH1ZDetectPos=20599;
            if(Tech.OutSH2ZDetectPos==0)
                Tech.OutSH2ZDetectPos=20469;
            if(Tech.OutSH1ZOneRowDetectPos==0)
                Tech.OutSH1ZOneRowDetectPos=20565;
            if(Tech.OutSH2ZOneRowDetectPos==0)
                Tech.OutSH2ZOneRowDetectPos=20453;
        }

        //AI(W906-TEACH-W1) 20260919: 已解閘（使用者裁決 A4=(b)，見
        // TECH_PARA::SaveToFile 上方那則偏離說明）。
        // ⚠ 這一段**會寫真實檔**（把 tech.dat 的值回寫成 teach.ini 並設
        //   Update2=1）。golden 就是這樣做的，依計畫 §0.5 照翻、§0.6 備份驗證。
        // ⓘ 這台機器 `Update2=1`，所以整個 `bUseIniFile==false` 分支到不了。
        for(int i=0; i<TECH_MAX_ITEM; i++)
            TechPara[i]->SaveToFile(true);

        for(int i=0; i<TechTwoItem; i++)
            TechTwoPara[i]->SaveToFile(true);

        TechSuckPara[0].SaveToFile(true);                                       //Steven 20240523 : Teach SUCKPARA改存成ini
        TechSuckPara[1].SaveToFile(true);
        if(USE_OUT_SORT_ARM!=eartUninstall)                                     //RogerYang 20250416 for HT9046AU add
        {
            TechSuckPara[2].SaveToFile(true);
        }

        bUseIniFile=true;
        WriteIniData(asTeachPath, "Teach INI", sUpdateKey, bUseIniFile);
    }
    else
    {
        //AI(W906-TEACH-W1) 20260919: 已解閘（使用者裁決 A4=(b)）。
        // ⚠ 這一段在 `[MInShuttle1]`/`[MInShuttle2]` 缺席時**會寫真實檔**。
        // ⓘ 實測這台機器四種拼法的區段都在，所以 golden 這個 if 本來就不成立。
        if(CheckSectionExist(asTeachPath, "MInShuttle1")==false ||              //Steven 20250507 : 修改group name
           CheckSectionExist(asTeachPath, "MInShuttle2")==false)
        {
            ReadData("d:\\HT9045\\system\\tech.dat", (char *)&Tech.iZLoad, sizeof(TECH));
            for(int i=0; i<TECH_MAX_ITEM; i++)
            {
                if(TechPara[i]->MotorSelect==MInShuttle1 ||
                   TechPara[i]->MotorSelect==MInShuttle2)
                {
                    TechPara[i]->SaveToFile(true);
                }
            }
        }

        for(int i=0; i<TECH_MAX_ITEM; i++)
            TechPara[i]->ReadFromFile();

        for(int i=0; i<TechTwoItem; i++)
            TechTwoPara[i]->ReadFromFile();

        TechSuckPara[0].ReadFromFile();                                         //Steven 20240523 : Teach SUCKPARA改存成ini
        TechSuckPara[1].ReadFromFile();
        if(USE_OUT_SORT_ARM!=eartUninstall)                                     //RogerYang 20250416 for HT9046AU add
        {
            TechSuckPara[2].ReadFromFile();
        }
        Tech.bAOAMatrix=CheckAndReadIniData(asTeachPath, "Teach INI", "bAOAMatrix", false);

        Tech.M_In_iRotateA_Backlash=CheckAndReadIniData(asTeachPath, "MInRotate", "edtEditRotateInBacklash", 0);        //RogerYang 20260113 : Rotator新增背隙補償
        Tech.M_Out_iRotateA_Backlash=CheckAndReadIniData(asTeachPath, "MOutRotate", "edtEditRotateOutBacklash", 0);
        // [W906-TEACH-W1 T2 widget-only] golden :4870-4871
        //edtEditRotateInBacklash->Text=Tech.M_In_iRotateA_Backlash;              //RogerYang 20260113 : Rotator新增背隙補償
        //edtEditRotateOutBacklash->Text=Tech.M_Out_iRotateA_Backlash;
    }

    if(MACHINE_HAS_AUTO_ALIGNMENT_CCD)
    {
        if(Tech.bAOAMatrix==false)
        {
            for(int i=0; i<8; i++)
            {
                Tech.iInArmZBasePickerAlignmentPos[i%2][i/2]           =Tech._iInArmZBasePickerAlignmentPos[i];
                Tech.iOutArmZBasePickerAlignmentPos[i%2][i/2]          =Tech._iOutArmZBasePickerAlignmentPos[i];
                Tech.iInArmCCD_Picker_PosX[i/4][i%4]                   =Tech._iInArmCCD_Picker_PosX[i];
                Tech.iInArmCCD_Picker_PosY[i/4][i%4]                   =Tech._iInArmCCD_Picker_PosY[i];
                Tech.iInArmCCD_Picker_PosZ[i/4][i%4]                   =Tech._iInArmCCD_Picker_PosZ[i];
                Tech.iOutArmCCD_Picker_PosX[i/4][i%4]                  =Tech._iOutArmCCD_Picker_PosX[i];
                Tech.iOutArmCCD_Picker_PosY[i/4][i%4]                  =Tech._iOutArmCCD_Picker_PosY[i];
                Tech.iOutArmCCD_Picker_PosZ[i/4][i%4]                  =Tech._iOutArmCCD_Picker_PosZ[i];
            }

            Tech.iInArmCCD_Pitch_PosX[0][0]=Tech._iInArmCCD_Pitch_PosX[0];
            Tech.iInArmCCD_Pitch_PosX[0][1]=Tech._iInArmCCD_Pitch_PosX[1];
            Tech.iInArmCCD_Pitch_PosX[0][2]=Tech._iInArmCCD_Pitch_PosX[2];
            Tech.iInArmCCD_Pitch_PosX[1][0]=Tech._iInArmCCD_Pitch_PosX[3];
            Tech.iInArmCCD_Pitch_PosX[1][1]=Tech._iInArmCCD_Pitch_PosX[4];
            Tech.iInArmCCD_Pitch_PosX[1][2]=Tech._iInArmCCD_Pitch_PosX[5];
            Tech.iOutArmCCD_Pitch_PosX[0][0]=Tech._iOutArmCCD_Pitch_PosX[0];
            Tech.iOutArmCCD_Pitch_PosX[0][1]=Tech._iOutArmCCD_Pitch_PosX[1];
            Tech.iOutArmCCD_Pitch_PosX[0][2]=Tech._iOutArmCCD_Pitch_PosX[2];
            Tech.iOutArmCCD_Pitch_PosX[1][0]=Tech._iOutArmCCD_Pitch_PosX[3];
            Tech.iOutArmCCD_Pitch_PosX[1][1]=Tech._iOutArmCCD_Pitch_PosX[4];
            Tech.iOutArmCCD_Pitch_PosX[1][2]=Tech._iOutArmCCD_Pitch_PosX[5];
            Tech.bAOAMatrix=true;
            //AI(W906-TEACH-W1) 20260919: ⚠ 這一行**會寫真實檔**（teach.ini 的
            // [Teach INI] bAOAMatrix）。golden 就是這樣寫的，依計畫 §0.5 照翻。
            // 觸發條件：機台有 Alignment CCD **且** ini 裡 bAOAMatrix 還是 0。
            // 實測這台 teach.ini 是 `bAOAMatrix=0`，所以只要
            // MACHINE_HAS_AUTO_ALIGNMENT_CCD 為真，第一次跑就會寫一次。
            // ⇒ 驗證時照計畫 §0.6：先備份 teach.ini、跑完比對內容、確認只多
            //   這一個鍵、再刪備份。
            WriteIniData(asTeachPath, "Teach INI", "bAOAMatrix", Tech.bAOAMatrix);
        }
    }
    else
    {
        Tech.bAOAMatrix=true;
    }

    AnsiString FileName=ExtractFileName(asTeachPath);
    AnsiString FilePath=ExtractFilePath(asTeachPath);
    //AI(W906-TEACH-W1) 20260919: golden :4913 沒有 NULL 檢查。本樹的 `elTeach`   ⚠ AI(W906-W5-TEACH) 20260925 更正：elTeach 現在由 FileRW/IniConfig.cpp 開機建構，wb_serve 另由 FileRW_Teach_Boot 註冊 golden 的 203 筆（docs/W5_PROGRESS.md）
    // 是 `Public/HTEditList.cpp:205` 的**裸指標，從未建構**（與 fTeach 同樣的
    // SIOF 迴避姿態）。deref 它就是 NULL deref。
    // ⚠ 後果：AOA 的 edit-list 文字不會從檔案讀回來。那一層是 UI 文字，
    //   Tech.* 的數值不經過它（數值是上面 ReadFromFile 讀的）。
    if(elTeach!=NULL)
        elTeach->ReadEditTextFromFile(FilePath, FileName);                      //JerryYang 20241119 : fix AOA

    //AI(W906-TEACH-W1) 20260919: GATE —— 缺相依，而且**第一次實跑就毀了現場資料**。
    //
    // ⚠ 我原本判斷這一段到不了，理由是「實測這台 `[InArm] AutoCleanPick=-1640`
    //   （非 0）」。**那個判斷是錯的**：golden 測的是全域 `Teach.iAutoCleanPick`，
    //   不是 ini 裡那個值。第一次實跑的結果（20260919，tools/realfile_guard.py
    //   check 抓到）：
    //       system\teach.ini  [InArm] AutoCleanPick  -1640 -> 0
    //   也就是把機台真正的教導值洗成 0。已用備份還原、逐位元組驗過。
    //
    // 缺的相依（兩段都不在）：
    //   1. `fTeach->InitialTeachEditList()` —— `cinitial.cpp:10927` 的 GATE n4-4
    //   2. `elTeach` 本身 —— `Public/HTEditList.cpp:205` 是裸指標，從未建構
    //   這兩段合起來才是 golden 用來把 teach.ini 讀進 `Teach.*` 的路。
    //   量過：全樹對 `Teach.iAutoCleanPick` 只有**兩個讀取點**
    //   （`cinitial.cpp:14949` / `:14950`），**零個寫入點**。
    //
    // ⇒ 在這棵樹，`Teach.iAutoCleanPick==0` 的意思**不是** golden 的
    //   「這一項沒被教過」，而是「載入鏈不存在，所以它還是初值」。
    //   拿假前提去寫檔，寫出去的一定是錯的值。
    //   這正是 memory `always-true-excuse-is-the-thing-to-measure` 講的那件事：
    //   一個永遠成立的條件，不是條件，是該去量的東西。
    //
    // UN-GATE 條件：`Teach.*` 真的會被載入之後（解 n4-4 + 建構 elTeach +
    //   落地 `HTEditList::ReadEditTextFromFile` 的消費端）。那是另一個波次。
    // ⚠ 在那之前，`Prod.ZInArm_AutoClean_Pick/Place`（cinitial.cpp:14949-14950）
    //   會用 `Teach.iAutoCleanPick==0` 去算 AutoClean 的取放高度 ——
    //   **AutoClean 在這棵樹的高度是錯的**。這是既有狀態，不是本波造成的，
    //   但本波第一次讓它變得可觀察，所以記在這裡。
#if 0 // GATE (W906-TEACH-W1-AUTOCLEANPICK): 假前提 —— Teach.* 的載入鏈不存在（n4-4 + elTeach），Teach.iAutoCleanPick==0 不代表「沒教過」   ⚠ 20260925 W5：wb_serve 開機已註冊 elTeach（FileRW_Teach_Boot）⇒ 在 wb_serve 裡前提已不成立；但 ctest 與其他 exe 沒有註冊，在那些行程裡解閘會讓 ctest 寫量產 teach.ini ⇒ 閘維持（docs/W5_PROGRESS.md §4-7）
    if(Teach.iAutoCleanPick==0)                                                 //RogerYang 20250627 提到Read ini後面
    {
        Teach.iAutoCleanPick=Tech.iInArmPlatePickZ2;                            //資料不存在帶入單抓
        WriteIniData(szDir, "InArm", "AutoCleanPick", Tech.iInArmPlatePickZ2);  //AutoClean pick Kit
    }
#endif // GATE (W906-TEACH-W1-AUTOCLEANPICK)
}
