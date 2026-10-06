# JSON 橋接層擋在哪裡 —— 需要移植的部分

舊引用路徑保留；[讀取整理後文件](../../hpi-web-hmi/references/bridge/references/porting-gaps.md)。

## 這份文件在講什麼

[讀取此節](../../hpi-web-hmi/references/bridge/references/porting-gaps/01.md#這份文件在講什麼)

## 一、⛔ 最高優先：`main.cpp` 整支不存在（型態 A）

[讀取此節](../../hpi-web-hmi/references/bridge/references/porting-gaps/02.md#一-最高優先maincpp-整支不存在型態-a)

## 二、⛔ 溫控讀值：執行緒存在但沒有人啟動它（型態 B）

[讀取此節](../../hpi-web-hmi/references/bridge/references/porting-gaps/03.md#二-溫控讀值執行緒存在但沒有人啟動它型態-b)

## 三、溫控超溫告警升級被閘擋住（型態 C，SAFETY）

[讀取此節](../../hpi-web-hmi/references/bridge/references/porting-gaps/04.md#三溫控超溫告警升級被閘擋住型態-csafety)

## 四、`TfContact::ReadFile()` 沒有本體（型態 A）

[讀取此節](../../hpi-web-hmi/references/bridge/references/porting-gaps/05.md#四tfcontactreadfile-沒有本體型態-a)

## 五、權限名稱層被閘擋住（型態 C，gate SEC1）

[讀取此節](../../hpi-web-hmi/references/bridge/references/porting-gaps/06.md#五權限名稱層被閘擋住型態-cgate-sec1)

## 六、留痕（RecordProcess 族）三個入口都不是真的

[讀取此節](../../hpi-web-hmi/references/bridge/references/porting-gaps/07.md#六留痕recordprocess-族三個入口都不是真的)

## 七、`fTemperFrom` 單例不存在（型態 A，影響 S7 v2）

[讀取此節](../../hpi-web-hmi/references/bridge/references/porting-gaps/08.md#七ftemperfrom-單例不存在型態-a影響-s7-v2)

## 八、S7 專屬：`sv` 上線時有 15 個通道 golden 從未賦值

[讀取此節](../../hpi-web-hmi/references/bridge/references/porting-gaps/09.md#八s7-專屬sv-上線時有-15-個通道-golden-從未賦值)

## 九、S7 專屬：`bTemperatureReady[]` 的初值本身就是壞的

[讀取此節](../../hpi-web-hmi/references/bridge/references/porting-gaps/10.md#九s7-專屬btemperatureready-的初值本身就是壞的)

## 十、~~`iTo3Unload[]` 宣告了但沒人填~~ ⛔ **整節作廢（20260923 同日撤回）—— 它早就填好了**

[讀取此節](../../hpi-web-hmi/references/bridge/references/porting-gaps/11.md#十ito3unload-宣告了但沒人填--整節作廢20260923-同日撤回-它早就填好了)

### ~~十、`iTo3Unload[]` 宣告了但沒人填 → QtyLog 六格全寫同一個 Bin（型態 A）~~

[讀取此節](../../hpi-web-hmi/references/bridge/references/porting-gaps/11.md#十ito3unload-宣告了但沒人填--qtylog-六格全寫同一個-bin型態-a)

## 十一、`fSortCT->ShowLoadingIC()`／`ShowSortIC()` 是 no-op facade（型態 A）

[讀取此節](../../hpi-web-hmi/references/bridge/references/porting-gaps/12.md#十一fsortct-showloadingicshowsortic-是-no-op-facade型態-a)

## 十二、`MyForceDirectories(QtyData\YYYYMM)` 跑在守衛前面 —— **這不是缺陷，是忠於 golden**

[讀取此節](../../hpi-web-hmi/references/bridge/references/porting-gaps/13.md#十二myforcedirectoriesqtydatayyyymm-跑在守衛前面--這不是缺陷是忠於-golden)

## 十三、`ATC.ini` 沒被讀 → 開機把配方的 `Chiller Temp` 改寫掉（型態 A，**會寫壞配方檔**）

[讀取此節](../../hpi-web-hmi/references/bridge/references/porting-gaps/14.md#十三atcini-沒被讀--開機把配方的-chiller-temp-改寫掉型態-a會寫壞配方檔)

## 十四、`TFTestIF::ReadTestIFFile()` 還在 GATE (F-5)（型態 C，**待辦**，使用者 20260924 指示列入）

[讀取此節](../../hpi-web-hmi/references/bridge/references/porting-gaps/15.md#十四tftestifreadtestiffile-還在-gate-f-5型態-c待辦使用者-20260924-指示列入)

## 十五、`slEventLog` 全樹是 NULL——golden `TfMain` 建構子那段沒翻（型態 A，20260926 新增）

[讀取此節](../../hpi-web-hmi/references/bridge/references/porting-gaps/16.md#十五sleventlog-全樹是-nullgolden-tfmain-建構子那段沒翻型態-a20260926-新增)

## 十六、開機少 golden `TfMain::FormShow` 的 `SetRunStartMode`——Tester Off-Line 機台開機顯示 Re-Test（型態 B，20260926 新增）

[讀取此節](../../hpi-web-hmi/references/bridge/references/porting-gaps/17.md#十六開機少-golden-tfmainformshow-的-setrunstartmodetester-off-line-機台開機顯示-re-test型態-b20260926-新增)

## 十七、`INDEX_SUCKER_TYPE` 只解了一半——開機讀了，但存檔仍即時寫記憶體（型態 C，20260926 新增）

[讀取此節](../../hpi-web-hmi/references/bridge/references/porting-gaps/18.md#十七index_sucker_type-只解了一半開機讀了但存檔仍即時寫記憶體型態-c20260926-新增)

## 十八、`BinCount.txt` 三個寫點在 SEAM S2 空替身裡——移植樹目前完全不寫這個檔（型態 C，20260926 新增，含 0925 稽核更正）

[讀取此節](../../hpi-web-hmi/references/bridge/references/porting-gaps/19.md#十八bincounttxt-三個寫點在-seam-s2-空替身裡移植樹目前完全不寫這個檔型態-c20260926-新增含-0925-稽核更正)

## 十九、筆電 `IO_CARD_TYPE=1` 時 IO 表沒綁——第 42 條 IO 解除這台機器測不到（環境限制，不是缺陷，20260926 新增）

[讀取此節](../../hpi-web-hmi/references/bridge/references/porting-gaps/20.md#十九筆電-io_card_type1-時-io-表沒綁第-42-條-io-解除這台機器測不到環境限制不是缺陷20260926-新增)

## 二十、HT9050 上 golden IO 物件打不到 1203 卡——`uiDevhand` 只在 `INSTALL_ETHETCAT` 成立時填（型態待確認，20260926 新增）

[讀取此節](../../hpi-web-hmi/references/bridge/references/porting-gaps/21.md#二十ht9050-上-golden-io-物件打不到-1203-卡uidevhand-只在-install_ethetcat-成立時填型態待確認20260926-新增)

## 二十一、網頁存檔不檢查運轉狀態——golden 設定鈕只在停機時按得到（偏離 golden 的缺閘，**SAFETY 相關，待 Jimmy 決定**，20260926 新增）

[讀取此節](../../hpi-web-hmi/references/bridge/references/porting-gaps/22.md#二十一網頁存檔不檢查運轉狀態golden-設定鈕只在停機時按得到偏離-golden-的缺閘safety-相關待-jimmy-決定20260926-新增)

## 二十二、St01 這一輪照建議先做的 `[W906]` 偏離與時序差異（20260926～27，**Steven 可推翻**，20260927 新增）

[讀取此節](../../hpi-web-hmi/references/bridge/references/porting-gaps/23.md#二十二st01-這一輪照建議先做的-w906-偏離與時序差異2026092627steven-可推翻20260927-新增)

## 排序建議

[讀取此節](../../hpi-web-hmi/references/bridge/references/porting-gaps/24.md#排序建議)

## 複驗指令

[讀取此節](../../hpi-web-hmi/references/bridge/references/porting-gaps/25.md#複驗指令)
