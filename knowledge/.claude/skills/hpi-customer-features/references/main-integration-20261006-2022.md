# 20時推送前main來源核對

已pull整合main `ef41839d4f3f39be54202252f51b46fda561b3e4`。V912未改，V906有新的機型／PLC／caller來源；舊原文與source341cea3d7候選樹完整保留，不能將舊查證日期自動當成新實機結果。

## 客戶候選再掃描

[本次摘要／source blob](versions/main-check-ef41839d4.json)記錄唯讀重掃2737個Git來源檔（V912 717／V906 2020）。仍為6382筆詞法候選，忽略來源blob後候選定位／條件提示／guard／出現次數沒有增刪。來源manifest已改，相關檔blob另列；數量相同不等於完整條件、caller或機台行為相同。

舊8列人工查證來源保留09d926ab8；ART、名稱、MachineType與各caller原檔未改，CMakeLists新增SafePLC來源，但原AutoRetest／FileRW_HSys收錄保留。本次不是覆寫舊candidate ID或自動回填其他主題表。

## HT9050與其他Handler共同／差異

| 主題 | 最新main靜態查證 | 適用界線 |
|---|---|---|
| START教點檢查 | cinitial.cpp::CompareTechData以W906_GpibModel==9050GPIB或MachineTypeChoice==Type_HT9050辨識；HT9050略過HotPlate2、三組Shuttle／Auto1-2、Fix組與原Auto2-3檢查 | 其他機型保留原分流；HT9050 Auto順序改走W906_Ht9050AutoOrderRefused，W906_HT9050_AUTO_ORDER_CHECK預設0；不是由客戶碼決定 |
| Shuttle與W-44 | acarry.cpp新增W906_ShtPosInSafeZone／W906_Ht9050ShtToSocketBlocked；安全帶W906_HT9050_SHT_SAFE_BAND預設1000，以home到socket方向判斷，移動中亦核對iOldPos | 正文Steven裁決不改；Type_HT9050的M108與Out Shuttle條件另列，Index press完整caller仍不能由hook安裝推定 |
| PLC模式 | MyPLC/SafePlcClock_St02.cpp::W906_SafePlcFastClockAdd在clock存在且Enable_PLCSafety_IO成立才切POLLED-Real並加1ms job；tick也看Enable_PLCSafety_IO | FastClockWbServe.cpp::Start有caller、CMake列來源；不宣稱各機台SafePlcIO值、通訊已連線或門已安全，沒有改runtime |
| Fix-AI CCD | 出料手臂的OutArmCycleCounterUpdate呼叫gate由#if0改#if1，外部仍有USE_Fix_AI_CCD及bEnableFix2BGAAICCD條件 | forms/fFixAICCD.cpp的同名函式仍空本體；不能說counter UI已實作，適用配置及caller另核對 |
| HeaterThread | uHeaterThread.cpp::HeaterThreadProcess開放CheckATC6System／HeaterDoorIsOpen／DoHeaterOn呼叫 | 只是此副本的gate開放；不能由Execute副本推定實際thread已啟動，FastClockHeaterBeat仍是另一路caller |

來源均用釘住版本＋檔名／function／變數定位。[AutoStart／AutoClean](../../hpi-autostart-autoclean/SKILL.md)、[Config](../../hpi-config/SKILL.md)、[Web HMI](../../hpi-web-hmi/SKILL.md)同批入口導讀本頁。原文、metadata、舊名、資源與W-44原裁決正文保持；只作文件／Git來源核對，未編譯、操作機台、API或執行期設定。
