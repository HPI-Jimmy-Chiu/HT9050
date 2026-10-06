# OutArm共同項與差異

基準main `372b91908`，20261006。可共用流程查證與資料語意，幾何／盤型／Shuttle／氣缸依機型與配置分派。

| 項目 | 共同判讀 | 分流／界線 |
|---|---|---|
| 流程鏈 | DoOutArm→dispatcher→取料→附加功能→搜盤／放料→後處理 | USE_PICKER_COUNT先選單Picker，其他由iInArmType；各Task值看實際callee |
| 吸嘴與資料 | Item、Suck／Destroy完成、真空、Shuttle資料與Bin分開 | 多吸嘴映射與HT9050四嘴合吸一顆不能互換 |
| 基準／Pitch | 教導、基準軸、Pitch與個別Offset是不同層 | 多吸嘴有E／D／C基準與獨立Out Y-Pitch；HT9050教導A／間距0 |
| 取料互鎖 | Shuttle到位與手臂XYZ、吸取完成的條件分開 | HT9050獨立Out Shuttle與M18出料Y；通用單Picker仍有MInShuttle1／2引用，不能推有兩台真實飛梭 |
| 出料盤 | 搜盤、滿盤、換盤、計數各有Task與資料 | HT9050 Auto1～3、沒有Fix；其他配置的Fix／Magazine／BulkBox按原文 |
| 附加功能 | Rotator、AOI、Fix AI CCD按選用路線 | dummy run未拍照不代表相機／映射正確；歷史AOI案例不是HT9050已驗證規則 |

[目前main](runtime/current-main.md)／[機型](machines/index.md)／[原完整流程](flow/original-entry.md)。
