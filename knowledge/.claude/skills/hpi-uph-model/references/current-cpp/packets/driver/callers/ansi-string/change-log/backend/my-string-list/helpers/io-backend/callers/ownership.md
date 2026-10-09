# buffer、JSON tree 與容器指標的生命週期

| 資源 | 本單元原文中的取得／保存 | 清理觀察 |
| --- | --- | --- |
| `file_buf` | common reader用malloc配置，回傳char* | LoadFile的成功與parse失敗return都沒有free |
| `loaded_root` | cJSON_Parse成功取得tree | LoadFile成功分支沒有cJSON_Delete |
| group指標 | new uHPSuckGroup，交AddHPSuckGroup | Add拒收時沒有delete；LoadFile直接Clear也沒有delete舊group |
| team指標 | LoadJSONFile逐項new uHPSuckTeam，再Add | ClearTeamList僅清容器；group destructor也沒有逐team delete |
| TList物件 | plate／group constructor用new TList | destructor清容器、delete容器，沒有因此delete容器內的void*指向物件 |

`Public/HTEditList.h` 的TList是 `std::vector<void*>` 包裝；
`Clear(){v.clear();Count=0;}` 清指標序列，沒有走delete pointee。
這裡的header guard是 `HTEditListH`／`HTEDITLIST_TLIST_SHIM`，
不能把別的同名TList或substrate窄版uPlateInfo的清理契約套過來。

`uPlateInfo::ClearGroupList` 逐group呼叫ClearTeamList後清group容器；
`uPlateInfo::~uPlateInfo` 和 `uHPSuckGroup::~uHPSuckGroup` 都只Clear並delete TList。
`uHPSuckTeam::~uHPSuckTeam` 呼叫ClearHPSuckTeam清欄位，沒有替group或plate持有的指標補清理。

repo的cJSON header明訂Parse結果由caller用cJSON_Delete釋放。
`cJSON_Delete` 遞迴釋放非reference child、依旗標釋放strings並用global_hooks.deallocate釋放node，
這不會代替caller釋放原reader的malloc buffer。ParseWithLengthOpts失敗會Delete當次部分tree，
但沒有取得file_buf的ownership。

這些是局部取得／未釋放路徑的靜態證據；未量測實際程序memory增量、allocator、
外部alias的保留或執行到這個LoadFile的次數，也未改動任何清理行為。
保存SaveFile只為確認根形狀：它有cJSON_Delete(root)，
`cJSON_Print(root)` 暫存回傳值直接傳writer；完整Print／writer釋放圖不在本單元完成範圍。
