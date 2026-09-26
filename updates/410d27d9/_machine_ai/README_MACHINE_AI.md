# 給機台端 Claude：小更新包 —— 網頁權杖卡住（GitLab main `410d27d9`，相對上一包 `56bbf785`）

> 筆電端 Claude 20260926 11:0x 產生。上一包是 GitHub `2944d3e`（GitLab main `56bbf785`，相對 66cb14e0 的合併包）。
> **這一包只有 4 個檔**，底稿是 `base_56bbf785\`（＝上一包帶過去的版本）。機台還在套 2944d3e 的話，**先把那一包套完、commit，再套這一包**。
> 目標一樣是 `D:\HT9045\_integ_ioweb`；做法與前幾包相同（`check_and_copy.ps1`）。

## 修什麼

機台 0926 實測：HW.IoSetView 按 Output 回「操作權杖在別的畫面手上」。原因是 `web/page/ht9045_recipe_client.js` 的 `acquire()`
拿到就不還，全 web 沒有人呼叫 `release()`；Motor Test 按 X 只是把 iframe 藏起來，連線還在 ⇒ 伺服器要 10 分鐘才收回。

| 檔 | 改了什麼 |
|---|---|
| `web/page/ht9045_recipe_client.js` | 要權杖的指令做完 **30 秒**沒有下一個就 `control.release`；還有指令在路上、或頁面登記的 hold 回 true 時不還；送出時以為拿著卻回 `not-operator`（伺服器 10 分鐘收回）⇒ 重拿、重送一次；別人拿著回 `control-held` 照原樣報錯。新 API：`setTokenHold(fn)`、`tokenIdleMs(ms)` |
| `web/page/motor-access.js` | `init` 時登記 hold＝動作還沒回報完成（pending）或頁面 `cfg.holdToken()` 回 true |
| `web/page/HW.MotorTest.html` | `holdToken`＝`#btnHome.down` 或 `#btnLoopMove.down`（停掉它們的 `start:false` 要權杖；btnStop 與 jog 放開走 `motor.stop`，本來就免權杖） |
| `HT9011UC_Cpp_V3.33.906.0/tools/webprobe/token_idle_selftest.cjs` | 新的離線自我測試（node），24 項 |

## ⚠ 三方合併：以機台版為準的地方

`HW.MotorTest.html` 與 `ht9045_recipe_client.js` 在機台上很可能是 LOCAL。**base＝`base_56bbf785\` 的版本**，合併時：

- 機台版的 **Loop Move 位置讀取（MT-E1b）、ALed、Light Scale、每 60 秒 keepAlive（:1092）一律保留機台的**。
- 筆電這包只加了三小段：recipe client 的權杖記帳（`cmd()` 包一層、`acquire()`／`release()` 各改一行、`global.HT9045Recipe = api` 之前那一段）、
  motor-access.js 第 13 行與 `init` 那一行、MotorTest 的 `holdToken:` 那一行。
- **機台版要多做一件事**：把 Light Scale 進行中也放進 `holdToken`（例：`|| lightScaleRunning`，用機台頁面自己的狀態變數），否則 Light Scale 跑超過 30 秒時權杖會被還掉。
  keepAlive 本身經過 recipe client，會把 30 秒往後推，不衝突。

## 步驟

1. 開場：沒有 wb_serve／cmake／ctest 在跑；整合樹與 web 樹先 **commit 目前的 WIP**（不要 stash）。
2. 備份：這一包只動網頁與一支測試，不碰 system／config；照慣例備份 `web\page` 這 3 支即可。
3. `powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`
4. EastSun 同意後 `-Mode Apply`；LOCAL 照上面的原則三方合併。
5. 驗證：
   - `node HT9011UC_Cpp_V3.33.906.0\tools\webprobe\token_idle_selftest.cjs` → `ALL PASSED (24 passed, 0 failed)`（改成測機台版 recipe client：`set W906_RECIPE_CLIENT=<機台檔的絕對路徑>` 再跑）
   - `node --check` 那兩支 .js
   - 瀏覽器（不用動機台）：開 MotorTest 按一顆**非動作**鈕（例 btnRange）→ 等 35 秒 → IO 頁按 Output 應該成功。
   - 會動機台的那一項等 EastSun 在旁：HOME 進行中 → IO 頁應該仍被擋；HOME 完成 30 秒後放行。
6. commit，回報：Check 報告前 5 行、LOCAL 清單、自我測試結果、新 commit hash。

## 沒動的

- 1203 Setting 頁（`web/js/pci1203.js` 的 autoControl，每 4 秒自動重拿，使用者 0914 要的「永遠取得控制權」）縮小時不卸載，仍會擋別頁。
  現場請**按 X 關掉**（會卸載並 release），不要只縮小。
- 伺服器端（WebBridgeServer.cpp 的單一操作員權杖）沒改。
