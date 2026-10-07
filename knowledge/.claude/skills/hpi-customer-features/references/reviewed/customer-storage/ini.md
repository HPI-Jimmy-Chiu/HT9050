# INI包裝：介面與寫入結果的界線

來源：[V906 common.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Cpp_V3.33.906.0/common.cpp)的 `OpenGeneralIniFile`／四個 `WriteIniDataGeneral` overload；[IniFiles.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Cpp_V3.33.906.0/vclcompat/IniFiles.cpp)與[IniFiles.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Cpp_V3.33.906.0/vclcompat/IniFiles.h)的TIniFile、TIniStore與下述具名helper；[V912 common.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/common.cpp)只核對四個WriteIniDataGeneral overload。

## V906普通TIniFile

OpenGeneralIniFile以 `new TIniFile(asGeneralPath)`建立INIFileGeneral；asGeneralPath來源與路徑分流見[開機reader](../customer-boot/index.md)，不開啟或修改實際runtime檔。普通TIniFile constructor令writeThrough_=true。CUSTOMER_CODE走int overload：WriteIniDataGeneral → INIFileGeneral.WriteInteger → WriteString → W906_Win32WriteProfileString。

W906_Win32WriteProfileString讀入原檔buf、經W906_IniApplyFast產生out；已讀原檔、out等於buf且W906_IniTouchSame成立時直接return。否則嘗試fopen("wb")後fwrite／fclose；probe表示路徑不存在時不嘗試開檔。這個helper是void，fopen失敗沒有向caller回傳成功旗標，這個body也沒有檢查fwrite／fclose回傳值。名稱含Win32不代表本段使用WritePrivateProfileString API。

WriteIniDataGeneral四個overload與WriteInteger／WriteString均為void。普通writeThrough模式的UpdateFile不另做flush；因此不能靠SaveSystemSet:write、Web saved=true或後續UpdateFile，證明每個鍵寫入成功。檔案是否存在、實際內容與回讀結果沒有在這次文件工作中驗證。

## 記憶體模式另看

MemTag constructor令writeThrough_=false並LoadFromFile；此模式WriteString走store_.WriteRaw，UpdateFile才呼叫flush → TIniStore::SaveToFile。WriteRaw修改記憶體資料，SaveToFile另嘗試fopen("wb")並輸出，其介面同樣是void。這是相容層中的模式差異，不宣稱本客戶碼caller用了TMemIniFile，也未查完所有生命週期與呼叫者。

ReadInteger依writeThrough_選W906_DiskIniReadRaw或store_.ReadRaw，沒有found時給def。只核對這個選路與介面，尚未全量追完磁碟reader、格式helper／錯誤傳播、所有writer或併發／原子性。

## V912與機型差異

V912 common.cpp四個WriteIniDataGeneral wrapper也分別呼叫INIFileGeneral的WriteBool／WriteInteger／WriteString，介面為void。BCB TIniFile底層這次沒有讀到，不能把V906相容層的writeThrough_、flush或檔案失敗處理當成V912已驗證事實。HT9050與其他機型的共用介面與版本差異留在同一Skill；實際runtime路徑、設定與存檔後效果另行查證。
