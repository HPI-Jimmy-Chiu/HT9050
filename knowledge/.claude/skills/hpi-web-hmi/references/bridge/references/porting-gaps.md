# JSON 橋接層擋在哪裡 —— 需要移植的部分

按需要選取以下章節，原文依順序保留。

- [JSON 橋接層擋在哪裡 —— 需要移植的部分](porting-gaps/00.md)
- [這份文件在講什麼](porting-gaps/01.md)
- [一、⛔ 最高優先：`main.cpp` 整支不存在（型態 A）](porting-gaps/02.md)
- [二、⛔ 溫控讀值：執行緒存在但沒有人啟動它（型態 B）](porting-gaps/03.md)
- [三、溫控超溫告警升級被閘擋住（型態 C，SAFETY）](porting-gaps/04.md)
- [四、`TfContact::ReadFile()` 沒有本體（型態 A）](porting-gaps/05.md)
- [五、權限名稱層被閘擋住（型態 C，gate SEC1）](porting-gaps/06.md)
- [六、留痕（RecordProcess 族）三個入口都不是真的](porting-gaps/07.md)
- [七、`fTemperFrom` 單例不存在（型態 A，影響 S7 v2）](porting-gaps/08.md)
- [八、S7 專屬：`sv` 上線時有 15 個通道 golden 從未賦值](porting-gaps/09.md)
- [九、S7 專屬：`bTemperatureReady[]` 的初值本身就是壞的](porting-gaps/10.md)
- [十、~~`iTo3Unload[]` 宣告了但沒人填~~ ⛔ **整節作廢（20260923 同日撤回）—— 它早就填好了**](porting-gaps/11.md)
- [十一、`fSortCT->ShowLoadingIC()`／`ShowSortIC()` 是 no-op facade（型態 A）](porting-gaps/12.md)
- [十二、`MyForceDirectories(QtyData\YYYYMM)` 跑在守衛前面 —— **這不是缺陷，是忠於 golden**](porting-gaps/13.md)
- [十三、`ATC.ini` 沒被讀 → 開機把配方的 `Chiller Temp` 改寫掉（型態 A，**會寫壞配方檔**）](porting-gaps/14.md)
- [十四、`TFTestIF::ReadTestIFFile()` 還在 GATE (F-5)（型態 C，**待辦**，使用者 20260924 指示列入）](porting-gaps/15.md)
- [十五、`slEventLog` 全樹是 NULL——golden `TfMain` 建構子那段沒翻（型態 A，20260926 新增）](porting-gaps/16.md)
- [十六、開機少 golden `TfMain::FormShow` 的 `SetRunStartMode`——Tester Off-Line 機台開機顯示 Re-Test（型態 B，20260926 新增）](porting-gaps/17.md)
- [十七、`INDEX_SUCKER_TYPE` 只解了一半——開機讀了，但存檔仍即時寫記憶體（型態 C，20260926 新增）](porting-gaps/18.md)
- [十八、`BinCount.txt` 三個寫點在 SEAM S2 空替身裡——移植樹目前完全不寫這個檔（型態 C，20260926 新增，含 0925 稽核更正）](porting-gaps/19.md)
- [十九、筆電 `IO_CARD_TYPE=1` 時 IO 表沒綁——第 42 條 IO 解除這台機器測不到（環境限制，不是缺陷，20260926 新增）](porting-gaps/20.md)
- [二十、HT9050 上 golden IO 物件打不到 1203 卡——`uiDevhand` 只在 `INSTALL_ETHETCAT` 成立時填（型態待確認，20260926 新增）](porting-gaps/21.md)
- [二十一、網頁存檔不檢查運轉狀態——golden 設定鈕只在停機時按得到（偏離 golden 的缺閘，**SAFETY 相關，待 Jimmy 決定**，20260926 新增）](porting-gaps/22.md)
- [二十二、St01 這一輪照建議先做的 `[W906]` 偏離與時序差異（20260926～27，**Steven 可推翻**，20260927 新增）](porting-gaps/23.md)
- [排序建議](porting-gaps/24.md)
- [複驗指令](porting-gaps/25.md)

# JSON 橋接層擋在哪裡 —— 需要移植的部分

[讀取此節](porting-gaps/00.md#json-橋接層擋在哪裡--需要移植的部分)

## 這份文件在講什麼

[讀取此節](porting-gaps/01.md#這份文件在講什麼)

## 一、⛔ 最高優先：`main.cpp` 整支不存在（型態 A）

[讀取此節](porting-gaps/02.md#一-最高優先maincpp-整支不存在型態-a)

## 二、⛔ 溫控讀值：執行緒存在但沒有人啟動它（型態 B）

[讀取此節](porting-gaps/03.md#二-溫控讀值執行緒存在但沒有人啟動它型態-b)

## 三、溫控超溫告警升級被閘擋住（型態 C，SAFETY）

[讀取此節](porting-gaps/04.md#三溫控超溫告警升級被閘擋住型態-csafety)

## 四、`TfContact::ReadFile()` 沒有本體（型態 A）

[讀取此節](porting-gaps/05.md#四tfcontactreadfile-沒有本體型態-a)

## 五、權限名稱層被閘擋住（型態 C，gate SEC1）

[讀取此節](porting-gaps/06.md#五權限名稱層被閘擋住型態-cgate-sec1)

## 六、留痕（RecordProcess 族）三個入口都不是真的

[讀取此節](porting-gaps/07.md#六留痕recordprocess-族三個入口都不是真的)

## 七、`fTemperFrom` 單例不存在（型態 A，影響 S7 v2）

[讀取此節](porting-gaps/08.md#七ftemperfrom-單例不存在型態-a影響-s7-v2)

## 八、S7 專屬：`sv` 上線時有 15 個通道 golden 從未賦值

[讀取此節](porting-gaps/09.md#八s7-專屬sv-上線時有-15-個通道-golden-從未賦值)

## 九、S7 專屬：`bTemperatureReady[]` 的初值本身就是壞的

[讀取此節](porting-gaps/10.md#九s7-專屬btemperatureready-的初值本身就是壞的)

## 十、~~`iTo3Unload[]` 宣告了但沒人填~~ ⛔ **整節作廢（20260923 同日撤回）—— 它早就填好了**

[讀取此節](porting-gaps/11.md#十ito3unload-宣告了但沒人填--整節作廢20260923-同日撤回-它早就填好了)

### ~~十、`iTo3Unload[]` 宣告了但沒人填 → QtyLog 六格全寫同一個 Bin（型態 A）~~

[讀取此節](porting-gaps/11.md#十ito3unload-宣告了但沒人填--qtylog-六格全寫同一個-bin型態-a)

## 十一、`fSortCT->ShowLoadingIC()`／`ShowSortIC()` 是 no-op facade（型態 A）

[讀取此節](porting-gaps/12.md#十一fsortct-showloadingicshowsortic-是-no-op-facade型態-a)

## 十二、`MyForceDirectories(QtyData\YYYYMM)` 跑在守衛前面 —— **這不是缺陷，是忠於 golden**

[讀取此節](porting-gaps/13.md#十二myforcedirectoriesqtydatayyyymm-跑在守衛前面--這不是缺陷是忠於-golden)

## 十三、`ATC.ini` 沒被讀 → 開機把配方的 `Chiller Temp` 改寫掉（型態 A，**會寫壞配方檔**）

[讀取此節](porting-gaps/14.md#十三atcini-沒被讀--開機把配方的-chiller-temp-改寫掉型態-a會寫壞配方檔)

## 十四、`TFTestIF::ReadTestIFFile()` 還在 GATE (F-5)（型態 C，**待辦**，使用者 20260924 指示列入）

[讀取此節](porting-gaps/15.md#十四tftestifreadtestiffile-還在-gate-f-5型態-c待辦使用者-20260924-指示列入)

## 十五、`slEventLog` 全樹是 NULL——golden `TfMain` 建構子那段沒翻（型態 A，20260926 新增）

[讀取此節](porting-gaps/16.md#十五sleventlog-全樹是-nullgolden-tfmain-建構子那段沒翻型態-a20260926-新增)

## 十六、開機少 golden `TfMain::FormShow` 的 `SetRunStartMode`——Tester Off-Line 機台開機顯示 Re-Test（型態 B，20260926 新增）

[讀取此節](porting-gaps/17.md#十六開機少-golden-tfmainformshow-的-setrunstartmodetester-off-line-機台開機顯示-re-test型態-b20260926-新增)

## 十七、`INDEX_SUCKER_TYPE` 只解了一半——開機讀了，但存檔仍即時寫記憶體（型態 C，20260926 新增）

[讀取此節](porting-gaps/18.md#十七index_sucker_type-只解了一半開機讀了但存檔仍即時寫記憶體型態-c20260926-新增)

## 十八、`BinCount.txt` 三個寫點在 SEAM S2 空替身裡——移植樹目前完全不寫這個檔（型態 C，20260926 新增，含 0925 稽核更正）

[讀取此節](porting-gaps/19.md#十八bincounttxt-三個寫點在-seam-s2-空替身裡移植樹目前完全不寫這個檔型態-c20260926-新增含-0925-稽核更正)

## 十九、筆電 `IO_CARD_TYPE=1` 時 IO 表沒綁——第 42 條 IO 解除這台機器測不到（環境限制，不是缺陷，20260926 新增）

[讀取此節](porting-gaps/20.md#十九筆電-io_card_type1-時-io-表沒綁第-42-條-io-解除這台機器測不到環境限制不是缺陷20260926-新增)

## 二十、HT9050 上 golden IO 物件打不到 1203 卡——`uiDevhand` 只在 `INSTALL_ETHETCAT` 成立時填（型態待確認，20260926 新增）

[讀取此節](porting-gaps/21.md#二十ht9050-上-golden-io-物件打不到-1203-卡uidevhand-只在-install_ethetcat-成立時填型態待確認20260926-新增)

## 二十一、網頁存檔不檢查運轉狀態——golden 設定鈕只在停機時按得到（偏離 golden 的缺閘，**SAFETY 相關，待 Jimmy 決定**，20260926 新增）

[讀取此節](porting-gaps/22.md#二十一網頁存檔不檢查運轉狀態golden-設定鈕只在停機時按得到偏離-golden-的缺閘safety-相關待-jimmy-決定20260926-新增)

## 二十二、St01 這一輪照建議先做的 `[W906]` 偏離與時序差異（20260926～27，**Steven 可推翻**，20260927 新增）

[讀取此節](porting-gaps/23.md#二十二st01-這一輪照建議先做的-w906-偏離與時序差異2026092627steven-可推翻20260927-新增)

## 排序建議

[讀取此節](porting-gaps/24.md#排序建議)

## 複驗指令

[讀取此節](porting-gaps/25.md#複驗指令)

# ── 一：bUT150Install 的填值來源分布（在 GOLDEN V912 跑）──────────────

[讀取此節](porting-gaps/25.md#-一but150install-的填值來源分布在-golden-v912-跑)

# raw 計數（含註解掉的）：main.cpp 1091 / csystem.cpp 11

[讀取此節](porting-gaps/25.md#raw-計數含註解掉的maincpp-1091--csystemcpp-11)

# 活賦值的逐函式分布：931 / 71 / 36 / 28 / 18 / 1(建構子清零)

[讀取此節](porting-gaps/25.md#活賦值的逐函式分布931--71--36--28--18--1建構子清零)

# ⚠ 最後一行印出空白名字的那筆 1 是建構子 TfMain::TfMain（main.cpp:2253-2256），

[讀取此節](porting-gaps/25.md#-最後一行印出空白名字的那筆-1-是建構子-tfmaintfmainmaincpp2253-2256)

#   它是 `for(i<tcTotalCount) bUT150Install[i]=false;` 的**全清零初始化**，

[讀取此節](porting-gaps/25.md#--它是-foritctotalcount-but150installifalse-的全清零初始化)

#   不是「決定哪些有裝」。不要去翻它。

[讀取此節](porting-gaps/25.md#--不是決定哪些有裝不要去翻它)

# ── 二～九：以下全部在 移植樹 跑 ──────────────────────────────────────

[讀取此節](porting-gaps/25.md#-二九以下全部在-移植樹-跑-)

# 二：heater 執行緒的啟動點（去掉註解後應為零）

[讀取此節](porting-gaps/25.md#二heater-執行緒的啟動點去掉註解後應為零)

# 三：GATE (T1)

[讀取此節](porting-gaps/25.md#三gate-t1)

# 四：TfContact::ReadFile 的本體（應為零，只有註解）

[讀取此節](porting-gaps/25.md#四tfcontactreadfile-的本體應為零只有註解)

# 五：SEC1 閘與名字數

[讀取此節](porting-gaps/25.md#五sec1-閘與名字數)

# 六：三個留痕入口

[讀取此節](porting-gaps/25.md#六三個留痕入口)

# 九：bTemperatureReady 的初值

[讀取此節](porting-gaps/25.md#九btemperatureready-的初值)

# 十：iTo3Unload 應為「只有宣告，零個賦值」

[讀取此節](porting-gaps/25.md#十ito3unload-應為只有宣告零個賦值)

# 十一：cSortCT.cpp 應該不存在；兩個巨集閘應該在

[讀取此節](porting-gaps/25.md#十一csortctcpp-應該不存在兩個巨集閘應該在)

# 十二：守衛在 MyForceDirectories 之後（順序本身就是要確認的東西）

[讀取此節](porting-gaps/25.md#十二守衛在-myforcedirectories-之後順序本身就是要確認的東西)

# 十五：slEventLog 建構點應為零（20260926 新增）

[讀取此節](porting-gaps/25.md#十五sleventlog-建構點應為零20260926-新增)

# 十六：開機鏈應該還沒有 SetRunStartMode（20260926 新增；解掉之後這行才會有命中）

[讀取此節](porting-gaps/25.md#十六開機鏈應該還沒有-setrunstartmode20260926-新增解掉之後這行才會有命中)

# 十七：INDEX_SUCKER_TYPE 的 GATE 與存檔即時寫點都應該還在（20260926 新增）

[讀取此節](porting-gaps/25.md#十七index_sucker_type-的-gate-與存檔即時寫點都應該還在20260926-新增)

# 十八：BinCount 的 SEAM S2 巨集範圍應該還在（20260926 新增）

[讀取此節](porting-gaps/25.md#十八bincount-的-seam-s2-巨集範圍應該還在20260926-新增)

# 十四（已結案）：C 路讀檔器應該接在開機／換配方鏈（20260926 新增）

[讀取此節](porting-gaps/25.md#十四已結案c-路讀檔器應該接在開機換配方鏈20260926-新增)

# 二十一：網頁存檔的運轉狀態閘（20260926 新增；加閘之前應為 0）

[讀取此節](porting-gaps/25.md#二十一網頁存檔的運轉狀態閘20260926-新增加閘之前應為-0)

# 十五（20260927 更新）：slEventLog 現在應有 2 個建構點（LogObjects.cpp:67／:73，St02 80bcd1fb）——上面「應為零」已過期

[讀取此節](porting-gaps/25.md#十五20260927-更新sleventlog-現在應有-2-個建構點logobjectscpp6773st02-80bcd1fb上面應為零已過期)

# 十八（20260927 補）：Exit 鈕的 BinCount 寫點（真本體，SEAM S2 範圍外）應該在

[讀取此節](porting-gaps/25.md#十八20260927-補exit-鈕的-bincount-寫點真本體seam-s2-範圍外應該在)

# 二十二：St01 這一輪的 [W906] 偏離應該都還在（Steven 推翻、或前提改變之後才會變）

[讀取此節](porting-gaps/25.md#二十二st01-這一輪的-w906-偏離應該都還在steven-推翻或前提改變之後才會變)
