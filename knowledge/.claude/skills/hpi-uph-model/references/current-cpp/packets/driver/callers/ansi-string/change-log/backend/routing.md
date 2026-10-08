# Change Log 與 Process 的現行路由

[cMyDB.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/98de4512b969bda046947413191dbb31d31aae31/HT9011UC_Cpp_V3.33.906.0/cMyDB.cpp)、[LogObjects.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/98de4512b969bda046947413191dbb31d31aae31/HT9011UC_Cpp_V3.33.906.0/LogObjects.cpp)、[aHotPlateSubstrate.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/98de4512b969bda046947413191dbb31d31aae31/HT9011UC_Cpp_V3.33.906.0/aHotPlateSubstrate.cpp)。完整所選原文、介面、歷史裁決與 byte／body hash 見 [manifest](source-manifest.json)。

RecordChangeLogProcess(S,S2) 在 CUSTOMER_CODE 既非 CC_ASE_KaohSiung、也非 CC_SPIL_SHINCHU，且 S==ExString、S.Pos(" pressed")!=0 時 return。其他情形先 ExString=S，再 MyDBIProcess("ChangeLog",S,S2)。這不是所有重複訊息去重；比較不含S2，也沒有以 bSPILFunction 代替該客戶條件。

ExString 是 [cMyDB.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/98de4512b969bda046947413191dbb31d31aae31/HT9011UC_Cpp_V3.33.906.0/cMyDB.h) 的 extern AnsiString。RecordProcess／NewRecordProcess 也讀寫它，但 gate 是非ASE客戶且 IniConfig.bSPILFunction==false；共享狀態可能跨訊息類別影響所選 pressed 分支，沒有鎖或各類獨立cache。NewRecordProcess 的 Greatek／bN14_1_EnableOEEFunction SaveMessageHistroy 呼叫在去重前；Debug空字串改空白，後續走MyDBIProcessNew。此helper的callee／form生命週期未在本輪閉合。

## 三參數 MyDBIProcess

正文先組 SData="@e02002"+S1+S2；ASE且S1不含ATC才呼叫RespondASECom。這只證明呼叫條件；該通訊callee、設備連線／送出成功不在本輪驗證。

接著組INSERT，呼叫MyDBExecSQL，再ProductionLog(S1+S2,true)。建立TStringList並按bSPILFunction拼cell；slEventLog非null才AddTextWithLineNo或AddTextWithDateTime、SaveEventLog。最後清除／delete該暫存list，Code="220000000"，SaveEventLogInfo(Code,S1,22," ")。外層catch(...)只把local str改成"!!"，沒有向外回報錯誤或證明前面的所有寫入成功。

table字串ChangeLog與下游iType=22是兩個欄位軸。SaveEventLogInfo只在iType==21時用CHANGE LOG，22落入PROCESS；RecordChangeLogProcess實際傳22，原文保留。不能把table名直接當事件KEYWORD，或替程式改成21。

MyDBIProcessNew 先傳ASE訊息、組含AlarmCode的INSERT與文字list；空白cell位置跟三參數版本不同。尾端用AlarmCode.SubString、atoi組Code="2%02d%06d"，同樣傳iType22，另帶S2作sErrPart。這個函式沒有三參數版本的整體try／catch；字串解析、清理／失敗時的副作用不能宣稱等效或交易原子性。

## 兩參數轉接器

aHotPlateSubstrate.cpp 的MyDBIProcess(S1,S2) 增加W906_MyDBIProcess_Count、保存LastS1／LastS2，接著MyDBIProcess(S1,S2,AnsiString(""))。其第一參數成asTable、第二參數成實際payload S1、最後為空S2；它已是轉接器，觀察counter不是整個後端只有計數的證據。W906_MyDBIProcess_Reset只清這三個觀察global。

該TU的extern三參數宣告不帶default；cMyDB.h保留S2=""。header的歷史default重複宣告、fastcall裁決及其他匿名namespace同名folder說明完整保存；沒有由名字相同推定所有caller都繫結同一overload。本輪不完成其他folder／全部caller／ABI。

回 [SQL gate](sqlite-gates.md)、[事件欄位](event-rows.md)、[未驗項](limits.md)。
