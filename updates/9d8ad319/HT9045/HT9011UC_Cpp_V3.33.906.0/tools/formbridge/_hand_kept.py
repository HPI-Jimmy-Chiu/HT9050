# -*- coding: utf-8 -*-
# AI(W906-E031) 20261003 [W906] (St01)：gen_formbridge.py 的「手寫 bridge 留存表」（keep-list）。檔名以 _ 開頭 ⇒ load_forms 不當表單設定讀。
#
# 為什麼要有這張表：
#   FileRW/_registry.cpp 與 FileRW/_formbridge_sources.cmake 是 gen_formbridge.py 全量（不帶 --only）的產生檔，但 TfTeach／Tfiosetview
#   兩個 bridge 是手寫的（FileRW/TeachFormShow_File.cpp、FileRW/IoSetViewFormShow_File.cpp：golden FormShow 的畫面那一半，不是 golden 機械轉的，
#   tools/formbridge/ 沒有它們的 <Class>.py）。20261002 是直接手改那兩個產生檔加的列（行尾寫著「keep when regenerating」），
#   所以下一次全量重產會把兩列刪掉：wb_serve 少了 GET /api/form/HW.teach.html 與 IO 頁的 golden FormShow，而且 build 不會紅——
#   兩邊一起少（登錄表沒參照、原始檔也不編），只是靜默少兩個 bridge；ctest TeachFormShowBridge／IoSetViewFormShowBridge 也照樣綠，
#   因為它們直接編那支 cpp、各自帶一個只有自己的 kBridges[]（tests/test_teach_formshow_bridge.cpp:21、test_iosetview_formshow_bridge.cpp:26）。
#   E-031 第二階段（ST01-E 1003）：改成由產生器照這張表接在產生的列後面，全量重產＝已 commit 的兩檔一字不差；
#   ctest E031_FormBridgeFullRun（tools/formbridge_fullrun_check.py）把全量跑進 TEMP 目錄、跟已 commit 的兩檔比，少一列就紅。
#
# 新增手寫 bridge：在 FileRW/ 寫好 <cpp>（裡面要有 `extern const BridgeDesc kBridge_<class> = {`），在這裡加一列，再全量重產。
# 欄位（行尾註解照原文輸出，前面由產生器補三個空白；空字串＝那一行沒有註解）：
#   class  kBridge_<class> 的 <class>（＝golden 表單類別）
#   cpp    FileRW/ 底下的檔名
#   decl   _registry.cpp 宣告列 `extern const BridgeDesc kBridge_<class>;` 的行尾註解
#   row    _registry.cpp kBridges[] 那一列 `&kBridge_<class>,` 的行尾註解
#   cmake  _formbridge_sources.cmake 那一列 `FileRW/<cpp>` 的行尾註解
# COUNT_NOTE：有手寫列時 `kBridgeCount = N;` 的行尾註解（N＝產生的＋手寫的）。
# 產生器會檢查（不符就中止，不寫任何檔）：<cpp> 在移植樹的 FileRW/ 存在且定義 kBridge_<class>；<class> 不可跟 tools/formbridge/<Class>.py 重複
# （重複定義）；<cpp> 不可跟產生的 FileRW/<struct>.cpp 同名（不然產生器會蓋掉手寫檔）。

HAND_KEPT = [
    {'class': 'TfTeach',
     'cpp': 'TeachFormShow_File.cpp',
     'decl': '// AI(W906-TEACH-FORMSHOW) 20261002: hand-written FileRW/TeachFormShow_File.cpp -- keep when regenerating',
     'row': '',
     'cmake': '# AI(W906-TEACH-FORMSHOW) 20261002: hand-written (golden TfTeach::FormShow screen half); keep this line when regenerating'},
    {'class': 'Tfiosetview',
     'cpp': 'IoSetViewFormShow_File.cpp',
     'decl': '// AI(W906-IOSV-FORMSHOW) 20261002: hand-written FileRW/IoSetViewFormShow_File.cpp -- keep when regenerating',
     'row': '// AI(W906-IOSV-FORMSHOW) 20261002',
     'cmake': '# AI(W906-IOSV-FORMSHOW) 20261002: hand-written (golden Tfiosetview::FormShow screen half); keep this line when regenerating'},
]

COUNT_NOTE = '// AI(W906-IOSV-FORMSHOW) 20261002: + Tfiosetview'
