# PlateInfo JSON的根形狀與欄位預設

`uPlateInfo::SaveFile` 建立object，以 `HPSuckGroupList_%d` 為key掛group object。
LoadFile沒有檢查根必須是array，也沒有依key文字排序；repo的GetArraySize／GetArrayItem
直接走child sibling，因此能遍歷此object的child。根scalar沒有child會得到0；
「可parse」不等於已驗證PlateInfo schema。

`uHPSuckGroup::LoadJSONFile` 僅在 `HPSuckTeamList` 是array時逐項new team。
`uHPSuckTeam::uHPSuckTeam` 先Clear，LoadJSONFile再讀欄位：

| 欄位 | 來源與判斷 | 無效／缺值的局部行為 |
| --- | --- | --- |
| P／Col／Row／Sht／Kit | uBasicPickPlace::GetIntegerFromJSON，`_obj->type==cJSON_Number` | 預設回0；不是通用數字轉字串 |
| PlateRow／PlateCol／Site | array大小必須等於MAX_ARM_Row*MAX_ARM_Col，逐number取valueint | 錯大小直接return，非number元素不改該格，初值-1 |
| Suck | 相同容量與number guard，`valueint==1`才true | 初值false；JSON boolean不會通過IsNumber；其他number值為false |

陣列用 `i/MAX_ARM_Col` 與 `i%MAX_ARM_Col` 映射；容量引用該版本的常數，
本單元沒有把另一機型的site／吸嘴數或runtime設定當成這兩個compile-time維度。
也沒有做ID範圍、位置合法性、機構互鎖、重複欄位或機台資料同步驗證。

GetIntegerFromJSON採type完整相等；cJSON_IsNumber／IsArray採 `type & 0xFF` 判斷。
這是兩種本體條件，不應宣稱全部欄位擁有同一reference旗標處理。
GetObjectItem傳case_sensitive=false走不分大小寫分支；compare helper、locale／UTF-8字元規則未完整展開。
