# V906 移植樹（C++）的 fAllMotorHome／回原點現況

> **給誰看**：要在移植樹動 Teach／Motor Test、`fAllMotorHome`、START 先回原點這一段的 Claude session 與 Steven。本 skill 的 `SKILL.md` §1～§11 講 BCB6 golden 的語意，這裡只記移植樹怎麼接。
> **三棵樹**：移植樹＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`（**下面沒寫樹根的路徑都在這棵底下**）；golden V912＝`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`（cp950，唯讀）；網頁＝`D:\HT9045\web\`。
> **核對基準**：分支 `v906/steven-cbridge-review6` HEAD `db1b7638`（20260927 晚，Steven 團隊 St01 整理）。裁決檔：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260927.md`；
> R 題：`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md`。

## 1. golden 的規則：開、關 Teach／Motor Test 都把 fAllMotorHome 清掉

| 時機 | golden V912 | 說明 |
|---|---|---|
| 開 Teach | `uteach.cpp:1610`（`TfTeach::FormShow`，`:1417` 起）`fAllMotorHome=false;` | 進教導頁就清 |
| 關 Teach | `main.cpp:28846-28847`（`TfMain::sbTeachingClick`：`fTeach->ShowModal();` 下一行 `fAllMotorHome=false;`） | `ShowModal` 回來＝視窗關了 |
| 關 Motor Test | `uteach.cpp:2442-2443`（`TfTeach::btnMotorTestClick`：`fMotorTest->ShowModal();` 下一行清） | Motor Test 只能從 Teach 開 |
| 運轉中 | `main.cpp:28829-28830` `sbTeachingClick` 第一行 `if(SystemStart) return;` | 運轉中開不了 Teach，所以運轉中永遠不會清 |

清掉之後：`fAllMotorHome==false` 時 `DoAllProcess` 每拍直接 return（移植樹 `csystem.cpp:1629` `if(SoftStop==true || SystemStart==false || fAllMotorHome==false) return;`），
下一次按 START 由 `TfMain::Start`（V912 `main.cpp:4528`）`:6390` `if(fAllMotorHome==false)` → `:6431` `Home("Home by Start")` 先整機回原點，回完由 `csystem.cpp:10925` `bHomeByStart` 那段接著生產。golden 沒有任何畫面提示。

## 2. S122：移植樹怎麼知道 Teach／Motor Test 關了（`0b166feb`；RULINGS_20260927 第 2 條第 18 題＝B、R80～R83＝A）

**裁決**：第 18 題 B（使用者 0927 07:4x「依據建議」）——只做「關掉 Teach／Motor Test ⇒ 標成必須重新 find home」；「主畫面斷線或重新整理就暫停生產」不做。
**更正**（`8f78217f`）：這不是比 golden 嚴，是照翻（§1 的三行）。方案全文：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\S122_TEACH_LEAVE_PLAN_20260927.md`。

網頁上 Teach／Motor Test 是 `D:\HT9045\web\background.html` 裡的全螢幕視窗，關掉只是藏起來；C++ 唯一知道「視窗開著沒」的來源是 background.html 送的視窗總表 `ui.windows.put`
（規則與三個問法見 skill `ht9045-html-json` 的 `references/wbserve-conventions.md` §10）。

- **本體**：`WebTeachLeave.h`／`WebTeachLeave.cpp`（新檔，不在 golden；只 include `WebWindowRegistry.h` 與標準庫，不 include `cmydef.h`，ctest 才連得起來）。
  wb_serve 每 500 ms（`pumpBeat`）呼叫 `W906_TeachLeaveTick(&fAllMotorHome, SystemStart)`：`tools\wb_serve.cpp:5953` 同一行插入；`CMakeLists.txt:3368` 同一行把 `WebTeachLeave.cpp` 加進 wb_serve。
- **判斷**：`WebWindowRegistryFShowPolicy("fTeach")`／`("fMotorTest")` 的**邊緣**——跟 MainProc「Teach／Motor Test 開著時暫停」用的是同一個判斷。兩個表單各自記上一拍：
  - 在用 → 不在用（關掉，R80）⇒ 清。
  - 不在用 → 在用（打開，R81＝A，照 golden `uteach.cpp:1610`）⇒ 也清；開關在 `WebTeachLeave.cpp:23` `kTeachLeaveClearOnOpen = true`，改成 false 就是 R81 的 B（只在關掉時清）。
  - 從 Teach 開的 Motor Test 按 EXIT 回到 Teach（fTeach 仍在用）也是一個邊緣 ⇒ 清（golden `uteach.cpp:2443`）。
  - `SystemStart` 時看到邊緣**不清、只印一行**，也不記下來等停機補清（R82＝A：運轉中清掉會讓 `DoAllProcess` 每拍 return、機台停在半路沒有警報；運轉中這兩頁的移動命令本來就全被擋）。
  - 操作員畫面不加提示（R83＝A，照 golden）。
  - 每個邊緣在主控台印一行，例 `[S122] fTeach closed -> fAllMotorHome=false (was 1; golden V912 main.cpp:28847 (after fTeach->ShowModal); RULINGS_20260927 #18=B)`。
- **全部過期時沿用上一拍**（`WebTeachLeave.cpp:83-95`）：某表單的回報全部過期（`WebWindowRegistryQuery(form).stale`；瀏覽器關掉或分頁被節流超過 15 秒）時，
  `WebWindowRegistryFShowConservative` 對任何表單都回「在用」（`WebWindowRegistry.cpp:235`），連最後說 closed／從沒開過的也一樣。不擋的話，關瀏覽器會讓 `fMotorTest` 變成「打開」⇒ 清旗標，心跳回來又「關掉」⇒ 再清。
  沿用上一拍之後，新鮮回報回來時只有「過期前在用、回來不在用」才算關掉（例 F5 重新整理、關掉瀏覽器後重開 HMI）。從沒收過總表（Q8-B）與總表裡缺這個表單都不算過期。總表與 MainProc 的行為沒改（只讀 `WebWindowRegistryQuery`）。
- **反應時間**（R80＝A）：按 EXIT 馬上算；F5 重新整理 15 秒內算；關瀏覽器／當掉要等下一個 HMI 連上；斷線重連、切到別的瀏覽器分頁不算。同時開兩個 HMI 分頁時可能誤判成關掉——代價只是下一次 START 多回一次原點。
- **其他會清的地方**（`0b166feb` 之前就有）：Motor Test 每個命令（含關視窗送的 `formClose`；`WebMotorAccess.cpp:4137` `if (!be.GoldenSystemStart()) be.GoldenClearAllMotorHome();`，運轉中不清）、Teach 存檔、Teach 頁載入（§3）。
- **ctest**：`WebTeachLeave`（`tests\test_teachleave.cpp`，`-Wall -Wextra`，`tests\CMakeLists.txt:4611` 起）；`0b166feb` 交件時在 St01 自己的 build 目錄（`D:\AI_TempFile\st01e-teachleave-build`，SIM）跑過 52 項全過，wb_serve 沒 build、沒跑。

## 3. J1：Teach 頁載入時清旗標，改成只在沒運轉時清（Jimmy `8b5a91b5`，已在 main，St01 `5b9dbc6d` 合進來）

- 移植樹 `FileRW\Teach.cpp:259`（C 路 Teach 開頁 `editlist.get`）照 golden `FormShow` 清 `fAllMotorHome`（`AI(W906-W5-b)`；它引的是 V906 BCB 樹的 `uteach.cpp:1606`——Teach 的產生器讀的 golden 是 V906 BCB 樹，不是 V912）。
- 問題：移植樹的 Teach iframe **開站就載入**（`Teach.cpp:259` 註解引 `web/background.html:460`／`:906`），運轉中開 HMI 或重新整理就會清掉旗標 ⇒ `DoAllProcess` 每拍 return、沒有警報，違反第 18 題 B。
  golden 運轉中根本打不開 Teach（V912 `main.cpp:28829-28830`）。St01 在 S122 方案 §6 J1 提出（decisions R82 的先前紀錄）。
- 修法（同一行）：`g_formShown = true;  if (SystemStart == false) fAllMotorHome = false;`。

## 4. ⚠ 已知風險：所有瀏覽器關掉超過 15 秒，運轉中的機台會安靜停下（回報了、沒改）

- 視窗總表的規則是「15 秒沒收到瀏覽器心跳＝不知道＝當成開著」（`WebWindowRegistry.cpp:235`，筆電 0920 寫的）；EastSun 0925 把 golden「Teach／Motor Test 開著時主流程暫停」
  （V912 `csystem.cpp:17764` `if(fMotorTest->fShow || fTeach->fShow)`，MainProc 裡）接到這個判斷上（移植樹 `csystem.cpp:30493-30494` → `WebMotorAccessLive.cpp:1140-1143` `W906_HookFShow` → `WebWindowRegistryFShowPolicy`，EastSun 裁決 R8，MT-E3b／E3c）。
- 合起來：例如夜班把瀏覽器全部關掉，機台跑到一半停住、沒有警報；打開網頁又自己繼續。golden 不會這樣（運轉中打不開 Teach）。跟第 18 題 B 衝突。S122 的邊緣偵測已避開這個方向（§2「沿用上一拍」），但 MainProc 的暫停本身沒改。
- 已寫成 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\NIGHT_REPORT.md`「🔁 20260927（日）16:4x 交接」E 節第 36 題與 `D:\HT9045\docs\handoff\TO_STEVEN.md`（main）§4 20260927 19:0x 那一列，給 Jimmy／EastSun；
  筆電建議 A（只改主流程用的 `W906_HookFShow`：全部過期時照最後一次回報，START 的保守擋法不動）。**Jimmy 回覆、EastSun 知道之前不動。** 暫時做法：機台運轉中至少留一個 HMI 分頁開著（GitHub 第 57 包 README 已提醒）。

## 5. 相關：單軸回原點的本體已照 golden 翻（Jimmy `23c264b9`，SMHOME）

- `ProcessSingleMotorHome`（golden `uhome.cpp:415-644`）＋`InitProcessSingleMotorTask`（`:397-401`）逐行翻到移植樹 `uhome.cpp` 檔尾，取代 `acatchtray_shims.cpp` 的兩支樁
  （以前 `ProcessSingleMotorHome` 一律回 true ⇒ 27 個呼叫者以為那顆馬達「已經回好原點」，實際上沒動）。⚠ 行為改變：單軸回原點會真的動那顆馬達、等 home 訊號。
- MainProc 在 Teach／Motor Test 開著時那一臂的 `fMotorTest->bSingleHome`（`csystem.cpp:30496-30503`）就是呼叫它；判斷成功仍照 `SKILL.md` §2（`HomeFlag==1`）。
