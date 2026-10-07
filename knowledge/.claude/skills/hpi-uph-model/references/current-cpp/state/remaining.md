# 候選盤點與仍待閉合的範圍

[manifest](source-manifest.json) 的 candidate_inventory 釘住 0c2eac30b4e56fec64b207f6adcc3ead69711903：V906／V912 1120 個追蹤 .cpp，排除 .svn／docs／tests，以 iUPH_LoaderCount、bRecordUPH、tUPH_PauseTime、tUPH_PauseStartTime、tUPH_StartTime、bCalculatePauseTime 六個文字詞盤點，16個候選檔。清單與SHA可重現。

原文搜尋可能命中註解、停用區、read-only consumer、定義、讀取或寫入；檔案命中不是活路徑、完整writer／caller或語意完成。四詞舊盤點只有14候選，補pause-start／calculate旗標後找到兩版ckernel.cpp；此處仍只把它列為後續候選。

本單元核對六個source blob／24定義與extern，以及兩版三個函式的保存hash；真正已讀的是20個reset附近區段與2個MainProc區段。六個完整body都明標未完成語意查證，沒有把hash存在說成已讀完整函式。

[Kernel子樹](kernel/index.md) 已補所選pause-start／刷新區段與兩排程body；[InArm caller子樹](../callers/index.md) 已補七call區段與一短stub，完整UI／caller未閉合。[Loader子樹](loader/index.md) 已補DoSupplyNewICTray的短入口／case 1300與bRecordUPH局部條件；其餘Task與helper未閉合。下一步追其他旗標寫者、上層Loader caller／派工、其他reset、WebBridgeTags／DB／SECS／UI／CSV consumer、實際保存結果、機型容量／site／校正。所有callback／機構動作／runtime、thread synchronization與平台日期等價仍未驗證。

HT9050與其他Handler仍使用[同題機型樹](../../machines/index.md)，不把所選V906／V912區段推成作用中HT9050測試結果。原文／metadata／資源／舊入口／裁決保留；沒有執行機台、C++／build、API／LIVE或寫runtime。
