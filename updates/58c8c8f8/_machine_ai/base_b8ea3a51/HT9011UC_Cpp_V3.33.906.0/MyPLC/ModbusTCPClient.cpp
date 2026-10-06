//------------------------------------------------------------------------------
// AI(W5-MyPLC-Translate) 20260710: BCB6 -> standard C++17 translation.
// Provenance: HT9011UC_Code_V3.33.906.0_20260618/MyPLC/ModbusTCPClient.cpp
// Changes from BCB6 original:
//   - Dropped #include "MachineDefine.h" / #pragma hdrstop / #include
//     "MyPLC_IO_Modbus.h" / #pragma package(smart_init): Borland-specific /
//     not needed by this .cpp (MyPLC_IO_Modbus.h is the OTHER direction of
//     the golden mutual-include; nothing in this file needs it).
//   - Event assignment (`pClinetSocket->OnConnect = SocketConnect;` etc.):
//     BCB6's __closure event-property assignment implicitly binds `this` to
//     a non-static member function -- standard C++ has no equivalent syntax.
//     Translated to a `this`-capturing lambda forwarding to the named method
//     (closest modern-C++ analogue of a bound-method closure); the named
//     handler methods (SocketConnect/SocketDisConnect/SocketError/SocketRead)
//     are otherwise UNCHANGED, so this is the minimal necessary edit.
//   - All other logic (Cycle/Connect/DisConnect/SendData/GetRecevie/handlers)
//     is a FAITHFUL, near-verbatim translation, including the disabled
//     (`//` / `/* */`) BCB6 dead-code comments and the "JerryYang 20250206 :
//     fix" quirk (bConnected is NOT set true right after Active=true; it is
//     only set by the SocketConnect callback -- preserved as-is).
//------------------------------------------------------------------------------

#include "vclcompat/vcl_compat.h"

#include "ModbusTCPClient.h"
//------------------------------------------------------------------------------
__fastcall ModbusTCPClient::ModbusTCPClient()
{
    //AI(W906-O3) 20260807: DELIBERATE DEVIATION FROM GOLDEN -- golden
    //  MyPLC/ModbusTCPClient.cpp:10-20 initialises NONE of the three scalar
    //  members (ModbusTCPClient.h:23 `bool bConnected;` and the two ints), so a
    //  freshly constructed client answers IsConnected() from whatever the
    //  allocation happened to contain.  It is not a port artefact: the golden
    //  ctor is reproduced above line for line and it has the same hole.  It
    //  stayed invisible because every unoptimised build this port ever made --
    //  and, in the BCB6 product, every instance that lives in a zero-initialised
    //  static -- happened to hand out zeroed memory.
    //  Under -DCMAKE_BUILD_TYPE=Release the memory is no longer zero and
    //  tests/test_myplc_modbus.cpp:250 fails: "client3 starts disconnected"
    //  (51 passed / 1 failed at -O3, 52/0 at -O0, same source).
    //  Reproducing undefined behaviour faithfully is not reproducing behaviour,
    //  so this is fixed rather than mirrored.  false/0/0 are the only values
    //  consistent with the rest of golden: bConnected is set true ONLY by the
    //  SocketConnect callback (:187) and false by SocketDisConnect (:192) and
    //  SocketError (:199), and iIP/iPort are meaningless until SetTCPInfo runs.
    //  This should be reported upstream against the BCB6 tree as well.
    iIP=0;
    iPort=0;
    bConnected=false;

    pClinetSocket=new TClientSocket(NULL);

    //AI(W5-MyPLC-Translate) 20260710: BCB6 __closure event assignment implicitly
    //  binds `this`; standard C++ has no such syntax, so each handler is wired
    //  via a `this`-capturing lambda forwarding to the unchanged named method.
    pClinetSocket->OnConnect       = [this](TObject *Sender, TCustomWinSocket *Socket){ SocketConnect(Sender, Socket); };
    pClinetSocket->OnDisconnect    = [this](TObject *Sender, TCustomWinSocket *Socket){ SocketDisConnect(Sender, Socket); };
    pClinetSocket->OnError         = [this](TObject *Sender, TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode){ SocketError(Sender, Socket, ErrorEvent, ErrorCode); };
    pClinetSocket->OnRead          = [this](TObject *Sender, TCustomWinSocket *Socket){ SocketRead(Sender, Socket); };

    InitializeCriticalSection(&cs);
}
//------------------------------------------------------------------------------
ModbusTCPClient::~ModbusTCPClient()
{
    DisConnect();
    if(pClinetSocket!=NULL)
    {
        delete pClinetSocket;
    }
}
//------------------------------------------------------------------------------
void ModbusTCPClient::SetScan(bool bSet)
{
    if(bSet)
    {
        Connect();
    }
    else
    {
        DisConnect();
    }
}
//------------------------------------------------------------------------------
void ModbusTCPClient::SetTCPInfo(AnsiString asIP, int Port)
{
    pClinetSocket->Address=asIP;
    pClinetSocket->Port=Port;
}
//------------------------------------------------------------------------------
bool ModbusTCPClient::Connect()
{
    try
    {
        pClinetSocket->Active=true;
//        bConnected=true;                                                      //JerryYang 20250206 : fix
    }
    catch(...)
    {
        bConnected=false;
        return false;
    }
    return bConnected;
}
//------------------------------------------------------------------------------
void ModbusTCPClient::Cycle()
{
    vector<int>::iterator it;
    if(vtEarseData.size()>0)
    {
        for(it=vtEarseData.begin(); it!=vtEarseData.end(); it++)
        {
            if(mapReplyData.find((*it))!=mapReplyData.end())
            {
                EnterCriticalSection(&cs);
                mapReplyData.erase(mapReplyData.find((*it)));
                vtEarseData.erase(it);
                LeaveCriticalSection(&cs);
                return;
            }
        }
    }
}
//------------------------------------------------------------------------------
bool ModbusTCPClient::DisConnect()
{
    try
    {
        pClinetSocket->Active=false;
        bConnected=false;
        if(pClinetSocket!=NULL)
            pClinetSocket->Close();
        return true;
    }
    catch(...)
    {
        return false;
    }
}
//------------------------------------------------------------------------------
bool ModbusTCPClient::IsConnected()
{
//    return pClinetSocket->Socket->Connected;                                  //JerryYang 20250206 : fix
    return bConnected;
}
//------------------------------------------------------------------------------
bool ModbusTCPClient::SendData(int iID, int iSlave, int iFunctionCode, int iAddr, VTBYTEDATA data)
{
    BYTE* ByteData;
    int iSizeCount;
    int iDataSize=data.size();
    if(IsConnected()==false || iDataSize<=0)
    {
        return false;
    }
    else
    {
        iSizeCount=iDataSize+10;
        ByteData=new BYTE[iSizeCount];

        //*************Header*************
        ByteData[0]=HIBYTE(iID);                                                //識別碼 (identifier)
        ByteData[1]=LOBYTE(iID);
        ByteData[2]=0;                                                          //通訊方式 0:Modbus TCP (comm mode 0:Modbus TCP)
        ByteData[3]=0;
        ByteData[4]=0;                                                          //Length
        ByteData[5]=iDataSize + 4                                               /*Targer Parameter:: Address:1 Byte  Function Code:1 Byte Start register addr:2 Byte*/;
        //*************Header*************
        ByteData[6]=iSlave;                                                     //Slave Address
        ByteData[7]=iFunctionCode;                                              //Function Code
        ByteData[8]=HIBYTE(iAddr);                                              //Start register addr
        ByteData[9]=LOBYTE(iAddr);

        for(int i=0; i<iDataSize; i++)
        {
            ByteData[i+10]=data[i];
        }
        try
        {
            pClinetSocket->Socket->SendBuf(ByteData, iDataSize+10);
        }
        catch(...)
        {}
        delete []ByteData;
        return true;
    }
}
//------------------------------------------------------------------------------
bool ModbusTCPClient::GetRecevie(int iID, VTBYTEDATA &data, bool bRemove)
{
    if(mapReplyData.size()==0)
        return false;

    if(mapReplyData.find(iID)==mapReplyData.end())
    {                                                                           //還沒收到或沒回應 (not yet received or no reply)
        return false;
    }
    else
    {
        data=mapReplyData[iID];
        if(bRemove)                                                             //刪除 (delete)
            vtEarseData.push_back(iID);                                         //刪除動作隔開避免執行緒互搶 (defer the delete so threads don't race)
        return true;
    }
}
//------------------------------------------------------------------------------
void __fastcall ModbusTCPClient::SocketConnect(TObject *Sender, TCustomWinSocket *Socket)
{                                                                               //預留 (reserved)
    bConnected=true;
}
//------------------------------------------------------------------------------
void __fastcall ModbusTCPClient::SocketDisConnect(TObject *Sender, TCustomWinSocket *Socket)
{                                                                               //預留 (reserved)
    bConnected=false;
}
//------------------------------------------------------------------------------
void __fastcall ModbusTCPClient::SocketError(TObject *Sender, TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode)
{
    Socket->Disconnect(Socket->RemotePort);
    Sleep(1000);
    bConnected=false;
    ErrorCode=0;
//    Connect();
}
//------------------------------------------------------------------------------
void __fastcall ModbusTCPClient::SocketRead(TObject *Sender, TCustomWinSocket *Socket)
{
    BYTE* ByteData;
    vector<BYTE> data;
    int iID;
    int iDataLength=Socket->ReceiveLength();
    if(iDataLength<=6)
    {                                                                           //長度不足 (length insufficient)
        return;
    }
    int iDataSize;
    int iTextDataSize;
    ByteData = new BYTE[iDataLength];
    Socket->ReceiveBuf( ByteData , iDataLength );

    iDataSize=ByteData[4]*256+ByteData[5];                                      //檢查資料長度 (check data length)
    if(iDataLength-6/*Header*/==iDataSize)                                      //長度正確 (length correct)
    {
        iTextDataSize=ByteData[8];                                              //內文Size (payload size)

        if(iDataLength-9 ==iTextDataSize)
        {
            iID=ByteData[0]*256+ByteData[1];

            for(int i=9; i<iDataLength; i++)
                data.push_back(ByteData[i]);

            if(mapReplyData.find(iID)==mapReplyData.end())                      //沒重複 (no duplicate)
            {
                EnterCriticalSection(&cs);
                mapReplyData.insert(make_pair(iID,data));                       //加入 (insert)
                LeaveCriticalSection(&cs);
            }
        }
    }
    delete []ByteData;
}
//------------------------------------------------------------------------------
