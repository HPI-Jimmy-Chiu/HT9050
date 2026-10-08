# GPIB Aux：啟用、recipe 與重啟

定位 HandlerGpibAux.cpp 的九個 body 與同名 header；完整原文見 [manifest](source-manifest.json)。以下是來源描述，本次沒有讀寫任何機台 INI／recipe。

| function／變數 | 選讀行為 |
| --- | --- |
| W906_GpibAuxEnable | 設 g_enabled；清 applies／deferred／asked，startPack=-1，發布 framing=-1 |
| BeforeBridgeStart | 先重設；enabled、有效 type=GPIB、實際 TestIF.iTestType=GPIB 都成立，才依 InitialOK 進 Prepare 或 deferred；最後發布值 |
| PortOn | CC_HONPREC_QC、CC_MAXIM_THAILAND、CC_MAXIM、CC_Microchip_Thai／Phil／China、CC_SCC 預設 true；檔案存在就讀 OpenTesterComm 的 RS232，以上述預設回退 |
| ReadSetupIni | 預設 baud=9600、ByteSize=2、StopBits=0、Parity=2；檔案存在才讀 COMPort 四欄 |
| SetupIniPack／PackRecipe | 前者直接 pack Setup.ini；後者將 TestIF_File.Rs232_Data 經 W906_Rs232* ordinal 映射後 pack |
| Prepare | 清 applies／startPack；實際非 GPIB 或 PortOn false 返回 -1；其餘 applies=true，組 recipe 的 Tester.Data，呼叫 SeedRecipeFile |
| Prepare／r==1 | 更新記憶體 Rs232_Data；有 refresh callback 就呼叫，有 fMain 就 BackupSetupFile，再記 NewRecordProcess |
| Prepare／r<0 | 記 not seeded；仍計算 startPack，返回 -1；其餘返回 pack，負值另記 framing 無效 |

SeedRecipeFile 先確認 Tester.Data 存在；marker 已為 1 就返回 0，沒有在該分支重讀四欄更新 rd。未 seed 時複製原 rd、讀 Setup.ini 值／預設；baud>0 才覆寫，ordinal 反向映射>=0 才覆寫各欄，再 WriteIniData 四欄及 W906_SeededFromSetupIni=1。回讀只確認 marker 與 BaudRate；失敗 -1，成功才設 detail、*rd 並返回 1。本 body 沒有全四欄回讀、原子交易或失敗回復。

Prepare 因此可能在 engine 啟動前寫 recipe、refresh 頁面與備份。BeforeBridgeStart 的啟用 guard 不是所有 public helper 的總開關；PortOn／SeedRecipeFile 可由其他 caller 直接呼叫，全 caller 仍待查。

NeedsRestart 先擋 !enabled、!bridgeUp、asked、!InitialOK 或 SystemStart。deferred 時清旗標、Prepare／store；publish<0 或等於 SetupIniPack 就 false，否則 asked=true 並 true。非 deferred 時，未 applies 或 recipe pack 等於 startPack 就 false，否則標 asked 並 true。此 body 只提出請求，沒有執行 Restart；tick／CloseGpibProgram／Wakeup caller 未閉合。

header 的 D:\GPIB9045\system\general.ini、D:\RS232Standard\System\Setup.ini 是保留的程式路徑；不代表本機已同步機台檔，也不是本次實測。

回 [索引](index.md)、[設定](settings.md)、[界線](limits.md)。
