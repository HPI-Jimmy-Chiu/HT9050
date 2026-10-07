// ============================================================================
//  cOffSet.cpp -- AI(W906-OFS-READFILE) 20260923
//
//  golden: HT9011UC_Code_V3.33.906.0_20260618\cOffSet.cpp (cp950 -> UTF-8).
//  Transcribed VERBATIM; every deviation is marked inline.
//
//  WHY A NEW FILE, AND WHY NOT forms/fOffSet.cpp.
//  TfOffSet::ReadFile() reaches fOffSet->GetOffsetPath(), whose body needs
//  FileInfo().PathCombin() -- ProductionInfo/FileInfo.cpp, i.e. library
//  **ht9045_sm** (CMakeLists.txt:2641).  ht9045_forms cannot link ht9045_sm
//  (the cycle recorded at CMakeLists.txt:559-560 / :612-646), which is exactly
//  why forms/fOffSet.h carries GATE (O-4) "GetOffsetPath -- 相依在 ht9045_sm".
//  Putting the body in forms/ would not fix that; it would only move the
//  undefined reference.
//
//  So this file follows the precedent already set by TfSetup: the facade lives
//  in forms/fSetup.h (ht9045_forms) while the heavy ReadFile body lives in
//  cSetUp.cpp (ht9045_sm, CMakeLists.txt:2406).  cOffSet.cpp is added to
//  ht9045_sm the same way, and GATE (O-4) is RETIRED by this file.
//
//  WHAT LANDS HERE (four bodies + three tables):
//      CapStrInput / CapStrOutput / CapStrSortput   golden :88-156
//      TfOffSet::GetOffsetPath                      golden :1426-1462   (37 L)
//      TfOffSet::ReadInvisibleFile                  golden :1855-1992  (138 L)
//      TfOffSet::ReadFile                           golden :1994-2310  (317 L)
//      TfOffSet::LoadLoaderScaleValues              golden :3930-3954   (25 L)
//  ReadFile calls the other three, so translating it alone would have been a
//  link error, not a feature.
//
//  ⚠ THESE BODIES WRITE.
//    * MyForceDirectories(szDir) creates the offset folder if absent.
//    * ReadInvisibleFile() is built on ReadWriteIni(..., true) -- it CREATES
//      D:\HT9045\IniData\DefineOffset\STD_125.Data and STD_25.Data when they
//      do not exist (golden :2289-2299 says so outright: "強制產生"), and
//      writes every missing key back into D:\HT9045\data\<recipe>.ini.
//      It only runs under CosFunction.bUseInvisibleOffset.
//    * The paths on golden :2289/:2295/:2301/:2305 are HARD-CODED to
//      D:\HT9045\... in golden itself.  They are kept verbatim; NOT rerouted
//      through DataPath, because inventing a path is a behaviour change.
//
//  NOT defined here: CapStr[] (golden :31-86) and SpecialOffSetName[] (golden
//  :158-171).  Both already have definitions in forms/fOffSet.cpp (:49 / :121),
//  which this tree uses as its stand-in for golden's cOffSet.cpp -- so defining
//  them again here would be a duplicate symbol, not a fix.  ReadFile DOES index
//  SpecialOffSetName (golden :2545-2547), so this wave adds the missing
//  `extern AnsiString SpecialOffSetName[];` to forms/fOffSet.h; golden has no
//  such extern because there the definition and the use share one file.
//  CapStr[] has no use in this file; ainarm2.cpp:1333-1377 still documents its
//  own gap, and that stays a separate decision.
// ============================================================================


#include "forms/fOffSet.h"

#include "cmydef.h"                  // CUSTOMER_CODE / CC_TSMC_TAINAN / Tempture_Hot
#include "cprod.h"                   // InArmOffSet_File / OutArmOffSet_File / SortArmOffSet_File /
                                     // Offset_File / InvisibleOffset / InArmSuck / OutArmSuck
#include "Config.h"                  // IniConfig
#include "CosFunction.h"             // CosFunction
#include "LastSet.h"                 // LastSet.iTemperature
#include "common.h"                  // ReadIniData / ReadWriteIni / MyForceDirectories / GetLastOpenFN / CheckRange
#include "MachineType.h"             // CC_ASE_CL / Tri_Temp_Machine / MAX_ARM_Col / MAX_ARM_Row
#include "mykitsuck.h"               // InArmSuck / OutArmSuck / OutArm2Suck (iMotCol / iMotRow)
#include "ProductionInfo/FileInfo.h" // FileInfo().PathCombin  -- the reason this file is in ht9045_sm
#include "forms/fMain.h"             // fMain->cbSetupFileName
#include "vclcompat/SysUtils.h"      // FileExists / DirectoryExists / ForceDirectories


// ---- golden :88-156 -- the three section-name tables ReadFile indexes ----
AnsiString CapStrInput[InOfsTotal]=
{
    "Loader",                           //0
    "Hot Plate1",                       //1
    "Hot Plate2",                       //2
    "Input Shuttle1",                   //3
    "Input Shuttle2",                   //4
    "Auto Clean",                       //5
    "OCR",                              //6
    "Input Rotate",                     //7
    "Loader Row B",                     //8 //Steven 20140811 : For 32 Site Loader and Shuttle Offset
    "In Shuttle 1 Left B",              //9 //Steven 20140811 : For 32 Site Loader and Shuttle Offset
    "In Shuttle 1 Right A",             //10 //Steven 20140811 : For 32 Site Loader and Shuttle Offset
    "In Shuttle 1 Right B",             //11 //Steven 20140811 : For 32 Site Loader and Shuttle Offset
    "In Shuttle 2 Left B",              //12 //Steven 20140811 : For 32 Site Loader and Shuttle Offset
    "In Shuttle 2 Right A",             //13 //Steven 20140811 : For 32 Site Loader and Shuttle Offset
    "In Shuttle 2 Right B",             //14 //Steven 20140811 : For 32 Site Loader and Shuttle Offset
    "Input Shuttle1 Auto Clean",        //15 //20140923 wei : For Shuttle Auto Clean
    "In Shuttle 1 Left B Auto Clean",   //16 //20140923 wei : For Shuttle Auto Clean
    "In Shuttle 1 Right A Auto Clean",  //17 //20140923 wei : For Shuttle Auto Clean
    "In Shuttle 1 Right B Auto Clean",  //18 //20140923 wei : For Shuttle Auto Clean
    "Input Shuttle2 Auto Clean",        //19 //20140923 wei : For Shuttle Auto Clean
    "In Shuttle 2 Left B Auto Clean",   //20 //20140923 wei : For Shuttle Auto Clean
    "In Shuttle 2 Right A Auto Clean",  //21 //20140923 wei : For Shuttle Auto Clean
    "In Shuttle 2 Right B Auto Clean",  //22 //20140923 wei : For Shuttle Auto Clean
    "Auto Shuttle 1",                   //23 //wei 20160914 Auto Shuttle Sensor
    "Auto Shuttle 2",                   //24 //wei 20160914 Auto Shuttle Sensor
    "Preciser",                         //25 //Frank 20180410 (Steven) : InArm Preciser Station
    "InArm Placement",                  //26 //JimmyChiu 20220908 add Pickup Error Placement
    "Bottom 2D",                        //27
};

AnsiString CapStrOutput[OutOfsTotal]=
{
    "Output Shuttle1",                  //0
    "Output Shuttle2",                  //1
    "Auto1",                            //2
    "Auto2",                            //3
    "Auto3",                            //4
    "Auto4",                            //5
    "Auto5",                            //6
    "Auto6",                            //7
    "Fix1",                             //8
    "Fix2",                             //9
    "Fix3",                             //10
    "Fix4",                             //11
    "Fix5",                             //12
    "Fix6",                             //13
    "Output Rotate",                    //14
    "Top View",                         //15 //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
    "PAD View",                         //16 //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
    "BGA View",                         //17 //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
    "Out Shuttle 1 Left B",             //18 //Steven 20190802 : For 32 Site Out Shuttle Offset
    "Out Shuttle 1 Right A",            //19 //Steven 20190802 : For 32 Site Out Shuttle Offset
    "Out Shuttle 1 Right B",            //20 //Steven 20190802 : For 32 Site Out Shuttle Offset
    "Out Shuttle 2 Left B",             //21 //Steven 20190802 : For 32 Site Out Shuttle Offset
    "Out Shuttle 2 Right A",            //22 //Steven 20190802 : For 32 Site Out Shuttle Offset
    "Out Shuttle 2 Right B",            //23 //Steven 20190802 : For 32 Site Out Shuttle Offset
    "Scan AOI",                         //24 //Ifor 20191229 : add
    "SortShuttleRight"                  //25                                    //RogerYang 20250417 for HT9046AU add
};

AnsiString CapStrSortput[SortOfsTotal]=                                         //RogerYang 20250417 for HT9046AU add
{
    "SortShuttleLeft",                  //0
    "Auto4",                            //1
    "Auto5",                            //2
    "Auto6"                             //3
};

// ============================================================================
//  AI(W906-OFS-READFILE) 20260923 -- PORT-ONLY HELPER, not a golden function.
//
//  golden allocates the six ARM_OFFSET arrays in TfMain's CONSTRUCTOR
//  (main.cpp:2123-2139).  This tree has no TfMain ctor, so cprod.cpp:67-72
//  defines the arrays and nothing ever fills them: every element is NULL.
//  ainarm2.cpp:2841-2844 already documents that as a live hazard ("NULL-DATA
//  HAZARD, DELIBERATELY NOT GUARDED ... open task #10").
//
//  TfOffSet::ReadFile() dereferences InArmOffSet_File[i] on its very first
//  statement, so without this the translated body is an immediate access
//  violation -- observed, 0xC0000005, before this helper existed.  Same shape
//  as fSetup->Init(): golden did the allocation somewhere this tree does not
//  have, so the caller has to do it explicitly.
//
//  The loops are golden main.cpp:2123-2139 verbatim; the only deviation is the
//  NULL guard, which makes the call idempotent.  That guard follows the tree's
//  existing precedent for the same class of TfMain-ctor allocation --
//  cSocket.cpp:218, `if(ArmData[i] == NULL) ArmData[i] = new TArm(...)`.
//
//  ⚠ ORDERING: ARM_OFFSET's constructor (cprod.cpp) sizes tArmPickOffset /
//  tArmPlaceOffset from InArmSuck.iMotRow * iMotCol, so this must run AFTER
//  SetMyKitSuckItemAmount() (cinitial.cpp:8014).  golden has the same
//  constraint and satisfies it the same way -- main.cpp:2121 calls
//  SetMyKitSuckItemAmount() two lines before the loops.
// ============================================================================
void EnsureArmOffsetObjects()
{
    for(int i=0; i<InOfsTotal; i++)                                             // golden main.cpp:2123-2127
    {
        if(InArmOffSet[i]     ==NULL) InArmOffSet[i]     =new ARM_OFFSET();
        if(InArmOffSet_File[i]==NULL) InArmOffSet_File[i]=new ARM_OFFSET();
    }

    for(int i=0; i<OutOfsTotal; i++)                                            // golden main.cpp:2129-2133
    {
        if(OutArmOffSet[i]     ==NULL) OutArmOffSet[i]     =new ARM_OFFSET();
        if(OutArmOffSet_File[i]==NULL) OutArmOffSet_File[i]=new ARM_OFFSET();
    }

    for(int i=0; i<SortOfsTotal; i++)                                           // golden main.cpp:2135-2139
    {
        if(SortArmOffSet[i]     ==NULL) SortArmOffSet[i]     =new ARM_OFFSET();
        if(SortArmOffSet_File[i]==NULL) SortArmOffSet_File[i]=new ARM_OFFSET();
    }
}

// ---- golden :1426-1462 -- GATE (O-4) RETIRED by this file ----
AnsiString TfOffSet::GetOffsetPath(AnsiString FileFolder)
{
    AnsiString szDir="", Str;
    int iPos;

    if(CUSTOMER_CODE==CC_ASE_CL ||
       IniConfig.bE45_AllSetupFileUseOneFile)                                   //Steven 20140827 : 所有工作檔共用同一個Offset檔案
    {
        szDir=FileInfo().PathCombin(DefaultPath, "DefineOffset");
    }
    else
    {
        if(FileFolder!="")
            LastFileName=FileFolder;
        else
            LastFileName=GetLastOpenFN();

        if(IniConfig.bE59GroupOffsetFile)                                       //Steven 20190327 : Offset file使用中括號做群組
        {
            iPos=LastFileName.AnsiPos("]");
            if(iPos!=0)
            {
                Str=LastFileName.SubString(0, iPos);
                szDir=FileInfo().PathCombin(Str, OffsetPath);
            }
            else
            {
                szDir=FileInfo().PathCombin(OffsetPath, LastFileName);          //Steven 20100927 Start : Offset改資料夾
            }
        }
        else
        {
            szDir=FileInfo().PathCombin(OffsetPath, LastFileName);              //Steven 20100927 Start : Offset改資料夾
        }
    }
    return szDir;
}

// ---- golden :1855-1992 -- writes STD_125/STD_25 + <recipe>.ini (see banner) ----
void TfOffSet::ReadInvisibleFile(AnsiString szDir2)
{
    AnsiString str="";
    InvisibleOffset.LoaderXOffset=CheckRange(ReadWriteIni(szDir2,       "Loader", "LoaderXOffset", InvisibleOffset.LoaderXOffset , 0, true), -300, 300);
    InvisibleOffset.LoaderYOffset=CheckRange(ReadWriteIni(szDir2,       "Loader", "LoaderYOffset", InvisibleOffset.LoaderYOffset , 0, true), -300, 300);
    InvisibleOffset.LoaderZOffset=CheckRange(ReadWriteIni(szDir2,       "Loader", "LoaderZOffset", InvisibleOffset.LoaderZOffset , 0, true), -300, 300);
    InvisibleOffset.LoaderXPOffset=CheckRange(ReadWriteIni(szDir2,      "Loader", "LoaderXPitchOffset", InvisibleOffset.LoaderXPOffset , 0, true), -300, 300);
    InvisibleOffset.LoaderXP2Offset=CheckRange(ReadWriteIni(szDir2,     "Loader", "LoaderXPitch2Offset", InvisibleOffset.LoaderXP2Offset , 0, true), -300, 300);
    InvisibleOffset.LoaderYPOffset=CheckRange(ReadWriteIni(szDir2,      "Loader", "LoaderYPitchOffset", InvisibleOffset.LoaderYPOffset , 0, true), -300, 300);

    InvisibleOffset.HotPlate1XOffset=CheckRange(ReadWriteIni(szDir2,    "HotPlate", "HotPlate1XOffset",InvisibleOffset.HotPlate1XOffset ,0, true), -300, 300);
    InvisibleOffset.HotPlate1YOffset=CheckRange(ReadWriteIni(szDir2,    "HotPlate", "HotPlate1YOffset",InvisibleOffset.HotPlate1YOffset ,0, true), -300, 300);
    InvisibleOffset.HotPlate1ZOffset=CheckRange(ReadWriteIni(szDir2,    "HotPlate", "HotPlate1ZOffset",InvisibleOffset.HotPlate1ZOffset ,0, true), -300, 300);
    InvisibleOffset.HotPlate1XPOffset=CheckRange(ReadWriteIni(szDir2,   "HotPlate", "HotPlate1XPitchOffset", InvisibleOffset.HotPlate1XPOffset , 0, true), -300, 300);
    InvisibleOffset.HotPlate1XP2Offset=CheckRange(ReadWriteIni(szDir2,  "HotPlate", "HotPlate1XPitch2Offset", InvisibleOffset.HotPlate1XP2Offset , 0, true), -300, 300);
    InvisibleOffset.HotPlate1YPOffset=CheckRange(ReadWriteIni(szDir2,   "HotPlate", "HotPlate1YPitchOffset", InvisibleOffset.HotPlate1YPOffset , 0, true), -300, 300);

    InvisibleOffset.HotPlate2XOffset=CheckRange(ReadWriteIni(szDir2,    "HotPlate", "HotPlate2XOffset",InvisibleOffset.HotPlate2XOffset ,0, true), -300, 300);
    InvisibleOffset.HotPlate2YOffset=CheckRange(ReadWriteIni(szDir2,    "HotPlate", "HotPlate2YOffset",InvisibleOffset.HotPlate2YOffset ,0, true), -300, 300);
    InvisibleOffset.HotPlate2ZOffset=CheckRange(ReadWriteIni(szDir2,    "HotPlate", "HotPlate2ZOffset",InvisibleOffset.HotPlate2ZOffset ,0, true), -300, 300);
    InvisibleOffset.HotPlate2XPOffset=CheckRange(ReadWriteIni(szDir2,   "HotPlate", "HotPlate2XPitchOffset", InvisibleOffset.HotPlate2XPOffset , 0, true), -300, 300);
    InvisibleOffset.HotPlate2XP2Offset=CheckRange(ReadWriteIni(szDir2,  "HotPlate", "HotPlate2XPitch2Offset", InvisibleOffset.HotPlate2XP2Offset , 0, true), -300, 300);
    InvisibleOffset.HotPlate2YPOffset=CheckRange(ReadWriteIni(szDir2,   "HotPlate", "HotPlate2YPitchOffset", InvisibleOffset.HotPlate2YPOffset , 0, true), -300, 300);

    InvisibleOffset.InSht1XOffset=CheckRange(ReadWriteIni(szDir2,    "InShuttle", "Shuttle1XOffset", InvisibleOffset.InSht1XOffset,0, true), -300, 300);
    InvisibleOffset.InSht1YOffset=CheckRange(ReadWriteIni(szDir2,    "InShuttle", "Shuttle1YOffset", InvisibleOffset.InSht1YOffset,0, true), -300, 300);
    InvisibleOffset.InSht1ZOffset=CheckRange(ReadWriteIni(szDir2,    "InShuttle", "Shuttle1ZOffset",InvisibleOffset.InSht1ZOffset,0, true), -300, 300);
    InvisibleOffset.InSht1XPOffset=CheckRange(ReadWriteIni(szDir2,   "InShuttle", "Shuttle1XPitchOffset", InvisibleOffset.InSht1XPOffset , 0, true), -300, 300);
    InvisibleOffset.InSht1XP2Offset=CheckRange(ReadWriteIni(szDir2,  "InShuttle", "Shuttle1XPitch2Offset", InvisibleOffset.InSht1XP2Offset , 0, true), -300, 300);
    InvisibleOffset.InSht1YPOffset=CheckRange(ReadWriteIni(szDir2,   "InShuttle", "Shuttle1YPitchOffset", InvisibleOffset.InSht1YPOffset , 0, true), -300, 300);

    InvisibleOffset.InSht2XOffset=CheckRange(ReadWriteIni(szDir2,    "InShuttle", "Shuttle2XOffset", InvisibleOffset.InSht2XOffset,0, true), -300, 300);
    InvisibleOffset.InSht2YOffset=CheckRange(ReadWriteIni(szDir2,    "InShuttle", "Shuttle2YOffset", InvisibleOffset.InSht2YOffset,0, true), -300, 300);
    InvisibleOffset.InSht2ZOffset=CheckRange(ReadWriteIni(szDir2,    "InShuttle", "Shuttle2ZOffset",InvisibleOffset.InSht2ZOffset,0, true), -300, 300);
    InvisibleOffset.InSht2XPOffset=CheckRange(ReadWriteIni(szDir2,   "InShuttle", "Shuttle2XPitchOffset", InvisibleOffset.InSht2XPOffset , 0, true), -300, 300);
    InvisibleOffset.InSht2XP2Offset=CheckRange(ReadWriteIni(szDir2,  "InShuttle", "Shuttle2XPitch2Offset", InvisibleOffset.InSht2XP2Offset , 0, true), -300, 300);
    InvisibleOffset.InSht2YPOffset=CheckRange(ReadWriteIni(szDir2,   "InShuttle", "Shuttle2YPitchOffset", InvisibleOffset.InSht2YPOffset , 0, true), -300, 300);

    InvisibleOffset.OutSht1XOffset=CheckRange(ReadWriteIni(szDir2,   "OutShuttle", "Shuttle1XOffset", InvisibleOffset.OutSht1XOffset,0, true), -300, 300);
    InvisibleOffset.OutSht1YOffset=CheckRange(ReadWriteIni(szDir2,   "OutShuttle", "Shuttle1YOffset", InvisibleOffset.OutSht1YOffset,0, true), -300, 300);
    InvisibleOffset.OutSht1ZOffset=CheckRange(ReadWriteIni(szDir2,   "OutShuttle", "Shuttle1ZOffset",InvisibleOffset.OutSht1ZOffset,0, true), -300, 300);
    InvisibleOffset.OutSht1XPOffset=CheckRange(ReadWriteIni(szDir2,  "OutShuttle", "Shuttle1XPitchOffset",InvisibleOffset.OutSht1XPOffset,0, true), -300, 300);
    InvisibleOffset.OutSht1XP2Offset=CheckRange(ReadWriteIni(szDir2, "OutShuttle", "Shuttle1XPitch2Offset",InvisibleOffset.OutSht1XP2Offset,0, true), -300, 300);
    InvisibleOffset.OutSht1YPOffset=CheckRange(ReadWriteIni(szDir2,  "OutShuttle", "Shuttle1YPitchOffset",InvisibleOffset.OutSht1YPOffset,0, true), -300, 300);

    InvisibleOffset.OutSht2XOffset=CheckRange(ReadWriteIni(szDir2,   "OutShuttle", "Shuttle2XOffset", InvisibleOffset.OutSht2XOffset, 0 , true), -300, 300);
    InvisibleOffset.OutSht2YOffset=CheckRange(ReadWriteIni(szDir2,   "OutShuttle", "Shuttle2YOffset", InvisibleOffset.OutSht2YOffset, 0 , true), -300, 300);
    InvisibleOffset.OutSht2ZOffset=CheckRange(ReadWriteIni(szDir2,   "OutShuttle", "Shuttle22ZOffset",InvisibleOffset.OutSht2ZOffset ,0, true), -300, 300);
    InvisibleOffset.OutSht2XPOffset=CheckRange(ReadWriteIni(szDir2,  "OutShuttle", "Shuttle2XPitchOffset",InvisibleOffset.OutSht2XPOffset,0, true), -300, 300);
    InvisibleOffset.OutSht2XP2Offset=CheckRange(ReadWriteIni(szDir2, "OutShuttle", "Shuttle2XPitch2Offset",InvisibleOffset.OutSht2XP2Offset,0, true), -300, 300);
    InvisibleOffset.OutSht2YPOffset=CheckRange(ReadWriteIni(szDir2,  "OutShuttle", "Shuttle2YPitchOffset",InvisibleOffset.OutSht2YPOffset,0, true), -300, 300);

    InvisibleOffset.Auto1XOffset=CheckRange(ReadWriteIni(szDir2,   "Unloader", "Auto1XOffset", InvisibleOffset.Auto1XOffset,0, true), -300, 300);
    InvisibleOffset.Auto1YOffset=CheckRange(ReadWriteIni(szDir2,   "Unloader", "Auto1YOffset", InvisibleOffset.Auto1YOffset,0, true), -300, 300);
    InvisibleOffset.Auto1ZOffset=CheckRange(ReadWriteIni(szDir2,   "Unloader", "Auto1ZOffset", InvisibleOffset.Auto1ZOffset,0, true), -300, 300);
    InvisibleOffset.Auto1XPOffset=CheckRange(ReadWriteIni(szDir2,  "Unloader", "Auto1XPitchOffset", InvisibleOffset.Auto1XPOffset,0, true), -300, 300);
    InvisibleOffset.Auto1XP2Offset=CheckRange(ReadWriteIni(szDir2, "Unloader", "Auto1XPitch2Offset", InvisibleOffset.Auto1XP2Offset,0, true), -300, 300);
    InvisibleOffset.Auto1YPOffset=CheckRange(ReadWriteIni(szDir2,  "Unloader", "Auto1YPitchOffset", InvisibleOffset.Auto1YPOffset,0, true), -300, 300);

    InvisibleOffset.Auto2XOffset=CheckRange(ReadWriteIni(szDir2,   "Unloader", "Auto2XOffset", InvisibleOffset.Auto2XOffset,0, true), -300, 300);
    InvisibleOffset.Auto2YOffset=CheckRange(ReadWriteIni(szDir2,   "Unloader", "Auto2YOffset", InvisibleOffset.Auto2YOffset,0, true), -300, 300);
    InvisibleOffset.Auto2ZOffset=CheckRange(ReadWriteIni(szDir2,   "Unloader", "Auto2ZOffset", InvisibleOffset.Auto2ZOffset,0, true), -300, 300);
    InvisibleOffset.Auto2XPOffset=CheckRange(ReadWriteIni(szDir2,  "Unloader", "Auto2XPitchOffset", InvisibleOffset.Auto2XPOffset,0, true), -300, 300);
    InvisibleOffset.Auto2XP2Offset=CheckRange(ReadWriteIni(szDir2, "Unloader", "Auto2XPitch2Offset", InvisibleOffset.Auto2XP2Offset,0, true), -300, 300);
    InvisibleOffset.Auto2YPOffset=CheckRange(ReadWriteIni(szDir2,  "Unloader", "Auto2YPitchOffset", InvisibleOffset.Auto2YPOffset,0, true), -300, 300);

    InvisibleOffset.Auto3XOffset=CheckRange(ReadWriteIni(szDir2,   "Unloader", "Auto3XOffset", InvisibleOffset.Auto3XOffset,0, true), -300, 300);
    InvisibleOffset.Auto3YOffset=CheckRange(ReadWriteIni(szDir2,   "Unloader", "Auto3YOffset", InvisibleOffset.Auto3YOffset,0, true), -300, 300);
    InvisibleOffset.Auto3ZOffset=CheckRange(ReadWriteIni(szDir2,   "Unloader", "Auto3ZOffset", InvisibleOffset.Auto3ZOffset,0, true), -300, 300);
    InvisibleOffset.Auto3XPOffset=CheckRange(ReadWriteIni(szDir2,  "Unloader", "Auto3XPitchOffset", InvisibleOffset.Auto3XPOffset,0, true), -300, 300);
    InvisibleOffset.Auto3XP2Offset=CheckRange(ReadWriteIni(szDir2, "Unloader", "Auto3XPitch2Offset", InvisibleOffset.Auto3XP2Offset,0, true), -300, 300);
    InvisibleOffset.Auto3YPOffset=CheckRange(ReadWriteIni(szDir2,  "Unloader", "Auto3YPitchOffset", InvisibleOffset.Auto3YPOffset,0, true), -300, 300);

    InvisibleOffset.Fix1XOffset=CheckRange(ReadWriteIni(szDir2,    "Unloader", "Fix1XOffset",InvisibleOffset.Fix1XOffset ,0, true), -300, 300);
    InvisibleOffset.Fix1YOffset=CheckRange(ReadWriteIni(szDir2,    "Unloader", "Fix1YOffset",InvisibleOffset.Fix1YOffset ,0, true), -300, 300);
    InvisibleOffset.Fix1ZOffset=CheckRange(ReadWriteIni(szDir2,    "Unloader", "Fix1ZOffset",InvisibleOffset.Fix1ZOffset ,0, true), -300, 300);
    InvisibleOffset.Fix1XPOffset=CheckRange(ReadWriteIni(szDir2,   "Unloader", "Fix1XPitchOffset",InvisibleOffset.Fix1XPOffset ,0, true), -300, 300);
    InvisibleOffset.Fix1XP2Offset=CheckRange(ReadWriteIni(szDir2,  "Unloader", "Fix1XPitch2Offset",InvisibleOffset.Fix1XP2Offset ,0, true), -300, 300);
    InvisibleOffset.Fix1YPOffset=CheckRange(ReadWriteIni(szDir2,   "Unloader", "Fix1YPitchOffset",InvisibleOffset.Fix1YPOffset ,0, true), -300, 300);

    InvisibleOffset.Fix2XOffset=CheckRange(ReadWriteIni(szDir2,    "Unloader", "Fix2XOffset",InvisibleOffset.Fix2XOffset ,0, true), -300, 300);
    InvisibleOffset.Fix2YOffset=CheckRange(ReadWriteIni(szDir2,    "Unloader", "Fix2YOffset",InvisibleOffset.Fix2YOffset ,0, true), -300, 300);
    InvisibleOffset.Fix2ZOffset=CheckRange(ReadWriteIni(szDir2,    "Unloader", "Fix2ZOffset",InvisibleOffset.Fix2ZOffset ,0, true), -300, 300);
    InvisibleOffset.Fix2XPOffset=CheckRange(ReadWriteIni(szDir2,   "Unloader", "Fix2XPitchOffset",InvisibleOffset.Fix2XPOffset ,0, true), -300, 300);
    InvisibleOffset.Fix2XP2Offset=CheckRange(ReadWriteIni(szDir2,  "Unloader", "Fix2XPitch2Offset",InvisibleOffset.Fix2XP2Offset ,0, true), -300, 300);
    InvisibleOffset.Fix2YPOffset=CheckRange(ReadWriteIni(szDir2,   "Unloader", "Fix2YPitchOffset",InvisibleOffset.Fix2YPOffset ,0, true), -300, 300);

    InvisibleOffset.Fix3XOffset=CheckRange(ReadWriteIni(szDir2,    "Unloader", "Fix3XOffset",InvisibleOffset.Fix3XOffset ,0, true), -300, 300);
    InvisibleOffset.Fix3YOffset=CheckRange(ReadWriteIni(szDir2,    "Unloader", "Fix3YOffset",InvisibleOffset.Fix3YOffset ,0, true), -300, 300);
    InvisibleOffset.Fix3ZOffset=CheckRange(ReadWriteIni(szDir2,    "Unloader", "Fix3ZOffset",InvisibleOffset.Fix3ZOffset ,0, true), -300, 300);
    InvisibleOffset.Fix3XPOffset=CheckRange(ReadWriteIni(szDir2,   "Unloader", "Fix3XPitchOffset",InvisibleOffset.Fix3XPOffset ,0, true), -300, 300);
    InvisibleOffset.Fix3XP2Offset=CheckRange(ReadWriteIni(szDir2,  "Unloader", "Fix3XPitch2Offset",InvisibleOffset.Fix3XP2Offset ,0, true), -300, 300);
    InvisibleOffset.Fix3YPOffset=CheckRange(ReadWriteIni(szDir2,   "Unloader", "Fix3YPitchOffset",InvisibleOffset.Fix3YPOffset ,0, true), -300, 300);

    for(int i=0; i<MAX_ARM_Col; i++)//8=In/out arm suction
    {
        for(int j=0; j<MAX_ARM_Row; j++)//8=In/out arm suction
        {
            if(i==iInArmXBase && j==iInArmYBase)
            {
                InvisibleOffset.InarmPickOffSet[j][i]=CheckRange(ReadWriteIni(szDir2,     "Loader", str.sprintf("PickUp %c", 'A'+(i*2)+j), InvisibleOffset.InarmPickOffSet[j][i] ,0, true), 0, 0);
                InvisibleOffset.InarmReleaseOffset[j][i]=CheckRange(ReadWriteIni(szDir2,  "Loader", str.sprintf("Place %c", 'A'+(i*2)+j), InvisibleOffset.InarmReleaseOffset[j][i] ,0, true), 0, 0);

                InvisibleOffset.OutarmPickOffSet[j][i]=CheckRange(ReadWriteIni(szDir2,     "Unloader", str.sprintf("PickUp %c", 'A'+(i*2)+j), InvisibleOffset.OutarmPickOffSet[j][i] ,0, true), 0, 0);
                InvisibleOffset.OutarmReleaseOffset[j][i]=CheckRange(ReadWriteIni(szDir2,  "Unloader", str.sprintf("Place %c", 'A'+(i*2)+j), InvisibleOffset.OutarmReleaseOffset[j][i] ,0, true), 0, 0);
            }
            else
            {
                InvisibleOffset.InarmPickOffSet[j][i]=CheckRange(ReadWriteIni(szDir2,     "Loader", str.sprintf("PickUp %c", 'A'+(i*2)+j),InvisibleOffset.InarmPickOffSet[j][i] ,0, true), -300, 300);
                InvisibleOffset.InarmReleaseOffset[j][i]=CheckRange(ReadWriteIni(szDir2,  "Loader", str.sprintf("Place %c", 'A'+(i*2)+j),InvisibleOffset.InarmReleaseOffset[j][i] ,0, true), -300, 300);

                InvisibleOffset.OutarmPickOffSet[j][i]=CheckRange(ReadWriteIni(szDir2,     "Unloader", str.sprintf("PickUp %c", 'A'+(i*2)+j),InvisibleOffset.OutarmPickOffSet[j][i] ,0, true), -300, 300);
                InvisibleOffset.OutarmReleaseOffset[j][i]=CheckRange(ReadWriteIni(szDir2,  "Unloader", str.sprintf("Place %c", 'A'+(i*2)+j),InvisibleOffset.OutarmReleaseOffset[j][i] ,0, true), -300, 300);
            }
        }
    }
    ReadWriteIni(szDir2,   "LoaderScale", "LoaderX", TestIF_File.fLoaderTrayXScaleBySetupFile ,1.0, true);
    ReadWriteIni(szDir2,   "LoaderScale", "LoaderY", TestIF_File.fLoaderTrayYScaleBySetupFile ,1.0, true);
    ReadWriteIni(szDir2,   "LoaderScale", "HotPlate1X", TestIF_File.fHotPlateXScaleBySetupFile[0]  ,1.0, true);
    ReadWriteIni(szDir2,   "LoaderScale", "HotPlate1Y", TestIF_File.fHotPlateYScaleBySetupFile[0]  ,1.0, true);
    ReadWriteIni(szDir2,   "LoaderScale", "HotPlate2X", TestIF_File.fHotPlateXScaleBySetupFile[1]  ,1.0, true);
    ReadWriteIni(szDir2,   "LoaderScale", "HotPlate2Y", TestIF_File.fHotPlateYScaleBySetupFile[1]  ,1.0, true);

    ReadWriteIni(szDir2,   "LoaderScale", "Shuttle1X", TestIF_File.fInShuttleXScaleBySetupFile[0]  ,1.0, true);
    ReadWriteIni(szDir2,   "LoaderScale", "Shuttle1Y", TestIF_File.fInShuttleYScaleBySetupFile[0]  ,1.0, true);
    ReadWriteIni(szDir2,   "LoaderScale", "Shuttle2X", TestIF_File.fInShuttleXScaleBySetupFile[1]  ,1.0, true);
    ReadWriteIni(szDir2,   "LoaderScale", "Shuttle2Y", TestIF_File.fInShuttleYScaleBySetupFile[1]  ,1.0, true);

    ReadWriteIni(szDir2,   "UnLoaderScale", "Shuttle1X", TestIF_File.fOutShuttleXScaleBySetupFile[0]  ,1.0, true);
    ReadWriteIni(szDir2,   "UnLoaderScale", "Shuttle1Y", TestIF_File.fOutShuttleYScaleBySetupFile[0]  ,1.0, true);
    ReadWriteIni(szDir2,   "UnLoaderScale", "Shuttle2X", TestIF_File.fOutShuttleXScaleBySetupFile[1]  ,1.0, true);
    ReadWriteIni(szDir2,   "UnLoaderScale", "Shuttle2Y", TestIF_File.fOutShuttleYScaleBySetupFile[1]  ,1.0, true);

    ReadWriteIni(szDir2,   "Index1","ShuttleROffset",0, 0, true);
    ReadWriteIni(szDir2,   "Index1","ShuttleLOffset",0, 0, true);
    ReadWriteIni(szDir2,   "Index2","ShuttleROffset",0, 0, true);
    ReadWriteIni(szDir2,   "Index2","ShuttleLOffset",0, 0, true);
}

// ---- golden :1994-2310 -- TfOffSet::ReadFile ----
void TfOffSet::ReadFile()
{
    AnsiString str="", S;
    AnsiString szDir="", szDir2="";

    if(CUSTOMER_CODE==CC_TSMC_TAINAN && LastFileName.AnsiPos("_NET")>0)         //wei 20161101 FTP下載讀取錯誤問題
    {
        LastFileName.Delete(LastFileName.Length()-3, LastFileName.Length());
    }

    if(CosFunction.bSaveOffsetByMachine && IniConfig.bA57_3SaveOffsetByMachine) //JimmyChiu 20220618 : save by machine
    {
        szDir=sSaveByMachine;
    }
    else
    {
        szDir=fOffSet->GetOffsetPath();                                         //Steven 20190109 : 整合Offset路徑
    }
    MyForceDirectories(szDir);

    if(Tri_Temp_Machine==1)                                                     //Ztex 2024.03.13 Add Low Temp Offset
        szDir+=Tri_Position_Offset();
    else
        szDir+="\\Position Offset.Data";

    for(int i=0; i<InOfsTotal; i++)                                             //Steven 20140425 : 重整Offset
    {
        InArmOffSet_File[i]->SetOneByOne (ReadIniData(szDir, CapStrInput[i], "One By One",0));
        InArmOffSet_File[i]->SetVariableY(ReadIniData(szDir, CapStrInput[i], "VariableY", 0.0));//ChungHung 20131231 alter AutoYPitch
        InArmOffSet_File[i]->SetVariable (ReadIniData(szDir, CapStrInput[i], "Variable",  0.0));
        InArmOffSet_File[i]->SetVariable2(ReadIniData(szDir, CapStrInput[i], "Variable2", 0.0));//Steven 20131002 : XY變距
        InArmOffSet_File[i]->SetVariable3(ReadIniData(szDir, CapStrInput[i], "Variable3", 0.0));
        InArmOffSet_File[i]->SetVariable4(ReadIniData(szDir, CapStrInput[i], "Variable4", 0.0));
        InArmOffSet_File[i]->SetX        (ReadIniData(szDir, CapStrInput[i], "Hand X",    0.0));
        InArmOffSet_File[i]->SetY        (ReadIniData(szDir, CapStrInput[i], "Hand Y",    0.0));
        InArmOffSet_File[i]->SetPickUp   (ReadIniData(szDir, CapStrInput[i], "PickUp",    0.0));
        InArmOffSet_File[i]->SetPlace    (ReadIniData(szDir, CapStrInput[i], "Place",     0.0));

        if(IniConfig.bE33InOutArmZOffsetSameOne==true)                          //jou 2014-10-07 加速 offset 讀取時間
        {
            if(i==InOfsLoader)
            {
                for(int j=0; j<InArmSuck.iMotCol; j++)
                {
                    for(int k=0; k<InArmSuck.iMotRow; k++)
                    {
                        InArmOffSet_File[i]->SingleOffSet->dPosOffSetX[k][j]   =ReadIniData(szDir, CapStrInput[i], str.sprintf("Hand %cX",  'A'+(j*2)+k), 0.0);
                        InArmOffSet_File[i]->SingleOffSet->dPosOffSetY[k][j]   =ReadIniData(szDir, CapStrInput[i], str.sprintf("Hand %cY",  'A'+(j*2)+k), 0.0);
                        InArmOffSet_File[i]->SingleOffSet->dPickUpOffSet[k][j] =ReadIniData(szDir, CapStrInput[i], str.sprintf("PickUp %c", 'A'+(j*2)+k), 0.0);
                        InArmOffSet_File[i]->SingleOffSet->dPlaceOffSet[k][j]  =ReadIniData(szDir, CapStrInput[i], str.sprintf("Place %c",  'A'+(j*2)+k), 0.0);
                    }
                }
            }
            else if(i==InOfsBottom2DID)                                         //KaiChen 20200414 ：新增Bottom2DID OffSet
            {
                for(int j=0; j<InArmSuck.iMotCol; j++)
                {
                    for(int k=0; k<InArmSuck.iMotRow; k++)
                    {
                        InArmOffSet_File[i]->SingleOffSet->dPosOffSetX[k][j]   =ReadIniData(szDir, CapStrInput[i], str.sprintf("Hand %cX",  'A'+(j*2)+k), 0.0);
                        InArmOffSet_File[i]->SingleOffSet->dPosOffSetY[k][j]   =ReadIniData(szDir, CapStrInput[i], str.sprintf("Hand %cY",  'A'+(j*2)+k), 0.0);
                        InArmOffSet_File[i]->SingleOffSet->dPickUpOffSet[k][j] =ReadIniData(szDir, CapStrInput[i], str.sprintf("PickUp %c", 'A'+(j*2)+k), 0.0);
                        InArmOffSet_File[i]->SingleOffSet->dPlaceOffSet[k][j]  =ReadIniData(szDir, CapStrInput[i], str.sprintf("Place %c",  'A'+(j*2)+k), 0.0);
                    }
                }
            }
            else
            {
                for(int j=0; j<InArmSuck.iMotCol; j++)
                {
                    for(int k=0; k<InArmSuck.iMotRow; k++)
                    {
                        InArmOffSet_File[i]->SingleOffSet->dPosOffSetX[k][j]   =0.0;
                        InArmOffSet_File[i]->SingleOffSet->dPosOffSetY[k][j]   =0.0;
                        InArmOffSet_File[i]->SingleOffSet->dPickUpOffSet[k][j] =0.0;
                        InArmOffSet_File[i]->SingleOffSet->dPlaceOffSet[k][j]  =0.0;
                    }
                }
            }
        }
        else
        {
            for(int j=0; j<InArmSuck.iMotCol; j++)
            {
                for(int k=0; k<InArmSuck.iMotRow; k++)
                {
                    InArmOffSet_File[i]->SingleOffSet->dPosOffSetX[k][j]   =ReadIniData(szDir, CapStrInput[i], str.sprintf("Hand %cX",  'A'+(j*2)+k), 0.0);
                    InArmOffSet_File[i]->SingleOffSet->dPosOffSetY[k][j]   =ReadIniData(szDir, CapStrInput[i], str.sprintf("Hand %cY",  'A'+(j*2)+k), 0.0);
                    InArmOffSet_File[i]->SingleOffSet->dPickUpOffSet[k][j] =ReadIniData(szDir, CapStrInput[i], str.sprintf("PickUp %c", 'A'+(j*2)+k), 0.0);
                    InArmOffSet_File[i]->SingleOffSet->dPlaceOffSet[k][j]  =ReadIniData(szDir, CapStrInput[i], str.sprintf("Place %c",  'A'+(j*2)+k), 0.0);
                }
            }
        }
        Offset_File.dInArmPickUp[i]=ReadIniData(szDir, CapStrInput[i], "PickUp",    0.0);           //KenHsieh 20220914 : 新增SECS用
    }

    for(int i=0; i<OutOfsTotal; i++)                                            //Steven 20140425 : 重整Offset
    {
        if(AUTO_EMPTY_COLOR<3)                                                  //Steven 20230907 : For HT-9011UC
        {
            if(i==OutOfsAuto4 || i==OutOfsAuto5 || i==OutOfsAuto6 ||
               i==OutOfsFix4  || i==OutOfsFix5  || i==OutOfsFix6)
            {
                continue;
            }
        }

        OutArmOffSet_File[i]->SetOneByOne (ReadIniData(szDir, CapStrOutput[i], "One By One", 0));
        OutArmOffSet_File[i]->SetVariableY(ReadIniData(szDir, CapStrOutput[i], "VariableY",  0.0)); //ChungHung 20131231 alter AutoYPitch
        OutArmOffSet_File[i]->SetVariable (ReadIniData(szDir, CapStrOutput[i], "Variable",   0.0));
        OutArmOffSet_File[i]->SetVariable2(ReadIniData(szDir, CapStrOutput[i], "Variable2",  0.0)); //Steven 20131002 : XY變距
        OutArmOffSet_File[i]->SetVariable3(ReadIniData(szDir, CapStrOutput[i], "Variable3",  0.0));
        OutArmOffSet_File[i]->SetVariable4(ReadIniData(szDir, CapStrOutput[i], "Variable4",  0.0));
        OutArmOffSet_File[i]->SetX        (ReadIniData(szDir, CapStrOutput[i], "Hand X",     0.0));
        OutArmOffSet_File[i]->SetY        (ReadIniData(szDir, CapStrOutput[i], "Hand Y",     0.0));
        OutArmOffSet_File[i]->SetPickUp   (ReadIniData(szDir, CapStrOutput[i], "PickUp",     0.0));
        OutArmOffSet_File[i]->SetPlace    (ReadIniData(szDir, CapStrOutput[i], "Place",      0.0));

        if(IniConfig.bE33InOutArmZOffsetSameOne==true)                          //jou 2014-10-07 加速 offset 讀取時間
        {
            if(i==OutOfsAuto1 || i==OutOfsPADView_Out ||
               i==OutOfsBGA_Out || i==OutOfsScannerAOI)                         //wei 20160617 Vitrox
            {
                for(int j=0; j<OutArmSuck.iMotCol; j++)
                {
                    for(int k=0; k<OutArmSuck.iMotRow; k++)
                    {
                        OutArmOffSet_File[i]->SingleOffSet->dPosOffSetX[k][j]  =ReadIniData(szDir, CapStrOutput[i], str.sprintf("Hand %cX",  'A'+(j*2)+k), 0.0);
                        OutArmOffSet_File[i]->SingleOffSet->dPosOffSetY[k][j]  =ReadIniData(szDir, CapStrOutput[i], str.sprintf("Hand %cY",  'A'+(j*2)+k), 0.0);
                        OutArmOffSet_File[i]->SingleOffSet->dPickUpOffSet[k][j]=ReadIniData(szDir, CapStrOutput[i], str.sprintf("PickUp %c", 'A'+(j*2)+k), 0.0);
                        OutArmOffSet_File[i]->SingleOffSet->dPlaceOffSet[k][j] =ReadIniData(szDir, CapStrOutput[i], str.sprintf("Place %c",  'A'+(j*2)+k), 0.0);
                    }
                }
            }
            else if(i==OutOfsAuto2 || i==OutOfsAuto3 ||                        //RogerYang 20260126 : 修正Auto1的Pick/Place offset被清成0 //eastsun 20251230 : 獨立偏移功能unloader區也要偏移
                    i==OutOfsAuto4 || i==OutOfsAuto5 || i==OutOfsAuto6 ||    //Steven 20260316 : Fix missing i== and add Auto4~6, Fix4~6
                    i==OutOfsFix1  || i==OutOfsFix2  || i==OutOfsFix3 ||
                    i==OutOfsFix4  || i==OutOfsFix5  || i==OutOfsFix6)
            {
                for(int j=0; j<OutArmSuck.iMotCol; j++)
                {
                    for(int k=0; k<OutArmSuck.iMotRow; k++)
                    {
                        OutArmOffSet_File[i]->SingleOffSet->dPosOffSetX[k][j]  =ReadIniData(szDir, CapStrOutput[i], str.sprintf("Hand %cX",  'A'+(j*2)+k), 0.0);
                        OutArmOffSet_File[i]->SingleOffSet->dPosOffSetY[k][j]  =ReadIniData(szDir, CapStrOutput[i], str.sprintf("Hand %cY",  'A'+(j*2)+k), 0.0);
                        OutArmOffSet_File[i]->SingleOffSet->dPickUpOffSet[k][j]=0.0;
                        OutArmOffSet_File[i]->SingleOffSet->dPlaceOffSet[k][j] =0.0;
                    }
                }
            }
            else
            {
                for(int j=0; j<OutArmSuck.iMotCol; j++)
                {
                    for(int k=0; k<OutArmSuck.iMotRow; k++)
                    {
                        OutArmOffSet_File[i]->SingleOffSet->dPosOffSetX[k][j]  =0.0;
                        OutArmOffSet_File[i]->SingleOffSet->dPosOffSetY[k][j]  =0.0;
                        OutArmOffSet_File[i]->SingleOffSet->dPickUpOffSet[k][j]=0.0;
                        OutArmOffSet_File[i]->SingleOffSet->dPlaceOffSet[k][j] =0.0;
                    }
                }
            }
        }
        else
        {
            for(int j=0; j<OutArmSuck.iMotCol; j++)
            {
                for(int k=0; k<OutArmSuck.iMotRow; k++)
                {
                    OutArmOffSet_File[i]->SingleOffSet->dPosOffSetX[k][j]  =ReadIniData(szDir, CapStrOutput[i], str.sprintf("Hand %cX",  'A'+(j*2)+k), 0.0);
                    OutArmOffSet_File[i]->SingleOffSet->dPosOffSetY[k][j]  =ReadIniData(szDir, CapStrOutput[i], str.sprintf("Hand %cY",  'A'+(j*2)+k), 0.0);
                    OutArmOffSet_File[i]->SingleOffSet->dPickUpOffSet[k][j]=ReadIniData(szDir, CapStrOutput[i], str.sprintf("PickUp %c", 'A'+(j*2)+k), 0.0);
                    OutArmOffSet_File[i]->SingleOffSet->dPlaceOffSet[k][j] =ReadIniData(szDir, CapStrOutput[i], str.sprintf("Place %c",  'A'+(j*2)+k), 0.0);
                }
            }
        }
        Offset_File.dOutArmPickUp[i]=ReadIniData(szDir, CapStrOutput[i], "PickUp",     0.0);        //KenHsieh 20220914 : 新增SECS用
    }

    //Sort Arm
    if(USE_OUT_SORT_ARM!=eartUninstall)                                         //RogerYang 20250417 for HT9046AU add
    {
        for(int i=0; i<SortOfsTotal; i++)
        {
            SortArmOffSet_File[i]->SetOneByOne (ReadIniData(szDir, CapStrSortput[i], "One By One", 0));
            SortArmOffSet_File[i]->SetVariableY(ReadIniData(szDir, CapStrSortput[i], "VariableY",  0.0)); //alter AutoYPitch
            SortArmOffSet_File[i]->SetVariable (ReadIniData(szDir, CapStrSortput[i], "Variable",   0.0));
            SortArmOffSet_File[i]->SetVariable2(ReadIniData(szDir, CapStrSortput[i], "Variable2",  0.0)); //XY變距
            SortArmOffSet_File[i]->SetVariable3(ReadIniData(szDir, CapStrSortput[i], "Variable3",  0.0));
            SortArmOffSet_File[i]->SetVariable4(ReadIniData(szDir, CapStrSortput[i], "Variable4",  0.0));
            SortArmOffSet_File[i]->SetX        (ReadIniData(szDir, CapStrSortput[i], "Hand X",     0.0));
            SortArmOffSet_File[i]->SetY        (ReadIniData(szDir, CapStrSortput[i], "Hand Y",     0.0));
            SortArmOffSet_File[i]->SetPickUp   (ReadIniData(szDir, CapStrSortput[i], "PickUp",     0.0));
            SortArmOffSet_File[i]->SetPlace    (ReadIniData(szDir, CapStrSortput[i], "Place",      0.0));

            if(IniConfig.bE33InOutArmZOffsetSameOne==true)                      //加速 offset 讀取時間
            {
                for(int j=0; j<OutArm2Suck.iMotCol; j++)
                {
                    SortArmOffSet_File[i]->SingleOffSet->dPosOffSetX[0][j]  =0.0;
                    SortArmOffSet_File[i]->SingleOffSet->dPosOffSetY[0][j]  =0.0;
                    SortArmOffSet_File[i]->SingleOffSet->dPickUpOffSet[0][j]=0.0;
                    SortArmOffSet_File[i]->SingleOffSet->dPlaceOffSet[0][j] =0.0;
                }
            }
            else
            {
                for(int j=0; j<OutArm2Suck.iMotCol; j++)                        //吸嘴只有Aa、Ab，定義A、C(左+右)
                {
                    SortArmOffSet_File[i]->SingleOffSet->dPosOffSetX[0][j]  =ReadIniData(szDir, CapStrSortput[i], str.sprintf("Hand %cX",  'A'+(j*2)), 0.0);
                    SortArmOffSet_File[i]->SingleOffSet->dPosOffSetY[0][j]  =ReadIniData(szDir, CapStrSortput[i], str.sprintf("Hand %cY",  'A'+(j*2)), 0.0);
                    SortArmOffSet_File[i]->SingleOffSet->dPickUpOffSet[0][j]=ReadIniData(szDir, CapStrSortput[i], str.sprintf("PickUp %c", 'A'+(j*2)), 0.0);
                    SortArmOffSet_File[i]->SingleOffSet->dPlaceOffSet[0][j] =ReadIniData(szDir, CapStrSortput[i], str.sprintf("Place %c",  'A'+(j*2)), 0.0);
                }
            }
            Offset_File.dSortArmPickUp[i]=ReadIniData(szDir, CapStrSortput[i], "PickUp",     0.0);        //新增SECS用
        }
    }

    //Test Arm
    for(int i=0; i<2; i++)
    {
        str.sprintf("Test Arm%d", i+1);                                         //Steven 20140825 : i --> i+1
        Offset_File.iIndexArmPickUp[i] =ReadIniData(szDir, str, "Pick Up",      0.0);
        Offset_File.iIndexArmPlace[i]  =ReadIniData(szDir, str, "Place",        0.0);
        Offset_File.iIndexArmContact[i]=ReadIniData(szDir, str, "Contact",      0.0);
    }

    //Tray Arm
    for(int i=0; i<=tOfsAuto6; i++)
    {
        if(AUTO_EMPTY_COLOR<3 &&                                            //Steven 20230907 : For HT-9011UC
           (i==tOfsAuto4 || i==tOfsAuto5 || i==tOfsAuto6))
        {
            continue;
        }

        Offset_File.iTrayArmX[i]     =ReadIniData(szDir, SpecialOffSetName[i], "Place", 0.0);
        Offset_File.iTrayArmX_ART[i] =ReadIniData(szDir, SpecialOffSetName[i], "ART-Place", 0.0); //kevin 20170831 (Steven) ART offset
        Offset_File.dTrayZseparate[i]=ReadIniData(szDir, SpecialOffSetName[i], "Loader Z", 0.0);  //Steven 20190813 : 入Tray改用步進馬達
    }

    if(CosFunction.bSaveOffsetByMachine && IniConfig.bA57_3SaveOffsetByMachine) //JimmyChiu 20220618 : save by machine
    {
        szDir=sSaveByMachine;
    }
    else
    {
        szDir=fOffSet->GetOffsetPath();                                         //Steven 20190109 : 整合Offset路徑
    }

    if(Tri_Temp_Machine==1)                                                     //Ztex 2024.03.13 Add Low Temp Offset
        szDir+=Tri_Position_Offset();
    else if(LastSet.iTemperature==Tempture_Hot)
        szDir+="\\Position Offset Hot.Data";
    else
        szDir+="\\Position Offset.Data";

    for(int i=0; i<2; i++)
    {
        InArmOffSet_File[InOfsInSh1+i]->SetX    (ReadIniData(szDir, CapStrInput[InOfsInSh1+i], "Hand X",    0.0));
        InArmOffSet_File[InOfsInSh1+i]->SetY    (ReadIniData(szDir, CapStrInput[InOfsInSh1+i], "Hand Y",    0.0));

        OutArmOffSet_File[OutOfsOutSh1+i]->SetX (ReadIniData(szDir, CapStrOutput[OutOfsOutSh1+i], "Hand X",     0.0));  //Steven 20140510
        OutArmOffSet_File[OutOfsOutSh1+i]->SetY (ReadIniData(szDir, CapStrOutput[OutOfsOutSh1+i], "Hand Y",     0.0));
    }

    for(int i=InOfsInSh1LB; i<=InOfsInSh2RB; i++)                               //Steven 20141006 : Fixed Offset
    {
        InArmOffSet_File[i]->SetX    (ReadIniData(szDir, CapStrInput[i], "Hand X",    0.0));
        InArmOffSet_File[i]->SetY    (ReadIniData(szDir, CapStrInput[i], "Hand Y",    0.0));
    }

    for(int i=OutOfsOutSh1LB; i<=OutOfsOutSh2RB; i++)                           //Steven 20190802 : For 32 Site Out Shuttle Offset
    {
        OutArmOffSet_File[i]->SetX    (ReadIniData(szDir, CapStrOutput[i], "Hand X",    0.0));
        OutArmOffSet_File[i]->SetY    (ReadIniData(szDir, CapStrOutput[i], "Hand Y",    0.0));
    }

    //Test Arm
    for(int i=0; i<2; i++)
    {
        str.sprintf("Test Arm%d", i+1);                                         //Steven 20140825 : i --> i+1
        Offset_File.iSHHalft[i]         =ReadIniData(szDir, str, "Busy SH Halt",  0.0);
        Offset_File.iSHRightPod[i]      =ReadIniData(szDir, str, "Shuttle Right", 0.0);
        Offset_File.iSHLeftPod[i]       =ReadIniData(szDir, str, "Shuttle Left" , 0.0);
        Offset_File.iSHLeft2D[i]        =ReadIniData(szDir, str, "Shuttle for 2D",0.0);      //Steven 20151218 : Offset for 2d reader
    }

    Offset_File.iPreciserOpen=ReadIniData(szDir, "Preciser", "Preciser Open",  0.0);    //Frank 20180410 (Steven) : InArm Preciser Station
    Offset_File.iPreciserClose=ReadIniData(szDir, "Preciser", "Preciser Close",  0.0);

    if(CosFunction.bUseInvisibleOffset)                                         //JerryYang 20250120 : add
    {
        szDir2="D:\\HT9045\\IniData\\DefineOffset\\STD_125.Data";               //強制產生125檔案
        if(FileExists(szDir2)==false)
        {
            ReadInvisibleFile(szDir2);
        }

        szDir2="D:\\HT9045\\IniData\\DefineOffset\\STD_25.Data";                //強制產生25檔案
        if(FileExists(szDir2)==false)
        {
            ReadInvisibleFile(szDir2);
        }

        szDir2.sprintf("D:\\HT9045\\data\\");
        if(!(DirectoryExists(szDir2)))
            ForceDirectories(szDir2);

        szDir2.sprintf("D:\\HT9045\\data\\%s.ini",fMain->cbSetupFileName->Text);
        ReadInvisibleFile(szDir2);
    }

    LoadLoaderScaleValues();
}

// ---- golden :3930-3954 -- called from ReadFile's last line ----
void TfOffSet::LoadLoaderScaleValues()
{
    AnsiString S,aPath;
    S=AnsiString(fMain->cbSetupFileName->Text);

    aPath.sprintf("D:\\HT9045\\Data\\%s.ini",S);

    TestIF_File.bInArmUseDifferentScaleBySetupFile   =ReadIniData(aPath,"LoaderScale","ScanleEnable",false);
    TestIF_File.fLoaderTrayXScaleBySetupFile     =CheckRange(ReadIniData(aPath,"LoaderScale","LoaderX",1.0), 0.95,1.05);
    TestIF_File.fLoaderTrayYScaleBySetupFile     =CheckRange(ReadIniData(aPath,"LoaderScale","LoaderY",1.0), 0.95,1.05);
    TestIF_File.fHotPlateXScaleBySetupFile[0]    =CheckRange(ReadIniData(aPath,"LoaderScale","HotPlate1X",1.0), 0.95,1.05);
    TestIF_File.fHotPlateYScaleBySetupFile[0]    =CheckRange(ReadIniData(aPath,"LoaderScale","HotPlate1Y",1.0), 0.95,1.05);
    TestIF_File.fHotPlateXScaleBySetupFile[1]    =CheckRange(ReadIniData(aPath,"LoaderScale","HotPlate2X",1.0), 0.95,1.05);
    TestIF_File.fHotPlateYScaleBySetupFile[1]    =CheckRange(ReadIniData(aPath,"LoaderScale","HotPlate2Y",1.0), 0.95,1.05);
    TestIF_File.fInShuttleXScaleBySetupFile[0]   =CheckRange(ReadIniData(aPath,"LoaderScale","Shuttle1X",1.0), 0.95,1.05);
    TestIF_File.fInShuttleYScaleBySetupFile[0]   =CheckRange(ReadIniData(aPath,"LoaderScale","Shuttle1Y",1.0), 0.95,1.05);
    TestIF_File.fInShuttleXScaleBySetupFile[1]   =CheckRange(ReadIniData(aPath,"LoaderScale","Shuttle2X",1.0), 0.95,1.05);
    TestIF_File.fInShuttleYScaleBySetupFile[1]   =CheckRange(ReadIniData(aPath,"LoaderScale","Shuttle2Y",1.0), 0.95,1.05);

    TestIF_File.bOutArmUseDifferentScaleBySetupFile  = ReadIniData(aPath,"UnLoaderScale","ScanleEnable",false);
    TestIF_File.fOutShuttleXScaleBySetupFile[0]      = CheckRange(ReadIniData(aPath,"UnLoaderScale","Shuttle1X",1.0), 0.95,1.05);
    TestIF_File.fOutShuttleYScaleBySetupFile[0]      = CheckRange(ReadIniData(aPath,"UnLoaderScale","Shuttle1Y",1.0), 0.95,1.05);
    TestIF_File.fOutShuttleXScaleBySetupFile[1]      = CheckRange(ReadIniData(aPath,"UnLoaderScale","Shuttle1X",1.0), 0.95,1.05);
    TestIF_File.fOutShuttleYScaleBySetupFile[1]      = CheckRange(ReadIniData(aPath,"UnLoaderScale","Shuttle1Y",1.0), 0.95,1.05);
}
