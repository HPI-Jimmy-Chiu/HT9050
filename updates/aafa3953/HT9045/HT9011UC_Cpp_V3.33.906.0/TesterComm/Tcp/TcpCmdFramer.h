// ===========================================================================
//  TesterComm/Tcp/TcpCmdFramer.h -- W10 R2: where the bytes the Handler's TCP command server (7016) receives
//  become commands.  AI(W906-W10) 20260927 (St02-E).  Pure C++ (no fMain, no socket): the pump
//  (TesterComm/Tcp/CmdServerPump.cpp) appends what each connection read and takes complete commands out.
//
//  golden (906_0625_Steven Command.cpp:12762-12805)          here
//  -------------------------------------------------------  ---------------------------------------------------------
//  one OnClientRead = one command: `char EthernetBuffer[100]` a buffer per connection; every read is appended
//    + ReceiveBuf(ALL waiting bytes) -- >=101 bytes overwrite   (no fixed buffer, nothing is written past anything)
//    the stack, exactly 100 has no NUL (deviation 1, W10 plan)
//  two commands in one read: only the first is answered,      every complete command is taken, in order, at most
//    the second's text lands in sData[2] / sData[3]              kMaxFramesPerPass per connection per pump pass
//  a command split across two reads: the halves are parsed    the halves are joined: a command ends at the comma its
//    as two commands (HTSET,322, then 5, -> writes bin 0)        id needs (FieldsFor), exactly where golden's parser
//                                                                stops reading sData[]
//  Steven 20260927 (R2): 「在 rs232 的 bcb 程式裡面，有做把命令接起來的動作，你要參考處理」 -- the golden RS232 program
//  D:\RS232Standard\RS232_Code32Bin_Rev12.13.902.0_20260410 MainForm.cpp: CommTesterReceiveData :1388-1405 appends
//  every read to `vector<Byte> ReceiveData` and loops GetAnalysisString (:3159-3217) -> DoRevCommand; a half frame
//  (no ETX yet) returns "_None_" and stays for the next read (:3179); noise returns "_CLEAR_" and DoRevCommand clears
//  the WHOLE buffer (:3708-3711); the loop cap iMaxDeal=10 is written `|| iNowDeal>iMaxDeal` (:1399, never stops at
//  10).  The protocol here has no STX / ETX: a frame is "HTGR," / "HTSET," + id + the fields that id reads.
//
//  Deviations from that program (recorded in docs/TESTERCOMM_PORT_LEDGER.md, W10):
//    * noise is dropped only up to the next "HTGR," / "HTSET," head (not the whole buffer) -- #Discard# n bytes;
//    * a frame cut short by a new head is dropped (#Incomplete#): TCP loses no bytes, so only malformed input does this;
//    * the loop takes at most kMaxFramesPerPass (10) frames with `&&` (the golden `||` bug is not carried);
//    * kCap (2048) bytes per connection: a frame longer than that, or waiting data that has grown past it without a
//      complete frame, is dropped whole with #Overflow# -- never truncated (a cut LotID or number would be a command the
//      client never sent).  golden's largest real request is HTSET,403 (text < 1023 characters, Command.cpp :13536).
//      Precedent for "log and drop, do not truncate": Handler rs232.cpp:3244-3252 / OCR.cpp:1358-1367 (> 1024 bytes).
//  Separators (CR, LF, space, TAB, NUL) between frames are skipped silently (a client that ends lines with CRLF works).
//  Not changed: the golden body still parses the frame (sData[0..3]), runs the same 98 branches, sends the same bytes.
// ===========================================================================
#ifndef TESTERCOMM_TCP_TCPCMDFRAMER_H
#define TESTERCOMM_TCP_TCPCMDFRAMER_H

#include <cstddef>
#include <map>
#include <string>
#include <vector>

namespace tcpcmd {

const size_t kCap = 2048;              // per connection (R2)
const int kMaxFramesPerPass = 10;      // per connection per pump pass (golden RS232 iMaxDeal, MainForm.cpp:1398)

// How many comma-terminated fields golden's TCPCommandServerClientRead reads for this command, the head ("HTGR" /
// "HTSET") and the id counted: "HTGR,101," = 2.  Generated from the V906 body (Command.cpp :16385-18176, the same
// branches as golden 906_0625_Steven :12762-14301) by tools/tcp_cmd_fields_census.py, which ctest
// TesterComm_TcpCmdFieldsCensus re-runs against this table.  *optionalExtra = true for HTSET,720 (3 fields; its 4th,
// OPID, is optional -- golden :13975 `if(sData[3]!="" ...)`): the 4th is taken when it is already in the buffer.
int FieldsFor(const std::string& head, const std::string& id, bool* optionalExtra);

enum EventKind
{
    kDiscard,      // bytes before a head that are not a head (noise)
    kIncomplete,   // a frame cut short by a new head
    kOverflow      // over kCap: a frame longer than kCap, or kCap bytes waiting without a complete frame
};

struct Event
{
    EventKind kind;
    size_t bytes;
};

// The step on one buffer: skips separators / noise, and returns the next complete frame (true) or false (wait for
// more bytes).  Every byte dropped on the way is reported in *events.  `buf` keeps whatever is left.
bool NextFrame(std::string& buf, std::string* frame, std::vector<Event>* events);

// One buffer per connection (key = the connection object; the pump drops it on disconnect).
class Framer
{
public:
    void Append(const void* key, const char* data, size_t n);
    bool Next(const void* key, std::string* frame, std::vector<Event>* events);
    void Drop(const void* key);
    void Clear();
    size_t Pending(const void* key) const;
    size_t Connections() const;

private:
    std::map<const void*, std::string> buf_;
};

}  // namespace tcpcmd

#endif
