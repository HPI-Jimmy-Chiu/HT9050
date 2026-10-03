
---

## §9 整棵樹加 `-fexcess-precision=fast`（20261003，RULINGS_20261003 第 7 條＝NIGHT_REPORT §0 #77 A，`AI(W906-FPFAST)`）

**一句話**：機台建置用的 WinLibs g++ 16.2 i686（-O0）加了這個旗標之後，§9.3 的 17 個探針情境全部回到跟 BCB6 一樣；
筆電的 oracle（MinGW.org g++ 6.3.0）**每一個編譯單元的機器碼都沒有變**（模擬、出貨兩個組態全部重編比對，§9.2）。

### §9.1 改在哪裡、為什麼是那一行

* **`CMakeLists.txt:7`**（原本的空行，其後行號全部不動）：
  `add_compile_options("$<$<AND:$<COMPILE_LANGUAGE:C>,$<C_COMPILER_ID:GNU>>:-fexcess-precision=fast>" "$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:GNU>>:-fexcess-precision=fast>")`
  只給 GNU 的 C 與 C++；MSVC（第二 oracle）與 windres（.rc）都看不到。目錄層級、排在第一個 target 之前 ⇒
  `third_party/sqlite3`、`tests/` 全部繼承。compile_commands.json 實測：1,594 條裡 1,593 條帶一次，只有 windres 那 1 條沒帶。
* **根因就在上一行**：`CMAKE_CXX_EXTENSIONS OFF`（:6）讓 WinLibs 線用嚴格的 `-std=c++14`；GCC ≥ 13 在嚴格模式的預設是
  `-fexcess-precision=standard`（FLT_EVAL_METHOD 2：十進位字面值以 long double 求值），`gnu++14` 的話預設本來就是 fast。
  g++ 6.3.0 的 C++ **只有** fast（給 `standard` 會 `sorry, unimplemented: -fexcess-precision=standard for C++`），所以 oracle 線一直是 fast。
* **三個 .bat 都不用改**：`build.bat`、`build_nonoracle.bat`（:do_configure）、`build_x64.bat`（:do_configure）只傳編譯器路徑、
  `-DHT9045_CXX_STANDARD=14`、`-DCMAKE_EXE_LINKER_FLAGS=-static`，沒有自己的 C／CXX FLAGS ⇒ 全部吃 `CMakeLists.txt:7`
  （`git grep CMAKE_CXX_FLAGS`：除了 `.vscode/tasks.json:170` 的註解，沒有任何腳本傳）。機台已 configure 好的
  `build_integ_ship_x86`／`build_integ_dbg_x86` 下一次 `cmake --build` 時，因為 CMakeLists.txt 變了會自動重新 configure 而吃到。
  x86_64（`build_x64.bat`，SSE）本來就沒有 excess precision，這行在那裡沒有作用。
* **EastSun 的 -O2 線**（`build_integ_ship_x86_o2`，`.vscode/tasks.json:169-172`）把 `-ffloat-store -fexcess-precision=standard`
  放在 CMAKE_CXX_FLAGS／CMAKE_C_FLAGS。用 WinLibs 照同一組旗標 configure 實測命令列是
  `… -ffloat-store -fexcess-precision=standard -fno-finite-loops -std=c++14 -fexcess-precision=fast …` ⇒ **後者勝，那條線也變成 fast**。
  後果不是全好，見 §9.3 的矩陣與 §9.5 第一點。
* `tools/gateverdict.sh:130` 的 R_NOTE 原寫「CMakeLists.txt 沒有 -ffloat-store / -fexcess-precision 保護」——現在有 -fexcess-precision 了，
  但 fast 不是那句話擔心的 -O3 保護（它正是允許 -O3 把算出來的值留在 80-bit 的模式），同一行改寫。

### §9.2 證明：oracle（g++ 6.3.0）的機器碼不變

**方法**（腳本在當天 scratchpad `fpfast/proof.py`）：用 build.bat 同一組參數 configure 兩個全新建置目錄
（`Obj/V906/build_fp`＝模擬、`build_fps`＝出貨 `-DW906_NO_SOFT_SIMULTE=ON`），取 compile_commands.json；
每一條**不重複的編譯命令**編兩次到 scratch——WITH＝原命令（＝建置真的會跑的那一條）、WITHOUT＝拿掉那一個旗標——
然後比：兩次的離開碼、stderr、物件檔位元組、`objdump -d -r --no-show-raw-insn` 逐段（段不同時再逐函式）、`objdump -s` 逐段（.text／.rdata／.data／.eh_frame…全部）。
編譯一律 IDLE 優先權、限定 6 顆核心。

* **WITHOUT 就是改之前的命令**：另把 HEAD 的 CMakeLists.txt（沒有那一行）configure 一次，1,594 條共同條目逐條
  「HEAD 的命令 ＝ WITH 拿掉旗標」，0 條不同。
* **比對器會抓到真的差異**（負向對照）：同一支腳本在 WITH 多加 `-ffloat-store`（§7 量過會改碼的那個旗標），
  `cContact.cpp` 與 `test_cContact.cpp` 兩條都判 DIFFERENT，列出 `ComputeMaxIndexForceLimit`、`ComputeMinForce`、`ComputeTotalAirForce`…（與 §7 一致）。

@@PROOF_TABLE@@

### §9.3 正向對照：BCB6 本人、oracle、WinLibs

**BCB6 回到這台了**（§8.1 寫的「不在」已過期）：`D:\ProgramFiles\Borland\CBuilder6\Bin\bcc32.exe`（1,398,272 B，Borland C++ 5.6.4）。
WinLibs 16.2.0 i686 在 `D:\HT9045\install\mingw32-16.2.0\mingw32\bin`（與機台同一個 zip：`D:\software\winlibs-i686-gcc16.2.0-msvcrt.zip`）。

**§4 的原探針**（`tools/fp_equality_probe.cpp`，未修改）：BCB6 `d == 40.2`／`via_param`／`volatile` 三項 true；
oracle 加不加旗標都 true；WinLibs **不加 false、加了 true**。位元組三方都是 `404419999999999A`。

**擴充探針**（同一份原始碼給三條線，pre-C++11；`1`＝true）。L＝讀進來的 double 與十進位字面值比（cContact／cinitial／fContact／adam6024／Adam6024Pressure／fHotPlate 的形狀）；
R＝`Adam6024Pressure_St02.cpp:822` 的嚴格範圍；S＝算出來、存進 double、再比（adam6024 KYEC `d = 28/10.0; d == 2.8`）；X＝BCB6 依賴 80-bit 中間值的純運算（不應該隨旗標變）。

@@POS_TABLE@@

@@POS_MATRIX@@

### §9.4 ctest 釘子 `FPFAST_Pin`（`tests/test_fpfast.cpp`）與它的對照

@@PIN_TEXT@@

### §9.5 範圍外、要知道的

@@SCOPE_TEXT@@
