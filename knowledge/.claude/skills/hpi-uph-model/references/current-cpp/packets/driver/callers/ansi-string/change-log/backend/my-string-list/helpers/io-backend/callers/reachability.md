# 版本、CMake註冊與startup gate

目前main的CMakeLists已把 `Public/HTEditList.cpp` 列入ht9045_sm，
`Public/cJSON.c` 列入ht9045_public並指定LANGUAGE C。
HTEditList header前段關於尚未註冊／同名class衝突的原始波次banner保留，
不能只引用它就斷言今天未編入。註冊也不能證明執行期會到這個reader或class ABI相容。

選定 `cinitial.cpp` 的兩處呼叫是：

```cpp
PickFromHPList->LoadFile(sHPPickRec);
PlaceToCleanList->LoadFile(sCleanPlaceRec);
```

它們同在 `#if 0 // GATE n2-20` region中，gate註解仍指LoadFile與sCleanPlaceRec阻塞。
本單元保留region及其條件，不把source中找到呼叫就算活startup路徑；
完整cinitial父function、所有header選路／其他LoadFile caller與實際link圖待續。
另保留aHotPlateSubstrate.h的窄版uPlateInfo宣告，與Public/HTEditList.h的完整類別分開。

## 同題機型與版本界線

本體沒有依Type_HT9050切分reader／JSON容量分支；該局部共用讀法同時供HT9050與其他Handler查證。
HT9050的機種身分、工單／snapshot及runtime資料同步仍依 [共用版本入口](../../../../../../../../../../index.md)，
這份PlateInfo文件不代表HT9050 startup已連上、不代表S8 UPH公式或機台IO驗證。

這次只讀V906移植main。V912公司維護、客戶V899、913最新golden、
歷史0618／BCB6註解是不同來源軸；未做913或BCB6 RTL新比較。
原註解的golden行號保存於manifest，活頁以function／變數定位，不重寫史料。
