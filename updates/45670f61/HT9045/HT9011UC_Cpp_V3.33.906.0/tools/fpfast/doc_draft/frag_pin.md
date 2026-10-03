新測試，獨立執行檔（不連 god stack、不寫任何檔；argv[1]＝原始碼根目錄），三部分：

* **[R] 執行期**：10 個 BCB6 答案（§9.3 用 bcc32 實測）——讀進來的 double 跟 40.2／5.6／26.67 比、`v<0.8 || v>5.2` 的 5.2 與 0.8 邊界、
  `d = 28/10.0` 存進 volatile double 再跟 2.8 比。每個值都經過 volatile 或 noinline，Release -O3 也不會被常數摺疊。
* **[S] 原始碼**：`CMakeLists.txt` 必須剛好一行「活的」（不在註解裡）`add_compile_options(…-fexcess-precision=fast…)`，C 與 CXX 都有、都限 GNU、
  排在第一個 add_library／add_executable／add_subdirectory 之前；兩份 CMakeLists 都不可以有活的 `-fexcess-precision=standard`。
* **[X] 只在 x87（FLT_EVAL_METHOD 2）**：`(1e16+1)-1e16 == 1`、`(1e308*10)/10 == 1e308`、`56 == 5.6*10` 為假——旗標不可以動到的 80-bit 中間值
  （BCB6 實測同答案）。有人改成 SSE 數學、或全域加 `-ffloat-store`（§9.3 矩陣）會紅。

**為什麼一定要有 [S]**：g++ 6.3.0 的 C++ 只有 fast，所以在 oracle 上把那一行拿掉，[R] 照樣全綠（下表 C1）——只有 [S] 看得到。

對照（scratchpad `fpfast/ctl/run_ctl.sh`；編譯旗標＝建置裡這支測試的旗標，只差那一個）：

| | 編譯器／旗標 | 原始碼根 | 結果 |
|---|---|---|---|
| C1 | g++ 6.3.0，**不加** | 真的樹 | 21 PASS／0 FAIL，離開碼 0 ——執行期那半在 oracle 上看不到旗標被拿掉（預期） |
| C2 | 同 C1 | 根目錄放 HEAD 的 `CMakeLists.txt`（沒有那一行） | 15 PASS／**1 FAIL**（S1），離開碼 1 |
| C3 | WinLibs 16.2 `-std=c++14`，**不加** | 真的樹 | 12 PASS／**9 FAIL**（R1–R8、R10），離開碼 1；R9（0.8 邊界）兩種模式答案本來就一樣 |
| C4 | WinLibs 16.2，加 | 真的樹 | 21 PASS／0 FAIL |
| C5 | 同 C4 | HEAD 的 `CMakeLists.txt` | 15 PASS／**1 FAIL**（S1） |
