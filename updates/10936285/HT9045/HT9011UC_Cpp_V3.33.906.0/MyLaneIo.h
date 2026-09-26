// =============================================================================
//  MyLaneIo.h  --  TLaneIO public facade (singleton IO controller)
//
//  Translated from: HT9011UC_Code_V3.33.906.0_20260618/MyLaneIo.h
//  Translation wave: W4-IO (IO HAL layer)
//  Translator: AI(W4-IO) 20260626
//
//  Key changes vs. BCB6 original:
//    - `#include "myMN200motor.h"` replaced by a local io_dims.h-equivalent
//      (IO_MAXRing/IO_MAXIP/IO_MAXPort defined in IOBackend.h) to avoid pulling
//      the full MN200 motor chain.  MAXRing/MAXIP/MAXPort are re-aliased below.
//    - `__fastcall` removed (vclcompat macro neutralizes it).
//    - `AnsiString` from vclcompat/vcl_compat.h.
//    - `byte` from windows.h (via MachineDefine.h / vclcompat).
//    - Added `TIOBackend* pIO` private member for backend dispatch.
//    - Added `void SetBackend(TIOBackend*)` for test/W6 injection.
//    - `TLaneIO::TLaneIO` (qualified ctor name from BCB6) translated as
//      unqualified `TLaneIO()` (standard C++).
//    - All 6 IO method signatures preserved verbatim (names, params, defaults).
//    - EtherCAT analog trio signatures preserved verbatim.
//    - iUseMNetIP public member preserved (used by myMN200motor.cpp Restore path).
// =============================================================================
#ifndef MyLaneIoH
#define MyLaneIoH

#include "vclcompat/vcl_compat.h"   // AnsiString, __fastcall no-op, etc.
#include <windows.h>                // byte / BYTE / DWORD
#include "IOBackend.h"              // TIOBackend + IO_MAXRing/MAXIP/MAXPort

// ---------------------------------------------------------------------------
//  MN200 device-info constants (Motor/mn200.h:183-187, READ-ONLY vendor header)
//  These are pure integer sentinels used by TLaneIO bookkeeping (iUseMNetIP[]).
//  Copied verbatim; NOT pulled from mn200.h to avoid the vendor header chain.
// ---------------------------------------------------------------------------
#ifndef DEV_INF_NO_DEV
#define DEV_INF_NO_DEV          0x00
#endif
#ifndef DEV_INF_IO_32OUT_DEV
#define DEV_INF_IO_32OUT_DEV    0x80
#endif
#ifndef DEV_INF_IO_32IN_DEV
#define DEV_INF_IO_32IN_DEV     0x84
#endif
#ifndef DEV_INF_IO_16IN_16OUT_DEV
#define DEV_INF_IO_16IN_16OUT_DEV 0x82
#endif

// ---------------------------------------------------------------------------
//  Dimension aliases  (match BCB6 names from Motor/myMN200motor.h:9-11)
// ---------------------------------------------------------------------------
#ifndef MAXRing
#define MAXRing  IO_MAXRing
#endif
#ifndef MAXIP
#define MAXIP    IO_MAXIP
#endif
#ifndef MAXPort
#define MAXPort  IO_MAXPort
#endif
static const int LANEIO_CACHE_IP = 256, LANEIO_CACHE_PORT = 32;   // AI(W906-LANEIO-CACHE) 20260926: 輸出命令快取 OutPortData／BackOutPortData（:76／:78）的維度。golden 是 [MAXRing][MAXIP=64][MAXPort=4]，裝不下 1203 的輸出位址（HT9050 表上輸出列 IP 到 84、Port 到 31）⇒ 14 組不同點位共用同一個快取位元，SW[].Status()／氣缸 GetOutBit() 會讀到別的點位。golden 缺陷，Jimmy 20260925 裁決修正（RULINGS_20260925 第 18／29 條）。MotionNet 的範圍檢查與 iUseMNetIP 仍用 golden 的 MAXIP／MAXPort
// ---------------------------------------------------------------------------
//  TLaneIO -- concrete singleton IO facade
//  Holds a TIOBackend* selected once at init (default: TSimIOBackend).
//  All 6 public IO methods keep their BCB6 names, signatures, and semantics;
//  they route vendor calls through pIO->WriteBit/ReadBit/etc.
// ---------------------------------------------------------------------------
class TLaneIO
{
private:
    unsigned int lOutPortData[128];
    unsigned int lInPortData[128];
    int  OutPos;
    int  InPos;
    int  OutPortIndex[MAXRing][MAXIP][MAXPort];
    int  InPortIndex[MAXRing][MAXIP][MAXPort];
    byte OutPortData[MAXRing][LANEIO_CACHE_IP][LANEIO_CACHE_PORT];   // AI(W906-LANEIO-CACHE) 20260926: golden [MAXRing][MAXIP][MAXPort]，見 :60
    byte InPortData[MAXRing][MAXIP][MAXPort];
    byte BackOutPortData[MAXRing][LANEIO_CACHE_IP][LANEIO_CACHE_PORT];   // AI(W906-LANEIO-CACHE) 20260926: 同 :76（BackUp／Restore 兩邊維度要一樣）

    TIOBackend* pIO;    TIOBackend* pIO1203; TIOBackend* pIOMN200; TIOBackend* pIOMnet;  // backend selected at init (default: TSimIOBackend)；AI(W906-A4-7) 20260924: golden 依點位分派三路（ePCI1203 → Acm_*、IO_CARD_TYPE 為 MN200 → mn_*、其他 → _mnet_*），各一個後端

public:
    TLaneIO();  // BCB6: __fastcall TLaneIO::TLaneIO() -- qualified name + __fastcall dropped

    void InitialMyOutIOData();
    void InitialMyUseIP();

    // Inject a different backend (used by tests and future W6 cinitial selection).
    void SetBackend(TIOBackend* backend);  void SelectVendorBackends();  void SetBackends(TIOBackend* p1203, TIOBackend* pMN200, TIOBackend* pMnet) { if(p1203) pIO1203=p1203; if(pMN200) pIOMN200=pMN200; if(pMnet) pIOMnet=pMnet; }  // AI(W906-A4-7) 20260924: 硬體初始化（InitHontechHardware 開頭）換成真後端；SOFT_SIMULTE 建置維持模擬

    // --- 6 primary IO methods (signatures preserved verbatim) ----------------
    // Sam 20230724: add PCI1203 IO support
    void IOBitOn   (int Ring, int IP, int Port, int Bit, int iISABase=0, AnsiString Alias="");
    void IOBitOff  (int Ring, int IP, int Port, int Bit, int iISABase=0, AnsiString Alias="");
    bool IOByteOut (int Ring, int IP, int Port, unsigned int Byte, int iISABase=0);
    bool IOOutBitStatus(int Ring, int IP, int Port, int Bit, int iISABase=0, AnsiString Alias="");
    bool IOInputBit(int Ring, int IP, int Port, int Bit, int iISABase, AnsiString Alias="");
    byte IOInputByte(int Ring, int IP, int Port, int iISABase=0);

    // JerryYang 20241231: used by RestoreOutputData and myMN200motor.cpp
    int iUseMNetIP[MAXRing][MAXIP];

    void SetUseIP(int Ring, int IP, int iISABase, AnsiString Alias, bool bOutPut=true);
    void BackUpOutputData();
    void RestoreOutputData();
    int  CheckPortRangeErr(bool DO_Type, int iISABase, int Ring, int IP, int Port, int Bit=999);

    // EtherCAT analog trio (Sam 20230210: VacuumUnit pressure sensor)
    // Acm bodies active only #if HAVE_PCI1203 in MyLaneIo.cpp; else return 999.0/false.
    double GetIOValue      (int Ring, int IP, int Port, int Bit, int iISABase=0, AnsiString Alias="");
    double GetIOValueThread(int Ring, int IP, int Port, int Bit, int iISABase=0, AnsiString Alias="");
    bool   SetIOValueThread(double dKpa, int Ring, int IP, int Port, int Bit, int iISABase=0, AnsiString Alias="");

    AnsiString GetIOErrStr(int iErr);
};

extern class TLaneIO MyLaneIO;

#endif // MyLaneIoH
