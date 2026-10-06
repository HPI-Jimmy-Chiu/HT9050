# type4 TFT 相關的段落：共用大檔（只看這些行，整檔不要合）

比對字：`NUMBER_PANEL_TYPE`、`MAGAZINE_BIN_DISP_TYPE`、`eTFT`、`*_TFT(`、`BinDisCtrl`、`InstallColorBinDisplay`、`BinRGB`；前後各 2 行。行號＝各自原檔。

## `MachineType.h`

**810_B06**（京元電 HT9046LS）

```
  1188      eHTA18           =1,  //HTA18
  1189      eHTBT008         =2,   //HT-BT008
  1190      eTFT             =3
  1191  };
  1192  
```

**V912**（HT9011UC_Code_V3.33.912.0_20260908_Jimmy）

```
  1428      eHTA18           =1,                                                        //HTA18
  1429      eHTBT008         =2,                                                         //HT-BT008
  1430      eTFT             =3
  1431  };
  1432  
```

## `cmydef.h`

**810_B06**（京元電 HT9046LS）

```
  2688  extern bool         LOAD_Z_USE_MOTOR[9]                 ; //Steven 20190813 : 入Tray改用步進馬達
  2689  extern bool         LOADUNLOAD_USE_CASSETTE[9]          ;
  2690  extern int          NUMBER_PANEL_TYPE                   ;
  2691  extern int          WEIGHT_CALIBRATION                  ; //Steven 20111108
  2692  extern int          ION_FAN_TYPE                        ;
    ...
  2882  extern int  NUDN1_MACID13_AMP_QTY                       ;   //Sam 20210518 : 新增 CanBus 軟體配置
  2883  extern int  AUTO3_IS_MAGAZINE                           ;   //JerryYang 20220909 : add magazine
  2884  extern int  MAGAZINE_BIN_DISP_TYPE                      ;   //JerryYang 20220909 : add magazine
  2885  extern int  VibrationMotorCount                         ;   //JerryYang 20230814 : add震動馬達通訊調速版本
  2886  //Motion Mode----------------------------------------------------------------------
    ...
  3217  extern bool bASMFinishOneCycle;                 //Steven 20120830 : AutoSiteMapping, 手動移除Loader Tray
  3218  extern int iTestBinCount;                       //Steven 20121112 : RS232支援32Bin
  3219  struct tBinRGB { int R; int G; int B; AnsiString Barcode; };  // Eastsun 20260827: barcode column                //Eastsun 20260825 : Mag Bin RGB
  3220  extern tBinRGB BinRGBTable[TEST_MAX_BIN];                //Eastsun 20260825 : Mag Bin RGB
  3221  extern void LoadBinRGBTable();                           //Eastsun 20260825 : Mag Bin RGB
  3222  extern void SaveBinRGBTable();                           //Eastsun 20260825 : Mag Bin RGB
  3223  extern void GetBinRGB(int iBinNo, int iColor, int &R, int &G, int &B);   //Eastsun 20260911 : Bin RGB 沒設定時給TFT預設色
  3224  
  3225  extern bool bWakeupGPIBFile;                    //Steven 20110116
```

**V912**（HT9011UC_Code_V3.33.912.0_20260908_Jimmy）

```
  2862  extern bool         LOAD_Y_USE_MOTOR[9]                 ; //Jimmychiu 20240307 : Loader Tray改用步進馬達
  2863  extern bool         LOADUNLOAD_USE_CASSETTE[9]          ;
  2864  extern int          NUMBER_PANEL_TYPE                   ;
  2865  extern int          WEIGHT_CALIBRATION                  ; //Steven 20111108
  2866  extern int          ION_FAN_TYPE                        ;
    ...
  3083  extern int  NUDN1_MACID14_AMP_QTY                       ;   //Sam 20210518 : 新增 CanBus 軟體配置
  3084  extern int  AUTO3_IS_MAGAZINE                           ;   //JerryYang 20220909 : add magazine
  3085  extern int  MAGAZINE_BIN_DISP_TYPE                      ;   //JerryYang 20220909 : add magazine
  3086  extern int  VibrationMotorCount                         ;   //JerryYang 20230814 : add震動馬達通訊調速版本
  3087  //Motion Mode----------------------------------------------------------------------
```

## `cmydef.cpp`

**810_B06**（京元電 HT9046LS）

```
  2845  bool        LOAD_Z_USE_MOTOR[9];                            //Steven 20190813 : 入Tray改用步進馬達
  2846  bool        LOADUNLOAD_USE_CASSETTE[9];
  2847  int         NUMBER_PANEL_TYPE               =0;
  2848  int         WEIGHT_CALIBRATION              =0;             //Steven 20111108
  2849  int         ION_FAN_TYPE                    =1;             //Steven 20100226
    ...
  3013  int         TRAY_ARM_MODE                   =0;             //Frank 20230419
  3014  int         AUTO3_IS_MAGAZINE               =0;             //JerryYang 20220909 : add magazine
  3015  int         MAGAZINE_BIN_DISP_TYPE          =0;             //JerryYang 20220909 : add magazine
  3016  
  3017  bool bContactCTOverCHK=false;
    ...
  5580  //------------------------------------------------------------------------------
  5581  //Eastsun 20260825 : Mag Bin RGB
  5582  tBinRGB BinRGBTable[TEST_MAX_BIN];
  5583  
  5584  void LoadBinRGBTable()
  5585  {
  5586      int i;
  5587      for(i=0; i<TEST_MAX_BIN; i++)
  5588      {
  5589          BinRGBTable[i].R = -1;
  5590          BinRGBTable[i].G = -1;
  5591          BinRGBTable[i].B = -1;
  5592          BinRGBTable[i].Barcode = ""; // Eastsun 20260827
  5593      }
  5594      AnsiString BinRGBAbsPath = ExtractFilePath(Application->ExeName) + "BinRGBTable.Data";   //Eastsun 20260825 : Mag Bin RGB
  5595      // Eastsun 20260904 : parse the file if it exists, but do NOT return early -
  5596      // the KYEC force-write block below must ALWAYS run
    ...
  5605          {
  5606  //            AnsiString sDbg;
  5607  //            sDbg.sprintf("LoadBinRGBTable: KYEC block fired (CUSTOMER_CODE=%d)", CUSTOMER_CODE);
  5608  //            RecordProcess(sDbg);
  5609          }
    ...
  5623              if(i >= 0 && i < TEST_MAX_BIN)
  5624              {
  5625                  BinRGBTable[i].R = KYECFixed[k].R;
  5626                  BinRGBTable[i].G = KYECFixed[k].G;
  5627                  BinRGBTable[i].B = KYECFixed[k].B;
  5628                  // Barcode is force-written below (per customer spec)
  5629              }
    ...
  5638                  AnsiString bc;
  5639                  bc.sprintf("-%c", (char)('A' + k));
  5640                  BinRGBTable[i].Barcode = bc;
  5641              }
  5642          }
    ...
  5650                  AnsiString bc;
  5651                  bc.sprintf("-B%c", (char)('A' + k));
  5652                  BinRGBTable[i].Barcode = bc;
  5653              }
  5654          }
  5655      }
  5656      if(FileExists(BinRGBAbsPath))
  5657      {
  5658      TStringList *sl = new TStringList;
    ...
  5661          try                                                                                  //Eastsun 20260825 : Mag Bin RGB
  5662          {                                                                                    //Eastsun 20260825 : Mag Bin RGB
  5663              sl->LoadFromFile(BinRGBAbsPath);
  5664          }                                                                                    //Eastsun 20260825 : Mag Bin RGB
  5665          catch(Exception &e) { /* Eastsun 20260904 : swallow, keep going to KYEC block */ }   //Eastsun 20260825 : Mag Bin RGB
    ...
  5699              if(bin>=0 && bin<TEST_MAX_BIN)
  5700              {
  5701                  BinRGBTable[bin].R = R;
  5702                  BinRGBTable[bin].G = G;
  5703                  BinRGBTable[bin].B = B;
  5704                  BinRGBTable[bin].Barcode = sBarcode; // Eastsun 20260827
  5705              }
  5706          }
    ...
  5722          {
  5723  //            AnsiString sDbg;
  5724  //            sDbg.sprintf("LoadBinRGBTable: KYEC block fired (CUSTOMER_CODE=%d)", CUSTOMER_CODE);
  5725  //            RecordProcess(sDbg);
  5726          }
    ...
  5740              if(i >= 0 && i < TEST_MAX_BIN)
  5741              {
  5742                  BinRGBTable[i].R = KYECFixed[k].R;
  5743                  BinRGBTable[i].G = KYECFixed[k].G;
  5744                  BinRGBTable[i].B = KYECFixed[k].B;
  5745                  // Barcode is force-written below (per customer spec)
  5746              }
    ...
  5755                  AnsiString bc;
  5756                  bc.sprintf("-%c", (char)('A' + k));
  5757                  BinRGBTable[i].Barcode = bc;
  5758              }
  5759          }
    ...
  5767                  AnsiString bc;
  5768                  bc.sprintf("-B%c", (char)('A' + k));
  5769                  BinRGBTable[i].Barcode = bc;
  5770              }
  5771          }
    ...
  5773  }
  5774  
  5775  void SaveBinRGBTable()
  5776  {
  5777      TStringList *sl = new TStringList;
    ...
  5782          for(int i=0; i<TEST_MAX_BIN; i++)
  5783          {
  5784              bool rgbValid = (BinRGBTable[i].R>=0 && BinRGBTable[i].R<=255 &&  // Eastsun 20260827
  5785                               BinRGBTable[i].G>=0 && BinRGBTable[i].G<=255 &&
  5786                               BinRGBTable[i].B>=0 && BinRGBTable[i].B<=255);
  5787              bool hasBc = !BinRGBTable[i].Barcode.IsEmpty();
  5788              if(rgbValid || hasBc)
  5789              {
  5790                  AnsiString line;
  5791                  line.sprintf("%d,%d,%d,%d,%s", i, BinRGBTable[i].R, BinRGBTable[i].G, BinRGBTable[i].B, BinRGBTable[i].Barcode.c_str());
  5792                  sl->Add(line);
  5793              }
  5794          }
  5795          AnsiString BinRGBAbsPath = ExtractFilePath(Application->ExeName) + "BinRGBTable.Data";   //Eastsun 20260825 : Mag Bin RGB
  5796          try                                                                                  //Eastsun 20260825 : Mag Bin RGB
  5797          {                                                                                    //Eastsun 20260825 : Mag Bin RGB
  5798              LogCsvChangeBeforeSave(BinRGBAbsPath, sl, "BinRGBTable", 0, false, "BinNo,R,G,B,Barcode");    //Eastsun 20261006 : save change log
  5799              sl->SaveToFile(BinRGBAbsPath);
  5800          }                                                                                    //Eastsun 20260825 : Mag Bin RGB
  5801          catch(Exception &e)                                                                  //Eastsun 20260825 : Mag Bin RGB
  5802          {                                                                                    //Eastsun 20260825 : Mag Bin RGB
  5803              ShowMessage("Save BinRGBTable.Data failed: " + e.Message);                     //Eastsun 20260825 : Mag Bin RGB
  5804          }                                                                                    //Eastsun 20260825 : Mag Bin RGB
  5805      }
    ...
  5813  //Eastsun 20260911 : 取得Bin顯示顏色, 沒設定RGB時回傳與TFT顯示器相同的預設色
  5814  //                   iColor : 1=Red(fail bin)  2=Green(pass bin)  3=Orange(該區未使用), 定義見cShowBinSelect
  5815  void GetBinRGB(int iBinNo, int iColor, int &R, int &G, int &B)
  5816  {
  5817      if(iBinNo>=0 && iBinNo<TEST_MAX_BIN &&
  5818         BinRGBTable[iBinNo].R>=0 && BinRGBTable[iBinNo].R<=255 &&
  5819         BinRGBTable[iBinNo].G>=0 && BinRGBTable[iBinNo].G<=255 &&
  5820         BinRGBTable[iBinNo].B>=0 && BinRGBTable[iBinNo].B<=255)
  5821      {
  5822          R = BinRGBTable[iBinNo].R;
  5823          G = BinRGBTable[iBinNo].G;
  5824          B = BinRGBTable[iBinNo].B;
  5825      }
  5826      else if(iColor==2)      //Green
```

**V912**（HT9011UC_Code_V3.33.912.0_20260908_Jimmy）

```
  3079  bool        LOAD_Y_USE_MOTOR[9];                            //Jimmychiu 20240307 : Loader Tray改用步進馬達
  3080  bool        LOADUNLOAD_USE_CASSETTE[9];
  3081  int         NUMBER_PANEL_TYPE               =0;
  3082  int         WEIGHT_CALIBRATION              =0;                                 //Steven 20111108
  3083  int         ION_FAN_TYPE                    =1;                                 //Steven 20100226
    ...
  3273  int         USE_2nd_LOADER                  =0;                                 //Steven 20240822 : For HT-9046AU
  3274  int         AUTO3_IS_MAGAZINE               =0;                                 //JerryYang 20220909 : add magazine
  3275  int         MAGAZINE_BIN_DISP_TYPE          =0;                                 //JerryYang 20220909 : add magazine
  3276  int         In_Shuttle_Auto_Latch           =0;                                 //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
  3277  bool bInSh1DoLtc=false;
```

## `database.h`

**810_B06**（京元電 HT9046LS）

```
   199      protected:
   200  //        void ChoiceTempController( int iType);
   201          void InstallColorBinDisplay(int iType);
   202      public:
   203  //        TMyTempCtrl *TempCtrl;           // 溫控器
   204          TMyBinDispCtrl  *BinDisCtrl;      // 彩色七段顯示器
   205          _fastcall SYSTEM_MODULAR::SYSTEM_MODULAR();
   206          _fastcall SYSTEM_MODULAR::~SYSTEM_MODULAR();
```

**V912**（HT9011UC_Code_V3.33.912.0_20260908_Jimmy）

```
   199      protected:
   200  //        void ChoiceTempController( int iType);
   201          void InstallColorBinDisplay(int iType);
   202      public:
   203  //        TMyTempCtrl *TempCtrl;           // 溫控器
   204          TMyBinDispCtrl  *BinDisCtrl;      // 彩色七段顯示器
   205          _fastcall SYSTEM_MODULAR::SYSTEM_MODULAR();
   206          _fastcall SYSTEM_MODULAR::~SYSTEM_MODULAR();
```

## `database.cpp`

**810_B06**（京元電 HT9046LS）

```
    74      InitCommonString();             //jou 2016-08-24 Initial Common String
    75      OpenGeneralIniFile();           //Steven 20141120 : Add Read/Write IniFile Speed
    76      BinDisCtrl=NULL;
    77      ATKRecipeInfo=new ATK_RECIPE_INFO();                  //Steven 20170901 (wei) : For ATK要新增工作檔比對用的檔案
    78  //    str="D:\\GPIB9045\\system\\general.ini", "Version", "Model", "9045GPIB";
    ...
   429  
   430      //數字顯示器----------------------------------------------
   431      NUMBER_PANEL_TYPE   =CheckAndReadIniDataGeneral("System",          "NUMBER_PANEL_TYPE",    2);
   432      iNumberPanelDelay   =CheckAndReadIniDataGeneral("NUMBER_PANEL",    "NUMBER_PANEL_DELAY",   1);
   433      //Steven 20120217 : Com Port改成可定義
    ...
   583  //    tFingerInterface.bUseFingerprint =CheckAndReadIniDataGeneral("System", "bUseFingerprint", false);
   584  //    tFingerInterface.bUseFingerprint=false; //jou 2012-02-03 強制關閉，等待PO
   585      MAGAZINE_BIN_DISP_TYPE=CheckAndReadIniDataGeneral("System", "MAGAZINE_BIN_DISP_TYPE", 0);   //JerryYang 20220909 : add magazine
   586  
   587      //ATC
    ...
  1388      MyGem=new HT9045Gem("HT9045", HGem);            //20140213  wei   KYEC SECS/GEM
  1389  
  1390      if(NUMBER_PANEL_TYPE==3 ||
  1391         NUMBER_PANEL_TYPE==4)                                                    //Sam 20240604 : 新增 BinDisplay TFT
  1392          InstallColorBinDisplay(NUMBER_PANEL_TYPE);
  1393  }
  1394  //---------------------------------------------------------------------------
    ...
  1560  //}
  1561  //---------------------------------------------------------------------------
  1562  void SYSTEM_MODULAR::InstallColorBinDisplay(int iType)
  1563  {
  1564  //jou 2013-01-16 沒有new,不能delete
  1565  //    if(BinDisCtrl!=NULL)
  1566  //        delete BinDisCtrl;
  1567  
  1568      BinDisCtrl=new TMyBinDispHT9046;
  1569      LoadBinRGBTable();                                                        //Eastsun 20260825 : Mag Bin RGB
  1570  
  1571      if(BinDisCtrl==NULL)                                                        //Sam 20240604 : 新增 BinDisplay TFT
  1572          return;
  1573  
    ...
  1581      }
  1582  
  1583      BinDisCtrl->SetComPort(sNumberPanelComPort);
  1584      BinDisCtrl->SetComPort2(sNumberPanelComPort2);
  1585  
  1586      for(int i=0; i<eBinDispTotal; i++)  //JerryYang 20220909 : 12->eBinDispTotal  //GG Steven
    ...
  1605          else
  1606          {
  1607              BinDisCtrl->InstalledUnit(i);
  1608          }
  1609          BinDisCtrl->Alias[i]=asTrayFeeder[i];
  1610      }
  1611  
    ...
  1615      if(autoempty==0 && EmptyEmpty==0)
  1616      {
  1617          BinDisCtrl->CloseUnit(1);
  1618          BinDisCtrl->CloseUnit(2);
  1619      }
  1620  
  1621      BinDisCtrl->SetDelayTime(iNumberPanelDelay);
  1622  }
  1623  //---------------------------------------------------------------------------
    ...
  1626      try
  1627      {
  1628          if(NUMBER_PANEL_TYPE==3 ||                                              //2013-04-12    Dell    debug 在不是使用彩色版七段時,關程式發生記憶體錯誤
  1629             NUMBER_PANEL_TYPE==4)                                                //Sam 20240604 : 新增 BinDisplay TFT
  1630          {
  1631              if(BinDisCtrl!=NULL)
  1632              {
  1633                  delete BinDisCtrl;
  1634                  BinDisCtrl=NULL;
  1635              }
  1636          }
```

**V912**（HT9011UC_Code_V3.33.912.0_20260908_Jimmy）

```
    49      InitCommonString();                                                         //jou 2016-08-24 Initial Common String
    50      OpenGeneralIniFile();                                                       //Steven 20141120 : Add Read/Write IniFile Speed
    51      BinDisCtrl=NULL;
    52      ATKRecipeInfo=new ATK_RECIPE_INFO();                                        //Steven 20170901 (wei) : For ATK要新增工作檔比對用的檔案
    53  
    ...
   513  
   514      //數字顯示器----------------------------------------------
   515      NUMBER_PANEL_TYPE   =CheckAndReadIniDataGeneral("System",          "NUMBER_PANEL_TYPE",    2);
   516      dNumberPanelDelay   =CheckAndReadIniDataGeneral("NUMBER_PANEL",    "NUMBER_PANEL_DELAY",   1.0);                    //Sam 20240604 : 顯示器輪巡時間改為 double
   517      //Steven 20120217 : Com Port改成可定義
    ...
   679  //    USE_FINGER_PRINT     =CheckAndReadIniDataGeneral("System",  "USE_FINGER_PRINT", 0);//Steven 20190503 : 指紋辨識權限
   680      USE_FINGER_PRINT=0;                                                         //Steven 20240920 : 移除指紋辨識
   681      MAGAZINE_BIN_DISP_TYPE=CheckAndReadIniDataGeneral("System", "MAGAZINE_BIN_DISP_TYPE", 0);                           //JerryYang 20220909 : add magazine
   682  
   683      //ATC
    ...
  1547      MyGem=new HT9045Gem("HT9045", HGem);                                        //20140213  wei   KYEC SECS/GEM
  1548  
  1549      if(NUMBER_PANEL_TYPE==3 ||
  1550         NUMBER_PANEL_TYPE==4)                                                    //Sam 20240604 : 新增 BinDisplay TFT
  1551          InstallColorBinDisplay(NUMBER_PANEL_TYPE);
  1552  }
  1553  //---------------------------------------------------------------------------
    ...
  1688  }
  1689  //---------------------------------------------------------------------------
  1690  void SYSTEM_MODULAR::InstallColorBinDisplay(int iType)
  1691  {
  1692      BinDisCtrl=new TMyBinDispHT9046;
  1693  
  1694      if(BinDisCtrl==NULL)                                                        //Sam 20240604 : 新增 BinDisplay TFT
  1695          return;
  1696  
    ...
  1702          return;
  1703      }
  1704      BinDisCtrl->SetComPort(sNumberPanelComPort);
  1705      BinDisCtrl->SetComPort2(sNumberPanelComPort2);
  1706  
  1707      for(int i=0; i<eBinDispTotal; i++)                                          //JerryYang 20220909 : 12->eBinDispTotal  //GG Steven
    ...
  1718          else
  1719          {
  1720              BinDisCtrl->InstalledUnit(i);
  1721          }
  1722          BinDisCtrl->Alias[i]=asTrayForBinDisp[i];
  1723      }
  1724  
    ...
  1728      if(autoempty==0 && EmptyEmpty==0)
  1729      {
  1730          BinDisCtrl->CloseUnit(1);
  1731          BinDisCtrl->CloseUnit(2);
  1732      }
  1733  
  1734      BinDisCtrl->SetDelayTime(dNumberPanelDelay);                                //Sam 20240604 : 顯示器輪巡時間改為 double
  1735  }
  1736  //---------------------------------------------------------------------------
    ...
  1739      try
  1740      {
  1741          if(NUMBER_PANEL_TYPE==3 ||                                              //2013-04-12    Dell    debug 在不是使用彩色版七段時,關程式發生記憶體錯誤
  1742             NUMBER_PANEL_TYPE==4)                                                //Sam 20240604 : 新增 BinDisplay TFT
  1743          {
  1744              if(BinDisCtrl!=NULL)
  1745              {
  1746                  delete BinDisCtrl;
  1747                  BinDisCtrl=NULL;
  1748              }
  1749          }
```

## `main.cpp`

**810_B06**（京元電 HT9046LS）

```
  3035      }
  3036  
  3037      if(NUMBER_PANEL_TYPE==2)    //Steven 20110517 : 雙位數Bin顯示器異常
  3038          fShowBinSelect->DoShowBinDigital();
  3039  
    ...
  9619      }
  9620  
  9621      if(NUMBER_PANEL_TYPE==3 ||
  9622         NUMBER_PANEL_TYPE==4)                                                    //Sam 20240604 : 新增 BinDisplay TFT
  9623          HSys.BinDisCtrl->InitialOK=InitialOK;
  9624  
  9625      //if(SHUTTLE_SENSOR_TYPE==eSensorCCLink || SHUTTLE_SENSOR_TYPE==eSensorCCLink3)   //Steven 20131008 : for HT9046AH
    ...
 10643      bSystemClose=true;
 10644  
 10645      if(NUMBER_PANEL_TYPE==3 ||
 10646         NUMBER_PANEL_TYPE==4)                                                    //Sam 20240604 : 新增 BinDisplay TFT
 10647          HSys.BinDisCtrl->InitialOK=InitialOK;
 10648  
 10649      //if(SHUTTLE_SENSOR_TYPE==eSensorCCLink || SHUTTLE_SENSOR_TYPE==eSensorCCLink3)   //Steven 20131008 : for HT9046AH
    ...
 23055      }
 23056  
 23057      if(NUMBER_PANEL_TYPE!=2)    //Steven 20110517 : 雙位數Bin顯示器異常
 23058          fShowBinSelect->DoShowBinDigital();
 23059      if(NUMBER_PANEL_TYPE==3 ||
 23060         NUMBER_PANEL_TYPE==4)                                                    //Sam 20240604 : 新增 BinDisplay TFT
 23061          fShowBinSelect->ChangeBinDispStatus();
 23062  
```

**V912**（HT9011UC_Code_V3.33.912.0_20260908_Jimmy）

```
  3553      }
  3554  
  3555      if(NUMBER_PANEL_TYPE==2)                                                    //Steven 20110517 : 雙位數Bin顯示器異常
  3556          fShowBinSelect->DoShowBinDigital();
  3557  
    ...
 10917      }
 10918  
 10919      if(NUMBER_PANEL_TYPE==3 ||
 10920         NUMBER_PANEL_TYPE==4)                                                    //Sam 20240604 : 新增 BinDisplay TFT
 10921          HSys.BinDisCtrl->InitialOK=InitialOK;
 10922  
 10923      //if(SHUTTLE_SENSOR_TYPE==eSensorCCLink || SHUTTLE_SENSOR_TYPE==eSensorCCLink3)   //Steven 20131008 : for HT9046AH
    ...
 12060      bSystemClose=true;
 12061  
 12062      if(NUMBER_PANEL_TYPE==3 ||
 12063         NUMBER_PANEL_TYPE==4)                                                    //Sam 20240604 : 新增 BinDisplay TFT
 12064          HSys.BinDisCtrl->InitialOK=InitialOK;
 12065  
 12066      if(NUMBER_PANEL_TYPE==3 ||
 12067         NUMBER_PANEL_TYPE==4)                                                    //Ifor 20260810 : stop bin display process and close comm threads while message loop is alive, avoid shutdown hang
 12068      {
 12069          HSys.BinDisCtrl->ProcessStopStart(false);
 12070          try
 12071          {
 12072              if(HSys.BinDisCtrl->CommBin!=NULL)
 12073              {
 12074                  HSys.BinDisCtrl->CommBin->StopComm();
 12075              }
 12076              if(HSys.BinDisCtrl->CommBin2!=NULL)
 12077              {
 12078                  HSys.BinDisCtrl->CommBin2->StopComm();
 12079              }
 12080          }
    ...
 25949      }
 25950  
 25951      if(NUMBER_PANEL_TYPE!=2)                                                    //Steven 20110517 : 雙位數Bin顯示器異常
 25952          fShowBinSelect->DoShowBinDigital();
 25953      if(NUMBER_PANEL_TYPE==3 ||                                                  //Steven 20110411
 25954         NUMBER_PANEL_TYPE==4)                                                    //Sam 20240604 : 新增 BinDisplay TFT
 25955          fShowBinSelect->ChangeBinDispStatus();
 25956  
    ...
 27138              slSnap->Add("");
 27139              slSnap->Add("---- BinDisplay Diag ----");
 27140              if(HSys.BinDisCtrl==NULL)
 27141              {
 27142                  s.sprintf("BinDisCtrl=NULL (NUMBER_PANEL_TYPE=%d, 未建立 Bin 顯示器控制)", NUMBER_PANEL_TYPE);
 27143                  slSnap->Add(s);
 27144              }
 27145              else
 27146              {
 27147                  s.sprintf("NUMBER_PANEL_TYPE=%d  RunStatus=%s  ComPort=%s  ComPort2=%s  Delay=%.2f",
 27148                            NUMBER_PANEL_TYPE, HSys.BinDisCtrl->GetRunStatus().c_str(),
 27149                            HSys.BinDisCtrl->GetComPort().c_str(), HSys.BinDisCtrl->GetComPort2().c_str(),
 27150                            HSys.BinDisCtrl->GetDelayTime());
 27151                  slSnap->Add(s);
 27152                  s.sprintf("TotalInstalledUnit=%d  bC14SaveBinDisplayLog=%d  bG16BinDispNeedAlarm=%d",
 27153                            HSys.BinDisCtrl->GetTotalInstalledUnit(),
 27154                            IniConfig.bC14SaveBinDisplayLog?1:0, IniConfig.bG16BinDispNeedAlarm?1:0);
 27155                  slSnap->Add(s);
    ...
 27160                  for(int i=0; i<eBinDispTotal; i++)
 27161                  {
 27162                      bool bInst=HSys.BinDisCtrl->UnitHasInstall(i);
 27163                      int  iVer =HSys.BinDisCtrl->GetVersion(i);
 27164                      bool bErr =HSys.BinDisCtrl->GerErrNow(i);
 27165                      if(i>eBinDispFix6 && bInst==false && iVer==0 && bErr==false)
 27166                          continue;                                               //常用的 0~11 一律列, 其餘只列有跡象的, 避免 36 行洗版
 27167                      s.sprintf("%3d %-12s %4d %3d %3d %3d %5d",
 27168                                i, asTrayForBinDisp[i].c_str(), bInst?1:0, bErr?1:0, iVer,
 27169                                HSys.BinDisCtrl->GetBinNow(i), HSys.BinDisCtrl->GetColorNow(i));
 27170                      slSnap->Add(s);
 27171                  }
```

## `csystem.cpp`

**810_B06**（京元電 HT9046LS）

```
   132  #include "LaserSensor.h"
   133  #include "ObserveMagazine.h"                                                    //Eastsun 20260116 :
   134  #include "database.h"            //Eastsun 20260513 KYEC: HSys.BinDisCtrl
   135  #pragma package(smart_init)
   136  
    ...
  3997                  CheckSystemPower=false;
  3998          SystemStart=false;
  3999          HSys.BinDisCtrl-> bFirstInit=true;
  4000          HSys.BinDisCtrl->ProcessStopStart(true);
  4001          RecordProcess("SnSystemPower Off");//kevin 20180319 add log
  4002      }
    ...
  5851      }
  5852  
  5853      if(HSys.BinDisCtrl!=NULL)
  5854          HSys.BinDisCtrl->FlashPro(pos);                                      //Eastsun 20260514
  5855  
  5856      switch(Task)
    ...
  6315              }
  6316              bAutoDuplicateErr[pos]=false;
  6317              if(HSys.BinDisCtrl!=NULL)
  6318                  HSys.BinDisCtrl->ClearAutoChangingWarn(pos);                                         //Eastsun 20260514 : Clear Flash
  6319              return true;
  6320      }
```

**V912**（HT9011UC_Code_V3.33.912.0_20260908_Jimmy）

```
  4494                  CheckSystemPower=false;
  4495          SystemStart=false;
  4496          HSys.BinDisCtrl-> bFirstInit=true;
  4497          HSys.BinDisCtrl->ProcessStopStart(true);
  4498          RecordProcess("SnSystemPower Off");                                     //kevin 20180319 add log
  4499      }
    ...
  6939      }
  6940  
  6941      if(HSys.BinDisCtrl!=NULL)
  6942          HSys.BinDisCtrl->FlashPro(Pos);                                             //Eastsun 20260514
  6943  
  6944      switch(Task)
    ...
  7645              }
  7646  
  7647              if(HSys.BinDisCtrl!=NULL)
  7648                  HSys.BinDisCtrl->ClearAutoChangingWarn(Pos);                        //Eastsun 20260514 : Clear Flash
  7649              return true;
  7650      }
```

## `cBinSel.cpp`

**810_B06**（京元電 HT9046LS）

```
    24  #include "mymessbox.h"
    25  #include "cAuthority.h"
    26  #include "BinRGBSetting.h"                                                     //Eastsun 20260825 : Mag Bin RGB
    27  #include "cSecurity.h"
    28  #include "BarcodeReader.h"          // 2013.11.29 , Joye , KYEC Barcode Reader  20140103 wei
    ...
  1657      if(AUTO3_IS_MAGAZINE==1)
  1658      {
  1659          btnBinRGB->Visible=true;
  1660      }
  1661      else
  1662      {
  1663          btnBinRGB->Visible=false;
  1664      }
  1665  
    ...
  1731      }
  1732  
  1733      if(NUMBER_PANEL_TYPE==3 ||
  1734         NUMBER_PANEL_TYPE==4)                                                    //Sam 20240604 : 新增 BinDisplay TFT
  1735          HSys.BinDisCtrl->ProcessStopStart(false);
  1736  
  1737      if(CUSTOMER_CODE==CC_SCC || CUSTOMER_CODE==CC_SCK)   //Steven 20101221   //ChungHung 20130621 add SCK RMS
    ...
  6810  //---------------------------------------------------------------------------
  6811  // Eastsun 20260825 : Mag Bin RGB
  6812  void __fastcall TfBinSel::btnBinRGBClick(TObject *Sender)
  6813  {
  6814      if(fBinRGBSetting != NULL)
  6815          fBinRGBSetting->ShowModal();
  6816  }
  6817  //---------------------------------------------------------------------------
```

**V912**（HT9011UC_Code_V3.33.912.0_20260908_Jimmy）

```
  1755      }
  1756  
  1757      if(NUMBER_PANEL_TYPE==3 ||
  1758         NUMBER_PANEL_TYPE==4)                                                    //Sam 20240604 : 新增 BinDisplay TFT
  1759          HSys.BinDisCtrl->ProcessStopStart(false);
  1760  
  1761      if(CUSTOMER_CODE==CC_SCC ||                                                 //Steven 20101221
```

## `cShowBinSelect.cpp`

**810_B06**（京元電 HT9046LS）

```
   172      for(i=0; i<eBinDispTotal; i++)  //JerryYang 20220909 : 12->eBinDispTotal
   173      {
   174          if(HSys.BinDisCtrl->UnitHasInstall(i))
   175          {
   176              bErrFlag[i]=HSys.BinDisCtrl->GerErrNow(i);
   177          }
   178          else
    ...
   210          }
   211          else if(bErrFlag[i])// ||                                               //Eastsun 20260327 : 有些沒有裝FIX 但有FIX顯示器123456
   212             //(i>=3 && HSys.BinDisCtrl->UnitHasInstall(i)==false)) //Steven 20211221 : Loader, empty, color可能沒裝
   213          {
   214              bHasError=true;
    ...
   246      for(i=0; i<eBinDispTotal; i++)  //JerryYang 20220909 : 12->eBinDispTotal
   247      {
   248  //        if(HSys.BinDisCtrl->UnitHasInstall(i))
   249          {
   250              iColor=HSys.BinDisCtrl->GetColorNow(i);
   251              iBin=HSys.BinDisCtrl->GetBinNow(i);
   252  
   253              if(i>=27)                         //kevin 20140317 256 bin 0 start
    ...
   298              bBinDispAlarm=true;
   299              #ifndef SOFT_SIMULTE
   300  //            HSys.BinDisCtrl->CommBin->StopComm();
   301              HSys.BinDisCtrl->bFirstInit=true;
   302              HSys.BinDisCtrl->ProcessStopStart(true);
   303              ShowMyMessage(AnsiString("Please check bin display. It have communication error! [")+asAlarmLog+"]", AnsiString("請確認Bin顯示器的狀態。")+" ["+asAlarmLog+"]");   //Eastsun 20260820 加入位置編碼
   304              //RecordProcess("Please check bin display. It have communication error! [")+asAlarmLog+"]");
    ...
   316          {
   317              #ifndef SOFT_SIMULTE
   318              HSys.BinDisCtrl->CommBin->StopComm();
   319              HSys.BinDisCtrl->bFirstInit=true;
   320              HSys.BinDisCtrl->ProcessStopStart(true);
   321              ShowMyMessage(AnsiString("Please check bin display. It have communication error! [")+asAlarmLog+"]", AnsiString("請確認Bin顯示器的狀態。")+" ["+asAlarmLog+"]");
   322              AlarmDelay.SetSecAndOn(60);
    ...
   335      else
   336      {
   337          sbRunStatus->Panels->Items[0]->Text=HSys.BinDisCtrl->GetRunStatus();
   338          sbRunStatus->Color=clBtnFace;
   339      }
    ...
   612              UnLoadLabel[i]->Caption=". . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ";
   613              UnLoadPanel[i]->Caption="X";
   614              if(NUMBER_PANEL_TYPE==3)                                           
   615              {
   616                  UnLoadPanel[i]->Color=(TColor)0x000080FF;
    ...
   631              UnLoadLabel[i]->Caption=". . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ";
   632              UnLoadPanel[i]->Caption="X";
   633              if(NUMBER_PANEL_TYPE==3 ||                                          //三色顯示器
   634                 NUMBER_PANEL_TYPE==4)                                            //Sam 20240604 : 新增 BinDisplay TFT
   635              {
   636                  UnLoadPanel[i]->Color=(TColor)0x000080FF;
    ...
   681  void __fastcall TfShowBinSelect::FormShow(TObject *Sender)
   682  {
   683      if(NUMBER_PANEL_TYPE==3 ||                                          //三色顯示器
   684         NUMBER_PANEL_TYPE==4)                                            //Sam 20240604 : 新增 BinDisplay TFT
   685          bUpdateBinDigital=true;
   686      else
    ...
   845  void __fastcall TfShowBinSelect::ShowBinDigital()
   846  {
   847      if(NUMBER_PANEL_TYPE==3 ||
   848         (NUMBER_PANEL_TYPE==4 && MAGAZINE_BIN_DISP_TYPE!=eMagBinUninstall))      //Sam 20240604 : 新增 BinDisplay TFT
   849          return;
   850  
   851      if(NUMBER_PANEL_TYPE!=0)            //Steven 20091001
   852      {
   853          int i, j;
    ...
   856          for(i=0; i<12; i++)
   857              iArray[i]=-1;
   858          if(NUMBER_PANEL_TYPE==2)        //2 Digital
   859          {
   860              iArray[0]=111;     //L
    ...
   862              iArray[2]=102;     //C
   863          }
   864          else if(NUMBER_PANEL_TYPE==1)   //1 Digital
   865          {
   866              iArray[0]=21;      //L
    ...
   963  {
   964      if(bUpdateBinDigital==true &&
   965         (NUMBER_PANEL_TYPE==3 ||                                                 //Steven : 彩色顯示器
   966          NUMBER_PANEL_TYPE==4))                                                  //Sam 20240604 : 新增 BinDisplay TFT
   967      {
   968          int i, j, k, Data, iBinSelCT=0;
    ...
  1284              }
  1285              //<== Eastsun 20260513
  1286              HSys.BinDisCtrl->WriteTargetBin(i, iBinSet[i], iBinColor[i]);
  1287          }
  1288  
  1289          bUpdateBinDigital=false;
  1290  
  1291          HSys.BinDisCtrl->ProcessStopStart(true);
  1292          return;
  1293      }
  1294  
  1295      if(NUMBER_PANEL_TYPE==0)
  1296          return; //Steven 20100512 : 沒有安裝
  1297      if(bUpdateBinDigital==false)
```

**V912**（HT9011UC_Code_V3.33.912.0_20260908_Jimmy）

```
   232      for(int i=0; i<eBinDispTotal; i++)                                          //JerryYang 20220909 : 12->eBinDispTotal
   233      {
   234          if(HSys.BinDisCtrl->UnitHasInstall(i))
   235          {
   236              bErrFlag[i]=HSys.BinDisCtrl->GerErrNow(i);
   237          }
   238          else
    ...
   262          }
   263          else if(bErrFlag[i] ||
   264                  (i>=3 && HSys.BinDisCtrl->UnitHasInstall(i)==false))            //Steven 20211221 : Loader, empty, color可能沒裝
   265          {
   266              bHasError=true;
    ...
   318      for(int i=0; i<eBinDispTotal; i++)                                          //JerryYang 20220909 : 12->eBinDispTotal
   319      {
   320          if(HSys.BinDisCtrl->UnitHasInstall(i))
   321          {
   322              iColor=HSys.BinDisCtrl->GetColorNow(i);
   323              if(iColor<0 || iColor>=(int)(sizeof(ColorMap)/sizeof(TColor)))     //Ifor 20260827 : guard ColorMap index (inline sizeof , a named const gets folded and raises W8080)
   324              {
   325                  iColor=0;
   326              }
   327              iBin=HSys.BinDisCtrl->GetBinNow(i);
   328  
   329              if(bErrFlag[i]==false)
    ...
   376              bBinDispAlarm=true;
   377              #ifndef SOFT_SIMULTE
   378  //            HSys.BinDisCtrl->CommBin->StopComm();
   379              HSys.BinDisCtrl->bFirstInit=true;
   380              HSys.BinDisCtrl->ProcessStopStart(true);
   381              ShowMyMessage("Please check bin display. It have communication error!", "請確認Bin顯示器的狀態。");
   382              #endif
    ...
   396              sTempEng=AnsiString().sprintf("%s Error part:%s", "Please check bin display. It have communication error!", sTempChi);
   397              sTempChi=AnsiString().sprintf("%s 異常位置:%s", "請確認Bin顯示器的狀態!", sTempChi);
   398              HSys.BinDisCtrl->CommBin->StopComm();
   399              HSys.BinDisCtrl->bFirstInit=true;
   400              HSys.BinDisCtrl->ProcessStopStart(true);
   401              ShowMyMessage(sTempEng, sTempChi);
   402              AlarmDelay.SetSecAndOn(60);
    ...
   413      else
   414      {
   415          sbRunStatus->Panels->Items[0]->Text=HSys.BinDisCtrl->GetRunStatus();
   416          sbRunStatus->Color=clBtnFace;
   417      }
    ...
   753              UnLoadLabel[i]->Caption=". . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ";
   754              UnLoadPanel[i]->Caption="X";
   755              if(NUMBER_PANEL_TYPE==3)    //三色顯示器
   756              {
   757                  UnLoadPanel[i]->Color=(TColor)0x000080FF;
    ...
   772              UnLoadLabel[i]->Caption=". . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ";
   773              UnLoadPanel[i]->Caption="X";
   774              if(NUMBER_PANEL_TYPE==3 ||                                          //三色顯示器
   775                 NUMBER_PANEL_TYPE==4)                                            //Sam 20240604 : 新增 BinDisplay TFT
   776              {
   777                  UnLoadPanel[i]->Color=(TColor)0x000080FF;
    ...
   819  void __fastcall TfShowBinSelect::FormShow(TObject *Sender)
   820  {
   821      if(NUMBER_PANEL_TYPE==3 ||
   822         NUMBER_PANEL_TYPE==4)                                                    //Sam 20240604 : 新增 BinDisplay TFT
   823          bUpdateBinDigital=true;
   824      else
    ...
   941  void __fastcall TfShowBinSelect::ShowBinDigital()
   942  {
   943      if(NUMBER_PANEL_TYPE==3 ||
   944         (NUMBER_PANEL_TYPE==4 && MAGAZINE_BIN_DISP_TYPE!=eMagBinUninstall))      //Sam 20240604 : 新增 BinDisplay TFT
   945  
   946          return;
   947  
   948      if(NUMBER_PANEL_TYPE!=0)                                                    //Steven 20091001
   949      {
   950          int i, j;
    ...
   953          for(i=0; i<12; i++)
   954              iArray[i]=-1;
   955          if(NUMBER_PANEL_TYPE==2)        //2 Digital
   956          {
   957              iArray[0]=111;     //L
    ...
   959              iArray[2]=102;     //C
   960          }
   961          else if(NUMBER_PANEL_TYPE==1)   //1 Digital
   962          {
   963              iArray[0]=21;      //L
    ...
  1061  {
  1062      if(bUpdateBinDigital==true &&
  1063         (NUMBER_PANEL_TYPE==3 ||                                                 //Steven : 彩色顯示器
  1064          NUMBER_PANEL_TYPE==4))                                                  //Sam 20240604 : 新增 BinDisplay TFT
  1065      {
  1066          int Data;//, iBinSelCT=0;
    ...
  1364                  if(iAOIUnit>=eBinDispMag1 && iAOIUnit<=eBinDispMag14)
  1365                  {
  1366                      bUnitIsTFT=(MAGAZINE_BIN_DISP_TYPE==eTFT);
  1367                  }
  1368                  else
  1369                  {
  1370                      bUnitIsTFT=(NUMBER_PANEL_TYPE==4);
  1371                  }
  1372                  if(bUnitIsTFT==true)
    ...
  1426              }
  1427              //<== Eastsun 20260513
  1428              HSys.BinDisCtrl->WriteTargetBin(i, iBinSet[i], iBinColor[i]);
  1429          }
  1430  
  1431          bUpdateBinDigital=false;
  1432  
  1433          HSys.BinDisCtrl->ProcessStopStart(true);
  1434          return;
  1435      }
  1436  
  1437      if(NUMBER_PANEL_TYPE==0)
  1438          return;                                                                 //Steven 20100512 : 沒有安裝
  1439      if(bUpdateBinDigital==false)
```

## `cSortCT.cpp`

**810_B06**（京元電 HT9046LS）

```
   311          ASE_Yield[0]+= AYield;                   //kevin 20170816 (Steven) add 傳送YIELD 給ASE
   312  
   313          if(NUMBER_PANEL_TYPE==4)                                            //Sam 20240604 : 新增 BinDisplay TFT
   314              HSys.BinDisCtrl->WriteTargetCount(i+3, LastSet.BinCT[0][i]);
   315      }
   316  
```

**V912**（HT9011UC_Code_V3.33.912.0_20260908_Jimmy）

```
   405              AYield.sprintf("%s=%s,", s6TrayName[i], myCountPanel[i].pnlYield->Caption);                                 //kevin 20170816 (Steven) add 傳送YIELD 給ASE
   406              ASE_Yield[0]+=AYield;                                               //kevin 20170816 (Steven) add 傳送YIELD 給ASE
   407              if(NUMBER_PANEL_TYPE==4)                                            //Sam 20240604 : 新增 BinDisplay TFT
   408                  HSys.BinDisCtrl->WriteTargetCount(iTo3Unload[i]+3, LastSet.BinCT[0][iTo3Unload[i]]);
   409          }
   410          //AI(ht9045-v912) 20260923: CASE-PTI-20260923-002 力成被 rotate 鎖住的 Fix1 仍比照 V899.37 顯示百分比與數量(只寫畫面，不計入總數、不送 ASE/Bin 顯示器)
```

## `HandlerSys.cpp`

**810_B06**（京元電 HT9046LS）

```
   118      ArmZAtoZH->ItemIndex=InOutArmPickerUseMotor;
   119  
   120      rgNumberPanelType->ItemIndex=NUMBER_PANEL_TYPE;
   121      rgIonFanType->ItemIndex=ION_FAN_TYPE;
   122      rgShuttleSensor->ItemIndex=SHUTTLE_SENSOR_TYPE;
    ...
   178      rgHeatGun->ItemIndex        =CheckAndReadIniDataGeneral("System", "INSTALL_HEAT_GUN",   0);  //kevin 20120523 : 選擇熱風槍機構模式
   179      rgATCHeatGun->ItemIndex     =CheckAndReadIniDataGeneral("System", "INSTALL_ATC_HEAT_GUN",   0);  //JerryYang 20220408 : add for ATC3.5
   180      rgMagBinDispType->ItemIndex =CheckAndReadIniDataGeneral("System", "MAGAZINE_BIN_DISP_TYPE",     0);  //JerryYang 20220909 : add magazine
   181  
   182      rgMotionCard->ItemIndex     =CheckAndReadIniDataGeneral("System", "MOTION_CARD_TYPE",   0);  //Brian 20121015 : 選擇Motion Card 模式
    ...
   501      fMain->InitialSuperVisorPassword(CUSTOMER_CODE);
   502  //----------------------------------
   503      WriteIniDataGeneral("System", "NUMBER_PANEL_TYPE", rgNumberPanelType->ItemIndex);
   504  //    NUMBER_PANEL_TYPE=rgNumberPanelType->ItemIndex;
   505  //----------------------------------
   506      WriteIniDataGeneral("System",    "WEIGHT_CALIBRATION", rgWeightCali->ItemIndex);    //Steven 20111108
    ...
   623      WriteIniDataGeneral("System", "USE_LASER_DISTANCE", rgLaserDistance->ItemIndex);        //Steven 20140228 : 雷射測距功能
   624      WriteIniDataGeneral("System", "USE_DEVICE_FLIPPER", rgDeviceFlipper->ItemIndex);        //Frank 20210612 : Flipper Function
   625      WriteIniDataGeneral("System", "MAGAZINE_BIN_DISP_TYPE", rgMagBinDispType->ItemIndex);   //JerryYang 20220909 : add magazine
   626  //----------------------------------
   627      WriteIniDataGeneral("System", "MACHINE_HAS_AUTO_ALIGNMENT_CCD", rgCCDAutoAlignmentMode->ItemIndex);     //ChungHung 20210113 add for Alignment CCD  //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
```

**V912**（HT9011UC_Code_V3.33.912.0_20260908_Jimmy）

```
   215      ArmZAtoZH->ItemIndex=InOutArmPickerUseMotor;
   216  
   217      rgNumberPanelType->ItemIndex=NUMBER_PANEL_TYPE;
   218      rgIonFanType->ItemIndex=ION_FAN_TYPE;
   219      rgShuttleSensor->ItemIndex=SHUTTLE_SENSOR_TYPE;
    ...
   300      rgHeatGun->ItemIndex        =CheckAndReadIniDataGeneral("System", "INSTALL_HEAT_GUN",   0);             //kevin 20120523 : 選擇熱風槍機構模式
   301      rgATCHeatGun->ItemIndex     =CheckAndReadIniDataGeneral("System", "INSTALL_ATC_HEAT_GUN",   0);         //JerryYang 20220408 : add for ATC3.5
   302      rgMagBinDispType->ItemIndex =CheckAndReadIniDataGeneral("System", "MAGAZINE_BIN_DISP_TYPE", 0);         //JerryYang 20220909 : add magazine
   303  
   304      rgMotionCard->ItemIndex     =CheckAndReadIniDataGeneral("System", "MOTION_CARD_TYPE",   0);             //Brian 20121015 : 選擇Motion Card 模式
    ...
   688      fMain->InitialSuperVisorPassword(CUSTOMER_CODE);
   689  //----------------------------------
   690      WriteIniDataGeneral("System", "NUMBER_PANEL_TYPE", rgNumberPanelType->ItemIndex);
   691  //    NUMBER_PANEL_TYPE=rgNumberPanelType->ItemIndex;
   692  //----------------------------------
   693      WriteIniDataGeneral("System",    "WEIGHT_CALIBRATION", rgWeightCali->ItemIndex);    //Steven 20111108
    ...
   836      WriteIniDataGeneral("System", "USE_LASER_DISTANCE", rgLaserDistance->ItemIndex);        //Steven 20140228 : 雷射測距功能
   837      WriteIniDataGeneral("System", "USE_DEVICE_FLIPPER", rgDeviceFlipper->ItemIndex);        //Frank 20210612 : Flipper Function
   838      WriteIniDataGeneral("System", "MAGAZINE_BIN_DISP_TYPE", rgMagBinDispType->ItemIndex);   //JerryYang 20220909 : add magazine
   839  //----------------------------------
   840      WriteIniDataGeneral("System", "MACHINE_HAS_AUTO_ALIGNMENT_CCD", rgCCDAutoAlignmentMode->ItemIndex);     //ChungHung 20210113 add for Alignment CCD  //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
```

## `Motor/myMN200motor.cpp`

**810_B06**（京元電 HT9046LS）

```
  1127                  SW[SwMagazineMotorBreaker].On();    //Ifor 20240227
  1128                  //Steven 20110621 Start : 關電後要重新Init Bin Disp & Power Off On一次
  1129                  if(NUMBER_PANEL_TYPE==3 ||                                      //Steven 20120106 : 不是使用七段顯示器的話，Reset Ring會記憶體破壞
  1130                     NUMBER_PANEL_TYPE==4)                                        //Sam 20240604 : 新增 BinDisplay TFT
  1131                  {
  1132                      HSys.BinDisCtrl->bFirstInit=true;
  1133                      HSys.BinDisCtrl->ProcessStopStart(true);
  1134                  }
  1135                  //Steven 20110621 End
    ...
  1985          bCheckPCI_L112StateRun=false;
  1986          if(iWriteErrorLogCT!=0) //Sam 20230508 : 修正24V斷電後 Bin顯示器顯示異常。
  1987              HSys.BinDisCtrl->ProcessStopStart(true);
  1988          iWriteErrorLogCT=0;
  1989          return 1;
```

**V912**（HT9011UC_Code_V3.33.912.0_20260908_Jimmy）

```
  1171              MySleep(500*iRatio);
  1172  
  1173              if(NUMBER_PANEL_TYPE==3 ||                                          //Steven 20120106 : 不是使用七段顯示器的話，Reset Ring會記憶體破壞
  1174                 NUMBER_PANEL_TYPE==4)                                            //Sam 20240604 : 新增 BinDisplay TFT
  1175              {
  1176                  HSys.BinDisCtrl->bFirstInit=true;                               //Steven 20110621 Start : 關電後要重新Init Bin Disp & Power Off On一次
  1177                  HSys.BinDisCtrl->ProcessStopStart(true);
  1178              }
  1179  
    ...
  2080          bCheckPCI_L112StateRun=false;
  2081          if(iWriteErrorLogCT!=0) //Sam 20230508 : 修正24V斷電後 Bin顯示器顯示異常。
  2082              HSys.BinDisCtrl->ProcessStopStart(true);
  2083          iWriteErrorLogCT=0;
  2084          return 1;
```

## `acatchtray.cpp`

**810_B06**（京元電 HT9046LS）

```
    32  #include "uLotInfo.h"
    33  #include "Magazine.h"
    34  #include "database.h"            //Eastsun 20260513 KYEC: HSys.BinDisCtrl
    35  //---------------------------------------------------------------------------
    36  #pragma package(smart_init)
    ...
  3788          return false;
  3789  
  3790      if(HSys.BinDisCtrl!=NULL)
  3791          HSys.BinDisCtrl->FlashPro(AutoTarget);                                  //Eastsun 20260514
  3792  
  3793      if(Task==1150)      //標配,改以sensor Enable作判斷
    ...
  6858              else if(DoPlaceTrayToAuto(Target))
  6859              {
  6860                  if(HSys.BinDisCtrl!=NULL)
  6861                  HSys.BinDisCtrl->ClearAutoChangingWarn(Target-1); //Eastsun 20260513 : 顯示器閃爍功能
  6862  //                bCatchTrayFinishAction=true;          //wei 20161219 (Steven) Tray Mapping 移到Task=400
  6863                  ret=WhichAutoNeedTray();
```

**V912**（HT9011UC_Code_V3.33.912.0_20260908_Jimmy）

```
    36  #include "uLotInfo.h"
    37  #include "common.h"
    38  #include "database.h"                                                           //Eastsun 20260513 KYEC: HSys.BinDisCtrl
    39  //---------------------------------------------------------------------------
    40  #pragma package(smart_init)
    ...
  4014      TColor cPtr[2]  ={clWhite, clYellow};
  4015      AnsiString str1 ="";
  4016      if(HSys.BinDisCtrl!=NULL)
  4017          HSys.BinDisCtrl->FlashPro(AutoTarget);                                      //Eastsun 20260514
  4018  
  4019      if(Task==1150)                                                              //標配,改以sensor Enable作判斷
    ...
  7429              else if(DoPlaceTrayToAuto(Target))
  7430              {
  7431                  if(HSys.BinDisCtrl!=NULL)
  7432                      HSys.BinDisCtrl->ClearAutoChangingWarn(Target-1); //Eastsun 20260513 : 顯示器閃爍功能
  7433                  ret=WhichAutoNeedTray();
  7434                  if(USE_LdUldCassetteMode==1)                                    //RogerYang 20260203 : Add for HT9046CR
```
