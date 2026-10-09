# 共用 reader：PlateInfo caller 與 JSON ownership

來源是 V906 移植樹 `Public/HTEditList.cpp`、其 header 與 repo 內 `Public/cJSON.c/h`，
以 `uPlateInfo::LoadFile`、`file_buf`、`loaded_root`、`HPSuckGroupList` 定位。
本單元新增23完整CPP／11完整C定義與12region共46原文；2既有common reader僅context。
完整原文、source blob／byte hash、條件與計數見 [manifest](source-manifest.json)。

| 查什麼 | 入口 |
| --- | --- |
| 先清群組、NULL／parse失敗及收件條件 | [LoadFile路徑](load.md) |
| buffer、JSON root、group／team釋放 | [ownership](ownership.md) |
| JSON根物件、預設值與陣列容量 | [schema與欄位](schema.md) |
| cJSON Parse／accessors／Delete實際本體 | [parser契約](parser.md) |
| test_common既有斷言與覆蓋缺口 | [靜態測試讀法](tests.md) |
| CMake、cinitial gate與機型／版本 | [可達性與適用範圍](reachability.md) |

同題仍在 [共用reader與平台契約](../index.md) 下，
另沿 [Handler目前C++入口](../../../../../../../../../../index.md) 選HT9050／其他Handler與版本。
只完成選定本體／相鄰callee文件；完整startup、ABI、實際IO／部署CRT與機台量測待續。
前MR !358與前reader單元不重算、不重寄。
