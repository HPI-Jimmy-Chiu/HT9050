// =============================================================================
//  cDIOStatus.cpp  --  TfMain::InitDIOStstus（TTLCfg → Prod.DIOCfg，並把 TTL 輸出設回初始狀態）＋ 開機輔助 W906_BootInitDIOStstus()
//
//  AI(W906-DIO) 20260925: 新檔（Steven 0925 08:17 交辦）。golden 對照：HT9011UC_Code_V3.33.906.0_20260618/main.cpp（cp950）
//    :24357-24375  void __fastcall TfMain::InitDIOStstus(bool bNeedOn)（宣告 main.h:1310，Steven 20110920）
//    :10106-10108  TfMain::FormShow 的 `#ifndef SOFT_SIMULTE  InitDIOStstus(false);  #endif`（本檔檔尾的開機輔助，
//                  tools/wb_serve.cpp 在 DoReadLastData／ReadLastSetIni 之後呼叫）
//    :27670-27672  TfMain::sbDioSetClick 的 `#ifndef SOFT_SIMULTE  InitDIOStstus(true);  #endif`（DIO 設定頁關表單之後；
//                  移植樹在 FileRW/TTLCfg.cpp 的 SaveFlow，寫了 DIO 檔、GetDIOFileName＋LoadData 重讀之後）
//  翻譯對照一律 906（docs/RULINGS_20260925.md 第 15 條）。本體由腳本從 golden 逐行複製，只改兩處：
//  拿掉 __fastcall；TTLLog 那一行閘住（見下方「閘」）。
//
//  ## 它做什麼（golden 本體整段在 #ifndef SOFT_SIMULTE 裡 —— 模擬組態是空函式；兩個呼叫點 golden 也各自再閘一次，照翻）
//    1. memcpy TTLCfg → Prod.DIOCfg：整個 TTL_DATA（cprod.h:18-31，cModeName[128]＋11 個 int）；兩邊同型別，sizeof 相同
//    2. 8 站（Alick 20161011）：SW[TTL_StartData[i]].Type=iSTLogicMode 後 Off()；
//       iDutType==DUTPosPluse → SW[TTL_Dut[i]].On()（golden 註解 0v），否則 Off()（5V）
//    3. SW[SwClear2]／SW[SwClear6].OnOff(bNeedOn) —— 開機 false（關）、DIO 設定頁存檔後 true（開）
//
//  ## TTL 輸出實際會到哪裡（20260925 量，不是推論）
//    * TMySwitch::On()/Off()（myswitch.cpp:74／:124）：先寫 OutValue，Enable==false 就 return（:86／:136），不碰任何後端。
//    * InitialSwitch（cinitial.cpp:1511，IO_CARD_TYPE==NewIO_MN200||PCI_P64C64 那半）依 IO_Table.csv 的 Alias 綁 SW[]；
//      找不到 Alias → Enable=false（:1524／:1561）。machines/HT9050/IO_Table.csv 與這台筆電的 D:\HT9045\system\IO_Table.csv
//      都沒有 SwStart0-7／SwDut0-7／SwClear2／SwClear6 這 18 列（grep 0 筆；唯一相近的是 SwDutHeaterCoolFan）
//      ⇒ HT9050 出貨組態上這 18 點全部 Enable=false：本函式只改 Type／OutValue，不會有任何 IO 寫出。
//      tests/test_init_dio_status.cpp 第三段用真表量這件事。
//    * 有綁的機台（IO 表有這 18 列）：ISABase eMotionNet(0)／ePCI1203(3) → MyLaneIO.IOBitOn/IOBitOff（MyLaneIo.cpp:257／:324）
//      → pIOMN200／pIOMnet／pIO1203，出貨組態由 InitHontechHardware 開頭的 SelectVendorBackends（MyLaneIo.cpp:913）換成真後端，
//      沒有對應 SDK 的建置（HAVE_PCI1203／HAVE_MN200 未定義）是回 0 的樁（IOBackend.cpp）；
//      eISABase(1)／ePCI1735U(2)／ePLCbase(4) → myio.cpp IOBitOn/IOBitOff（x64 上原始 port 寫入是 GATE (1)，no-op）。
//    * TTL_CARD_TYPE（筆電 Gerneral.ini:33 = 2，RS232 TTL 板）golden 本函式不看，照翻。
//    * 模擬組態：本體是空的（golden）。cinitial.cpp:1657-1676 模擬時把 SwStart0-7／SwDut0-7 強制 Enable=true，
//      但本函式不跑，所以不影響。
//    沒有加任何 golden 沒有的閘（交辦：「硬體輸出照 golden 打，不要加額外的閘」）。
//
//  ## 閘：1 個（相依不存在）
//    golden :24373 TTLLog("InitDIOStstus") —— TTLLog 的本體在 cpublic.cpp:667-694 的 `#if 0 // TODO(GA1-B3)` 裡
//    （它寫 fMain->slTTLLog，TfMain 門面沒有這個成員；golden 是 main.cpp:1526 new 的 TMyStringList "TTL_Signal_LOG"），
//    cpublic.h:42 只有宣告 ⇒ 呼叫會連結失敗。TTL 訊號 log（D:\HT9045_Log\TTL_Signal_LOG）因此不寫。
//
//  ## 為什麼不放 forms/fMain.cpp
//    同 cSensorScan.cpp／cCleanOut.cpp：forms/fMain.cpp 在 ht9045_forms，只准連 vclcompat＋ht9045_globals＋ht9045_core；
//    本體要 SW[]（myswitch.cpp，ht9045_io）。宣告在 forms/fMain.h（非 virtual，同 golden main.h:1310；virtual 會讓 forms 的
//    vtable 引用 sm 的符號）。
// =============================================================================
#include "forms/fMain.h"           // TfMain、fMain（forms/fMain.h:1247）
#include "vclcompat/vcl_compat.h"  // AnsiString
#include "cprod.h"                 // Prod（cprod.h:1138，.DIOCfg :597）、TTLCfg（cprod.h:32）、TTL_DATA（cprod.h:18-31）；也帶進 MachineType.h（SOFT_SIMULTE）
#include "cmydef.h"                // TTL_StartData[8]（cmydef.h:2693）、TTL_Dut[8]（:2694）、SwClear2（:1747）、SwClear6（:1860）、DUTPosPluse（:3078）
#include "myswitch.h"              // SW[MAX_SWITCH_ITEM]、TMySwitch::On/Off/OnOff

#include <cstdio>
#include <cstring>                 // memcpy

//------------------------------------------------------------------------------
void TfMain::InitDIOStstus(bool bNeedOn)                                     // golden main.cpp:24357（拿掉 __fastcall）
{
#ifndef SOFT_SIMULTE
    memcpy(&Prod.DIOCfg.cModeName[0], &TTLCfg.cModeName[0], sizeof(TTLCfg));
    for(int i=0; i<8; i++)                                                      //Alick 20161011 (Steven) : TTL支援8Site
    {
        SW[TTL_StartData[i]].Type=Prod.DIOCfg.iSTLogicMode;
        SW[TTL_StartData[i]].Off();

        if(Prod.DIOCfg.iDutType==DUTPosPluse)
            SW[TTL_Dut[i]].On();                                                //0v
        else
            SW[TTL_Dut[i]].Off();                                               //5V
    }
    SW[SwClear2].OnOff(bNeedOn);
    SW[SwClear6].OnOff(bNeedOn);                                                //Alick 20161011 (Steven) : TTL支援8Site
#if 0 // AI(W906-DIO) 20260925: GATE（相依不存在）—— golden :24373。TTLLog 本體在 cpublic.cpp:667-694 的 #if 0 (TODO(GA1-B3)) 裡（要 fMain->slTTLLog，TfMain 門面沒有），cpublic.h:42 只有宣告，呼叫會連結失敗
    TTLLog("InitDIOStstus");                                                    //Steven 20151123 : Log for TTL
#endif
#endif
}
//------------------------------------------------------------------------------

// =============================================================================
//  W906_BootInitDIOStstus  --  golden TfMain::FormShow main.cpp:10106-10108
//
//  AI(W906-DIO) 20260925: wb_serve 開機呼叫（tools/wb_serve.cpp，DoReadLastData 片段＋ReadLastSetIni／SetWorkParameter 之後、
//  WebLogin_Boot 之前 —— golden 的 :10107 在 DoReadLastData :9562／ReadLastSetIni :9564 之後、szSupervisor :10566 之前）。
//  包成自由函式的理由同 cSensorScan.cpp 的 W906_BootSimLoaderCheckBox：wb_serve 用區塊內 extern 呼叫，不必在它的檔頭加 include。
//  ⚠ 移植樹開機沒有 golden DoReadLastData main.cpp:8990-8991 的 GetDIOFileName＋LoadData（FileRW/TTLCfg.cpp 檔尾
//    FileRW_TTLCfg_Boot 的註解：「本檔不做，整合者決定」），所以這裡複製進 Prod.DIOCfg 的 TTLCfg 是開機時的值（沒讀檔就是全 0），
//    與移植樹 SetWorkParameter（cinitial.cpp:8664）早一步複製的那一份相同。
//  printf 是整合診斷（golden 沒有）：印出這一次做了什麼、18 個 TTL 輸出點有幾個真的綁在 IO 表上。
// =============================================================================
void W906_BootInitDIOStstus()
{
    //AI(W906-DIO-BOOTGATE) 20260925: 開機這一處先閘住 —— 相依不存在：golden 在它之前的 DoReadLastData main.cpp:8990-8991
    //  `fDIOFrom->GetDIOFileName(); fDIOFrom->LoadData(S);` 把 DIO 檔讀進 TTLCfg，移植樹開機沒有這一步（FileRW/TTLCfg.cpp 檔尾「本檔不做」）。
    //  照翻會用全 0 的 TTLCfg 打 TTL 輸出：SW[TTL_StartData].Type 被設成 0，TMySwitch::Off() 在 Type==0 時走 IOBitOn ⇒
    //  在 IO 表有綁 SwStart0-7、且 DIO 檔 iSTLogicMode=1 的機台上，開機會用和 golden 相反的極性打 START 線（R28 獨立審查 20260925）。
    //  HT9050 與筆電的 IO 表都沒有這 18 列（實際寫出 0 次），所以閘住不改變任何現況；DIO 設定頁存檔那一處（FileRW/TTLCfg.cpp）照 golden 活著。
    //  解閘條件：開機讀 DIO 檔翻進來（golden 的副作用要一起翻：可能 CopyFile 母檔進配方資料夾、DIO 檔不存在時 SystemStart=false、ELMessage）。
    #if 0
    #ifndef SOFT_SIMULTE
    fMain->InitDIOStstus(false);                                                // golden main.cpp:10107（FormShow 的成員語法 → 自由函式要 fMain->）
    #endif
    #endif

    int iBound=0;
    for(int i=0; i<8; i++)
    {
        if(SW[TTL_StartData[i]].Enable) iBound++;
        if(SW[TTL_Dut[i]].Enable)       iBound++;
    }
    if(SW[SwClear2].Enable) iBound++;
    if(SW[SwClear6].Enable) iBound++;
    #ifndef SOFT_SIMULTE
    std::printf("InitDIOStstus(false) (golden FormShow main.cpp:10106-10108): GATED -- the boot DIO file read before it"
                " (golden DoReadLastData main.cpp:8990-8991) is not ported, so TTLCfg would be all zero; TTL outputs bound in"
                " IO table: %d of 18 (SwStart0-7/SwDut0-7/SwClear2/SwClear6)\n", iBound);
    #else
    std::printf("InitDIOStstus(false): skipped -- SOFT_SIMULTE build (golden main.cpp:10106-10108 and the body :24359 are both"
                " #ifndef SOFT_SIMULTE); TTL switch points enabled: %d of 18\n", iBound);
    #endif
}
