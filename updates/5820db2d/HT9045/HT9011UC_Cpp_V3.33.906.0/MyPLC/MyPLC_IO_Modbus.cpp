//------------------------------------------------------------------------------
// AI(W5-MyPLC-Translate) 20260710: BCB6 -> standard C++17 translation.
// Provenance: HT9011UC_Code_V3.33.906.0_20260618/MyPLC/MyPLC_IO_Modbus.cpp
// Changes from BCB6 original:
//   - Dropped #include "MachineDefine.h" / #pragma hdrstop / #pragma
//     package(smart_init): Borland-specific, not portable.
//   - #include "common.h" / "mymessbox.h" DROPPED: neither is needed by the
//     ACTIVE code in this file. common.h's MySleepEx is still gated
//     `#if 0 // TODO(wave-timing)` there (a local guarded fallback def is
//     supplied below, same HAVE_xxx idiom as MyLaneIo.cpp's HAVE_PLC_IO --
//     see that comment). mymessbox.h's ShowMyMessage is referenced ONLY
//     inside the golden `/* ... */` DISABLED block in PLCStatusCheck (see
//     "Ken 20250428 : Mark" below) -- dead code, so no dependency is needed.
//   - `#include "cmydef.h"` kept: supplies Enable_PLCSafety_IO / iPLCSafetyVer
//     (cmydef.h:5605-5606) and MyBitMask (cmydef.h:127), already translated.
//   - `HTimer hCheckTime/hCheckThreadTime`: golden HTimer.h is not present in
//     this repo snapshot (external component header); same faithful alias
//     already established by acatchtray.cpp -- `typedef TQPF_Timer HTimer;`.
//   - `TPLCIOThread : public TThread` -> STANDALONE offline-inert class (see
//     MyPLC_IO_Modbus.h header note); Execute()'s do-while body is kept
//     VERBATIM but is dead code (never invoked -- no real thread spawns it).
//   - `tpNormal` (golden VCL Classes::TThread::TPriority enumerator): a
//     FILE-SCOPE (internal-linkage, `const int` at namespace scope) local
//     constant -- deliberately NOT put in a shared header, to avoid any risk
//     of colliding with a similarly-named constant a different translation
//     wave might introduce for another TThread-derived class (ScanBtnThread/
//     uHeaterThread/uShuttleThread all also derive from TThread in golden,
//     none translated yet).
// ------------------------------------------------------------------------------
// IMPORTANT CROSS-UNIT LINK NOTE (see this unit's translate report):
//   bPLCInData / bPLCIO here are the REAL, canonical definitions (matching
//   MyLaneIo.cpp's already-existing offline stub of the SAME two arrays,
//   guarded `#ifndef HAVE_PLC_IO`). When this file is linked into the SAME
//   executable as MyLaneIo.cpp, MyLaneIo.cpp must be recompiled with
//   -DHAVE_PLC_IO=1 (that file already documents this exact plan) to avoid a
//   duplicate-definition link error. This file needs NO reciprocal guard.
//------------------------------------------------------------------------------

#include "vclcompat/vcl_compat.h"

#include "MyPLC_IO_Modbus.h"

#include "ModbusTCPClient.h"
#include "cmydef.h"          // Enable_PLCSafety_IO, iPLCSafetyVer, MyBitMask
//------------------------------------------------------------------------------

// golden HTimer.h alias (not present in this repo snapshot); same idiom
// already established by acatchtray.cpp: `typedef TQPF_Timer HTimer;`
typedef TQPF_Timer HTimer;

// ---------------------------------------------------------------------------
// MySleepEx offline fallback (golden common.h:260, gated `#if 0 //
// TODO(wave-timing)` there). The W6.5 SHUTTLE wave already ships a REAL
// definition in acarry_shims.cpp (offline no-op returning 0) for the SAME
// signature; guard so a link unit that includes BOTH acarry_shims.cpp and
// this file does not get a duplicate symbol -- set HAVE_MYSLEEPEX=1 when
// compiling THIS file into such a link unit (same idiom as MyLaneIo.cpp's
// HAVE_PLC_IO guard, see above).
// ---------------------------------------------------------------------------
#ifndef HAVE_MYSLEEPEX
// AI(W906-GA1-B2-integrate) 20260804: MySleepEx offline fallback RETIRED -- real
// common.cpp (wave17) body now reachable; keeping this collided when a target
// links ht9045_comms together with ht9045_core.
DWORD MySleepEx(DWORD dwMilliseconds, bool bAlertable);  // real body: common.cpp (declaration here because common.h's is still gated)
#else
extern DWORD MySleepEx(DWORD dwMilliseconds, bool bAlertable);
#endif

// golden VCL Classes::TThread::TPriority enumerator value used only by
// `MyPLCIOThread->Priority=tpNormal;` below; internal linkage (see header
// note above) so it cannot collide with any other translation wave's copy.
static const int tpNormal = 1;

//------------------------------------------------------------------------------

TPLCIOThread *MyPLCIOThread=NULL;
ModbusTCPClient PlcComm;
std::vector<TMyPLC_IO_Modbus*> TMyPLC_IO_Modbus::PLC_IO_Members;

int iTransactionID=0;
int iPLCIOTask=-1;
int iPLCStatusTask=-1;  static void W906_PlcM1SFill(int slave, const VTBYTEDATA& data);  void W906_PlcM1SAllDoors(int slave);   //ben 20230913 add   //AI(W906-PLCMODEL) 20261006: model 1 parser, end of this file; same line

bool bOSSDData[INPUT_MAX_Slave][2][8];
bool bScanSlave[INPUT_MAX_Slave];
// AI(W906-GA1-B2-integrate) 20260804: demoted to extern -- definitions live in
// MyLaneIo.cpp (ht9045_io, in every closure); golden home was here (:86-87),
// deviation disclosed there. Zero behaviour change (same storage).
extern bool bPLCInData[INPUT_MAX_Slave][INPUT_MAX_REGISTER][8];
extern bool bPLCIO[2048][8];                                                    //for 舊架構 (for the legacy architecture)
bool bPLCIOEffect=false;
bool bIOPowered=false;
bool bSafePLCThread=false;                                                      //ben 20230913 add

byte InPortData_Byte[INPUT_MAX_Slave][INPUT_MAX_REGISTER];

HTimer hCheckTime;
HTimer hCheckThreadTime;                                                        //ben 20230913 add
//------------------------------------------------------------------------------
void InitPLCIO(AnsiString asIP,int iPort)
{
    int slave;
    iPLCIOTask=1;
    iPLCStatusTask=1;                                                           //ben 20230913 add
    for(int i=0; i<(int)TMyPLC_IO_Modbus::PLC_IO_Members.size(); i++)
    {
        slave=(TMyPLC_IO_Modbus::PLC_IO_Members.at(i))->GetSlave();

        if(slave>=0 && slave<INPUT_MAX_Slave)                                   //僅掃描有使用的 (only scan the ones in use)
        {
            bScanSlave[slave]=true;
        }
    }
    PlcComm.SetTCPInfo(asIP,iPort);

    PlcComm.SetScan(false);
    PlcComm.SetScan(true);

    if(MyPLCIOThread==NULL)
    {
        MyPLCIOThread=new TPLCIOThread(false);
        MyPLCIOThread->Priority=tpNormal;
        MyPLCIOThread->Resume();                                                //ben 20230913 add
    }
}
//------------------------------------------------------------------------------
TPLCIOThread::TPLCIOThread(bool CreateSuspended)
    : Terminated(false), Priority(0)
{
    (void)CreateSuspended;   // golden: TThread(CreateSuspended); no base offline (see header note)
}
//------------------------------------------------------------------------------
void TPLCIOThread::PLCIOProcess(void)
{
    PLCIOTaskCycle();
    PLCStatusCheck();                                                           //ben 20230913 add
    PlcComm.Cycle();
}
//------------------------------------------------------------------------------
bool bFirstConnect=true;
TQPF_Timer tCycleConnect;
void PLCStatusCheck()                                                           //ben 20230913 add
{
    if(PlcComm.IsConnected()==false)
    {
        if(Enable_PLCSafety_IO)                                                 //Austin 20190531.01 增加PLC_IO模組及安全相關IO. //Jason 20230619 增加安全PLC部分 (add the PLC_IO module + safety-related IO / add the safety-PLC part)
        {
/*            if(bFirstConnect==false)                                            //JerryYang 20250206 : fix //Ken 20250428 : Mark
            {
                for(int i=0; i<INPUT_MAX_Slave; i++)
                {
                    for(int j=0; j<INPUT_MAX_REGISTER; j++)
                    {
                        if(j>=ePLCSafeDoor1)                                    //KenHsieh 20250208 : Fix PLC 斷線
                        {
                            for(int k=0; k<8; k++)
                            {
                                if(j>=20)                                       //OSSD 安全輸出 30021 30022
                                {
                                    bOSSDData[i][j-20][k]=InPortData_Byte[i][j] & MyBitMask[k];
                                }
                                else
                                {
                                   bPLCInData[i][j][k]=false;
                                }
                            }
                        }
                    }
                }

                ShowMyMessage("PLC disconnect. Initial PLC!");
            }*/

            if(bFirstConnect || tCycleConnect.Off())
            {
                bFirstConnect=false;
                bScanSlave[0]=true;
                InitPLCIO("172.16.8.120", 502);                                 //固定 (fixed)
                tCycleConnect.SetMSAndOn(5000);
            }
        }
        return;
    }
    else
    {
        tCycleConnect.SetMSAndOn(5000);                                         //JerryYang 20250206 : fix
    }

    int &Task=iPLCStatusTask;

    switch(Task)
    {
        case 1:
            hCheckThreadTime.SetSecAndOn(1);
            Task=10;
            break;
        case 10:
            if(hCheckThreadTime.Off())
            {
                bSafePLCThread=!bSafePLCThread;
                Task=1;
            }
            break;
    }
}
//------------------------------------------------------------------------------
void PLCIOTaskCycle()
{
    if(PlcComm.IsConnected()==false)
        return;

    int &Task=iPLCIOTask;
    static int iSlave=0;

    VTBYTEDATA data;
    data.clear();
    AnsiString  asTemp;

    switch(Task)
    {
        case 1:
            if(iSlave<0 || iSlave>=INPUT_MAX_Slave)
            {
                for(int i=0; i<INPUT_MAX_Slave; i++)
                {
                    if(bScanSlave[i])
                    {
                        iSlave=i;
                        i=INPUT_MAX_Slave;
                    }
                }
            }

            if(iTransactionID>0xffff)
                iTransactionID=0;                                               // 0-0xffff (2byte)

            if(bScanSlave[iSlave])
            {
                data.push_back(HIBYTE(iSafePlcModel==1 ? 17 : (INPUT_MAX_REGISTER+1)/2));   //一個Register為1個Byte 但一個位址包含兩個Register (one register is one byte, but one address spans two registers)   //AI(W906-PLCMODEL) 20261006: model 1 (Reer M1S COM) = 17 registers 0x400-0x410 (manual 8547780 p.28)
                data.push_back(LOBYTE(iSafePlcModel==1 ? 17 : (INPUT_MAX_REGISTER+1)/2));   //AI(W906-PLCMODEL) 20261006: same
                //PLC Reg 30001 == 0x00 回應的資料為2Byte資料 30002 30001 但30001為系統狀態 輸入暫存器從30002開始 (the reply for PLC Reg 30001==0x00 is 2 bytes: 30002,30001 -- but 30001 is system status, the input registers actually start at 30002)
                //PLC Reg 30003 == 0x01 回應的資料為2Byte資料 30004 30003 (the reply for PLC Reg 30003==0x01 is 2 bytes: 30004,30003)
                PlcComm.SendData(iTransactionID, iSlave, 4, iSafePlcModel==1 ? 0x400 : 0x00, data);   //AI(W906-PLCMODEL) 20261006: model 1 reads from 0x400 (System I/O; 0x000-0x001 there are the host's WRITE area -- why model 0's address read all zero on this machine)
                hCheckTime.SetSecAndOn(1);
                Task=100;
            }
            else
            {
                iSlave++;
            }
            break;
        case 100:
            if(PlcComm.GetRecevie(iTransactionID,data))                         //讀取成功 (read succeeded)
            {   if(iSafePlcModel==1) W906_PlcM1SFill(iSlave, data); else        //寫入Byte陣列 (write into the byte array)   //AI(W906-PLCMODEL) 20261006: model 1 lays the M1S System I/O block into the same byte array (end of file), then golden's bit split below runs unchanged; model 0 = golden's byte loop on the next line. Same line
                for(int i=0; i<(int)data.size(); i++)
                {
                    if(i<INPUT_MAX_REGISTER)
                    {                                                           //資料順序整理 因PLC暫存器是先放高位資料 所以 0->1  1->0  2->3  3->2 (reorder the data: the PLC register puts the high byte first, so 0->1 1->0 2->3 3->2)
                        InPortData_Byte[iSlave][(i/2)*2+abs(i%2-1)]=data[i];
                    }
                }

                for(int i=0; i<INPUT_MAX_Slave; i++)                            //寫入boolean陣列 (write into the boolean array)
                {
                    for(int j=0; j<INPUT_MAX_REGISTER; j++)
                    {
                        for(int k=0; k<8; k++)
                        {
                            if(j>=20)                                           //OSSD 安全輸出 30021 30022 (OSSD safety output 30021 30022)
                            {
                                bOSSDData[i][j-20][k]=InPortData_Byte[i][j]&MyBitMask[k];
                            }
                            else
                            {
                                bPLCInData[i][j][k]=InPortData_Byte[i][j]&MyBitMask[k];
                            }
                        }
                    }
                }

                bIOPowered=bOSSDData[0][0][0];

                //if(iPLCSafetyVer==202204)                                     //joe 20220406.01 debug safe plc
                    bIOPowered=true;

                bPLCIOEffect=true;                                              //完成過一次 (completed at least once)
                iSlave++;
                iTransactionID++;
                Task=1;
            }
            else
            {
                if(hCheckTime.Off())
                {
                    iSlave++;
                    iTransactionID++;
                    Task=1;
                }
            }
            break;
    }
}
//------------------------------------------------------------------------------
void TPLCIOThread::Execute()
{
    // Faithful golden loop -- see header note: this is NEVER invoked offline
    // (no real thread spawns it; Resume() is a no-op), so it is dead code
    // here, kept compiling/callable for a future real-thread wave.
    do
    {
        PLCIOProcess();          // golden: Synchronize(PLCIOProcess) -- no VCL
                                  // thread marshaling needed since this never
                                  // actually runs on a second thread offline.
        MySleepEx(1,true);
    } while (!Terminated);
}
//------------------------------------------------------------------------------
void TPLCIOThread::Resume()
{
    // Offline: no real OS thread to resume (see header note). A future
    // real-thread wave replaces this with an actual CreateThread + Execute().
}
//------------------------------------------------------------------------------
TMyPLC_IO_Modbus::TMyPLC_IO_Modbus()
{
    TMyPLC_IO_Modbus::PLC_IO_Members.push_back(this);
    iSlave=-1;
    iPort=-1;
    iBit=-1;
}
//------------------------------------------------------------------------------
void TMyPLC_IO_Modbus::SetPortInformation(int Card, int Port, int Bit)
{
    iSlave=Card;
    iPort=Port;
    iBit=Bit;
}
//------------------------------------------------------------------------------
bool TMyPLC_IO_Modbus::IsOn()
{
    if(iSlave>=0    && iSlave<INPUT_MAX_Slave       &&
       iPort>=PLCIO_INPUT_START_PORT_NUMBER &&
       iPort<(INPUT_MAX_REGISTER*2+PLCIO_INPUT_START_PORT_NUMBER)   &&
       iBit>=0      && iBit<8)
    {
        if(iPLCSafetyVer==202204)
            return bPLCInData[iSlave][iPort-PLCIO_INPUT_START_PORT_NUMBER+2][iBit];//joe 20220406.01 debug safe plc
        else
            return bPLCInData[iSlave][iPort-PLCIO_INPUT_START_PORT_NUMBER+1][iBit];
    }
    else
    {
        return false;
    }
}
//------------------------------------------------------------------------------
bool TMyPLC_IO_Modbus::IsOff()
{
    return !IsOn();
}
//------------------------------------------------------------------------------
void TMyPLC_IO_Modbus::On()                                                     //Output模組 暫無使用 (預留) (output module, not currently used (reserved))
{
}
//------------------------------------------------------------------------------
void TMyPLC_IO_Modbus::Off()                                                    //Output模組 暫無使用 (預留) (output module, not currently used (reserved))
{
}
//------------------------------------------------------------------------------
//Byte TMyPLC_IO_Modbus::IOInputByte()
//{
//
//}
//------------------------------------------------------------------------------
void TMyPLC_IO_Modbus::IOByteOut(int port, byte Byte)                           //Output模組 暫無使用 (預留) (output module, not currently used (reserved))
{
}
//------------------------------------------------------------------------------
void TMyPLC_IO_Modbus::InitialMyOutIOData()
{
}
//------------------------------------------------------------------------------
void bPLCStatusCheck()                                                          //kevin 20250407 check PLC
{
    PLCIOTaskCycle();
    PLCStatusCheck();                                                           //ben 20230913 add
    PlcComm.Cycle();
}
//------------------------------------------------------------------------------
//AI(W906-PLCMODEL) 20261006: EastSun 1006「PLC 要用型號分支出去」-- Gerneral.ini [System] SafePlcModel=1 = Reer MOSAIC M1S COM
//  (Modbus TCP), vendor manual "Process data mapping for M1S COM Master module V2.0" 8547780 Rev.4 (D:\HT9045\_vendor_manuals\
//  安全PLC.pdf) p.28: FC3/FC4 System I/O at registers 0x400-0x410 (17 registers); each register carries two bytes, the LOW byte
//  being the even-numbered one:
//      0x400       lo = System status, hi = reserved
//      0x401-0x408 Input status bytes 0-15        0x409-0x40A Restart input bytes 0-3
//      0x40B-0x40C Fieldbus feedback bytes 0-3    0x40D-0x40E Probe status bytes 0-3
//      0x40F-0x410 OSSD status bytes 0-3 (the manual's table draws these two high byte first; read here like the others --
//                  golden forces bIOPowered=true right after, so nothing reads them yet)
//  Golden's request (model 0: FC4 from 0x0000, 11 registers) lands on 0x000-0x001 = the host's WRITE area (FC6/FC16 "Fieldbus
//  inputs") and 0x002-0x00A, which the manual does not define -- measured on this machine 20261006: 22 zero bytes for 20 min,
//  while 0x400 x17 returned System status 0x01 and Input bytes 0x55 0x15 0x10 0x05 (control: CRC/date at 0x620 non-zero).
//  This lays the block into golden's byte array so EVERYTHING downstream is golden's, unchanged:
//      InPortData_Byte[slave][0]      = System status
//      InPortData_Byte[slave][1+n]    = Input status byte n (n = 0..15)  -> golden's "+1" index (MyLaneIo.cpp IOInputBit,
//                                       TMyPLC_IO_Modbus::IsOn, iPLCSafetyVer != 202204) maps IO_Table Port 0x300+n to Input byte n
//      InPortData_Byte[slave][17..19] = 0
//      InPortData_Byte[slave][20..21] = OSSD status bytes 0..1 -> golden's j>=20 branch fills bOSSDData[slave][0..1]
//  golden's bit split (the loop after this call in PLCIOTaskCycle case 100) then fills bPLCInData / bOSSDData as before.
//  ModbusTCPClient::GetRecevie hands the register bytes as sent on the wire (high byte first per register).
static void W906_PlcM1SFill(int slave, const VTBYTEDATA& data)
{
    if(slave<0 || slave>=INPUT_MAX_Slave)
        return;
    unsigned char m[34]={0};
    for(int i=0; i<(int)data.size() && i<34; i++)
        m[(i/2)*2 + ((i%2)==0 ? 1 : 0)]=data[i];                              // wire hi,lo -> manual byte order lo,hi
    for(int j=0; j<INPUT_MAX_REGISTER; j++)
        InPortData_Byte[slave][j]=0;
    InPortData_Byte[slave][0]=m[0];                                             // System status
    for(int n=0; n<16 && 1+n<20; n++)
        InPortData_Byte[slave][1+n]=m[2+n];                                     // Input status byte n
    InPortData_Byte[slave][20]=m[30];                                           // OSSD status byte 0
    InPortData_Byte[slave][21]=m[31];                                           // OSSD status byte 1
    W906_PlcM1SAllDoors(slave);                                                 // AI(W906-PLCDOOR) 20261006: byte 17 bit 0 = all PLC safety doors closed (end of file)
}
//------------------------------------------------------------------------------
//AI(W906-PLCDOOR) 20261006: EastSun 1006「門警報不用看其他地方 看PLC通訊就好 請把IO_TABLE 做修正 寫出對應rear PLC的設定 並且功能要齊全」.
//  Golden's SafePlcIO=1 needs an "all safety doors closed" input from the PLC (Sen[SnAllSafeDoor]): bCheckPLCAllSafedoorAndEMGEnable
//  (csystem.cpp) refuses to run -- SystemStart=false + StopAllMotor every pass -- unless SnAllSafeDoor and SnAllEMG are both enabled
//  (or SnSafeMode is), and CheckSafeDoorIsClosed raises WAR16125 from it. This machine's MOSAIC project has one input per door and
//  no combined door bit (every Input status bit mapped 20261006 by EastSun opening the doors one by one), so model 1 makes it here:
//      InPortData_Byte[slave][17] bit 0 (IO_Table Port 310 / Bit 0, ISABase 4) = 1 only when EVERY door of golden's iSafeDoor[] list
//      that the IO table puts on the PLC (ISABase 4, i.e. force-enabled by cinitial.cpp under SafePlcIO=1) reads 1 (closed) in this
//      same frame. No PLC door configured -> 0 (fail safe: WAR16125, never a silent "closed").
//  The per-door alarms (MES1611+N-1 at the door's position) are golden's, unchanged; this only feeds golden's generic check.
#include "mysensor.h"
void W906_PlcM1SAllDoors(int slave)
{
    if(slave<0 || slave>=INPUT_MAX_Slave)
        return;
    int doors=0, open=0;
    for(int i=0; i<MAX_SAFE_DOOR_CNT; i++)
    {
        const int s=iSafeDoor[i];
        if(s<0 || s>=MAX_SENSOR_ITEM || Sen[s].ISABase!=ePLCbase)
            continue;
        const int j=Sen[s].Port-PLCIO_INPUT_START_PORT_NUMBER+1;               // the same index golden's IOInputBit reads (iPLCSafetyVer != 202204)
        if(j<1 || j>16 || Sen[s].Bit<0 || Sen[s].Bit>7)
        {   ++open; continue; }                                                 // a door outside the 16 Input status bytes counts as open
        ++doors;
        if((InPortData_Byte[slave][j] & MyBitMask[Sen[s].Bit])==0)
            ++open;
    }
    InPortData_Byte[slave][17]=(doors>0 && open==0) ? 0x01 : 0x00;
}
