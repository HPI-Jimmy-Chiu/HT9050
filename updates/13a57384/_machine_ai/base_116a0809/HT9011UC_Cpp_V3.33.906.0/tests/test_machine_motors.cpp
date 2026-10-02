// ===========================================================================
//  tests/test_machine_motors.cpp
//
//  AI(W906-W3-10) 20260925: 週末計畫 W3 第 10 項（LoadMotData＋InitialMotorName／InitialMotorParameter
//  ＋各廠牌 InitMotor）的「第三級：值有被維護」測試。
//
//  golden InitialMotorParameter 的 if（IO_CARD_TYPE==NewIO_MN200 || PCI_P64C64）那半（移植 cinitial.cpp:3870 起）
//  對每一軸 M%02d 查 Mot_Table.csv，依 CardModel 建驅動物件（PCI1203 -> TMyEtherCatMotor、MN200 -> TMyMN200Motor、
//  SYNTEK -> TMySYNTEKMotor、其他 -> TMySMCMotor），再把齒輪比／方向／速度／軟體極限／加減速寫進物件。
//  尾段把沒被指派的軸補成 TMySYNTEKMotor(-1)＋Enable=false。Index 四軸在 INDEX_MOTION_CARD==0 時改走 Galil。
//
//  這支拿版控的 machines/HT9050/Mot_Table.csv（argv[1]）餵進去：
//    (1) 全表：表內每一軸，驅動類型與 CardModel 對得上、16 個欄位逐一等於表值（零不符）；
//        1203／MN200 軸的 iBoardID／iPortID 等於表的 BoardID／Port（位址碼 board*100+port 的往返）。
//    (2) 手算：M00 MInArmX、M02 MInArmPitch、M14 MTestZ1、M140 MMagazine 的值直接寫死（CSV 逐列讀出，不經解析器）。
//    (3) Enable：模擬組態 golden 強制全關；出貨組態 = 表的 Enable（且位址碼 >= 0）。
//    (4) INDEX_MOTION_CARD：HT9050 的 Index 走 1203，機台設定要非 0（=1）；設 0 時四軸改成 TMyGALILMotor（golden 行為）。
//  出貨組態的 Enable 軸會呼叫 InitMotor：這台沒有 1203 SDK（HAVE_PCI1203=0）⇒ Open_Axis 不開軸、InitMotor 回 false，
//  不碰硬體；有卡的機台上這一步會真的寫軸參數（這支不能在有卡的機台上跑）。
//
//  用法：test_machine_motors <Mot_Table.csv>
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include "database.h"
#include "common.h"
#include "cinitial.h"
#include "cmydef.h"
#include "CosFunction.h"
#include "Motor/mymotor.h"
#include "Motor/myEthercatmotor.h"
#include "Motor/myMN200motor.h"
#include "Motor/mySMCmotor.h"
#include "Motor/mySYNTEKmotor.h"
#include "Motor/myGALILmotor.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>

#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define HT9045_TEST_PUTENV _putenv
#else
#define HT9045_TEST_PUTENV putenv
#endif

void InitialMotorParameter();   // cinitial.cpp:3835（golden cinitial.cpp:3392）

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line) {
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_machine_motors.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

static bool feq(double a, double b) { return std::fabs(a - b) < 1e-9; }

// TMyEtherCatMotor 自己宣告 protected 的 `short iBoardID/iPortID`（golden myEthercatmotor.h:39-40），遮蔽了
// HTMotor 的 `unsigned int iBoardID/iPortID`（golden HTMotor.h:49-50）；ctor 只寫自己那份（golden :49-68），
// HTMotor 的建構子兩份都不寫 ⇒ 透過 HTMotor* 讀 1203 軸的 iBoardID 是未定義值（golden 相同：HTMotor 不是 TObject，
// BCB6 的 new 不清零）。所以 1203 的位址要讀它自己那份；用衍生類別取成員指標（標準允許的 protected 存取）。
struct EcatPeek : TMyEtherCatMotor {
    static short TMyEtherCatMotor::* Board() { return &EcatPeek::iBoardID; }
    static short TMyEtherCatMotor::* Port()  { return &EcatPeek::iPortID; }
};
static int EcatBoard(HTMotor* m) { TMyEtherCatMotor* e = dynamic_cast<TMyEtherCatMotor*>(m); return e ? e->*EcatPeek::Board() : -9999; }
static int EcatPort(HTMotor* m)  { TMyEtherCatMotor* e = dynamic_cast<TMyEtherCatMotor*>(m); return e ? e->*EcatPeek::Port()  : -9999; }

static TMOTDATA* RowOf(int i)
{
    AnsiString nm; nm.sprintf("M%02d", i);
    HSys.mapMotTableIter = HSys.mapMotTable.find(nm);
    if (HSys.mapMotTableIter == HSys.mapMotTable.end()) return 0;
    const int k = HSys.mapMotTableIter->second.ToIntDef(-1);
    if (k < 0) return 0;
    return HSys.MotTable[k];
}

int main(int argc, char** argv)
{
    if (argc < 2) { std::printf("usage: test_machine_motors <Mot_Table.csv>\n"); return 2; }
    static std::string env = std::string("W906_MOTTABLE_PATH=") + argv[1];
    HT9045_TEST_PUTENV(const_cast<char*>(env.c_str()));

    IO_CARD_TYPE      = NewIO_MN200;   // golden 的 if 那半
    INDEX_MOTION_CARD = 1;             // HT9050：Index 走 1203（Mot_Table 的 M14 MTestZ1 = PCI1203）
    for (int i = 0; i < TOTAL_MOTOR; ++i) MOT[i].Motor = NULL;
    InitialMotorParameter();
    std::printf("Mot_Table rows: %d\n", (int)HSys.MotTable.size());
    CHECK(HSys.MotTable.size() > 0);

    // ---- (1) 全表 ----
    int rows = 0, bad = 0, typeBad = 0, addrBad = 0, nullMot = 0, n1203 = 0, nMN200 = 0, nSMC = 0, enabled = 0;
    int smcBad = 0, smcAddr = 0;   // AI(W906-W3-AUDIT) 20260925
    for (int i = 0; i < TOTAL_MOTOR; ++i) {
        if (MOT[i].Motor == NULL) { ++nullMot; continue; }
        if (MOT[i].Motor->Enable) ++enabled;
        TMOTDATA* r = RowOf(i);
        if (!r) continue;
        ++rows;
        HTMotor* m = MOT[i].Motor;
        bool typeOk;
        if (r->CardModel == "PCI1203")     { typeOk = dynamic_cast<TMyEtherCatMotor*>(m) != 0; ++n1203; }
        else if (r->CardModel == "MN200")  { typeOk = dynamic_cast<TMyMN200Motor*>(m) != 0;    ++nMN200; }
        else if (r->CardModel == "SYNTEK") { typeOk = dynamic_cast<TMySYNTEKMotor*>(m) != 0; }
        else if (r->CardModel == "MC88X1") { typeOk = dynamic_cast<TMySYNTEKMotor*>(m) != 0; }   // golden 的 MC88X1 分支不 new（HTMC88X1Motor 被註解）⇒ 尾段補 SYNTEK(-1)
        else                               { typeOk = dynamic_cast<TMySMCMotor*>(m) != 0;       ++nSMC; }
        if (!typeOk) { if (typeBad < 5) std::printf("  type mismatch: M%02d model=%s\n", i, r->CardModel.c_str()); ++typeBad; }
        CHECK(MOT[i].CardType == r->CardModel);

        if ((r->CardModel == "PCI1203" || r->CardModel == "MN200") && r->iBoardID != -1 && r->iPort != -1) {
            const bool ecat = (r->CardModel == "PCI1203");
            const int b  = ecat ? EcatBoard(m) : (int)m->iBoardID;
            const int pt = ecat ? EcatPort(m)  : (int)m->iPortID;
            if (b != r->iBoardID || pt != r->iPort) {
                if (addrBad < 5) std::printf("  addr mismatch: M%02d %d/%d vs %d/%d\n", i, b, pt, r->iBoardID, r->iPort);
                ++addrBad;
            }
        }

        // AI(W906-W3-AUDIT) 20260925: SMC 軸的位址往返（W3 稽核缺口：原本只驗 1203／MN200）。
        //   golden：iAdder=BoardID*10+Port（cinitial.cpp:4001，條件是表上 BoardID／Port 都有填）→
        //   TMySMCMotor(Addr) 拆成 iBoardID=Addr/10、iPortID=Addr%10+1（Motor/mySMCmotor.cpp:360-361）；
        //   沒填的軸 iAdder=-1 → 兩者都是 MAX_SMC_CARD-1（:354-355）。SMC 沒有遮蔽 HTMotor 的這兩個成員。
        if (r->CardModel == "SMC") {
            const unsigned MAX_SMC_CARD = 16;   // Motor/mySMCmotor.cpp:162 的檔內 #define（測試看不到）；值改了這裡會紅
            const bool hasAddr = (r->iBoardID != -1 && r->iPort != -1);
            const int addr = hasAddr ? r->iBoardID * 10 + r->iPort : -1;
            const unsigned eb = hasAddr ? (unsigned)(addr / 10)     : (unsigned)(MAX_SMC_CARD - 1);
            const unsigned ep = hasAddr ? (unsigned)(addr % 10 + 1) : (unsigned)(MAX_SMC_CARD - 1);
            if (m->iBoardID != eb || m->iPortID != ep) {
                if (smcBad < 5) std::printf("  SMC addr mismatch: M%02d %u/%u vs expected %u/%u\n", i, m->iBoardID, m->iPortID, eb, ep);
                ++smcBad;
            }
            if (hasAddr) ++smcAddr;
        }

        double acc = r->dAcc;
        if (r->CardModel == "MN200" && acc > 1) acc = r->dAcc / 100.0;
        const bool ok = feq(m->GearRatio, r->dGearRatio) &&
                        m->Direction == (r->iDirection == 1) && m->HomeDirection == (r->iHomeDirectior == 1) &&
                        (int)m->PHomeHighSpeed == r->iHomeHighSpeed && (int)m->PHomeLowSpeed == r->iHomeLowSpeed &&
                        (int)m->PJogHighSpeed == r->iJogHighSpeed && (int)m->PJogLowSpeed == r->iJogLowSpeed &&
                        (int)m->InitSpeed == r->iInitSpeed && m->PServoAlarmOn == (r->iServoAlarmOn == 1) &&
                        m->MotorType == r->i1P2P && m->bSensorType == (r->iSensorType != 0) &&
                        m->bLimitLogic == (r->iLimitLogic == 1) && m->bIn1Logic == (r->iIn1Logic == 1) &&
                        m->PSoftLimitN == r->iSoftLimitN && m->PSoftLimitP == r->iSoftLimitP &&
                        MOT[i].SimulateSpeed == r->iSimulateSpeed && feq(m->GetAccDataBase(), acc);
        if (!ok) { if (bad < 5) std::printf("  field mismatch: M%02d %s\n", i, r->Alias.c_str()); ++bad; }
    }
    std::printf("axes in table %d (PCI1203 %d, MN200 %d, SMC %d) | field bad %d | type bad %d | addr bad %d | NULL %d | enabled %d\n",
                rows, n1203, nMN200, nSMC, bad, typeBad, addrBad, nullMot, enabled);
    CHECK(rows == 48);                   // M00..M43（44）＋M108／M140／M141／M153
    CHECK(n1203 == 19 && nMN200 == 16 && nSMC == 13);   // awk 量 CSV 第 23 欄：PCI1203 19、MN200 16、SMC 13
    CHECK(bad == 0 && typeBad == 0 && addrBad == 0);
    CHECK(nullMot == 0);                 // 尾段補齊：MainProc 每拍直接讀 MOT[i].Motor->Enable，不可有 NULL
    // AI(W906-W3-AUDIT) 20260925: SMC 位址。有填 BoardID／Port 的 SMC 軸 = 4（M12 0/4、M140 2/4、M141 2/5、M153 5/1，
    //   直接讀 CSV 數的，不是拿這支測試的輸出回填）；手算一列 M140：2*10+4=24 → iBoardID 2、iPortID 5。
    std::printf("SMC: %d with BoardID/Port, %d address mismatch\n", smcAddr, smcBad);
    CHECK(smcBad == 0);
    CHECK(smcAddr == 4);
    if (MOT[140].Motor && dynamic_cast<TMySMCMotor*>(MOT[140].Motor)) {
        CHECK(MOT[140].Motor->iBoardID == 2u && MOT[140].Motor->iPortID == 5u);
    } else {
        CHECK(!"MOT[140] (MMagazine) should be a TMySMCMotor");
    }

    // ---- (2) 手算（CSV 逐列讀出） ----
    {   // M00,MInArmX,-999999,999999,0,0,,0,1,0,200,50,500,10000,100,90,1,1,100,1,1,10000,PCI1203,100000,100000,2,,0,1  //AI(W906-MACHINES) 20260928: machine master Mot_Table.csv:2 (Direction 1 -> 0, RULINGS_20260925 #5 B)
        HTMotor* m = MOT[0].Motor;
        CHECK(dynamic_cast<TMyEtherCatMotor*>(m) != 0);
        CHECK(EcatBoard(m) == 0 && EcatPort(m) == 0);
        CHECK(feq(m->GearRatio, 1.0) && m->Direction == false && m->HomeDirection == false);   //AI(W906-MACHINES) 20260928: Direction column 0 in the machine master (was 1)
        CHECK(m->PHomeHighSpeed == 200 && m->PHomeLowSpeed == 50 && m->InitSpeed == 500);
        CHECK(m->PJogHighSpeed == 10000 && m->PJogLowSpeed == 100);
        CHECK(m->PServoAlarmOn == true && m->MotorType == 1 && m->bSensorType == true);
        CHECK(m->bLimitLogic == false && m->bIn1Logic == true);
        CHECK(m->PSoftLimitN == -999999 && m->PSoftLimitP == 999999);
        CHECK(feq(m->GetAccDataBase(), 100000.0));
    }
    {   // M02,MInArmPitch,-999999,999999,2,18,,1,2.5,1,50,10,50,400,100,50,0,0,30,0,0,10000,MN200,0.1,0.1,0,,0,1
        HTMotor* m = MOT[2].Motor;
        CHECK(dynamic_cast<TMyMN200Motor*>(m) != 0);
        CHECK(m->iBoardID == 2 && m->iPortID == 18);
        CHECK(feq(m->GearRatio, 2.5) && m->Direction == true && m->HomeDirection == true);
        CHECK(m->PHomeHighSpeed == 50 && m->PJogHighSpeed == 400 && m->PServoAlarmOn == false);
        CHECK(feq(m->GetAccDataBase(), 0.1));                       // MN200 單位是秒；<= 1 不除 100
        CHECK(m->Enable == false);                                  // 表的 Enable=0（兩組態都關）
    }
    {   // M14,MTestZ1,-999999,999999,14,0,,1,1,1,6000,300,100,900000,100,10,1,1,70,1,0,10000,PCI1203,9000000,9000000,2,-99999,0,1
        HTMotor* m = MOT[MTestZ1].Motor;
        CHECK(MTestZ1 == 14);
        CHECK(dynamic_cast<TMyEtherCatMotor*>(m) != 0);             // INDEX_MOTION_CARD=1 ⇒ 照表走 1203
        CHECK(EcatBoard(m) == 14 && EcatPort(m) == 0);  { extern int W906_EcEngineMotorsAt(int, int); CHECK(W906_EcEngineMotorsAt(14, 0) == 1); }   //AI(W906-INDEXZ) 20260930: the one TMyEtherCatMotor at M14's address is MTestZ1 itself (INDEX_MOTION_CARD=1)
        CHECK(m->PHomeHighSpeed == 6000 && m->PJogHighSpeed == 900000);
        CHECK(MOT[MTestZ1].IndexPickLimit == -99999);               // golden :4088 只給 MTestZ1／Z2
    }
    {   // M140,MMagazine,-610,39137,2,4,,1,0.1,0,500,10,1000,8000,100,50,0,1,25,1,0,10000,SMC,70,70,2,,1,1
        HTMotor* m = MOT[140].Motor;
        CHECK(dynamic_cast<TMySMCMotor*>(m) != 0);
        CHECK(m->PSoftLimitN == -610 && m->PSoftLimitP == 39137);
        CHECK(feq(m->GearRatio, 0.1) && m->InitSpeed == 1000 && m->bLimitLogic == true);
    }

    // ---- (3) Enable ----
#ifdef SOFT_SIMULTE
    CHECK(enabled == 0);                 // golden：模擬組態每一軸 Enable=false
#else
    CHECK(MOT[0].Motor->Enable == true && MOT[MTestZ1].Motor->Enable == true);   // 表 Enable=1、位址碼 >= 0
    CHECK(MOT[2].Motor->Enable == false);                                         // 表 Enable=0
    CHECK(MOT[13].Motor->Enable == false);                                        // M13 MTestY1：SMC、BoardID／Port 空 ⇒ 位址碼 -1 ⇒ 關
    CHECK(enabled == 14);                // awk 量：表內 PCI1203 19 軸，其中 14 軸 Enable=1 且有 BoardID／Port；MN200／SMC 全是 0   //AI(W906-MOTZ-OFF) 20260930: 19 -> 14 -- M35 MLoaderZ / M36 MEmptyZ / M38-M40 MAuto1Z-3Z Enable 1 -> 0 in machines/HT9050/Mot_Table.csv (HT9050 tray elevators are cylinders, [TrayZ] *_Z_USE_MOTOR=0; user 0930 RULINGS_20260930 #2 / #3)
#endif

    // ---- (4) INDEX_MOTION_CARD==0：Index 四軸改走 Galil（golden :3923-3948） ----
    INDEX_MOTION_CARD = 0;
    InitialMotorParameter();
    CHECK(dynamic_cast<TMyGALILMotor*>(MOT[MTestY1].Motor) != 0);
    CHECK(dynamic_cast<TMyGALILMotor*>(MOT[MTestZ1].Motor) != 0);
    CHECK(dynamic_cast<TMyGALILMotor*>(MOT[MTestZ2].Motor) != 0);
    CHECK(dynamic_cast<TMyGALILMotor*>(MOT[MTestY2].Motor) != 0);
    CHECK(dynamic_cast<TMyEtherCatMotor*>(MOT[0].Motor) != 0);      // 其他軸不受影響
#ifdef SOFT_SIMULTE
    CHECK(MOT[MTestZ1].Motor->Enable == false);
#else
    CHECK(MOT[MTestZ1].Motor->Enable == true);                      // golden：Galil 分支非 3 軸就開（這台沒有 Galil 卡 ⇒ 開了也動不了）
#endif

    // ---- (5) AI(W906-INDEXZ) 20260930: INBOX 113 -- no Index Z1 route installed = golden's construction (review #7) ----
    //   cc426093 built M13 / M15 / M16 disabled from the table right here in InitialMotorParameter, so an HT9050 ship build
    //   WITHOUT the route was not golden. D1 now happens in the installer (W906_GaliRouteDisableAbsentIndexAxes), after a
    //   successful install only. And the engine motor route can never claim M14 here: no TMyEtherCatMotor at its address.
    {
        extern bool W906_GaliRouteOwns(int); extern int W906_EcEngineMotorsAt(int, int);
        CHECK(!W906_GaliRouteOwns(MTestZ1));               // no route installed (no ctest but the two route tests installs one)
#ifndef SOFT_SIMULTE
        CHECK(MOT[MTestY1].Motor->Enable == true && MOT[MTestZ2].Motor->Enable == true && MOT[MTestY2].Motor->Enable == true);   // golden ship arm, 4-axis
#endif
        CHECK(W906_EcEngineMotorsAt(14, 0) == 0);          // MOT[MTestZ1] is a TMyGALILMotor (INDEX_MOTION_CARD=0)
        const TMOTDATA* y1 = RowOf(MTestY1); const TMOTDATA* z2 = RowOf(MTestZ2); const TMOTDATA* y2 = RowOf(MTestY2);
        CHECK(y1 && z2 && y2 && y1->iEnable == 0 && z2->iEnable == 0 && y2->iEnable == 0);   // what the installer's D1 reads: M13 / M15 / M16 Enable 0
    }

    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
