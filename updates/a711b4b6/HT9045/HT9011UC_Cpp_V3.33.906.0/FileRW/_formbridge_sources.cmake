# 產生檔 -- tools/gen_formbridge.py。golden 表單 bridge 的原始檔（相對於 HT9011UC_Cpp_V3.33.906.0），
# tests/CMakeLists.txt 與 wb_serve（CMakeLists.txt）都 include 它（審查第 8 輪 M-2：正面清單，不 GLOB）。
set(W906_FORMBRIDGE_SRC
    FileRW/_registry.cpp
    FileRW/HotPlateForm_File.cpp
)
