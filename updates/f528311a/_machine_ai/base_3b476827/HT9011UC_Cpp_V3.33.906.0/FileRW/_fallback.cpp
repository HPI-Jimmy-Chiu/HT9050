// ===========================================================================
//  FileRW/_fallback.cpp  --  AI(W906-FILERW-FALLBACK) 20260924
//
//  Steven 05f2695b（S12-C）讓 cprod.cpp（ht9045_globals）呼叫兩個 FileRW 函式：
//      FileRW_ProxyChecked()                    FileRW/_EditList.cpp:371
//      FileRW_IniConfig_ChangeCBListProperty()  FileRW/IniConfig.cpp:222
//  但 FileRW/*.cpp 只編進 wb_serve 這個執行檔（CMakeLists.txt add_executable(wb_serve …)），
//  於是其他連到 ht9045_globals 的程式（test_cUnitConvert／test_config_loaders／test_machine_iotable／
//  test_machine_cylinders／test_io_points／test_motor_points）從 05f2695b 起連結失敗。
//
//  這支放在 ht9045_globals（跟 cprod.cpp 同一個 archive），只定義這兩個函式：
//    * wb_serve：自己的 FileRW 物件是直接連結的物件，先解掉這兩個符號 ⇒ 本成員不會被抽出（archive 只在
//      還有未解符號時才抽成員），wb_serve 行為不變。
//    * 其他程式：抽出本成員，行為 = FileRW 沒啟動時的真函式行為 ——
//        FileRW_ProxyChecked：找不到替身元件時回 false（_EditList.cpp:373-374 同義）
//        FileRW_IniConfig_ChangeCBListProperty：g_booted 為假時什麼都不做（IniConfig.cpp:224 同義）
//      也就是 Steven 改之前那兩處還是 GATE GA1-B2 時的行為。
//  ⚠ 不要在這裡加其他 FileRW 函式：多定義一個 wb_serve 也需要的符號，本成員就可能被抽進 wb_serve、跟
//    真的那份撞名（或更糟，靜默蓋掉）。正解是把 FileRW 搬進 archive，那要跟 Steven 對齊。
// ===========================================================================
bool FileRW_ProxyChecked(const char* /*form*/, const char* /*name*/) { return false; }
void FileRW_IniConfig_ChangeCBListProperty() {}
