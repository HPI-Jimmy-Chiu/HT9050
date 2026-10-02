// ===========================================================================
//  adam6024.cpp  --  AI(W906-P2b-CF) 20260919
//
//  golden adam6024.cpp:1038-1796  `TransformFuntion`（759 行）。
//  括號收支用 tools/span_of.py 驗過（span 1038-1796，759 行）。
//
//  它做什麼
//  ---------------------------------------------------------------------------
//  把「要壓幾公斤」換算成 EP（電氣比例閥）的類比輸出碼。Double EP 的機台
//  Start 時就是靠它把下壓力道寫出去（golden main.cpp:6233-6254）。
//  ⚠ 量過：這支函式本身 **ADAM 寫 0 / ADAM 讀 0 / 馬達 0 / 氣缸 0 / 檔案 0**，
//    只有一個 ShowMyMessage。它是**純換算**。真正的寫出在呼叫端
//    （`ADAM_DirectWriteData(iInputValue, 0, 0)`，golden :6252）。
//    RULINGS_20260917 §C11.5 已更正過「這是寫 IO」那句話。
//
//  P2a 解掉的那個阻塞
//  ---------------------------------------------------------------------------
//  golden 把四張力量表掛在 VCL 表單 `fContactForce` 上，而那個表單刻意未移植
//  （ContactForce.h:1-16 的 scope 宣告）。759 行裡有 **44 個** `fContactForce->…`
//  —— 這就是 §C11.2 判定「停在量測」的原因。
//
//  20260919 的 P2a 把四張表搬進 `ContactForceTables()`（ContactForce.h:336）
//  並接上 wb_serve 的 bring-up，所以那 44 個現在有家了。替換是機械的：
//
//      fContactForce->SLKClass[i]->dLoadRate     ->  cft.SLKClass.items[i].dLoadRate
//      fContactForce->SLKIndClass.size()         ->  cft.SLKIndClass.size()
//      fContactForce->slSLKTypeInd->Count        ->  cft.iSlkTypeIndTokens
//      fContactForce->slDieForceOneByOneSLKType->Count -> cft.iDieForceTypeTokens
//
//  44 個用到的欄位只有六個，全部在 `SlkForceData` 裡：
//      dDiameter / dLoadRate / dLoadRate_NS / dHotOffset /
//      dContactOffset / dContactOffset_NS
//
//  ⚠ 後兩個是 **TStringList 的 Count**，也就是 CSV 的**原始 token 數**，
//    不是表的 size()。填充迴圈會丟掉空 token 與 atof<=15.0 的，兩者不同。
//    名字裡的 `Tokens` 就是為了讓下一個人不會拿錯（ContactForce.h 有註解）。
//
//  ⚠⚠ 呼叫端要自己確認表載過了
//  ---------------------------------------------------------------------------
//  `ContactForce.h:319` 的警告：四張表**開場是空的**、`bLoaded==false`，
//  「A caller that reads it without checking bLoaded is reproducing exactly
//  the silent-30.0 defect this wave exists to prevent.」
//  本函式照 golden 逐字翻譯，**golden 沒有那個檢查**（它的表單 ctor 保證載過），
//  所以這裡也沒有 —— 但 wb_serve 的 bring-up 已經呼叫
//  `LoadContactForceTables()`（tools/wb_serve.cpp），那就是移植樹的保證。
//  任何**新的**呼叫端如果不走那條 bring-up，必須自己先載。
// ===========================================================================
#define _USE_MATH_DEFINES   // MinGW 在 -std=c++1z（__STRICT_ANSI__）下不給 M_PI

#include "adam6024.h"

#include "ContactForce.h"        // SlkForceTables / ContactForceTables()
#include "cmydef.h"
#include "cprod.h"
#include "CosFunction.h"
#include "MachineType.h"
#include "Config.h"
#include "LastSet.h"
#include "canary_support.h"      // ShowMyMessage
#include "ainarm9045_2x8_8.h"    // iCloseSiteModeFor2x8（:48）
#include "cContact.h"            // ComputeDutCount()（本波新增，golden TfContact::DutCount 的資料半邊）

#include <cmath>

int TransformFuntion(double fInputKG, bool bDualForce, bool bSoft, int iArm)
{
    //AI(W906-P2b-CF) 20260919: golden 的四張表掛在 fContactForce 上；
    // 移植樹的家是 ContactForceTables()（P2a 接的）。下面 44 個引用全部
    // 從這一個參考出發，替換是機械的 —— 見檔頭橫幅的對照表。
    SlkForceTables& cft = ContactForceTables();
    //AI(W906-P2b-CF) 20260919: `bSoft` 在 **golden 裡也完全沒被用到** ——
    // 全 golden adam6024.cpp 只有簽章那一行提到它（實測 grep -nw）。
    // 參數照留（簽章是 golden 的、呼叫端也是 golden 的），只讓 -Wunused 閉嘴。
    (void)bSoft;

    double  fMinMPA=0.0,                                                        //最小輸出氣壓
            fMaxMPA=0.0,                                                        //最大輸出氣壓
            fMinUnit=0.0,                                                       //最小輸出單位
            fMaxUnit=0.0,                                                       //最大輸出單位
            f64KgUnit=0.0,                                                      //64KG時的單位數        //Steven 20111214 : 避免誤用
            f16KgUnit=0.0,                                                      //16KG時的單位數        //Steven 20111214 : 避免誤用
            fLoadRate=0.0;                                                      //負荷率 (HT9046<8Site)=0.9;  (HT9046>=8Site)=0.85 (HT9xxx || HT7xxx)=0.95

    double fLoadRateInd, dTempInd, fInputFInd,fInputMPAInd, fInputKGInd, fInputKGIndRel;
    int iResultTemp;

    double  fInputMPA,                                                          //單顆浮動頭的輸出氣壓
            fInputF,                                                            //單顆浮動頭的輸出壓力
            fDiameter;                                                          //浮動頭的直徑 (NS的是3mm, HT的是4mm)
    double  dTemp;

    int iResult;                                                                //最後輸出電流值
    int iTag=-1;

    double dIndex60mmLoadRate, dIndex40mmLoadRate, dIndex30mmLoadRate, dIndex56mmLoadRate;  //2014-06-26    Dell    for TSMC 高溫Load cell offset     //wei 20151005 add 56mm
    bool bNSKit=false, bNSKitSwitch=false;
    if(CosFunction.bEPUseNSSLK==true)                                           //kevin 20170804 (Stven) EP表頭另一種TYPE
    {
        bNSKit=true;                                                            //使用NS KIT
        if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        {
            if(TestIF_File.bNSKitPress)
                bNSKitSwitch=true;                                              //使用NS KIT
        }
        else
        {
            bNSKitSwitch=true;
        }
    }

    if(CosFunction.bUseLoadCellOffsetByHeater &&
       LastSet.iTemperature==Tempture_Hot)                                      //2014-06-26    Dell    for TSMC 高溫Load cell offset
    {
    // ---- golden adam6024.cpp:1078 ----
        dIndex60mmLoadRate = LastSet.dIndexLoadRate[0][0] + LastSet.dIndexLoadRate[2][0];
        dIndex56mmLoadRate = LastSet.dIndexLoadRate[0][1] + LastSet.dIndexLoadRate[2][1];   //wei 20151005 add 56mm
        dIndex40mmLoadRate = LastSet.dIndexLoadRate[0][2] + LastSet.dIndexLoadRate[2][2];
        dIndex30mmLoadRate = LastSet.dIndexLoadRate[0][3] + LastSet.dIndexLoadRate[2][3];
    }
    else if(bNSKit && bNSKitSwitch &&                                                                                //kevin 20170804 (Steven) 使用另一種EP 壓力表
            (TestIF_File.bNSKitPress ||
             TestIF_File.bNS7000kit ||
             TestIF_File.bNS7000CS ||
             TestIF_File.bNS8000CS))                                            //wei 20150303   京元NS浮動頭
    {
        dIndex60mmLoadRate = LastSet.dIndexLoadRate[1][0];
        dIndex56mmLoadRate = LastSet.dIndexLoadRate[1][1];                      //wei 20151005 add 56mm
        dIndex40mmLoadRate = LastSet.dIndexLoadRate[1][2];
        dIndex30mmLoadRate = LastSet.dIndexLoadRate[1][3];
    }
    else if(EP_Install==5)
    {
        if(iArm==0)
        {
            dIndex60mmLoadRate = LastSet.dIndexLoadRate[0][0];
            dIndex56mmLoadRate = LastSet.dIndexLoadRate[0][1];                  //wei 20151005 add 56mm
            dIndex40mmLoadRate = LastSet.dIndexLoadRate[0][2];
            dIndex30mmLoadRate = LastSet.dIndexLoadRate[0][3];
        }
        else
        {
            dIndex60mmLoadRate = LastSet.dIndexLoadRate[1][0];
            dIndex56mmLoadRate = LastSet.dIndexLoadRate[1][1];                  //wei 20151005 add 56mm
            dIndex40mmLoadRate = LastSet.dIndexLoadRate[1][2];
            dIndex30mmLoadRate = LastSet.dIndexLoadRate[1][3];
        }
    }
    else
    {
        dIndex60mmLoadRate = LastSet.dIndexLoadRate[0][0];
        dIndex56mmLoadRate = LastSet.dIndexLoadRate[0][1];                      //wei 20151005 add 56mm
        dIndex40mmLoadRate = LastSet.dIndexLoadRate[0][2];
        dIndex30mmLoadRate = LastSet.dIndexLoadRate[0][3];
    }

    fMaxMPA=EP_MAXKPA/1000.00;                                                  //ChungHung 20140513  統一由外面讀取
    fMinMPA=EP_MINMPA;                                                          //Steven 20190304 : 提升小公斤數的精準度

    if(EP_Install==1 || EP_Install==3 || EP_Install==5)                         //20111111 Dell //20111217 ChungHung
    {
        fMaxUnit=4095.0;
    }
    else if(EP_Install==2)
    {
        fMaxUnit=1022.0;
    }
    fMinUnit=0.0;
    fLoadRate=0.9;

    if(WEIGHT_CALIBRATION)                                                      //Steven 20111108
    {
        if(bDualForce==true)
        {
            fDiameter=(DeviceForm_File.dDieForceKitDiameter);                   //Ifor 20191003 : add Die Force 可以自定義Kit直徑
        }
        else
        {
            fDiameter=DeviceForm_File.dKitDiameter;                             //Steven 20240807 : DeviceForm --> DeviceForm_File
        }

        if(fDiameter==6.0)                                                      //Steven 20110722 : 改用fDiameter當作判斷值,並拉到外面來
        {
            f64KgUnit=IniConfig.iContactForceMap[2][1];
            f16KgUnit=IniConfig.iContactForceMap[2][0];
            fLoadRate=dIndex60mmLoadRate;
        }
        else if(fDiameter==4.0)
        {
            f64KgUnit=IniConfig.iContactForceMap[1][1];
            f16KgUnit=IniConfig.iContactForceMap[1][0];
            fLoadRate=dIndex40mmLoadRate;
        }
        else if(fDiameter==5.6)                                                 //wei 20151005 add 56mm
        {
    // ---- golden adam6024.cpp:1158 ----
            f64KgUnit=IniConfig.iContactForceMap[3][1];
            f16KgUnit=IniConfig.iContactForceMap[3][0];
            fLoadRate=dIndex56mmLoadRate;
        }
        else
        {
            f64KgUnit=IniConfig.iContactForceMap[0][1];
            f16KgUnit=IniConfig.iContactForceMap[0][0];
            fLoadRate=dIndex30mmLoadRate;
        }

        //    fMaxMPA=64;
        //    fMinMPA=16;
        //    fMaxMPA-fMinMPA=48 直接代入
        iResult=((f64KgUnit-f16KgUnit)/48)*(fInputKG-16)+f16KgUnit;             //Steven 20110312
    }
    else
    {
        //jou 2011-12-14 不能鎖死，因為沒選EP值會錯亂！
//        if(INDEX_PRESS_TYPE==e240KG) //Steven 20110310 : 240KG
//        {
            double fComplianceUnit=1.0;
            if(bDualForce==true)
            {
                fDiameter=(DeviceForm_File.dDieForceKitDiameter);               //Ifor 20191003 : add Die Force 可以自定義Kit直徑
            }
            else
            {
                fDiameter=DeviceForm_File.dKitDiameter;
            }

            switch(DeviceForm_File.iHeadDeviceCT)
            {
                case 2:                                                         // 1 Device with 1 Compliance Unit
                    fComplianceUnit=1.0;
                    dfComplianceUnit=1.0;                                       //kevin 20200313 add 浮動頭對應缸徑
                    break;
                case 3:                                                         // 2 Device with 1 Compliance Unit
                    fComplianceUnit=0.5;
                    dfComplianceUnit=0.5;                                       //kevin 20200313 add 浮動頭對應缸徑
    // ---- golden adam6024.cpp:1198 ----
                    break;
                case 4:                                                         // 4 Device with 1 Compliance Unit
                    fComplianceUnit=0.25;
                    dfComplianceUnit=0.25;                                      //kevin 20200313 add 浮動頭對應缸徑
                    break;
                case 5:                                                         // 2 Device with 4 Compliance Unit
                    fComplianceUnit=2.0;
                    dfComplianceUnit=2.0;                                       //kevin 20200313 add 浮動頭對應缸徑
                    break;
                case 6:                                                         // 8 Device with 1 Compliance Unit
                    fComplianceUnit=0.125;
                    dfComplianceUnit = 0.125;                                   //kevin 20200313 add 浮動頭對應缸徑
                    break;
            }

            if(bDualForce==false)
            {
                switch(TestIF.iTestMode)
                {
                    case SingleSite:                                            //JerryYang 20171214 (Steven) fix single site contact force問題
                        if(DeviceForm_File.bUseAddWeight)                       //kevin 20170802 (Steven) add 加重
                        {
                            fInputKG+=1.25;
                        }
                        fInputKG=fInputKG/(1.0*fComplianceUnit);
                        break;
                    case DualSite:                                              //1x2
                    case QualSite2X2N:                                          //Wei 20220216 : fixed for 2x2 nn mode
                        if(DeviceForm_File.bUseAddWeight)                       //kevin 20170802 (Steven) add 加重
                        {
                            if(TestIF.iTestMode==DualSite)
                                fInputKG+=1.25*2;
                            else
                                fInputKG+=1.25;
                        }                                                       //ChungHung 20130910 alter for SCK can close site by Index

                        if(((LastSet.bUseTestSocket[0][0][1]==false && LastSet.bUseTestSocket[1][0][1]==false) ||
                            (LastSet.bUseTestSocket[0][0][0]==false && LastSet.bUseTestSocket[1][0][0]==false)) && //Steven 20110915 : 1x2關Site,單Dut要可以壓到85KG
                            IniConfig.bD27UseSingleSite85kg)                    //2012-01-03    Dell 在1X2模式下,關Site能達85kg
                            fInputKG=fInputKG/(1.0*fComplianceUnit);
    // ---- golden adam6024.cpp:1238 ----
                        else
                            fInputKG=fInputKG/(2.0*fComplianceUnit);
                        break;
                    case DualSite2x1:                                           //ChungHung 20130910 alter for SCK can close site by Index
                        if(DeviceForm_File.bUseAddWeight)                       //kevin 20170802 (Steven) add 加重
                        {
                            fInputKG+=1.25*2;
                        }

                        if(((LastSet.bUseTestSocket[0][1][0]==false && LastSet.bUseTestSocket[1][1][0]==false) ||
                            (LastSet.bUseTestSocket[0][0][0]==false && LastSet.bUseTestSocket[1][0][0]==false))  //Steven 20110915 : 1x2關Site,單Dut要可以壓到85KG
                            && IniConfig.bD27UseSingleSite85kg)                 //2012-01-03    Dell 在1X2模式下,關Site能達85kg
                            fInputKG=fInputKG/(1.0*fComplianceUnit);
                        else
                            fInputKG=fInputKG/(2.0*fComplianceUnit);
                        break;
                    case TriSite1X3:                                            //wei 20171102 (jou) adam 1x3模式
                    case _6Site2X3N:                                            //Steven 20220425 : 2X3NN Mode
                        if(DeviceForm_File.bUseAddWeight)                       //kevin 20170802 (Steven) add 加重
                        {
                            fInputKG+=1.25*3;
                        }
                        fInputKG=fInputKG/(3.0*fComplianceUnit);
                        break;
                    case QualSite1X4:                                           //1x4
                    case QualSite2X2:                                           //2x2
                    case _8Site2X4N:                                            //Wei 20231211 : 2X4NN Mode
                        if(DeviceForm_File.bUseAddWeight)                       //kevin 20170802 (Steven) add 加重
                        {
                            fInputKG+=1.25*4;
                        }
                        fInputKG=fInputKG/(4.0*fComplianceUnit);
                        break;
                    case _8Site1X4:                                             //ChungHung 20150528 add for 海思 _8Site1x4
                        fInputKG=fInputKG/(4.0*fComplianceUnit);
                        break;
                    case _6Site2X3:                                             //ChungHung 20140115 add for 2x3_6
                        if(DeviceForm_File.bUseAddWeight)                       //kevin 20170802 (Steven) add 加重
                        {
                            fInputKG+=1.25*6;
    // ---- golden adam6024.cpp:1278 ----
                        }
                        fInputKG=fInputKG/(6.0*fComplianceUnit);
                        break;
                    case _16Site4X4:                                            //Sam 20190226 : 16Site4X4
                    case _8Site2X4:                                             //2x4
                        if(TestIF_File.bOctal_12Kit)                            //ChungHung 20140508 add for SCK
                        {
                            if(DeviceForm_File.bUseAddWeight)                   //kevin 20170802 (Steven) add 加重
                            {
                                fInputKG+=1.25*12;
                            }
                            fInputKG=fInputKG/(12.0*fComplianceUnit);
                        }
                        else
                        {
                            if(DeviceForm_File.bUseAddWeight)                   //kevin 20170802 (Steven) add 加重
                            {
                                fInputKG+=1.25*8;
                            }
                            fInputKG=fInputKG/(8.0*fComplianceUnit);
                        }
                        break;
                    case _10Site2X5:                                            //wei 20190614 10 site
                        fInputKG=fInputKG/(10.0*fComplianceUnit);
                        break;
                    case _12Site2X6:
                        if(DeviceForm_File.bUseAddWeight)                       //kevin 20170802 (Steven) add 加重
                        {
                            fInputKG+=1.25*12;
                        }
                        fInputKG=fInputKG/(12.0*fComplianceUnit);
                        break;
                    case _16Site2X8: //2x8
                        if(DeviceForm_File.bUseAddWeight)                       //kevin 20170802 (Steven) add 加重
                        {
                            fInputKG+=1.25*16;
                        }
                        //AI(W906-P2b-CF) 20260919: golden 讀 `fContact->dDutCount`（TfContact 成員）。
                        // 移植樹沒有那個全域 —— cContact.cpp 把它做成 ComputeTotalAirForce 的
                        // in/out 參數，因為表單沒移植。這裡改呼叫 `ComputeDutCount()`
                        // （cContact.cpp 尾端，golden TfContact::DutCount() 的資料半邊逐字翻譯）。
                        //
                        // ⚠ **刻意不加閘**：這一行是**除法**。閘掉 fInputKG 就不會被除，
                        //   力量會**偏大** —— 機台壓得比該壓的重，那是 fail-dangerous 的方向。
                        //   §0.5 的「加閘唯一合法理由是相依不存在」在這裡不成立：
                        //   這個相依可以存在，golden 的 DutCount() 是純邏輯。
                        // ⚠ 偏離：golden 讀快取成員，這裡當場重算。輸入相同 -> 值相同；
                        //   差別只在 golden 的快取可能是舊的。詳見 ComputeDutCount 的橫幅。
                        fInputKG=ChangeToFloatNonPcnt((double)(fInputKG), (double)(ComputeDutCount(TestIF_File.iTestMode,
                                                                                          IniConfig.bD27UseSingleSite85kg,
                                                                                          LastSet.bUseTestSocket,
                                                                                          iCloseSiteModeFor2x8,
                                                                                          TestIF_File.bOctal_12Kit,
                                                                                          TestIF_File.iSiteMap)*fComplianceUnit)); //JerryYang 20221216 : fix 16 site只開中間8site的時候氣量只剩下一半 //Steven 20260505 : add zero-guard for dDutCount
                        break;
                    case _32Site4X8N:                                           //Steven 20140619 : for 32Site
    // ---- golden adam6024.cpp:1318 ----
                    case _32Site4X8M:
                        if(DeviceForm_File.bUseAddWeight)                       //kevin 20170802 (Steven) add 加重
                        {
                            fInputKG+=1.25*16;                                  //KenHsieh 20230313 : NN Mode Dutcount 32 -> 16
                        }
                        fInputKG=fInputKG/(16.0*fComplianceUnit);               //KenHsieh 20230313 : NN Mode Dutcount 32 -> 16
                        break;
                }
            }
//        }
//        else
//        {
//            switch(TestIF.iTestMode)
//            {
//                case 0: //1x2
//                case 1: //1x1 BusyShuttle
//                    if(LastSet.bUseTestSocket[0][1]==false || LastSet.bUseTestSocket[0][0]==false)      //Steven 20110915 : 1x2關Site,單Dut要可以壓到85KG
//                        fInputKG=fInputKG/1.0;
//                    else
//                        fInputKG=fInputKG/2.0;
//                    fDiameter=4.0;
//                    break;
//                case 2: //1x4
//                case 3: //2x2
//                case 4: //2x1 BusyShuttle
//                    fInputKG=fInputKG/4.0;
//                    fDiameter=3.0;
//                    break;
//                case 5: //2x4
//                case 6: //2x8
//                    fInputKG=fInputKG/8.0;
//                    fDiameter=3.0;
//                    break;
//            }
//        }

        if(CosFunction.bUseDynamicKitDiameter==false)                           //Steven 20170605 (wei) : 可以自定義Kit直徑
        {
            if(fDiameter==6.0)                                                  //Steven 20110722 : 改用fDiameter當作判斷值,並拉到外面來
            {
    // ---- golden adam6024.cpp:1358 ----
                fLoadRate=dIndex60mmLoadRate;                                   //wei 20150303     IniConfig.dIndex40mmLoadRate-->dIndex60mmLoadRate
            }
            else if(fDiameter==4.0)
            {
                fLoadRate=dIndex40mmLoadRate;                                   //wei 20150303     IniConfig.dIndex40mmLoadRate-->dIndex40mmLoadRate
            }
            else if(fDiameter==5.6)
            {
                fLoadRate=dIndex56mmLoadRate;                                   //wei 20151005 add 56mm
            }
            else
            {
                if(fDiameter==0)                                                //Steven 20140627 : 避免分母為0
                    fDiameter=3.0;
                fLoadRate=dIndex30mmLoadRate;
            }
        }
        else
        {
            if(bDualForce==true)                                                //Ifor 20191003 : add Die Force 可以自定義Kit直徑
            {
                if(INSTALL_DOUBLE_EP==0)                                        //JerryYang 20210119 : 增加dual EP防呆
                {
                   ShowMyMessage("無安裝dual force,請確認硬體選項");
                }

                if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI && TestIF_File.bIndEPSLK==true)         //==> //Eastsun 20260525 INSTALL_DOUBLE_EP_3 整合 begin
                {
                    int k=0;
                    for(int i=0; i<cft.iDieForceTypeTokens; i++)
                    {
                        for(int j=0; j<8; j++)
                        { if(i*8+j>=(int)cft.DieForceOneByOneSLKClass.size()) break;   //AI(W906-ST02-ADAM) 20261002 (St02-E helper H2): [912] golden 912 adam6024.cpp:1396-1398 F15 guard (AI(mg899to910) 20260902: fewer entries than Count*8 can be built -- empty or <=15.0 tokens are dropped -- so check before reading); 906 :1390-1391 has none
                            if(cft.DieForceOneByOneSLKClass.items[i*8+j].dDiameter==fDiameter*10)
                            {
                                fInputKGInd=fInputKG;
                                fLoadRateInd=cft.DieForceOneByOneSLKClass.items[i*8+j].dLoadRate;
                                if((fInputKGInd-cft.DieForceOneByOneSLKClass.items[i*8+j].dContactOffset)>0)
                                    fInputKGIndRel=fInputKGInd-cft.DieForceOneByOneSLKClass.items[i*8+j].dContactOffset;
                                dTempInd=(fDiameter*fDiameter*M_PI/4.0*fLoadRateInd);
    // ---- golden adam6024.cpp:1398 ----
                                if(dTempInd!=0) fInputFInd=fInputKGIndRel/dTempInd;
                                else            fInputFInd=fMinMPA;
                                fInputMPAInd=fInputFInd/10.197;
                                iResultTemp=((fMaxUnit-fMinUnit)/(fMaxMPA-fMinMPA))*(fInputMPAInd-fMinMPA)+fMinUnit;
                                iResultTemp=CheckRange(iResultTemp, 0, int(fMaxUnit));
                                iResultTemp=iResultTemp*16;
                                if(k<8) { if(DeviceForm_File.bUseDieForce==true)   //AI(W906-ST02-ADAM) 20261002 (St02-E helper H2): [912] golden 912 :1412-1418 F15 clamp ("k 超過 8 會蓋掉 DualSite 用的 [8]/[9], 再多會寫進隔壁 iAPAXDualEPValue"); the block closes on :467; 906 :1404-1407 unclamped
                                    iAPAXDualEPValue[k]=iResultTemp;
                                else
                                    iAPAXDualEPValue[k]=0; }   //AI(W906-ST02-ADAM) 20261002 (St02-E helper H2): [912] closes the golden 912 :1412 `if(k<8)` block opened on :464
                                k++;
                            }
                        }
                    }
                }
                else
                {
                    for(unsigned int i=0; i<cft.DieForceSLKClass.size(); i++)
                    {
                        double d=cft.DieForceSLKClass.items[i].dDiameter/10.0;
                        if(d - fDiameter < 1e-6 && d - fDiameter > -1e-6)  //AI(W906-P18-A3) 20260924: 偏離 golden 原文 `d==fDiameter`、行為對齊 BCB6（不是字面值但機制相同：Release -O3 時 56/10.0 留在 80-bit、與記憶體裡的 5.6 判不等），A3 範圍外、依 20260917 常設裁決（≥90%）修；量測見 docs/FP_ORACLE_FINDINGS.md §8
                        {
                            iTag=i;
                        }
                    }

                    if(iTag==-1 || iTag>(int)cft.DieForceSLKClass.size())
                        iTag=0;

                    if((fInputKG-cft.DieForceSLKClass.items[iTag].dContactOffset)>0)
                    {
                        fInputKG=fInputKG-cft.DieForceSLKClass.items[iTag].dContactOffset;
                    }

                    fLoadRate=cft.DieForceSLKClass.items[iTag].dLoadRate;
                }
            }
            else
            {
                for(unsigned int i=0; i<cft.SLKClass.size(); i++)
    // ---- golden adam6024.cpp:1438 ----
                {
                    double d=cft.SLKClass.items[i].dDiameter/10.0;

                    if(CUSTOMER_CODE==CC_KYEC_LEE)                              //Ifor 20200407 : Fix KYEC 特殊缸徑造成資料異常
                    {
                        if(d - 2.8 < 1e-6 && d - 2.8 > -1e-6)  //AI(W906-P18-A3) 20260924: 偏離 golden 原文 `d==2.8`、行為對齊 BCB6（Release -O3 時 d 留在 80-bit 暫存器、== 判假），使用者裁決 A3；量測見 docs/FP_ORACLE_FINDINGS.md §8
                        {
                            d=3.0;
                        }
                        else if(d - 5.8 < 1e-6 && d - 5.8 > -1e-6)  //AI(W906-P18-A3) 20260924: 偏離 golden 原文 `d==5.8`、行為對齊 BCB6（同上一行），使用者裁決 A3；量測見 docs/FP_ORACLE_FINDINGS.md §8
                        {
                            d=6.0;
                        }
                    }

                    if(d - fDiameter < 1e-6 && d - fDiameter > -1e-6)  //AI(W906-P18-A3) 20260924: 偏離 golden 原文 `d==fDiameter`、行為對齊 BCB6（同 :478），A3 範圍外、依 20260917 常設裁決（≥90%）修；量測見 docs/FP_ORACLE_FINDINGS.md §8
                    {
                        if(EP_Install==5)
                        {
                            if(iArm==0)
                                iTag=i;
                            else
                                iTag=i+1;
                        }
                        else
                        {
                            iTag=i;
                        }
                        break;
                    }
                }

                if(iTag==-1 || iTag>(int)cft.SLKClass.size())
                    iTag=0;

                if(CUSTOMER_CODE==CC_ASE_SG &&
                   cft.SLKClass.items[iTag].dDiameter==80 &&
                   DeviceForm_File.dPress>240)
                {
                    iTag=iTag+1;
    // ---- golden adam6024.cpp:1478 ----
                }

                bool bUseNSKit=false;
                if(CUSTOMER_CODE==CC_KYEC_LEE)
                {
                    switch(TestIF.iTestMode)
                    {
                        case SingleSite:
                        case DualSite:
                            if(CosFunction.bCanUseBias==true && TestIF_File.bNS7000kit==true)
                            {
                                bUseNSKit=true;
                            }
                            else
                            {
                                bUseNSKit=false;
                            }
                            break;
                        case QualSite1X4:
                            if((CosFunction.bCanUseBias==true && TestIF_File.bNS7000kit==true) || TestIF_File.bNS7000CS==true)
                            {
                                bUseNSKit=true;
                            }
                            else
                            {
                                bUseNSKit=false;
                            }
                            break;
                        case QualSite2X2:
                            if((CosFunction.bCanUse2x2Bias==true && TestIF_File.bNS7000kit==true) || TestIF_File.bNS7000CS==true)
                            {
                                bUseNSKit=true;
                            }
                            else
                            {
                                bUseNSKit=false;
                            }
                            break;
                        case DualSite2x1:
                            if(TestIF_File.bNS7000CS==true)
    // ---- golden adam6024.cpp:1518 ----
                            {
                                bUseNSKit=true;
                            }
                            else
                            {
                                bUseNSKit=false;
                            }
                            break;
                        case TriSite1X3:
                        case _8Site1X4:
                            if(ATC_SYSTEM>eATC30 && TestIF_File.bNS7000CS==true)
                            {
                                bUseNSKit=true;
                            }
                            else
                            {
                                bUseNSKit=false;
                            }
                            break;
                        case _8Site2X4:
                            if((ATC_SYSTEM>eATC30 && TestIF_File.bNS7000CS==true) ||
                               (USE_16_HEATER!=eht4Heater && TestIF_File.bNS8000CS==true))
                            {
                                bUseNSKit=true;
                            }
                            else
                            {
                                bUseNSKit=false;
                            }
                            break;
                        default :
                            bUseNSKit=false;
                            break;
                    }
                }
                else
                {
                    bUseNSKit=false;
                }

    // ---- golden adam6024.cpp:1558 ----
                if(TestIF_File.bNSKitPress || bUseNSKit==true)
                {
                    if((fInputKG-cft.SLKClass.items[iTag].dContactOffset_NS)>0)    //kevin 20170807
                    {
                        fInputKG=fInputKG-cft.SLKClass.items[iTag].dContactOffset_NS;
                    }
                }
                else
                {
                    if(INSTALL_DOUBLE_EP==DOUBLE_EP_INDIVIAL  && TestIF_File.bIndEPSLK==true)
                    {
                        //Jimmychiu 20230630 : Individual EP No effect contact force offset of 40mm compliance
                        //<==
                        int iSLKIndClassLen=cft.SLKIndClass.size();
                        int iKitDiameterType=0;
                        if(DeviceForm_File.dKitDiameter==2.0)
                        {
                            iKitDiameterType=0;
                        }
                        else if(DeviceForm_File.dKitDiameter==3.0)
                        {
                            iKitDiameterType=1;
                        }
                        else if(DeviceForm_File.dKitDiameter==4.0)
                        {
                            iKitDiameterType=2;
                        }
                        else
                        {
                            iKitDiameterType=-1;
                        }

                        if(iKitDiameterType>=0)
                        {
                            int itag=iKitDiameterType*16;
                            if(itag<iSLKIndClassLen)
                            {
                                if((fInputKG-cft.SLKIndClass.items[itag].dContactOffset)>0)
                                {
                                    fInputKG=fInputKG-cft.SLKIndClass.items[itag].dContactOffset;
    // ---- golden adam6024.cpp:1598 ----
                                }
                            }
                        }
                        //<==
                        //Jimmychiu 20230630 : Individual EP No effect contact force offset of 40mm compliance
                    }
                    else if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI && TestIF_File.bIndEPSLK==true)    //==> //Eastsun 20260525 INSTALL_DOUBLE_EP_3 整合 begin
                    {
                        int iSLKIndClassLen=cft.SLKIndClass.size();
                        int iKitDiameterType=0;
                        if(DeviceForm_File.dKitDiameter==2.0)      iKitDiameterType=0;
                        else if(DeviceForm_File.dKitDiameter==3.0) iKitDiameterType=1;
                        else if(DeviceForm_File.dKitDiameter==4.0) iKitDiameterType=2;
                        else                                       iKitDiameterType=-1;
                        if(iKitDiameterType>=0)
                        {
                            int itag=iKitDiameterType*8;
                            if(itag<iSLKIndClassLen)
                            {
                                if((fInputKG-cft.SLKIndClass.items[itag].dContactOffset)>0)
                                    fInputKG=fInputKG-cft.SLKIndClass.items[itag].dContactOffset;
                            }
                        }
                    }
                    else
                    {
                        if((fInputKG-cft.SLKClass.items[iTag].dContactOffset)>0)
                        {
                            fInputKG=fInputKG-cft.SLKClass.items[iTag].dContactOffset;
                        }
                    }
                }

                if(bNSKit && bNSKitSwitch &&                                    //kevin 20170804 (Steven) 使用另一種EP 壓力表
                   (TestIF_File.bNSKitPress ||
                    TestIF_File.bNS7000kit ||
                    TestIF_File.bNS7000CS ||
                    TestIF_File.bNS8000CS))                                     //wei 20150303   京元NS浮動頭
                {
                    fLoadRate=cft.SLKClass.items[iTag].dLoadRate_NS;
    // ---- golden adam6024.cpp:1638 ----
                }
                else if(bUseNSKit==true)
                {
                    fLoadRate=cft.SLKClass.items[iTag].dLoadRate_NS;
                }
                else
                {
                    fLoadRate=cft.SLKClass.items[iTag].dLoadRate;
                }

                if(CosFunction.bUseLoadCellOffsetByHeater &&
                   LastSet.iTemperature==Tempture_Hot)                          //2014-06-26    Dell    for TSMC 高溫Load cell offset
                {
                    fLoadRate+=cft.SLKClass.items[iTag].dHotOffset;
                }
            }
        }

        dTemp=(fDiameter*fDiameter*M_PI/4.0*fLoadRate);
        //if(INSTALL_DOUBLE_EP!=DOUBLE_EP_MULTI)                            //AI(ht9045-v899) 20260610: keep existing 0/1/2 EP fill; mode 3 (Multi EP) uses the 8-channel block below.
        {                                                                       //Eastsun 20260616 上面註解掉
            int k=0; int iSLKIndSize=(int)cft.SLKIndClass.size();   //AI(W906-ST02-ADAM) 20261002 (St02-E helper H2): [912] golden 912 :1671 (AI(mg899to910) 20260901: F4 bounds guard), read on :763; 906 :1659 has only `int k=0;`
            for(int i=0; i<cft.iSlkTypeIndTokens; i++)                 //JerryYang 20210413 : 讀取獨立EP offset
            {
                if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI && bDualForce==false)                       //==> //Eastsun 20260525 INSTALL_DOUBLE_EP_3 整合 begin
                {
                    for(int j=0; j<8; j++)
                    { if(i*8+j>=(int)cft.SLKIndClass.size()) break;   //AI(W906-ST02-ADAM) 20261002 (St02-E helper H2): [912] golden 912 :1678-1679 F15 guard (same reason as :449); 906 :1666 has none
                        double dDiameterCK=cft.SLKIndClass.items[i*8+j].dDiameter;
                        if(CUSTOMER_CODE==CC_KYEC_LEE)
                        {
                            if(dDiameterCK==28)      dDiameterCK=30;
                            else if(dDiameterCK==58) dDiameterCK=60;
                        }

                        if(dDiameterCK==fDiameter*10)
                        {
                            fInputKGInd=fInputKG;
                            fLoadRateInd=cft.SLKIndClass.items[i*8+j].dLoadRate;
                            if((fInputKGInd-cft.SLKIndClass.items[i*8+j].dContactOffset)>0)
    // ---- golden adam6024.cpp:1678 ----
                                fInputKGIndRel=fInputKGInd-cft.SLKIndClass.items[i*8+j].dContactOffset;
                            dTempInd=(fDiameter*fDiameter*M_PI/4.0*fLoadRateInd);
                            if(dTempInd!=0) fInputFInd=fInputKGIndRel/dTempInd;
                            else            fInputFInd=fMinMPA;
                            fInputMPAInd=fInputFInd/10.197;
                            iResultTemp=((fMaxUnit-fMinUnit)/(fMaxMPA-fMinMPA))*(fInputMPAInd-fMinMPA)+fMinUnit;
                            iResultTemp=CheckRange(iResultTemp, 0, int(fMaxUnit));
                            iResultTemp=iResultTemp*16;
                            if(k<8) iAPAXEPValue[k]=iResultTemp;   //AI(W906-ST02-ADAM) 20261002 (St02-E helper H2): [912] golden 912 :1700 F15 clamp ("k 超過 8 會蓋掉 DualSite 用的 [8]/[9], 再多會寫進隔壁 iAPAXDualEPValue"); 906 :1686 unclamped
                            k++;
                        }
                    }
                    continue;
                }
                if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)                          //Eastsun 20260616 加個保險
                    continue;

                for(int j=0; j<16; j++)
                { if(i*16+j>=iSLKIndSize) break;   //AI(W906-ST02-ADAM) 20261002 (St02-E helper H2): [912] golden 912 :1711-1712 F4 guard (slSLKTypeInd and SLKIndClass out of step); 906 :1697 has none
                    bool bDiaMatch;                                                 //==> Eastsun 20260511 F008 整合: Ifor 20200407 Fix KYEC 特殊烏徑造成資料異常
                    if(CUSTOMER_CODE==CC_KYEC_LEE)                                  //Ifor 20200407 : Fix KYEC 特殊烏徑造成資料異常
                    {
                        double dDiameterCK=cft.SLKIndClass.items[i*16+j].dDiameter;
                        if(dDiameterCK==28)      dDiameterCK=30;                    //2.8mm 視為 3.0mm
                        else if(dDiameterCK==58) dDiameterCK=60;                    //5.8mm 視為 6.0mm
                        bDiaMatch=(dDiameterCK==fDiameter*10);
                    }
                    else
                    {
                        bDiaMatch=(cft.SLKIndClass.items[i*16+j].dDiameter==fDiameter*10);  // 原 c L1473 原條件，非 KYEC 走原總輯
                    }

                    if(bDiaMatch)
                    {
                        fInputKGInd=fInputKG;
                        fLoadRateInd=cft.SLKIndClass.items[i*16+j].dLoadRate;
                        if((fInputKGInd-cft.SLKIndClass.items[i*16+j].dContactOffset)>0)
                        {
                            fInputKGIndRel=fInputKGInd-cft.SLKIndClass.items[i*16+j].dContactOffset;
                        }
    // ---- golden adam6024.cpp:1718 ----
                        dTempInd=(fDiameter*fDiameter*M_PI/4.0*fLoadRateInd);
                        if(dTempInd!=0)
                            fInputFInd=fInputKGIndRel/dTempInd;
                        else
                            fInputFInd=fMinMPA;
                        fInputMPAInd=fInputFInd/10.197;
                        iResultTemp=(int)(ChangeToFloatNonPcnt((double)(fMaxUnit-fMinUnit), (double)(fMaxMPA-fMinMPA)))*(fInputMPAInd-fMinMPA)+fMinUnit; //Steven 20260505 : add zero-guard for (fMaxMPA-fMinMPA)
                        iResultTemp=CheckRange(iResultTemp, 0, int(fMaxUnit));
                        iResultTemp=iResultTemp*16;
                        iAPAXEPValue[k]=iResultTemp;
                        k++;
                    }
                }
            }
        }

        //AI(ht9045-v899) 20260526: Multi EP (mode 3) single-force path - fill iAPAXEPValue from SLKIndClass with 8-channel layout (i*8+j). Uses main zero-guard ChangeToFloatNonPcnt.
//        if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI && bDualForce==false)           //Eastsun 20260616 註解掉
//        {
//            int kMep=0;
//            int iSLKIndSizeMep=(int)cft.SLKIndClass.size();
//            for(int iMep=0; iMep<cft.iSlkTypeIndTokens; iMep++)
//            {
//                for(int jMep=0; jMep<8; jMep++)
//                {
//                    int idxMep=iMep*8+jMep;
//                    if(idxMep>=iSLKIndSizeMep)
//                        break;
//
//                    double dDiameterMep=cft.SLKIndClass.items[idxMep].dDiameter;
//                    if(CUSTOMER_CODE==CC_KYEC_LEE)
//                    {
//                        if(dDiameterMep==28)
//                            dDiameterMep=30;
//                        else if(dDiameterMep==58)
//                            dDiameterMep=60;
//                    }
//
//                    if(dDiameterMep==fDiameter*10)
//                    {
    // ---- golden adam6024.cpp:1758 ----
//                        double fInputKGIndMep = fInputKG;
//                        double fLoadRateIndMep = cft.SLKIndClass.items[idxMep].dLoadRate;
//                        double fOffsetMep = cft.SLKIndClass.items[idxMep].dContactOffset;
//                        double fInputKGIndRelMep = fInputKGIndMep;
//                        if((fInputKGIndMep - fOffsetMep) > 0)
//                            fInputKGIndRelMep = fInputKGIndMep - fOffsetMep;
//                        double dTempIndMep = (fDiameter*fDiameter*M_PI/4.0*fLoadRateIndMep);
//                        double fInputFIndMep = (dTempIndMep!=0) ? (fInputKGIndRelMep/dTempIndMep) : fMinMPA;
//                        double fInputMPAIndMep = fInputFIndMep/10.197;
//                        int iResultTempMep = (int)(ChangeToFloatNonPcnt((double)(fMaxUnit-fMinUnit), (double)(fMaxMPA-fMinMPA)))*(fInputMPAIndMep-fMinMPA)+fMinUnit; //AI(ht9045-v899) 20260610: main zero-guard
//                        iResultTempMep = CheckRange(iResultTempMep, 0, int(fMaxUnit));
//                        iResultTempMep = iResultTempMep*16;
//                        if(kMep<8) iAPAXEPValue[kMep] = iResultTempMep;
//                        kMep++;
//                    }
//                }
//            }
//        }

        if(dTemp!=0)
            fInputF=fInputKG/dTemp;
        else
            fInputF=fMinMPA;
        fInputMPA=fInputF/10.197;

        //AI(W906-P2b-CF) 20260919: GATE (W906-P2B-CONTACTDISPLAY) —— 缺相依：
        //   `KpaTransferKG()`      只存在於 atester.cpp 的 #if 0 裡
        //   `fContact->edSetKg`    全域 fContact 是 `TfContactShim`
        //   `fContact->edTransfer`   （atester_shims.h:251），沒有這些 widget
        //   `fContact->edAirKPA`
        // 行為：Contact 畫面上的「設定公斤數 / EP 換算公斤數 / 輸出氣壓」三個
        //   欄位不會被更新。**純顯示，不影響回傳值** —— 下一行的 `iResult`
        //   計算完全沒被閘，所以寫出去的類比碼與 golden 相同。
        // ⇒ 這是 fail-safe 的方向：少顯示三個數字，不會讓機台壓錯力道。
        // UN-GATE：等真的 TfContact 有實例、且 KpaTransferKG 解閘。
#if 0 // GATE (W906-P2B-CONTACTDISPLAY): 缺 KpaTransferKG 與 fContact 的三個 widget
        double dbTransferKg = KpaTransferKG(fInputMPA*1000);                    //Ifor 20150907 :新增顯示目前下壓公斤數與EP轉換公斤數

        if(bDualForce==false)                                                   //kevin 20220225 Die Force  不改資料
        {
            fContact->edSetKg->Text=FormatFloat("0.0000", fInputKG);
            fContact->edTransfer->Text=FormatFloat("0.0000", dbTransferKg/1000);
            fContact->edAirKPA->Text=FormatFloat("0.0000", fInputMPA);          //Steven 20160630 : 在Contact畫面顯示輸出的壓力值
        }
#endif // GATE (W906-P2B-CONTACTDISPLAY)
        iResult=(int)(ChangeToFloatNonPcnt((double)(fMaxUnit-fMinUnit), (double)(fMaxMPA-fMinMPA)))*(fInputMPA-fMinMPA)+fMinUnit; //Steven 20260505 : add zero-guard for (fMaxMPA-fMinMPA)
    }
    int iMax = fMaxUnit;                                                        //20111111  Dell
    iResult=CheckRange(iResult, 0, iMax);
    return iResult;
}
//==============================================================================
//AI(W906-FLOW-2) 20260928: golden adam6024.cpp:3016-3101 -- EPSwitchOnOff + EpSwitch
//  (Steven 20250401 整合EP開關), decoded from golden cp950 line for line (not hand-copied).
//  Until now EPSwitchOnOff was an empty stand-in at atester_shims.cpp:333 ("offline: no EP DAQ").
//  That reason was wrong: golden does not touch the EP DAQ here -- it switches four DIGITAL outputs
//  through SW[] (SwEpArm1/2 cmydef.cpp:1996-1997, SwIndEpArm1/2 cmydef.cpp:2204-2205), and every
//  symbol it reads is live in the port:
//    SW[] / TMySwitch::Enable / OnOff            myswitch.h:35-36, myswitch.cpp:213 (ht9045_io)
//    FrontTestHeadHasIC / RearTestHeadHasIC      csystem.cpp:19107 / :19112 (ht9045_sm; golden csystem.cpp)
//    fiosetview->fShow                           atester_shims.h:359 / atester_shims.cpp:380 (ht9045_sm)
//    TestIF_File.bArm1PickPlaceArm2Test          cprod.h:2132
//    IniConfig.bD30EnableSiteModeSelect          Config.h:506
//    TestIF.iShuttleMode / TestIF.iShuttle_Sel   cprod.h:1655-1656
//  Declarations (enum EPSwOn, EPSwitchOnOff, EpSwitch) are in atester_shims.h:281-282, the port's
//  stand-in for golden adam6024.h:45-51; the old stub line is now a forward declaration.  The three
//  includes sit here, not at the top of the file, so no line above moves.
//  GOLDEN ODDITY KEPT: in the fShow (IO screen open) branch golden's comment says "Index 有IC的話,
//  狀態不可以改變" but the code switches ONLY when that head HAS an IC -- translated as written.
//  What it does: under SOFT_SIMULTE every SW[i].Enable is forced false (cinitial.cpp:1563-1565), so
//  the sim still switches nothing; on a real machine it drives whichever of the four outputs
//  IO_Table.csv enables (HT9050: SwIndEpArm1 only, a PCI-1203 output -- machines/HT9050/IO_Table.csv:839).
//==============================================================================
#include "atester_shims.h"          // enum EPSwOn / EPSwitchOnOff / EpSwitch declarations + fiosetview
#include "myswitch.h"               // SW[] (TMySwitch)
#include "csystem.h"                // FrontTestHeadHasIC / RearTestHeadHasIC

void EPSwitchOnOff(int iArm)                                                    // 0:全關, 1:Arm1開, 2:Arm2開, 3:全開 //Steven 20250401 : 整合EP開關
{
    if(iArm==eEPSwOff)
    {
        EpSwitch(false, false);
    }
    else if(TestIF_File.bArm1PickPlaceArm2Test==true)
    {
        EpSwitch(true, true);
    }
    else if(IniConfig.bD30EnableSiteModeSelect &&
            TestIF.iShuttleMode==1)                                             //JerryYang 20240111 : add
    {
        if(TestIF.iShuttle_Sel==0)                                              //Front Arm Only
        {
            EpSwitch(true, false);
        }
        else if(TestIF.iShuttle_Sel==1)                                         //Rear Arm Only
        {
            EpSwitch(false, true);
        }
    }
    else
    {
        if(iArm==eEPSwOff)
            EpSwitch(false, false);
        else if(iArm==eEPSwArm1)
            EpSwitch(true, false);
        else if(iArm==eEPSwArm2)
            EpSwitch(false, true);
        else
            EpSwitch(true, true);
    }
}
//------------------------------------------------------------------------------
void EpSwitch(bool Arm1, bool Arm2)                                             //Steven 20250401 : 整合EP開關
{
    if(W906_FormShowing("fiosetview", fiosetview->fShow)==true)                 //IO畫面  //AI(W906-FLOW-2) 20260929: read the IO screen state through St01's page table (W906FormShowing.h, csystem.h:421) like TriTemp.cpp:3058 -- FShow_Audit baseline 0 for this file after the St01 380a6a8e merge; also closes INBOX row 106 (the branch was unreachable while fShow stayed false)
    {
        if(SW[SwEpArm1].Enable==true)                                           //Steven 20110708
        {
            if(FrontTestHeadHasIC())                                            //Index 1 有IC的話,狀態不可以改變
                SW[SwEpArm1].OnOff(Arm1);
        }

        if(SW[SwEpArm2].Enable==true)                                           //Index 2 有IC的話,狀態不可以改變
        {
            if(RearTestHeadHasIC())
                SW[SwEpArm2].OnOff(Arm2);
        }

        if(SW[SwIndEpArm1].Enable==true)                                        //Steven 20110708
        {
            if(FrontTestHeadHasIC())                                            //Index 1 有IC的話,狀態不可以改變
                SW[SwIndEpArm1].OnOff(Arm1);
        }

        if(SW[SwIndEpArm2].Enable==true)                                        //Index 2 有IC的話,狀態不可以改變
        {
            if(RearTestHeadHasIC())
                SW[SwIndEpArm2].OnOff(Arm2);
        }
    }
    else
    {
        if(SW[SwEpArm1].Enable==true)                                           //Steven 20110708
        {
            SW[SwEpArm1].OnOff(Arm1);
        }

        if(SW[SwEpArm2].Enable==true)
        {
            SW[SwEpArm2].OnOff(Arm2);
        }

        if(SW[SwIndEpArm1].Enable==true)                                        //Steven 20110708
        {
            SW[SwIndEpArm1].OnOff(Arm1);
        }

        if(SW[SwIndEpArm2].Enable==true)                                        //Index 2 有IC的話,狀態不可以改變
        {
            SW[SwIndEpArm2].OnOff(Arm2);
        }
    }
}
//---------------------------------------------------------------------------
