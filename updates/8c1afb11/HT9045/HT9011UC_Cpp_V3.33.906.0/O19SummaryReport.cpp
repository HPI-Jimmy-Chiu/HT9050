// =============================================================================
//  O19SummaryReport.cpp -- AI(W906-O19-C4) 20261002 (St02-E helper): card ST02-C4 (todo D-004 (1)).  St02's file.
//
//  golden 906_0625_Steven TfObserver::DoProduction_Summary_Report (cObserver.cpp:4889-5058, declared cObserver.h:552;
//  V912 cObserver.cpp:5120 is identical) as a free function on `obs` (= golden `this`), and the three helpers the O19
//  block of RecordTimeData needs (golden cMyDB.cpp:433-476, V906 cMyDB.cpp:576-634).  See O19SummaryReport.h.
//
//  The body is golden :4890-5057 line for line, golden comments kept (UTF-8 here).  Every changed line says [W906] and
//  why.  The changes are text or structure only:
//    * `this` -> obs: strngrdMDBQuery / Chart2 are local pointers to the two existing members, so the body reads as golden;
//    * strngrdTemp (golden cObserver.h:293) is a hidden TStringGrid on the form (cObserver.dfm:1598-1605, 5 x 5 default,
//      Visible = False) that only this function touches (cObserver.cpp:4925-5001; whole golden tree grep) and that is
//      cleared before each use -- a function-local grid gives the same result (it only no longer lives between calls);
//    * Rows[i]->Clear() -> ClearRow(i) (vclcompat/StringGrid.h:246; no Rows[] in vclcompat);
//    * DateSeparator save / set / restore dropped (none in the port; vclcompat StrToDateTime reads Y-M-D, '-' or '/');
//    * DTStart+i -> DTStart+TDateTime(i) (vclcompat TDateTime + int is ambiguous, cpublic.cpp:461 / cMyDB.cpp:563);
//    * two RowCount statements spelled for the vclcompat proxy (no +=; proxy = proxy would copy the proxy itself);
//    * asProduct_LoaderPath read through the A4 ctest seam W906_PRODLOADER_ROOT (cMyDB.cpp:477); unset = golden global.
//
//  Data in the port (CSV only: user rulings 20260923 / 20260926, cMyDB.cpp:113-118, docs/CMYDB_PORT_LEDGER.md:12-20):
//    * sections 1 / 2 (jam list, jam statistics): golden queries handler.db through MyDBVProcess.  With bUseMDB==false
//      golden never opens the DB (cMyDB.cpp:116-130), sqlite3_get_table(NULL, ...) leaves rows 0 and MyDBVProcess writes
//      "No Record!!" (cMyDB.cpp:1392-1398).  The port's CSV-only MyDBVProcess (V906 cMyDB.cpp:1706-1711) is that result.
//    * section 3 (Data Period, Run Time/H, Total Loader, Total Count, Down Time, MTTR): golden reads the hourly
//      Production_Loader INI files that RecordTimeData writes (golden cMyDB.cpp:423-430, V906 cMyDB.cpp:566-573).
// =============================================================================
#include "O19SummaryReport.h"
#include "cMyDB.h"      // MyDBVProcess (CSV build: golden's bUseMDB==false rows==0 result, cMyDB.cpp:1706-1711)
#include "common.h"     // ReadIniData, asProduct_LoaderPath (common.cpp:314)
#include "cpublic.h"    // ConvertSecondToSPC (cpublic.cpp:165)
#include "Config.h"     // IniConfig.asO19_SavePath (Config.h:1370)
#include <cstdlib>
#include <string>
#include <windows.h>    // GetComputerNameExA / GetComputerNameA: kernel32 only (no ws2_32, no new DLL import)

// golden asProduct_LoaderPath (common.cpp:314); the same env seam as cMyDB.cpp:477 W906ProductLoaderDir (A4), but the
// fallback is the golden global itself (this file is not compiled into test_ga1_cmydb)
static AnsiString W906O19_ProductLoaderDir()
{
    const char* e = std::getenv("W906_PRODLOADER_ROOT");
    return (e != 0 && *e != 0) ? AnsiString(e) : asProduct_LoaderPath;
}

// golden IniConfig.asO19_SavePath (golden cMyDB.cpp:444; [Event Log] asO19_SavePath, cConfiguration.cpp:4073-4078)
AnsiString W906O19_SaveDir()
{
    const char* e = std::getenv("W906_O19_ROOT");
    return (e != 0 && *e != 0) ? AnsiString(e) : IniConfig.asO19_SavePath;
}

// golden cConfiguration.cpp:121-127: WSAStartup, gethostname(HostName, 80), edtN04_Host->Text=HostName.  Done the
// EventLogAnalysis/ElaFtp.cpp:211-221 way: GetComputerNameExA(ComputerNameDnsHostname) is the name Winsock returns,
// without pulling ws2_32 into ht9045_db.  (A DNS host label is at most 63 bytes, so golden's 80-byte buffer never cuts it.)
AnsiString W906O19_HostName()
{
    char b[256];
    DWORD n = sizeof(b);
    if (::GetComputerNameExA(ComputerNameDnsHostname, b, &n) && n > 0)
        return AnsiString(std::string(b, n).c_str());
    n = sizeof(b);
    if (::GetComputerNameA(b, &n) && n > 0)
        return AnsiString(std::string(b, n).c_str());
    return AnsiString("");
}

// golden TDateTime::DayOfWeek() = BCB SysUtils.DayOfWeek(D): DateTimeToTimeStamp(D).Date mod 7 + 1, Date = 693594 +
// whole days since 1899-12-30 (a Saturday -> 7).  1 = Sunday .. 7 = Saturday.  (BCB rounds D to the millisecond first;
// this truncates, which differs only within half a millisecond of midnight.)
int W906O19_DayOfWeek(const TDateTime &dt)
{
    return (693594 + (int)dt.Val()) % 7 + 1;
}

//---------------------------------------------------------------------------
void W906O19_DoProduction_Summary_Report(TfObserver *obs, AnsiString asStartData, AnsiString asStartTime, AnsiString asEndData, AnsiString asEndTime)  //Sam 20210107 : Summary Report fuction
{
    TfObserverGrid *strngrdMDBQuery=obs->strngrdMDBQuery;                       // [W906] golden `this->strngrdMDBQuery` (cObserver.h:173): free function on obs, see the file head
    TfObserverChart *Chart2=obs->Chart2;                                        // [W906] golden `this->Chart2` (cObserver.h:174), as the line above
    vclcompat::TStringGrid strngrdTempGrid(5, 5);                               // [W906] golden member strngrdTemp (cObserver.h:293, cObserver.dfm:1598-1605: hidden, 5 x 5): used only here and cleared before each use -- a local grid, see the file head
    vclcompat::TStringGrid *strngrdTemp=&strngrdTempGrid;                       // [W906] as the line above
    int i, j;
    AnsiString asQuery, WhereQuery;
    int iRowNullSpace=2;
    int iStartRow2=0;
    int iStartRow3=0;
    int iAllJamCount=0;
    TDateTime DTStart, DTEnd;
    int iDay;
    // [W906] golden cObserver.cpp:4899 `char DateSeparatorOld=DateSeparator;` -- no DateSeparator in the port (vclcompat StrToDateTime always reads Y-M-D)
    AnsiString asYear,asData,asFilePath;
    int iLoader,iLoaderSum,iProductTime,iProductTimeSum;
    double dTime;
    int iDownTimeSum=0;
    AnsiString asTime;
    AnsiString asFileTime[2]={"0800-2000","2000-0800"};
    strngrdMDBQuery->Visible=true;
    Chart2->Visible=false;

    for(i=0; i<strngrdMDBQuery->RowCount; i++)
    {
        strngrdMDBQuery->ClearRow(i);                                           // [W906] golden cObserver.cpp:4911 Rows[i]->Clear() -- vclcompat::TStringGrid has no Rows[]; ClearRow (StringGrid.h:246) clears the row
    }
    strngrdMDBQuery->ColCount=7;
    strngrdMDBQuery->DefaultColWidth=70;

    strngrdMDBQuery->ColWidths[0]=70;   //No                        /Data Period
    strngrdMDBQuery->ColWidths[1]=70;   //UnitName                  /Start Time
    strngrdMDBQuery->ColWidths[2]=70;   //AlarmCode                 /End Time
    strngrdMDBQuery->ColWidths[3]=550;  //Message
    strngrdMDBQuery->ColWidths[4]=55;   //Recovery   /Count
    strngrdMDBQuery->ColWidths[5]=60;   //Date       /Stop Time
    strngrdMDBQuery->ColWidths[6]=60;   //Time       /MTTR

    //Print Only Jam
    strngrdTemp->ColCount=8;
    for(i=0; i<strngrdTemp->RowCount; i++)
    {
        strngrdTemp->ClearRow(i);                                               // [W906] golden cObserver.cpp:4928 Rows[i]->Clear(), as :4911
    }

    WhereQuery.sprintf(" WHERE ((OccurDateTime >= '%s %s' AND OccurDateTime <= '%s %s') "
                           " AND (AlarmCode > 'JAM01' AND AlarmCode < 'JAM99') "
                           " AND Duplicate=0) ",
                           asStartData, asStartTime,
                           asEndData, asEndTime);
    asQuery.sprintf("SELECT UnitName, AlarmCode, Message, Recovery, DATE(OccurDateTime) Date, TIME(OccurDateTime) Time FROM AlarmHistoryView %s ORDER BY OccurDateTime DESC", WhereQuery);
    MyDBVProcess(asQuery, strngrdTemp);
    iStartRow2=strngrdTemp->RowCount+iRowNullSpace;
    strngrdMDBQuery->RowCount=(int)strngrdTemp->RowCount;                       // [W906] golden cObserver.cpp:4939 without (int): proxy = proxy would pick RowCountProxy's implicit copy assignment (StringGrid.h:149) and re-point the proxy at strngrdTemp instead of resizing
    if(strngrdTemp->Cells[1][1]=="No Record!!")
        iAllJamCount=0;
    else
        iAllJamCount=strngrdTemp->RowCount-1;

    for(i=0; i<strngrdTemp->RowCount; i++)
    {
        for(j=0; j<7; j++)
        {
            strngrdMDBQuery->Cells[j][i]=strngrdTemp->Cells[j][i];
        }
    }

    //Jam Statistics
    for(i=0; i<strngrdTemp->RowCount; i++)
    {
        strngrdTemp->ClearRow(i);                                               // [W906] golden cObserver.cpp:4956 Rows[i]->Clear(), as :4911
    }

    WhereQuery.sprintf(" WHERE ((OccurDateTime >= '%s %s' AND OccurDateTime <= '%s %s') "
                           " AND (AlarmCode > 'JAM01' AND AlarmCode < 'JAM99') "
                           " AND Duplicate=0) ",
                            asStartData, asStartTime,
                            asEndData, asEndTime);

    asQuery.sprintf(
                    "SELECT                                                                                     "
                    "      UnitName.UnitName,                                                                   "
                    "      AlarmList.AlarmCode,                                                                 "
                    "      (AlarmList.Message || ' ' ||  MotorAlarmList.MotMess) Message,                       "
                    "      Count(AlarmList.AlarmCode) Count,                                                    "
                    "      Sum(EventLog.StopedTime) StopTime,                                                   "
                    "      AVG(EventLog.StopedTime) MTTR                                                        "
                    "FROM                                                                                       "
                    "      EventLog                                                                             "
                    "      INNER JOIN AlarmList ON (EventLog.ID_AlarmList = AlarmList.ID_AlarmList)             "
                    "      INNER JOIN MotorAlarmList ON (EventLog.ID_MotorList = MotorAlarmList.ID_MotorList)   "
                    "      INNER JOIN UnitName ON (AlarmList.UnitNo = UnitName.ID_UnitNo)                       "
                    "%s                                                                                         "
                    "GROUP BY                                                                                   "
                    "       AlarmCode                                                                           "
                    "ORDER BY                                                                                   "
                    "      Count DESC                                                                           ", WhereQuery);

    MyDBVProcess(asQuery, strngrdTemp);

    strngrdMDBQuery->RowCount=strngrdMDBQuery->RowCount+strngrdTemp->RowCount+iRowNullSpace;   // [W906] golden cObserver.cpp:4986 RowCount+=...; the vclcompat proxy has no += (StringGrid.h:149-163), same value
    iDownTimeSum=0;
    for(i=0; i<strngrdTemp->RowCount; i++)
    {
        for(j=0; j<7; j++)
        {
            if((j==5 || j==6) && i!=0)
            {
                asTime=ConvertSecondToSPC(atoi(strngrdTemp->Cells[j][i].c_str()));
                strngrdMDBQuery->Cells[j][iStartRow2+i]=asTime;
                if(j==5)
                    iDownTimeSum+=atoi(strngrdTemp->Cells[j][i].c_str());       //Sam 20210824 : 新增 Down Time/ MTTR(avg.)
            }
            else
            {
                strngrdMDBQuery->Cells[j][iStartRow2+i]=strngrdTemp->Cells[j][i];
            }
        }
    }

    //Time Statistics
    // [W906] golden cObserver.cpp:5007 `DateSeparator='-';` -- see :4899 above
    DTStart=StrToDateTime(asStartData);
    DTEnd=StrToDateTime(asEndData);
    iDay=(DTEnd-DTStart);
    if(iDay<=0)
    {
        iDay=1;
    }
    // [W906] golden cObserver.cpp:5015 `DateSeparator=DateSeparatorOld;` -- see :4899 above
    iLoaderSum=0;
    iProductTimeSum=0;
    for(i=0; i<iDay; i++)
    {
        asYear=(DTStart+TDateTime(i)).FormatString("yyyy");                    // [W906] golden cObserver.cpp:5020 (DTStart+i) -- vclcompat TDateTime + int is ambiguous (cpublic.cpp:461, cMyDB.cpp:563); same value
        asData=(DTStart+TDateTime(i)).FormatString("yyyy-mm-dd");              // [W906] golden cObserver.cpp:5021, as the line above
        for(j=0; j<2; j++)
        {
            asFilePath.sprintf("%s\\%s\\%s-%s.txt",W906O19_ProductLoaderDir(),asYear,asData,asFileTime[j]);   // [W906] golden cObserver.cpp:5024 asProduct_LoaderPath, through the A4 ctest seam W906_PRODLOADER_ROOT (cMyDB.cpp:477); unset = the golden global
            if(FileExists(asFilePath))
            {
                iLoader         =ReadIniData(asFilePath,"Product","LoaderCount",0);
                iProductTime    =ReadIniData(asFilePath,"Product","ProductTime",0);
                iLoaderSum      +=iLoader;
                iProductTimeSum +=iProductTime;
            }
        }
    }

    iStartRow3=strngrdMDBQuery->RowCount+iRowNullSpace;
    strngrdMDBQuery->RowCount=strngrdMDBQuery->RowCount+iRowNullSpace+6;        //Sam 20210824 : 新增 Down Time/ MTTR(avg.) 4>6 //結檔時間/日/時間, 實際run時間/H, 投入數量, 發生jam次數  // [W906] golden cObserver.cpp:5036 RowCount+=..., as :4986

    strngrdMDBQuery->Cells[0][iStartRow3]  ="Data Period";
    strngrdMDBQuery->Cells[0][iStartRow3+1]="Run Time/H";
    strngrdMDBQuery->Cells[0][iStartRow3+2]="Total Loader";
    strngrdMDBQuery->Cells[0][iStartRow3+3]="Total Count";
    strngrdMDBQuery->Cells[0][iStartRow3+4]="Down Time";                        //Sam 20210824 : 新增 Down Time/ MTTR(avg.)
    strngrdMDBQuery->Cells[0][iStartRow3+5]="MTTR";                             //Sam 20210824 : 新增 Down Time/ MTTR(avg.)

    strngrdMDBQuery->Cells[1][iStartRow3]=asStartData  +" "+   asStartTime;
    strngrdMDBQuery->Cells[2][iStartRow3]=asEndData    +" "+   asEndTime;

    dTime=(double)iProductTimeSum/3600.0;                                       //sec > Hour
    asTime.sprintf("%1.1f",dTime);
    strngrdMDBQuery->Cells[1][iStartRow3+1]=asTime;

    strngrdMDBQuery->Cells[1][iStartRow3+2]=IntToStr(iLoaderSum);
    strngrdMDBQuery->Cells[1][iStartRow3+3]=IntToStr(iAllJamCount);

    strngrdMDBQuery->Cells[1][iStartRow3+4]=ConvertSecondToSPC(iDownTimeSum);   //Sam 20210824 : 新增 Down Time/ MTTR(avg.)
    if(iAllJamCount!=0)
        strngrdMDBQuery->Cells[1][iStartRow3+5]=ConvertSecondToSPC(iDownTimeSum/iAllJamCount);
}
//---------------------------------------------------------------------------
