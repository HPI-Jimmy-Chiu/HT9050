# ST02-C7 唯讀規劃：Lot Info 的 FTP 存檔鈕（E-019 LI-9）與 Observer 的 SG_JamCount 查詢（E-019 OB-7）

- 讀者：Steven、St02-M、St01（ST01-M／ST01-E）、筆電（Jimmy）。摘要在第一節，看完第一節就知道現況與下一步；後面各節是證據（檔:行）。
- 依據：工作卡 ST02-C7（D:\HT9045\docs\handoff\FROM_STEVEN.md §4，20261002 04:47，commit 7e08c936）原文：「read-only FTP plan for LI-9 / OB-7 ... a plan for Lot Info's FTP Save buttons (btnFtpServerClick, golden uLotInfo.cpp:5001-5223) and Observer's SG_JamCount query (cObserver.cpp:5361-5369, StatisticalJamCount :5060) ... what exists, what's gated, who owns which line; docs only」。
- 盤點人：St02-E 的唯讀 helper。只讀不改程式、沒有建置、沒有執行任何 exe／node；本檔是這次唯一新增的檔。
- 對照版本：移植樹與網頁＝origin/main d218cc07（在 D:\AI_TempFile\st02-s14 讀；表中路徑以 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\ 為根，網頁以 D:\HT9045\web\ 為根）。golden＝906 樹 D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven（Steven 20260927 裁決：本機先用這一份；cp950 解碼），下文「golden」一律指這棵樹。網頁 title 與部分 C++ 註解裡的行號是 V912，跟這裡不同。
- 裁決依據：RULINGS_20261001 第 0 條（照 golden 忠實翻譯）、S25（客戶碼專屬的程式不移植）、W36＝C、W36-1、W58（Q1～Q5）、W46-1、W60＝B、W62、R120、R136（全文在 D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md）。
- 日期：2026-10-02

---

## 一、摘要（先看這一節就好）

### (B) Observer「SG_JamCount」查詢（OB-7）：本體已翻好，只差接線，約 1 天

- **golden 做什麼**：System Message 分頁下的 SG_JamCount 子分頁有兩顆鈕，Query Now／Query Yesterday（golden cObserver.cpp:5361-5369），只各呼叫一次 StatisticalJamCount(false／true)（:5060-5276）。它讀「今天／昨天」的事件記錄 CSV（D:\HT9045_Log\EventLogTxt\YYYY\MM\EventLogTxt_YYYYMMDD.csv，**不是 MDB**），把 JAM01～JAM19 開頭的代碼逐一計數填進表格，寫一份統計檔 D:\HT9045_Log\EventLogTxt\SGJamCount\YYYY\MM\<HandlerID>_YYYYMMDD_RawData.csv。**沒有任何訊息框**。按 Query Yesterday 之後 iOneDayLoaderCount 會被歸零（golden :5262-5263，照翻）。FTP 上傳只在 [N26] 開著時做，而 N26 只有矽格（SIGURDFunction）看得到 ⇒ S25。
- **移植樹現況**：本體全部已照 golden 翻在 cObserver.cpp:3320-3613（筆電 58c66991，0818），只有 FTP 上傳尾段 #if 0（GATE Q5a，cObserver.cpp:3524-3530）。開機那一次呼叫已接（FileRW/MainBoot.cpp:191-216，由 tools/wb_serve.cpp:4269 呼叫）。網頁兩顆鈕在（web/page/Data.Observer.html:130，沒有 disabled），**但沒有任何事件、沒有 WS 指令**（observer.get 的 act 清單 cObserver.cpp:7931-7935 沒有它），回覆也沒有帶表格與 Loader Count。
- **St02 可以做**：一支新檔 JsonBridge/actions/ObserverSGJam.cpp（act.observerSG.queryNow／queryYesterday，只呼叫 golden 那兩支）＋一支新網頁檔 web/page/ht9045_observer_sgjam.js；St02 自己的三行（JsonBridge/ChanAction.cpp:347、CMakeLists.txt:3398、tests/CMakeLists.txt:3955）同一行附加；只需要認領 St01 頁 Data.Observer.html 一行（:234 行尾）。
- **前提**：等 St01 的 E-021（f3e2574b，在 origin/v906/st01-q59）先上 main —— E-021 正在改同一頁（Data.Observer.html:234、ht9045_observer_wire.js）、同一支 cObserver.cpp（檔尾 act.observer.*）與 ChanAction.cpp:344。第 36 批只合 q59 到 a4a51e4a，E-021 不在裡面。

### (A) Lot Info「FTP」分頁的存檔鈕（LI-9）：幾乎全缺，分三段做，St02 約 8～10 天

- **golden 做什麼**：FTP 分頁的 Server／HD／Tester Name／DataFTP Save to Data 四顆共用 btnFtpServerClick（uLotInfo.cpp:5001-5126，用 Tag 0～3 分辨），通過權限與 SystemStart 檢查後打開 KYEC 的 FTP 對話框 TfFTPClient（KYECFTP/FTPClient.cpp ShowFTPModal :1215-1573）：Tag 0 連 [FTP] 伺服器列出 *.zip、下載工作檔（plSLoadClick :847-1102）；Tag 1 列出本機工作檔、上傳（plUnloadClick :1111-1212）或從本機換檔（plLoadClick :1575-1653）；Tag 2 設定 Tester Name（btSafeTasterNameClick :1868-1953）。下載成功後由程式自己按隱藏的「Save to handler」btSaveSetupFileClick（uLotInfo.cpp:5128-5223）把 X_NET 存成 X、清掉其他工作檔。Tag 3（TSMC）、Connection test（GIGAS）、DownLoad by Device ID（JSCC_OS）都是 S25。
- **移植樹現況**：btnFtpServerClick、btSaveSetupFileClick、整個 TfFTPClient 表單（沒有 class、沒有 fFTPClient 全域）、TfLotInfo::DownloadFromServer（golden :4182-4704，523 行）、ClearAllSetupFile（:11833-11890）**都沒有**。網頁 FTP 分頁只有一段「未接」說明（web/page/Data.LotInfo.html:129-134），沒有任何 WS 指令。已經有的：筆電翻好的 KYECFTP 傳輸四支（LoadFileFormServer2／UploadFileToServer2 等，KYECFTP/FTPClient_Transfer.cpp），建在 MiniFtpEngine 上；但 (1) wb_serve 沒有連這個庫（CMakeLists.txt:3441-3444），(2) 引擎預設模擬模式、全樹沒有人切成真連線（FTPClient_Transfer.cpp:374／:661）⇒ **今天 Handler 端沒有任何 FTP 會真的傳檔**。St01 已放好「下載成功後解開 START」的兩支安裝座（WebStart.cpp:3951、:3979），沒有呼叫者，註解寫明頁面與指令是 St02 的 W62。
- **已裁決、不必再問**：W62（下載在主迴圈同步做，只准閒置、機台內沒 IC；上機驗證用真的工作檔資料夾，不導沙盒）；W60＝B（KYEC 這組留在 MiniFtpEngine、照 golden 被動）；W36＝C／W36-1／W58（模擬版跟出貨版一樣真的連，但每次啟動 [FTP] Enable FTP 先當成沒勾，SimNet/SimNetMask.cpp:32；操作員勾選存檔後才連，config.ini 照寫）；**W58 Q5 只管「\\」或對應磁碟的存檔路徑（W906_SimNetPathBlocked，common.cpp:2785），不管 FTP 主機**；ctest 一律不連真 FTP（decisions-decided.md D-f）。
- **St02 可以做（三段）**：F1 對話框外殼＋本機三件（列本機工作檔、從本機換檔、Tester Name），不碰網路；F2 伺服器列表＋下載＋上傳（含翻 DownloadFromServer、btSaveSetupFileClick、ClearAllSetupFile）；F3 由各擁有者解開已存在的 FTP 閘（ckernel、BarcodeReader、Command.cpp 的 SECS／GPIB 遠端下載）。
- **擋住的**：筆電檔 KYECFTP/FTPClient_Transfer.cpp 五行同一行認領（真連線開關＋測試掛鉤、Gate #2 接上 DownloadFromServer）；WebRecipeChange.cpp 兩支 static 函式要開放（:261、:389）；fMain->ShowTestHeadComp 是空殼（forms/fMain.cpp:247）；St01 頁 Data.LotInfo.html 四行認領（:130-133）；FTP 對話框放哪（新視窗或分頁內）請 ST01-M 定。

### 要問的（很短；第六節）

- 給 Steven：**沒有**。golden 與既有裁決都答得出來。
- 給 ST01-M：(1) OB-7、LI-9 是否歸 St02（E-019 表寫「待確認」）；(2) FTP 對話框做成新視窗還是 Lot Info 分頁內的覆蓋層（W62 說等 St01 的新頁面題 N-3，本機找不到 N-3 原文）。
- 給筆電：FTPClient_Transfer.cpp 五行同一行認領、CMake 連結可不可以（第四節 4.5）。

---

## 二、(B) Observer SG_JamCount（OB-7）

### 2.1 golden（906_0625_Steven）

**畫面（cObserver.dfm）**：tsMDBQuery（System Message，:1323）→ pgcMessage（:1326）→ tsSGJamCount（Caption 'SG_JamCount'，:1839-1913）。上方 Panel2 有 labLoaderCount（:1851-1865）、Label21「Loader Count :」（:1866-1878）、btnSG_QueryNow「Query Now」（:1879-1887）、btnSG_QueryYesterday「Query Yesterday」（:1888-1896）；下方 strngrdJamLog（TStringGrid，:1898-1912）。**沒有圖表**。全樹沒有任何地方設 tsSGJamCount->TabVisible（只有 cObserver.h:294 宣告）⇒ 這個子分頁永遠看得到；golden cObserver.cpp:379 依 CosFunction.bUseMDB 只藏 tsMDB 那一個子分頁。表頭在建構子 TfObserver::TfObserver 寫（cObserver.cpp:137 起，:305-317）：No、UnitName、AlarmCode、Message、Count、Rate (%)，ColCount=6。

**兩顆鈕（cObserver.cpp:5361-5369，原文）**

```
5361: void __fastcall TfObserver::btnSG_QueryNowClick(TObject *Sender)
5362: {
5363:     StatisticalJamCount(false);
5364: }
5365: //---------------------------------------------------------------------------
5366: void __fastcall TfObserver::btnSG_QueryYesterdayClick(TObject *Sender)
5367: {
5368:     StatisticalJamCount(true);
5369: }
```

**StatisticalJamCount（cObserver.cpp:5060-5276，節錄原文）**

```
5060: void TfObserver::StatisticalJamCount(bool bIsNextDay)                           //Sam 20210224 : Auto Upload FTP JAMRawData 功能 //KaiChen 20200618 ：矽格，增加Jam統計頁面
5061: {
5062:     if(InitialOK==false)
5063:     {
5064:         return;
5065:     }
...
5075:     AnsiString HTPath=slEventLog->Path;
5076:     AnsiString HTFileName=slEventLog->FileName;
...
5089:     if(bIsNextDay)                                                              //上傳時間回剛好跨日所以要用昨天時間
5090:     {
5091:         Year    =SystemYearYesterday;
5092:         Month   =SystemMonthYesterday;
5093:         Date    =SystemDateYesterday;
5094:     }
...
5102:     sPathName.sprintf("%s\\%04d\\%02d", HTPath, Year, Month);
5103:     MyForceDirectories(sPathName);
5104: 
5105:     sFileName.sprintf("%s\\%s_%04d%02d%02d.csv", sPathName, HTFileName, Year, Month, Date);
5106:     if(FileExists(sFileName)==false)
5107:     {
5108:         asErr.sprintf("JamRawData is error. EventLog is not exist. %s",sFileName);
5109:         RecordProcess(asErr);
5110:         return;
5111:     }
...
5117:     tsLogFile->LoadFromFile(sFileName);
5118:     Str="JAM";
...
5121:     for(int i=1; i<strngrdJamLog->RowCount; i++)
5122:     {
5123:         strngrdJamLog->Rows[i]->Clear();
5124:     }
5125: 
5126:     strngrdJamLog->RowCount=2;
5127: 
5128:     for(int i=1; i<tsLogFile->Count; i++)
5129:     {
5130:         tsRow->Clear();
5131:         tsRow->CommaText=tsLogFile->Strings[i];
5132:         if(tsRow->Count>3)
5133:         {
5134:             if(tsRow->Strings[3].AnsiPos(Str)==1)
5135:             {
5136:                 if(StatisticalJamCountEnable(tsRow->Strings[3])==true)
5137:                 {
...（:5138-5150 在表格第 2 欄找同一個 AlarmCode）
5152:                     if(bNewCode)
5153:                     {
5154:                         strngrdJamLog->Cells[0][x]=x;                           //No
5155:                         strngrdJamLog->Cells[1][x]=tsRow->Strings[2];           //UnitName
5156:                         strngrdJamLog->Cells[2][x]=tsRow->Strings[3];           //AlarmCode
5157:                         strngrdJamLog->Cells[3][x]=tsRow->Strings[7];           //Message
5158:                         strngrdJamLog->Cells[4][x]=1;
5159:                         x++;
5160:                         strngrdJamLog->RowCount++;
5161:                     }
5162:                     else
5163:                     {
5164:                         int aaa=StrToInt(strngrdJamLog->Cells[4][iii]);
5165:                         strngrdJamLog->Cells[4][iii]=aaa+1;
5166:                     }
...
5176:             if(iOneDayLoaderCount>0)
5177:             {
5178:                 iJamCnt=StrToInt(strngrdJamLog->Cells[4][j]);
5179:                 dAverage=ChangeToFloat((double)iJamCnt, (double)iOneDayLoaderCount);    //Steven 20250820 : 針對除以0加上保護
5180:                 AnsiString asAverage;
5181:                 asAverage.printf("%0.2f", dAverage);
5182:                 strngrdJamLog->Cells[5][j]=asAverage;
5183:             }
5184:             else
5185:             {
5186:                 strngrdJamLog->Cells[5][j]=0;
5187:             }
...
5191:     if(IniConfig.asA32_1_HandlerID=="")
5192:     {
5193:         asHandlerID="HandlerID";
5194:     }
...
5201:     tsLogLog=new TMyStringList("D:\\HT9045_Log\\EventLogTxt",
5202:                                  asHandlerID,
5203:                                 "Date, Time, No, UnitName, AlarmCode, Message, Count, Rate (%), LoaderCount");
...
5207:     tsLogLog->MySaveSGJamCountToFile(true, bIsNextDay);
...（:5209-5239 每一個 JAM 列 AddTextWithDateTime 後 MySaveSGJamCountToFile(false, ...)；一列都沒有時寫一列空白＋LoaderCount）
5241:     if(bIsNextDay && IniConfig.bN26_UseJamRawDataUpdataToFTP)                   //Sam 20210224 : Auto Upload FTP JAMRawData 功能
5242:     {
...
5250:         sPathName.sprintf("%s\\SGJamCount\\%04d\\%02d", HTPath, Year, Month);
...
5257:             sFileName.sprintf("%s_%04d%02d%02d_RawData.csv", HTFileName, Year, Month, Date);
5258:         }
5259:         fFTPClient->UploadFileFTP(sPathName, sFileName, IniConfig.sN26_FTPUplaodPath, sFileName, IniConfig.sN26_FTPUserName, IniConfig.sN26_FTPPassword,IniConfig.sN26_FTPHost,__FUNC__);
5260:     }
5261: 
5262:     if(bIsNextDay)
5263:         iOneDayLoaderCount=0;
...
5268:     labLoaderCount->Caption=(AnsiString)iOneDayLoaderCount;
```

**輔助函式**：StatisticalLoaderCount（:5278-5287，iOneDayLoaderCount++ 後寫 D:\HT9045_Log\EventLogTxt\SGJamCount\LoaderCount.txt [Loader] Count）、ReadLoaderCount（:5289-5299，讀回同一檔）、StatisticalJamCountEnable（:5301-5329，讀 SGJamCount\JamCountEnable.ini [JamCountEnable] 01～19，缺鍵時 CheckAndReadIniData 寫入預設 1，common.cpp:449-464；代碼以 JAM01～JAM19 開頭且對應鍵為 1 才算）。存檔本體 TMyStringList::MySaveSGJamCountToFile（Public/MyStringList.cpp:742-804：bDelete=true 先刪當天檔，之後每列 append）。

**呼叫者（golden 全樹）**

| 誰呼叫 | 檔:行 | 條件 |
|---|---|---|
| Query Now／Query Yesterday 兩顆鈕 | cObserver.cpp:5363、:5368 | 無（鈕永遠看得到、按得下去） |
| 開機 TfMain::FormShow | main.cpp:11137-11141（ReadLoaderCount＋StatisticalJamCount()） | IniConfig.bN26_UseJamRawDataRecord |
| 每天 00:00 TFormHS 計時器 | HS_Function.cpp:177-178（StatisticalJamCount(true)） | IniConfig.bN26_UseJamRawDataRecord |
| Loader 每吸一盤 | ainarm9045.cpp:2769-2770（StatisticalLoaderCount） | IniConfig.bSIGURDFunction（S25） |

**讀寫的檔**

| 動作 | 路徑 | 出處 |
|---|---|---|
| 讀 | <slEventLog->Path>\YYYY\MM\<FileName>_YYYYMMDD.csv（預設 D:\HT9045_Log\EventLogTxt\YYYY\MM\EventLogTxt_YYYYMMDD.csv；TByDay 命名 MyStringList.cpp:335） | cObserver.cpp:5102-5117；slEventLog 建構 main.cpp:1501-1512 |
| 建資料夾 | 同上 YYYY\MM（MyForceDirectories） | :5103 |
| 讀／缺鍵寫 | D:\HT9045_Log\EventLogTxt\SGJamCount\JamCountEnable.ini（每一個 JAM 列都讀 19 次） | :5308-5316 |
| 刪後重寫 | D:\HT9045_Log\EventLogTxt\SGJamCount\YYYY\MM\<HandlerID>_YYYYMMDD_RawData.csv | :5201-5239；MyStringList.cpp:772-802 |
| 寫 | 程序記錄 RecordProcess「JamRawData is error. EventLog is not exist. <檔名>」（CSV 不存在時） | :5108-5109 |
| 上傳（S25） | 上面那份 RawData.csv → [JamRawDataUpdataToFTP] sN26_FTPHost／sN26_FTPUplaodPath | :5241-5260 |

**設定與客戶條件**

- InitialOK（:5062）；IniConfig.asA32_1_HandlerID（空字串時檔名用「HandlerID」，:5191-5198）。
- IniConfig.bN26_UseJamRawDataUpdataToFTP／sN26_*：Configuration N26 分頁只有 CosFunction.bUseJamRawData 時才顯示可改（cConfiguration.cpp:3757-3772，否則 bFixedValue 0）；bUseJamRawData 只有 SIGURDFunction() 設 true（CosFunction.cpp:3452-3456；預設 false :4318）⇒ **FTP 尾段＝S25**。
- IniConfig.bSPILFunction 時事件記錄 CSV 的表頭不同（main.cpp:1501-1506，第 3 欄是 OccurDateTime 不是 AlarmCode）⇒ SPIL 機台這頁統計不到東西（S25，照 golden）。
- CosFunction.bUseMDB **不影響**這一頁：統計只讀事件記錄 CSV。

**操作員看到的**：沒有任何 ShowMyMessage／訊息框。表格（No、UnitName、AlarmCode、Message、Count、Rate (%)）與 Loader Count 數字。CSV 不存在時直接 return，表格保留上一次的內容、Loader Count 也不更新。

**改到的全域**：iOneDayLoaderCount（Query Yesterday 時歸零，:5262-5263）；strngrdJamLog 各格與 RowCount；labLoaderCount->Caption。沒有寫 LastSet。

**golden 的怪處（照翻，不修）**

1. Query Yesterday 會把**今天**的 iOneDayLoaderCount 歸零（:5262-5263 原本是給 00:00 跨日用的），之後今天的 Rate (%) 分母從 0 重新算；LoaderCount.txt 要等下一次 StatisticalLoaderCount（只有矽格）才會被覆寫。
2. 非矽格機台 iOneDayLoaderCount 永遠是 0（只有 bSIGURDFunction 才累加，ainarm9045.cpp:2769）⇒ Rate (%) 一律是 0；Loader Count 顯示 0。
3. 只算 JAM01～JAM19 開頭（:5322-5324）：JAM00xx、JAM2xxx 不算。
4. 每一個 JAM 列都重讀 JamCountEnable.ini 19 次（:5136 → :5312-5316）。
5. RowCount++ 會在表格最後多一列空白（:5160）。
6. MySaveSGJamCountToFile 提早返回用 AND（MyStringList.cpp:750），移植樹已記在 docs/GOLDEN_DEFECT_LEDGER.md 第 10 列。

### 2.2 移植樹現況

| 部件 | 移植樹位置 | 狀態 | 誰的 |
|---|---|---|---|
| strngrdJamLog | forms/fObserver.h:568（TfObserverGrid）；建構 cObserver.cpp:348；表頭 :617-629 | 有 | 筆電（jimmychiu） |
| labLoaderCount | forms/fObserver.h:846-849（用 TPanel 當替身，有 Caption） | 有 | 筆電 |
| StatisticalJamCount | cObserver.cpp:3320-3547（golden 逐行；Rows[i]->Clear 改成清每一格 S-b :3383-3387、RowCount++ 改成 +1 S-a :3424） | 已翻，ACTIVE | 筆電 58c66991（FW-Q5，20260818） |
| FTP 上傳尾段 | cObserver.cpp:3524-3530 #if 0 | **GATE (Q5a)** | 筆電 |
| StatisticalLoaderCount／ReadLoaderCount／StatisticalJamCountEnable | cObserver.cpp:3549-3559、:3561-3572、:3574-3603 | 已翻 | 筆電 |
| btnSG_QueryNowClick／btnSG_QueryYesterdayClick | cObserver.cpp:3605-3608、:3610-3613（註解「NOT wired to any button」） | 已翻，沒有接線 | 筆電 58c66991／61d86509 |
| 寫檔轉向 | cObserver.cpp:3314-3318 W906_EventLogRootQ5()：環境變數 W906_EVENTLOG_ROOT 沒設＝golden 字面「D:\HT9045_Log\EventLogTxt」 | 有 | 筆電 |
| slEventLog（讀的那一份 CSV） | LogObjects.cpp:101-107（golden main.cpp:1501-1512，路徑 as9045LogPath+"\\EventLogTxt"，ctest 由 W906_HT9045LOG_ROOT 轉向，common.cpp:240）；wb_serve 開機在 tools/wb_serve.cpp:4166 建立 | 有（ctest 裡是 NULL，除非測試自己建） | St02（80bcd1fb cMyDB P1） |
| 開機呼叫 | FileRW/MainBoot.cpp:191-216 W906_FRWBoot_JamRawDataRecord，tools/wb_serve.cpp:4269 呼叫（在 :4166 建 slEventLog 之後） | 已接（:184-187 的註解說 slEventLog 是 NULL，已過期；現在 :200 的守衛會成立） | Steven 4dee7107（提交訊息沒寫 St01／St02；同檔 a99d8e6c 是 St01） |
| 每天 00:00 呼叫 | 沒有：golden HS_Function.cpp:177-178 在 TFormHS::TimerAutoBackupTimer 裡，移植樹整支沒翻（forms/fHS.h:235、:691 GATE Cat A；forms/fHS.cpp 沒有本體） | 缺；條件是 S25 的 N26 | 筆電（fHS） |
| Loader 計數呼叫 | ainarm9045.cpp:6526-6529 GATE W7D-K2-G12（理由「沒有這個方法」已過期，NB2 列 LIKELY-STALE） | 閘住；本體條件 bSIGURDFunction＝S25，不動 | 筆電 |
| 網頁 | web/page/Data.Observer.html:130：子分頁頁籤 data-t="3"「SG_JamCount」、Panel2、labLoaderCount、Label21、btnSG_QueryNow（left 818，NOOVERLAP 左移 6px）、btnSG_QueryYesterday、strngrdJamLog（佔位表格「執行期填值」）。兩顆鈕**沒有 disabled**，按了沒反應 | 畫面在、沒接 | 頁面 St01（E-019 表）；:130 最後由 8b2a666a（機台 EastSun NOOVERLAP-C）改 |
| WS 指令 | observer.get → W906_ObserverJson（cObserver.cpp:7924 起；act 清單 :7931-7935：open、timer、tab、rowNo、form、year、month、file、filter、query、cc*、yield*）**沒有 SG**；回覆也不帶 strngrdJamLog／labLoaderCount | 缺 | St01（8314e3bd 寫明「cObserver.cpp (St01 section only)」；f259d400） |
| 網頁 JS | web/page/ht9045_observer_wire.js：沒有 btnSG_* 的 listener（全 web/page 的 *.js 0 筆） | 缺 | Steven（St01 S116 系列）＋筆電 a53614c9 |
| 伺服器防連點 | WebCmdGuard.cpp:113-114 kObserverActs（只列 observer.get 的讀取型 act） | — | — |
| St01 E-021（**還沒上 main**） | origin/v906/st01-q59 f3e2574b：新 WS act.observer.<op>（ChanAction.cpp:344 同一行 → cObserver.cpp 檔尾 W906_ObserverAct :8311-9441）、新檔 ht9045_observer_ev.js、Data.Observer.html:234 同一行、ht9045_observer_wire.js :237／:701。涵蓋 OB-1/2/3/4/6/8/9，**不含 OB-7**（也不含 OB-5） | St01 正在改 cObserver.cpp | St01 |

**移植樹的 FTP 層（整理；LI-9 也用這張）**

| 東西 | 位置 | 狀態 | 誰的 |
|---|---|---|---|
| Nmftp::TNMFTP（golden TNMFTP 的自寫替代） | KYECFTP/MiniFtpEngine.{h,cpp}；庫 ht9045_nmftp（CMakeLists.txt:3097-3106） | 只會被動（PASV）；**預設模擬模式**，SetSimMode(false) 才真的開 socket（MiniFtpEngine.h:479-484）；模擬模式沒有測試掛鉤時 Connect 失敗（tests/test_FTPClient_Transfer.cpp:250-252） | 筆電 8aa5ac0d（W5-Final）；R0 拆庫只動 CMake（St02） |
| NMFTP1* 事件處理（含 NMFTP1ListItem 列檔） | KYECFTP/FTPClient_EventHandlers.{h,cpp}；全域 tmpList／bTempList／bListOk／bError；畫面寫入走 FTPClientEvt_LogSink 掛鉤 | 已翻；:94 自述「No TfFTPClient facade exists anywhere yet」 | 筆電 |
| LoadFileFormServer2／UploadFileToServer2／Download_2DSortingList／Download_2DID_WhiteList | KYECFTP/FTPClient_Transfer.{h,cpp}（宣告 .h:98-101；全域 FTP_DownloadFail／bPIDTransferErr／NMFTP3 .cpp:346-348）；庫 ht9045_kyecftp（CMakeLists.txt:3108-3135） | 已翻；**wb_serve 沒連**（CMakeLists.txt:3441-3444）；全樹除測試外沒人呼叫；引擎在 :374／:661 用 new TNMFTP(NULL) 建立後從不 SetSimMode(false)；fLotInfo->DownloadFromServer 是回 false 的替身（Gate #2，.cpp:255-271） | 筆電 |
| TfFTPClient 表單／fFTPClient 全域 | 沒有（BarcodeReader.h:44-48 B-F1；Command.cpp:15327 A5） | 缺 | — |
| 等著 fFTPClient 的閘 | ckernel.cpp:2593-2595、:2639-2641（SAFETY-GATE W906-MSTATE-FTP，`fFTPClient->bShow==false` 當成「FTP 視窗沒開」）；BarcodeReader.cpp（B-F1 兩處）；Command.cpp:9722-9725、:11963-11967、:14479-14482、:16792-16795（SECS／GPIB 遠端下載 ShowFTPModal(0)，#if 0）；csystem.cpp:8988-8989（#if 0）；cObserver.cpp:3528-3530（Q5a）；AutoRetest.cpp:328（檔內替身 W906ART_FTPCLIENT，ShowFTPModal 空函式） | 閘住 | 筆電（ckernel、Command、csystem、cObserver、AutoRetest）；BarcodeReader 筆電 |
| TfFTP（golden ProductionInfo 的 FTP 類別） | ProductionInfo/TfFTP.{h,cpp}（ht9045_sm），建在 MiniFtpEngine 上 | 唯一使用者在 #if 0（HANA） | 筆電 8c5e3fb6 |
| ELA 的 FTP | EventLogAnalysis/ElaFtp.{h,cpp}、ElaFtpWinInet.cpp（CMakeLists.txt:1373-1378，WinINet）；Null／LogOnly／WinINet 三種傳輸，W36＝C 兩組態都裝 WinINet（ElaFtp.h:33-43） | 唯一真的會連 FTP 的地方 | St02（53bf2655、44c8bf7f） |
| 模擬版網路開關 | SimNet/SimNetMask.cpp:32（N06 [FTP] Enable FTP）、:33（N06-1）、:61（N26-1）；common.cpp:2785-2808 W906_SimNetPathBlocked（W58 Q5，只看路徑字串） | 有 | St02（W58）；筆電 c9cc2aaa 修過 |

### 2.3 相依與擋住的

1. **要不要 MDB：不要。** golden 統計只讀事件記錄 CSV；CosFunction.bUseMDB 為 false 時 golden 只藏 tsMDB 子分頁（cObserver.cpp:379），SG_JamCount 照常。移植樹 bUseMDB 已鎖死 false（56319e02「SQLite 退場」），事件記錄 CSV 由 St02 的 cMyDB CSV 計畫寫（LogObjects.cpp slEventLog＋SaveEventLog），檔名與 golden 相同（TByDay，Public/MyStringList.cpp 同 golden :335／:491）。
2. **slEventLog 不能是 NULL**：wb_serve 在 tools/wb_serve.cpp:4166 建立；ctest 裡預設 NULL（LogObjects.cpp 檔頭），測試要自己建（tests/test_observer_core.cpp:528 先例）。golden 不會是 NULL，所以這是移植樹多加的拒絕，不是改行為。
3. **InitialOK**：golden 第一行就 return（:5062）；wb_serve 在 PumpInit 之後才為真。
4. **網路**：只有 N26 尾段會碰 FTP ⇒ S25，GATE Q5a 保持關；模擬版 N26-1 本來就被 SimNetMask 每次啟動關掉（SimNetMask.cpp:61）。這一列**不需要**任何 FTP 傳輸。
5. **E-021 先上 main**（St01 在 cObserver.cpp、Data.Observer.html:234、ht9045_observer_wire.js、ChanAction.cpp:344 都有改）。
6. **筆電的 tools/websync/sync_web.py**：新 js 要加進 OURS（同 li12、E-021 先例），由筆電改。
7. **會寫真檔**：D:\HT9045_Log\EventLogTxt\SGJamCount\（JamCountEnable.ini、RawData.csv）—— ctest 要轉向，並加進 real-file watch。

### 2.4 St02 的做法（建議）

**(1) St02 新檔 `JsonBridge/actions/ObserverSGJam.cpp`（手寫，St02）**

- 函式：`std::string W906_ObserverSGJamAct(const std::string& cmd, const std::string& payloadJson)`。
  - `act.observerSG.queryNow` → `fObserver->btnSG_QueryNowClick(nullptr)`（golden :5361-5364）。
  - `act.observerSG.queryYesterday` → `fObserver->btnSG_QueryYesterdayClick(nullptr)`（golden :5366-5369）。
  - 只呼叫、不改 cObserver.cpp 任何一行。
- 移植樹多加的拒絕（都不改 golden 行為，只是不讓它當掉）：fObserver 是 NULL → "not-ready"；slEventLog 是 NULL → "log-objects-not-created"（不呼叫，什麼都不寫）。InitialOK 為 false 時照樣呼叫（golden 自己 return），回覆標 `initialOk:false`。
- 回覆（JsonWriter，同 WebLotInfo.cpp 的寫法）：`executed`、`op`、`grid`（RowCount、6 欄每一格，含最後那一列空白）、`labLoaderCount`（Caption）、`iOneDayLoaderCount`、`eventLogCsv`（照 golden :5102-5105 同樣算出的路徑＋是否存在；唯讀 FileExists）、`sgJamCountCsv`（照 MyStringList.cpp:772-778 算出的路徑）、`ftpTail`（bIsNextDay 且 bN26_UseJamRawDataUpdataToFTP 時回 "gated (Q5a, S25)"）。
- 分派：St02 自己的 **JsonBridge/ChanAction.cpp:347**（act.main.testerConnect 那一行，572de694，St02 P2e）同一行加：`if (cmd.compare(0, 15, "act.observerSG.") == 0) { std::string W906_ObserverSGJamAct(const std::string&, const std::string&); return W906_ObserverSGJamAct(cmd, payloadJson); }`，**放在該行 `//` 註解之前**（同一行加呼叫要放在註解前面）。前綴第 13 個字元是 'S' 不是 '.'，E-021 :344 的 `act.observer.` 判斷不會吃掉它。
- 編譯：St02 自己的 **CMakeLists.txt:3398**（8f34b568，wb_serve 來源行）與 **tests/CMakeLists.txt:3955**（572de694，test_sjson_chan 來源行）同一行加 `JsonBridge/actions/ObserverSGJam.cpp`（放在 `#` 註解之前）。新檔只用 fObserver（cObserver.cpp，ht9045_sm）、slEventLog（cmydef）、JsonWriter（ht9045_webbridge），兩個目標都已經連。
- 權杖與防連點：act.* 一律要權杖、走 WebCmdGuard 的預設（同一指令＋同一 value 在冷卻內擋掉），不必加白名單列（這兩個 op 會寫檔，本來就不該進讀取名單）。

**(2) St02 新檔 `web/page/ht9045_observer_sgjam.js`（手寫，St02）**

- 綁 btnSG_QueryNow／btnSG_QueryYesterday 的 click → 送 act.observerSG.*（權杖的取法照 E-021 的 ht9045_observer_ev.js，等它上 main 再讀）；送出中與 ack 後冷卻內同一顆鈕不再送（Steven 0926「全部按鈕要防連點」，HT9045Busy）。
- 用回覆畫 strngrdJamLog（表頭 No、UnitName、AlarmCode、Message、Count、Rate (%)，照 golden :312-317）與 labLoaderCount。CSV 不存在時 golden 什麼都不顯示、表格不變 ⇒ 頁面也不清表格，只在狀態列寫一行（移植樹的說明，不是訊息框）。

**(3) 要認領的別人的行（一行）**

| 檔:行 | 擁有者 | 舊 | 新 | 前後行 |
|---|---|---|---|---|
| web/page/Data.Observer.html:234 | St01 頁（現行最後 8314e3bd；E-021 會在行尾再加 `<script src="ht9045_observer_ev.js">`） | E-021 上 main 後的整行 | 同一行行尾再加 `<script src="ht9045_observer_sgjam.js"></script><!-- AI(W906-OB7) 2026xxxx (St02-E)：SG_JamCount 兩顆鈕 → act.observerSG.* -->` | :233 `<script src="ht9045_wire_dataobserver.js"></script>`；:235 `<script>` |

- 行數不變。cObserver.cpp、forms/fObserver.h、ht9045_observer_wire.js 都不動。
- 另請筆電在 tools/websync/sync_web.py OURS 加 `page/ht9045_observer_sgjam.js`。

**(4) ctest 想法（St02 新檔 tests/test_st02_observer_sgjam.cpp，ctest 名 St02_ObserverSGJam；STEVEN-NB3 只編譯，請 St01 代跑）**

- 隔離：W906_EVENTLOG_ROOT（SGJamCount 寫入）與 W906_HT9045LOG_ROOT 都指到 build 下的 scratch；測試自己 new 一個指到 scratch 的 slEventLog（tests/test_observer_core.cpp:518-535 先例），結束時把 InitialOK、slEventLog、iOneDayLoaderCount、IniConfig.asA32_1_HandlerID、bN26_UseJamRawDataUpdataToFTP 還原；不連網路（這條路本來就沒有 socket）。
- 案例：(a) queryNow：今天的 CSV 放 JAM0301×2、JAM0302、WAR16102、JAM0001、JAM2001 → 表格只有 JAM0301（Count 2）、JAM0302（Count 1），iOneDayLoaderCount=100 時 Rate「2.00」／「1.00」，labLoaderCount「100」，scratch 下有 RawData.csv 與 JamCountEnable.ini（19 鍵）；(b) queryYesterday 而且昨天的 CSV 存在 → 之後 iOneDayLoaderCount==0（golden :5262-5263），檔名是昨天；(c) queryYesterday 而昨天的 CSV 不存在 → 先刪掉（照 test_observer_core.cpp:571-595 的哨兵寫法）→ 提早返回、計數不變、表格不變；(d) slEventLog 為 NULL → 回 "log-objects-not-created"、scratch 沒有新檔；(e) InitialOK 為 false → 沒有寫檔；(f) bN26_UseJamRawDataUpdataToFTP=true＋yesterday → 回覆 ftpTail 是 gated；(g) HandleActionWithTag 分派：act.observerSG.queryNow 到得了；act.nope.nope 仍回 unknown-action。
- real-file watch：D:\HT9045_Log\EventLogTxt\SGJamCount 跑前跑後要一樣（請 ST01-M 加進 full_gate_template）。

**(5) 要人工審核的（交 ST01-M 登記 ST02_HUMAN_REVIEW_<日期>.md）**

- A 操作員：Observer → System Message → SG_JamCount → Query Now 出表；Query Yesterday 之後 Loader Count 變 0（golden）。
- B 行為：按一次就刪掉重寫 D:\HT9045_Log\EventLogTxt\SGJamCount\YYYY\MM\<HandlerID>_YYYYMMDD_RawData.csv，並在缺鍵時寫 JamCountEnable.ini；Query Yesterday 把今天的 iOneDayLoaderCount 歸零（只在記憶體）。
- C 移植樹多的：slEventLog 為 NULL 時拒絕（golden 不會發生）。

**(6) 工作量**：C++ 半天、JS 半天、ctest 半天；等 E-021 上 main 後開工。

---

## 三、(A) Lot Info FTP 分頁（LI-9）—— golden

### 3.1 畫面（uLotInfo.dfm:1803-1949，tsFTP 'FTP'）

| 元件 | Tag | Caption | dfm 初始 | OnClick | 做什麼 | 客戶 |
|---|---|---|---|---|---|---|
| btnFtpServer（:1830-1844） | 0 | Server | 看得到 | btnFtpServerClick | 開 FTP 對話框「伺服器」頁：列出伺服器工作檔、下載 | 通用（[FTP] Enable FTP） |
| btSaveSetupFile（:1877-1892） | — | Save to handler | Visible=False | btSaveSetupFileClick | 下載完成後由程式按：X_NET 存成 X | 通用（下載流程內部） |
| btnDataFTPSaveToData（:1893-1908） | 3 | DataFTP Save  to  Data | 看得到，但 FormShow :566-581 只有 TSMC 顯示 | btnFtpServerClick | DataFTP → Data 複製 | S25（CC_TSMC_TAINAN） |
| btnFtpHD（:1845-1860） | 1 | HD | 看得到 | btnFtpServerClick | 開對話框「本機」頁：上傳、或從本機換檔 | 通用 |
| btnFtpTester（:1861-1876） | 2 | Tester Name | 看得到；SPIL／TERAPOWER／AMD_M／OEE／GIGAS 等分支藏起來（uLotInfo.cpp:462、:490、:494、:697、:803、:982） | btnFtpServerClick | 開對話框「Tester」頁：設定 Tester Name | 通用（被藏的是 S25） |
| btnFTPTryConnect（:1909-1924） | 4 | Connection test | Visible=False；GIGAS 顯示（:983） | dfm **沒有** OnClick；handler btnFTPTryConnectClick（:13836-13839 → CheckFTPConnection）沒有綁 | 測試連線 | S25（CC_GIGAS） |
| lbFTPStatus（:1815-1829） | — | Status | Visible=False | — | CheckFTPConnection 寫 Connect OK／Fail（FTPClient.cpp:4594、:4599） | S25 |
| BtnPause（:1925-1948） | — | PAUSE | — | BtnPauseClick | 機台暫停 | 不在本卡（E-019 LI-10，St01） |
| btnFTPDownLoadbyDeviceID（Lot ID 分頁 :435-443） | — | DownLoad | Visible=False，JSCC_OS 顯示（:671-676） | btnFTPDownLoadbyDeviceIDClick（:16074-16077 → btnFtpServer->Click()） | 依 Device ID 下載 | S25（CC_JSCC_OS） |

**頁籤與鈕的可見／可按**

- tsFTP->TabVisible：FormShow（JCET 用 IniConfig.bEnableFTP，其他用 CosFunction.bFTPFunction，:401-404），之後 Timer2Timer 一律改成 IniConfig.bEnableFTP（:7093，VTEST 除外）；HonPrec 等級登入時 main.cpp:12559-12562 也設成 bEnableFTP；MTI／PTI／TFME_CHINA 藏（:364-369、:431-435，S25）。⇒ 一般機台看 config.ini [FTP] Enable FTP。
- ckernel 狀態機設 Enabled（ckernel.cpp）：SystemStart 時兩顆都關（:1068-1072）；PAUSE 而機台內有 IC 或手臂沒停 → Server 關（:1590-1594），否則 CosFunction.bFTPFunction && IniConfig.bEnableFTP 時依 iServerEnable／iHDEnable 與 AccessLevel 開（:1597-1615），且 FTP 視窗沒開才開 Server（:1603）；PAUSE 時 HD 一律開（:1623）；HALT 時同樣依等級、機台內沒 IC 才開 Server（:1644-1676）。CosFunction.bFTPFunction 預設 false（CosFunction.cpp:3893），由客戶碼打開。
- KYEC 開機自動按 Tester Name（main.cpp:25640-25648，S25）。

### 3.2 btnFtpServerClick（uLotInfo.cpp:5001-5126，原文；客戶分支節錄）

```
5001: void __fastcall TfLotInfo::btnFtpServerClick(TObject *Sender)
5002: {
5003:     TButton *Ptr;
5004:     Ptr=(TButton *)Sender;
5005: 
5006:     if(CUSTOMER_CODE==CC_JSCC_OS)                                               //RogerYang 20260127 : Add For JSCC_OS download by Device list
...（:5006-5040 JSCC_OS：讀 DataPath+"DeviceCorrespond.ini" 找 Program，訊息 "Please Input Device ID First!" 等 —— S25）
5042:     if(IniConfig.bEnableFTP==false && CUSTOMER_CODE==CC_TSMC_TAINAN)            //ChungHung 20150413 add for TSMC   //ChungHung 20150415 add for TSMC
5043:     {
5044:         Ptr->Enabled=true;
5045:         return;
5046:     }
5047:     AnsiString TargetPath,SourcePtah,str;
5048: 
5049:     Ptr->Enabled=false;
5050:     if(Ptr->Tag==0)
5051:     {
5052:         if(IniConfig.iServerEnable>AccessLevel)
5053:         {
5054:             Ptr->Enabled=true;
5055:             return;
5056:         }
5057:     }
5058:     else if(Ptr->Tag==1)
5059:     {
5060:         if(IniConfig.iHDEnable>AccessLevel)
5061:         {
5062:             Ptr->Enabled=true;
5063:             return;
5064:         }
5065:     }
5066:     else if(Ptr->Tag==2)
5067:     {
5068:         if(IniConfig.bSPILFunction==true)                                       //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
5069:         {
5070:             return;
5071:         }
5072:     }
5073:     else if(Ptr->Tag==3)                                                        //ChungHung 20150413 add for TSMC
...（:5073-5093 TSMC：XCOPY D:\HT9045\IniData\DataFTP\<檔> → Data\<檔> —— S25）
5103:     if(SystemStart==true)
5104:     {
5105:         Ptr->Enabled=true;
5106:         return;
5107:     }
5108: 
5109:     fFTPClient->ShowFTPModal(Ptr->Tag);
5110:     fFTPClient->bShow=false;
5111: 
5112:     if(CUSTOMER_CODE==CC_TSMC_TAINAN && SystemStart==false)                     //ChungHung 20150413 add for TSMC
5113:     {
5114:         _DelTree("D:\\HT9045\\IniData\\DataFTP\\", GetLastOpenFN());
5115:     }
5116: 
5117:     Ptr->Enabled=true;
5118: 
5119:     if(CUSTOMER_CODE==CC_TERAPOWER)
5120:     {
5121:         if(fMain->cbSetupFileName->Text.Pos("_NET")>0)
5122:         {
5123:             btSaveSetupFile->Click();
5124:         }
5125:     }
5126: }
```

（:5095-5101 是註解掉的 CheckCanChangeRealDummy。golden 怪處：SPIL 的 Tag 2 在 :5049 關掉鈕之後 return，沒有開回來 —— S25。）

### 3.3 btSaveSetupFileClick（uLotInfo.cpp:5128-5223，原文；TSMC 節錄）

```
5128: void __fastcall TfLotInfo::btSaveSetupFileClick(TObject *Sender)
5129: {
5130:     SetCurrentDirectory(_T("D://"));
...
5133:     AnsiString OrgPath="", NewPath="", str1, str2;
5134:     AnsiString SPath[2]={IncludeTrailingPathDelimiter(DataPath), IncludeTrailingPathDelimiter(OffsetPath)};
5135: 
5136:     str2=fMain->cbSetupFileName->Text;
5137:     str1=str2.SubString(1, str2.Length()-4);
5138:     int SPath_length=sizeof(SPath)/sizeof(AnsiString);
5139:     for(int i=0; i<SPath_length; i++)                                           //Jimmychiu 20230307 Replace constant with array length
5140:     {
5141:         if(IniConfig.bE45_AllSetupFileUseOneFile && i==1)                       //Steven 20190117 : 沒有要複製Offset
5142:             continue;
5143: 
5144:         OrgPath=SPath[i];
5145:         NewPath=OrgPath;
5146:         OrgPath+=str2;
5147:         NewPath+=str1;
5148:         MyForceDirectories(NewPath);
...
5151:         OrgPath+="\\*.*";
5152:         if(CUSTOMER_CODE!=CC_TSMC_TAINAN ||
5153:            (CUSTOMER_CODE==CC_TSMC_TAINAN && i!=1))                             //wei 20160726 TSMC offset不複製
5154:         {
...
5158:             ZeroMemory(&oFile, sizeof(SHFILEOPSTRUCT));
5159:             oFile.hwnd=Handle;
5160:             oFile.wFunc=FO_COPY;
5161:             oFile.pFrom=cStr1;
5162:             oFile.pTo=cStr2;
5163:             oFile.fFlags=FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI;
5164:             SHFileOperation(&oFile);
...
5168:         if(i==2)                                                                //Steven 20101118 : 強制要加加
5169:             i++;
5170:     }
5171:     MySleep(500);                                                               //landam
5172: 
5173:     ClearAllSetupFile(str1, str2);                                              //Steven 20200512 : 刪除全部工作檔, 只留下當下的
5174: 
5175:     if(bNoSendSiteOnOff==true)
5176:         EventReport(SECS_EVENT.SwitchSetupFile);
5177: 
5178:     bNoSendSiteOnOff=false;                                                     //wei 20160511 Send Site On Off
...
5181:     if(CosFunction.bFTPDownLoadSiteBySetupFile==true)                           //Ifor 20171123 (Steven) : add FTP DownLoad Site By SetupFile
5182:     {
5183:         if(bDownloadFTP && bFTPDownLoadHasTestMode)
5184:         {
5185:             fMain->ShowTestHeadComp(false);
5186:         }
5187:         else
5188:         {
5189:             fMain->ShowTestHeadComp(true);
5190:         }
5191:     }
5192:     else
5193:     {
5194:         fMain->ShowTestHeadComp(true);                                          //20140218  WEI
5195:     }
...
5198:     btSaveSetupFile->Visible=false;
5199: 
5200:     if(ATC_SYSTEM==eATCHonPrecType)                                             //Ifor 20160101 工作檔更換重新連線ATC
5201:     {
5202:         if(ATCInterfaceForm->IsOnLine()==false)
5203:             ATCInterfaceForm->OnLine();
5204:         if(ATCInterfaceForm->ATC_SYS.IsChillerRun()==false)
5205:             ATCInterfaceForm->ATCChillerSwitch(true);
5206:         if(ATCInterfaceForm->ATC_SYS_PAL[0]->bATCRunSetting==false)
5207:             ATCInterfaceForm->SetRunATC(true);
5208:         bRunATC=true;                                                           //ChungHung 20160118 add for Hisi V102
5209:     }
...（:5211-5222 TSMC：ModifyTester(ON_LINE)、REALLY、重畫 —— S25）
5223: }
```

ClearAllSetupFile（:11833-11890）：先 fBuilder->DeleteSetupFile(X_NET)；CosFunction.bKeepOnly1SetupFile 時把 DataPath、OffsetPath 底下 X 以外的資料夾全部 DeleteDirectory（:11842-11874）；然後 fSetup->bFirstTime、RecordProcess("Clear All Setup File")、重列清單、cbSetupFileName=X、WriteLastDataFN(X)、fMain->ChangeSetUpFile(X)，bLastSetInSetUpFile 時 SetRunStartMode（:11877-11889）。

### 3.4 KYEC FTP 對話框 TfFTPClient（KYECFTP/FTPClient.cpp）

**ShowFTPModal(int HD)（:1215-1573，關鍵原文）**

```
1219:     if(bShow)
1220:     {
1221:         ShowMyMessage("FTP Form already Opened!!");
...
1226:         return;
1227:     }
1228: 
1229:     bShow=true;
1230:     bError=false;
1231:     tmpList = new TStringList;
...
1233:     iHD=HD;
1234:     switch(HD)
1235:     {
1236:         case 0:                                                                 //Server 要列表,有問題不能用return, 要用break,這樣才能delete NMFTP3
...
1271:                 NMFTP3->Vendor                  =NMOS_AUTO;
1272:                 NMFTP3->TimeOut                 =20000;                         //Landam
1273:                 NMFTP3->Passive                 =true;                          //Steven 20121020 : 實驗看看
...
1283:                     if(IniConfig.FtpHost!="")
1284:                         NMFTP3->Host                =IniConfig.FtpHost;
1285:                     NMFTP3->UserID                  =IniConfig.FtpUserName;
1286:                     NMFTP3->Password                =IniConfig.FtpPassword;
...
1298:                 NMFTP3->Port                    =StrToInt(IniConfig.N06_FtpPort); //Ifor 20201015 add:使用者自定義 FTP Port
...
1304:                     NMFTP3->Connect();
...
1316:                 if(!NMFTP3->Connected)
1317:                 {
1318:                     if(bControlByGPIB==false)                                   //KaiChen 20190530 ：Sigurd FTP Automation
1319:                         ShowMyMessage("FTP Server is not connected","");
...
1337:                     DirName=sRootPath+IniConfig.FtpDownloadPath;
...（:1338-1358 ChangeDir）
1381:                     NMFTP3->Nlist();
...（:1404-1410 最多等 20×300 ms 的 bListOk）
1414:                 if(tmpList->Count==0)                                           //jou 2013-01-22 FTP error show alarm message
1415:                 {
1416:                     ShowMyMessage("FTP Error -- List fail! Empty directory!","FTP錯誤，資料夾找不到檔案");  //RogerYang 202601278 : add chinnese
...
1442:         case 1: //HD
...（:1451-1470 TSMC／Greatek 列資料夾 —— S25）
1473:                 for(int i=0; i<fMain->cbSetupFileName->Items->Count; i++)
1474:                 {
1475:                     lstHDFile->Items->Add(fMain->cbSetupFileName->Items->Strings[i]);
1476:                     tmpList->Add(fMain->cbSetupFileName->Items->Strings[i]);    //Landam
1477:                 }
...
1480:         case 2: //Taster
...（:1486-1514 N06_TasterListFile 存在時載入 Tester 類型／編號／名稱）
1520:     if(bError==false)
1521:     {
...（:1524-1560 SECS／GPIB 遠端與 Greatek 自動下載不開畫面，直接 plSLoadClick）
1561:         else
1562:         {
1563:             ShowModal();
1564:         }
1565:     }
```

- 列檔 NMFTP1ListItem（:1799-1829）：名字含 ".zip" 而且（不含 ".Offset" 或 不含 "ATC_Recipe.zip"）就加進 lstServerFile（去掉 ".zip"）。golden 怪處：這個「或」幾乎永遠成立，ATC_Recipe.zip 也會被列出來。
- 篩選 FilterList（:2098-2115）：輸入框文字（不分大小寫）**完全相同**才留（Sam 20190925 改的）；雙擊清單把名字帶進輸入框（:2127-2145）。

**下載 plSLoadClick（:847-1102，關鍵原文）**

```
 849:     if(bControlBySECSGEM==false         &&
 850:        Barcode_Reader(bcSetupFile)==0   &&                                      //ChungHung 20150515 add Control by SECSGEM // 20140103 wei KYEC Barcode Reader
 851:        bControlByGPIB==false)                                                   //KaiChen 20181129 ：Add FTP Downlaod Setup File by GPIB Command
 852:     {
 853:         return;
 854:     }
...
 865:     if(bDownloadFTP==true)
 866:     {
 867:         iErrorBySECSGEM=1;                                                      //ChungHung 20150515 add Control by SECSGEM
 868:         return;
 869:     }
 870:     bDownloadFTP=true;
...
 876:         if(lstServerFile->Items->Count==0)                                      // Landam  選對一定會剩一條record
...
 881:                 ShowMyMessage("未選擇路徑");
...
 913:             if(edtServerWaferName->Text!=lstServerFile->Items->Strings[0])      // Landam  選對名稱會一樣
 914:             {
 915:                 ShowMyMessage("未選擇這路徑");
...
 981:     iOldRunStartMode=LastSet.iRunStartMode;
...
 986:         LoadFileFormServer2(sRootPath+IniConfig.FtpDownloadPath, edtServerWaferName->Text);
 987: 
 988:     DownloadPasswordFormServer();                                               //Sam 20210526 : 從 N06 DownloadPath 下載密碼本
 989:     bCanExit=false;  //Steven 20260618 fix: == -> = (P11 bug fix)                                                            //ChungHung 20150413 add for TSMC
 990:     SYS_SetupFile=edtServerWaferName->Text;                                     //20140103 wei
 991: 
 992:     if(FTP_DownloadFail==true)                                                  //jou 2014-04-16 ftp download fail fix
...
 996:         bFTPDownloadSetupFile = false;
 997:     }
 998:     else
 999:     {
1000:         bFTPDownloadSetupFile = true;
1001:         bInitNeedDownloadFTP=false;                                             //JerryYang 20200416 艾科要求切initial start按start要強制download recipe
1002:         if(bEnablePEModel==false)                                               //Ifor 20160824 add 非PE工程模式底下才可清除旗標
1003:         {
1004:             bHasEnteredPEModel=false;                                           //Ifor 20160823 FTP檔案下載後關閉PE模式判斷旗標
1005:             WriteIniDataGeneral("System", "bHasEnteredPEModel", bHasEnteredPEModel); //Ifor 20160824 避免 PE模式修改後程式被關閉上傳
1006:         }
1007:     }
...
1016:     fMain->cbSetupFileName->Clear();
1017:     fMain->LookForFile();
1018: 
1019:     edtServerWaferName->Enabled=false;                                          //Ifor 20160511 避免下載途中被修改到名稱
1020:     fMain->cbSetupFileName->Text=edtServerWaferName->Text+"_NET";
...
1023:     sDirPath.sprintf("D:\\HT9045\\IniData\\Data\\%s\\TestMode.Data", fMain->cbSetupFileName->Text);
1024:     if(FileExists(sDirPath))                                                    //Ifor 20161221 (Steven) 判斷FTP檔案是否存在
1025:         bFTPDownLoadHasTestMode=true;
...
1029:     fMain->cbSetupFileNameChange(fMain);
1030:     fLotInfo->btSaveSetupFile->Visible=true;
1031: 
1032:     fLotInfo->btSaveSetupFile->Click();
1033:     bDownloadFTP=false;
...
1038:     if(IniConfig.bEnable_SECS_GEM==true)                                        //wei 20150525 : Secs Gem
...（:1040-1046 SECS 遠端下載時 EventReport DownLoadRecipeByFTPOK／NG）
1049:     if(CosFunction.bUseFTPDownloadDataCheck==true &&                            //Ifor 20170727 (wei) add 比對下載資料 by 矽格中興廠
...（:1049-1069 抄 *_NET 快照，St01 已翻成 W906_FTPClient_DownloadRecordNetData）
1071:     bSetTempChange=true;                                                        //Ifor 20160425 工作檔切換需重新設定ATC參數
...
1101:     Close();
```

- LoadFileFormServer2（:107-389）：連線（同上帳號、被動、Port＝N06_FtpPort）→ CWD → 依 FtpTransMode 設 ASCII／IMAGE／BYTE → 同一個 <名>.zip 下載 3 次到 DataPath+<名>_NET.zip、互比本機大小（:226-236、:362-368；不一致 → "FTP DownLoad File Size Error"）→ 下載 OffsetPath+<名>_NET.Offset（:305）→ 條件成立時下載 ATC_Recipe（:317-335）→ fLotInfo->DownloadFromServer(<名>_NET)（:336）。
- TfLotInfo::DownloadFromServer（uLotInfo.cpp:4182-4704）：用 d:\HT9045\7z.exe 解壓（沒有就從 C:\Program Files\7-Zip\7z.exe 複製，:4333-4335；IniConfig.FtpUseSystemCallToUnZip 選 system() 或 ShellExecute，:4445-4506）、MD5 檢查（WAR16118，:4535-4538）、必要檔案檢查、不是新工作檔時還原本機參數等；失敗跳 WAR1684（:4511）。RMS 下載也走這一支。

**上傳 plUnloadClick（:1111-1212，關鍵原文）**

```
1127:     if(bHasEnteredPEModel==true)                                                //Ifor 20160823 進入PE工程模式或者PE工程模式開啟 不可上傳檔案
1128:     {
1129:         if(bEnablePEModel)
1130:             ShowMyMessage("PE engineering model can not upload files\r\nPE工程模式不可上傳檔案");
1131:         else
1132:             ShowMyMessage("Leave PE engineering model must download Setup File first\r\n離開PE工程模式必須先下載Setup File");
1133:         return;
1134:     }
...（:1136-1152 清單空的或名字不符 → "未選擇這路徑"）
1154:     plUnload->Enabled=false;
1155: 
1156:     SetMD5ByFolder(DataPath+edtHDWaferName->Text);                              //Steven 20170927 (wei) : 將工作檔加入檢查碼
...
1206:     else
1207:     {
1208:         UploadFileToServer2(sRootPath+IniConfig.FtpUplaodPath, edtHDWaferName->Text, true);    //jou 2015-01-16 修正FTP upload error
1209:     }
1210:     plUnload->Enabled=true;
1211:     edtHDWaferName->Text="";
```

- UploadFileToServer2（:390-846）：7z 把 DataPath\<名>\*.* 壓成 DataPath\<名>.zip、OffsetPath\<名>.Offset（:564-569），STOR 上去（:588-595），刪本機壓縮檔（:693-697），成功跳 "Upload done"（:805）。
- plLoadClick（:1575-1653）：不連網路；名字檢查後 fMain->ChangeSetUpFile(<名>)，等於從本機換工作檔。
- btSafeTasterNameClick（:1868-1953）：Tester 名稱要是「類型-編號」而且都在清單裡，否則清空並跳 "Tester Name 錯誤"（:1912-1913）；寫 IniConfig.N06_TasterListMap（:1936-1938，可能是網路路徑）、SaveTasterInfo（config.ini）。

### 3.5 操作員看到的訊息（golden 原字串；非客戶）

| 時機 | 訊息 | 出處 |
|---|---|---|
| 對話框已開著 | FTP Form already Opened!! | FTPClient.cpp:1221 |
| 連不上 | FTP Server is not connected | :1319、:158（LoadFileFormServer2）、:452／:485（UploadFileToServer2） |
| 中途斷線 | FTP Server is disconnect | :1365、:1425、:1435 |
| 伺服器資料夾沒有檔 | FTP Error -- List fail! Empty directory!／FTP錯誤，資料夾找不到檔案 | :1416 |
| 下載沒選 | 未選擇路徑 | :881 |
| 下載名字不符／上傳、本機換檔沒選或不符 | 未選擇這路徑 | :915、:1141、:1150、:1587、:1596 |
| 下載三次大小不一 | FTP DownLoad File Size Error | :367 |
| 解壓／MD5 | WAR1684（下載 %s.zip 失敗）、WAR16118（MD5 check fail!）、WAR1683（Connection Fail，RMS） | uLotInfo.cpp:4511、:4537、:4207 |
| PE 工程模式上傳 | PE engineering model can not upload files\r\nPE工程模式不可上傳檔案 | FTPClient.cpp:1130 |
| 離開 PE 模式未下載就上傳 | Leave PE engineering model must download Setup File first\r\n離開PE工程模式必須先下載Setup File | :1132 |
| ATC recipe 上傳失敗 | Upload ATC recipe fail, check below path | :583 |
| 上傳完成 | Upload done | :805 |
| Tester 名稱不對 | Tester Name 錯誤 | :1913 |
| 例外 | e.Message | :1309、:1356、:823 |

客戶專屬（S25，不移植）：JSCC_OS 的 "Please Input Device ID First!" 等（uLotInfo.cpp:5010-5032）、TSMC 的「未選取產品名稱」（:879、:896）、KYEC_LEE 的 "Different Setup File Name,Need Finish Clean Out!"（:860）、GIGAS 的 ContinueStart 訊息（:956、:1635）、AMD_M 的「密碼錯誤不可上傳檔案」（:1117）。

### 3.6 讀寫的檔與路徑（下載＋存檔一次）

| 動作 | 路徑 | 出處 |
|---|---|---|
| 讀設定 | config.ini [FTP]：Enable FTP、FTP User Name／Password／Host、FTP Download Path／Upload Path、FTP HD Enable（iHDEnable）、FTP Server Enable（iServerEnable）、Port、Mode、FtpUseSystemCallToUnZip、Taster 相關（cConfiguration.cpp:5038-5056 是畫面對應） | — |
| 網路 | FTP 伺服器：CWD／NLST／RETR（<名>.zip ×3、<名>.Offset、ATC_Recipe）；上傳 STOR | FTPClient.cpp:1304-1381、:229、:305、:590、:595 |
| 寫 | DataPath\<名>_NET.zip、OffsetPath\<名>_NET.Offset、解壓到 DataPath\<名>_NET\ | :180-183；uLotInfo.cpp:4447 |
| 寫 | D:\HT9045\7z.exe（從 C:\Program Files\7-Zip\7z.exe 複製） | uLotInfo.cpp:4333-4335；FTPClient.cpp:519、:558 |
| 讀 | D:\HT9045\IniData\Data\<名>_NET\TestMode.Data（硬寫路徑） | FTPClient.cpp:1023 |
| 寫 | system\Gerneral.ini [System] bHasEnteredPEModel | :1005 |
| 複製 | DataPath\<名>_NET\*.* → DataPath\<名>\、OffsetPath 同（SHFileOperation FO_COPY） | uLotInfo.cpp:5139-5170 |
| 刪 | <名>_NET（fBuilder->DeleteSetupFile）；bKeepOnly1SetupFile 時 Data／Offset 底下其他全部工作檔 | uLotInfo.cpp:11839-11874 |
| 寫 | lastdata（WriteLastDataFN）、換工作檔（ChangeSetUpFile） | :11886-11887 |
| 上傳前寫 | DataPath\<名>\ 的 MD5 檢查碼（SetMD5ByFolder） | FTPClient.cpp:1156 |
| 上傳暫存 | DataPath\<名>.zip、OffsetPath\<名>.Offset（上傳後刪） | :564-569、:693-697 |
| Tester | IniConfig.N06_TasterListMap 檔、config.ini（SaveTasterInfo） | :1936-1946 |

### 3.7 設定、CosFunction 與客戶條件

- 通用：IniConfig.bEnableFTP（頁籤）、iServerEnable／iHDEnable 對 AccessLevel、SystemStart、CosFunction.bFTPFunction（只影響 ckernel 開鈕）、FtpHost／FtpUserName／FtpPassword／N06_FtpPort／FtpDownloadPath／FtpUplaodPath／FtpTransMode、FtpUseSystemCallToUnZip、bE45_AllSetupFileUseOneFile、CosFunction.bKeepOnly1SetupFile、CosFunction.bFTPDownLoadSiteBySetupFile、CosFunction.bUseFTPDownloadDataCheck、CosFunction.bFTPUseBarcodeReader＋IniConfig.bN06_UseBarcode（Barcode_Reader，KYEC 讀碼器）、ATC_SYSTEM==eATCHonPrecType、bEnable_SECS_GEM、bEnablePEModel／bHasEnteredPEModel、IniConfig.bN20_CheckMD5。
- S25（客戶碼專屬，不移植）：CC_JSCC_OS、CC_TSMC_TAINAN（Tag 3、DataFTP、XCOPY、_DelTree、切 ON_LINE）、CC_TERAPOWER、CC_KYEC_LEE／CC_KYEC_XILINX（HiSilicon 檢查表、bNoSendSiteOnOff、路徑標籤）、CC_GIGAS（Tag 4、upload-before-download、InitialStart、CheckFTPConnection）、CC_Greatek（自動下載、資料夾列表）、CC_AMD_M（密碼、多下載四個檔）、CC_PTI（不設模式）、CC_ASE_N、CC_CYUEAN、CC_JCET、bSPILFunction、bSIGURDFunction（不下載 Offset）、bVTESTFunction、Sigurd FTP Automation（bSigurdDownload_Recipe／bSigurdUpload_Recipe）、HiSilicon。由客戶碼打開的 CosFunction 旗標（例：PassworDownloadByFTP，CosFunction.cpp:1207、:1319、:1784；bUseATCFileTransfer、bUseFTPDownLoadATCRecipe）條件照留、本體依 S25 處理。

### 3.8 改到的全域（golden）

- btnFtpServerClick：按鈕 Enabled、fFTPClient->bShow（sJSCCOSFileName 是 S25）。
- ShowFTPModal／FormClose：bShow、bError、iHD、tmpList、bTempList、bListOk、NMFTP3、iErrorBySECSGEM／iErrorByGPIB、bControlBySECSGEM／bControlByGPIB、bSigurd*、bHasFTPDownload。
- plSLoadClick：bDownloadFTP（下載中為 true）、iOldRunStartMode、SYS_SetupFile、FTP_DownloadFail、bFTPDownloadSetupFile、bInitNeedDownloadFTP、bHasEnteredPEModel（＋寫檔）、bCanExit、bFTPDownLoadHasTestMode、*_NET 快照（DownloadWorkFile_NET 等，bUseFTPDownloadDataCheck 時）、bSetTempChange、fMain->cbSetupFileName（換工作檔）。
- btSaveSetupFileClick：bNoSendSiteOnOff、fSetup->bFirstTime、bRunATC、btSaveSetupFile->Visible；ShowTestHeadComp 牽動的 Site 畫面與 ATC Site。
- 怪處（照翻）：R136 —— 「成功沒」看的是 DownloadPasswordFormServer 留下的 FTP_DownloadFail（兩個密碼本開關都開時）；:1029 記錄的 DownloadWorkFile_NET 是上一次的值（St01 在 WebStart.cpp:3934-3946 已寫明）；btSaveSetupFileClick 的 `if(i==2) i++;` 永遠不成立（:5168）；btnFTPTryConnect 在 dfm 沒有 OnClick。

---

## 四、(A) Lot Info FTP 分頁（LI-9）—— 移植樹、相依、做法

### 4.1 移植樹現況

| 部件 | 移植樹位置 | 狀態 | 誰的 |
|---|---|---|---|
| tsFTP 與五顆鈕的門面成員 | forms/fLotInfo.cpp:276（tsFTP）、:609（btnFtpTester）、:617（btnDataFTPSaveToData）、:637-638（btnFtpServer／btnFtpHD）、:651-652（btnFTPTryConnect／lbFTPStatus）；forms/fLotInfo.h:2035-2036、:2051-2052 | 有；**btSaveSetupFile 沒有成員** | 筆電（adbd3dc3、25f92074，0819） |
| FormShow 可見度 | forms/fLotInfo.cpp:3175-3177、:3222、:3249、:3287、:3291、:3378、:3389、:3493-3498、:3520-3522、:3640、:3827-3831、:3913；Timer2Timer :4322 | 已翻 | 筆電 |
| 網頁頁籤可見度 | forms/fLotInfo.cpp:6043 W906_RefreshTabVisible（:6097-6099、:6222、:6255-6258）→ tag lot.tab.tsFTP → web/page/ht9045_lotinfo_wire.js:66 | 有 | Steven 8af13c07（C-route；E-019 判定 Data.LotInfo 頁屬 St01） |
| btnFtpServerClick／btSaveSetupFileClick／btnFTPTryConnectClick／btnFTPDownLoadbyDeviceIDClick | 沒有；forms/fLotInfo.h:1081-1090 出口登記「B. SENDS AN OUTBOUND COMMAND」 | **缺** | — |
| DownloadFromServer（:4182-4704）／ClearAllSetupFile（:11833-11890） | 沒有（KYECFTP/FTPClient_Transfer.cpp:264-271 用回 false 的替身；SECSGEM/uHGemHT9045.cpp:824 記「TfLotInfo has no ... ClearAllSetupFile」；MainTimer8.cpp:111 在 #if 0 GATE T8-1 裡） | **缺** | — |
| ckernel 開關鈕 | ckernel.cpp:2062-2063、:2584-2614、:2649-2674 ACTIVE；`fFTPClient->bShow==false` 兩處閘住 :2593-2595、:2639-2641（理由「缺相依：TfFTPClient 全樹沒有 port……等同『FTP 視窗沒開』」） | 已翻（閘住條件行） | 筆電（290f0238、ccbd8de5） |
| FTP 對話框與傳輸 | 見 2.2 的「移植樹的 FTP 層」表 | 對話框缺；傳輸已翻但沒連、沒真連線 | 筆電 |
| 下載成功解開 START | WebStart.cpp:3908-3950 說明、:3951 W906_FTPClient_DownloadResult（golden :1005-1022）、:3979 W906_FTPClient_DownloadRecordNetData（golden :1049-1069）；讀者 WebStart.cpp:763、:780（NETDownloadDataCheck，出貨版才編）；開機快照 FileRW/MainBoot.cpp:535 | 安裝座，**沒有呼叫者** | St01（a99d8e6c，R120-F） |
| 換工作檔 | WebRecipeChange.cpp：W906_RC_LookForFile :155（可呼叫）、W906_RC_ChangeSetUpFile :261 與 W906_RC_cbSetupFileNameChange :389 是 **static** | 有，但 St02 叫不到後兩支 | Steven 8af13c07／a97739ae（C-route，請 ST01-M 確認歸屬） |
| 其他可用的 | fBuilder->DeleteSetupFile（forms/fBuilder.h:523 LIVE）、SetMD5ByFolder（cpublic.cpp:993）、Barcode_Reader（BarcodeReader.cpp:445）、fSetup->bFirstTime（forms/fSetup.h:452）、ATCInterfaceForm（ATC/ATCInterface.h:528）、bDownloadFTP／bFTPDownloadSetupFile／bFTPDownLoadHasTestMode（cmydef.cpp:4455、:3782、:4576） | 有 | 各擁有者 |
| fMain->ShowTestHeadComp | forms/fMain.cpp:247 空函式（golden main.cpp:22608-22617：ChangeArmSiteView、ShowTestHeadComp1、ChangeATCSiteUse、SECS SiteOnOff） | **空殼** | 筆電（fMain 門面） |
| 網頁 | web/page/Data.LotInfo.html:43 頁籤 tab_tsFTP；:129-134 只有「未接：Server／HD／Tester Name（btnFtpServerClick :5112）都是 FTP 網路下載／上傳 —— 不接。」**沒有任何鈕** | 缺 | St01 頁（8af13c07） |
| WS 指令 | lotinfo.op（WebLotInfo.cpp:345，tools/wb_serve.cpp:4833）只有 barcode.*、testerLog.get、selection.*、ocr.clearList、lotEnd*；**沒有 FTP** | 缺 | St01（WebLotInfo.cpp） |

### 4.2 相依與擋住的

1. **真連線開關**（筆電檔）：KYECFTP/FTPClient_Transfer.cpp:374、:661 建引擎後從不 SetSimMode(false)，而且沒有地方讓外面在連線前裝測試掛鉤（tests/test_FTPClient_Transfer.cpp:253-261 自己寫明）。⇒ 需要同一行認領（4.5）。
2. **連結**：wb_serve 沒有連 ht9045_kyecftp（CMakeLists.txt:3441-3444）。:3441 是 St02 的行（3e4ce1de ELA P3），可以同一行把 ht9045_kyecftp 加進去；加之前要 nm 查 tmpList／bTempList／bListOk／bError／FTP_DownloadFail／bPIDTransferErr／NMFTP3 這幾個裸名全域在 wb_serve 沒有撞名（grep 目前只在 KYECFTP），並確認 ws2_32 的 DLL 匯入沒有影響別的程式（記取 W58 IniBoolOverride 把 WININET 拉進 ELA_Ftp 的教訓）。
3. **TfLotInfo::DownloadFromServer 523 行**要翻（RMS 下載也會用到），以及 ClearAllSetupFile、btSaveSetupFileClick、btnFtpServerClick。TfLotInfo 門面是筆電／St01 的檔 ⇒ 照 E-020（W906_LotInfo_SECSLotStart）與 C4 的做法，St02 翻成自己檔裡的自由函式，只呼叫 fLotInfo-> 既有成員；沒有的成員（btSaveSetupFile）用 St02 檔內的替身，不動 forms/fLotInfo.h。
4. **WebRecipeChange.cpp 兩支 static**：plSLoadClick 要 cbSetupFileNameChange（:1029），plLoadClick 與 ClearAllSetupFile 要 ChangeSetUpFile（:1643、:11887）⇒ 認領把 :261、:389 的 `static` 拿掉（同一行），或請擁有者給一個對外的包裝。
5. **fMain->ShowTestHeadComp 是空殼**：btSaveSetupFileClick 照呼叫（golden 行為由擁有者補本體時自然回來）；記進人工審核：存檔後 Site 畫面／ATC Site 不會重整。
6. **網路規則**（不是擋，是條件）：W36＝C 模擬版跟出貨版一樣真的連；W36-1／W58 Q1-Q3：模擬版每次啟動 [FTP] Enable FTP（N06）、N06-1 先當成沒勾（SimNet/SimNetMask.cpp:32-33），操作員勾選存檔才放行並寫進 config.ini ⇒ 模擬版預設看不到 FTP 頁籤。**W58 Q5（W906_SimNetPathBlocked）只管「\\」或對應磁碟機的存檔路徑**，FTP 主機不在它的範圍；但 Tester Name 寫的 IniConfig.N06_TasterListMap 如果是網路路徑，模擬版照 Q5 要跳過（呼叫前判斷 W906_SimNetPathBlocked）。
7. **主動／被動**：W46-1 要每個 FTP 可選；W60＝B 定案 KYEC 這組留在 MiniFtpEngine、照 golden 被動（golden 寫死 Passive=true，FTPClient.cpp:1273 等）。MiniFtpEngine 只有 PASV ⇒ N06 先只做被動；`[FTPUpLoad] bN06FtpPassive` 鍵（St02 W46-1 清單第 1 列，預設被動）等引擎有主動時再加，現在加了也不會生效。
8. **W62**：下載在主迴圈同步做（只准閒置、機台內沒 IC，golden ckernel 的開鈕條件已翻），訊息框照 golden 用 ShowMyMessage（網頁 MbWait 等操作員）。上機驗證用真的工作檔資料夾。
9. **7-Zip**：解壓與上傳壓縮都靠 C:\Program Files\7-Zip\7z.exe（golden 複製到 D:\HT9045\7z.exe）。

### 4.3 St02 的做法（三段）

**F1（不碰網路；約 3 天）：對話框外殼＋本機三件**

- St02 新檔 `KYECFTP/FTPClientForm_St02.{h,cpp}`（手寫）：`class TfFTPClient` 門面（只放 golden 用到的成員名：bShow、iHD、bCanExit、sRootPath、bControlBySECSGEM／bControlByGPIB、iErrorBySECSGEM／iErrorByGPIB、aSetUpNameBySECSGEM、asSetUpNameByGPIB、edtServerWaferName／edtHDWaferName、lstServerFile／lstHDFile、ListBox1、memoFTP、plSLoad／plUnload／plLoad、PageControl 的頁）＋ `TfFTPClient *fFTPClient` 全域；bError／bListOk／tmpList／bTempList／FTP_DownloadFail／NMFTP3 **用筆電檔已有的全域**（extern，不重宣告）。安裝 FTPClientEvt_LogSink（FTPClient_EventHandlers.h），讓 NMFTP1ListItem 寫進門面的清單與 memo。
  - 照 golden 翻：ShowFTPModal（:1215-1573，`ShowModal()` 那一行改成「回報網頁：對話框開著」，同 E-021「golden Close() 改成網頁關窗」的做法）、FormShow（:1708-1797）、FormClose（:1655-1701）、FilterList／edt*Change／lst*DblClick（:2098-2145）、plLoadClick（:1575-1653）、Tester 頁（cbTesterTypeChange :1831、btSafeTasterNameClick :1868、cbTesterIDChange :1954、rgInputMethodClick :2067、GetTesterType :2147）。CheckFTPConnection（GIGAS）不翻（S25）。
  - 新全域符號放 St02 自己的檔，建置後 nm 確認只定義一次。
- St02 新檔 `forms/fLotInfo_Ftp_St02.cpp`（手寫）：`W906_St02_LotInfo_btnFtpServerClick(int iTag)`（golden :5001-5126 逐行；Ptr 依 Tag 取 fLotInfo->btnFtpServer／btnFtpHD／btnFtpTester／btnDataFTPSaveToData；客戶分支照 A1／A2 的做法：判斷式活著、本體 S25）。
- St02 新檔 `JsonBridge/actions/LotInfoFtp.cpp`（WS，手寫）：`act.lotInfoFtp.<op>`：open（帶 tag）、state（唯讀：對話框頁、清單、輸入框、鈕的 Enabled，含 ckernel 設的 btnFtpServer／btnFtpHD->Enabled）、filter、pick、loadHD、saveTester、close；F2 再加 download、upload。為了讓 test_sjson_chan 不必連 KYECFTP，本體用「安裝」：LotInfoFtp.cpp 只轉呼叫一個函式指標，wb_serve 開機時才裝上真本體（同 InstallClarnDataBody 的做法：沒裝＝回 not-installed）。分派加在 St02 的 ChanAction.cpp:348（S119，1c9a99fc）同一行、`//` 之前；來源加在 St02 的 CMakeLists.txt:3398 與 tests/CMakeLists.txt:3955／:3956 同一行。
- 網頁：St02 新檔 `web/page/Data.FTPClient.html`（照 KYECFTP/FTPClient.dfm 版面，可由 tools/dfm2rc/ir_out/KYECFTP/FTPClient.dfm.ir.json 產生骨架）＋ `web/page/ht9045_lotinfo_ftp.js`（Lot Info FTP 分頁的鈕）＋ `web/page/ht9045_ftpclient_wire.js`。放哪請 ST01-M 定（第六節 Q-b）。

**F2（網路；筆電同意 4.5 的認領後；約 5 天）：伺服器列表、下載、上傳**

- FTPClientForm_St02.cpp 再翻：ShowFTPModal case 0（:1236-1441，NMFTP3 用 Nmftp::TNMFTP、Passive=true，照 golden；W60 精神是 KYEC 這組留在 MiniFtpEngine，列檔與接著的下載用同一套引擎）、plSLoadClick（:847-1102；照 St01 WebStart.cpp:3920-3933 寫好的順序呼叫 LoadFileFormServer2 → DownloadPasswordFormServer → W906_FTPClient_DownloadResult → 換工作檔 → btSaveSetupFile → W906_FTPClient_DownloadRecordNetData）、plUnloadClick（:1111-1212）、DownloadPasswordFormServer（:4453-4542，條件是客戶碼開的旗標，本體照 S25）。
- forms/fLotInfo_Ftp_St02.cpp 再翻：`W906_St02_LotInfo_DownloadFromServer(sDLFileName, bFromFTP)`（uLotInfo.cpp:4182-4704）、`W906_St02_LotInfo_btSaveSetupFileClick()`（:5128-5223）、`W906_St02_LotInfo_ClearAllSetupFile(sSetupFile, sSetupFile_Net)`（:11833-11890）；btSaveSetupFile 用檔內替身（Visible 旗標＋Click 轉呼叫）。
- St02 自己的線：CMakeLists.txt:3441（wb_serve 連結行，St02 ELA P3）同一行加 ht9045_kyecftp；wb_serve 開機的「安裝」與真連線開關加在 St02 自己的 wb_serve.cpp 開機行（例：:4166 行尾 ELA 那段是 St02 的；動之前 merge-tree，並確認筆電「網頁更新頻率」那一列沒有在改那一行）。
- WS 加 download、upload 兩個 op（會寫檔、會連網路；不進讀取型白名單）。

**F3（由各擁有者做，St02 只通知）**

- 筆電：ckernel.cpp:2593-2595、:2639-2641 的 `fFTPClient->bShow` 閘可以解；BarcodeReader B-F1 兩處；Command.cpp 四處 SECS／GPIB 遠端下載（ShowFTPModal(0)＋bControlBySECSGEM／ByGPIB）；csystem.cpp:8988；fMain->ShowTestHeadComp 補本體。
- St01：SECSGEM/uHGemHT9045.cpp 與 MainTimer8.cpp 的 ClearAllSetupFile 呼叫點（現在閘住，要 TfLotInfo 成員；可改呼叫 St02 的自由函式）。
- 不做：cObserver.cpp:3528 的 N26 上傳（S25）。

### 4.4 ctest 想法（St02 新檔；STEVEN-NB3 只編譯，請 St01 代跑）

- **永不連網路**：4.5 的掛鉤在 ctest 裡裝成 MiniFtpEngine 的 SetSimServerHook 假伺服器（tests/test_MiniFtpEngine.cpp 的腳本寫法：220、USER／PASS、CWD、PASV、NLST、RETR、STOR、226）；沒裝掛鉤＝模擬模式＝今天的行為。WS 層的測試只用 not-installed 與守衛（同 test_sjson_chan 不呼叫 InstallClarnDataBody 的理由）。
- **scratch 根**：W906_INIDATA_ROOT（DataPath／OffsetPath，common.cpp:204-206）、W906_HT9045LOG_ROOT；golden 硬寫的 "D:\\HT9045\\IniData\\Data\\%s\\TestMode.Data"（FTPClient.cpp:1023）與 7z 路徑在 St02 檔裡用同樣的前綴轉向（環境變數沒設＝golden 字面）；7z 的 system()／ShellExecute 走一個掛鉤（同 E-021 的 W906_E021_ExecHook），ctest 換成假解壓（直接放好檔案）。Gerneral.ini 走既有的測試轉向（TESTGUARD）。
- **案例**：(1) btnFtpServerClick 守衛：iServerEnable／iHDEnable 大於 AccessLevel、SystemStart → 不開、鈕回 Enabled；(2) 對話框已開 → "FTP Form already Opened!!"；(3) 列檔：假伺服器回 a.zip、b.Offset、c_ATC_Recipe.zip、d.txt → 清單 a、c_ATC_Recipe（golden 怪處照留）；空資料夾 → 空目錄訊息；(4) FilterList 完全相同才留；(5) 下載成功：DataPath\a_NET\ 出現、存成 a、a_NET 被刪；bKeepOnly1SetupFile 時其他工作檔被刪（只在 scratch）；bFTPDownloadSetupFile＝true；(6) 三次大小不一 → "FTP DownLoad File Size Error"、FTP_DownloadFail；(7) 上傳：PE 兩種訊息、假伺服器收到 a.zip 與 a.Offset、本機暫存檔被刪、"Upload done"；(8) Tester Name：格式不對 → "Tester Name 錯誤"；N06_TasterListMap 是 \\ 路徑時模擬版跳過（W58 Q5）；(9) 模擬版 SimNetMask 未放行時 tsFTP 看不到、WS open 回 tab-hidden。
- **real-file watch**：D:\HT9045\IniData\Data、D:\HT9045\IniData\Offset、D:\HT9045\system\Gerneral.ini、D:\HT9045\config\config.ini、D:\HT9045\7z.exe 跑前跑後一樣（請 ST01-M 加進 full_gate_template）。

### 4.5 要認領的別人的行（同一行改、行數不變）

| 檔:行 | 擁有者 | 舊（現行原文） | 新 | 前後行 |
|---|---|---|---|---|
| KYECFTP/FTPClient_Transfer.cpp:346 | 筆電 | `bool           FTP_DownloadFail = false; // golden FTPClient.cpp:46` | 同一行、`//` 之前加兩個掛鉤指標的定義：`void (*W906_KyecFtpPrepareHook)(Nmftp::TNMFTP*) = NULL;  bool (*W906_KyecFtpDownloadFromServerHook)(const AnsiString&, bool) = NULL;`（NULL＝今天的行為：模擬模式、Gate #2 回 false；extern 宣告放 St02 新 .h） | :345 `// ====...`；:347 `bool           bPIDTransferErr  = false; ...` |
| KYECFTP/FTPClient_Transfer.cpp:374 | 筆電 | `        NMFTP2 = new TNMFTP(NULL);                        // golden: new TNMFTP(this) -- no owning form exists offline` | 同一行、`//` 之前加 `if (W906_KyecFtpPrepareHook) W906_KyecFtpPrepareHook(NMFTP2);`（wb_serve 裝的版本做 SetSimMode(false)；ctest 裝的版本掛假伺服器） | :373 `    {`；:375 空行 |
| KYECFTP/FTPClient_Transfer.cpp:661 | 筆電 | `        NMFTP2 = new TNMFTP(NULL);                      // golden: new TNMFTP(this) -- no owning form exists offline` | 同上 | :660 `    {`；:662 空行 |
| KYECFTP/FTPClient_Transfer.cpp:264 | 筆電 | `bool Gated_LotInfo_DownloadFromServer(const AnsiString& /*sDLFileName*/, bool /*bFromFTP*/ = true)` | 同一行先放宣告、再把參數名打開：`extern bool (*W906_KyecFtpDownloadFromServerHook)(const AnsiString&, bool);  bool Gated_LotInfo_DownloadFromServer(const AnsiString& sDLFileName, bool bFromFTP = true)`（定義在 :346，比 :264 晚，所以這裡要先宣告） | :263 註解；:265 `{` |
| KYECFTP/FTPClient_Transfer.cpp:266 | 筆電 | `    return false;` | `    return W906_KyecFtpDownloadFromServerHook ? W906_KyecFtpDownloadFromServerHook(sDLFileName, bFromFTP) : false;`（掛鉤指標定義放 :346 同一行；NULL＝今天的替身） | :265 `{`；:267 `}` |
| WebRecipeChange.cpp:261 | Steven（8af13c07，C-route） | `static int W906_RC_ChangeSetUpFile(AnsiString FileName)` | 拿掉 `static`（先 grep 確認沒有同名） | 前後是函式本體 |
| WebRecipeChange.cpp:389 | 同上 | `static void W906_RC_cbSetupFileNameChange()` | 拿掉 `static` | 同上 |
| web/page/Data.LotInfo.html:130-133 | St01 頁（8af13c07） | :130 `  <div class="nowire">`；:131-:133 三行說明（`<b>FTP</b>…`、`可見條件：…`、`<span class="why">未接：…不接。</span>`） | :130 改成 Panel25 容器 `<div class="pnl" id="Panel25" ...>`；:131-:133 同位置放 dfm 的 btnFtpServer、btSaveSetupFile（藏）、btnDataFTPSaveToData（藏）、btnFtpHD、btnFtpTester、btnFTPTryConnect（藏）、lbFTPStatus（藏），:133 行尾加 `<script src="ht9045_lotinfo_ftp.js"></script>`；:134 `  </div>` 照留當容器結尾 | :129 `<div class="tabPane" data-pane="ftp">`；:134 `  </div>`；:135 `</div>` |

- 新視窗的話還要在 web/background.html 的 WINDOWS 表（:436-472 一帶，例 :440 observer 那一列）同一行加一列 `{id:'ftpclient', form:'fFTPClient', ... src:'page/Data.FTPClient.html', hidden:true}`；background.html 最近由筆電改（a53614c9、e977284a），要先問。
- tools/websync/sync_web.py OURS 加新 js／html（筆電改）。
- 不動：forms/fLotInfo.h、forms/fLotInfo.cpp、ckernel.cpp、cObserver.cpp、MiniFtpEngine.*、FTPClient_EventHandlers.*、WebLotInfo.cpp、tools/wb_serve.cpp 筆電「網頁更新頻率」那幾行。

### 4.6 要人工審核的（交 ST01-M 登記）

- A 操作員（用 Steven 團隊自己的測試 FTP 伺服器，W36）：模擬版先在 Configuration 勾 [FTP] Enable FTP 並存檔（W58 Q1 會寫進 config.ini）→ Lot Info 出現 FTP 頁籤 → Server 列出 zip → 選一個下載 → 主畫面工作檔換成它（不帶 _NET）→ HD 上傳 → 伺服器多 zip 與 Offset、跳 "Upload done" → Tester Name 存檔。
- B 行為：下載在主迴圈同步做，伺服器沒回應時每一步最多 20 秒，期間其他警報會晚（W62 已同意）；下載成功會刪掉 <名>_NET，CosFunction.bKeepOnly1SetupFile 的客戶會刪掉其他所有工作檔（W62：上機用真資料夾）；寫 Gerneral.ini bHasEnteredPEModel；寫 D:\HT9045\7z.exe；確安（CC_CYUEAN）結批後要靠這裡重新下載才解開 START（R120）；「成功沒」看密碼本的結果（R136，照 golden）。
- C 安全例外／偏離：模擬版放行後真的連外（W36＝C）；只有被動模式（W60＝B）；ShowModal 改成網頁視窗；fMain->ShowTestHeadComp 是空殼，存檔後 Site 畫面不重整，直到擁有者補本體。

### 4.7 工作量

F1 約 3 天、F2 約 5 天（DownloadFromServer 523 行佔 2 天）、ctest 與頁面約 2 天，合計約 8～10 天（不含 F3 各擁有者的部分）。順序建議：OB-7 先（小、等 E-021），LI-9 的 F1 可以先在本機做好、等認領同意再接 F2。

---

## 五、誰的檔（彙整）

| 檔 | 相關行 | 擁有者（git log／交接檔） | 現在有人在改嗎 |
|---|---|---|---|
| cObserver.cpp | :3292-3613（SG 家族） | 筆電 58c66991（FW-Q5） | **St01 E-021 在改檔尾**（q59 f3e2574b，未上 main） |
| cObserver.cpp | :7924 起 W906_ObserverJson | St01（8314e3bd「St01 section only」） | 同上 |
| forms/fObserver.h | :568、:846-849 | 筆電 | — |
| web/page/Data.Observer.html | :130、:234 | St01 頁；:130 機台 EastSun 8b2a666a | E-021 改 :234 |
| web/page/ht9045_observer_wire.js | — | St01 系列＋筆電 a53614c9 | E-021 改 :237／:701 |
| JsonBridge/ChanAction.cpp | :344（Steven 8deffd10）、:347（St02 572de694）、:348-349（St02 1c9a99fc）、:350（筆電 852fd6d0） | 混合 | E-021 改 :344 |
| CMakeLists.txt | :3398（St02 8f34b568）、:3441（St02 3e4ce1de）、:3097-3135（筆電 KYECFTP） | — | — |
| tests/CMakeLists.txt | :3955（St02）、:3956（St02）、檔尾 | — | 第 36 批與 E-021 都在檔尾加 |
| FileRW/MainBoot.cpp | :191-216 | Steven 4dee7107（未註明；同檔 a99d8e6c St01） | 筆電 f474abc8 WIP INBOX 127 碰過 |
| LogObjects.cpp | :91-107 | St02 80bcd1fb | — |
| KYECFTP/*、ProductionInfo/TfFTP.* | 全檔 | 筆電 8aa5ac0d、8c5e3fb6 | — |
| EventLogAnalysis/ElaFtp*.cpp、SimNet/SimNetMask.*、common.cpp:2785 | — | St02 | — |
| forms/fLotInfo.cpp／.h | FTP 成員與可見度 | 筆電（0819 FW）；:6043 起 Steven 8af13c07 | — |
| web/page/Data.LotInfo.html | :43、:129-134 | St01 頁（8af13c07）；:417 St02 li12 經 St01 同意 | — |
| WebLotInfo.cpp | lotinfo.op | St01 頁的 Steven 提交 | — |
| WebStart.cpp | :3908-3990 | St01 a99d8e6c | — |
| WebRecipeChange.cpp | :155、:261、:389 | Steven 8af13c07／a97739ae（C-route） | — |
| ckernel.cpp | :2062-2674 | 筆電 | — |
| ainarm9045.cpp | :6526-6529 | 筆電 | 筆電第 36 批在改 :746-763 |

（作者欄 Steven01 與 Steven02 都是 steven@honprec.com；St01／St02 以提交訊息與交接檔為準，沒寫的標「Steven」。）

---

## 六、問題（golden 與既有裁決答不出來的，很短）

- 給 Steven：**沒有。**（照 golden：第 0 條；客戶分支：S25；網路：W36＝C、W36-1、W58；引擎與主被動：W60＝B；同步下載與上機資料夾：W62；START 解鎖：R120、R136。）
- Q-a（給 ST01-M）：E-019 的 OB-7、LI-9 歸屬寫「待確認」——是否交給 St02？預設：是（ST02-C7 這張卡就是這個意思）。
- Q-b（給 ST01-M）：FTP 對話框做成新視窗（新頁＋background.html 一列，最像 golden 獨立的 TfFTPClient 表單）還是 Lot Info FTP 分頁裡的覆蓋層（不必動 background.html）？W62 寫「等 St01 的新頁面題 N-3」，本機的裁決檔與交接檔都找不到 N-3 原文。預設：新視窗。
- Q-c（給筆電）：4.5 表裡 KYECFTP/FTPClient_Transfer.cpp 的五行同一行認領（:264、:266、:346、:374、:661）可以嗎？之後 St02 會在自己的 CMakeLists.txt:3441 把 ht9045_kyecftp 連進 wb_serve。
