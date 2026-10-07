# V906 C++路由

讀[原cpp_build正文](../cpp/original-entry.md)的build.bat模式、V906_BUILD_DIR／V906_GENERATOR／V906_CMAKE_ARGS／V906_OBJ_ROOT、sim／ship、PE與完整完成標準。原量測、作者、日期與反向檢查要求全部保存；這次沒有跑gate或更換工具鏈。

本次釘main2db43115d的build.bat：serve由TARGETARG只選wb_serve，quick不跑ctest；gate與testonly另一路。原始碼存在／label分流不表示已在這台執行。詳[靜態來源](../runtime/current-source.md)。

源樹預設C++17／UTF-8；HT9045_CXX_STANDARD／W906_NO_SOFT_SIMULTE與cache／工具鏈旗標決定實際組態。ship參數不是執行期--dry；目前--dry已退場。不能把build.bat serve或F5名稱當成程式已經啟動。

Steven 1006 14:0x的WinLibs決定與「oracle切換前要由筆電排兩組態gate對照」保留在原4.4a／4.5；main入口仍可指舊oracle路徑，決定、檔案落地與本機安裝狀態分開。[同事代編](../../../ops-ht9045-proxy-build/SKILL.md)按原角色與當次需求處理，本次不接管代理建置或其他session。
