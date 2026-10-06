//------------------------------------------------------------------------------
// AI(W5-MyPLC-Translate) 20260710: BCB6 -> standard C++17 translation.
// Provenance: HT9011UC_Code_V3.33.906.0_20260618/MyPLC/ModbusTCPClient.h
// Changes from BCB6 original:
//   - #include "SPComm.hpp" / <Classes.hpp> / <ScktComp.hpp> -> vclcompat
//     (vcl_compat.h for AnsiString/TObject/windows.h; ClientSocket.h for the
//     new Scktcomp::TClientSocket/TCustomWinSocket/TErrorEvent shim -- see
//     that header's PROVENANCE/SCOPED API SURFACE comment).
//   - __fastcall neutralized by vcl_compat.h (kept on the ctor/event methods
//     for a minimal, faithful diff against golden's spelling).
//   - Class layout / method surface otherwise UNCHANGED (faithful, mirrors
//     golden field-for-field and method-for-method).
//------------------------------------------------------------------------------
#ifndef ModbusTCPClientH
#define ModbusTCPClientH
#include "vclcompat/vcl_compat.h"     // AnsiString, TObject, windows.h (CRITICAL_SECTION)
#include "vclcompat/ClientSocket.h"   // Scktcomp::TClientSocket/TCustomWinSocket/TErrorEvent
#include <iostream>
#include <vector>
#include <map>

using namespace std;
typedef vector<BYTE> VTBYTEDATA;
typedef map<int, VTBYTEDATA> MAP_VTBYTEDATA;

class ModbusTCPClient
{
    private:
        int iIP;
        int iPort;
        MAP_VTBYTEDATA mapReplyData;                                            //回應資料 (reply data)
        vector<int> vtEarseData;
        TClientSocket* pClinetSocket;
        bool bConnected;
        CRITICAL_SECTION cs;

    public:
        __fastcall ModbusTCPClient();
        ~ModbusTCPClient();
        void SetScan(bool bSet);
        void Cycle();
        void SetTCPInfo(AnsiString asIP, int Port);
        bool Connect();
        bool DisConnect();
        bool IsConnected();
        bool SendData(int iID, int iSlave, int iFunctionCode, int iAddr, VTBYTEDATA data);
        bool GetRecevie(int iID, VTBYTEDATA &data, bool bRemove=true);

        void __fastcall SocketConnect(TObject *Sender, TCustomWinSocket *Socket);
        void __fastcall SocketDisConnect(TObject *Sender, TCustomWinSocket *Socket);
        void __fastcall SocketError(TObject *Sender, TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode);
        void __fastcall SocketRead(TObject *Sender, TCustomWinSocket *Socket);
};
//------------------------------------------------------------------------------
#endif
