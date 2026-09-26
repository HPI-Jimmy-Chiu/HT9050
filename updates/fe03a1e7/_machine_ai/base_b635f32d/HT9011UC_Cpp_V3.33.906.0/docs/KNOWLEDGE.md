# HT9045 V906 BCB6 → C++（Visual C++）累積知識（KNOWLEDGE）

> 決策、接縫知識、gotcha。逐步累積，供備查。逐步紀錄見 `DEVLOG.md`。
> 目標：把 BCB6/VCL C++ 翻譯成標準/Visual C++（脫離 Borland 方言），硬體與 UI 用 interface 切割（先 stub 後接）。**非 C#**（C# 方向已停用，git `326df6c` 可回溯）。

## 架構（ports-and-adapters，以 C++ 實作）
- 「port」＝**C++ 抽象基底類別（純虛擬）**。能沿用既有 C++ 多型基底就沿用（如運動的 `HTMotor` 本來就是虛擬基底）。
- 編排/邏輯層只依賴抽象基底，不直接依賴具體驅動或 UI 框架。
- 每個硬體 port 與每個 UI view 都有兩個可換實作：**Sim/Stub（開發、無硬體）** 與 **Real/真 UI（之後接）**。
- 由一處 **composition root / factory** 依設定選 Sim↔Real、Stub↔真UI（取代 `cinitial.cpp:3454-3565` 的 dispatch ladder）。
- 即時硬體核心策略（沿用 §9 HAL）：先以 stub/SOFT_SIMULTE 讓離線版可編可跑，再逐接縫換真實驅動、邊測邊接。

## 檔案位置與命名（鏡射 BCB6，使用者指令）
- **翻譯出的 .cpp/.h 一律鏡射 BCB6 原始檔的相對路徑＋檔名**，方便 1:1 反查。例：原 `HT9011UC_Code_..._20260618\ContactForce.cpp` → 譯為 `HT9011UC_Cpp_V3.33.906.0\ContactForce.cpp`（同在 root、同名）；原 `Motor\mymotor.cpp` → `Motor\mymotor.cpp`。**不要另開 src/ 改路徑**。
- 無 BCB6 對應的「附加檔」可用新結構：`CMakeLists.txt`、`tests/`、`docs/`（這些原本是 .bpr/無對應）。`.bpr` → 以 CMake 取代（建置系統不同，屬合理偏離）。
- 例外（無法照做時提出）：含特殊字元/空白的原路徑（如 `ASE_K Socket\`）照樣鏡射但建置需處理空白；`.dfm` 表單不直接鏡射成 .cpp（UI 走介面承接，另議）。

## vclcompat 相容層（W0，載體最重，勿亂動）
- 位置 `vclcompat/`（附加，非鏡射）。翻譯檔以 `#include "vclcompat/vcl_compat.h"` 取代 `<vcl.h>`。
- `AnsiString`（backed by std::string）**1-based 語意**：`s[1]`=首字、`Pos`回 1-based 或 0、`SubString(start1,len)`、`Delete/Insert` 1-based；`AnsiString(int/double)` 格式化；`operator+` 串接；`UpperCase/LowerCase/Trim` 回新；`sprintf` 就地。**已通過語意測試 test_vclcompat**。新增方法時務必維持 1-based 與 BCB6 語意。
- `TStringList`/`TStrings`、`TDateTime`(OLE double, days since 1899-12-30)、`SysUtils`（IntToStr/StrToInt/StrToIntDef/StrToFloat/FloatToStr/FloatToStrF/Format/FileExists/ExtractFileName/Path/Now/FormatDateTime…）。只實作專案用到的子集；要新方法就照 BCB6 補。
- 建置：CMake 把 vclcompat 編成 static lib `vclcompat`，各模組 lib 連它；cJSON.c 以 C 編（`set_source_files_properties LANGUAGE C` + 標頭 extern "C"）。

## vclcompat 補充（W5）
- **`ClientSocket.h/.cpp`**（TCP client shim，2026-07-11 新增）：仿 `Comm.h`(TComm serial shim) 的 Sim/Real 二分模式——Sim 預設(離線可測、in-memory loopback)、Real 走 WinSock2(MinGW 內建，同 `Public/WinSocketErrorCode.cpp` 既有 WSA 錯誤碼轉譯可共用)。由 `MyPLC/ModbusTCPClient.cpp` 首次引入，**未來 TesterTCP.cpp/SECSGEM 協定引擎(uHGemEquipment.cpp 的 HSMS TClientSocket/TServerSocket)波次應重用此元件，不要重新發明**——三者都需要類似的 TCP client/server 需求，recon 已標記這是共用基礎設施。
- 新增元件前務必先檢查 `vclcompat/` 現有檔案，避免重複發明（本波前先核實過僅 MyPLC 這個 unit 真的需要 TCP client，其餘同批 unit 皆不需要）。
- **預測已兌現（2026-07-11 W5-Final）**：上一條「未來 TesterTCP.cpp 波次應重用此元件」如期發生——`Interface/TesterTCP_Socket.cpp` 直接建於 `ClientSocket.h` 之上，並反向擴充該 shim 4 項：`Tag`(int，模擬真 VCL `TComponent.Tag`，供 BarCode 的 13 個共用 socket 事件處理常式透過 `(TClientSocket*)Sender)->Tag` 判斷是哪個 socket 觸發)、`Open()`(`Active=true` 薄別名，同一條 `DoConnect_()` 路徑)、`SendText()`/`ReceiveText()`(原始位元組透傳，不處理行終止符——呼叫端自理，同真 VCL `ScktComp` 語意)。SECSGEM 的 HSMS TClientSocket/TServerSocket 波次仍待重用同一元件。
- **`TList.h`**（VCL.Classes `TList` 泛型 `void*` list shim，2026-07-11 新增，SECSGEM SV/EC 註冊 API 前置需求）：**刻意不 `using` 進全域命名空間**——`aHotPlateSubstrate.h`（硬邊界共用檔）已存在一個不相關、範圍較窄的全域 `class TList`，兩者同時可見會在全域命名空間重定義衝突（已實測編譯確認）。消費端一律用完整限定名 `vclcompat::TList`，或在不含 `aHotPlateSubstrate.h` 的 TU 內自行 `using vclcompat::TList;`。**任何新翻譯單元若同時需要 golden `TList` 語意又會 `#include aHotPlateSubstrate.h`，務必先檢查這條命名衝突，不要重新踩。**
- **`SysUtils::StringReplace`**（2026-07-11 新增，`TesterTCP_Socket.cpp`/BarCode 需求）：`TReplaceFlags`/`TReplaceFlag`(`rfReplaceAll`/`rfIgnoreCase`) 支援 Borland `Set<>` 疊加寫法 `TReplaceFlags()<<rfReplaceAll`；空 `OldPattern` 是 no-op(防無窮迴圈,同 Delphi 官方行為)。
- **`ServerSocket.h/.cpp`**（`TServerSocket`多連線 shim，2026-07-13 新增，尚無真消費端）：**預測再次兌現**——上面「SECSGEM 的 HSMS TClientSocket/TServerSocket 波次仍待重用」如期發生。golden 的 `clientGemRead`事件常式**同時**被接到 `TClientSocket.OnRead`與`TServerSocket.OnClientRead`(`.dfm`兩處)，代表連線物件必須是**同一個**`TCustomWinSocket`可重用型別，因此本次是**擴充**既有`ClientSocket.h`(加`Connected`/`LocalAddress`/`LocalPort`/`Handle`/`SocketHandle`欄位+`SetReadNotifyHook`/`SetCloseNotifyHook`/`StartReaderThread`/`AttachRealSocket_`四個 additive-only 掛鉤)而非另建平行型別。**⚠ Real-mode 拆卸 use-after-free gotcha(已修，記取教訓)**：`TServerSocket`第一版只在`~TServerSocket()`裡`delete`每條連線，卻從未等待該連線自己的背景 reader thread(`ReaderProc_`，卡在`recv()`)真正結束——`TClientSocket::DoClose_`自己一直都有做`shutdown+WaitForSingleObject+CloseHandle`這套 join，但`TServerSocket`是獨立 TU 摸不到那段 private `Impl`，翻譯時漏了對應動作。修法：在`TCustomWinSocket`上新增公開的`StopReaderThread()`(同一套 join 邏輯抽出來)，`~TServerSocket()`對**每一個**曾建立過的連線都先呼叫這個方法才真正刪除。**日後任何新 shim 只要牽涉「背景 thread 寫入的物件被另一個執行緒/解構子刪除」都要問這個問題**：deferred-delete 只解決「同執行緒自我刪除」，解決不了「跨執行緒刪除時背景 thread 還沒收工」。

## vclcompat 補充（W3）
- 檔案系統：`SysUtils` 加 FindFirst/FindNext/FindClose/TSearchRec/faAnyFile/faDirectory/faReadOnly/RemoveDir/FileSetAttr/FileGetAttr/HexStrToInt（backed by MinGW windows.h）。
- INI：`IniFiles.h/.cpp` 的 `TIniFile`(write-through，每次寫即 flush)/`TMemIniFile`(eager load，僅 UpdateFile/dtor flush)，共用 TIniStore（插入序、raw-byte Big5-safe、section/key 大小寫不敏）；default-fallback verbatim；ReadInteger 支援十進位與 $/0x hex；ReadFloat 用 '.' 小數；double 以 `%0.4f` 文字存。已 43/43 測試。
- **⚠ Win32 巨集衝突（重要 gotcha）**：`<windows.h>`（經 common.h→MachineType.h 帶入）`#define DeleteFile DeleteFileA`、`CopyFile CopyFileA`，會 shadow vclcompat 的 `DeleteFile(AnsiString)`/`CopyFile` 多載（巨集先改 token，命名空間限定救不了）。暫以呼叫端 `#undef DeleteFile/CopyFile`（include 後）解。**待辦：在 vclcompat 傘狀標頭 vcl_compat.h 統一 `#undef` 這些 A/W 巨集（或改名 vclcompat 函式）**。

## ⚠ 翻譯中文註解亂碼（U+FFFD）— 根因 + go-forward 規則
- **根因**：Read 工具把 Big5(cp950) 原始碼當 UTF-8 解→中文變 U+FFFD，agent「看到的」就是亂碼，故無法忠實重現中文註解。已中招：`cprod.cpp`(283)、`cpublic.cpp`(62)（皆只在註解，編譯不受影響；golden 唯讀樹仍有原中文於同 path:line 可反查）。其餘 67 檔乾淨。
- **AI(W906-W7-DOCfix) 20260731 全樹重掃更正**：以 `errors='strict'` UTF-8 解碼 + 計數 U+FFFD 碼位掃過整棵 ported 樹（排除 `.git`/`build*`/scratch），**中招的是 3 個檔不是 2 個**，且全部是既有 commit 帶進來的：`cprod.cpp` 283 行 / 2,443 個 U+FFFD、`cpublic.cpp` 62 行 / 520 個、以及本節原本沒記的 **`tests/test_IniFiles.cpp` 1 行 / 4 個**（該行是一段描述 Big5 位元組的英文註解，寫的人把 Big5 位元組貼成了 U+FFFD；無害但同樣違反 S12 閘 4）。全樹合計 **2,967 個 U+FFFD、346 行、3 檔**。所以 S12 閘 4「全樹 = 0」從來沒有被達成過——見 plan S12 閘 4 的重新界定。
    - **AI(W906-W7-DOCfix2) 20260801 複驗**：本次重跑同一個掃描（UTF-8 嚴格解碼，排除 `.git`/`build*`/scratch 與二進位副檔名，掃到 1,644 個非空檔），上面每一個數字都逐字重現：`cprod.cpp` 283 行 / 2,443、`cpublic.cpp` 62 行 / 520、`tests/test_IniFiles.cpp` 1 行 / 4，合計 2,967 / 346 行 / 3 檔。**唯一被拿掉的是原本那句「這 3 檔沒有一個出現在 W7-F1/F2/L1 那批 30 個檔案裡（該 30 檔全數乾淨）」**——「那批 30 個檔案」在任何文件裡都沒有可據以重數的清單，本次無法複驗，故不保留該宣稱（三個中招檔的身分本身不受影響）。
- **go-forward 規則（翻譯 agent 必遵）**：翻含中文註解的檔時，**以 cp950 感知方式讀 golden**（`iconv -f CP950 -t UTF-8` 或 python `open(encoding='cp950')`）取得真中文，譯出檔註解寫成**正確 UTF-8**；若不便，則以**英文 gloss + golden file:line 出處**取代該中文註解（精確中文留在唯讀 golden）。**勿用 Read 工具直接搬中文註解**。
- 補救（低優先，已追蹤）：對 cprod.cpp/cpublic.cpp 跑一次 cp950→UTF-8 註解轉碼修復（見 ROADMAP DEFERRED）。
- **AI(W906-Gate4) 20260802 現況更新（閘 4 的另外那一半已經清掉了）**：plan S12 閘 4 長期紅著的 **414** 個違規，其中 **411 個「缺結尾換行」已全部修復**（產生器 8 個 `json.dump()` 呼叫點 + 一次性掃描，見本檔 gotcha #19；四支 dfm2rc 閘測試全過含逐位元組的 G7）。**閘 4 現在是 `scanned 1460 file(s); 3 violation(s)`，剩下的 3 個就是本節這三個 U+FFFD 檔**——它們現在是全樹唯一擋著閘 4 轉綠的東西，優先級因此不再是「低」。
- **⚠ 修復這三檔時的關鍵事實（本次實測，動手前必讀）**：**ported 與 golden 的行號不對齊**，不能用「同 path 同 line」去取原文。實測 `cprod.cpp` golden **4001** 行 vs ported **4037** 行（差 36），抽查偏移非定值：ported `:13` 對到的內容在 golden 完全不同位置，ported `:67`(`class ARM_OFFSET *InArmOffSet[...]`) 與 golden `:97`(`class ARM_OFFSET *InArmOffSet_File[...]`) 也只是相鄰兄弟而非同一行。→ **正確作法是以「`//` 前的程式碼文字 + 註解的作者/日期前綴」在 golden 內做唯一比對**，比對不唯一的個案要單獨列出人工裁決，不可猜。

## 已翻譯模組 + 領域發現
- **Public/HTMD5（W0）**：`class MD5` + `md5()/md5_File()/md5_Folder()`（回 AnsiString）+ SearchFile/SearchFolder。翻譯時：VCL `TMask` 萬用字元比對→重建簡易 matcher；`std::auto_ptr`→`unique_ptr`；`MyDBIProcess` log→stub；`FindFirstFile` 用 MinGW windows.h（可攜）；保留 Ifor 20200826 buffer-overflow fix。MD5 演算法 bit-exact（命中 RFC1321 向量）。
- **Public/cJSON（W0）**：標準可攜 C，原樣翻、extern "C"、以 C 編譯，parse/print round-trip 過。
- **全域標頭（W0-tail）**：`MachineType.h`/`Config.h`/`CosFunction.h`/`myTimer.h`/`cpublic.h`/`cprod.h`/`cmydef.h`/`MachineDefine.h` 已去 VCL 化、單一 TU 一起編可過（lib `ht9045_globals` = myTimer/cprod/cpublic/cmydef .cpp）。**契約逐位保留**（PROD_INFO_ST 欄位 layout、enum/#define 值，已用 static_assert/offsetof + test_globals 對拍）。處理：`vcl.h`→vclcompat；`PACKAGE`/`__fastcall` 中和；form 指標 `extern PACKAGE TfXxx*`→前置宣告 `class TfXxx;`(W7)；前置宣告 `class TMyLog;`/`class TMyStringList;`、`typedef int TColor;`、uPoint2D stub、`typedef LARGE_INTEGER TLargeInteger;`(myTimer)；ARM_OFFSET 的 `_fastcall` ctor/dtor typo 修掉；cmydef.h 丟棄未用 include(handlerlog/MyBinDisp/HTEditList)。`.cpp` 觸及 main.h/狀態機的 body `#if 0`→W3/W6（見 ROADMAP DEFERRED）。Config.h/CosFunction.h 是 cprod.h 的純 value-struct 相依，一併翻。
- **ContactForce（已翻計算核心）**：`ContactForce.cpp/.h` 其實是 **VCL 表單 `TfContactForce` + 四個 SLK 元件類別**（THTSLKClass / THTSLKIndClass / THTDieForceSLKClass / THTDieForceOneByOneSLKClass），是 **SLK 直徑→力 的資料產生端**。文件常提的 `CalculateTotalAirForce`/`GetMinForce`/`GetMaxIndexForceLimit` 其實在 **`cContact.cpp`（VCL 表單 TfContact，計算消費端）**，讀 `fContactForce->SLKClass[i]->dDiameter/dMinForce/dMaxForce`。已抽出 ContactForce 的純計算核心為 C++（`SlkForceData`/`ComputeSlkForce`/`ComputeEpMaxVoltage`，root 鏡射）。
- 翻譯慣例（已確立）：extract-calc-core（VCL 表單/widget/IO 不翻，純算抽成可測函式，輸入用 struct/參數）；保留字面常數（如 `3.14` 不換 M_PI 以維數值同一）；特例綁對的類別（如 402 僅 STANDARD）；caption-only 變數（iCount）不影響數值。
- 驗證模式（無 Borland）：CMake(MinGW Makefiles)+g++ 6.3 編譯（`-Wall -Wextra -Wshadow -Wconversion -Wpedantic` 零警告）+ CTest；數值對「手算 BCB6 公式值」（非 live diff，已標限制）。

## 命名對照（鏡射 BCB6，供反查；C++ 端保留原名/原方法名）
- 運動：`HTMotor`(Motor/HTMotor.h) 已是虛擬基底（~50 virtual）；方法保留 InitMotor/MoveToPos/ReadPos/HomeObject/HomeFlag/Stop/MotionDone/GetAlarm/SetSpeed(uint,bool=false)/**ServerOnOff**(BCB6 拼法)/SetSoftLimit/JogP/JogN/ScanMotorStatus。加一個 Sim 子類即可。
- IO：`TLaneIO`(MyLaneIo.h)：IOBitOn/IOBitOff/IOByteOut/IOOutBitStatus/IOInputBit/IOInputByte（int 回傳，<0=fail）。
- 授權：`KEYPRO_GET_LEVEL`(Public/HTKeyPro.h, HS_Function.cpp:4933)。
- Tester：`ITesterInterface` 概念（main.h:1213/1277/1337/1392/1400）：Find/RunTestProgram/SendMSG_CMD/CloseGpibProgram/GetTesterResult。
- 全域：`PROD_INFO_ST Prod`(cprod.h)；`iArmTask` 系列。

## 建置 / 工具（本機）
- **g++ 6.3.0 (MinGW) + CMake 4.0.2**。翻譯成**可攜標準 C++**（建議 C++14/17），用 CMake + g++ 編譯驗證可攜性。
- **⚠️ 2026-07-28 實測更正**：本節原記載「無 MSVC/clang……最終 MSVC build 在開發機驗」**已過時**。本機**有** MSVC：VS 2022 BuildTools 17.14.3（`cl.exe` at MSVC `14.44.35207`/`14.42.34433`）+ VS 2019 BuildTools 16.11.47（`14.29.30133`），Windows SDK `10.0.18362`～`10.0.26100`。實測可用 MSVC configure+build 全樹並跑完 95 個 ctest。**但 MFC 元件未安裝**（`atlmfc\` 只剩 `lib\spectre\arm64` 空殼，無 `include\`／`afxwin.h`／`mfc*.lib`），需跑 VS Installer 加「C++ MFC for latest v143 build tools」。→ MSVC-targeting 的碼在本機即可驗證，但**跑 MSVC 測試前務必先讀 Gotcha 9**。 **✅ 2026-07-29 再更正：MFC 元件已安裝並實測通過**——`Microsoft.VisualStudio.Component.VC.ATLMFC` 已裝進 `...\MSVC.44.35207tlmfc\`（`afxwin.h`+`atlbase.h` 皆在，include 346 檔、x86/x64 各 31 個 lib），正是本專案使用的 toolset。**驗收是實編非只看檔案存在**：一個含 `CDialog` 子類+`DECLARE_MESSAGE_MAP`/`ON_BN_CLICKED`+`CWinApp`+`CString` 的 probe，在 32-bit `vcvarsall x86 -vcvars_ver=14.44` + **Release CRT `/MD`** + `/D_AFXDLL` 下 `CL_EXIT=0`。→ **W7-U（MFC UI 本體）的工具鏈阻塞已解除。**⚠️ 編譯時會出現 `_WIN32_WINNT not defined. Defaulting to _WIN32_WINNT_MAXVER` 資訊訊息，接 UI 波前應明確定義 `_WIN32_WINNT`，避免各 TU 隱含目標版本不一致。⚠️ 安裝方式備忘（先前連錯三次）：正確形狀是 GUI 模式 `setup.exe modify --installPath "<含空格路徑要加引號>" --add <ComponentId>`；**不可**加 `--norestart`（它必須搭配 `--quiet`/`--passive`，單獨用 exit 87），也不可用 `--quiet`/`--passive`（拒絕自我提權 exit 5007，而本 shell 非互動叫不出 UAC）。失敗時看 `%TEMP%\dd_installer_*.log`（Big5，需 cp950 解）。
- 純邏輯/計算 class 可獨立 CMake target + g++ 編譯 + 簡單測試框架（或自寫 assert main）驗證，無需 VCL/硬體。

## Gotcha（踩過/要注意）

> ### ⚠️ Gotcha 編號登記表（**本檔是唯一發號單位**，AI(W906-W7-DOCfix) 20260731 建立）
>
> **背景（真的踩到）**：2026-07-31 發現三份文件各自在配 gotcha 編號 —— `W7_UI_ARCHITECTURE_PLAN.md` S10 的第 7/8/9/10 項自行預留「新增 KNOWLEDGE gotcha #10/#11/#12/#13」，而本檔的 **#10 早就是另一則**（ported 行號腐爛），`DEVLOG.md:1214` 又另外預留 **#11** 給第三個無關主題（測試可失敗性）。三份文件對 10-13 的說法互不相同。
>
> **裁決**：編號**只在本檔配發**，其他文件（plan / DEVLOG / 報告）一律**引用**本表的編號，不得自行造號。
>
> **AI(W906-W7-DOCfix2) 20260801 更正 —— 本裁決原本的最後一句是假的**：原文寫「plan S10 第 7-10 項**已改成**引用本表」。那句在 2026-07-31 寫下的當下**不成立**，而且寫它的 `AI(W906-W7-DOCfix)` 波**根本沒有寫過那份 plan**——它在自己沒碰過的檔案上宣告了完成式。以 `git show f61e25e:...W7_UI_ARCHITECTURE_PLAN.md` 核對（本次親自跑）：當時 S10 第 7-10 項仍逐字寫著「**建議新增 KNOWLEDGE gotcha #10/#11/#12/#13**」，零回指——正是本裁決宣稱已經化解的那組碰撞。
>
> **真正改掉它的是 2026-08-01 的另一波 `AI(W906-W7-DOCS)`**（本次親自讀過 plan S10 現況）：第 7-10 項現在各自開頭標「⛔ 編號已被取代（改用 KNOWLEDGE gotcha #13 / #14 / #15 / #16）；內容仍未寫入」，第 10 項之後另有一段掛 `AI(W906-W7-DOCS) 20260801` 的統一狀態說明，指向本表的 #13-#16 並要求「後續引用一律用 KNOWLEDGE 的 #13-#16，不要再用本檔第 7-10 項寫的 #10-#13」。**該筆 plan 變更在本行寫下時仍未提交**（工作區狀態，由文件軌道持有）。裁決本身不受影響——發號權仍只在本檔。
>
> | # | 主題 | 狀態 |
> |---|------|------|
> | 1-9 | 見下方原文 | 已寫 |
> | 10 | 散文/註解引用 **ported 樹行號**會腐爛 | 已寫 |
> | 11 | **證明測試會紅（proof-of-failability）才算交付** | 已寫（本次新增；即 `DEVLOG.md:1214` 預留的那一則） |
> | 12 | **Sensor 三態陷阱 + Cylinder 用相反慣例** | 已寫（本次新增） |
> | 13 | `rc.exe` 的 `\xHH` 在 ANSI 字面值裡逐 byte 靜默毀字；`windres` 比 `rc.exe` 寬鬆 | **保留號，未寫**。證據在 plan S9-R2 / S9-R4；本次 DOCfix 波沒有能力獨立複驗（需實跑 `rc.exe`），故不代寫 |
> | 14 | `aled.pas` `CreateLedBitmap` 的尺寸覆寫**看起來**會丟掉 `.dfm` 幾何，實測 1,840 個裡 0 個被丟棄 | **保留號，未寫**。證據在 plan S6-B1c |
> | 15 | FP 驗證腳本必須用 runtime/volatile 輸入（常數折疊會給假結論） | **保留號，未寫**。證據在 plan S4-V5 |
> | 16 | `.dfm` 直方圖抓不到**動態建立**的控制項（`TTMyTray256` 0 個 `.dfm` 實例卻被 `new`） | **保留號，未寫**。證據在 plan S4-V12 |
> | 17 | **以剖析原始碼文字為手段的 pin，其註解剝除器必須「認得字串字面值」**，否則字串裡的 `//` 會靜默吃掉被 pin 的那段碼 | 已寫（AI(W906-W7-DOCfix2) 20260801 新增，證據自跑） |
> | 18 | **「每個呼叫點都走到了」不等於「下對了命令」**——軌跡/覆蓋型斷言完全不約束致動器的**方向與極性** | 已寫（AI(W906-W7-DOCfix2) 20260801 新增，證據自跑） |
> | 19 | **`tools/dfm2rc/` 三組產生語料的行尾慣例不一致（`ir_out`/`reports` = CRLF，`layout_out`/`rc_out` = LF），而閘 G7 逐位元組比對**——補結尾換行時補錯 EOL 會把 G7 弄紅 | 已寫（AI(W906-Gate4) 20260802 新增，證據自跑） |
>
> | 20 | **`ctest` 的 exit code 對「失敗集合」零鑑別力**（任何測試失敗都回 8）——拿它判 gate 會把紅燈讀成綠燈 | 已寫（AI(W906-FW-GATEVERDICT) 20260827 新增，證據自跑） |
>
> | 21 | **括號平衡切函式 span 時不剝註解 → 註解裡的 `{` 會靜默吞掉後面數十支定義**，讓 census 造出假缺口 | 已寫（AI(W906-FW-SPANSANITY) 20260827 新增，證據自跑） |
>
> 13-16 標「保留號、未寫」是刻意的：把號先鎖住可以消除碰撞，但**不冒充已驗證**。要寫進來的人請自己重跑證據再補正文，並把狀態改成「已寫」。

1. **AnsiString 1-based vs std::string 0-based（最高風險）**：`AnsiString.Pos()`/`.SubString()` 是 **1-based**（全專案 ~1,910 處）。直翻成 `std::string`(0-based, npos) 會 off-by-one／切錯字串且**編得過**。翻譯時逐處改 index，或先做一個 1-based 相容的 AnsiString-like 包裝（`.Length()/.Pos()/.SubString()/.UpperCase()/.sprintf()` 同名同語意）降低風險。`.UpperCase/.LowerCase/.sprintf/.Trim/.Delete` 在 std::string 無對應，需 helper。
2. **Big5 編碼**：906 原始碼是 Big5。翻譯出的 C++ 檔若含繁中字面值，需確保編譯器以正確 code page 讀（MSVC `/source-charset`；或避免在碼裡放中文、字串走外部資源/Big5 I/O 邊界）。執行期：外部 Big5 資料在 I/O 邊界轉碼，內部統一一種表示；加 U+FFFD 檢查。勿把 Big5 原始碼整檔轉 UTF-8（見記憶 ht9045-big5-edit-corruption）。
3. **CSV 名稱式解析**：`database.cpp`(SetMOTTableNo/SetIOTableNo) 用 **header 名稱**(AnsiPos、last-match) 解析 Mot_Table/IO_Table，**非固定位置**；**Mot_Table 實體欄序 ≠ emot* enum 序**(database.cpp:2047-2079)。翻譯時務必做名稱式解析，勿位置式（會 silently 誤對馬達/IO）。真實 header：Mot_Table 29 欄、IO_Table 15 欄。
4. **config 語意細節**(database.cpp)：IO 欄數須 ==15；Mot 接受 >=28；空 cell：IO Lane/ModuleType/IP/Port/Bit→-1、InType→0、ISABase→0(eMotionNet)、Enable→0；**Port 當 ISABase∈{1,2,4} 以 HEX 解析**(database.cpp:1825-1830)，否則十進位。
5. **全域狀態耦合**：`PROD_INFO_ST Prod`(~770 行 cprod.h:368-1136)、`iXXXTask`、`HotTime[2][50][50]`、offset 陣列是跨模組/跨執行緒耦合熱點（接縫盤點 §B TOP-5），翻譯時先切邊界，勿整包搬。
6. **workflow/session 中斷復原（2026-07-13 實例）**：一整批已完成、已 CMake 接上、各自 build 目錄驗證過的翻譯工作（W5-Final：KYECFTP+Automation 剩餘+TesterTCP_Socket+BarCode 收尾+SECSGEM 前置切片，2026-07-11 執行），因當時 session/workflow 中斷，跨戰線合併複驗+DEVLOG/ROADMAP 記錄+commit 三步從未執行，工作樹帶著 2 天未提交狀態。接續時**不可信任何殘留的 agent 文字或 cached 摘要**，必須：(1) `git status`/`git diff --stat` 看實際未提交變更，(2) 對照檔案時間戳判斷是同一批還是不同批工作，(3) 若有舊 build_* 目錄，讀 `Testing/Temporary/LastTest.log` 當線索但不當定論，(4) 一定要重跑一次全新 from-scratch build+ctest 拿到當下的 ground truth，才能決定是否可以安心 commit。
7. **AI 修改註解標籤別混用 V899/V906（2026-07-13 踩過）**：CLAUDE.md 的「AI 修改註解格式」(`//AI(AgentName) YYYYMMDD:`，AgentName 預設 `ht9045-v899`) 是**專屬 V899 BCB6 直改檔案**的慣例；V906 這邊翻譯波次一律用「波次代號」當標籤(如 `AI(W5-Final-Integrate)`、`AI(W906-ServerSocket)`)，不是 agent 名稱。曾有翻譯 agent 把整批 V906 新註解誤標成 `AI(ht9045-v899)`(59+2 處)，事後才發現改回正確波次標籤——**指派翻譯 agent 時若沒特別強調，容易把兩套慣例搞混，之後每次交派新翻譯任務前應在 prompt 裡明講這是 V906、用波次代號、不是 V899 的 AgentName 格式**。
8. **背景 thread 物件的跨執行緒刪除，deferred-delete 不等於安全（2026-07-13 踩過）**：`TServerSocket`（vclcompat 新 shim）第一版把每條連線的「不在自己執行緒上自我刪除」這個 hazard 處理對了（deferred-delete 到`~TServerSocket()`才真的`delete`），但漏了另一個獨立問題：`delete`之前並沒有等待該連線自己的背景 reader thread 真正結束(`recv()`卡住的執行緒)，導致刪除當下該執行緒仍可能在跑、之後 dereference 已釋放記憶體。**這是兩個不同的 hazard，各自要分開處理**：(a) 同執行緒自我刪除→deferred-delete 解；(b) 跨執行緒刪除時背景 thread 是否已經真正收工→一定要顯式`join`(`WaitForSingleObject`)，deferred-delete 完全沒處理到這塊。任何新 shim 只要有背景 thread 寫入某個物件、而該物件可能被別的執行緒(含解構子)刪除，都要同時檢查這兩點。

9. **測試崩潰會彈 modal 對話框、卡死整批 ctest 並洗版使用者螢幕（2026-07-28 踩過，已加防護）**：一個 agent 用 **MSVC Debug CRT** configure+build 全樹後跑 `ctest -j4`，跑到 **94/95 就無限停住**——因為 Debug CRT 的 `assert()`／`_CrtDbg` 失敗會彈出 modal「Debug Assertion Failed!」對話框等人按確定。**致命之處在於它不寫 log**：ctest log 看起來只是「卡在最後一支」，完全看不出是在等互動，而使用者螢幕同時被彈窗洗版。這不是 MSVC 專屬——MinGW 下的硬崩潰（access violation、`STATUS_STACK_OVERFLOW`，本專案兩種都真的發生過，見 DEVLOG 2026-07-27）會彈 Windows 錯誤回報視窗，同樣會 block。**已加兩道防護（勿移除）**：(a) `tests/test_bootstrap.cpp`——透過 `tests/CMakeLists.txt` 最上方的 `ht9045_test_bootstrap` INTERFACE library 編進**每一個** test 執行檔，static init 階段呼叫 `SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX|SEM_NOOPENFILEERRORBOX)`，MSVC 另加 `_CrtSetReportMode/_CrtSetReportFile`(→stderr)、`_set_abort_behavior(0,...)`、`_set_invalid_parameter_handler`；(b) 同檔最下方對全部 95 個 test 設 `TIMEOUT 600`。**刻意用 INTERFACE library 而非 static library**——只含 static initializer 的 object 檔會被 linker 當成沒人引用而丟掉。防護**不會遮蔽失敗**：測試照樣失敗，只是改成立刻在 stderr 大聲失敗（實測 null-deref 立即 exit 139、零彈窗、沒吃到 timeout），而不是無聲阻塞。**衍生紀律**：任何在背景／平行 agent 裡執行的程式，都不可以有能彈 modal 視窗的路徑——那會綁架使用者的機器，而且從 log 上完全看不出原因。

10. **在散文/註解裡引用「ported 樹」的行號會腐爛，而且是本專案審查發現的最大單一來源（2026-07-28/29 反覆踩到，已定紀律）**：連續三輪修正波次（審查 15 項 → 修完又冒 11 項 → 再修完又冒 13 項）中，**絕大多數新發現都是同一種**：某段新寫的註解／docstring 引用了 ported 樹的某個 `檔案:行號`，而那個行號是錯的、或在寫下的當下就已經被同一次編輯推移掉了。最荒謬的一例：某次修正加了 7 行揭露註解，結果**同一個 pass 自己寫進 `W7-UI-SKIPPED.md` 的 `SCK_ART_Remainder.cpp:1651` 立刻變成 stale**，因為那 7 行把目標往下推了。另一例：某 wave 在 `forms/fMain.h:157` 正確引用 `main.h:543`，卻在 5 行後的 `:162` 把同一個 `:543` 誤用給另一個成員。
    **根因是結構性的，不是誰不小心**：`golden` 樹是**唯讀且永不改變**，所以引用 golden 的 `檔案:行號` 是穩定資產、值得寫（也是本專案翻譯保真度的核心憑據，必須繼續寫）；但 **ported 樹每一波都在動**，任何指向它的行號從寫下那一刻就開始腐爛，而且沒有任何自動機制會偵測到。
    **紀律（go-forward）**：
    - 引用 **golden** → 照舊寫 `檔案:起-迄`，這是必要的，golden 不會變。
    - 引用 **ported 樹** → **寫符號名稱，不寫行號**（例如「見 `TStringGrid` 上方的 SCOPE 註解」、「見 `RenderLed` 的 flood-seed 推導」），需要更精確就寫「函式名 + 小節標題」。已經有正確示範：`Controls.h` 的 `Down` 修正就是刻意改成「by name rather than by an absolute line number, so it cannot rot」。
    - 寫「已驗證」「verified this wave」「grepped the whole tree」這類**總括式宣稱前先想清楚它可不可證**——`FormWidgets.h` 就因為掛了一句「本波所有 golden 引用皆已 grep 驗證」，結果其中一條是錯的，整句宣稱反而變成負資產。**寧可不寫總括宣稱，只寫你真的逐條查過的那幾條。**
    - 派修正波次時，prompt 一律加：「你寫的替代文字必須是你親自查證過的，不要用一個未驗證宣稱換掉另一個」。實測有效但**不足以**擋掉行號腐爛，所以要靠上面的「ported 樹不寫行號」從源頭消除。
    **另一個相關的教訓（審查建議本身也可能是錯的）**：`LedRender` flood-seed 事件中，第一輪審查的「建議修法」本身誤讀了 golden（沒注意 `FloodFill` 在 `CreateLedBitmap` 內、緊接風格尺寸賦值之後），主迴圈照抄進修正指示，結果造成真迴歸（1,840 個 LED 實例中 262 個偏離，一個實例直接渲染出 0 像素）。**審查的「發現」與「建議修法」要分開對待**：發現通常可信（它是從證據來的），建議修法必須自己重新從 golden 推導一次才能派工。

11. **交付物是「證明測試會紅」，不是「測試通過」（proof-of-failability；2026-07-29/31 反覆踩到，已定紀律）**：在**同一個檔案家族**上，要求 agent 寫「有意義的測試」**兩次都拿回恆真斷言**，而且 agent 兩次都相信自己照做了。第二次前主迴圈已明確加碼要求「要真的 pump 狀態機並斷言全域變數，不要只呼叫函式看回傳」，agent 也回報照做——**做出來的仍是恆真式**。
    **所以「要求測試有意義」這個指令形狀本身無效**。唯一實測有效的形狀是把**證明**列為交付物：
    - 指定一個具名 mutation（例：把某個 `Task=50` 改成別的值、把某個 ctor 改名、把某個 initialiser 由 0 改成 3）；
    - 要求 agent **實際套用**它、**跑**測試、貼出**那一條具名斷言**變紅的輸出；
    - 還原 mutation，並在回報裡同時附上 mutation 內容 + 紅的輸出 + 已還原的確認。
    「綠色的測試套件」不是任何東西的證據；測試名稱也不是。**只有看過它紅過，才知道它在測東西。**
    **推論（同樣重要）**：連「這個測試是 load-bearing」這種**註解**都不可信 —— 有一輪某測試檔自己寫著「把 ctor 改名會讓這個測試大聲失敗（by design）」，mutation agent 真的改了 ctor 名，測試**維持綠色**。
    **這個缺陷已經在已 commit 的基線裡，不只是風險**：`tests/test_w6_1_empty_canary.cpp` 裡 `DoLoadNewEmptyTrayToCar` 那個 200 次上限收斂迴圈後面的第二條 `CHECK`（AI(W906-W7-DOCfix2) 20260801 把原本寫在這裡的 ported 行號 `:197` 改成符號定位，本條自己就在違反 gotcha 10；當日重讀確認該 `CHECK` 仍在），就是被禁止的形狀：
    ```
    for (steps = 0; steps < 200; ++steps) { if (DoLoadNewEmptyTrayToCar()) { done = true; break; } }
    ...
    CHECK(done || iLoadNewEmptyTrayToCarTask != 1 || steps == 200, "...");
    ```
    離開那個 bounded loop 只有兩條路：`break`（則 `done==true`）或迴圈跑完（則 `steps==200`）。所以 `done || steps == 200` **恆為真**，第二個 disjunct 完全不影響結果 —— 這條 `CHECK` **不可能失敗**。掃新測試時就用這個判準：**在 bounded loop 之後，把 loop 的終止條件本身寫進斷言的 disjunction，就是恆真式。**

12. **Sensor 是三態、Cylinder 用相反慣例（兩者都會讓「停用」被讀成「有料」；本專案七路獨立 recon 有六路各自重新發現同一件事，故收進本檔而不是留在每一波的腦袋裡）**：
    - **`TMySensor`（golden `mysensor.cpp`）**：`IsOn()`(:79) 與 `IsOff()`(:119) **兩者在 `Enable==false` 時都 `State=-1; return false;`**。所以一顆停用的 sensor **同時滿足** `IsOff()==false`（讀的人常當成「有料/到位」）**和** `IsOn()==false`（讀的人常當成「沒料」）。`IsOn()`/`IsOff()` **不是互補**，它是三態（on / off / disabled）被塞進 bool。
    - **`TMyCylinder`（golden `mycylin.cpp`）用相反慣例，而且看的是不同旗標**：`OnSensor()`(:151) / `OffSensor()`(:169) 開頭是 `if(Enable==false) return true;`——停用回 **true**；而 `OnStatus()`(:123) / `OffStatus()`(:190) 是 `if(OnSenEnable){...} else return false;` / `if(OffSenEnable){...} else return false;`——停用回 **false**，**且看的是 `OnSenEnable`/`OffSenEnable` 這組不同的旗標，不是 `Enable`**。（AI(W906-W7-DOCfix2) 20260801：本行原本把它寫成 `if(OnSenEnable==false) return false;`，那個形狀在 golden 裡不存在——語意相同但**不是逐字**，已改成實際形狀；本節 golden 引用 `mysensor.cpp:79/:119`、`mycylin.cpp:123/:151/:169/:190`、`asendic.cpp:529-560/:661-662/:720` 皆已於當日重新以 cp950 解碼 golden 逐行複核。）
    - **真的咬過人**：golden `asendic.cpp` 的 `AutoCylinderUp` 在同一支狀態機裡把兩種慣例混用——`case 100`(:661-662) 走 predicate `AutoCylinderMidIsOn`(:529-560，`Enable==false` 回 **true**)，`case 201`(:720) 卻改要 raw 的 `Cylinder[CylinderNameMid].OnStatus()`(停用回 **false**)。**結果：在預設離線全停用設定下這支 SM 會在 `1→50→100→200→201→1` 之間永遠繞圈、永不回 true。** 詳見 `W7-UI-SKIPPED.md` 的 W7-L1 段。
    - **拘束性規則**：(a) 翻譯時把每一個 `IsOn`/`IsOff`/`OnStatus`/`OffStatus`/`OnSensor`/`OffSensor` 的**拼法逐字保留**，**絕不**把 `IsOff()==false` 正規化成 `IsOn()`（或反之）——那在 disabled 態下是不同的值；(b) 寫測試時**明確設定 enable/type**餵資料，不要依賴「預設全停用」跑出來的結果，否則斷言測到的是 disabled 分支而不是邏輯。

> （13-16 依登記表為「保留號、未寫」，故下方正文由 17 接續，不是漏編。）

17. **以「剖析原始碼文字」釘住值的 pin，註解剝除器必須認得字串字面值，否則字串裡的 `//` 會靜默吃掉被 pin 的那段碼（2026-08-01 本波自跑證實）**：
    當某個常數只活在某個 TU 的 file-`static` 物件裡、任何測試 TU 都指不到它時（本專案 `csystem.cpp` 的兩個 SckArt seam 正是如此），唯一可用的守門形式就是「讀該檔的原始碼文字、按符號名找到那個初始式、比對它的值」。這種 pin 的正確性**完全等於它的詞法正確性**。
    - **洞在哪**：只認「`//` 到行尾」的剝除器，一碰到**字串字面值裡面的 `//`**，就會連同該行後面的真程式碼一起刪掉。被刪掉的若正好是「ctor body 裡改寫該欄位」那一行，pin 就看不到改寫，於是回報「初始式仍是 0」而**維持綠色**，但物件實際建構出來的值已經變了。
    - **實測（本波自跑，全部在 scratch 目錄內；受管檔與 `build/` 皆未被寫入）**：把 pin 測試與 `csystem.cpp` 各複製一份到 scratch，對 seam ctor 加上**兩個只差一個 `/`**、`g++ -fsyntax-only` 都 exit 0、建構值**完全相同**（兩者 `iTesterType` 都變成 1）的變異：
      ```
      A:  ...iOutputJamCnt(0){ core.sLOTSTATUS = "/x";  iTesterType = 1; }
      B:  ...iOutputJamCnt(0){ core.sLOTSTATUS = "//x"; iTesterType = 1; }
      ```
      **字串感知**的剝除器（CODE/LINE/BLOCK/DQ/SQ 狀態機）：未變異 `PASS 46/46`、A `FAIL 1/46`、B `FAIL 1/46` —— 洞是關的。
      同一份測試，**只把該剝除器的 DQ/SQ 兩個狀態拿掉**（其餘一字不動）：未變異 `PASS 46/46`、A `FAIL 1/46`、**B `PASS 46/46`** —— 同一個錯誤，只因為字串裡多了一個 `/`，就從紅變綠。
    - **一般化（真正要記住的那一句）**：**任何以文字剖析當閘門的機制（pin、census、grep 清點、`.dfm`/`.rc` 產生器的 IR），只要它對字串與註解的詞法判斷不正確，就是一道剛好開在別人會踩的位置上的洞。** 這與 plan S10-20 提的「只過濾 `//` 會漏掉 `/* */` 區塊註解」是同一族缺陷的兩面：前者**少看**了註解，後者**多看**了不是註解的東西。寫這類閘門時，`//`、`/* */`、`"..."`、`'...'`、跳脫字元要一次做齊。

18. **「每個呼叫點都走到了」不等於「下對了命令」——軌跡/覆蓋型斷言完全不約束致動器的方向與極性（2026-08-01 本波自跑證實；對機台控制翻譯是安全相關的盲點）**：
    `tests/test_w7_l1_auto2.cpp`（`asendic_Auto2.cpp` 的落地測試）在 `f61e25e` 的版本有 46 條斷言（本波實跑，輸出 `RESULT: 46 passed, 0 failed`），其中代表性的一條字面寫著「trajectory covers 50,100,200,201,300,400,410,420 (all AutoCylinder\* call sites reached)」——它證明的只有「狀態機走到了那些 case」。
    - **實測（本波自跑）**：把 `asendic_Auto2.cpp` 複製到 scratch 變異、off-tree 編譯後 `ar r` 進 `libht9045_sm.a` 的**副本**再連結出獨立 exe（`build/` 內的封存檔未被改動），對同一支 f61e25e 測試跑三個彼此獨立的變異：

      | 變異 | 內容 | 結果 |
      |---|---|---|
      | M1 極性反轉 | 輸送帶 `SW[SwACAuto2]` / `SW[SwACAuto2CW]` 的所有 `On();` ↔ `Off();` 對調 | `46 passed, 0 failed` |
      | M2 方向對調 | 全檔氣缸 `.Push()` ↔ `.Pop()` 對調 | `46 passed, 0 failed` |
      | M3 動作刪除 | 刪掉全部 `MOT[...].ClearTray(__FUNC__);` 呼叫 | `46 passed, 0 failed` |

    - **M3 的數量就地更正**：交派給本波的敘述說是「四個 `MOT[].ClearTray()` 呼叫」；本波實際計數是 **19 個**（`asendic_Auto2.cpp` 全檔非註解行的 `MOT[...].ClearTray(__FUNC__)` 呼叫點），**19 個全刪仍然全綠**。結論不變，證據更強。
    - **為什麼綠得下去（本波逐條看過那 46 條）**：沒有任何一條斷言讀 `SW[...]` 或 `Cylinder[...]` 的致動器狀態，也沒有任何一條觀察 SUT 自己呼叫的 `ClearTray`——測試裡出現的 27 處 `ClearTray` 全是它自己的 fixture 清場；全檔僅有的 2 條 `.Off()` 斷言，對象都是計時器 `DoAuto2Delay`，不是輸出點（第 3 處 `.Off()` 在註解裡）。
    - **拘束性規則**：翻譯 `asendic_*` 這一族（以及任何運動/IO 序列）時，測試至少要對**每一個被命令的輸出點**斷言「哪一個點、被設成哪一個狀態、在哪一個 case」，而不是只斷言 case 走過。**軌跡覆蓋當起點可以，當驗收不行**——照 gotcha 11 的形狀，交付前先拿「反轉一個輸出點的極性」當 mutation 跑一次；不會紅，就表示這件事根本還沒被測到。
    - **狀態聲明（不冒充他人成果）**：上述量測是對 `f61e25e` 的**已提交**版本做的。本輪另有軌道正在補這些洞，其成果不在本條的複驗範圍內；本條記的是教訓與證據，不是完成度。

19. **`tools/dfm2rc/` 產生的三組語料「行尾慣例不一致」，而閘 G7 是逐位元組比對——補結尾換行時補錯 EOL 會把 G7 弄紅（2026-08-02 主迴圈修 gate 4 時實測，差一步就踩下去）**：
    - **背景**：plan S12 閘 4 要求每個檔以換行結尾，全樹長期紅著 **414** 個違規。根因單一且明確：**`json.dump()` 不寫結尾換行**。
    - **文件原本的歸因是錯的**：plan S12（`:902`）寫「其中 **404** 個是 `tools/dfm2rc/ir_out/*.ir.json` 產生檔」。實測分佈是 **`ir_out` 133 + `layout_out` 133 + `rc_out` 133 + `reports` 5 + 手寫 7 = 411**——是**三組**語料各 133，不是 `ir_out` 一組 404。**只修一個 emitter 會讓三分之二繼續紅**。真正的呼叫點有 **8 個**：`run_b1a.py`(2)、`run_b1d.py`(2)、`emit_layout.py`、`emit_rc.py`、`run_b1b.py`、`run_b1c.py` 各 1。
    - **⚠ 真陷阱**：這三組語料的行尾慣例**不一樣**，因為寫檔時 `open()` 的 `newline=` 參數不同——
      | 語料 | 寫檔方式 | 實際行尾 |
      |---|---|---|
      | `ir_out/*.dfm.ir.json`、`reports/*.json` | `open(p,'w',encoding='utf-8')`（預設 newline=None → Windows 換行轉譯） | **CRLF** |
      | `layout_out/*_events.gen.json`、`rc_out/*.rcmeta.json` | `open(p,'w',encoding='utf-8',newline='\n')`（關掉轉譯） | **LF** |
    - **為什麼會咬人**：`dfm2rc_idempotent`（閘 G7）會把 golden 重新跑一遍整條管線、和簽入的語料**逐位元組 diff**。若一律 append `b'\n'`，CRLF 那批的最後一行會變成孤零零的 LF，而修好的產生器重跑時寫的是 CRLF → **G7 立刻紅**，而且症狀（133 個檔全部 byte diff）看起來像產生器壞了，不像補換行補錯。
    - **正確作法（本次採用，已驗證）**：產生器一律加 `fh.write('\n')`（在預設 newline 下自然變成 CRLF、在 `newline='\n'` 下維持 LF，各自吻合該檔既有慣例）；一次性掃描既有檔時**逐檔判斷主導行尾**（比較 `raw.count(b'\r\n')` 與 `raw.count(b'\n') - raw.count(b'\r\n')`）再 append 對應的終止符。
    - **驗收**：閘 4 由 **414 → 3**（只剩 U+FFFD 三檔，見本檔上方「翻譯中文註解亂碼」一節）；四支 dfm2rc 閘測試全過，其中 `dfm2rc_idempotent` **Passed 23.36s**，其自產報告 `tools/dfm2rc/reports/b1d_idempotent_report.json` 內 `{"files": 133, "g7_diff_count": 0}`——**用報告內容證明它真的比對了 133 個檔、不是空跑**（該報告本身現在也以換行結尾，反過來證明被 patch 的產生器程式碼確實執行到了）。
    - **通則**：任何對「產生出來的語料」做的整批機械修改，都要先問「有沒有一支測試在對它做 byte-diff」，以及「這批檔的行尾/編碼慣例是不是一致的」。本專案兩個答案分別是「有（G7）」和「不是」。

## 接縫（HAL）與 64-bit 跨位元
- 即時硬體（運動/IO/互鎖/ATC）保留 native C++（既有已驗證 wrapper），翻譯後的 C++ **同程序直接呼叫**（無 P/Invoke、無 managed 邊界）。「interface 切割」＝抽象基底 + Sim/Real 子類。
- **64-bit 跨位元地雷**：若 64-bit Handler 與 **32-bit GPIB 橋**走 WM_COPYDATA，`MessageDef.h` 的 `MV`/`VM` 內嵌 `HWND`(行 259-260) 指標寬度會錯位 → 改 `DWORD`+`HandleToLong` 或移出；扁平 `M_V`(InterfaceSYS.h:222) 才安全。
- KeyPro：64-bit 需自家出 64-bit DLL；`HTKeyPro.h` 以值傳 `std::string` 跨界（綁 Borland RTL）→ 改純 C ABI(char*+len)。906 只用 `KEYPRO_GET_LEVEL`(純 int，C 安全)。

## 本機（CUSTOMER_CODE=838 CC_CSAMQ / GPIB recipe）實際啟用硬體 = 64-bit/接線範圍
- 運動：CONTEC SMC 41 軸 + Galil DMC index 4 軸（INDEX_MOTION_CARD=0 覆寫 SMC 馬達表，cinitial.cpp:3464，**待實機確認有無 Galil 卡**）。
- 數位 IO 389 點：MN200 MotionNet（IO_CARD_TYPE=2，mn_set/get_port_bit，**待確認獨立卡 vs 走 SMC**）。
- 溫控 Delta DTK4848(serial, HEATER_CTRL_TYPE=4)、震動 2 板(serial)、條碼(serial)、Tester=GPIB 橋(外部 32-bit, WM_COPYDATA)、sqlite3、KeyPro(自家 dongle, 只用 GET_LEVEL)。
- 本機關閉(出範圍)：Syntek/Aurotek/PCI-1203/1735U/CC-Link/EtherCAT/RFID-MR/Laser/AOI-CCD/ESD/AGV/TrayMap/FTP/Moxa(EJ1N 2020 改 TComm)。
- Tester 模式(54 recipe)：GPIB 50/RS232 3/TCP 1/TTL 0，Handler 端全可攜(green)。
- **⚠ 修正（2026-07-11 W5 recon）**：上面「CC-Link 出範圍」是**執行期設定關閉**，不是死碼——`TfCCLink` 表單仍會在啟動時**無條件被實例化**，部分方法無論設定為何都會跑一次；翻譯時不能因為「本機關閉」就當作可以整段跳過，仍需 `#if HAVE_CCLINK` 執行期 Sim 分支處理（同 IOBackend.cpp 既有 HAVE_MN200/HAVE_PCI1203 慣例），而非編譯期排除。
- **RFID-MR 是真死碼**（與上面不同）：2026-07-11 recon+主迴圈核實，`MR\` 目錄全部檔案(acatchcassette.cpp/RFID.cpp/Cassette.cpp/Delta.cpp)完全不在 `HT9045.bpr` 的 FILELIST 內——不是「設定關閉」，是根本沒編進這支 exe。除非未來有明確客戶需求，不排入任何波次。

## Log 語料位置（驗證/對拍來源）
- 大部分：`D:\HT9045_Log`（~3.0GB、9259 檔；csv/txt/xls）。
  - EventLog 告警/事件 CSV：`D:\HT9045_Log\EventLog\HT9045_EventLogBackup_*.csv`，欄 `No, UnitName, AlarmCode, Date, Time, Recovery, StopedTime, Duplicate, Message`（Big5）。
    - **格式由 9011UC 寫入端確認**：`SGDToCSV(grid, ",", ";", path)`(common.cpp:2050-2064，呼叫於 cObserver.cpp:2410/cMyDB.cpp:452)，DB-grid 匯出非 tail-append；欄分隔逗號+TAB；空欄字面 `\t`；**cell 內逗號被 escape 成 `;`(common.cpp:2059)→真實 Message 不含逗號**。另有不同格式的 live tail-append `EventLogTxt`(slEventLog, main.cpp:1503, MyStringList::MySaveToFileShareMode)。
    - 真實檔(2023_07，唯一現存)：3409 資料列、UnitName Process=3382/Motion=19/Message=8、含 MES2108、日期 2022-06-06~30。
  - 其他：EventLogTxt(INI 計數)、Test_TCPIP(TCP log)、Alarm.txt、各模組 csv/xls。
- SECS log：`D:\SECS_GEM_LOGS`（依年份；`SECSGEM_TextLog_*.txt`，`[Send]/[Receive]` trace+時戳）。
- **原則（使用者指令）**：log/行為格式不清楚時，務必檢閱 9011UC 寫入端 C++（handlerlog/csystem/database/cMyDB 等），勿臆測。原始碼是 source of truth，log 是驗證的尺。

> 參考：C++ 接縫分析見 `D:\HT9045\docs\migration\RD5軟體_HT9045_906_64bit遷移計畫_20260625_193240.md` §9（HAL、三道牆、硬體 64-bit 稽核）。

## 工具導入評估決策（2026-06-29）：CodeGraph / Superpowers → 不導入為常設工具

> 多角度評估（4 平行調查 + 對抗式 skeptic，交叉驗證）後的決議，記此避免日後重複討論。

**CodeGraph（符號/邊/呼叫路徑索引 MCP）：預設不導入。**
- 反對理由（皆有實證）：(1) golden 是凍結樹，全樹 grep 實測 ~0.45s 且已分好定義/讀寫點，索引省的是 sub-second，省不掉後面一定要做的「讀懂語意」；(2) 源碼 Big5，codegraph 逐字回傳會跟 Read 工具一樣把中文註解變 U+FFFD（本樹已實際中招 cprod.cpp 283 行 / cpublic.cpp 62 行），忠實翻譯仍須走 cp950-aware 讀檔；(3) tree-sitter 不跑前處理器，`__property`(18 檔，`={read=...}`→ERROR node)/`__fastcall`(363 檔)/`__closure` 的解析風險「恰好集中在價值最高的 VCL form 檔（如 cContact）」，殘缺/錯接的圖比沒有圖更糟（在安全關鍵翻譯上給假信心）；(4) SM 之間是 predicate(IndexHasIC… ~312 處)+全域資料流耦合，符號圖抓不到資料流語意、不比 grep 強；(5) nm 稽核是 link 層真相，source 圖取代不了。
- 唯一可選動作（需使用者授權 `codegraph init`，預設不做）：對 golden 一個純邏輯子樹（ainarm9045.cpp + headers）+ 一個 `__property` 重 header 做時間盒 spike。採用門檻高：符號/邊須明顯勝過 grep 且 `__property`/`__closure` 不漏接成員；任一不過即丟棄；只索引凍結 golden、絕不索引會 stale 的新樹。

**Superpowers（obra/superpowers 技能集）：跳過大宗，維持現狀。**
- 已裝 3 個通用方法論技能（brainstorming / systematic-debugging / writing-plans，均已 defer 到本專案 CLAUDE.md；systematic-debugging 已內含「只在實機驗證」caveat）。其餘多為冗餘或不合本專案約束：`executing-plans`/`verification-before-completion` 已被「每波 ReadPlan→Translate→Verify→commit + 5 道閘 ratchet + DEFERRED 表」更強實現；`test-driven-development` 前提反了（golden 已是 spec、refactor 會偏離 golden、無實機 binary）；通用 `subagent`/`dispatching-parallel-agents` 不懂 ODR/nm 邊界，比本專案三段式（平行翻→序列整合→clean verify）更不安全；code-review/writing-skills 與內建 `/code-review` 及 bespoke `skill-creator` 重複。
- 注意：`defense-in-depth`/`root-cause-tracing`/`condition-based-waiting` 不是獨立技能，已內含於 systematic-debugging，勿當名字裝。

**真正標的不是導航效率，是覆蓋率/驗證深度（兩工具皆無解）**：W7 的 `#if 0` gated 大宗（csystem MainProc ladder ~2000 行、DoOneCycleFinishCheck/DoCleanOutFinishCheck ~2900 行、cContact 22761 行 index SM、W5 comms ~22k LOC）從未端到端在真實 HAL pump 過。降風險只能靠「盡早接真實/半真實 HAL pump gated 主路徑 + 持續 golden-cited oracle」。下一階段規劃重點應從「再多翻 site variant（純機械 clone）」挪向此。

## vclcompat CommaText：對格式良好檔=VCL 全等；對壞檔刻意不 bug-for-bug（20260818）

FW-3 cObserver recon 判定（vclcompat/TStringList.cpp:163-225 vs BCB6 classes.pas）：

- **編碼端（GetCommaText）**：欄位含空白/逗號/引號就補雙引號——與真實 VCL 一致。
  golden 的 EventLog 寫入端（cMyDB.cpp/handlerlog.cpp 的 AddTextWithLineNo(SL->CommaText)）
  走這條，所以**本系統自己寫出的 CSV 讀回必然全等**。
- **解析端（SetCommaText）**：真實 VCL 對未加引號 token 是 `P^ > ' '` 掃描——
  **空白本身是分隔符**（甬矽案「整列右移」的根因，見記憶
  ht9045-eventlog-csv-quoting-load-bearing）。vclcompat 只在逗號切斷、
  頭尾裁空白——**不重現右移 bug**。
- **後果**：對歷史累積、可能夾雜未加引號列的機台 EventLogTxt，V906 web 檢視
  與 V899 VCL 畫面會顯示不同的欄位對齊（V899 右移出包、V906 完整保留）。
  這是**刻意的健壯化**，不是缺陷；若有客戶要求 bug-for-bug 重現右移，
  要單獨開波並問過使用者（預設不重現）。
- 適用面：cObserver GetEventLogText 三個 CommaText 讀點（golden :3848/:3892/:3927）
  ＋StatisticalJamCount（:5131）。

20. **`ctest` 的 exit code 對「失敗集合」零鑑別力，而它長得像一個確認訊號（2026-08-27 踩到，已工具化）**：
    `ctest` 對「有任何測試失敗」**一律回 exit 8**，不論失敗的是 5 個還是 7 個。
    本樹的 gate 腳本把它寫進 sentinel（`echo "G_EXIT=$?" > _<tag>_gate_g.txt`），
    結果 **w23/w26/w28/w29/w30/w31/w32（真的只有常駐五項失敗）與 w34（Debug 7 個、
    Release 6 個）的 sentinel 逐位元組相同**——都是 `G_EXIT=8`。
    **後果是實際發生的**：W34 在紅燈上 commit（`build_w34g` 135/142、`build_w34r` 136/142，
    多出 `dfm2rc_fidelity` 逾時與 `ShowBinSelectCore` SEGFAULT），而 commit `5d85337`
    與 DEVLOG 都寫成「各 137/142、失敗集合逐項相同」。
    **政策裡「比對失敗集合逐項、不看數字」那條規則就是為了擋這個**，但只要 sentinel 還
    給得出一個看起來像確認的 8，紀律遲早會輸。
    **處置是工具化，不是提醒自己下次記得**：`tools/gateverdict.sh <tag>` 抽失敗集合、
    與常駐五項比對，印 `*_FAILSET`/`*_EXTRA`/`*_VERDICT`，RED 回 1、GREEN 回 0；
    `tools/dualgate.sh` 的 sentinel 已改成把它的輸出一併寫入。
    **附帶一個同型的自我提醒**：`gateverdict.sh` 第一版自己就有同一個缺陷——
    ctest.log 還在寫的時候沒有「The following tests FAILED」區段，抽出來的失敗集合是空的，
    `EXTRA` 空 → 判 GREEN。已加守衛：沒有 `tests passed` 摘要行就判 INCOMPLETE。
    **任何「從 log 推導通過與否」的工具，都要先證明自己分得出「沒有失敗」與「還沒寫完」。**

21. **括號平衡切函式 span 時不剝註解，註解裡的一個 `{` 就會靜默吞掉後面數十支定義（2026-08-27 踩到，已加旁證工具）**：
    `tools/census/census.py` 的 `functions()` 用括號平衡切 span，**但不剝註解與字串字面值**。
    port `Command.cpp:1228` 有一行 `//2013.01.24 Q_Q TSMC GPIB COMMAND Part 2. {`
    ——那個 `{` 讓深度永遠多 1，`TfMain::PERSITETemperatureStrings` 的 span 從
    `1218..1249` 變成 **`1218..5219`（4,002 行）**，吞掉後面 **61 支定義**。
    被吞的名字不會進 `pnames`，於是 census 把它們判成「golden 有、port 沒有」＝未翻譯，
    憑空造出 **43 支 / 813 行的假缺口**。
    **後果是實際發生的**：主迴圈照那份清單派了一整波翻譯 agent，agent 查完回報
    「這 36 支早在 20260817 的 FW-3 Wave A 就翻完了，本波零變更」。
    而那個 `{` 是**忠實翻譯的產物**——golden 自己就這樣寫。
    **失敗完全是靜默的**：`defn_probe` 有認出被吞函式那一行，不是探針失敗，是 span 覆蓋。
    量到的邊界（A/B：同一個 `functions()`，差別只有先不先剝註解）：
    **port 側 3 檔受影響、只有 `Command.cpp` 有實際後果**；golden 側 9 檔、281 支被吞
    （`main.cpp` 239→382、`cContact.cpp` 103→139、`Magazine.cpp` 21→50）。
    **分母沒被污染**——`gcode` 是 `code_lines()` 另外算的，不經 span；
    被扭曲的只有分子裡的 per-file 缺口清單。
    **處置刻意不是修 `functions()`**：改它會讓 golden 與 port 的所有 span 一起變、
    歷史數字全部失去可比性，而且上一次「修好」census 的嘗試算出過 parked > gcode
    與 35,289 行的假 credit。改成新增唯讀旁證工具 `tools/census/span_sanity.py`，
    **選波次標的前必跑**（單檔模式會列出被吞的定義名與行號）。
    **通則**：任何用括號平衡切原始碼區段的工具，都必須先剝註解與字串——
    這與 gotcha #17（註解剝除器必須認得字串字面值）是同一個家族的另一面。
    **並且：census 的「缺」不是待辦清單。**

## Advantech Common Motion（PCI/PCIE-1203）兩個實測陷阱（20260818）

1. **MinGW 開 `HAVE_PCI1203` 必加 `-D_STDCALL_SUPPORTED`**：AdvMotApi.h 只在
   `_MSC_VER>=800 || _STDCALL_SUPPORTED` 時定義 `ADVCMNAPI=__stdcall`，MinGW 兩者皆無
   → 每個 Acm_* 宣告解析失敗（error: expected initializer before 'Acm_...'）。
   至今沒踩到只因 HAVE_PCI1203 從未真的開過。
2. **無卡（且未註冊虛擬卡）時 `Acm_GetAvailableDevs`/`Acm_DevOpen` 阻塞 ≥15s 不返回**
   （實測 timeout 124 殺掉）——與「批次不得 modal」同族的批次殺手，任何開發機
   smoke/probe 必包 timeout；SDK 有官方虛擬卡（Adv_V_PCI1203，Common Motion Utility 註冊）。

其餘（HAL 已存在、版本子集判定、呼叫序列、啟用步驟）見 docs/RECON_PCIE1203_CommonMotion.md。

## Galil gclib 兩個實測陷阱（20260818）

1. **64/32 位元數不合的連結失敗偽裝成「符號不存在」**：32-bit MinGW ld 對讀不懂的
   x64 archive member 靜默跳過 → `undefined reference to GOpen`（不是 format error）；
   32-bit nm 對 x64 庫報 File format not recognized。查位元數用 `file`（DLL）＋
   `ar x` 抽成員讀 COFF machine（0x8664=AMD64）。本機這份 gclib 全套 64-bit only。
2. **gclib 官方文件對舊世代 DMC32/DMCCOM 隻字未提**（doc/html grep dmc32/migrat 零命中），
   `gclib_compat.h` 是 gclib 自己新舊 API 的相容層、對 906 用的 7 個 DMC* 函式覆蓋率零
   ——遷移對照表只能自己維護（docs/RECON_GALIL_gclib.md §3）。

另：MSVC 已於 20260817 自 build.bat 移除且本機無 cl.exe → 目前專案沒有任何工具鏈能連
64-bit 函式庫；要動 gclib（或任何 x64-only SDK）先解 64-bit 工具鏈前置。

## MN200：MN200DLL.lib 是 COFF 不是 OMF——CMakeLists 舊註解搞混兩個檔（20260818）

CMakeLists.txt:50-53 寫「shipped MN200DLL.lib 是 32-bit OMF、MinGW 無可用」——**實測推翻**：
`Lib/MN200DLL.lib` 是 MS-COFF import library（`!<arch>` magic），MinGW.org 6.3 的 nm
列得出全部 906 用的 35 個 `_name@N` 符號，探針編譯+連結 exit 0。真 OMF 的是
**`MN200BCB.lib`**（`f0 0d` magic，BCB6 專用）——兩檔被混為一談。
**但 offline stub（vendor_offline_motionnet.cpp）仍有獨立價值**（無卡可建置、免執行期
vendor DLL 依賴），真連結可行≠該退役 stub；改註解/接真 lib 屬設計取捨，動之前讀
docs/RECON_MN200_PISO.md。另：x64 SDK 與 x86 同批同版（1.0.18.1，exports 各 192）。

## census 的 dict key 讓 **C++ 重載塌陷**，golden 分母被靜默低估（20260827，FW3-ATC1 撿到）

`census.py` 的 `functions()` 把結果存進以 `Class::name` 為 key 的 dict。
**C++ 重載集因此只留下一個**——同名、不同參數的兩個真實本體，會被算成一個。

實例（波次 agent 先發現，主迴圈獨立驗證）：
golden `ATC/ATC_Handler_Side.cpp` 定義了兩次 `ChangeRecipe`——
`:1920` 的 `ChangeRecipe(AnsiString)` 與 `:1927` 的 `ChangeRecipe(AnsiString, double)`，
**兩個都是真實獨立本體**。`functions()` 只回一個 key（保留 `:1927` 那個），
於是回報 **168 支**，而欄位 0 的定義行實數是 **169**。

**這是 census 第五種量尺缺陷，也是第一個低估 golden 側的。**
前四種（註解裡的括號吞併／刻意寄放他檔／檔名比對/本體搬到別的 port 檔）都是低估 port 側。

### 影響範圍要講精確

- **對 census 自己的百分比**：影響**大致相抵**——重載在 golden 與 port 兩側都會塌陷，
  分子分母一起少。**但只要有一個重載被翻、另一個沒翻，就不再相抵**，
  而且那正好是最需要量準的情況。
- **對波次分母**：**一定低估**。派工單寫「X/168」而真值是 169。

### 處置

`tools/census/wave_preflight.py` 的 `[1]` 區塊加了 overload check：
剝註解後數欄位 0 的 `Class::name(` 定義行，同名出現多次就報出行號，
並直接印出 `TRUE DENOMINATOR IS n BODIES, not m`。

**census.py 一樣不動**——改 `functions()` 會讓所有歷史數字失去可比性，
而上一次「修好」census 的嘗試算出過 parked > gcode 與 35,289 行的假 credit。

### 回歸結果（有價值，不是形式）

拿 overload check 回頭掃先前三波的 golden 檔：
`AutoClean/uCleaning.cpp`、`OCR.cpp`、`HS_Function.cpp` **各 0 個重載**。
所以已 commit 的分母 **72／88／66 全部不受影響，不需要回溯更正**。

## `gate_depth_map` 只模型化 `#if 0`，**不評估巨集**（20260827，波次開工工具化時撿到）

census 的 `gate_depth_map` 回答的是「這一行在不在 `#if 0` 之類的死區塊裡」。
它**不會**去判斷 `#ifdef SOMETHING` 裡的 `SOMETHING` 到底有沒有被定義——
那需要前處理器。所以拿它當「這一行是不是活的」會在一個方向上系統性地說錯：
**`#ifdef <未定義巨集>` 內的碼會被判成 live。**

實例（`tools/census/wave_preflight.py` 第一版就中）：
`csystem.cpp:17614` 有 `fCleaning->btnResetCleanCountClick(fCleaning);`，
而 `forms/fCleaning.h` 根本沒有這個成員——照理該是連結破。
真相是它包在 `csystem.cpp:17612` 的 **`#ifdef SOFT_SIMULTE`** 內，而
**`SOFT_SIMULTE` 在 V906 build 沒有定義**，所以整段是死碼，樹當然照編。

`SOFT_SIMULTE` 未定義的證據（三條獨立，20260827）：
1. 全樹 `#define SOFT_SIMULTE` **零命中**（Grep 工具，排除 `build*`）。
2. `CMakeLists.txt` 與 `build.bat` 都沒有定義它。
3. **編譯器親口說的**——`#include "MachineType.h"` 後接
   `#ifdef SOFT_SIMULTE / #error`，用標準旗標組 `-fsyntax-only` 編過，exit 0。

第 3 條才是權威。前兩條是 grep，而 grep 找到（或找不到）字串只證明文字，
不證明前處理器怎麼想——這正是既有原則 #4 的另一面。

**處置**：`wave_preflight.py` 加一個 `KNOWN_OFF` 集合（目前只有 `SOFT_SIMULTE`，
帶上述證據），並把「工具不評估的其他 `#if`」單獨列成 CONDITIONAL 一欄誠實交代，
**不假裝評估**。往 `KNOWN_OFF` 加東西必須附同等級的編譯器證據——
一個其實有定義的巨集被加進去，就會往反方向靜默說錯。

> 記憶裡本來就有「`SOFT_SIMULTE` 是全機模擬旗標、正式 build 關閉、
> 被 `#ifdef SOFT_SIMULTE` 包住的功能在實機被編譯掉」這一條。
> 這次的新東西是：**我們的量測工具也會被它騙**，而且騙的方向是
> 「把死碼報成必須支援的 live 需求」——那會讓下一波去做不該做的事。

## MOTION_IO 是兩家 vendor 搶名（HAVE_PCI1203=1 必炸，20260818 診斷）

**症狀**：`-DHAVE_PCI1203=1` 編 cinitial.cpp → `conflicting declaration typedef
struct MOTION_IO`。**根因不是本樹的碼**：ICPDAS `Motor/vendor/MN200.h:221`
（`typedef struct _MOTION_DEV_IO {...} MOTION_IO, *PMOTION_IO;`）與 Advantech
`EtherCAT/vendor/AdvMotDrv.h:2594`（匿名 struct 同名 typedef）都在全域宣告
`MOTION_IO/PMOTION_IO`。任何同時看到兩份 vendor header 的 TU 都會炸——
cinitial.cpp 經 `Motor/myMN200motor.h:90`＋`Motor/myEthercatmotor.h:69` 同時掛到。
預設組態（HAVE_PCI1203=0）不受影響。

**修法屬 1203 HAL 設計輪，不在此先斬**（與「MN200 保留 vs 併入 EtherCAT」的
使用者裁決糾纏）：
1. **pimpl seam（正解候選）**：wrapper header（my*motor.h）不再 include vendor
   header，vendor 型別只活在 wrapper .cpp——HAL 本來就該長這樣。
2. **改名巨集（快解）**：在 myMN200motor.h include vendor 前
   `#define MOTION_IO MN200_MOTION_IO`＋`#define PMOTION_IO MN200_PMOTION_IO`，
   include 後 #undef。vendor 檔不動；限制＝跨 wrapper 傳 MOTION_IO 的碼要跟改。
3. namespace 包 include：C linkage 下不可靠，不建議。

順帶量到：cinitial.cpp:4525 起有一批 `iPortID`（unsigned int）塞進 `BYTE`
陣列的 -Wnarrowing 警告，golden 同型（BCB6 不告警），非本診斷範圍。
## census 量不到 FW facade 的進展——這是刻意的，**不要去「修」**（20260828，TIF1+DTL1 後撿到）

**量到什麼**：在 HEAD `f30cd06`，也就是 FW3-TIF1 與 FW3-DTL1 交付 **14 個新 facade 檔、
約 5,400 行**之後，`census.py` 的六個數字與兩波之前（`6dd26d1`）**逐項完全相同**：
非表單 322,536/336,509＝95.8%；表單 45,292/261,862＝17.3%；全案 367,828/598,371＝61.5%。
連 `files with NO port mirror: 2 non-form / 88 form` 都沒動。
`census.py --detail` 的輸出裡 `forms/f` 的命中數是 **0**。

**原因**：census 用**檔名**配對 golden↔port，而 FW 的 facade 刻意叫 `forms/fXxx.{h,cpp}`，
不是 golden 的檔名。這個檔名比對的保守是**有量測背書的刻意選擇**——先前試圖「修好」它的那次，
算出 parked > gcode 並假記了 35,289 行。**不要動 census。**

**操作上的後果（這才是重點）**：**census（第一軸）結構上量不到 FW 戰役的進展。**
拿它當 FW 的進度數字，不管交付多少都會回報「沒進展」。
FW 的軸是**每個表單的覆蓋**（ACTIVE 成員數／golden 成員數，以及 live golden span 行／span 行），
那是每一波回報的那組數字。「三軸不可互相換算」這條規則，在這裡以新的形狀咬人。

**我自己做錯的事**：這幾波的每一次交接，我都引用 census 當作進度指標。
它不是——它是 PT 戰役（鏡射式翻譯）的軸。兩個戰役共用一棵樹，但不共用進度尺。

## 編譯加速：ccache + PCH 的七個實測事實（20260921，FASTBUILD）

起因：使用者拿 BCB6 那邊的 bcc32pch 外掛（Hausladen 的 C++ Compiler Enhancements 2.79）問
「906 也能自製嗎」。答案：它做的四件事（少 parse／少 I/O／跳過相依檢查／搬出 process 批次化）
在 CMake+GCC 上全是標準品；這棵樹以前**一個都沒開**。落地在 `cmake/W906_FastBuild.cmake` ＋
`tools/pch_eligibility.py`，由 `CMakeLists.txt` 一行 `include()` 掛上，`build.bat` 多了
`V906_BUILD_DIR / V906_GENERATOR / V906_CMAKE_ARGS` 三個環境旋鈕並自動偵測 ninja/ccache。
量測數字見 DEVLOG 20260921 條目。

1. **問題的形狀**：697 個物件；中位數 TU 前處理後 ~6.5 萬行，自己只有幾百行（`Config.cpp` 57 行 → 68,764 行）。
   `cmydef.h`／`cprod.h`／`MachineType.h`／`vclcompat/vcl_compat.h` **每一個單獨**展開都是 4.3–5.9 萬行
   （大宗是 windows.h＋STL＋vclcompat 傘），所以同一批文字每次 clean build 被 parse ~700 次。
2. **PCH 不改變機器碼**：單 TU 實測 `objcopy -g` 後 `nm` 相同、`objdump -d` 相同，只有 `.debug_line` 檔名字串不同。
   全樹的逐 .obj 比對見 DEVLOG。
3. **哪些 TU 能吃 PCH由掃描器每次 configure 重算**，不是寫死清單。規則兩條：(a) 本來就（間接）含到全部 PCH 標頭；
   (b) 沒有 `#define/#undef/#pragma push_macro` 早於後面某個 `#include`。(b) 抓到 22 支——`csystem.cpp` 用
   push_macro/#undef 圍 `common.h`、`cinitial.cpp` 在 include 前設 `PT_N3_HAVE_iShuttleTempPos`、
   `adam6024.cpp` 需 `_USE_MATH_DEFINES` 先於 math.h——PCH 會在 TU 自己的巨集存在之前就把標頭展開完。
   掃描是文字的，只可能**多判**（把編譯器沒看到的 include 算進去），所以有 `--audit <無PCH的 build dir>`
   對 `compiler_depend.internal`／`ninja -t deps` 逐 TU 驗：20260921 268 個判定可用、0 over-claim。
   ⚠ audit 只能對 `-DW906_PCH=OFF` 的 build dir 做——有 PCH 的 dir 裡每個 TU 的相依都會經由 cmake_pch.hxx 含到那些標頭，檢查會恆真。
4. **ccache 對 PCH TU 要能快取，sloppiness 必須 `pch_defines,time_macros`**（四個變體逐一測；沒設就全部 uncacheable；
   `-fpch-preprocess` 不需要）。`W906_FastBuild.cmake` 在 configure 時用 `ccache --set-config` 寫進使用者設定
   （另設 `max_size=20G`、`base_dir=<樹>`）。代價：`__DATE__/__TIME__` 的三支 TU
   （`cObserver.cpp:522`、`WebBridgeTags.cpp:755`、`TempCtrl/TriTemp.cpp:599`）可能拿回第一次編譯的日期——
   增量 make 本來就有同樣陳舊性，ccache 把它延伸到 clean build。要可信的出貨時間戳：`-DW906_CCACHE=OFF`。
   ⚠ 測 ccache 一定要用 CMake 真實的 `-include <絕對路徑>` 形狀；用相對路徑它找不到 `.gch`，會產生**假 hit**。
5. **GCC 的 `.gch` 是記憶體映像、非確定性**（同目錄重產兩次 31 MB 差 13 MB）且含絕對路徑 ⇒ PCH TU **跨 build dir 永遠 ccache miss**。
   dualgate 每波開新目錄，所以兩個工具分工：PCH 加速 ~53% 的 TU，ccache 讓另外 ~47%（含 148 支測試 exe）近乎免費。
   同 dir 只要 `.gch` 沒重產（PCH 標頭沒變）就能命中。
6. **Unity/jumbo build 刻意不做**：這棵樹有兩個活的 ODR 陷阱（`uPlateInfo`、`TMyKitSuck`）＋ TU 內沒 `#undef` 的巨集。
7. **`ranlib.exe: unable to rename 'libX.a'; reason: Permission denied`** 是 MinGW-on-Windows 的防毒暫時鎖檔 flake，
   整個 `cmake --build` 因此 exit 1；重跑即可。做 timing 的那一輪要作廢重來。

**關掉的方法**：`cmake -DW906_PCH=OFF -DW906_CCACHE=OFF <build dir>`；新 dir 用
`set V906_CMAKE_ARGS=-DW906_PCH=OFF -DW906_CCACHE=OFF` 再跑 `build.bat`。
`tools/dualgate.sh` 不用改，它自動繼承（仍是 MinGW Makefiles、-j8）。

**工作流程上踩到的兩個坑**（記憶亦有）：改 `build.bat` 時同儕 session 的 `build.bat gate` 正在跑——
cmd 按位元組偏移逐行讀批次檔，執行中改檔會讓它從錯的位置繼續；以及共用工作樹上做 before/after 量測不可靠
（同儕邊改邊 commit），改用 `git worktree add --detach` ＋ sparse-checkout 只取 V906 子樹釘在一顆 commit 上量。

## vclcompat IniFiles 與 BCB6 在「寫回檔案的位元組」上的四個差異（20260924，夜間迴圈量測）

第 34 行那段摘要講的是語意（讀值對不對）；這一節講**寫回磁碟的位元組**。在真機上這是看得見的：
配方目錄有 `<md5>.MD5`（golden `SetMD5ByFolder`／`CompareMD5ByFolder`，這台 `bN20_CheckMD5=1`）。

| # | 差異 | 狀態 |
|---|---|---|
| 1 | **TMemIniFile 的版面**：BCB6 `UpdateFile` → `GetStrings` 在每個區段後補一個空行（連最後一段），vclcompat 原本不補 ⇒ golden 寫出的配方檔（UdUld.Data、Tray.Data…）讀完再寫回會掉空行 | ✅ 52b7618 修好（只動 Mem 版）；往返測試：原檔每段後有空行的 4 個配方檔變成逐位元組恆等 |
| 2 | **write-through TIniFile 的寫法**：BCB6 `WriteString` = `WritePrivateProfileStringA`（就地改一行、讀磁碟當下內容）；vclcompat 原本每次寫都用建構時的映像整檔重建（trim 鍵縮排、丟註解，且回捲別人寫的東西） | ✅ 382eaf1 合併分支 `t4-inifmt2`（008e309＋常駐 ctest `IniFiles_Win32Diff` 540980f；使用者 20260924 06:2x 裁決合併） |
| 3 | **TIniFile 的讀取**：Win32 每次讀磁碟；vclcompat 讀建構時的映像 ⇒ 別人（瀏覽器）改的值要重啟才看得到 | ✅ A5（使用者 20260924「按照舊版作法」，`W906-A5-INIREAD`）：write-through TIniFile 每次 Read*／ValueExists／SectionExists／ReadSection(s) 都重讀磁碟，讀取規則照 GetPrivateProfileStringA 實測（見下）；常駐 ctest `IniFiles_Win32ReadDiff`。TMemIniFile 不變（讀一次）。成本：wb_serve 開機約 5,400 次 Read*、閒置每拍 0 次、START→PAUSE 約 790 次 |
| 4 | **TMemIniFile 解構時無條件 flush**（`IniFiles.cpp:370-375`）；golden 的 TMemIniFile 解構不寫檔 | ⏳ 沒做。拿掉要先盤點「靠解構寫檔」的移植碼；`WebBridgeRecipeDoc.h:48-58` 已為它繞路 |

**Win32 WritePrivateProfileStringA 量出來的規則**（用 Python `ctypes.windll.kernel32` 對 scratch 檔直接呼叫；隨機差分 15,000 筆 0 差異）：
修改＝保留到 `=`（含）為止的原文、其後換成值；新鍵插在該區段**最後一個鍵行**之後（空區段在標頭後）——例外：該區段是最後一段且檔案最後一行是空白 ⇒ 接在檔尾；
新區段接檔尾、前面不空行；新產生的行一律 CRLF（LF 檔也是）；檔尾永遠補成 `\n` 結尾；檔尾「沒有終止符的純空白行」先丟掉；
`;` 是註解、`#` **不是**；區段／鍵比對 trim＋不分大小寫；重複區段寫第一個；極短的 LF 檔可能被 Windows 的 IsTextUnicode 誤判而改寫成 UTF-16。
⇒ 模仿任何 Win32 薄包裝時，**用 ctypes 量真 API，不要憑記憶寫規則**（這些規則有一半跟直覺相反）。

**量「程式有沒有改真實檔」的方法**：`tools/realfile_guard.py snap` → 跑 → **`check`（還原前）** → `restore` → `check`。
只在還原後 check 只證明還原成功 —— 20260924 因此把「開機會重寫 Tray.Data」誤寫成「讀檔沒有改寫配方」。

**Win32 GetPrivateProfileStringA 量出來的讀取規則**（A5，20260924；ctypes 對真 kernel32，3,300 個隨機檔、369,600 次讀＋29,700 次列舉 0 差異；
程式在 `vclcompat/IniFiles.cpp` 檔尾、常駐 ctest `IniFiles_Win32ReadDiff`）：
行尾 CRLF／LF／單獨 CR 都算；trim ＝去掉兩端**所有 0x01–0x20**（0xA0 不算空白）；區段／鍵比對不分大小寫且**逐位元組**（Big5 trail byte 0x41–0x5A 也被折疊，實測）；
**只看第一個**同名區段（後面重複的區段對讀取不存在）；鍵行＝trim 後含 `=` 且不以 `;` 開頭（`#` 不是註解），鍵＝第一個 `=` 前 trim；沒有 `=` 的行不是鍵；
重複鍵取第一個（列舉兩個都列）；值＝`=` 後 trim，再去掉**一對**成對引號（`"…"` 或 `'…'`，長度 ≥2）；行內 `;` 不是註解；
找不到 ⇒ 預設值去掉尾端**空格**（只有 0x20，tab 保留）；結果截到 nSize−1（BCB6 ReadString 的緩衝 2048 ⇒ 最多 2047）；
列舉（key 或 section 為 NULL）：名稱各接一個 NUL，**只有 list＋2 ≤ nSize 才完整**，否則截成 nSize−2 位元組（最後一個名稱可能被截半；被截斷的雙位元組字的 lead byte 變 NUL，ACP 950 實測；值與預設值則照位元組截）；區段列舉**含重複**；鍵列舉含空鍵名與重複鍵（小 nSize 另掃 5,300 檔／26,500 次列舉 0 差異，其中 1,500 檔含 Big5 名稱）。
BCB6 那一層（Delphi 6 IniFiles.pas，**憑記憶轉寫、本機沒有 BCB6 原始碼**）：ReadSection／ReadSections 用 16384 緩衝＋`while P^ <> #0`（第一個空鍵名就停）；
`ValueExists`＝ReadSection＋IndexOf（不分大小寫、Ident 不 trim）；`SectionExists`＝ReadSection 筆數 > 0 ⇒ **沒有鍵的區段＝不存在**；
ReadInteger／ReadFloat／ReadBool 是解析 `ReadString(…, '')`，**不是** GetPrivateProfileIntA（實測兩者不同：`abc`→0、`12abc`→12、`$1F`→0）。
真實檔對兩個上限的普查（system／config／IniData／CFG 共 1,562 檔，唯讀）：最大鍵列舉 14,782 位元組（`HandlerCondition.Data` 的 `[laser]`）< 16,384；最長值 1,791 < 2,047。
瀏覽器的配方文件（`WebBridgeRecipeDoc.cpp` RecipeDocToJson）刻意**不**跟著改：它改用一次性的 TIniStore 快照，維持原本「`raw` 逐字、`#` 是註解」的對外契約；要不要讓網頁顯示 Win32 讀到的值，是使用者的決定。
