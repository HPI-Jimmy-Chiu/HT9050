# HT9045 的國際牌 RS232 實作（golden 0618 → V912 → V906 移植）

舊引用路徑保留；[讀取整理後文件](../../../hpi-motor-control/references/control/references/panasonic-rs232/ht9045-implementation.md)。

## 1. 樹、檔案、怎麼讀

[讀取此節](../../../hpi-motor-control/references/control/references/panasonic-rs232/ht9045-implementation.md#1-樹檔案怎麼讀)

## 2. 設定與開埠

[讀取此節](../../../hpi-motor-control/references/control/references/panasonic-rs232/ht9045-implementation.md#2-設定與開埠)

## 3. 節拍

[讀取此節](../../../hpi-motor-control/references/control/references/panasonic-rs232/ht9045-implementation.md#3-節拍)

## 4. 函式地圖（rs232.cpp）

[讀取此節](../../../hpi-motor-control/references/control/references/panasonic-rs232/ht9045-implementation.md#4-函式地圖rs232cpp)

## 5. 讀扭力狀態機 `ReadTorque_Panasonic`（:820-1000）

[讀取此節](../../../hpi-motor-control/references/control/references/panasonic-rs232/ht9045-implementation.md#5-讀扭力狀態機-readtorque_panasonic820-1000)

## 6. 收資料 `Comm1ReceiveData`（:1750-1833）

[讀取此節](../../../hpi-motor-control/references/control/references/panasonic-rs232/ht9045-implementation.md#6-收資料-comm1receivedata1750-1833)

## 7. 參數讀寫狀態機 `ReadWriterParameter_Panasonic`（:1523-1681）

[讀取此節](../../../hpi-motor-control/references/control/references/panasonic-rs232/ht9045-implementation.md#7-參數讀寫狀態機-readwriterparameter_panasonic1523-1681)

## 8. 寫上限＋讀回 `iWriteAndCheckMotorTorque`（:1847-2009）

[讀取此節](../../../hpi-motor-control/references/control/references/panasonic-rs232/ht9045-implementation.md#8-寫上限讀回-iwriteandcheckmotortorque1847-2009)

## 9. 呼叫者

[讀取此節](../../../hpi-motor-control/references/control/references/panasonic-rs232/ht9045-implementation.md#9-呼叫者)

### 9.1 量產 Index 下壓 `DoTestHeadMotor`（atester.cpp:5562）

[讀取此節](../../../hpi-motor-control/references/control/references/panasonic-rs232/ht9045-implementation.md#91-量產-index-下壓-dotestheadmotoratestercpp5562)

### 9.2 其他

[讀取此節](../../../hpi-motor-control/references/control/references/panasonic-rs232/ht9045-implementation.md#92-其他)

## 10. UI 欄位

[讀取此節](../../../hpi-motor-control/references/control/references/panasonic-rs232/ht9045-implementation.md#10-ui-欄位)

## 11. V912 差異

[讀取此節](../../../hpi-motor-control/references/control/references/panasonic-rs232/ht9045-implementation.md#11-v912-差異)

## 12. V906 移植現況

[讀取此節](../../../hpi-motor-control/references/control/references/panasonic-rs232/ht9045-implementation.md#12-v906-移植現況)

### 12.1 review6（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\rs232.cpp`）

[讀取此節](../../../hpi-motor-control/references/control/references/panasonic-rs232/ht9045-implementation.md#121-review6dht9045ht9011uc_cpp_v3339060rs232cpp)

### 12.2 main 的 E-038 Phase A（`e6cec741`，已在 origin/main，不在 review6）

[讀取此節](../../../hpi-motor-control/references/control/references/panasonic-rs232/ht9045-implementation.md#122-main-的-e-038-phase-ae6cec741已在-originmain不在-review6)

### 12.3 E-044（`9432ff6e`，分支 `v906/st01e-e044`；1004 17:44 時不在 origin/main）

[讀取此節](../../../hpi-motor-control/references/control/references/panasonic-rs232/ht9045-implementation.md#123-e-0449432ff6e分支-v906st01e-e0441004-1744-時不在-originmain)

## 13. 已知陷阱

[讀取此節](../../../hpi-motor-control/references/control/references/panasonic-rs232/ht9045-implementation.md#13-已知陷阱)

## 14. 還沒答案的

[讀取此節](../../../hpi-motor-control/references/control/references/panasonic-rs232/ht9045-implementation.md#14-還沒答案的)
