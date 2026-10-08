# 舊 manifest 的來源差異分類

查證main `37a048f1ce5a9c3128cf75d69d579b602afce918`。18項source-row是下表的分母：9項至少一個選定片段已變，9項選定片段相同。來源檔在上游有其他改動，不等於舊片段的結論全部失效，也不等於整檔其他改動已驗。
逐項舊manifest local byte SHA、source pin、目前source blob、片段數與符號分類保存在 [manifest](source-manifest.json) 的drift_review；原文已先回核舊pin，再判定目前文字是否仍完整存在。這是文字保存／來源差異分類，不是語意等價或runtime證明。

| 舊節點／source | 選定片段數 | 已變數 | 分類 |
| --- | ---: | ---: | --- |
| `state/source-manifest.json`／`csystem.cpp` | 0 | 0 | 選定文字相同；整檔其他改動仍待查 |
| `packets/driver/source-manifest.json`／`GpibDriver.h` | 4 | 1 | 改變的選定定義／class／table對應本節點新版原文 |
| `packets/driver/source-manifest.json`／`GpibDriver.cpp` | 7 | 1 | 改變的選定定義／class／table對應本節點新版原文 |
| `packets/driver/callers/source-manifest.json`／`test_testercomm_gpib.cpp` | 4 | 2 | 改變的選定定義／class／table對應本節點新版原文 |
| `packets/driver/callers/source-manifest.json`／`CMakeLists.txt` | 2 | 0 | 選定文字相同；整檔其他改動仍待查 |
| `packets/driver/implementations/source-manifest.json`／`GpibDriver.h` | 2 | 2 | 改變的選定定義／class／table對應本節點新版原文 |
| `packets/driver/implementations/source-manifest.json`／`GpibDriver.cpp` | 31 | 3 | 改變的選定定義／class／table對應本節點新版原文 |
| `packets/driver/lifecycle/source-manifest.json`／`GpibEngine.h` | 1 | 1 | 改變的選定定義／class／table對應本節點新版原文 |
| `packets/driver/lifecycle/source-manifest.json`／`GpibEngine.cpp` | 9 | 2 | 改變的選定定義／class／table對應本節點新版原文 |
| `packets/driver/callers/callback-lifetime/source-manifest.json`／`GpibEngine.cpp` | 2 | 0 | 選定文字相同；整檔其他改動仍待查 |
| `packets/driver/callers/init-selection/source-manifest.json`／`HandlerTesterSide.cpp` | 3 | 1 | 改變的選定定義／class／table對應本節點新版原文 |
| `packets/driver/callers/settings-hub/source-manifest.json`／`HandlerTesterSide.cpp` | 1 | 0 | 選定文字相同；整檔其他改動仍待查 |
| `packets/driver/callers/tick-consumer/source-manifest.json`／`HandlerBridgeCtl.cpp` | 2 | 1 | 改變的選定定義／class／table對應本節點新版原文 |
| `consumers/command/source-manifest.json`／`Command.cpp` | 0 | 0 | 選定文字相同；整檔其他改動仍待查 |
| `consumers/webbridge/source-manifest.json`／`WebBridgeTags.cpp` | 0 | 0 | 選定文字相同；整檔其他改動仍待查 |
| `consumers/command/transport/source-manifest.json`／`HandlerBridgeCtl.cpp` | 0 | 0 | 選定文字相同；整檔其他改動仍待查 |
| `consumers/command/transport/source-manifest.json`／`HandlerTesterSide.cpp` | 0 | 0 | 選定文字相同；整檔其他改動仍待查 |
| `consumers/command/transport/mailbox/engine/payload/source-manifest.json`／`GpibEngine.cpp` | 0 | 0 | 選定文字相同；整檔其他改動仍待查 |

沒有將舊pin、舊manifest或已交付報告更新成新版本；完整caller／WebBridge新增欄位仍見 [剩餘](versions.md)。回 [版本入口](index.md)。
