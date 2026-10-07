# 共同介面與資料辨識

本次main `2db43115d`核對V912與V906兩份Interface/InterfaceSYS.cpp的SendCommand_EventLog函式正文：均由CosFunction.bUseMDB==false包住EventLog指令，外部視窗以TfrmELA／Event Log Analyzer尋找，M_V帶CMD，WM_COPYDATA／WM_EventAnalysis傳送。V906另外先呼叫非空W906_ElaPostHook；這是兩版本的共同與差異，未驗全caller／build define或實際機台狀態，來源見[局部查證](runtime/source-review.md)。

TfObserver::GetEventLogText是Handler單檔檢視；TfrmELA::GetEventLogText是舊分析器多日統計，V906由ela::Analyzer等核心承接。同名不代表同一分析範圍，原對照與案例見[保存正文](ela/original-entry.md)。JsonBridge的log.tail是執行期事件串流，不能直接當EventLogTxt CSV或多日統計；[JSON bridge](../../ht9045-json-bridge/SKILL.md)另管串流。

資料來源需記錄版本、日期區間、檔名週期、CSV欄位／編碼、客戶碼與功能開關，再選[分析](analysis/index.md)或[報表](reports/index.md)。SPIL、VTEST及客戶報表的差異見[客戶表](customers.md)，HT9050與其他機型同題，但本次未重新查全機型分支，不能推成每台配置相同。

讀取CSV可能引出JAM0000.dat缺鍵寫回、ReadConfig補key、報表寫檔／上傳與排程。文件整理不呼叫任何分析器／API或讀寫機台真檔；原測試記錄不作本次執行證據。
