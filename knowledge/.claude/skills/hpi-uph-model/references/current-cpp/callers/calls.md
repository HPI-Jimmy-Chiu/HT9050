# AddLoadingCount 的所選呼叫區段

來源檔／版本與保存見 [來源入口](index.md)／[manifest](source-manifest.json)。定位各DoInArmPickFromLoadStage／DoArmPickFromLoadStage的InArmSuckUse、NULL_IC、bSuckEnd、Suck()、iYpos／iXPosition與AddLoadingCount；不以程式行號為主要參照。

七個所選call均位於i／j迴圈，先看InArmSuckUse[i][j]與Item[i][j]==NULL_IC，再要求bSuckEnd[i][j]==false與Suck()回傳為true。Suck helper、完整Task／retry／初始化／其他寫者尚未讀完，不能把一次文字call當作吸取、輸出或整顆計數成功。

| 所選caller／版本 | 所見迴圈上限 | 第3／4個實參（callee名為iTrayRow／iTrayCol） | 本處差異 |
| --- | --- | --- | --- |
| DoInArmPickFromLoadStage_9045；V906／V912 | iMaxRow／iMaxCol | iYpos／iXPosition[j] | 負iXPosition先顯示WAR0149並continue |
| DoArmPickFromLoadStage_9045_1x2_1；V912 | MAX_ARM_Row／MAX_ARM_Col | iYpos／iXPosition[j] | 本區在call前設bSuckEnd=true、InArmSuckUse=false、清bPickLoaderDuplicateErr；不推定所有retry口徑 |
| DoArmPickFromLoadStage_9045_2x4_16；V906／V912 | iPickRow／iPickCol | iXPosition[j]／iYpos | 附近SOFT_SIMULTE的chkInPickLoadError分支會改Error／bSuckEnd，未判作用中編譯設定 |
| DoArmPickFromLoadStage_9045_2x8_32；V906／V912 | MAX_ARM_Row／MAX_ARM_Col | iXPosition[j]／iYpos | V906此完整舊body在#if 0；V912所見call沒有字面停用guard |

iYpos在所選區段由iYPosition+iLoadPitchStepY*i計算。參數方向不同是此版本的文字事實；完整座標映射、機型dispatch、有效MAX／iPick／iMax值、site／HP／Tray容量仍未驗，不稱為對調錯誤，也不以檔名推定現場使用的配置。

所選標準／2x4／2x8區段的Error分支與非Use／非NULL分支另設bSuckEnd；1x2的成功分支在call前也改旗標。call前的旗標位置不同，仍須連同既有 [AddLoadingCount body](../count-time.md) 的提前return與其他旗標writer查證；本層不宣稱每格／每顆恰好增量一次，也不乘上site或整排數。

[Kernel](../state/kernel/index.md) 暫停開始與 [state](../state/index.md) 累加／重置另分層保存。全部上層caller、MainProc／asendic_Loader／MoveInArmXYToLoader、counter與reset生命週期、producer／consumer／落盤和時間有效性仍待閉合。
