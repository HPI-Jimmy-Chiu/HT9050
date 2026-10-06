//----------------------------------------------------------------------------
#ifndef MyBinDispH
#define MyBinDispH
//----------------------------------------------------------------------------

#include <Classes.hpp>
#include "SPComm.hpp"
//#include "HTimer.h"
#include "MachineType.h"
#include "myTimer.h"
enum eBinDispStatus                                                             //Sam 20240604 : 新增 BinDisplay TFT
{
    eBDP_Initial    =0,
    eBDP_GetStatus  =1,
    eBDP_ColorSet   =2,
    eBDP_BinSet     =3,
    eBDP_BinRun     =4,
    eBDP_DispErr    =5
};
//----------------------------------------------------------------------------
class TDataModule3 : public TDataModule
{
__published:
    TComm *BinDisp;
    TComm *BinDisp2;
    void __fastcall DataModuleDestroy(TObject *Sender);
private:
public:
    virtual __fastcall TDataModule3(TComponent* AOwner);
};
//----------------------------------------------------------------------------
extern PACKAGE TDataModule3 *DataModule3;
//----------------------------------------------------------------------------
class TMyBinDispCtrl
{
private :

protected:  // User declarations
    int    Addr;
    int    MagAddr;                                                             //JerryYang 20230515 : Magazine BIN DISP
    TTimer *Timer1;

    TQPF_Timer BinDisDelay;
    TQPF_Timer BinDisCycleDelay;

    void __fastcall Timer1Timer(TObject *Sender);
    void __fastcall CommBinReceiveData(TObject *Sender, Pointer Buffer, WORD BufferLength);
    void __fastcall CommBinReceiveData2(TObject *Sender, Pointer Buffer, WORD BufferLength);
    bool  bHasUnitArray[MAX_BIN_UNIT];                                          // 個別的顯示器是否有安裝
    bool  bHasUnit;                                                             // 如果都沒有裝顯示器就會=false
    bool  bSliding[MAX_BIN_UNIT];                                               // 判斷該位置是否需要跳號
    bool  bSliding_Cnt[MAX_BIN_UNIT];                                           //Sam 20240604 : 新增 BinDisplay TFT
    bool  bStopProcess;                                                         // 是否要停止Timer
    bool  bSetBin[MAX_BIN_UNIT];                                                // 那些顯示器要開始設定Bin
    int   iSetBin[MAX_BIN_UNIT][TEST_MAX_BIN];                                  // 顯示器要設定的Bin            //Steven 20140402 : Fixed 記憶體破壞
    bool  bSetColor[MAX_BIN_UNIT];                                              // 那些顯示器要開始設定Bin
    bool  bGetStatus[MAX_BIN_UNIT];
    int   iSetColor[MAX_BIN_UNIT];                                              // 顯示器要設定的Color
    bool  bStartSetBin;                                                         // 開始進行設定顯示器Bin
    bool  bStartSetColor;                                                       // 開始進行設定顯示器Color
    double dDelaySec;                                                           // Bin輪替的時間    //Sam 20240604 : 顯示器輪巡時間改為 double
    AnsiString sUnitName[MAX_BIN_UNIT];                                         // 紀錄 Log 使用    //Sam 20240604 : 新增 BinDisplay TFT
    int iSetCount[MAX_BIN_UNIT];                                                // 顯示 IC Count 使用
    int iCountTFT[MAX_BIN_UNIT];                                                // Bin輪巡使用
    bool bStartOnce;                                                            // 開始進行設定TFT顯示器設定
    bool bStartCycle;                                                           // 開始進行 TFT 顯示器
    int   iVersion[MAX_BIN_UNIT];                                               // 顯示器的版本
    int   iBinNow[MAX_BIN_UNIT];                                                // 顯示器目前的Bin
    int   iColorNow[MAX_BIN_UNIT];                                              // 顯示器目前的的Color
    bool  bHasError[MAX_BIN_UNIT];                                              // 顯示器是否有出錯
    int   iRusStatus;                                                           // 顯示目前在執行的動作

    char SendBuffer[1024];                                                      // 欲對顯示器通訊之資料
    bool BinDispRecv;                                                           // 有從顯示器通訊之 Comm Port收到一筆資料
    bool BinDispRecv2;                                                          // 有從顯示器通訊之 Comm Port收到一筆資料
    char BinDispCom2Buffer[1024];                                               // 有從顯示器通訊之 Comm Port收到一筆資料之內容
    char BinDispCom2Buffer2[1024];                                              // 有從顯示器通訊之 Comm Port收到一筆資料之內容
    AnsiString  ComPort;                                                        // 顯示器通訊之 Comm Port
    AnsiString  ComPort2;                                                       // 顯示器通訊之 Comm Port
    TParity ComParity;                                                          // 顯示器通訊之 Parity
    unsigned char Com2ReceiveByte[1024];
    int Receivelen;
    int iOnceTask;                                                              //Sam 20240604 : 新增 BinDisplay TFT
    int iOnceTFTTask;
    int iCycleTask;
    int iCycleTFTTask;
    int iStartSetBinTask;
    int iStartSetColorTask;
    int iStartGetStatusTask;
    int iBinDispCtrlTask;
    int iTotalInstalledUnit;

    //虛擬函式，需要在子類別實做
    virtual void WriteBin  (int Addr, int Command, short value)=0;              // 對address=Index顯示器送出欲設定之Bin
    virtual void WriteColor(int Addr, short value)=0;                           // 對address=Index顯示器送出欲設定之顏色
    virtual void WriteBin2  (int Addr, int Command, short value)=0;             // 對address=Index顯示器送出欲設定之Bin
    virtual void WriteColor2(int Addr, short value)=0;                          // 對address=Index顯示器送出欲設定之顏色

    virtual void ReadVersion(int Addr)=0;                                       // 讀取顯示器的狀態
    virtual void ReadVersion2(int Addr)=0;                                      // 讀取顯示器的狀態
    virtual bool DoStartSetBin()    {return false;};                            // 開始Bin的顯示
    virtual bool DoStartSetColor()  {return false;};                            // 開始設定顯示器顏色
    virtual bool DoStartGetStatus() {return false;};                            // 確認顯示器的狀態
    virtual void InitialTask()=0;                                               //Sam 20240604 : 新增 BinDisplay TFT
    virtual bool DoOnce()                               {return false;};
    virtual bool DoOnceTFT(bool bReset, int iSetType)   {return false;};
    virtual bool DoCycle()                              {return false;};
    virtual bool DoCycleTFT(bool bReset, int iWriteType){return false;};

public:
    TMyBinDispCtrl();
    ~TMyBinDispCtrl();
    AnsiString Alias[MAX_BIN_UNIT];
    TComm  *CommBin;
    TComm  *CommBin2;
    void ProcessStopStart(bool Value)  ;                                        // 啟動或停止跳號
    void SetComPort(AnsiString port)   ;                                        // 設定顯示器群組是利用那一個COM PORT
    void SetComPort2(AnsiString port)  ;                                        // 設定顯示器群組是利用那一個COM PORT
    void SetComParity(TParity Parity)  ;                                        // 設定顯示器群組是利用那一個Parity
    bool UnitHasInstall(int Index)     ;                                        // 確認該顯示器使是否有啟用
    void CloseUnit(int Index)          ;                                        // 關閉該顯示器
    void OpenUnit(int Index)           ;                                        // 開啟該顯示器
    void SetDelayTime(double Sec)      ;                                        // 設定多久輪替一次 //Sam 20240604 : 顯示器輪巡時間改為 double
    double  GetDelayTime()             ;                                        // 取得輪替的時間
    int  GetTotalInstalledUnit()       ;                                        // 取得總共安裝了幾個顯示器
    int  GetColorNow(int Index)        ;                                        // 顯示器目前的顏色
    int  GetBinNow(int Index)          ;                                        // 顯示器目前的字元
    bool GerErrNow(int Index)          ;                                        // 顯示器是否有出錯
    AnsiString GetRunStatus()          ;                                        // 回傳目前在執行的動作

    void InstalledUnit(int Index);                                              // 設定address=Index 的顯示器是有存在的
    void WriteTargetBin(int Index, int *bin, int color);                        // 將address=Index顯示器設定文字與顏色
    void WriteTargetCount(int Index, int iCount);                               // 設定 IC 數量 //Sam 20240604 : 新增 BinDisplay TFT
    bool InitialOK;                                                             //判斷主控端是否已經準備完成
    bool bFirstInit;
    void ShowCommLog(unsigned char ucLog[], int Address, AnsiString sFun);      //Sam 20240604 : 新增 BinDisplay TFT
    void command_TFT_Input(char *cStr, int index, int iDisplabel, AnsiString sValue);
    void command_TFT_Font (char *cStr, int index, int iDisplabel, Byte iXPos, Byte iYPos, Byte iWidth, Byte iHeigh, Byte iFontSize, Byte iFill, int iColor);
    TMyStringList *slBinDispLog;                                                //Steven 20210910 : Bin顯示器的log
    AnsiString sReadBuffer;                                                     //kevin 20170711 (Steven) fix define
    AnsiString sReadBuffer2;                                                    //kevin 20170711 (Steven) fix define
    AnsiString sReadBufferMag;
    
    TQPF_Timer FlashTimer[MAX_BIN_UNIT];                                        //Eastsun 20260513 : 閃爍功能
    void ProcessFlash(void);                                                    //Eastsun 20260513 : 閃爍功能
    

    //==> Eastsun 20260513 KYEC: AutoChanging flash public API
//    void StartFlash(int Index, int colorA, int colorB, int periodMs);
    //void StopFlash(int Index);

    void ClearAutoChangingWarn(int AutoTarget);
    void FlashPro(int AutoTarget);
    //<== Eastsun 20260513
};
//----------------------------------------------------------------------------
class TMyBinDispHT9046:public TMyBinDispCtrl
{
    private:
    protected:
        virtual void WriteBin(int Addr, int Command, short Value);
        virtual void WriteBin2(int Addr, int Command, short Value);
        virtual void MagazineWriteBin_HTA18(int index,int ivalue);              //JerryYang 20230515 : 二位數BIN DISP, HTA18
        virtual void MagazineWriteBin_BT008(int index,int ivalue);              //JerryYang 20230515 : 三位數BIN DISP, HT-BT008
        virtual void MagazineWriteBin_TFT(int index,int ivalue, bool bmulti=false);   //JerryYang 20231106 : TFT
        virtual void MagazineWriteBinFont_TFT(int index, int iColor, bool binitial=false, bool bmulti=false, int iBinNoOverride=-1);   //Eastsun 20260825 : Mag Bin RGB   //JerryYang 20231106 : TFT
        virtual void WriteColor(int Addr, short Value);
        virtual void WriteColor2(int Addr, short Value);
        virtual void ReadVersion(int Addr);
        virtual void ReadVersion2(int Addr);
        virtual void ReadVersion_TFT(int index);
        virtual bool DoStartSetBin();
        virtual bool DoStartSetColor();
        virtual bool DoStartGetStatus();
        virtual void InitialTask();                                             //Sam 20240604 : 新增 BinDisplay TFT
        virtual void SetNoBackGround_TFT(int index);                            //清空背景
        virtual void SetBackGround_TFT(int index);                              //設定背景
        virtual void SetFontBin_TFT(int index, int iColor, int iValue);         //設定 Bin 別字型
        virtual void SetFontBinWord_TFT(int index, int iColor, int iValue);     //設定 "Bin" 字型
        virtual void SetFontEA_TFT(int index, int iColor, int iValue);          //設定 "EA" 字型
        virtual void SetFontCount_TFT(int index, int iColor, int iValue);       //設定 "Count" 字型
        virtual void WriteBinWord_TFT(int index,int ivalue);                    //顯示 "Bin"
        virtual void WriteEA_TFT(int index,int ivalue);                         //顯示 "EA"
        virtual void WriteCount_TFT(int index,int ivalue, int iCount);          //顯示 Count 數量
        virtual void WriteBin_TFT(int index,int ivalue);                        //顯示Bin別 By TFT
        virtual bool DoOnce();
        virtual bool DoOnceTFT(bool bReset, int iSetType);
        virtual bool DoCycle();
        virtual bool DoCycleTFT(bool bReset, int iWriteType);
};
#endif
