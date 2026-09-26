# 產生檔 -- tools/gen_editlist.py。C 形狀（HTEditList／具名替身）的原始檔（相對於 HT9011UC_Cpp_V3.33.906.0），
# CMakeLists.txt 的 wb_serve include 它。新增結構：在 STRUCTS 加一筆、寫 FileRW/<struct>.cpp、重跑本產生器。
set(W906_EDITLIST_SRC
    FileRW/_EditList.cpp
    FileRW/_EditPage.cpp
    FileRW/_KitSuck.cpp
    FileRW/IniConfig.cpp
    FileRW/Ld_UldDelayTime.cpp
    FileRW/UserDefForm_File.cpp
    FileRW/ArmSpeed_File.cpp
    FileRW/BinSelect.cpp
    FileRW/DeviceForm_File.cpp
    FileRW/HSys.cpp
    FileRW/Offset_File.cpp
    FileRW/TTLCfg.cpp
    FileRW/Temperature.cpp
    FileRW/TestIF_File_SetUp.cpp
    FileRW/TestIF_File_YieldMonitoring.cpp
    FileRW/TrayForm.cpp
    FileRW/Teach.cpp
)
