// ===========================================================================
//  ContactForceLoad.cpp  --  AI(W906-P2a-CF) 20260919
//  見 ContactForceLoad.h 的橫幅：為什麼是獨立 TU、它補的是哪個洞。
//
//  golden 對應：ContactForce.cpp:421-652（ctor 的資料半邊）
//               ContactForce.cpp:963-1178（ReadFile 的資料半邊）
// ===========================================================================
#include "ContactForceLoad.h"

#include "ContactForce.h"            // SlkForceTables / Load*SlkTable / Apply*
#include "common.h"                  // CheckAndReadIniData（common.h:222/:225）
#include "cprod.h"                   // TestIF_File（cprod.h:2577）/ IniConfig
#include "cmydef.h"                  // EP_Install / INSTALL_DOUBLE_EP / iIndEPCnt
#include "CosFunction.h"             // CosFunction.bUseDynamicKitDiameter
#include "MachineType.h"             // CUSTOMER_CODE / CC_KYEC_LEE / CC_ASE_SG / DOUBLE_EP_MULTI / DualSite
#include "Config.h"                  // IniConfig.bSPILFunction
#include "vclcompat/SysUtils.h"      // FileExists（:90）

#include <cstdio>
#include <string>

namespace {

//  golden ContactForce.cpp:428。字面值照抄，不走任何重導 ——
//  這個檔是機台共用參數，讀它是 golden 的行為。
const char* kContactInfoIni = "D:\\HT9045\\system\\ContactInfo.ini";

//  一筆的五個值（golden :1061-1065）。群組名由呼叫端給。
SlkIniValues ReadFive(const AnsiString& file, const std::string& group)
{
    SlkIniValues v;
    const AnsiString g(group.c_str());
    v.dLoadRate         = CheckAndReadIniData(file, g, AnsiString("LoadRate"),         1.0);
    v.dLoadRate_NS      = CheckAndReadIniData(file, g, AnsiString("LoadRate_NS"),      1.0);
    v.dHotOffset        = CheckAndReadIniData(file, g, AnsiString("HotOffset"),        0.0);
    v.dContactOffset    = CheckAndReadIniData(file, g, AnsiString("ContactOffset"),    0.0);
    v.dContactOffset_NS = CheckAndReadIniData(file, g, AnsiString("ContactOffset_NS"), 0.0);
    return v;
}

}  // namespace

int LoadContactForceTables()
{
    SlkForceTables& t = ContactForceTables();
    const AnsiString file(kContactInfoIni);                                     // golden :428

    //  重複呼叫安全：先清空，不累加。golden 的 ctor 只跑一次，所以它沒有這個問題；
    //  移植樹沒有表單生命期，載入時機由整合者決定，因此必須自己保證冪等。
    t.SLKClass.Clear();
    t.SLKIndClass.Clear();
    t.DieForceSLKClass.Clear();
    t.DieForceOneByOneSLKClass.Clear();
    //AI(W906-P2b-CF) 20260919: `Clear()` 清的是表，不會動這兩個 token 數，
    //  所以重載時自己重設。忘了這一行的後果是「表空了但上界還是舊的」。
    t.iSlkTypeIndTokens   = 0;
    t.iDieForceTypeTokens = 0;

    if (CosFunction.bUseDynamicKitDiameter == false)                            // golden :430
        return 0;

    const bool bHasFile = FileExists(file);                                     // golden :432-435
    const bool bKyec    = (CUSTOMER_CODE == CC_KYEC_LEE);                       // golden :437

    // ---- EP 的壓力/回授範圍（golden ReadFile :980-1032）---------------------
    //
    //AI(W906-P2b-CF) 20260919: 這 12 個全域是 `TransformFuntion` 的**承重輸入**：
    //   golden adam6024.cpp:1156  fMaxMPA = EP_MAXKPA/1000.0
    //   golden adam6024.cpp:1157  fMinMPA = EP_MINMPA
    //   golden adam6024.cpp:1793  iResult = ratio(fMaxUnit-fMinUnit, fMaxMPA-fMinMPA)
    //                                       * (fInputMPA-fMinMPA) + fMinUnit
    //
    // ⚠ 發現的方式：P2b 的測試顯示 `TransformFuntion` **對每一個輸入都回 0**。
    //   根因是 `cmydef.cpp:3055 double EP_MAXKPA=0.0;` 而**全樹沒有任何人把它
    //   從 ini 讀進來** —— 唯一提到載入的 `HandlerSys.cpp:406` 是把值寫進
    //   **widget**（`edMaxKpa->Text`），不是全域。
    //   ⇒ `fMaxMPA-fMinMPA == 0` -> 零保護回 0 -> `iResult` 恆為 0。
    //   **編譯綠、不當掉、回傳值也在合法範圍內 —— 但那個值沒有意義。**
    //   這正是 `ContactForce.h:319` 說的 silent defect 的另一種面貌。
    //
    // 來源是 `system\Gerneral.ini` 的 `[System]`（`CheckAndReadIniDataGeneral`），
    // 不是 ContactInfo.ini。golden 的 gate 是 `EP_Install!=0`（golden :980）。
    // 12 個全域在 cmydef.h 都有，0 個缺件。
    // ⚠⚠ `CheckAndReadIniDataGeneral()` 無條件解參考全域 `INIFileGeneral`
    //   （common.cpp，忾實的 BCB6 行為），而那個指標只有
    //   `OpenGeneralIniFile()` 會設。`database.h:360-380` 已經把這個陷阱
    //   寫成一整段警告：順序錯了**不是行為變樣，是 SEGFAULT**。
    //   實測過：第一版沒守衛，ctest 裡一叫就當掉。
    //   → 沒開就跳過。值維持在 cmydef.cpp 的定義初值（0.0），
    //     `TransformFuntion` 因此會回 0 —— 那是誠實的「還沒組態好」。
    //   ⓘ wb_serve 的 bring-up 在 LoadMachineConfig() 之後才叫本函式，
    //     而 LoadMachineConfig() 會開它，所以 production 路徑上這個守衛成立。
    //AI(W906-FRW-S57) 20260926: golden V912 建構子 :542-544（V899 :538-540 同）在填完 SLKIndClass 之後、**不看 EP_Install**
    //   先讀這三鍵（kevin 20170803「避免一開始值被修改」），ReadFile（:983）才在 EP_Install!=0 時讀 12 鍵。原本這個載入器只有後者 ——
    //   EP_Install==0 的機台 EP_MAXKPA／EP_MAXAFB／EP_MINMPA 停在 cmydef.cpp 的初值 0。缺鍵時照 golden 補寫預設值
    //   （CheckAndReadIniDataGeneral）。golden 是先填表、後讀這三鍵，兩者互不相依，放在這裡等價。INIFileGeneral 的 null 守衛同下一段。
    if (INIFileGeneral != 0)
    {
        EP_MAXKPA      = CheckAndReadIniDataGeneral(AnsiString("System"), AnsiString("EP_MAXKPA"),          499.0); // golden :542
        EP_MAXAFB      = CheckAndReadIniDataGeneral(AnsiString("System"), AnsiString("EP_MAXA"),            5.013); // golden :543
        EP_MINMPA      = CheckAndReadIniDataGeneral(AnsiString("System"), AnsiString("EP_MINMPA"),          0.001); // golden :544
    }

    if (EP_Install != 0 && INIFileGeneral != 0)                                 // golden :980 ＋ port 的 null 守衛
    {
        EP_MAXKPA      = CheckAndReadIniDataGeneral(AnsiString("System"), AnsiString("EP_MAXKPA"),          499.0); // golden :982
        EP_MAXAFB      = CheckAndReadIniDataGeneral(AnsiString("System"), AnsiString("EP_MAXA"),            5.013); // golden :983
        EP_MINMPA      = CheckAndReadIniDataGeneral(AnsiString("System"), AnsiString("EP_MINMPA"),          0.001); // golden :984
        EP_MinAFB      = CheckAndReadIniDataGeneral(AnsiString("System"), AnsiString("EP_MINA_FeedBack"),   0.908); // golden :985

        EP_MAXKPA_1032 = CheckAndReadIniDataGeneral(AnsiString("System"), AnsiString("EP_MAXKPA_1032"),     499.0); // golden :991
        EP_MAXAFB_1032 = CheckAndReadIniDataGeneral(AnsiString("System"), AnsiString("EP_MAXA_1032"),       5.013); // golden :992
        EP_MINMPA_1032 = CheckAndReadIniDataGeneral(AnsiString("System"), AnsiString("EP_MINMPA_1032"),     0.001); // golden :993
        EP_MinAFB_1032 = CheckAndReadIniDataGeneral(AnsiString("System"), AnsiString("EP_MINA_FeedBack_1032"), 0.908); // golden :994

        EPDual_MAXKPA  = CheckAndReadIniDataGeneral(AnsiString("System"), AnsiString("EPDual_MAXKPA"),      899.0); // golden :1000
        EPDual_MAXAFB  = CheckAndReadIniDataGeneral(AnsiString("System"), AnsiString("EPDual_MAXAFB"),      4.905); // golden :1001
        EPDual_MINMPA  = CheckAndReadIniDataGeneral(AnsiString("System"), AnsiString("EPDual_MINMPA"),      0.001); // golden :1002
        EPDual_MinAFB  = CheckAndReadIniDataGeneral(AnsiString("System"), AnsiString("EPDual_MinAFB"),      0.968); // golden :1003
    }

    // ---- Index Z offset（golden ReadFile :1014-1036）-------------------------
    //AI(W906-FRW-S57) 20260926: 0925 cmydef 盤點 P4：`dIndexZOffset[3][15]`（cmydef.h:5214）全樹唯一的載入點是 golden
    //   TfContactForce::ReadFile；這個載入器原本沒讀 → 移植樹的讀者（cinitial.cpp DoSetupSystemToProd :8593-8667、
    //   cContact.cpp :665-753）拿到 0（連 [2][*] 的壓力階梯 120／180／…／800 也是 0，`dPress>dIndexZOffset[2][13]` 永遠成立）。
    //   golden 何時讀：建構子只在 bUseDynamicKitDiameter 時呼叫 ReadFile（有檔直接讀；沒檔 WriteFile 尾端 :1375 也讀）→ 與本函式
    //   上面 :430 的 gate 同一個條件。bUseDynamicKitDiameter==false 的機台 golden 要等開 ContactForce 頁（FormShow :689）才讀 ——
    //   那一半在 FileRW/ContactForce.cpp（C 路），照 golden。
    //   [2][*] 是 golden 程式算的固定階梯（不讀檔）；[0..1][*] 讀 Gerneral.ini [Test Arm] "dIndexZOffset[i][j]"，夾 0～10
    //   （golden `CheckRange(v, 10.0, 0.0)`），缺鍵照 golden 補寫 0.0。畫面半邊（IndexZOffsetEdit[i][j]->Text）不在這裡。
    //   INIFileGeneral 沒開就只算 [2][*]（null 守衛同上一段），[0..1][*] 維持原值。
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 15; j++)                                            // golden :1016（Ifor 20230829 800KG 11 -> 15）
        {
            if (i == 2)
            {
                if (j >= 7)
                    dIndexZOffset[i][j] = 120 + (6 * 60) + ((j - 6) * 40);      // golden :1022
                else
                    dIndexZOffset[i][j] = 120 + (j * 60);                       // golden :1026
            }
            else if (INIFileGeneral != 0)
            {
                AnsiString Str;
                Str.sprintf("dIndexZOffset[%d][%d]", i, j);                     // golden :1031（JerryYang 20190910 fix ATC offset 存檔問題）
                dIndexZOffset[i][j] = CheckRange(CheckAndReadIniDataGeneral(AnsiString("Test Arm"), Str, 0.0), 10.0, 0.0);   // golden :1032
            }
        }
    }

    // ⚠ golden 的 ReadFile 在同一段還把這些值寫進 widget（edMaxKpa->Text 等）。
    //   那是 UI 半邊，不在這裡 —— 與本檔其餘部分同一個界線。

    // ---- [SLK Type] 與 [SLK Type Ind] 的兩組 CSV（golden :437-453）----------
    //  預設值不在這裡硬寫：`ContactForce.h:348-353` 已經把 golden 的六個預設
    //  做成具名常數，就是為了讓這一層不必重新推導。
    const AnsiString sType    = CheckAndReadIniData(file, AnsiString("SLK Type"),
                                                    AnsiString("Type"),
                                                    AnsiString(SlkDefaultTypeCsv(bKyec)));
    const AnsiString sVisible = CheckAndReadIniData(file, AnsiString("SLK Type"),
                                                    AnsiString("Visible"),
                                                    AnsiString(SlkDefaultVisibleCsv(bKyec)));
    const AnsiString sTypeInd    = CheckAndReadIniData(file, AnsiString("SLK Type Ind"),
                                                       AnsiString("Type"),
                                                       AnsiString(SlkDefaultTypeIndCsv()));
    const AnsiString sVisibleInd = CheckAndReadIniData(file, AnsiString("SLK Type Ind"),
                                                       AnsiString("Visible"),
                                                       AnsiString(SlkDefaultVisibleIndCsv()));

    // ---- SLKClass（golden :455-491）----------------------------------------
    {
        SlkLoadOptions opt;
        opt.iEpInstall       = EP_Install;
        opt.bCustomerAseSg   = (CUSTOMER_CODE == CC_ASE_SG);
        opt.bCustomerKyecLee = bKyec;
        LoadSlkTable(t.SLKClass, std::string(sType.c_str()),
                     std::string(sVisible.c_str()), opt);
    }

    // ---- SLKIndClass（golden :493-539）--------------------------------------
    //  ⚠ 第三個參數是 **SLKClass 的** Visible CSV，不是 Ind 的。那不是筆誤：
    //    golden :534（一般臂）讀 slSLKTypeVisible，只有 :509（MULTI 臂）讀
    //    slSLKTypeIndVisible。`ContactForce.h:427-433` 已經把這個 quirk 寫死在
    //    介面上並註明「MUST NOT BE FIXED」。
    {
        SlkIndLoadOptions opt;
        opt.bDoubleEpMulti    = (INSTALL_DOUBLE_EP == DOUBLE_EP_MULTI);
        opt.iIndEPCnt         = iIndEPCnt;
        opt.bTestModeDualSite = (TestIF_File.iTestMode == DualSite);
        LoadSlkIndTable(t.SLKIndClass, std::string(sTypeInd.c_str()),
                        std::string(sVisibleInd.c_str()),
                        std::string(sVisible.c_str()), opt);
    }

    //AI(W906-P2b-CF) 20260919: golden 的 `slSLKTypeInd->Count`（TransformFuntion :623）。
    //  是 CSV 的**原始 token 數**，不是 SLKIndClass.size() —— 兩者不同，見
    //  ContactForce.h 裡 iSlkTypeIndTokens 的註解。
    t.iSlkTypeIndTokens = (int)SlkSplitCommaText(std::string(sTypeInd.c_str())).size();

    // ---- DieForce 兩張表（golden :575-637）----------------------------------
    if (INSTALL_DOUBLE_EP > 0)                                                  // golden :575
    {
        std::string sDfType;
        std::string sDfVis;
        if (bHasFile)                                                           // golden :577
        {
            const AnsiString a = CheckAndReadIniData(file, AnsiString("SLK Type"),
                                                     AnsiString("DieForceType"),
                                                     AnsiString(SlkDefaultDieForceTypeCsv()));
            const AnsiString b = CheckAndReadIniData(file, AnsiString("SLK Type"),
                                                     AnsiString("DieForceVisible"),
                                                     AnsiString(SlkDefaultDieForceVisibleCsv(
                                                         IniConfig.bSPILFunction)));
            sDfType = a.c_str();
            sDfVis  = b.c_str();
        }
        else                                                                    // golden :589-593
        {
            sDfType = "20,30,40,50";
            sDfVis  = "1,1,1,0";
        }

        //  golden :595-601 的 SPIL 後調整。已經是一支純函式，直接用。
        sDfVis = SlkDieForceVisibleCsvForSpil(sDfType, sDfVis, IniConfig.bSPILFunction);

        //AI(W906-P2b-CF) 20260919: golden 的 `slDieForceOneByOneSLKType->Count`
        //  （TransformFuntion :350）。golden :617-621 把它從
        //  edtDieForceCurrentType->Text 建起來，與 slDieForceSLKType 同一份 CSV。
        t.iDieForceTypeTokens = (int)SlkSplitCommaText(sDfType).size();

        LoadDieForceSlkTable(t.DieForceSLKClass, sDfType, sDfVis);              // golden :603-615
        if (INSTALL_DOUBLE_EP == DOUBLE_EP_MULTI)                               // golden :624
            LoadDieForceOneByOneSlkTable(t.DieForceOneByOneSLKClass, sDfType, sDfVis);
    }

    // ---- 每一筆的 ini 值（golden ReadFile :1035-1177）-----------------------
    //
    //  ⚠ golden 的 ctor 尾端是 `if(bHasFile) ReadFile(); else WriteFile();`
    //    （golden :639-642）。`WriteFile()` 是**寫檔**，而且會把整份
    //    ContactInfo.ini 重寫（golden :1180-1374，195 行）。
    //    那一半屬於 `ContactForce.h` 明講延後的 IO，這裡不做。
    //    後果講清楚：**檔案不存在時，四張表仍然照 CSV 預設值建起來、
    //    每一筆維持 SlkIniValues 的建構預設（LoadRate 1.0、offset 0.0），
    //    但 ContactInfo.ini 不會被建立。** golden 會建。
    //    ⇒ 下一次呼叫仍然走同一條路，不會像 golden 那樣「第一次之後就有檔了」。
    if (bHasFile == false)                                                      // golden :1035
        return (int)(t.SLKClass.size() + t.SLKIndClass.size() +
                     t.DieForceSLKClass.size() + t.DieForceOneByOneSLKClass.size());

    //  SLKClass：三種群組名（golden :1040-1060），bHasDiameter 是 golden 的
    //  函式範圍 latch（golden :967），必須跨整個 walk 帶著。
    {
        bool bHasDiameter = false;
        const bool bAseSg = (CUSTOMER_CODE == CC_ASE_SG);
        for (size_t i = 0; i < t.SLKClass.items.size(); ++i)
        {
            SlkForceData& e = t.SLKClass.items[i];
            const std::string g = SlkIniGroupName(e, bAseSg, EP_Install,
                                                  bHasDiameter, nullptr);
            ApplySlkIniValues(e, ReadFive(file, g));                            // golden :1061-1065
        }
    }

    //  DieForce（golden :1076-1105）。這兩張表只有 LoadRate 與 ContactOffset，
    //  而且 ini 預設值不同：DieForce 的 ContactOffset 預設是 **1.0**（golden :1082），
    //  OneByOne 的是 0.0（golden :1098）。差別是 golden 的，照留。
    if (INSTALL_DOUBLE_EP > 0)
    {
        for (size_t i = 0; i < t.DieForceSLKClass.items.size(); ++i)
        {
            SlkForceData& e = t.DieForceSLKClass.items[i];
            const AnsiString g(DieForceSlkIniGroupName(e).c_str());
            const double r = CheckAndReadIniData(file, g, AnsiString("LoadRate"),      1.0);
            const double c = CheckAndReadIniData(file, g, AnsiString("ContactOffset"), 1.0);
            ApplySlkPairIniValues(e, r, c);
        }
        if (INSTALL_DOUBLE_EP == DOUBLE_EP_MULTI)
        {
            for (size_t k = 0; k < t.DieForceOneByOneSLKClass.items.size(); ++k)
            {
                SlkForceData& e = t.DieForceOneByOneSLKClass.items[k];
                const AnsiString g(DieForceOneByOneSlkIniGroupName(e, (int)(k % 8)).c_str());
                const double r = CheckAndReadIniData(file, g, AnsiString("LoadRate"),      1.0);
                const double c = CheckAndReadIniData(file, g, AnsiString("ContactOffset"), 0.0);
                ApplySlkPairIniValues(e, r, c);
            }
        }
    }

    //  SLKIndClass（golden :1142-1176）。MULTI 是每型 8 筆、一般是每型 16 筆，
    //  群組名的 j 由索引取模得到 —— 與 LoadSlkIndTable 推的 tag（i*8+j / i*16+j）
    //  同一個規則。
    {
        const bool bMulti = (INSTALL_DOUBLE_EP == DOUBLE_EP_MULTI);
        const int  iPer   = bMulti ? 8 : 16;
        for (size_t k = 0; k < t.SLKIndClass.items.size(); ++k)
        {
            SlkForceData& e = t.SLKIndClass.items[k];
            const AnsiString g(SlkIndIniGroupName(e, (int)(k % (size_t)iPer), bMulti).c_str());
            const double r = CheckAndReadIniData(file, g, AnsiString("LoadRate"),      1.0);
            const double c = CheckAndReadIniData(file, g, AnsiString("ContactOffset"), 0.0);
            ApplySlkPairIniValues(e, r, c);
        }
    }

    t.SLKClass.bLoaded                 = true;
    t.SLKIndClass.bLoaded              = true;
    t.DieForceSLKClass.bLoaded         = true;
    t.DieForceOneByOneSLKClass.bLoaded = true;

    return (int)(t.SLKClass.size() + t.SLKIndClass.size() +
                 t.DieForceSLKClass.size() + t.DieForceOneByOneSLKClass.size());
}
