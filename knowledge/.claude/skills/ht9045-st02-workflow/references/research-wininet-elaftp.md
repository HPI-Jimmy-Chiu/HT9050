# 調查：ELA 上傳用 WinINet 的 ElaFtp 設計（St02-M 20260927，唯讀調查，未改任何檔）

> 裁決背景：ELA 的 FTP 傳輸＝**WinINet**（Steven 20260927）；連不連照 config 開關（#22 D-f）；ctest 一律假傳輸；上傳要驗證＋錯開＋開機補跑；報表 UTF-8。
> 待裁決（已交 ST01-M 統整）：模擬組態要不要一律只用假傳輸（見 §3「SIM build rule」）；N25-3／N10 的 Passive 預設。
> **實作狀態（20260927，St02-E）**：R4 照這份做在 `v906/steven-elaftp-wip`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaFtp.*`、`ElaFtpWinInet.cpp`、`ElaChipMos.*`、ctest `ELA_Ftp`）。跟這份不同的地方：Passive 暫定 active（golden TfFTP 不設；一行開關 `ElaFtp.h:57`，待 Steven），N25-3 與 N10 共用它（沒讀 `bN10FtpPassive`）；`ElaSchedule`（R5）還沒做，R4 只有退避純函式；FTP_Log 在 `ElaFtp.cpp`（`ela::FtpLog`）。
> 行號：Rev891 = `D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\`；906 = `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\`；V906 = `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`。

## 1. 各報表實際用到的 FTP 動作

**N25-3 Jam log**（Rev891 `uChipMosZHUBEI_Func.cpp` `N25_UploadJamDataToFTP` :184-255，由 `UploadJamCode` :328-350 呼叫，`Analyzer.cpp:161-166` 分派）
- Connect：帳號／密碼／主機用 N25-2 設定（:202-204；ini 鍵 `Analyzer.cpp:3122-3124`）；`TfFTP.cpp:53-81`，`Vendor=NMOS_AUTO`（:55）、`TimeOut=5000`（:56），**不設 Port／Passive**（用元件預設）。
- CWD，失敗就 MKD：`TfFTP.cpp:144-171` 的 `ChangeDir`（CWD :150、MKD :162）；一次建一層、絕對路徑、之後不 CWD。三次：base（:216）→ base/<hostname>（:218）→ base/<hostname>/BackUp（:221）；`<hostname>`＝`gethostname`（`Analyzer.cpp:429-432`）。
- STOR ×2：`Upload`（`TfFTP.cpp:392-423`）先再 ChangeDir（:402），再以完整遠端路徑 STOR（:414）。主檔 :225；BackUp 複本 `Jam Rate_yyyymmdd_hhnn.txt` :235（檔名 :200）。
- 完成記號：整個呼叫結束後寫 `HadUpload.txt`（:346-349）；BackUp 失敗不擋（`bReturn` 被覆蓋）。
- Close：`TfFTP.cpp:490-512`。Handler 觸發在 906 `main.cpp:21442-21447`，位於 `#ifndef SOFT_SIMULTE`（:21425-21459）內。

**N10 BYFILE**（`UploadByFile_N10` :392-418 讀 `UploadFile.csv` :396，每列一次 `N10_UploadDataToFTP` :402-408）
- 用 N10 的帳號／密碼／主機（:148-150）；分析器**忽略** N10 的 port 與 passive 設定。
- 每次上傳寫一行 FTP_Log（:151；`MyStringList.cpp:141-156`；log 路徑 `Analyzer.cpp:220-224`）。
- `ChangeDirectories`（:158，`TfFTP.cpp:83-142`）：每層 NLST（:103）、MKD（:120）、CWD（:131）。**golden bug**：:120 對父層 `sCurr` 做 MKD，不是新資料夾。然後 `Upload`（:161）。

**範圍內都沒用到**：DELE（`TfFTP.cpp:425-455`）、RNFR/RNTO（:195-228）、下載（:235-385）、REST。

**相關 golden FTP 設定**
- Handler 自己的 N10 FTP：30 s timeout＋`bN10FtpPassive`／`iN10FtpPort`（906 `HS_Function.cpp:2133-2142`），預設 passive 關（0）、port 21（`cConfiguration.cpp:3377-3378`）。
- Handler 明確設的地方都是 `Passive=true`：`KYECFTP/FTPClient.cpp:119,411,1273,3846,4467,4561,4886,5276`；`ProductionInfo/TfFTP.cpp:26-27` 預設 passive=true、port=21。元件真正預設值查不到（本機沒有 `NMFtp.hpp`）；到處明寫 `=true` 暗示預設是 false，Jimmy 的註解（`KYECFTP/MiniFtpEngine.h:52-63`）推測是 true。

**UploadProdLog（N17，W13）不是 FTP**：是把檔案 `CopyFile(src, dst, bFailIfExists=TRUE)` 到 `asN17ProductionLogPath` 資料夾（906 `Command.cpp:12401-12447`，CopyFile :12431，資料夾檢查 :12426；預設 `D:\RMS\`，`cConfiguration.cpp:3583`），01:00 由 `HS_Function.cpp:204-207` 觸發。V906 在 `Command.cpp:3608`（CopyFile :3638），目前沒人呼叫。⇒ W13 要的是「檔案複製驗證」（`GetFileAttributesExA` 比大小），不是 IElaFtp；因為 `bFailIfExists=TRUE`，重試時「目標已存在且大小相同＝成功」。

## 2. MinGW `wininet.h`／`libwininet.a`

全部都有（`nm` 查過，例 `_FtpCommandA@24`、`_FtpGetFileSize@8`）：

| 群組 | 項目（wininet.h 行號） |
|---|---|
| Session | `InternetOpenA` :768、`InternetConnectA` :771、`InternetSetOptionA` :783、`InternetGetLastResponseInfoA` :787 |
| 列目錄 | `FtpFindFirstFileA` :791、`InternetFindNextFileA` :779 |
| 檔案 | `FtpGetFileSize` :790、`FtpPutFileA` :795、`FtpDeleteFileA` :797、`FtpRenameFileA` :799、`FtpOpenFileA` :801 |
| 目錄 | `FtpCreateDirectoryA` :803、`FtpSetCurrentDirectoryA` :807、`FtpGetCurrentDirectoryA` :809 |
| 原始指令 | **`FtpCommandA` :811** |
| 旗標 | `INTERNET_FLAG_RELOAD` :31、`_PASSIVE` :35、`_NO_CACHE_WRITE` :36、`INTERNET_OPEN_TYPE_DIRECT` :76、`INTERNET_SERVICE_FTP` :161、`FTP_TRANSFER_TYPE_BINARY` :183 |
| 選項 | `INTERNET_OPTION_CONNECT_TIMEOUT` :86、`_CONNECT_RETRIES` :87、send/receive／data timeout :89-94 |
| 錯誤 | `ERROR_INTERNET_EXTENDED_ERROR` :378、`_OPERATION_CANCELLED` :392 |

注意：`FtpFindFirstFileA` 宣告用 TCHAR 的 `LPWIN32_FIND_DATA`，只有 UNICODE 關時才對（目前關）——WinINet 那支檔加 `#ifdef UNICODE #error`。`wininet.h` 自己 include `<windows.h>`，跟 `Sync.h` 的 `WIN32_LEAN_AND_MEAN` 相容。

## 3. 設計

**檔案（ht9045_ela）**
- `EventLogAnalysis/ElaFtp.h/.cpp`：介面、不連網的 null 傳輸、純函式（SIZE 回覆解析、密碼遮蔽、錯誤對應、路徑串接）、照 golden 順序的 `EnsureDir`（每層 CWD，失敗 MKD＋CWD；修掉 :120 bug，記偏離）與 `UploadAndVerify`。
- `EventLogAnalysis/ElaFtpWinInet.cpp`：唯一 include `<wininet.h>` 的檔，提供 `IElaFtp* NewWinInetFtp()`。
- `EventLogAnalysis/ElaSchedule.h/.cpp`（R5）：重試、退避、錯開、開機補跑。
- `tests/ElaFtpFake.h`：腳本化的假傳輸，header-only，不進正式 archive。

**介面（傳輸層保持單純）**
```cpp
struct FtpEndpoint { std::string host, user, password; int port=21; bool passive=true; unsigned connectMs=5000, transferMs=30000; };
struct FtpStatus  { bool ok; int err; unsigned long sysErr; int ftpCode; std::string text; };  // text 已遮蔽
struct FtpEntry   { std::string name; bool isDir; long long size; };
class IElaFtp { public: virtual ~IElaFtp(){}
  virtual FtpStatus Connect(const FtpEndpoint&)=0;  virtual void Close()=0;  virtual void Abort()=0; // Abort：任何執行緒
  virtual FtpStatus ChangeDir(const std::string&)=0; virtual FtpStatus MakeDir(const std::string&)=0;
  virtual FtpStatus Put(const std::string& local, const std::string& remote)=0;            // binary STOR
  virtual FtpStatus Size(const std::string& remote, long long* n)=0;                       // SIZE；500/502/504 → SIZE_UNSUPPORTED
  virtual FtpStatus List(const std::string& dir, std::vector<FtpEntry>*)=0; };             // 一律重新列、不用快取
```

**WinINet 實作**
- 每個工作一次 `InternetOpenA(..., INTERNET_OPEN_TYPE_DIRECT, ...)`（IE proxy 不會改道 FTP）。
- 選項：`CONNECT_TIMEOUT`＝connectMs（golden 5 s，`TfFTP.cpp:56`）、`CONNECT_RETRIES`＝1；控制／資料 send/receive timeout＝transferMs。
- `InternetConnectA(..., INTERNET_SERVICE_FTP, passive ? INTERNET_FLAG_PASSIVE : 0)`。
- Put：`FtpPutFileA(..., FTP_TRANSFER_TYPE_BINARY)`（binary，伺服器大小才等於本機位元組數）。
- Size：`FtpCommandA(h, FALSE, BINARY, "TYPE I")` 再 `FtpCommandA(h, FALSE, BINARY, "SIZE <path>")`，從 `InternetGetLastResponseInfoA` 解 `213 <n>`。**不用 `FtpGetFileSize`**：它要 `FtpOpenFile(GENERIC_READ)` 的 handle，會開始 RETR 下載並卡住整個 session。
- List：先 CWD 進資料夾，`FtpFindFirstFileA(h, NULL, &fd, INTERNET_FLAG_RELOAD|INTERNET_FLAG_NO_CACHE_WRITE, 0)`＋`InternetFindNextFileA` 到 `ERROR_NO_MORE_FILES`，find handle 立刻關（一個 session 一次 find）。用 NULL 列目前資料夾，避免把有空白的 `Jam Rate.txt` 當 LIST 參數；RELOAD 避免 WinINet 快取造成過期清單。
- 錯誤文字：`GetLastError()`；`ERROR_INTERNET_EXTENDED_ERROR` 時用 `InternetGetLastResponseInfoA`（buffer 不夠就加大）讀伺服器回覆、取最後一行的 code；12xxx 用 `FormatMessageA`（wininet 模組）。**之後遮掉帳號密碼**（331 回覆常回帳號）。
- Timeout：`CONNECT_TIMEOUT` 對 FTP 不可靠 ⇒ 加看門狗 `ScopedDeadline`：開一條 `WbThread` 等 Win32 event，逾時就 `InternetCloseHandle(hOpen)`（取消阻塞 WinINet 呼叫的正規做法）；WinINet 呼叫本身留在 hub worker。期限：connect＝connectMs＋1 s；put＝transferMs＋檔案大小／32 KiB/s。`WbThread` 沒有限時 join，所以用 event。
- `Abort()` 關 `hOpen`；`Hub::Stop` 先 Abort 再 join，關機不必等 30 s 傳輸。

**驗證（`UploadAndVerify`）三項都成立才算成功**：① Put 成功；② SIZE＝本機大小（伺服器不支援 SIZE 時改用 LIST 的大小；本機非空但大小 0 算失敗）；③ 重新列出的清單裡有這個檔名。SIZE 不支援且 LIST 沒有大小 ⇒ 標「只驗檔名」寫進 FTP_Log 與 API。N25-3 主檔與 BackUp 都通過才寫 `HadUpload.txt`。每次嘗試都寫 FTP_Log（走 `W906_HT9045LOG_ROOT` 接縫），不含帳密。

**執行緒**：每個 IElaFtp 在 ElaHub worker 上的一個工作內建立、使用、關閉，不共用；`runMu_` 已保證一次一個工作＝一次一個傳輸；只有看門狗／`Abort` 的 `InternetCloseHandle` 從別的執行緒來。

**排程（ElaSchedule，傳輸層不管）**：可注入時鐘（TDateTime double）；建傳輸前先檢查閘：O10（D-a）、功能開關（例 `bN25_3_EnableULJamLog`）、主機非空、客戶碼 851／868（D-f）；錯開 offset＝FNV-1a-32(Machine ID) mod W；退避＝min(60·2^(n-1), 1800) s ×(1±0.2·u)（u 由 Machine ID＋n 決定），6 次或到下一個排程時段就放棄；報表資料範圍用排程時段、不用實際執行時間；開機補跑：每個工作一個「最後完成時段」檔，開機只補最近一次錯過的，時間＝開機＋offset。

**傳輸注入**：`Hub` 的 `FtpFactory` 函式指標**預設＝null 傳輸**（測試裡建的 Hub 連不到網路）；只有 `W906_ElaStart`（`ElaService.cpp`）裝 `NewWinInetFtp`。

**SIM 組態規則（待裁決，已交 ST01-M）**：golden 先例——SIM 的 Handler 不送 N25 觸發（906 `main.cpp:21425`），golden 各 FTP 類別在 SIM 把主機改成 127.0.0.1（`TfFTP.cpp:15-23`、`uChipMosZHUBEI_Func.cpp:206-210`、906 `HS_Function.cpp:2124-2128`）。選項：**A（建議）** 沒定義 `W906_NO_SOFT_SIMULTE`（＝模擬組態）時用只寫 log 的傳輸；**B** 照 golden，SIM 用 WinINet 但主機強制 127.0.0.1；**C** 照 D-f 只看 config。

**Passive（待裁決或記帳本）**：N25-3 用 passive=true（跟 golden Handler 所有明寫的一致）；N10 BYFILE 照 Handler 的 N10 路徑讀 `bN10FtpPassive`／`iN10FtpPort`（偏離分析器，它忽略這兩個）。

**CMake**
```cmake
add_library(ht9045_ela STATIC ... EventLogAnalysis/ElaFtp.cpp EventLogAnalysis/ElaFtpWinInet.cpp EventLogAnalysis/ElaSchedule.cpp)
target_link_libraries(ht9045_ela PUBLIC vclcompat)
if(WIN32) target_link_libraries(ht9045_ela PUBLIC wininet) endif()
```

## 4. R0 拆庫怎麼辦

現況：`CMakeLists.txt:1372` 讓 `ht9045_ela` 連 `vclcompat ht9045_nmftp`；`:3089-3098` 建 `ht9045_nmftp`；`:3110` `ht9045_kyecftp` PUBLIC 連它。ELA 沒有任何檔用 `Nmftp::`。
**建議**：**保留 `ht9045_nmftp` 這個庫**（無害、讓 TNMFTP 只有一份定義；還原要重開 Jimmy 的 kyecftp 區塊）；在 R4 那顆把 :1372 的 `ht9045_nmftp` 換成 `wininet`，並改 :1372 與 :3083-3088 說 ELA 會用它的註解（現在是假相依）。MiniFtpEngine 的測試是直接編原始檔，不受影響。

## 5. ctest 計畫（只寫 %TEMP% 或 build 資料夾；log root 經 ENVIRONMENT 接縫）

**`ELA_Ftp`**（`tests/test_ela_ftp.cpp`＋`ElaFtpFake.h`，假傳輸記錄呼叫、可指定某次失敗）：
1. 成功：base／HOST／BackUp 各「CWD 失敗 → MKD → CWD」，STOR 主檔、STOR `BackUp/Jam Rate_20260927_0000.txt`，SIZE ×2、LIST ×2（重新列旗標），寫 `HadUpload.txt`。
2. SIZE 不符：失敗、沒有 `HadUpload.txt`、下次在 +60 s ±20%；修好後第二次成功。
3. SIZE 不支援（502）：改用 LIST 大小；LIST 大小 0 → 失敗；LIST 沒大小 →「只驗檔名」。
4. LIST 沒有檔名 → 失敗。
5. BackUp STOR 失敗 → 失敗、沒有 `HadUpload.txt`。
6. Connect／登入（530）失敗：之後沒有 CWD／STOR、有呼叫 Close、log 裡帳密變 `***`；逾時算可重試。
7. 不存在的資料夾逐層 CWD/MKD（含 golden :120 做錯的情況）。
8. N10 BYFILE 每列各自的端點。
9. 閘（bN25_3 關、主機空、客戶碼 ≠ 851、O10 關）→ factory 呼叫 0 次。
10. 網路防護：測試開頭與結尾 `GetModuleHandleA("wininet.dll")==NULL`（證明 `ElaFtpWinInet.o` 沒被連進來）、正式 factory 呼叫計數 0。

**`ELA_Schedule`**（假時鐘、不 sleep）：FNV-1a 已知值（"" → 0x811C9DC5）與 mod 900；Machine ID 是 "HT-90xx" 或空時改用 hostname；六段退避 ±20% 且可重現、6 次放棄；每日工作一天一次、VTEST 一週一次；開機只補最近一次錯過的時段、資料範圍取時段；O10 與客戶碼閘；N17 複製驗證（成功、目標已存在同大小＝成功、目標資料夾不存在＝重試）。

WinINet 那支檔沒有執行期測試（要網路），只要求兩組態都編得過。
