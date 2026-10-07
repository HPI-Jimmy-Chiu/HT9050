# 客戶與一般 VTEST 旗標的 CSV 格式

來源 pin `06fb64e540dff625f1f3af002179d56a2e49389e`：
[V906 ainarm9045.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/06fb64e540dff625f1f3af002179d56a2e49389e/HT9011UC_Cpp_V3.33.906.0/ainarm9045.cpp)、
[V912 ainarm9045.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/06fb64e540dff625f1f3af002179d56a2e49389e/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/ainarm9045.cpp)。以函式名及 filename／t／iTrayCount／iOpenSiteNum 定位。

| Writer | 指定 body 的格式／路徑 | 版本差異 |
| --- | --- | --- |
| LotRecordUPH | as9045UPH 加 edtSysLotID->Text_UPH.csv；Start Time、End Time、Pause Time、UPH 四欄 | V906／V912 指定 body SHA 相同；兩版 t 都含換行 |
| RecordLotUPH_For_FOREHOPE_NINGBO | V912 依 Now() 日期建立年／月／日目錄及日期_UPH.csv，六欄增加 Tray Count、Site Count | V906 的同名指定函式是空 body；不能由 CalculateUPH 有呼叫推定有 CSV 輸出 |
| RecordLotUPH_For_VTEST | V912 使用相同每日路徑格式，七欄增加 Elaps. Time、Total Units、Site | 此 pin 的 V906 ainarm9045.cpp 沒有這個定義；不由單檔缺少推定全功能不存在 |

非空三種 writer 都先用 FileExists(filename) 決定是否寫標題，再呼叫 [WriteDataToFile](common-io.md) 寫資料。不存在時標題與資料是兩次獨立開檔／寫入呼叫；writer 沒有合併成一次交易，也不回傳或檢查保存結果。

FOREHOPE／VTEST 的 sprintf 格式本身不含換行，由 common helper 追加。KYEC 的字串已有換行。所有欄值按引數格式輸出，沒有在指定 writer 內再乘 site、換算盤數、扣 pause 或重算 UPH。

沿用 [CalculateUPH 的 caller 證據](../outputs.md)：P11 先分 KYEC_LEE、再 FOREHOPE_NINGBO，V912 其後才分 bVTESTFunction；VTEST 是一般設定，不能另造 CUSTOMER_CODE。
FOREHOPE 的 iTrayCount 名稱來自 caller 暫存 iUPH_LoaderCount，Site Count 來自 GetSiteCount(false)；VTEST Total Units 接相同暫存 count，Site 接 iShtRow*iShtCol。所有計數 writer、site helper、容量與實際單位仍待查。

writer 的使用日期是呼叫當下 Now()，不由 UPH_StartTime／UPH_EndTime 推算。日期檔名不含 Lot ID 或 CUSTOMER_CODE；若 as9045UPH 與日期相同，FOREHOPE／VTEST 的字串規則會指向同一路徑。這是公式比較，未觀察現場混用、資料污染或檔案內容。
