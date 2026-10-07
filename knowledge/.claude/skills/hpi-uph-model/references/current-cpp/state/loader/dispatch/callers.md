# 一般機型與 V912 AutoTeach caller

版本及不可變檔案見 [來源](index.md)；定位 `DoLoad`／`LoadTask`／`case 1000`、`AutoTeachInArmZLoaderTray`／`iAutoTeachInArmZTask`／`case 11`。

| 路徑 | 呼叫前的本地條件 | 呼叫回 true 後所選區段 |
| --- | --- | --- |
| V906 `DoLoad` 一般分支 | HT9050 在入口已轉 `DoLoad_9050` 並 return；其後才有 `bOneTimeHotPlateCheckAll`、`bNewResetFunction`／`bResetLoadTray` 提早返回 | `case 1000` 的清針旗標及 `bAskStopPort[ePortLoader]` 可 break；`DoSupplyNewICTray()` true 後才更新 TrayID、`iLoadTrayCount` 等並設 `Task=1` |
| V912 `DoLoad` | 入口先查 `IsE84LoaderTransferLock(0)`，後有 HotPlate／reset 提早返回 | 所選 `case 1000` 的清針／stop-port／供盤 true 區段與 V906 文字相同；這不是整個 DoLoad 等價證明 |
| V912 `AutoTeachInArmZLoaderTray` | `Reset` 可先清 `iAutoTeachInArmZTask` 與陣列後 return false | `case 11` 呼叫 `DoSupplyNewICTray()`，true 時 `iTask=20`；這是另一條 AutoTeach 路徑，不是已核實的生產排程 |

`iLoadTrayCount`、`ASE_InTrayNum`、`iOneTrayPickCount` 的更新不等同 `iUPH_LoaderCount`。所選呼叫區段本身未直接寫 `bRecordUPH`；旗標寫者仍是 [被呼叫函式的 case 1300](../trigger.md)，完整 helper／Task／後续取樣使用端尚未閉合。

只已讀三個入口與三個 case，三個完整 caller body 保存 hash；未確認所有呼叫來源、巨集展開、編譯組態或實際呼叫順序。
