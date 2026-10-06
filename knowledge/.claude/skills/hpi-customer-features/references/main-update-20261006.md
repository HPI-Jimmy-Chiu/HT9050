# 來源基準與後續main裁決

候選來源釘住main `341cea3d7c7136606223490c91de59019e6c0995`；本機之後pull整合main `0dfb844151ce8fb3a72ddd236e76de63e45edad9`。兩次main之間只更新文件，V912／V906來源檔與MachineType.h沒有改動，既有候選仍保持來源commit，不偽換成新掃描。

[RULINGS_20261006.md第22條](../../../../HT9011UC_Cpp_V3.33.906.0/docs/RULINGS_20261006.md)新增Frank本人答覆。這些是機台功能／裁決，不能改列成某客戶已生效功能；本批只讀文件，不接線、不改執行期：

| 項目 | 裁決／待答 | 實作界線 |
|---|---|---|
| Auto Get Height | 操作員手動選模式後才動作 | 不推論會在自動流程觸發 |
| 四吸嘴有料判斷 | 四個Suck狀態全On才標HAS_IC | 真空產生器對應仍待EastSun確認，不用客戶碼推硬體配置 |
| 冷溫門／飛梭保養 | 都需要接，分別交St02-E／Frank01 | LowTempIdleCheckSafeDoor、ShuttleSensorContinuousMove與維護caller；裁決不代表main已完成接線 |
| 停機門、測試逾時高度 | 門讀Real Time；Z1回Safe即可 | 按csystem／atester完整機型分派，不借舊機型雙Z行為 |
| 試壓／清潔放氣、額外功能、後相機路徑 | 放氣與額外功能尚待白話重問；後相機先確認caller是否到達 | ProcessIndexSuckDestroy、DoTestY case 210／DoTestHeadMotor case 600保持查證界線；相機改Line Scan前先暫緩 |

上表為本次來源導讀，原裁決正文保持原位。後續推送前仍需fetch／整合最新main，若來源碼／定義變更就重掃並核對；文件變更另查新裁決，不把WIP／他人派卡擴張成ST-GPT的程式修改範圍。

## 19時後整合

本機再次pull main至 `09d926ab86fea6cf366bd9d6c90167d51dedf317`。相比0dfb84415有HTDesigner、文件、night-loop角色分流及18:59機台快照更新；V912／V906的C++來源未改。候選仍釘住341cea3d7，新增[ART／客戶名稱人工核對](reviewed/index.md)另以09d926ab8及20個source blob記錄。

18:59的HT9050 IO快照新增SnAllEMG；SnSafeDoor2～8映射至PLC port 301／302、type 4。這是機台鏡像資料，不能推論所有門都用同一映射，也不由它推定ART客戶功能；Binasgn.Data、lastdata及general.ini狀態另有鏡像更新。本批只讀Git，不將快照套用runtime。

[MACHINE_PATCHES_20261006.md](../../../../HT9011UC_Cpp_V3.33.906.0/docs/MACHINE_PATCHES_20261006.md)第13／14節分別記cpp0243機台採用及cpp0244 W906IdxStop記錄節流。後者仍列pending batch 81，不把機台補丁文字當成main已含同一來源實作。HTDesigner與night-loop的新角色規則保持原owner，本批未執行其程式或替其他session操作。

19:39再pull整合main `4b6c7638e22cf1859158bcfd4876e070924ecb1b`：19:14快照只增備份與README，參數／工單未變；MACHINE_PATCHES第15節新增cpp0245 PLCDOOR待第81批來源整合紀錄。V912／V906來源仍未改，人工來源manifest維持09d926ab8，候選保持341cea3d7；不把pending紀錄改稱目前main實作。
