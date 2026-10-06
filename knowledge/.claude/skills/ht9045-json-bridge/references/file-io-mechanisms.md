# HT9045 BCB6 的三套讀寫檔機制 —— 實作層對照

舊引用路徑保留；[讀取整理後文件](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms.md)。

## 總表

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/01.md#總表)

## A. ini 文字：`ReadIniData` / `WriteIniData`（`common.cpp`）

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/02.md#a-ini-文字readinidata--writeinidatacommoncpp)

### A.1 快取：一次只開一個檔

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/02.md#a1-快取一次只開一個檔)

### A.2 讀：四個多載，找不到就回預設值，**不寫回**

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/02.md#a2-讀四個多載找不到就回預設值不寫回)

### A.3 寫：六個多載，帶 Change Log 副作用

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/02.md#a3-寫六個多載帶-change-log-副作用)

### A.4 對橋接層的意義

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/02.md#a4-對橋接層的意義)

## B. `HTEditList`：控制項驅動的 ini（`Public/HTEditList.cpp`）

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/03.md#b-hteditlist控制項驅動的-inipublichteditlistcpp)

### B.1 設計意圖（檔頭註解 `:2-25`，Steven 20170629）

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/03.md#b1-設計意圖檔頭註解-2-25steven-20170629)

### B.2 型別（`Public/HTEdit.h`，`TEditContent`）

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/03.md#b2-型別publichtedithteditcontent)

### B.3 寫：`SaveEditTextToFile(Path, FileName)`（`:574-1007`，429 行）

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/03.md#b3-寫saveedittexttofilepath-filename574-1007429-行)

### B.4 讀：`ReadEditTextFromFile(Path, FileName)`（`:1009-1321`，306 行）

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/03.md#b4-讀readedittextfromfilepath-filename1009-1321306-行)

### B.5 對橋接層的意義

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/03.md#b5-對橋接層的意義)

## C. 二進位 blob：整塊 `sizeof(struct)`

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/04.md#c-二進位-blob整塊-sizeofstruct)

### C.1 原語（`cprod.cpp:1320` / `:1340`）

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/04.md#c1-原語cprodcpp1320--1340)

### C.2 `LAST_LEVEL_SET` ↔ `system\levelset.dat`（`cSecurity.cpp`）

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/04.md#c2-last_level_set--systemlevelsetdatcsecuritycpp)

### C.3 `LAST_GENERAL_SET` ↔ `system\lastdata.dat`（`cprod.cpp`）

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/04.md#c3-last_general_set--systemlastdatadatcprodcpp)

### C.4 對橋接層的意義

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/04.md#c4-對橋接層的意義)

## D. 自由函式與總指揮（不屬於任何表單）

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/05.md#d-自由函式與總指揮不屬於任何表單)

## F. Offset 族：物件 setter ＋ 變數區段 ＋ 第四種格式（使用者 20260923 點名）

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/06.md#f-offset-族物件-setter--變數區段--第四種格式使用者-20260923-點名)

### F.1 資料型別

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/06.md#f1-資料型別)

### F.2 讀（`TfOffSet::ReadFile()`，`cOffSet.cpp:1998`）

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/06.md#f2-讀tfoffsetreadfilecoffsetcpp1998)

### F.3 寫（`TfOffSet::SaveFile(iSelPartData, SpecialMode, bReset)` → `SaveSetupFile(...)`，`:1406`／`:1470`）

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/06.md#f3-寫tfoffsetsavefileiselpartdata-specialmode-breset--savesetupfile14061470)

### F.4 第四種格式：Tab 分隔文字（`TStringList`）

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/06.md#f4-第四種格式tab-分隔文字tstringlist)

### F.5 移植樹現況

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/06.md#f5-移植樹現況)

### F.6 對橋接層的意義

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/06.md#f6-對橋接層的意義)

## G. `IniConfig` ↔ `config.ini` 族：`ReadLastSetIni()`／`SaveLastSetIni()`（使用者 20260923 點名）

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/07.md#g-iniconfig--configini-族readlastsetinisavelastsetini使用者-20260923-點名)

### G.1 `ReadLastSetIni()`（`cprod.cpp:2977-3090`）

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/07.md#g1-readlastsetinicprodcpp2977-3090)

### G.2 `SaveLastSetIni()`（`cprod.cpp:3092-3148`）

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/07.md#g2-savelastsetinicprodcpp3092-3148)

### G.3 第五種呼叫形式：`ReadWriteIni(path, 區段, 鍵, 現值, 預設, bRead[, bCheckRange, max, min])`

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/07.md#g3-第五種呼叫形式readwriteinipath-區段-鍵-現值-預設-bread-bcheckrange-max-min)

### G.4 對橋接層的意義

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/07.md#g4-對橋接層的意義)

## H. `TfDIOFrom` ↔ `iniData\DioCfg\*.ini`（`Config.DIOInterFaceCFG.html`）

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/08.md#h-tfdiofrom--inidatadiocfginiconfigdiointerfacecfghtml)

## I. 留痕（event log）：`RecordProcess` 族 → `MyDBIProcess`（使用者 20260923 點名）

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/09.md#i-留痕event-logrecordprocess-族--mydbiprocess使用者-20260923-點名)

### 移植樹現況

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/09.md#移植樹現況)

## E. 三套機制在移植樹的現況（20260923）

[讀取此節](../../hpi-web-hmi/references/bridge/references/file-io-mechanisms/10.md#e-三套機制在移植樹的現況20260923)
