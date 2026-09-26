// =============================================================================
//  EtherCAT/Pci1203IoRoute.h -- the engine's IO, routed onto the PCIE-1203.
//
//  AI(W906-IOWEB-P17) 20260925: new file. User 20260925: "請幫我接上1203".
//  Weekend plan item A4-7 ("MyLaneIO 的 1203 後端 ... 接上後 C++ 流程才會讀到卡片
//  的 IO、打出去的輸出才會真的通電"), done the way the user ruled on 20260924:
//  "控制精神和底層必須按照Eastsun的" -- i.e. through EastSun's Pci1203Control.
//
//  WHAT IT CONNECTS
//      MyLaneIO.IOBitOn/IOBitOff/IOByteOut/IOInputBit/IOInputByte   (golden)
//        -> pIO1203 = TPci1203Backend  (MyLaneIo.cpp SelectVendorBackends)
//        -> IOBackend.cpp's #else arm  (ht9045_io has no HAVE_PCI1203)
//        -> TPci1203IoRoute             (IOBackend.h EOF)  <- installed here
//             reads : the 1203 monitor's DI / DO samples    (no new vendor call)
//             writes: TPci1203Control::Execute(kCmdDoSetBit / kCmdDoSetByte)
//                     = Acm_DaqDoSetBitEx(dev, ring, station, stationChan*8+bit, v)
//
//  So everything golden layers ABOVE the backend keeps working unchanged:
//  IdleCheckSafeDoorByCylinder, the OutPortData command cache, CheckPortRangeErr,
//  and every SW[]/Sen[]/Cylinder[] caller. And everything EastSun layers BELOW
//  keeps its guarantees: the vendor call sits in Pci1203Control.cpp's allowlist
//  (tools/pci1203_control_gate.ps1), the monitor stays read-only
//  (tools/pci1203_readonly_gate.ps1), and WB_PUMP_1203_CONTROL_LIVE is the ONE
//  switch that decides whether a write reaches the card -- DRY RUN otherwise.
//
//  WHAT A WRITE IS REFUSED FOR (before Execute), each with a reason on record:
//    * no armed control object / no open card
//    * no ring-attributed DO byte at (Lane, IP, Port/8)  -- ChanIo.h PickIoSample
//    * that byte did not read back (valid == false)      -- no blind write
//    * the station at (ring, IP) is a DRIVE (CiA 402, or /SERVOPACK/ in its
//      name) -- the same rule as the web page's stationIsDrive (view.js:3906),
//      moved to the side that actually issues. Ring-0 station 1 is a SERVOPACK
//      and ring-1 station 1 is a 32DO (card map 20260924): a table row with the
//      wrong Lane would otherwise write a servo's RxPDO and get SUCCESS back.
//
//  ⚠ NOT A FIX FOR GOLDEN'S OutPortData BOUNDS. The command cache is
//    byte OutPortData[4][64][4] and 1203 rows index it with IP up to 179 and
//    Port up to 31 -- out of the declared bounds (golden MyLaneIo.cpp:142, the
//    same indexing), landing inside the object for Lane 1 but aliasing 14 DO
//    pairs. That is TMySwitch::Status() / TMyCylinder::GetOutBit()'s problem,
//    not the card's: this route reads and writes the card by (ring, station,
//    channel) and never touches the cache. Recorded in the commit for a ruling.
// =============================================================================
#ifndef ETHERCAT_PCI1203IOROUTE_H
#define ETHERCAT_PCI1203IOROUTE_H

#include <string>

namespace ht9045 {

// The most recent write that went through the route -- what the web click
// reports back, because MyLaneIO.IOBitOn() returns void and golden has nowhere
// to put the answer.
struct Pci1203RouteWrite {
    unsigned long seq;          // 0 = nothing written through the route yet
    bool          byte;         // IOByteOut (kCmdDoSetByte) rather than a bit
    int           ring;         // as ASKED: IO_Table Lane
    int           station;      //           IP
    int           port;         //           Port (bit: channel in station; byte: byte in station)
    int           value;        //           0/1, or the byte
    int           slot;         // the monitor DO slot it resolved to, -1 = none
    int           stationChan;  // byte within the station, -1 = none
    bool          reached;      // got as far as TPci1203Control::Execute
    bool          accepted;
    bool          issued;       // a vendor call was made (false in DRY RUN)
    bool          dryRun;
    unsigned long ret;
    std::string   why;          // refusal / vendor error / "dry run: ..."
    std::string   exCall;       // the Ex call LIVE issues, spelled out (see .cpp)

    Pci1203RouteWrite()
        : seq(0), byte(false), ring(-1), station(-1), port(-1), value(0)
        , slot(-1), stationChan(-1), reached(false), accepted(false)
        , issued(false), dryRun(true), ret(0) {}
};

const Pci1203RouteWrite& Pci1203RouteLastWrite();
bool Pci1203RouteInstalled();

// Everything a bit write checks BEFORE Execute, without writing: lets a caller
// refuse with the reason instead of finding out from a void function.
// `slot` / `stationChan` are filled when it returns true.
bool Pci1203RouteCanWriteBit(int ring, int station, int port,
                             int* slot, int* stationChan, std::string& why);

// Who is writing, for the console line. The web click sets "web" around its
// IOBitOn so its line is not printed twice; everything else is "engine".
void Pci1203RouteSetSource(const char* source);

}  // namespace ht9045

// Called once from wb_serve after the 1203 control block (global name, declared
// inline at the call site like W906_BootReadLotSummary).
void W906_InstallPci1203IoRoute();

#endif  // ETHERCAT_PCI1203IOROUTE_H
