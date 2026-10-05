// =============================================================================
//  rs232.cpp  --  golden TCOM2 的 Index Z 扭力那一半（rs232.cpp／rs232.h／rs232.dfm）
//
//  AI(W906-R28TORQ) 20260925: 新檔。使用者 20260925 裁決第 6 條「出貨組態的 Index Z 扭力上限要對齊原 BCB6
//  版本做法」（docs/RULINGS_20260925.md §6；NB2 R14 S1／R16，RD5軟體_NB2_出貨組態假成功替身普查_20260925_031755.md）。
//  golden：HT9011UC_Code_V3.33.906.0_20260618（翻譯對照一律 906，同檔第 15 條），cp950 → UTF-8，0 個 U+FFFD。
//
//  以前：atester.cpp 的 DoTestHeadMotor（非 SOFT_SIMULTE 臂）呼叫 COM2->iWriteAndCheckMotorTorque(<arm>, Prod.iMaxPreasure)，
//    COM2 被 `#define COM2 (&W7T1_com2_ext)` 導到回 1 的替身 ⇒ 真機上扭力上限根本沒寫進驅動器、卻回報成功；
//    接著 12110 等 edTorue0 讀回值，那也是 TU 區域替身、永遠是空字串 ⇒ 出貨組態的 Index 檢查停在 12110。
//  現在：照 golden 走真的 RS232。COM2 仍是 TCOM2Shim（移植樹唯一的 COM2 物件，atester_shims.h），這裡把 golden TCOM2
//    的扭力成員補上。golden 的 COM2 是 TCOM2（TDataModule；HT9045.cpp:174 CreateForm、rs232.cpp:34 `TCOM2 *COM2;`），
//    扭力走它的 Comm1（SPComm TComm，rs232.dfm:39-71）；Comm1 由 RS232Init（rs232.cpp:236-262）用
//    HSys.sTorqueComPort（Gerneral.ini [IndexDriver] COM_PORT）開埠，InitialHandler（cinitial.cpp:5840）呼叫。
//    週期驅動在 MainProc：COM2->ReadTorque()／COM2->ReadWriterParameter()（csystem.cpp:16848／:16860）。
//
//  這一檔翻了什麼（golden 行號）：
//    TCOM2 建構子的扭力段 :95-125 ............. W906_CreateFormBoot（見下「為什麼不在建構子」）
//    RS232Init :149-688 的扭力通道（:155-161、:236-262）..... 其餘通道閘著，見函式內
//    TorqueSend :765 / ReadIndexTorqueSetting :779 / WriteIndexTorqueSetting :789 / InitReadTorueTask :803
//    ReadTorque :809 / ReadTorque_Panasonic :820-1000
//    ReadIndexTorqueSetting_Pana :1368 / WriteIndexTorqueSetting_Pana :1414 / ResetPanasonicTime :1504
//    ReadWriterParameter :1513 / ReadWriterParameter_Panasonic :1523-1681
//    Comm1ReceiveData :1750-1833 / InitWriteAndCheckMotorTorqueTask :1837-1845 / iWriteAndCheckMotorTorque :1847-2009
//    ReadWriterParameter_Mitsubishi :2114 / ReadIndexTorqueSetting_Mitu :2144 / WriteIndexTorqueSetting_Mitu :2156
//    DoChangeASCII_TO_INT :2174 / ReadTorque_Mitsubishi :2186-2457 / InitSetTorque_Mitsubishi :2460
//    DoSet_Torque_Action :2466-2912 / InitReadTorque_Mitsubishi :2915 / DoRead_Now_Torque_Action :2920-3233
//    GetReadTorueTask :3763 / TfMain::AddTorqueLog（golden main.cpp:32586-32609）
//
//  與 golden 不同的地方（每一條都有理由；其餘逐句照翻）：
//   (1) SPComm 的交付語意補在消費端（W906_Comm1QueueRx／W906_PumpComm1）。golden 的 SPComm 在**主執行緒**呼叫
//       OnReceiveData，而且 Comm1 的 ReadIntervalTimeout=100（rs232.dfm:61）讓一包回覆（例：Panasonic 的 ACK+ENQ
//       兩個位元組、7／9 位元組的扭力值）一次到齊 —— Comm1ReceiveData 靠這兩點：它用 BufferLength==7／9 判斷是不是
//       扭力值、用 data[0]/data[1] 判斷 ACK+ENQ。vclcompat 的 TComm（vclcompat/Comm.cpp:66-93）在**讀取執行緒**上、
//       有幾個位元組就交幾個 ⇒ 一包會被切碎（BufferLength==7 永遠不成立），而且跟 MainProc 同時改 bReceive／
//       edtReadZ1->Text（AnsiString）。所以讀取執行緒只把位元組排進佇列，MainProc 的節拍（ReadTorque／
//       ReadWriterParameter 開頭，golden 每拍都呼叫它們）在「安靜滿 100 ms」之後才把整包交給 Comm1ReceiveData。
//       100 ms 就是 dfm 那個值，不是新發明的數字。
//   (2) rs232.dfm:61 的 ReadIntervalTimeout=100 **不**設到 vclcompat 的 Comm1 上：vclcompat 用同步（非 overlapped）
//       handle，非零的 interval＋零 total 會讓 ReadFile 等到第一個位元組才回來；同一個 handle 上的同步 I/O 會被
//       序列化，於是 MainProc 的 WriteCommData 會卡在讀取執行緒的 ReadFile 後面，而那個 ReadFile 等的正是這次要寫
//       出去的指令的回覆 —— 節拍執行緒會永遠卡住。維持 vclcompat 預設（0 → MAXDWORD，立刻回來），分包交給 (1)。
//   (3) HP 通訊卡（TorqueUseHPComCard，Gerneral.ini [IndexDriver] USE_HP_COM_CARD=1）那一半**閘著**。
//       為什麼該閘（陷阱 #3 重問過一次）：HP 卡的回覆**只**在 TimerHPCardTimer（rs232.cpp:4316-4670）裡解析 ——
//       Comm1ReceiveData 的 HP 分支（:1763-1776）只把位元組推進 _byte_datas、不設 bReceive；而 TimerHPCardTimer 是
//       rs232.dfm:289 的 VCL TTimer（Interval=100，RS232Init :686-687 才開），vclcompat 沒有 TTimer，移植樹也沒有它的
//       任何替代；它的本體又解參考 fMain->chtTorque（TChart 的曲線，顯示狀態在瀏覽器）、fMain->slTorqueLogNew
//       （golden main.cpp:1662 在 TfMain 建構子 new 的檔案 log，沒移植）、fMain->edtSetZ1/2、dTorqueArray 等。
//       少了它，HP 卡的寫入送得出去卻永遠等不到確認。所以四個分派點（ReadTorque／ReadWriterParameter／
//       Read/WriteIndexTorqueSetting）的 HP 臂一律不做事。**後果**：USE_HP_COM_CARD=1 的機台，
//       iWriteAndCheckMotorTorque 在 case 200 讀到 edtReadZ1="0"≠設定值，重試 5 次後回 2 ⇒ atester 顯示
//       「Motor torque set error」並停下 Index（fAllMotorHome=false）—— 看得見的失敗，不是假成功。
//       要接上：翻 TimerHPCardTimer＋HP 卡四支 _HPCard 函式（rs232.cpp:1002-1366、:1467-1502、:1683-1748），
//       並決定 TTimer 在移植樹由誰驅動（可比照 (1) 在節拍上以 100 ms 呼叫）。
//   (4) 三菱那幾個 golden 緩衝區會溢位，照「≥90% 確定是問題且知道怎麼修才修」修掉，送出的位元組不變：
//       cCheckSumTemp[3] 用 "%X" 印 3 位數的和（>0xFF）會寫 4 個位元組 → [8]；
//       cSendMitsubishi_Data[32] 被拿來 sprintf 紀錄字串（時間 8＋收到的資料最多 31＋數字）會超過 32 → [128]；
//       Comm1ReceiveData 的 `::sprintf(cReciveMitsubishi_Data, data)`（:1827）把沒有 NUL 結尾的收到資料當格式字串 →
//       改成最多 31 個位元組的有界複製（遇 NUL 停，與 sprintf 相同）。golden 的 "%C"（寬字元）→ "%c"（ASCII 相同）。
//   (5) 本檔的 file-scope 全域一律 static（golden 是外部連結，但全 golden 沒有任何一處 extern 它們 ——
//       Grep `extern\s+\w+\s+(bReceive|bRWActionFlag|…)` over golden = 0）：只改連結性，不改行為，避免跟之後翻進來的
//       同名全域撞名。
//   (6) golden 的 fContact->PnlTorue0/1（接觸畫面的扭力欄）→ 移植樹的 fContactForm->PnlTorue0/1（forms/fContact.h:1105，
//       golden TfContact 的移植；atester_shims.h 的 fContact 是另一個沒有這兩個欄位的替身）。
//   (7) fMain->slTorqueLog->AddTextWithDateTime（:1783，只在 BufferLength>2000 的異常分支）閘著：slTorqueLog 是 golden
//       main.cpp:1657 在 TfMain 建構子 new 的 TMyStringList 檔案 log，移植樹沒有；那一行只多記一筆異常 log。
//   (8) golden rs232.dfm 的 OnReceiveError／OnRequestHangup = Comm1ReceiveError／Comm1RequestHangup（只設 bCom1Error）
//       沒接：vclcompat TComm 沒有這兩個事件（vclcompat/Comm.h:39-41）。bCom1Error 只在 HP 臂（已閘）被讀。
//
//  為什麼 golden 建構子的那一段不在 TCOM2Shim 的建構子裡：COM2 是 `new TCOM2Shim()` 靜態初始化（atester_shims.cpp），
//    在 main() 之前跑；golden 那一段呼叫 CheckAndReadIniDataGeneral，它直接解參考 INIFileGeneral（common.cpp:1606-1610
//    的警告），那時還是 NULL（陷阱 #4）。所以拆成 W906_CreateFormBoot()，由 wb_serve 在 golden CreateForm(TCOM2) 的位置
//    （HT9045.cpp:174：TfMain 之後、TfTeach／TfConfiguration 之前）呼叫。
//
//  ⚠ 真實檔寫入：W906_CreateFormBoot 的 CheckAndReadIniDataGeneral 在 Gerneral.ini 缺 [IndexDriver] INDEX_DRIVER_TYPE／
//    USE_HP_COM_CARD 時會把預設值補寫回去（golden 同樣會）。其餘都是對 COM 埠與 SW[SwReadTorue] 的硬體輸出。
// =============================================================================
#include "atester_shims.h"          // TCOM2Shim / COM2 / InitWriteAndCheckMotorTorqueTask（golden rs232.h）
#include "MachineType.h"            // SOFT_SIMULTE
#include "myTimer.h"                // TQPF_Timer（golden rs232.h:7）
#include "cmydef.h"                 // INDEX_DRIVER_TYPE / TorqueUseHPComCard / iPanasonicDriverType / iTorqueCommMaxTime /
                                    // Torque[] / SwReadTorue / MTestZ1/2 / InitialOK / SystemYear.. / *_DRIVER
#include "common.h"                 // CheckAndReadIniDataGeneral
#include "cpublic.h"                // GetErrorMessage / GetTimeInfo
#include "database.h"               // HSys.sTorqueComPort
#include "myswitch.h"               // SW[]
#include "Motor/mymotor.h"          // MOT[] / iAlarmLed / iServoalarmLed
#include "EJ1N/TextProcess.h"       // GetCOMPortStatus
#include "canary_support.h"         // ShowMyMessage
#include "FormsFacade.h"            // fMain（chkReadTorque1/2、edTorue0/1、edtReadZ1/2、ListBox14、AddTorqueLog）
#include "forms/fContact.h"         // fContactForm->PnlTorue0/1（見檔頭 (6)）
#include <windows.h>
#include <vector>
#include <cstdio>
#include <cstring>
#include <cstdlib>

extern void MyDBIProcess(AnsiString S1, AnsiString S2);   // golden cMyDB.h（移植樹同其他檔的前向宣告）

// ---------------------------------------------------------------------------
//  golden rs232.cpp:40-76 的扭力全域（只收扭力用得到的；見檔頭 (5) 為什麼是 static）
// ---------------------------------------------------------------------------
static bool bRWPanasonicParameterFlag;                                          //True = Read ; False = Write   // golden :40
static int  iReadPanasonicIndex;                                                //0 = Index Arm 1 ; 1 = Index Arm 2   // golden :41

static bool bReceive=false;                                                     // golden :43
//static bool bReceive1=false;  -- golden :44（KenHsieh 20220409 nn Mode）只在 HP 臂用，已閘

//---------------------------------------------------------------------------
//Mitsubishi Driver Use  Eliot 2010_05_01
//---------------------------------------------------------------------------
static int iReadTorqueTask=1;                                                   // golden :50（golden 自己也沒用到）
static int iWriteTorqueTask=1;                                                  // golden :51
static int iRead_Now_Torque_Task=1;                                             // golden :52
static char _SOH=0x01, _STX=0x02, _ETX=0x03, _EOT=0x04;                         // golden :53
static char cSendMitsubishi_Data[128]="";                                       // golden :54 是 [32]，見檔頭 (4)
static char cReciveMitsubishi_Data[32]="";                                      // golden :55

static char cChnageIntToASCII[2]="";                                            // golden :57
static char cMotorAddress;                                                      // golden :58
static char cCommand_1,cCommand_2;                                              //命令.   // golden :59
static char cDataNo_1,cDataNo_2;                                                //命令指定資料.   // golden :60
static char cWriteDataTitle[2];                                                 // golden :61
static char cNoData[8]="";                                                      //有的資料需附指定內容.   // golden :62
static char cCheckSumTemp[8]="";                                                // golden :63 是 [3]，見檔頭 (4)
static int iCheckSum=0;                                                         // golden :64
static double fReadMotorTorque;                                                 // golden :65
static double fWriteTorque;                                                     // golden :66
static double fRead_Now_TorqueP, fRead_Now_TorqueN;                             // golden :67
static TQPF_Timer hTimeOutLimit;                                                // golden :68
static TDateTime tTimeTemp;                                                     // golden :69

static double fSetTorqueValue;                                                  // golden :71
static int iSetMitsubishiIndex;                                                 // golden :72
static int iReadMitsubishiIndex;                                                // golden :73
static bool bsetTorque_Mitsubishi=false;                                        // golden :74
static bool bReadTorque_Mitsubishi=false;                                       // golden :75
static bool bRWActionFlag=false;                                                // golden :76

// ---------------------------------------------------------------------------
//  見檔頭 (1)：SPComm「主執行緒、100 ms 安靜才交付」的語意。
//  這個結構只碰自己的成員 —— 靜態初始化時 InitializeCriticalSection 是安全的（不碰陷阱 #4 那 18 個全域）。
// ---------------------------------------------------------------------------
namespace {
struct W906_Comm1RxQueue {
    CRITICAL_SECTION         cs;
    std::vector<unsigned char> pending;
    DWORD                    lastTick;
    W906_Comm1RxQueue() : lastTick(0) { ::InitializeCriticalSection(&cs); }
};
W906_Comm1RxQueue g_comm1Rx;
const DWORD W906_COMM1_INTERVAL_MS = 100;     // rs232.dfm:61 Comm1 ReadIntervalTimeout = 100
const size_t W906_COMM1_MAX_PENDING = 2048;   // 不是 golden 的值：只為了讓一條一直在講話的線不會把佇列無限撐大（仍在 Word 範圍內）
}

//---------------------------------------------------------------------------
//  TCOM2Shim 建構子（原本在 atester_shims.cpp；移過來是因為要設定 Comm1）。
//  只做 golden「dfm 串流」那一層的事：建出 Comm1 並填 rs232.dfm:39-71 的設計期值、接 OnReceiveData。
//  golden 建構子本體（讀 Gerneral.ini）在 W906_CreateFormBoot，理由見檔頭。
//---------------------------------------------------------------------------
TCOM2Shim::TCOM2Shim()
    : bCCDDummyRum(true),                                                       // offline: dummy-run true so RTC/CCD branches short-circuit（原本的值，不變）
      Comm1(new TComm(0)), Comm2(new TComm(0)),                                // golden rs232.dfm:39 object Comm1: TComm; Comm2 = rs232.dfm:8 object Comm2: TComm (AI(W906-I03) 20261002, same line)
      rwCommandDelay(0),                                                        // golden VCL 物件零初始化；W906_CreateFormBoot 設 5（rs232.cpp:119）
      fPanasonicParameterRW(false),
      autoTask(0)                                                               // golden VCL 物件零初始化；InitReadTorueTask 在 Panasonic 時設 1
{
    Comm1->CommName="COM1";                                                     // rs232.dfm:40
    Comm1->BaudRate=9600;                                                       // rs232.dfm:41
    Comm1->ParityCheck=false;                                                   // rs232.dfm:42
    Comm1->Outx_XonXoffFlow=false;                                              // rs232.dfm:48
    Comm1->Inx_XonXoffFlow=false;                                               // rs232.dfm:49
    Comm1->ByteSize=_8;                                                         // rs232.dfm:55
    Comm1->Parity=None;                                                         // rs232.dfm:56
    Comm1->StopBits=_1;                                                         // rs232.dfm:57
    // rs232.dfm:61 ReadIntervalTimeout = 100 —— 刻意不設，見檔頭 (2)
    Comm1->OnReceiveData=[this](vclcompat::TObject * /*Sender*/, void *Buffer, Spcomm::Word BufferLength)
                         { W906_Comm1QueueRx(Buffer, BufferLength); };  W906_Comm2DfmBoot();   // rs232.dfm:66 OnReceiveData = Comm1ReceiveData（經 (1) 的佇列）；AI(W906-I03) 20261002: W906_Comm2DfmBoot = Comm2 的 rs232.dfm:8-38 設計期值＋OnReceiveData＋兩個縫（檔尾），同一行
}
//---------------------------------------------------------------------------
void TCOM2Shim::W906_Comm1QueueRx(void *Buffer, unsigned short BufferLength)   // 讀取執行緒
{
    const unsigned char *p=(const unsigned char*)Buffer;
    ::EnterCriticalSection(&g_comm1Rx.cs);
    g_comm1Rx.pending.insert(g_comm1Rx.pending.end(), p, p+BufferLength);
    g_comm1Rx.lastTick=::GetTickCount();
    ::LeaveCriticalSection(&g_comm1Rx.cs);
}
//---------------------------------------------------------------------------
void TCOM2Shim::W906_PumpComm1()                                                // MainProc 節拍執行緒
{
    std::vector<unsigned char> frame;
    ::EnterCriticalSection(&g_comm1Rx.cs);
    if(g_comm1Rx.pending.empty()==false &&
       ((DWORD)(::GetTickCount()-g_comm1Rx.lastTick)>=W906_COMM1_INTERVAL_MS ||
        g_comm1Rx.pending.size()>=W906_COMM1_MAX_PENDING))
    {
        frame.swap(g_comm1Rx.pending);
    }
    ::LeaveCriticalSection(&g_comm1Rx.cs);

    if(frame.empty())
        return;

    const unsigned short n=(unsigned short)frame.size();
    frame.push_back(0);                                                         // golden 讀 data[1]（單一位元組的 EOT 也讀）；SPComm 的緩衝區比一包大，這裡補一格 NUL 讓那一讀不越界
    Comm1ReceiveData(Comm1, &frame[0], n);
}
//---------------------------------------------------------------------------
//  golden TCOM2::TCOM2（rs232.cpp:95-125）—— 扭力那幾行照翻；其餘是別的通道的初值，列出不做。
//---------------------------------------------------------------------------
void TCOM2Shim::W906_CreateFormBoot()
{
    bsetTorque_Mitsubishi=false;                                                // golden :98
    bReadTorque_Mitsubishi=false;                                               // golden :99

    // golden :101 Com2Buffer="";  —— Comm2（溫控 KT4H）那一半，不是這條路

    INDEX_DRIVER_TYPE=CheckAndReadIniDataGeneral("IndexDriver", "INDEX_DRIVER_TYPE", Panasonic_DRIVER);            // golden :103
    TorqueUseHPComCard=CheckAndReadIniDataGeneral("IndexDriver", "USE_HP_COM_CARD", false); //Steven 20210204 : 使用鴻勁自製的通訊卡   // golden :104

    iPanasonicDriverType=Panasonic_DRIVER;                                      // A4   // golden :106
    if(INDEX_DRIVER_TYPE==Panasonic_DRIVER_A5)                                  // 2011.08.11 , Joye , Panasonic A5 //Steven 20120629 add from 7045--------
    {
        INDEX_DRIVER_TYPE=Panasonic_DRIVER;
        iPanasonicDriverType=Panasonic_DRIVER_A5;                               // A5
    }
    fPanasonicParameterRW=false;                                                // golden :112
    // golden :113-116 flagCommSD[0..3]=false —— SD tester 通道，不是這條路
    InitReadTorueTask();                                                        // golden :117
    // golden :118 bRTCVerSupportAutoTurnning=false —— RTC 視覺，不是這條路
    rwCommandDelay=5;                                                           //ChungHung 20140610 modify all Type use 0.5S   // golden :119

    // golden :121 iOpenRTCComPortAgainCount=0 —— RTC 視覺
    // golden :122-124 _byte_datas.clear()／bGetValue／bGetValue1 —— HP 通訊卡那一半（檔頭 (3)）
}
// wb_serve 用 `{ extern void W906_COM2_CreateFormBoot(); ... }` 呼叫（該檔不 include atester_shims.h，沿用它的 extern 慣例）
void W906_COM2_CreateFormBoot()
{
    COM2->W906_CreateFormBoot();
}
//---------------------------------------------------------------------------
//  golden TCOM2::RS232Init（rs232.cpp:149-688）。
//  活的：扭力通道（:155-161 檢查 HSys.sTorqueComPort、:236-262 開 Comm1）；AI(W906-I03) 20261002 起還有溫控 Comm2（:163-182、:264-281、:684-685，檔尾）。
//  閘著的（都不是這條路；它們的 TComm 成員與接收函式 Comm2ReceiveData／Comm4ReceiveData／TempComm6ReceiveData／
//  cmATC*ReceiveData／PadCommReceiveData／cmVisionLightReceiveData 在 TCOM2Shim 上都不存在 —— 只探測或打開一個
//  沒有消費者的埠，只會在開機時多跳警告、或把那個埠佔住）：
//    （:163-182 溫控 KT4H／EJ1N 的埠檢查 —— AI(W906-I03) 20261002 已接上，本體檔尾 W906_RS232InitTempCheck）
//    :184-189  Real Time CCD（HSys.sRTCComPort）       :191-196  溫度 IC（HSys.sTempDynamicComPort）
//    :198-219  ATC 1-4（SOFT_SIMULTE 臂本來就全 false）   :221-234  Tray 步進馬達／控制面板（HSys.TrayStepMotor_ComPort）
//    （:264-281 Comm2 開埠：已接上，W906_RS232InitTempOpen）      :283-560  Comm4（RTC）開埠＋RTC 指令字串表
//    :562-574  TempComm6（溫度 IC）開埠                     :576-646  ATC 1-4 開埠
//    :648-682  Vision light（cmVisionLight）
//    （:684-685 Rs232Comm2Busy／InitQue(&RxQue)：已接上，同上）
//    :686-687  TimerHPCard->Enabled=true —— HP 通訊卡（檔頭 (3)）
//---------------------------------------------------------------------------
void TCOM2Shim::RS232Init()
{
    int iErr=0;
    bool flag[11];
    ZeroMemory(flag, sizeof(flag));
    AnsiString Str;                                                             //KenHsieh 20211222 : Pad與步進馬達為同一Comport
    flag[0]=(IO_CARD_TYPE==PCI1203_IO) ? false : GetCOMPortStatus(HSys.sTorqueComPort);   //Steven 20120217 : Com Port改成可定義 -- AI(W906-HT9050-AS-LS) 20260926：改看 IO_CARD_TYPE==PCI1203_IO（開機這一刻 Mot_Table 還沒讀：cinitial.cpp:16955 RS232Init 在 :16956 InitHontechHardware→InitialMotorParameter 之前）。原註 AI(W906-TORQUE-1203) 20260926: HT9050 的扭力走 1203（rs232.cpp 檔尾），沒有 RS232 扭力驅動器 ⇒ 不探測、不開扭力埠（NB2 R46／R53：缺鍵預設 COM1，會撞到 BIN 的那一埠）

    if(flag[0]==false && IO_CARD_TYPE!=PCI1203_IO)                        // AI(W906-TORQUE-1203) 20260926: HT9050 刻意不開，不是錯誤  AI(W906-HT9050-AS-LS) 20260926：判準改成 1203 IO 卡
    {
        iErr=GetLastError();
        ShowMyMessage("Index Torque : "+HSys.sTorqueComPort+" port error , "+GetErrorMessage(iErr), "");
    }

    { extern void W906_RS232InitTempCheck(bool *flag); W906_RS232InitTempCheck(flag); }   // AI(W906-I03) 20261002: golden :163-182 溫控埠檢查（檔尾）；:184-234 其餘通道的埠檢查仍閘著，理由見函式上方

    if(flag[0])
    {
        COM2->Comm1->CommName="\\\\.\\"+HSys.sTorqueComPort;
        try
        {
            //Eliot 2010_05_01
            if(INDEX_DRIVER_TYPE==Mitsubishi_DRIVER)
                COM2->Comm1->Parity=Even;
            else
                COM2->Comm1->Parity=None;

            if(TorqueUseHPComCard)                                              //Steven 20210204 : 使用鴻勁自製的通訊卡
                COM2->Comm1->BaudRate=115200;
            else
                COM2->Comm1->BaudRate=9600;
            COM2->Comm1->ByteSize=_8;
            COM2->Comm1->ParityCheck=false;
            COM2->Comm1->StopBits=_1;

            COM2->Comm1->StartComm();                                           //僅能啟動一次
        }
        catch(...)
        {
            MyDBIProcess("Exception", "TCOM2::RS232Init");
            ShowMyMessage("Index Torque : "+HSys.sTorqueComPort+" port error", "");
        }
    }

    { extern void W906_RS232InitTempOpen(const bool *flag); W906_RS232InitTempOpen(flag); }   // AI(W906-I03) 20261002: golden :264-281 開溫控埠＋:684-685（檔尾）；:283-682 其餘通道開埠、:686-687 TimerHPCard 仍閘著，理由見函式上方
}
//----------------------------------------------------------------------------
#define MOTOR_MIN_WAIT  10                                                      //2008/07/15 lee   // golden :761
static int iPanasonicWT=500;                                                    //Sam 20200507 : 400>500   // golden :762
static int ptreot;                                                              // golden :763
static int ptrenq;                                                              // golden :764
void TCOM2Shim::TorqueSend(unsigned char *str, int len)                         // golden :765
{
    AnsiString S="Send    ";
    static char str2[256];
    Comm1->WriteCommData((char*)str, len);                                      // golden 傳 unsigned char*（BCB6 容許）；vclcompat 簽名是 char*
    for(int i=0; i<len; i++)
    {
        sprintf(str2," %02X", (unsigned)str[i]);
        S+=AnsiString(str2);
    }
    fMain->AddTorqueLog(S);
    bReceive=false;
}
//----------------------------------------------------------------------------
void TCOM2Shim::ReadIndexTorqueSetting(int Index)                               //Steven 20210223 : 整合Torque存取   // golden :779
{
    if(TorqueUseHPComCard)                                                      //Steven 20210204 : 使用鴻勁自製的通訊卡
    {
        // golden :782 ReadIndexTorqueSetting_HPCard(Index); —— HP 通訊卡那一半閘著，見檔頭 (3)
    }
    else if(INDEX_DRIVER_TYPE==Panasonic_DRIVER)                                //Panasonic
        ReadIndexTorqueSetting_Pana(Index);
    else                                                                        //Mitsubishi
        ReadIndexTorqueSetting_Mitu(Index);
}
//----------------------------------------------------------------------------
void TCOM2Shim::WriteIndexTorqueSetting(int Index, AnsiString Torque)          //Steven 20210223 : 整合Torque存取   // golden :789
{
    if(TorqueUseHPComCard)                                                      //Steven 20210204 : 使用鴻勁自製的通訊卡
    {
        // golden :792 WriteIndexTorqueSetting_HPCard(Index, atoi(Torque.c_str())); —— HP 通訊卡那一半閘著，見檔頭 (3)
    }
    else if(INDEX_DRIVER_TYPE==Panasonic_DRIVER)                                //Panasonic
        WriteIndexTorqueSetting_Pana(Index, atoi(Torque.c_str()));
    else                                                                        //Mitsubishi
        WriteIndexTorqueSetting_Mitu(Index, atof(Torque.c_str()));
}
//----------------------------------------------------------------------------
static int iPanasonicNum;                                                       // golden :799
static int iPanasonicTask=1;                                                    // golden :800
static int iReadTorueIndex;                                                     // golden :801

void TCOM2Shim::InitReadTorueTask()                                             // golden :803
{
    if(INDEX_DRIVER_TYPE==Panasonic_DRIVER)                                     //Panasonic
        autoTask=1;
}
//----------------------------------------------------------------------------
void TCOM2Shim::ReadTorque()                                                    // golden :809
{
    W906_PumpComm1();  { extern int W906_Ht9050TorqueRead(); const int w906E038=W906_Ht9050TorqueRead(); if(w906E038>=0){ if(w906E038==1) autoTask=999; else if(w906E038==2) autoTask=1; return; } }   /*AI(W906-E038) 20261003 NOT GOLDEN (St01, Q87 SAFETY): HT9050 (MOT[MTestZ1].CardType=="PCI1203", the same test as :950) reads Index Z1 torque from the 1203's 6077h instead of RS-232 (IndexZTorque1203.cpp). -1 = SOFT_SIMULTE or not a PCI1203 row -> golden dispatch below, unchanged; 1 = value written -> autoTask=999 as golden case 40 (golden 0618 rs232.cpp:991-995); 2 = not armed -> autoTask=1 as golden :854-858; 0 = armed, no value yet*/                                                           // 見檔頭 (1)：golden 在這一拍之前，主執行緒已經把收到的整包交給 Comm1ReceiveData

    if(TorqueUseHPComCard)                                                      //Steven 20210204 : 使用鴻勁自製的通訊卡
    {
        // golden :812 ReadTorque_HPCard(); —— HP 通訊卡那一半閘著，見檔頭 (3)
    }
    else if(INDEX_DRIVER_TYPE==Panasonic_DRIVER)                                //Panasonic
        ReadTorque_Panasonic();
    else
        ReadTorque_Mitsubishi();
}
//----------------------------------------------------------------------------
static TQPF_Timer ReadTorque_PanasonicDelay;                                    // golden :819
void TCOM2Shim::ReadTorque_Panasonic()                                          // golden :820
{
    #ifdef SOFT_SIMULTE
    {
        return;
    }
    #else
    {
        static int Address;
        static int ct=0, iAlarmCT=0;
        static unsigned char ENQ[1]={0x05};                                     //請求傳送資料
        static unsigned char EOT[1]={0x04};                                     //同意對方可以傳送資料
        static unsigned char ACK[1]={0x06};                                     //確定收到正確資料
        unsigned char datatrq[4]={0x00, 0x00, 0x52, 0xAE};                      //PC對馬達所傳送的資料，要求讀取扭力值

        int &Task=autoTask;
        AnsiString asString;

        if(fPanasonicParameterRW)
            return;

        if(fMain->chkReadTorque1->Checked || fMain->chkReadTorque2->Checked)
        {
            if(fMain->chkReadTorque1->Checked)
            {
                iReadTorueIndex=0;
                SW[SwReadTorue].Off();
            }
            else
            {
                iReadTorueIndex=1;
                SW[SwReadTorue].On();
            }
        }
        else
        {
            Task=1;
            iAlarmCT=0;
            return;
        }

        if(iAlarmCT>10)                                                         //jou 981225 start : rs232 fail need alarm
        {
            if(fMain->chkReadTorque1->Checked)
                asString="Z1";
            else
                asString="Z2";
            ShowMyMessage("Rs232 Read Index "+asString+" Torque error!! maybe Relay or Com Port fail, please check .", "Rs232 讀取 Index "+asString+" 扭力錯誤!! 可能是Relay或是線材脫落,請確認.");

            Task=1;
            iAlarmCT=0;
        }

        switch(Task)
        {
            case 1:
                ReadTorque_PanasonicDelay.SetMSAndOn(100);
                Task=3;
//                break;
            case 3:
                if(ReadTorque_PanasonicDelay.Off())
                {
                    Address=iReadTorueIndex;
                    Task=5;
                }
                break;
            case 5:
                TorqueSend(ENQ, 1);                                             //告知馬達要求傳送資料
                bReceive=false;
                ct=0;
                Task=10;
                break;
            case 10:
                ct++;
                if(ct>iPanasonicWT)
                {
                    Task=1;
                    Comm1->StopComm();
                    Comm1->StartComm();
                    Torque[Address]=-9999;
                    iAlarmCT++;
                    break;
                }

                if(bReceive)                                                    //待馬達回應可以傳送資料
                {
                    iAlarmCT=0;
                    if(ptreot==EOT[0])
                    {
                        datatrq[1]=Address;
                        datatrq[3]=0xAE - Address;                              // golden `0xAE-Address`：C++17 把 `0xAE-Address` 整串當成一個 pp-number（E 後面接 - 是指數記號），加空白，意思不變
                        TorqueSend(datatrq, 4);                                 //寫入資料，要求讀取扭力值
                        bReceive=false;
                        Task=20;
                        ct=0;
                    }
                    else
                    {
                        Task=1;
                        break;
                    }
                }
                break;
            case 20:
                ct++;
                if(ct>iPanasonicWT)
                {
                    Task=1;
                    Comm1->StopComm();
                    Comm1->StartComm();
                    Torque[Address]=-9999;
                    iAlarmCT++;
                    break;
                }

                if(bReceive)
                {
                    iAlarmCT=0;
                    if(ptreot==ACK[0] && ptrenq==ENQ[0])                        //待馬達告知PC要求將扭力值傳回
                    {
                        TorqueSend(EOT, 1);                                     //回應馬達可以將資料傳回
                        bReceive=false;
                        Task=30;
                        ct=0;
                    }
                    else
                    {
                        Task=1;
                        break;
                    }
                }
                break;
            case 30:
                ct++;
                if(ct>iPanasonicWT)
                {
                    Task=1;
                    Comm1->StopComm();
                    Comm1->StartComm();
                    Torque[Address]=-9999;
                    iAlarmCT++;
                    break;
                }

                if(bReceive)
                {
                    iAlarmCT=0;
                    if(ptreot==0x03)                                            //確定資料已收到
                    {
                        TorqueSend(ACK, 1);                                     //回應馬達已收到正確資料
                        Task=40;
                        ct=0;

                        if(fMain->chkReadTorque1->Checked)                      //jou 981016 start
                        {
                            fMain->edTorue0->Text=asReceiveTorue;
                            fContactForm->PnlTorue0->Caption=asReceiveTorue;    // golden fContact->PnlTorue0（檔頭 (6)）
                        }
                        else if(fMain->chkReadTorque2->Checked)
                        {
                            fMain->edTorue1->Text =asReceiveTorue;
                            fContactForm->PnlTorue1->Caption=asReceiveTorue;    // golden fContact->PnlTorue1（檔頭 (6)）
                        }
                    }
                    else
                    {
                        Task=1;
                        break;
                    }
                }
                break;
            case 40:
                Task=999;
                fMain->chkReadTorque1->Checked=false;
                fMain->chkReadTorque2->Checked=false;
                iAlarmCT=0;
                break;
        }
    }
    #endif
}
//---------------------------------------------------------------------------
//  golden :1002-1366 ReadTorque_HPCard／StartReadTorque_HPCard／StopReadTorque_HPCard／ReadVer_HPCard —— HP 通訊卡，閘著（檔頭 (3)）
//---------------------------------------------------------------------------
static unsigned char Panasonicdatatrq[20];                                      // golden :1366
static TQPF_Timer hPanasonicParameterTimeOut;                                   // golden :1367
void TCOM2Shim::ReadIndexTorqueSetting_Pana(int Index)                          // golden :1368
{
    if(Index==0)
        SW[SwReadTorue].Off();
    else
        SW[SwReadTorue].On();

    if(iPanasonicDriverType==Panasonic_DRIVER_A5)                               // 2011.08.11 , Joye , Panasonic A5 //Steven 20120629 add from 7045
    {
        iPanasonicNum=6;
        unsigned char szReadCommandStr[]={0x02, 0x00, 0x07, 0x00, 0x0D, 0x00};
        unsigned char sum=0;

        for(int i=0; i<5; i++)
            sum+=szReadCommandStr[i];

        sum=~sum+1;
        szReadCommandStr[5]=sum;

        for(int i=0; i<6; i++)
            Panasonicdatatrq[i]=szReadCommandStr[i];
    }
    else
    {
        iPanasonicNum=5;
        unsigned char szReadCommandStr[]={0x01, 0x00, 0x08, 0x5E, 0x00};
        unsigned char sum=0;

        szReadCommandStr[3]=0x5e;
        for(int i=0; i<4; i++)
            sum+=szReadCommandStr[i];

        sum=~sum+1;                                                             //checksum 二補數
        szReadCommandStr[4]=sum;
        for(int i=0; i<5; i++)
            Panasonicdatatrq[i]=szReadCommandStr[i];
    }
    fPanasonicParameterRW=true;
    hPanasonicParameterTimeOut.SetSecAndOn(10);
    bRWPanasonicParameterFlag=true;                                             //read
    iReadPanasonicIndex=Index;

    iPanasonicTask=1;                                                           //jou 2011-11-18重新initial Task
}
//----------------------------------------------------------------------------
static bool bPanasonicCommErr=false;                                            // golden :1413（golden 只寫不讀）
void TCOM2Shim::WriteIndexTorqueSetting_Pana(int Index, unsigned Data)          // golden :1414
{
    unsigned char sum=0;

    if(Index==0)
        SW[SwReadTorue].Off();
    else
        SW[SwReadTorue].On();

    if(iPanasonicDriverType==Panasonic_DRIVER_A5)                               // 2011.08.11 , Joye , Panasonic A5 //Steven 20120629 add from 7045
    {
        iPanasonicNum=10;
        unsigned char szReadCommandStr[]={0x06, 0x00, 0x17, 0x00, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00};

        szReadCommandStr[5]=Data%256;
        szReadCommandStr[6]=Data/256;
        szReadCommandStr[7]=0x00;
        szReadCommandStr[8]=0x00;

        for(int i=0; i<9; i++)
            sum+=szReadCommandStr[i];

        sum=~sum+1;
        szReadCommandStr[9]=sum;

        for(int i=0; i<10; i++)
            Panasonicdatatrq[i]=szReadCommandStr[i];
    }
    else
    {
        iPanasonicNum=7;
        unsigned char szReadCommandStr[]={0x03, 0x00, 0x18, 0x5E, 0x00, 0x00, 0x00};

        szReadCommandStr[3]=0x5e;
        szReadCommandStr[4]=Data%256;
        szReadCommandStr[5]=Data/256;
        for(int i=0; i<6; i++)
            sum+=szReadCommandStr[i];

        sum=~sum+1;
        szReadCommandStr[6]=sum;
        for(int i=0; i<7; i++)
            Panasonicdatatrq[i]=szReadCommandStr[i];
    }

    fPanasonicParameterRW=true;
    hPanasonicParameterTimeOut.SetSecAndOn(10);
    bPanasonicCommErr=false;
    bRWPanasonicParameterFlag=false;                                            //write

    iPanasonicTask=1;                                                           //jou 2011-11-18重新initial Task
}
//----------------------------------------------------------------------------
//  golden :1467-1502 ReadIndexTorqueSetting_HPCard／WriteIndexTorqueSetting_HPCard —— HP 通訊卡，閘著（檔頭 (3)）
//----------------------------------------------------------------------------
void TCOM2Shim::ResetPanasonicTime()                                            // golden :1504
{
    if(fPanasonicParameterRW)
    {
        hPanasonicParameterTimeOut.SetMSAndOn(5000);
    }
}
//----------------------------------------------------------------------------
static TQPF_Timer hPDelay;                                                      // golden :1512
void TCOM2Shim::ReadWriterParameter()                                           // golden :1513
{
    W906_PumpComm1();                                                           // 見檔頭 (1)（ReadTorque 已經抽過；再抽一次是冪等的，保證這一支單獨被呼叫時也照 golden）

    if(TorqueUseHPComCard)                                                      //Steven 20210204 : 使用鴻勁自製的通訊卡
    {
        // golden :1516 ReadWriterParameter_HPCard(); —— HP 通訊卡那一半閘著，見檔頭 (3)
    }
    else if(INDEX_DRIVER_TYPE==Panasonic_DRIVER)                                //Panasonic
        ReadWriterParameter_Panasonic();
    else                                                                        //Mitsubishi
        ReadWriterParameter_Mitsubishi();
}
//----------------------------------------------------------------------------
void TCOM2Shim::ReadWriterParameter_Panasonic()                                 // golden :1523
{
    static int Address;
    static int ct=0;
    static unsigned char ENQ[1]={0x05};                                         //請求傳送資料
    static unsigned char EOT[1]={0x04};                                         //同意對方可以傳送資料
    static unsigned char ACK[1]={0x06};                                         //確定收到正確資料

    if(fPanasonicParameterRW==false)
        return;

    int &Task=iPanasonicTask;

    if(hPanasonicParameterTimeOut.Off())                                        //10 sec time out, but 機台暫停後ResetPanasonicTime()重設5 sec
    {
        fPanasonicParameterRW=false;
        bPanasonicCommErr=true;
        return;
    }
    switch(Task)
    {
        case 1:
            hPDelay.Set0_1SecAndOn(rwCommandDelay);                             //0.1 sec  //2013-01-15    Dell 設定通訊的delay時間 (9046LS)設0.1會一直timeout
            Address=0;                                                          // which motor
            Task=5;
//            break;
        case 5:
            if(hPDelay.Off())
            {
                TorqueSend(ENQ, 1);                                             //告知馬達要求傳送資料
                bReceive=false;
                ct=0;
                Task=10;
            }
            break;
        case 10:
            ct++;
            if(ct>iPanasonicWT)
            {
                Task=1;
                Comm1->StopComm();
                Comm1->StartComm();
                break;
            }

            if(bReceive)                                                        //待馬達回應可以傳送資料
            {
                if(ptreot==EOT[0])
                {
                    TorqueSend(Panasonicdatatrq, iPanasonicNum);
                    bReceive=false;
                    Task=20;
                    ct=0;
                }
                else
                {
                    Task=1;
                    break;
                }
            }
            break;
        case 20:
            ct++;
            if(ct>iPanasonicWT)
            {
                Task=1;
                Comm1->StopComm();
                Comm1->StartComm();
                break;
            }

            if(bReceive)
            {
                if(ptreot==ACK[0] && ptrenq==ENQ[0])                            //待馬達告知PC要求將扭力值傳回
                {
                    TorqueSend(EOT, 1);                                         //回應馬達可以將資料傳回
                    bReceive=false;
                    Task=30;
                    ct=0;
                }
                else
                {
                    Task=1;
                    break;
                }
            }
            break;
        case 30:
            ct++;
            if(ct>iPanasonicWT)
            {
                Task=1;
                Comm1->StopComm();
                Comm1->StartComm();
                break;
            }

            if(bReceive)
            {
                if(iPanasonicDriverType==Panasonic_DRIVER_A5)                   // 2011.08.11 , Joye , Panasonic A5 //Steven 20120629 add from 7045
                {
                    if(iPanasonicNum==6 && ptreot==0x05)                        //確定資料已收到
                    {
                        TorqueSend(ACK, 1);                                     //回應馬達已收到正確資料
                        Task=40;
                        ct=0;
                    }
                    else if(iPanasonicNum==10 && ptreot==0x01)                  //確定資料已收到
                    {
                        TorqueSend(ACK, 1);                                     //回應馬達已收到正確資料
                        Task=40;
                        ct=0;
                    }
                    else if(iPanasonicNum==8 && ptreot==0x01)                   //2013-01-15    Dell//確定資料已收到
                    {
                        TorqueSend(ACK, 1);                                     //回應馬達已收到正確資料
                        Task=40;
                        ct=0;
                    }
                    else
                    {
                        Task=1;
                        break;
                    }
                }
                else
                {
                    if(iPanasonicNum==5 && ptreot==0x03)                        //確定資料已收到
                    {
                        TorqueSend(ACK, 1);                                     //回應馬達已收到正確資料
                        Task=40;
                        ct=0;
                    }
                    else if(iPanasonicNum==7 && ptreot==0x01)                   //確定資料已收到
                    {
                        TorqueSend(ACK, 1);                                     //回應馬達已收到正確資料
                        Task=40;
                        ct=0;
                    }
                    else
                    {
                        Task=1;
                        break;
                    }
                }
            }
            break;
        case 40:
            ct++;
            if(ct>MOTOR_MIN_WAIT)
            {
                ct=0;
                Task=1;
                fPanasonicParameterRW=false;
                break;
            }
            break;
    }
    (void)Address;                                                              // golden 設了沒讀（:1546）
}
//----------------------------------------------------------------------------
//  golden :1683-1748 ReadWriterParameter_HPCard —— HP 通訊卡，閘著（檔頭 (3)）
//----------------------------------------------------------------------------
void TCOM2Shim::Comm1ReceiveData(void * /*Sender*/, void *Buffer,              // golden :1750
      unsigned short BufferLength)
{
    if(InitialOK==false)                                                        //Steven 20120202 : 加入Thread保護
        return;

    AnsiString S1="Recv ", Arm, str3="";
    char str2[2000];
    short int k;
    unsigned char *data;                                                        // golden `byte`（windows.h rpcndr.h 的 unsigned char）；用本名避開 C++17 std::byte 的歧義
    data=(unsigned char *)Buffer;
    unsigned char str[2000]={'\0'};

    if(TorqueUseHPComCard)                                                      //Steven 20210204 : 使用鴻勁自製的通訊卡
    {
        // golden :1763-1776 把收到的位元組推進 _byte_datas，由 TimerHPCardTimer 解析 —— HP 通訊卡那一半閘著，見檔頭 (3)
        //   （留著推會讓一個沒有消費者的佇列無限長大）
    }
    else
    {
        if(BufferLength>2000)
        {
            S1.sprintf("Receive (%d): %s", BufferLength, Buffer);
            fMain->AddTorqueLog(S1);
            // golden :1783 fMain->slTorqueLog->AddTextWithDateTime(S1); —— 閘著，見檔頭 (7)
            return;
        }

        for(int i=0; i<BufferLength; i++)
        {
            sprintf(str2, " %02X", data[i]);
            S1+=str2;

            sprintf(str2, "%c", data[i]);
            str3+=str2;

            str[i]=data[i];
        }
        fMain->AddTorqueLog(str3+AnsiString(" : ")+S1);                         //Steven 20160908 : 整合成Function

        if(INDEX_DRIVER_TYPE==Panasonic_DRIVER)                                 //Panasonic
        {
            if(BufferLength==7 ||
               (iPanasonicDriverType==Panasonic_DRIVER_A5 && BufferLength==9))  // 2011.08.11 , Joye , Panasonic A5 //Steven 20120629 add from 7045
            {
                k=str[4];
                k<<=8;
                k+=str[3];
                if(str[1]<6)
                {
                    if(k<0)
                        k=0;
                    Torque[iReadTorueIndex]=(double)(k)/20.0;
                    sprintf(str2, "%5.2f", Torque[str[1]]);                     // golden 原樣：Torque[] 只有 5 格（cmydef.h:2589），str[1]==5 時會讀到界外；伺服回覆的位址位元組只會是 0／1
                    asReceiveTorue=str2;
                }

                if(fPanasonicParameterRW && bRWPanasonicParameterFlag)          //read
                {
                    if(iReadPanasonicIndex==0)
                        fMain->edtReadZ1->Text=k;
                    else
                        fMain->edtReadZ2->Text=k;
                }
            }
        }
        else
        {
            // golden :1827 ::sprintf(cReciveMitsubishi_Data, data); —— 有界複製，見檔頭 (4)
            int n=0;
            for(; n<(int)BufferLength && n<(int)sizeof(cReciveMitsubishi_Data)-1 && data[n]!='\0'; n++)
                cReciveMitsubishi_Data[n]=(char)data[n];
            cReciveMitsubishi_Data[n]='\0';
        }
        bReceive=true;
    }
    ptreot=data[0];
    ptrenq=data[1];
}
//---------------------------------------------------------------------------
// 10/01
//---------------------------------------------------------------------------
static int iWriteAndCheckMotorTorqueTask=1;                                     // golden :1837
void InitWriteAndCheckMotorTorqueTask()                                         // golden :1838
{
    iWriteAndCheckMotorTorqueTask=1;
    fMain->edtReadZ1->Text=0;
    fMain->edtReadZ2->Text=0;
    fMain->chkReadTorque1->Checked=false;
    fMain->chkReadTorque2->Checked=false;
}
//---------------------------------------------------------------------------
int TCOM2Shim::iWriteAndCheckMotorTorque(int MotorIndex, int Torque)            // MotorIndex =0 , 1   // golden :1847
{
    static int iRetryCount=0;

    #ifdef SOFT_SIMULTE
    {
        if(iRetryCount!=0)
        {
            iRetryCount=0;
        }
        return 1;
    }
    #else
    {
        static int iRetryTimeOutCTW=0, iRetryTimeOutCTR=0;  { extern int W906_Ht9050TorqueLimit(int, int); if(MOT[MTestZ1].CardType==AnsiString("PCI1203")) return W906_Ht9050TorqueLimit(MotorIndex, Torque); }   /*AI(W906-TORQUE-1203) 20260925: HT9050 的 Index Z 扭力上限走 1203 SDO（檔尾），其他機種照 golden 走 RS232。AI(W906-HT9050-AS-LS) 20260926：判準改成 Index Z 是 1203 卡（MOT[MTestZ1].CardType，cinitial.cpp:3923 從 Mot_Table 設；與機台 8929d13 IsIndexMotorOutOfPower 同一判法）*/                      //jou 2014-07-06 修正寫入motor driver中途被按pause會秀alarm
        TEdit *Ptr[2]={fMain->edtReadZ1, fMain->edtReadZ2};
        int &Task=iWriteAndCheckMotorTorqueTask;
        double fReadTorque;

        if(INDEX_DRIVER_TYPE==Mitsubishi_DRIVER)                                //Steven 20101111 : 三菱馬達Contact過壓不會Alarm
        {
            if(MOT[MTestZ1].Led[iAlarmLed]==true || MOT[MTestZ1].Led[iServoalarmLed]==true ||
               MOT[MTestZ2].Led[iAlarmLed]==true || MOT[MTestZ2].Led[iServoalarmLed]==true)
            {
                return 2;
            }

            if(Torque>100)
                Torque=100;
        }

        switch(Task)
        {
            case 1:
                iRetryCount=0;
                iRetryTimeOutCTW=0;
                iRetryTimeOutCTR=0;
                Task=2;
                break;
            case 2:
                fMain->edtReadZ1->Text=0;
                fMain->edtReadZ2->Text=0;

                if(MotorIndex==0)
                {
                    fMain->chkReadTorque1->Checked=true;
                    fMain->chkReadTorque2->Checked=false;
                }
                else
                {
                    fMain->chkReadTorque1->Checked=false;
                    fMain->chkReadTorque2->Checked=true;
                }

                iWriteAndCheckMotorTorqueDelay.SetMSAndOn(20);

                Task=5;
                break;
            case 5:
                if(iWriteAndCheckMotorTorqueDelay.Off())
                {
                    Task=10;
                }
                break;
            case 10:
                if(INDEX_DRIVER_TYPE==Mitsubishi_DRIVER)                        //Mitsubishi
                {
                    if(bRWActionFlag)
                        return 0;
                }

                WriteIndexTorqueSetting(MotorIndex, Torque);
                iWriteAndCheckMotorTorqueDelay.SetSecAndOn(iTorqueCommMaxTime);
                Task=100;
                break;
            case 100:
                if(INDEX_DRIVER_TYPE==Mitsubishi_DRIVER)                        //Mitsubishi
                {
                    if(bRWActionFlag)
                        return 0;
                }

                if(fPanasonicParameterRW==false)
                {
                    ReadIndexTorqueSetting(MotorIndex);
                    iWriteAndCheckMotorTorqueDelay.SetSecAndOn(iTorqueCommMaxTime);
                    iRetryTimeOutCTW=0;
                    Task=200;
                }
                else if(iWriteAndCheckMotorTorqueDelay.Off())
                {
                    Task=2;
                    iRetryTimeOutCTW++;
                    if(iRetryTimeOutCTW>1)                                      //jou 2014-07-06 修正寫入motor driver中途被按pause會秀alarm
                    {
                        ShowMyMessage("Torque Comm Timeout. Please check Torque Comm and try home or to restart!", "");
                        iRetryTimeOutCTW=0;
                    }
                }
                break;
            case 200:
                if(fPanasonicParameterRW==false)
                {
                    if(INDEX_DRIVER_TYPE==Panasonic_DRIVER)                     //Panasonic
                    {
                        if(Ptr[MotorIndex]->Text!=AnsiString(Torque))
                        {
                            iRetryCount++;
                            if(iRetryCount<5)                                   //jou 2014-07-06 3->5 修正馬達扭力異常錯誤
                            {
                                bRWActionFlag=false;
                                Task=2;                                         //jou 2014-07-06 1->2 修正馬達扭力異常錯誤
                                break;
                            }
                            return 2;
                        }
                        else
                        {
                            iRetryCount=0;
                            iRetryTimeOutCTR=0;
                            return 1;
                        }
                    }
                    else
                    {
                        fReadTorque=atof(Ptr[MotorIndex]->Text.c_str());

                        if(fReadTorque<(Torque-1) || fReadTorque>(Torque+1))
                        {
                            iRetryCount++;
                            if(iRetryCount<5)                                   //jou 2014-07-06 3->5 修正馬達扭力異常錯誤
                            {
                                bRWActionFlag=false;                            //Eliot 2010_05_03
                                Task=2;                                         //jou 2014-07-06 1->2 修正馬達扭力異常錯誤
                                break;
                            }
                            return 2;
                        }
                        else
                        {
                            iRetryCount=0;
                            iRetryTimeOutCTR=0;
                            return 1;
                        }
                    }
                }

                if(iWriteAndCheckMotorTorqueDelay.Off())
                {
                    Task=2;
                    iRetryTimeOutCTR++;
                    if(iRetryTimeOutCTR>1)                                      //jou 2014-07-06 修正寫入motor driver中途被按pause會秀alarm
                    {
                        ShowMyMessage("Torque Comm Timeout. Please check Torque Comm and try home or to restart!", "");
                        iRetryTimeOutCTR=0;
                    }
                }
                break;
        }
        return 0;
    }
    #endif
}
//---------------------------------------------------------------------------
void TCOM2Shim::ReadWriterParameter_Mitsubishi()                                // golden :2114
{
    if(bReadTorque_Mitsubishi)
    {
        bRWActionFlag=true;
        if(DoRead_Now_Torque_Action(iReadMitsubishiIndex))
        {
            if(iReadMitsubishiIndex==0)
                fMain->edtReadZ1->Text=fRead_Now_TorqueP;
            else
                fMain->edtReadZ2->Text=fRead_Now_TorqueP;

            bRWActionFlag=false;
            bReadTorque_Mitsubishi=false;
            fPanasonicParameterRW=false;
        }
    }

    if(bsetTorque_Mitsubishi)
    {
        bRWActionFlag=true;
        if(DoSet_Torque_Action(iSetMitsubishiIndex, fSetTorqueValue))
        {
            bRWActionFlag=false;
            bsetTorque_Mitsubishi=false;
            fPanasonicParameterRW=false;
        }
    }
}
//---------------------------------------------------------------------------
void TCOM2Shim::ReadIndexTorqueSetting_Mitu(int iIndex)                         // golden :2144
{
    if(bRWActionFlag)
        return;

    iReadMitsubishiIndex=iIndex;
    InitReadTorque_Mitsubishi();
    fPanasonicParameterRW=true;
    bReadTorque_Mitsubishi=true;
    bsetTorque_Mitsubishi=false;
}
//---------------------------------------------------------------------------
void TCOM2Shim::WriteIndexTorqueSetting_Mitu(int iIndex, double fTorque)       // golden :2156
{
    if(bRWActionFlag)
        return;

    if(fTorque>=100)                                                            //Steven 20140528 : Add From 7045 for Mitsubishi Motor
        fTorque=100;

    iSetMitsubishiIndex=iIndex;
    fSetTorqueValue=fTorque;
    InitSetTorque_Mitsubishi();
    fPanasonicParameterRW=true;
    bsetTorque_Mitsubishi=true;
    bReadTorque_Mitsubishi=false;
}
//---------------------------------------------------------------------------
//Mitsubishi Driver communication
//---------------------------------------------------------------------------
double TCOM2Shim::DoChangeASCII_TO_INT(char *Input)                             //把ascii碼轉成數字.   // golden :2174
{
    if(Input[0]>='0' && Input[0]<='9')
        return StrToInt(Input[0]);                                              //wei 20150316  原int(Input[0])轉換後與StrToInt(Input[0])不一樣會導致hang up
    else if(Input[0]>='a' && Input[0]<='f')
        return double(Input[0])-87;
    else if(Input[0]>='A' && Input[0]<='F')
        return double(Input[0])-55;
    else
        return 0;
}
//---------------------------------------------------------------------------
//  三菱的指令組字（golden 每一支都手寫同一段；逐字保留，只把 "%C" 換 "%c"，見檔頭 (4)）
//---------------------------------------------------------------------------
bool TCOM2Shim::ReadTorque_Mitsubishi()                                         //讀現在的扭力值.   // golden :2186
{                                                                               //1次只讀1個馬達,return ture後數值存於float fReadMotorTorque.
    int &Task=autoTask;
    AnsiString asSendData;

    if(fPanasonicParameterRW)
        return false;

    if(fMain->chkReadTorque1->Checked || fMain->chkReadTorque2->Checked)
    {
        if(fMain->chkReadTorque1->Checked)
        {
            iReadTorueIndex=0;
            SW[SwReadTorue].Off();
        }
        else
        {
            iReadTorueIndex=1;
            SW[SwReadTorue].On();
        }
    }
    else
    {
        Task=1;
        return false;
    }

    switch(Task)
    {
        case 1:                                                                 //讀出指定參數群組的指令.
            if(iReadTorueIndex<0 || iReadTorueIndex>31)
            {
                ShowMyMessage("can't read mitsubishi index");
                return false;
            }

            if(iReadTorueIndex<10)
                sprintf(cChnageIntToASCII, "%c", '0'+iReadTorueIndex);          //ascii   // golden "%C"
            else
                sprintf(cChnageIntToASCII, "%c", 'A'+iReadTorueIndex-10);       // golden "%C"

            cMotorAddress=cChnageIntToASCII[0];
            cCommand_1='0';
            cCommand_2='4';
            cDataNo_1='0';
            cDataNo_2='1';

            iCheckSum=
                int(cMotorAddress)+
                int(cCommand_1)+
                int(cCommand_2)+
                _STX+
                int(cDataNo_1)+
                int(cDataNo_2)+
                _ETX;

            sprintf(cCheckSumTemp, "%X", iCheckSum);

            sprintf(cSendMitsubishi_Data, "%c%c%c%c%c%c%c%c%c%c",
                                            _SOH,
                                            cMotorAddress,
                                            cCommand_1,
                                            cCommand_2,
                                            _STX,
                                            cDataNo_1,
                                            cDataNo_2,
                                            _ETX,
                                            cCheckSumTemp[strlen(cCheckSumTemp)-2],
                                            cCheckSumTemp[strlen(cCheckSumTemp)-1]);

            cReciveMitsubishi_Data[0]='\0';                                     // golden sprintf(cReciveMitsubishi_Data, "")
            Comm1->WriteCommData(cSendMitsubishi_Data, strlen(cSendMitsubishi_Data));
            asSendData="Send : ";
            asSendData+=cSendMitsubishi_Data;
            fMain->AddTorqueLog(asSendData);                                    //Steven 20160908 : 整合成Function
            hTimeOutLimit.SetSecAndOn(3);                                       //Steven 20140528 : Add From 7045 for Mitsubishi Motor
            Task=10;
            break;
        case 10:
            if(strlen(cReciveMitsubishi_Data)>1)                                //Steven 20140528 : Add From 7045 for Mitsubishi Motor
            {
                iCheckSum=0;
                for(int i=1; i<=7; i++)
                    iCheckSum+=int(cReciveMitsubishi_Data[i]);

                sprintf(cCheckSumTemp, "%X", iCheckSum);
                if(cReciveMitsubishi_Data[1]==cMotorAddress                 &&
                   cReciveMitsubishi_Data[2]=='A'                           &&  //'A'代表正常
                   cReciveMitsubishi_Data[strlen(cReciveMitsubishi_Data)-2]==cCheckSumTemp[strlen(cCheckSumTemp)-2] &&
                   cReciveMitsubishi_Data[strlen(cReciveMitsubishi_Data)-1]==cCheckSumTemp[strlen(cCheckSumTemp)-1])
                {
                    if(cReciveMitsubishi_Data[3]=='0'    &&                     //確認參數群組為0002,不然要重新指定.
                       cReciveMitsubishi_Data[4]=='0'    &&
                       cReciveMitsubishi_Data[5]=='0'    &&
                       cReciveMitsubishi_Data[6]=='2'    )
                    {
                        Task=30;
                    }
                    else
                    {
                        Task=20;
                    }
                }
                else                                                            //Steven 20140528 : Add From 7045 for Mitsubishi Motor
                {
                    Task=1;
                    return false;
                }
            }

            if(hTimeOutLimit.Off())
            {
                Task=1;
                return false;
            }
            break;
        case 20:                                                                //指定參數群組為0002的指令.
            cCommand_1='8';
            cCommand_2='5';
            cDataNo_1='0';
            cDataNo_2='0';
            cNoData[0]='0';
            cNoData[1]='0';
            cNoData[2]='0';
            cNoData[3]='2';

            iCheckSum=
                int(cMotorAddress)+
                int(cCommand_1)+
                int(cCommand_2)+
                _STX+
                int(cDataNo_1)+
                int(cDataNo_2)+
                int(cNoData[0])+
                int(cNoData[1])+
                int(cNoData[2])+
                int(cNoData[3])+
                _ETX;

            sprintf(cCheckSumTemp, "%X", iCheckSum);

            sprintf(cSendMitsubishi_Data, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
                                            _SOH,
                                            cMotorAddress,
                                            cCommand_1,
                                            cCommand_2,
                                            _STX,
                                            cDataNo_1,
                                            cDataNo_2,
                                            cNoData[0],
                                            cNoData[1],
                                            cNoData[2],
                                            cNoData[3],
                                            _ETX,
                                            cCheckSumTemp[strlen(cCheckSumTemp)-2],
                                            cCheckSumTemp[strlen(cCheckSumTemp)-1]);

            Comm1->WriteCommData(cSendMitsubishi_Data, strlen(cSendMitsubishi_Data));
            asSendData="Send : ";
            asSendData+=cSendMitsubishi_Data;
            fMain->AddTorqueLog(asSendData);                                    //Steven 20160908 : 整合成Function
            Task=1;
            break;
        case 30:                                                                //讀取狀態.
            cCommand_1='0';
            cCommand_2='1';

            cDataNo_1='8';
            cDataNo_2='A';

            iCheckSum=
                int(cMotorAddress)+
                int(cCommand_1)+
                int(cCommand_2)+
                _STX+
                int(cDataNo_1)+
                int(cDataNo_2)+
                _ETX;

            sprintf(cCheckSumTemp, "%X", iCheckSum);

            sprintf(cSendMitsubishi_Data, "%c%c%c%c%c%c%c%c%c%c",
                                            _SOH,
                                            cMotorAddress,
                                            cCommand_1,
                                            cCommand_2,
                                            _STX,
                                            cDataNo_1,
                                            cDataNo_2,
                                            _ETX,
                                            cCheckSumTemp[strlen(cCheckSumTemp)-2],
                                            cCheckSumTemp[strlen(cCheckSumTemp)-1]);

            for(int i=0; i<32; i++)
                cReciveMitsubishi_Data[i]=0x00;
            Comm1->WriteCommData(cSendMitsubishi_Data, strlen(cSendMitsubishi_Data));
            asSendData="Send : ";
            asSendData+=cSendMitsubishi_Data;
            fMain->AddTorqueLog(asSendData);                                    //Steven 20160908 : 整合成Function
            hTimeOutLimit.SetSecAndOn(3);                                       //Steven 20140528 : Add From 7045 for Mitsubishi Motor
            Task=40;
            break;
        case 40:                                                                //回收的資料= STX+馬達編號+異常碥+資料+ETX+檢查合.
            if(strlen(cReciveMitsubishi_Data)>1)                                //Steven 20140528 : Add From 7045 for Mitsubishi Motor
            {
                iCheckSum=0;
                for(int i=1; i<(int)(strlen(cReciveMitsubishi_Data)-2); i++)
                {
                    iCheckSum+=int(cReciveMitsubishi_Data[i]);
                }

                sprintf(cCheckSumTemp,"%X",iCheckSum);
                if(cReciveMitsubishi_Data[1]==cMotorAddress                 &&
                   cReciveMitsubishi_Data[2]=='A'                           &&  //'A'代表正常
                   cReciveMitsubishi_Data[strlen(cReciveMitsubishi_Data)-2]==cCheckSumTemp[strlen(cCheckSumTemp)-2] &&
                   cReciveMitsubishi_Data[strlen(cReciveMitsubishi_Data)-1]==cCheckSumTemp[strlen(cCheckSumTemp)-1])
                {
                    fReadMotorTorque=DoChangeASCII_TO_INT(&cReciveMitsubishi_Data[10])*(16<<12);//65536
                    fReadMotorTorque+=DoChangeASCII_TO_INT(&cReciveMitsubishi_Data[11])*(16<<8);//4096
                    fReadMotorTorque+=DoChangeASCII_TO_INT(&cReciveMitsubishi_Data[12])*(16<<4);//256
                    fReadMotorTorque+=DoChangeASCII_TO_INT(&cReciveMitsubishi_Data[13])*16;
                    fReadMotorTorque+=DoChangeASCII_TO_INT(&cReciveMitsubishi_Data[14]);

                    if(cReciveMitsubishi_Data[7]=='F')
                    {
                        fReadMotorTorque=int(fReadMotorTorque)-1048576;
                    }

                    for(int i=1; i<StrToInt(cReciveMitsubishi_Data[4]); i++)    //數字2=除10,3=除100.   //jou 2014-10-22 StrToInt -> atoi   //wei 20150316 改回StrToInt
                        fReadMotorTorque/=10;

                    tTimeTemp=Now();
                    sprintf(cSendMitsubishi_Data,"%s < %s =%2.1f",
                        tTimeTemp.FormatString("hh:nn:ss").c_str(),
                        cReciveMitsubishi_Data,
                        fReadMotorTorque);
                    asSendData="Data : ";
                    asSendData+=cSendMitsubishi_Data;
                    fMain->AddTorqueLog(asSendData);                            //Steven 20160908 : 整合成Function

                    if(fReadMotorTorque>100)
                        fReadMotorTorque=-1;

                    if(iReadTorueIndex==0)
                    {
                        fMain->edTorue0->Text=fReadMotorTorque;
                        fContactForm->PnlTorue0->Caption=fReadMotorTorque;      // golden fContact->PnlTorue0（檔頭 (6)）
                    }
                    else
                    {
                        fMain->edTorue1->Text=fReadMotorTorque;
                        fContactForm->PnlTorue1->Caption=fReadMotorTorque;      // golden fContact->PnlTorue1（檔頭 (6)）
                    }
                    Task=1;
                    return true;
                }
                else                                                            //Steven 20140528 : Add From 7045 for Mitsubishi Motor
                {
                    Task=1;
                    return false;
                }
            }

            if(hTimeOutLimit.Off())
            {
                Task=1;
                return false;
            }
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
static int iMtsubishi_Count=0;                                                  //Steven 20140627 : Fixed Mitsubishi Torque Hang Up   // golden :2459
void TCOM2Shim::InitSetTorque_Mitsubishi()                                      // golden :2460
{
    iWriteTorqueTask=1;
    iMtsubishi_Count=0;
}
//---------------------------------------------------------------------------
bool TCOM2Shim::DoSet_Torque_Action(int iIndex, double fTorque)                //寫入需要的扭力值.   // golden :2466
{
    int &Task=iWriteTorqueTask;
    AnsiString asSendData;
    switch(Task)
    {
        case 1:                                                                 //讀出指定參數群組的指令.
            if(iIndex<0 || iIndex>31)
            {
                ShowMyMessage("can't read mitsubishi index");
                return false;
            }

            if(iIndex<10)
                sprintf(cChnageIntToASCII, "%c", '0'+iIndex);                   //ascii   // golden "%C"
            else
                sprintf(cChnageIntToASCII, "%c", 'A'+iIndex-10);                // golden "%C"

            cMotorAddress=cChnageIntToASCII[0];
            cCommand_1='0';
            cCommand_2='4';
            cDataNo_1='0';
            cDataNo_2='1';

            iCheckSum=
                int(cMotorAddress)+
                int(cCommand_1)+
                int(cCommand_2)+
                _STX+
                int(cDataNo_1)+
                int(cDataNo_2)+
                _ETX;

            sprintf(cCheckSumTemp, "%X", iCheckSum);

            sprintf(cSendMitsubishi_Data, "%c%c%c%c%c%c%c%c%c%c",
                                            _SOH,
                                            cMotorAddress,
                                            cCommand_1,
                                            cCommand_2,
                                            _STX,
                                            cDataNo_1,
                                            cDataNo_2,
                                            _ETX,
                                            cCheckSumTemp[strlen(cCheckSumTemp)-2],
                                            cCheckSumTemp[strlen(cCheckSumTemp)-1]);

            cReciveMitsubishi_Data[0]='\0';                                     // golden sprintf(cReciveMitsubishi_Data, "")
            Comm1->WriteCommData(cSendMitsubishi_Data, strlen(cSendMitsubishi_Data));
            asSendData="Send : ";
            asSendData+=cSendMitsubishi_Data;
            fMain->AddTorqueLog(asSendData);                                    //Steven 20160908 : 整合成Function
            hTimeOutLimit.Set0_1SecAndOn(20);                                   // 2010/5/4 lee
            fWriteTorque=int(fTorque*10.0);                                     //because this motor torque 0~100.0,must change new Torque lile 0~300.
            Task=10;
            break;
        case 10:
            if(strlen(cReciveMitsubishi_Data)>1)                                //Steven 20140528 : Add From 7045 for Mitsubishi Motor
            {
                iCheckSum=0;
                for(int i=1; i<=7; i++)
                    iCheckSum+=int(cReciveMitsubishi_Data[i]);

                sprintf(cCheckSumTemp, "%X", iCheckSum);
                if(cReciveMitsubishi_Data[1]==cMotorAddress                 &&
                   cReciveMitsubishi_Data[2]=='A'                           &&  //'A'代表正常
                   cReciveMitsubishi_Data[strlen(cReciveMitsubishi_Data)-2]==cCheckSumTemp[strlen(cCheckSumTemp)-2] &&
                   cReciveMitsubishi_Data[strlen(cReciveMitsubishi_Data)-1]==cCheckSumTemp[strlen(cCheckSumTemp)-1])
                {
                    if(cReciveMitsubishi_Data[3]=='0'    &&                     //確認參數群組為0000,不然要重新指定.
                       cReciveMitsubishi_Data[4]=='0'    &&
                       cReciveMitsubishi_Data[5]=='0'    &&
                       cReciveMitsubishi_Data[6]=='0'    )
                    {
                        Task=30;
                    }
                    else
                    {
                        Task=20;
                    }
                }
                else                                                            //Steven 20140528 : Add From 7045 for Mitsubishi Motor
                {
                    Task=1;
                    return false;
                }
            }

            if(hTimeOutLimit.Off())
            {
                Task=1;
                iMtsubishi_Count++;

                if(iMtsubishi_Count>10)                                         //Steven 20140627 : Fixed Mitsubishi Torque Hang Up
                {
                    Comm1->StopComm();
                    Comm1->StartComm();
                }
                return false;
            }
            break;
        case 20:                                                                //指定參數群組為0000的指令.
            cCommand_1='8';
            cCommand_2='5';
            cDataNo_1='0';
            cDataNo_2='0';
            cNoData[0]='0';
            cNoData[1]='0';
            cNoData[2]='0';
            cNoData[3]='0';

            iCheckSum=
                int(cMotorAddress)+
                int(cCommand_1)+
                int(cCommand_2)+
                _STX+
                int(cDataNo_1)+
                int(cDataNo_2)+
                int(cNoData[0])+
                int(cNoData[1])+
                int(cNoData[2])+
                int(cNoData[3])+
                _ETX;

            sprintf(cCheckSumTemp, "%X", iCheckSum);

            sprintf(cSendMitsubishi_Data, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
                                            _SOH,
                                            cMotorAddress,
                                            cCommand_1,
                                            cCommand_2,
                                            _STX,
                                            cDataNo_1,
                                            cDataNo_2,
                                            cNoData[0],
                                            cNoData[1],
                                            cNoData[2],
                                            cNoData[3],
                                            _ETX,
                                            cCheckSumTemp[strlen(cCheckSumTemp)-2],
                                            cCheckSumTemp[strlen(cCheckSumTemp)-1]);

            Comm1->WriteCommData(cSendMitsubishi_Data, strlen(cSendMitsubishi_Data));
            asSendData="Send : ";
            asSendData+=cSendMitsubishi_Data;
            fMain->AddTorqueLog(asSendData);                                    //Steven 20160908 : 整合成Function
            Task=1;
            break;
        case 30:                                                                //read torque P
            cCommand_1='0';
            cCommand_2='5';

            cDataNo_1='0';
            cDataNo_2='B';

            iCheckSum=
                int(cMotorAddress)+
                int(cCommand_1)+
                int(cCommand_2)+
                _STX+
                int(cDataNo_1)+
                int(cDataNo_2)+
                _ETX;

            sprintf(cCheckSumTemp, "%X", iCheckSum);

            sprintf(cSendMitsubishi_Data, "%c%c%c%c%c%c%c%c%c%c",
                                            _SOH,
                                            cMotorAddress,
                                            cCommand_1,
                                            cCommand_2,
                                            _STX,
                                            cDataNo_1,
                                            cDataNo_2,
                                            _ETX,
                                            cCheckSumTemp[strlen(cCheckSumTemp)-2],
                                            cCheckSumTemp[strlen(cCheckSumTemp)-1]);

            for(int i=0; i<32; i++)
                cReciveMitsubishi_Data[i]=0x00;
            Comm1->WriteCommData(cSendMitsubishi_Data, strlen(cSendMitsubishi_Data));
            asSendData="Send : ";
            asSendData+=cSendMitsubishi_Data;
            fMain->AddTorqueLog(asSendData);                                    //Steven 20160908 : 整合成Function
            hTimeOutLimit.Set0_1SecAndOn(20);
            Task=40;
            break;
        case 40:                                                                //回收的資料= STX+馬達編號+異常碥+資料+ETX+檢查合.
            if(strlen(cReciveMitsubishi_Data)>1)                                //Steven 20140528 : Add From 7045 for Mitsubishi Motor
            {
                iCheckSum=0;
                for(int i=1; i<(int)(strlen(cReciveMitsubishi_Data)-2); i++)
                {
                    iCheckSum+=int(cReciveMitsubishi_Data[i]);
                }

                sprintf(cCheckSumTemp, "%X", iCheckSum);
                if(cReciveMitsubishi_Data[1]==cMotorAddress                 &&
                   cReciveMitsubishi_Data[2]=='A'                           &&  //'A'代表正常
                   cReciveMitsubishi_Data[strlen(cReciveMitsubishi_Data)-2]==cCheckSumTemp[strlen(cCheckSumTemp)-2] &&
                   cReciveMitsubishi_Data[strlen(cReciveMitsubishi_Data)-1]==cCheckSumTemp[strlen(cCheckSumTemp)-1])
                {
                    if(cReciveMitsubishi_Data[3]=='1')
                    {
                        fReadMotorTorque=DoChangeASCII_TO_INT(&cReciveMitsubishi_Data[6])*(16<<12);//65536
                        fReadMotorTorque+=DoChangeASCII_TO_INT(&cReciveMitsubishi_Data[7])*(16<<8);//4096
                        fReadMotorTorque+=DoChangeASCII_TO_INT(&cReciveMitsubishi_Data[8])*(16<<4);//256
                        fReadMotorTorque+=DoChangeASCII_TO_INT(&cReciveMitsubishi_Data[9])*16;
                        fReadMotorTorque+=DoChangeASCII_TO_INT(&cReciveMitsubishi_Data[10]);
                    }

                    tTimeTemp=Now();
                    sprintf(cSendMitsubishi_Data, "%s < %s =%2.1f",
                        tTimeTemp.FormatString("hh:nn:ss").c_str(),
                        cReciveMitsubishi_Data,
                        fReadMotorTorque);
                    asSendData="Data : ";
                    asSendData+=cSendMitsubishi_Data;
                    fMain->AddTorqueLog(asSendData);                            //Steven 20160908 : 整合成Function

                    if(fReadMotorTorque==fWriteTorque)
                    {
                        Task=60;
                    }
                    else
                    {
                        Task=50;
                    }
                }
                else                                                            //Steven 20140528 : Add From 7045 for Mitsubishi Motor
                {
                    Task=1;
                    return false;
                }
            }

            if(hTimeOutLimit.Off())
            {
                Task=1;
            }
            break;
        case 50:                                                                //write torque P
            cCommand_1='8';
            cCommand_2='4';
            cDataNo_1='0';
            cDataNo_2='B';
            cWriteDataTitle[0]='3';
            cWriteDataTitle[1]='2';

            sprintf(cNoData, "%06X", int(fWriteTorque));

            iCheckSum=
                int(cMotorAddress)+
                int(cCommand_1)+
                int(cCommand_2)+
                _STX+
                int(cDataNo_1)+
                int(cDataNo_2)+
                int(cWriteDataTitle[0])+
                int(cWriteDataTitle[1])+
                int(cNoData[0])+
                int(cNoData[1])+
                int(cNoData[2])+
                int(cNoData[3])+
                int(cNoData[4])+
                int(cNoData[5])+
                _ETX;

            sprintf(cCheckSumTemp, "%X", iCheckSum);

            sprintf(cSendMitsubishi_Data, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
                                            _SOH,
                                            cMotorAddress,
                                            cCommand_1,
                                            cCommand_2,
                                            _STX,
                                            cDataNo_1,
                                            cDataNo_2,
                                            cWriteDataTitle[0],
                                            cWriteDataTitle[1],
                                            cNoData[0],
                                            cNoData[1],
                                            cNoData[2],
                                            cNoData[3],
                                            cNoData[4],
                                            cNoData[5],
                                            _ETX,
                                            cCheckSumTemp[strlen(cCheckSumTemp)-2],
                                            cCheckSumTemp[strlen(cCheckSumTemp)-1]);

            Comm1->WriteCommData(cSendMitsubishi_Data, strlen(cSendMitsubishi_Data));
            asSendData="Send : ";
            asSendData+=cSendMitsubishi_Data;
            fMain->AddTorqueLog(asSendData);                                    //Steven 20160908 : 整合成Function
            Task=30;
            break;
        case 60:                                                                //read torque N
            cCommand_1='0';
            cCommand_2='5';

            cDataNo_1='0';
            cDataNo_2='C';

            iCheckSum=
                int(cMotorAddress)+
                int(cCommand_1)+
                int(cCommand_2)+
                _STX+
                int(cDataNo_1)+
                int(cDataNo_2)+
                _ETX;

            sprintf(cCheckSumTemp, "%X", iCheckSum);

            sprintf(cSendMitsubishi_Data, "%c%c%c%c%c%c%c%c%c%c",
                                            _SOH,
                                            cMotorAddress,
                                            cCommand_1,
                                            cCommand_2,
                                            _STX,
                                            cDataNo_1,
                                            cDataNo_2,
                                            _ETX,
                                            cCheckSumTemp[strlen(cCheckSumTemp)-2],
                                            cCheckSumTemp[strlen(cCheckSumTemp)-1]);

            for(int i=0; i<32; i++)
                cReciveMitsubishi_Data[i]=0x00;
            Comm1->WriteCommData(cSendMitsubishi_Data, strlen(cSendMitsubishi_Data));
            asSendData="Send : ";
            asSendData+=cSendMitsubishi_Data;
            fMain->AddTorqueLog(asSendData);                                    //Steven 20160908 : 整合成Function
            hTimeOutLimit.Set0_1SecAndOn(20);
            Task=70;
            break;
        case 70:                                                                //回收的資料= STX+馬達編號+異常碥+資料+ETX+檢查合.
            if(strlen(cReciveMitsubishi_Data)>0)                                //Steven 20140528 : Add From 7045 for Mitsubishi Motor
            {
                iCheckSum=0;
                for(int i=1; i<(int)(strlen(cReciveMitsubishi_Data)-2); i++)
                {
                    iCheckSum+=int(cReciveMitsubishi_Data[i]);
                }

                sprintf(cCheckSumTemp, "%X", iCheckSum);
                if(cReciveMitsubishi_Data[1]==cMotorAddress                 &&
                   cReciveMitsubishi_Data[2]=='A'                           &&  //'A'代表正常
                   cReciveMitsubishi_Data[strlen(cReciveMitsubishi_Data)-2]==cCheckSumTemp[strlen(cCheckSumTemp)-2] &&
                   cReciveMitsubishi_Data[strlen(cReciveMitsubishi_Data)-1]==cCheckSumTemp[strlen(cCheckSumTemp)-1])
                {
                    if(cReciveMitsubishi_Data[3]=='1')
                    {
                        fReadMotorTorque=DoChangeASCII_TO_INT(&cReciveMitsubishi_Data[6])*(16<<12);//65536
                        fReadMotorTorque+=DoChangeASCII_TO_INT(&cReciveMitsubishi_Data[7])*(16<<8);//4096
                        fReadMotorTorque+=DoChangeASCII_TO_INT(&cReciveMitsubishi_Data[8])*(16<<4);//256
                        fReadMotorTorque+=DoChangeASCII_TO_INT(&cReciveMitsubishi_Data[9])*16;
                        fReadMotorTorque+=DoChangeASCII_TO_INT(&cReciveMitsubishi_Data[10]);
                    }

                    tTimeTemp=Now();
                    sprintf(cSendMitsubishi_Data, "%s < %s =%2.1f",
                                                    tTimeTemp.FormatString("hh:nn:ss").c_str(),
                                                    cReciveMitsubishi_Data,
                                                    fReadMotorTorque);
                    asSendData="Data : ";
                    asSendData+=cSendMitsubishi_Data;
                    fMain->AddTorqueLog(asSendData);                            //Steven 20160908 : 整合成Function

                    if(fReadMotorTorque==fWriteTorque)
                    {
                        Task=1;
                        return true;
                    }
                    else
                    {
                        Task=80;
                    }
                }
                else                                                            //Steven 20140528 : Add From 7045 for Mitsubishi Motor
                {
                    Task=1;
                    return false;
                }
            }

            if(hTimeOutLimit.Off())
            {
                Task=1;
            }
            break;
        case 80:                                                                //write torque P
            cCommand_1='8';
            cCommand_2='4';
            cDataNo_1='0';
            cDataNo_2='C';
            cWriteDataTitle[0]='3';
            cWriteDataTitle[1]='2';

            sprintf(cNoData, "%06X", int(fWriteTorque));

            iCheckSum=
                int(cMotorAddress)+
                int(cCommand_1)+
                int(cCommand_2)+
                _STX+
                int(cDataNo_1)+
                int(cDataNo_2)+
                int(cWriteDataTitle[0])+
                int(cWriteDataTitle[1])+
                int(cNoData[0])+
                int(cNoData[1])+
                int(cNoData[2])+
                int(cNoData[3])+
                int(cNoData[4])+
                int(cNoData[5])+
                _ETX;

            sprintf(cCheckSumTemp, "%X", iCheckSum);

            sprintf(cSendMitsubishi_Data, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
                                            _SOH,
                                            cMotorAddress,
                                            cCommand_1,
                                            cCommand_2,
                                            _STX,
                                            cDataNo_1,
                                            cDataNo_2,
                                            cWriteDataTitle[0],
                                            cWriteDataTitle[1],
                                            cNoData[0],
                                            cNoData[1],
                                            cNoData[2],
                                            cNoData[3],
                                            cNoData[4],
                                            cNoData[5],
                                            _ETX,
                                            cCheckSumTemp[strlen(cCheckSumTemp)-2],
                                            cCheckSumTemp[strlen(cCheckSumTemp)-1]);

            Comm1->WriteCommData(cSendMitsubishi_Data, strlen(cSendMitsubishi_Data));
            asSendData="Send : ";
            asSendData+=cSendMitsubishi_Data;
            fMain->AddTorqueLog(asSendData);                                    //Steven 20160908 : 整合成Function
            Task=60;
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
void TCOM2Shim::InitReadTorque_Mitsubishi()                                     // golden :2915
{
    iRead_Now_Torque_Task=1;
}
//---------------------------------------------------------------------------
bool TCOM2Shim::DoRead_Now_Torque_Action(int iIndex)                            //讀取Driver設定的扭力值.   // golden :2920
{
    int &Task=iRead_Now_Torque_Task;
    AnsiString asSendData;
    switch(Task)
    {
        case 1:                                                                 //讀出指定參數群組的指令.
            if(iIndex<0 || iIndex>31)
            {
                ShowMyMessage("can't read mitsubishi index");
                return false;
            }

            if(iIndex<10)
                sprintf(cChnageIntToASCII, "%c", '0'+iIndex);                   //ascii   // golden "%C"
            else
                sprintf(cChnageIntToASCII, "%c", 'A'+iIndex-10);                // golden "%C"

            cMotorAddress=cChnageIntToASCII[0];
            cCommand_1='0';
            cCommand_2='4';
            cDataNo_1='0';
            cDataNo_2='1';

            iCheckSum=
                int(cMotorAddress)+
                int(cCommand_1)+
                int(cCommand_2)+
                _STX+
                int(cDataNo_1)+
                int(cDataNo_2)+
                _ETX;

            sprintf(cCheckSumTemp, "%X", iCheckSum);

            sprintf(cSendMitsubishi_Data, "%c%c%c%c%c%c%c%c%c%c",
                _SOH,
                cMotorAddress,
                cCommand_1,
                cCommand_2,
                _STX,
                cDataNo_1,
                cDataNo_2,
                _ETX,
                cCheckSumTemp[strlen(cCheckSumTemp)-2],
                cCheckSumTemp[strlen(cCheckSumTemp)-1]);

            cReciveMitsubishi_Data[0]='\0';                                     // golden sprintf(cReciveMitsubishi_Data,"")
            Comm1->WriteCommData(cSendMitsubishi_Data,strlen(cSendMitsubishi_Data));
            asSendData="Send : ";
            asSendData+=cSendMitsubishi_Data;
            fMain->AddTorqueLog(asSendData);                                    //Steven 20160908 : 整合成Function
            hTimeOutLimit.SetSecAndOn(3);                                       //Steven 20140528 : Add From 7045 for Mitsubishi Motor
            Task=10;
            break;
        case 10:
            if(strlen(cReciveMitsubishi_Data)>1)                                //Steven 20140528 : Add From 7045 for Mitsubishi Motor
            {
                iCheckSum=0;
                for(int i=1; i<=7; i++)
                    iCheckSum+=int(cReciveMitsubishi_Data[i]);

                sprintf(cCheckSumTemp,"%X",iCheckSum);
                if(cReciveMitsubishi_Data[1]==cMotorAddress                 &&
                   cReciveMitsubishi_Data[2]=='A'                           &&  //'A'代表正常
                   cReciveMitsubishi_Data[strlen(cReciveMitsubishi_Data)-2]==cCheckSumTemp[strlen(cCheckSumTemp)-2] &&
                   cReciveMitsubishi_Data[strlen(cReciveMitsubishi_Data)-1]==cCheckSumTemp[strlen(cCheckSumTemp)-1])
                {
                    //確認參數群組為0000,不然要重新指定.
                    if(cReciveMitsubishi_Data[3]=='0'    &&
                       cReciveMitsubishi_Data[4]=='0'    &&
                       cReciveMitsubishi_Data[5]=='0'    &&
                       cReciveMitsubishi_Data[6]=='0'    )
                    {
                        Task=30;
                    }
                    else
                    {
                        Task=20;
                    }
                }
            }

            if(hTimeOutLimit.Off())
            {
                Task=1;
                return false;
            }
            break;
        case 20:                                                                //指定參數群組為0000的指令.
            cCommand_1='8';
            cCommand_2='5';
            cDataNo_1='0';
            cDataNo_2='0';
            cNoData[0]='0';
            cNoData[1]='0';
            cNoData[2]='0';
            cNoData[3]='0';

            iCheckSum=
                int(cMotorAddress)+
                int(cCommand_1)+
                int(cCommand_2)+
                _STX+
                int(cDataNo_1)+
                int(cDataNo_2)+
                int(cNoData[0])+
                int(cNoData[1])+
                int(cNoData[2])+
                int(cNoData[3])+
                _ETX;

            sprintf(cCheckSumTemp, "%X", iCheckSum);

            sprintf(cSendMitsubishi_Data, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
                                            _SOH,
                                            cMotorAddress,
                                            cCommand_1,
                                            cCommand_2,
                                            _STX,
                                            cDataNo_1,
                                            cDataNo_2,
                                            cNoData[0],
                                            cNoData[1],
                                            cNoData[2],
                                            cNoData[3],
                                            _ETX,
                                            cCheckSumTemp[strlen(cCheckSumTemp)-2],
                                            cCheckSumTemp[strlen(cCheckSumTemp)-1]);

            Comm1->WriteCommData(cSendMitsubishi_Data, strlen(cSendMitsubishi_Data));
            asSendData="Send : ";
            asSendData+=cSendMitsubishi_Data;
            fMain->AddTorqueLog(asSendData);                                    //Steven 20160908 : 整合成Function
            Task=1;
            break;
        case 30:                                                                //read torque P
            cCommand_1='0';
            cCommand_2='5';

            cDataNo_1='0';
            cDataNo_2='B';

            iCheckSum=
                int(cMotorAddress)+
                int(cCommand_1)+
                int(cCommand_2)+
                _STX+
                int(cDataNo_1)+
                int(cDataNo_2)+
                _ETX;

            sprintf(cCheckSumTemp, "%X", iCheckSum);

            sprintf(cSendMitsubishi_Data, "%c%c%c%c%c%c%c%c%c%c",
                                            _SOH,
                                            cMotorAddress,
                                            cCommand_1,
                                            cCommand_2,
                                            _STX,
                                            cDataNo_1,
                                            cDataNo_2,
                                            _ETX,
                                            cCheckSumTemp[strlen(cCheckSumTemp)-2],
                                            cCheckSumTemp[strlen(cCheckSumTemp)-1]);

            for(int i=0; i<32; i++)
                cReciveMitsubishi_Data[i]=0x00;
            Comm1->WriteCommData(cSendMitsubishi_Data, strlen(cSendMitsubishi_Data));
            asSendData="Send : ";
            asSendData+=cSendMitsubishi_Data;
            fMain->AddTorqueLog(asSendData);                                    //Steven 20160908 : 整合成Function
            hTimeOutLimit.SetSecAndOn(3);                                       //Steven 20140528 : Add From 7045 for Mitsubishi Motor
            Task=40;
            break;
        case 40:                                                                //回收的資料= STX+馬達編號+異常碥+資料+ETX+檢查合.
            if(strlen(cReciveMitsubishi_Data)>1)                                //Steven 20140528 : Add From 7045 for Mitsubishi Motor
            {
                iCheckSum=0;
                if(strlen(cReciveMitsubishi_Data)>1)
                {
                    for(int i=1; i<(int)(strlen(cReciveMitsubishi_Data)-2); i++)
                    {
                        if(cReciveMitsubishi_Data[i]!='\0')
                            iCheckSum+=int(cReciveMitsubishi_Data[i]);
                    }
                }

                sprintf(cCheckSumTemp, "%X", iCheckSum);
                if(cReciveMitsubishi_Data[1]==cMotorAddress                 &&
                   cReciveMitsubishi_Data[2]=='A'                           &&  //'A'代表正常
                   cReciveMitsubishi_Data[strlen(cReciveMitsubishi_Data)-2]==cCheckSumTemp[strlen(cCheckSumTemp)-2] &&
                   cReciveMitsubishi_Data[strlen(cReciveMitsubishi_Data)-1]==cCheckSumTemp[strlen(cCheckSumTemp)-1])
                {
                    if(cReciveMitsubishi_Data[3]=='1')
                    {
                        fReadMotorTorque=DoChangeASCII_TO_INT(&cReciveMitsubishi_Data[6])*(16<<12);//65536
                        fReadMotorTorque+=DoChangeASCII_TO_INT(&cReciveMitsubishi_Data[7])*(16<<8);//4096
                        fReadMotorTorque+=DoChangeASCII_TO_INT(&cReciveMitsubishi_Data[8])*(16<<4);//256
                        fReadMotorTorque+=DoChangeASCII_TO_INT(&cReciveMitsubishi_Data[9])*16;
                        fReadMotorTorque+=DoChangeASCII_TO_INT(&cReciveMitsubishi_Data[10]);
                    }

                    for(int i=1; i<StrToInt(cReciveMitsubishi_Data[4]); i++)    //數字2=除10,3=除100. //jou 2014-10-22 StrToInt -> atoi   //wei 20150316 改回StrToInt
                        fReadMotorTorque/=10.0;

                    fRead_Now_TorqueP=fReadMotorTorque;

                    tTimeTemp=Now();
                    sprintf(cSendMitsubishi_Data,"%s < %s =%2.1f",
                        tTimeTemp.FormatString("hh:nn:ss").c_str(),
                        cReciveMitsubishi_Data,
                        fReadMotorTorque);
                    asSendData="Data : ";
                    asSendData+=cSendMitsubishi_Data;
                    fMain->AddTorqueLog(asSendData);                            //Steven 20160908 : 整合成Function

                    Task=50;
                }
            }

            if(hTimeOutLimit.Off())
            {
                Task=1;
            }
            break;
        case 50:                                                                //read torque N
            cCommand_1='0';
            cCommand_2='5';

            cDataNo_1='0';
            cDataNo_2='C';

            iCheckSum=
                int(cMotorAddress)+
                int(cCommand_1)+
                int(cCommand_2)+
                _STX+
                int(cDataNo_1)+
                int(cDataNo_2)+
                _ETX;

            sprintf(cCheckSumTemp, "%X", iCheckSum);

            sprintf(cSendMitsubishi_Data, "%c%c%c%c%c%c%c%c%c%c",
                                            _SOH,
                                            cMotorAddress,
                                            cCommand_1,
                                            cCommand_2,
                                            _STX,
                                            cDataNo_1,
                                            cDataNo_2,
                                            _ETX,
                                            cCheckSumTemp[strlen(cCheckSumTemp)-2],
                                            cCheckSumTemp[strlen(cCheckSumTemp)-1]);

            for(int i=0; i<32; i++)
                cReciveMitsubishi_Data[i]=0x00;
            Comm1->WriteCommData(cSendMitsubishi_Data,strlen(cSendMitsubishi_Data));
            asSendData="Send : ";
            asSendData+=cSendMitsubishi_Data;
            fMain->AddTorqueLog(asSendData);                                    //Steven 20160908 : 整合成Function
            hTimeOutLimit.SetSecAndOn(3);                                       //Steven 20140528 : Add From 7045 for Mitsubishi Motor
            Task=60;
            break;
        case 60:                                                                //回收的資料= STX+馬達編號+異常碥+資料+ETX+檢查合.
            if(strlen(cReciveMitsubishi_Data)>1)                                //Steven 20140528 : Add From 7045 for Mitsubishi Motor
            {
                iCheckSum=0;
                for(int i=1; i<(int)(strlen(cReciveMitsubishi_Data)-2); i++)
                {
                    iCheckSum+=int(cReciveMitsubishi_Data[i]);
                }

                sprintf(cCheckSumTemp,"%X",iCheckSum);
                if(cReciveMitsubishi_Data[1]==cMotorAddress                 &&
                   cReciveMitsubishi_Data[2]=='A'                           &&  //'A'代表正常
                   cReciveMitsubishi_Data[strlen(cReciveMitsubishi_Data)-2]==cCheckSumTemp[strlen(cCheckSumTemp)-2] &&
                   cReciveMitsubishi_Data[strlen(cReciveMitsubishi_Data)-1]==cCheckSumTemp[strlen(cCheckSumTemp)-1])
                {
                    if(cReciveMitsubishi_Data[3]=='1')
                    {
                        fReadMotorTorque=DoChangeASCII_TO_INT(&cReciveMitsubishi_Data[6])*(16<<12);//65536
                        fReadMotorTorque+=DoChangeASCII_TO_INT(&cReciveMitsubishi_Data[7])*(16<<8);//4096
                        fReadMotorTorque+=DoChangeASCII_TO_INT(&cReciveMitsubishi_Data[8])*(16<<4);//256
                        fReadMotorTorque+=DoChangeASCII_TO_INT(&cReciveMitsubishi_Data[9])*16;
                        fReadMotorTorque+=DoChangeASCII_TO_INT(&cReciveMitsubishi_Data[10]);
                    }

                    for(int i=1; i<StrToInt(cReciveMitsubishi_Data[4]);i++)     //數字2=除10,3=除100. //jou 2014-10-22 StrToInt -> atoi   //wei 20150316 改回StrToInt
                        fReadMotorTorque/=10.0;

                    fRead_Now_TorqueN=fReadMotorTorque;

                    tTimeTemp=Now();
                    sprintf(cSendMitsubishi_Data, "%s < %s =%2.1f",
                                                    tTimeTemp.FormatString("hh:nn:ss").c_str(),
                                                    cReciveMitsubishi_Data,
                                                    fReadMotorTorque);
                    asSendData="Data : ";
                    asSendData+=cSendMitsubishi_Data;
                    fMain->AddTorqueLog(asSendData);                            //Steven 20160908 : 整合成Function
                    Task=1;
                    return true;
                }
            }

            if(hTimeOutLimit.Off())
            {
                Task=1;
            }
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
int TCOM2Shim::GetReadTorueTask()                                               //jou 2011-11-29防止Read Torue後數值被清掉，還傻傻的在那邊等   // golden :3763
{
    return autoTask;
}
//---------------------------------------------------------------------------
//  golden TfMain::AddTorqueLog（main.cpp:32586-32609）。宣告在 forms/fMain.h（非 virtual），本體放在這裡（ht9045_sm）：
//  它要 GetTimeInfo／SystemYear…（cpublic／cmydef），ht9045_forms 那一層拿不到。
//  HP 通訊卡的兩段（slTorquMUClog：golden TfMain 建構子 new 的 TMyStringList 檔案 log，移植樹沒有）閘著，見檔頭 (3)。
//---------------------------------------------------------------------------
void TfMain::AddTorqueLog(AnsiString Msg)                                       //Steven 20160908 : 整合成Function
{
    AnsiString sFileName="", sMegTime="", asLog="";

    GetTimeInfo();
    sMegTime.sprintf("%04d-%02d-%02d %02d:%02d:%02d %03d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec);
    asLog.sprintf("%s : %s", sMegTime,Msg);
    AnsiString S1="";

    if(ListBox14->Items->Count>300)
    {
        // golden :32597-32598 if(TorqueUseHPComCard) slTorquMUClog->MySaveToFile(); —— HP 通訊卡，閘著
        ListBox14->Clear();
    }

    ListBox14->Items->Add(asLog);
    // golden :32604-32608 if(TorqueUseHPComCard){ S1.sprintf("MCU: %s",Msg); slTorquMUClog->AddTextWithDateTime(S1); } —— HP 通訊卡，閘著
    (void)sFileName; (void)S1;
}
//---------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  AI(W906-TORQUE-1203) 20260925: HT9050 的 Index Z 扭力上限改走 1203（docs/EVAL_HT9050_INDEXZ_TORQUE_VIA_1203_20260925.md）。
//  使用者 20260925：「如果技術上可行，開始評估9050都是透過1203來獲取，你用machine type=9050來區隔功能使用」；EastSun 20260925：
//  用 CiA402 60E0h／60E1h、同意加進 Pci1203Control 的 SDO 白名單。golden 沒有 HT9050（golden 一律寫 Panasonic Pr0.13 第一扭力限制，%）。
//  分工：機台做 Pci1203Control 的內部 kind kCmdAxTorqueLimitSet（寫 60E0h＋60E1h 同一值→兩個讀回→比對，一次 Execute 一輪）；
//  筆電這裡做 golden 狀態機的外層語意 —— 值＝golden %×10（0.1% 單位；歸零 300% → 3000），回傳照 golden：
//  0＝進行中（這一輪沒對上，下次再試）、1＝成功、2＝連續 5 次沒對上（golden iRetryCount<5，:1955）。
//  生產函式庫不帶 HAVE_PCI1203 ⇒ 經 wb_serve 安裝的掛鉤呼叫 Pci1203Control；**沒有掛鉤＝回 2 並寫明原因，不假成功、也不改走 RS232**
//  （HT9050 沒有 RS232 扭力驅動器）。掛鉤由 wb_serve 在機台的 kCmdAxTorqueLimitSet 帶回 main 之後安裝。
//  MotorIndex 1（MTestZ2）在 HT9050 是 SMC、Enable=0 ⇒ 沒有驅動器可寫，視為成功（golden 的 SwReadTorue 繼電器在這台不存在）。
//------------------------------------------------------------------------------
int (*W906_Pci1203TorqueLimitHook)(int motIndex, int value01pct, AnsiString* why) = 0;   // 回 1＝寫入且兩個讀回都對上

int W906_Ht9050TorqueLimit(int MotorIndex, int Torque)
{
    static int iTries = 0;
    const int mi = (MotorIndex == 0) ? MTestZ1 : MTestZ2;
    if (MotorIndex != 0 && (MOT[mi].Motor == 0 || !MOT[mi].Motor->Enable)) {
        iTries = 0;
        return 1;
    }
    if (W906_Pci1203TorqueLimitHook == 0) {
        iTries = 0;
        std::printf("[torque] HT9050 Index Z torque limit %d%%: no 1203 hook (Pci1203Control kCmdAxTorqueLimitSet not in this build) -> 2\n", Torque);
        return 2;
    }
    AnsiString why;
    if (W906_Pci1203TorqueLimitHook(mi, Torque * 10, &why) == 1) {
        iTries = 0;
        return 1;
    }
    if (++iTries >= 5) {
        iTries = 0;
        std::printf("[torque] HT9050 Index Z torque limit %d%%: 5 tries without a matching read-back -> 2 (%s)\n", Torque, why.c_str());
        return 2;
    }
    return 0;
}
//==============================================================================
//  AI(W906-I03) 20261002 (Ifor01): golden TCOM2 的溫控器那一半（Comm2）—— I-03 第一階段
//  （docs/handoff/TO_IFOR.md I-03；FROM_IFOR §1 1002 09:0x）。golden（906，cp950 → UTF-8）：
//    rs232.dfm:8-38 object Comm2: TComm ......... W906_Comm2DfmBoot（建構子呼叫）
//    rs232.cpp:131 / :133-147 Rs232Comm2Busy／Que／RxQue／InitQue
//    rs232.cpp:163-182 RS232Init 的溫控埠檢查 ...... W906_RS232InitTempCheck
//    rs232.cpp:264-281 開 Comm2、:684-685 ........... W906_RS232InitTempOpen
//    rs232.cpp:721-757 Comm2ReceiveData
//  沒搬：golden :127-130 的 RS232zipShowFlag／Comm2ReceiveFlag／BIBTestOkFlag／Comm2BusyFlag —— 整棵 golden 除了定義沒有任何人讀寫。
//
//  與 golden 的語意差，跟扭力 Comm1 同一件事（檔頭 (1)(2)）：golden 的 SPComm 在主執行緒、以 ReadIntervalTimeout=100
//  （rs232.dfm:30）切好一包才呼叫 Comm2ReceiveData，而 Comm2ReceiveData 靠「一包一次到齊」：BufferLength<7 直接丟掉、TC401／TMC401
//  讀 data[0..7]、KT4H／E5DC／DTK4848 把整包 sprintf("%s") 進 Com2Buffer。vclcompat 的 TComm 在讀取執行緒上有幾個位元組就交幾個
//  ⇒ 讀取執行緒只排隊（W906_Comm2QueueRx），golden 的消費端 DoThermo 每拍開頭（bthermo.cpp）呼叫 W906_PumpComm2：
//  安靜滿 100 ms（dfm 那個值）才把整包交給 Comm2ReceiveData。ReadIntervalTimeout 同樣**不**設到 vclcompat 的 Comm2 上（檔頭 (2)）。
//
//  g_pCOM2Comm2／g_pDTKComm（cpublic.cpp：ht9045_globals 的縫，cpublic.cpp 在 COM2 的 library 底下，不能直接寫 COM2->Comm2）
//  在建構子就指到 Comm2 ＝ golden「COM2->Comm2 一直在」。埠沒開時 WriteCommData 回 false、什麼都不寫（vclcompat/Comm.cpp:396，同 SPComm）。
//
//  開機不會因為溫控埠打不開而卡住：RS232Init 在 InitialHandler 裡（wb_serve.cpp:4066），在 PumpInit 設 InitialOK=true（:4212）
//  之前，而 ShowMyMessage 在 InitialOK==false 時只記一筆 Exception、不顯示（golden mymessbox.cpp:765-769 Ifor 20151230；
//  wb_serve W906MbShowMyMessage 照翻）；模擬組態另外照 golden :771 直接略過含 " port error" 的訊息。
//------------------------------------------------------------------------------
extern Spcomm::TComm* g_pCOM2Comm2;   // cpublic.cpp（golden COM2->Comm2 的縫）
extern Spcomm::TComm* g_pDTKComm;     // cpublic.cpp / cpublic.h:394（DTK4848，golden 也是 COM2->Comm2）

namespace {
struct W906_Comm2RxQueue {
    CRITICAL_SECTION cs;
    std::vector<unsigned char> pending;
    DWORD lastTick;
    W906_Comm2RxQueue() : lastTick(0) { ::InitializeCriticalSection(&cs); }   // 只碰自己的成員（同 g_comm1Rx）
};
W906_Comm2RxQueue g_comm2Rx;
const DWORD  W906_COMM2_INTERVAL_MS = 100;    // rs232.dfm:30 Comm2 ReadIntervalTimeout = 100
const size_t W906_COMM2_MAX_PENDING = 2048;   // 不是 golden 的值：同 Comm1，只為了不讓一條一直在講話的線把佇列無限撐大

const int QUE_LEN_W906 = 4096;                // golden :133 #define QUE_LEN 4096（不用巨集：本樹別處可能也有同名巨集）
typedef struct tagQue                         // golden :134-139
{
    int Read;
    int Write;
    unsigned char ptr[QUE_LEN_W906];          // golden Byte ptr[QUE_LEN]
}Que;
Que RxQue;                                    // golden :140 static Que RxQue;
BOOL InitQue(Que *que)                        // golden :142-147
{
    que->Read=0;
    que->Write=0;
    return TRUE;
}
bool Rs232Comm2Busy;                          // golden :131（golden 只在 :684 寫、沒有人讀）
}

//------------------------------------------------------------------------------
//  golden rs232.dfm:8-38（object Comm2: TComm）的設計期值。建構子最後呼叫（Comm1 的設定之後）。
//------------------------------------------------------------------------------
void TCOM2Shim::W906_Comm2DfmBoot()
{
    Comm2->CommName="COM2";                                                     // rs232.dfm:9
    Comm2->BaudRate=9600;                                                       // rs232.dfm:10
    Comm2->ParityCheck=false;                                                   // rs232.dfm:11
    Comm2->Outx_XonXoffFlow=false;                                              // rs232.dfm:17
    Comm2->Inx_XonXoffFlow=false;                                               // rs232.dfm:18
    Comm2->ByteSize=_8;                                                         // rs232.dfm:24
    Comm2->Parity=None;                                                         // rs232.dfm:25
    Comm2->StopBits=_1;                                                         // rs232.dfm:26
    // rs232.dfm:30 ReadIntervalTimeout = 100 —— 刻意不設，理由同 Comm1（檔頭 (2)）
    Comm2->OnReceiveData=[this](vclcompat::TObject * /*Sender*/, void *Buffer, Spcomm::Word BufferLength)
                         { W906_Comm2QueueRx(Buffer, BufferLength); };          // rs232.dfm:35 OnReceiveData = Comm2ReceiveData（經佇列）
    g_pCOM2Comm2=Comm2;                                                         // golden 的 COM2->Comm2（cpublic.cpp 的 KT4H／TMC401／E5DC 寫入）
    g_pDTKComm=Comm2;                                                           // golden DTK4848Word*NoSucm 也寫 COM2->Comm2（cpublic.cpp 原本就走這個縫）
}
//------------------------------------------------------------------------------
void TCOM2Shim::W906_Comm2QueueRx(void *Buffer, unsigned short BufferLength)   // 讀取執行緒
{
    const unsigned char *p=(const unsigned char*)Buffer;
    ::EnterCriticalSection(&g_comm2Rx.cs);
    g_comm2Rx.pending.insert(g_comm2Rx.pending.end(), p, p+BufferLength);
    g_comm2Rx.lastTick=::GetTickCount();
    ::LeaveCriticalSection(&g_comm2Rx.cs);
}
//------------------------------------------------------------------------------
void TCOM2Shim::W906_PumpComm2()                                                // DoThermo 的節拍（bthermo.cpp）
{
    std::vector<unsigned char> frame;
    ::EnterCriticalSection(&g_comm2Rx.cs);
    if(g_comm2Rx.pending.empty()==false &&
       ((DWORD)(::GetTickCount()-g_comm2Rx.lastTick)>=W906_COMM2_INTERVAL_MS ||
        g_comm2Rx.pending.size()>=W906_COMM2_MAX_PENDING))
    {
        frame.swap(g_comm2Rx.pending);
    }
    ::LeaveCriticalSection(&g_comm2Rx.cs);

    if(frame.empty())
        return;

    const unsigned short n=(unsigned short)frame.size();
    frame.push_back(0);                                                         // golden 把整包當 C 字串 sprintf("%s") 進 Com2Buffer；SPComm 的緩衝區比一包大，這裡補一格 NUL
    Comm2ReceiveData(Comm2, &frame[0], n);
}
//------------------------------------------------------------------------------
//  golden TCOM2::Comm2ReceiveData（rs232.cpp:721-757），逐句照翻。
//------------------------------------------------------------------------------
void TCOM2Shim::Comm2ReceiveData(void * /*Sender*/, void *Buffer, unsigned short BufferLength)
{
    if(InitialOK==false)                                                        //Steven 20120202 : 加入Thread保護
        return;

    if(BufferLength<7)
        return;

    unsigned char *data;                                                        // golden byte *data;
    data=(unsigned char *)Buffer;

    if(BufferLength>250)
       BufferLength=250;

    if(TC401HeaterControl==TC401)                                               //Steven 20141030 : 新增OMRON E5DC溫控器
    {
        for(int i=0; i<8; i++)
            READBUFF[i]=data[i];
    }
    else if(TC401HeaterControl==E5DC)                                           //Steven 20141030 : 新增OMRON E5DC溫控器
    {
        Com2Buffer.sprintf("%s", (char*)Buffer);                                //Steven 20111028 : 改成AnsiString
    }
    else
    {
        if(data[0]!=':')
        {
            return;
        }
        else
        {
            Com2Buffer.sprintf("%s", (char*)Buffer);                            //Steven 20111028 : 改成AnsiString
        }
    }
    Com2ReceiveOK=true;
}
//------------------------------------------------------------------------------
//  golden TCOM2::RS232Init 的溫控埠檢查（rs232.cpp:163-182），逐句照翻。RS232Init :271 同一行呼叫。
//------------------------------------------------------------------------------
void W906_RS232InitTempCheck(bool *flag)
{
    if(TC401HeaterControl==NoHeater)                                            //Steven 20171227 (Wei) : Add for HT-9045L
    {
        flag[1]=false;
        flag[2]=false;
    }
    else
    {
        flag[1]=GetCOMPortStatus(HSys.sTempComPort);
        if(flag[1]==false)
            ShowMyMessage("Temperature KT4H : "+HSys.sTempComPort+" port error", "");

        if(USE_16_HEATER==eht16HeaterEJ1N ||
           USE_16_HEATER==eht32HeaterEJ1N ||
           USE_16_HEATER==eht32HeaterDTME08)                                    //Steven 20140923 : Index使用EJ1N版32組加熱器
        {
            flag[2]=GetCOMPortStatus(HSys.sTempOmronComPort);
            if(flag[2]==false)
                ShowMyMessage("Temperature EJ1N :"+HSys.sTempOmronComPort+" port error", "");
        }
    }
}
//------------------------------------------------------------------------------
//  golden TCOM2::RS232Init 的開溫控埠（rs232.cpp:264-281）與 :684-685，逐句照翻。RS232Init :301 同一行呼叫。
//  golden 的 SPComm StartComm 開不了埠會丟例外 → 下面的 catch；vclcompat 的 StartComm 開不了會安靜地轉成模擬
//  （vclcompat/Comm.cpp:340-343），所以多看一次 IsSimMode()，讓「開不了」照樣走 golden catch 那兩行（記錄＋訊息）。
//------------------------------------------------------------------------------
void W906_RS232InitTempOpen(const bool *flag)
{
    if(flag[1] && USE_NEW_TEMPCTRL_FUNCTION==false)
    {
        COM2->Comm2->CommName="\\\\.\\"+HSys.sTempComPort;
        COM2->Comm2->Parity=None;
        COM2->Comm2->BaudRate=9600;
        COM2->Comm2->ByteSize=_8;
        COM2->Comm2->ParityCheck=false;
        COM2->Comm2->StopBits=_1;
        try
        {
            COM2->Comm2->StartComm();                                           //僅能啟動一次
            if(COM2->Comm2->IsSimMode())                                        // AI(W906-I03): SPComm 會在這裡丟例外（見上）
                throw 0;
        }
        catch(...)
        {
            MyDBIProcess("Exception", "TCOM2::RS232Init");
            ShowMyMessage("Temperature KT4H : "+HSys.sTempComPort+" port error", "");
        }
    }

    Rs232Comm2Busy=false;                                                       // golden :684
    InitQue(&RxQue);                                                            // golden :685
}
//------------------------------------------------------------------------------
//  bthermo.cpp DoThermo（ht9045_sm；不 include atester_shims.h）每拍開頭用 block-scope extern 呼叫這一支。
//------------------------------------------------------------------------------
void W906_PumpTempComm2()
{
    COM2->W906_PumpComm2();
}
