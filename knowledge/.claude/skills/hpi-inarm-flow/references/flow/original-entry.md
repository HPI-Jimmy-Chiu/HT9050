# HT9045 InArm Flow Knowledge

按需要選取以下章節，原文依順序保留。

- [HT9045 InArm Flow Knowledge](original-entry/00.md)
- [適用場景](original-entry/01.md)
- [吸嘴物理排列（8-Sucker 模式）](original-entry/02.md)
- [InArm 基準軸（取放料計算原點）](original-entry/03.md)
- [關鍵原始檔](original-entry/04.md)
- [參考文件](original-entry/05.md)
- [1. 呼叫階層](original-entry/06.md)
- [2. DoInArm() — 入口 Guard Checks (ainarm2.cpp ~1588)](original-entry/07.md)
- [3. DoInArm_9045() — Dispatch (ainarm9045.cpp ~3907)](original-entry/08.md)
- [4. 主狀態機 (DoInArm_9045_XxY_Z, Task = iArmTask)](original-entry/09.md)
- [5. DoInArmPickFromLoadStage_9045() (Task = iPickFromLoadStageTask)](original-entry/10.md)
- [6. DoInArmAdditionalFunction() (Task = iInArmAdditionalFunctionTask)](original-entry/11.md)
- [7. DoPlaceToHotPlate_9045() (Task = iInArmPlaceToHotPlateTask)](original-entry/12.md)
- [8. DoInArmPickFromHotPlate_9045() (Task = iInArmPickFromHotPlateTask)](original-entry/13.md)
- [9. DoInArmPlaceToShuttle_9045() (Task = iInArmPlaceToShuttleTask)](original-entry/14.md)
- [10. 吸嘴配置 (iInArmType)](original-entry/15.md)
- [10.5 InArm Pitch 模式 — `ArmSpeed[InArm].bVariModeFIX`](original-entry/16.md)
- [11. 常見問題快查](original-entry/17.md)
- [10. HP 縮 Pitch 公式與放/取料流程（Steven 2023.07 軟體重構）](original-entry/18.md)
- [11. RT-Initial Start 與 Auto Tray 清除保護](original-entry/19.md)
- [11b. InArm 安全互鎖判斷函式（與 OutArm 對照）](original-entry/20.md)
- [12. Hot Mode Loader 抽空 → 「吸嘴有料卻不放 HP」+ CatchTray 換盤死結](original-entry/21.md)
- [13. WAR0152 Loader 取料「motor will out of limit」— 奇數寬盤 + 1x2 AxEx 單顆取料超極限](original-entry/22.md)
- [14. HP 取料帳本 `PickFromHPList`（`uPlateInfo`）— 設計原則與 ASM 補料失步](original-entry/23.md)
- [15. 加熱模式「熱盤填滿才取」的取放優先權 —— 以及 RT 小批量為何永遠不出料](original-entry/24.md)
- [1. 放料：**8 格全部 `NULL_IC` 才成立**](original-entry/25.md)
- [2. 取料：**跟 booking 走，不要求整塊**](original-entry/26.md)
- [3. 推論：空格集合恆為「整塊」的聯集](original-entry/27.md)
- [4. ⚠ 容忍度只有 1 顆 —— 熱盤「接近滿」是**設計上的常態**](original-entry/28.md)
- [5. 已知缺陷（未修）：閘門與搜尋的判準不一致](original-entry/29.md)
- [合併補充：repo 既有參考（20261001）](original-entry/30.md)

# HT9045 InArm Flow Knowledge

[讀取此節](original-entry/00.md#ht9045-inarm-flow-knowledge)

## 適用場景

[讀取此節](original-entry/01.md#適用場景)

## 吸嘴物理排列（8-Sucker 模式）

[讀取此節](original-entry/02.md#吸嘴物理排列8-sucker-模式)

### Variable Pitch 模式

[讀取此節](original-entry/02.md#variable-pitch-模式)

### iInArmType → 吸嘴選擇路由（重要）

[讀取此節](original-entry/02.md#iinarmtype--吸嘴選擇路由重要)

### 2x2 模式 iInArmType 決策條件（QualSite2X2 Hot 路徑）

[讀取此節](original-entry/02.md#2x2-模式-iinarmtype-決策條件qualsite2x2-hot-路徑)

### InArm 動作方向

[讀取此節](original-entry/02.md#inarm-動作方向)

## InArm 基準軸（取放料計算原點）

[讀取此節](original-entry/03.md#inarm-基準軸取放料計算原點)

### 依 `USE_IN_OUT_ARM_Y_PITCH` 切換

[讀取此節](original-entry/03.md#依-use_in_out_arm_y_pitch-切換)

### Loader 取料公式（多吸嘴版，`ainarm9045.cpp` L4660）

[讀取此節](original-entry/03.md#loader-取料公式多吸嘴版ainarm9045cpp-l4660)

### X-Pitch 單步距離（`ainarm9045.cpp` L4476-4610）

[讀取此節](original-entry/03.md#x-pitch-單步距離ainarm9045cpp-l4476-4610)

## 關鍵原始檔

[讀取此節](original-entry/04.md#關鍵原始檔)

## 參考文件

[讀取此節](original-entry/05.md#參考文件)

## 1. 呼叫階層

[讀取此節](original-entry/06.md#1-呼叫階層)

## 2. DoInArm() — 入口 Guard Checks (ainarm2.cpp ~1588)

[讀取此節](original-entry/07.md#2-doinarm--入口-guard-checks-ainarm2cpp-1588)

## 3. DoInArm_9045() — Dispatch (ainarm9045.cpp ~3907)

[讀取此節](original-entry/08.md#3-doinarm_9045--dispatch-ainarm9045cpp-3907)

## 4. 主狀態機 (DoInArm_9045_XxY_Z, Task = iArmTask)

[讀取此節](original-entry/09.md#4-主狀態機-doinarm_9045_xxy_z-task--iarmtask)

### Ambient Mode (常溫: Loader → Shuttle)

[讀取此節](original-entry/09.md#ambient-mode-常溫-loader--shuttle)

### Hot Mode (高溫: Loader → HotPlate → Shuttle)

[讀取此節](original-entry/09.md#hot-mode-高溫-loader--hotplate--shuttle)

### State Descriptions

[讀取此節](original-entry/09.md#state-descriptions)

## 5. DoInArmPickFromLoadStage_9045() (Task = iPickFromLoadStageTask)

[讀取此節](original-entry/10.md#5-doinarmpickfromloadstage_9045-task--ipickfromloadstagetask)

## 6. DoInArmAdditionalFunction() (Task = iInArmAdditionalFunctionTask)

[讀取此節](original-entry/11.md#6-doinarmadditionalfunction-task--iinarmadditionalfunctiontask)

## 7. DoPlaceToHotPlate_9045() (Task = iInArmPlaceToHotPlateTask)

[讀取此節](original-entry/12.md#7-doplacetohotplate_9045-task--iinarmplacetohotplatetask)

## 8. DoInArmPickFromHotPlate_9045() (Task = iInArmPickFromHotPlateTask)

[讀取此節](original-entry/13.md#8-doinarmpickfromhotplate_9045-task--iinarmpickfromhotplatetask)

## 9. DoInArmPlaceToShuttle_9045() (Task = iInArmPlaceToShuttleTask)

[讀取此節](original-entry/14.md#9-doinarmplacetoshuttle_9045-task--iinarmplacetoshuttletask)

## 10. 吸嘴配置 (iInArmType)

[讀取此節](original-entry/15.md#10-吸嘴配置-iinarmtype)

### General.ini 吸嘴相關參數

[讀取此節](original-entry/15.md#generalini-吸嘴相關參數)

### TMyKitSuck 核心成員速查

[讀取此節](original-entry/15.md#tmykitsuck-核心成員速查)

### 軟體架構深入（HP 縮 Pitch、Teaching 推導、機型拓樸）

[讀取此節](original-entry/15.md#軟體架構深入hp-縮-pitchteaching-推導機型拓樸)

## 10.5 InArm Pitch 模式 — `ArmSpeed[InArm].bVariModeFIX`

[讀取此節](original-entry/16.md#105-inarm-pitch-模式--armspeedinarmbvarimodefix)

### 10.5.1 語意

[讀取此節](original-entry/16.md#1051-語意)

### 10.5.2 三層變數結構

[讀取此節](original-entry/16.md#1052-三層變數結構)

### 10.5.3 主要 Pitch 判斷點（讀取 `bVariModeFIX`）

[讀取此節](original-entry/16.md#1053-主要-pitch-判斷點讀取-bvarimodefix)

### 10.5.4 設值點與時機（寫入 `bVariModeFIX`）

[讀取此節](original-entry/16.md#1054-設值點與時機寫入-bvarimodefix)

### 10.5.5 SECS/GEM 整合

[讀取此節](original-entry/16.md#1055-secsgem-整合)

### 10.5.6 與多顆吸取 (iRowCT) 的影響表（已更正 2026-05-11）

[讀取此節](original-entry/16.md#1056-與多顆吸取-irowct-的影響表已更正-2026-05-11)

## 11. 常見問題快查

[讀取此節](original-entry/17.md#11-常見問題快查)

## 10. HP 縮 Pitch 公式與放/取料流程（Steven 2023.07 軟體重構）

[讀取此節](original-entry/18.md#10-hp-縮-pitch-公式與放取料流程steven-202307-軟體重構)

### 10.1 HP 縮 Pitch 數學模型

[讀取此節](original-entry/18.md#101-hp-縮-pitch-數學模型)

### 10.2 重構後 HP 放/取料核心函式

[讀取此節](original-entry/18.md#102-重構後-hp-放取料核心函式)

### 10.3 放料順序表（4 吸嘴範例）

[讀取此節](original-entry/18.md#103-放料順序表4-吸嘴範例)

### 10.4 關鍵 Source 位置（V3.33.904.2 基準）

[讀取此節](original-entry/18.md#104-關鍵-source-位置v3339042-基準)

## 11. RT-Initial Start 與 Auto Tray 清除保護

[讀取此節](original-entry/19.md#11-rt-initial-start-與-auto-tray-清除保護)

## 11b. InArm 安全互鎖判斷函式（與 OutArm 對照）

[讀取此節](original-entry/20.md#11b-inarm-安全互鎖判斷函式與-outarm-對照)

## 12. Hot Mode Loader 抽空 → 「吸嘴有料卻不放 HP」+ CatchTray 換盤死結

[讀取此節](original-entry/21.md#12-hot-mode-loader-抽空--吸嘴有料卻不放-hp-catchtray-換盤死結)

### 12.1 為什麼「吸嘴有料卻不去放 HotPlate」

[讀取此節](original-entry/21.md#121-為什麼吸嘴有料卻不去放-hotplate)

### 12.2 三方互等死結

[讀取此節](original-entry/21.md#122-三方互等死結)

### 12.3 為什麼 CatchTray 不搬空盤：消去法鎖定 `InArmXYZSafe()==false`（取代先前 LED 推論）

[讀取此節](original-entry/21.md#123-為什麼-catchtray-不搬空盤消去法鎖定-inarmxyzsafefalse取代先前-led-推論)

#### In Pos 語意（RogerYang 確認）

[讀取此節](original-entry/21.md#in-pos-語意rogeryang-確認)

#### ✅ SaveTaskList 直接記了答案（20260623）—— In Pos 作廢，失敗在 X encoder

[讀取此節](original-entry/21.md#-savetasklist-直接記了答案20260623-in-pos-作廢失敗在-x-encoder)

#### 仍待查（為何還不能定論）

[讀取此節](original-entry/21.md#仍待查為何還不能定論)

### 12.4 可信度分級 & 下一步

[讀取此節](original-entry/21.md#124-可信度分級--下一步)

### 12.5「config/IO/DB 都一樣卻只壞一台」（機制待查）

[讀取此節](original-entry/21.md#125configiodb-都一樣卻只壞一台機制待查)

## 13. WAR0152 Loader 取料「motor will out of limit」— 奇數寬盤 + 1x2 AxEx 單顆取料超極限

[讀取此節](original-entry/22.md#13-war0152-loader-取料motor-will-out-of-limit-奇數寬盤--1x2-axex-單顆取料超極限)

### 13.1 觸發條件

[讀取此節](original-entry/22.md#131-觸發條件)

### 13.2 機制（為何偏偏最右欄才報）

[讀取此節](original-entry/22.md#132-機制為何偏偏最右欄才報)

### 13.3 內建縮 pitch 補救為何失效（關鍵）

[讀取此節](original-entry/22.md#133-內建縮-pitch-補救為何失效關鍵)

### 13.4 解法

[讀取此節](original-entry/22.md#134-解法)

## 14. HP 取料帳本 `PickFromHPList`（`uPlateInfo`）— 設計原則與 ASM 補料失步

[讀取此節](original-entry/23.md#14-hp-取料帳本-pickfromhplistuplateinfo-設計原則與-asm-補料失步)

### 14.0 總綱：帳本錯亂全景（先讀這節，20260829 定版）

[讀取此節](original-entry/23.md#140-總綱帳本錯亂全景先讀這節20260829-定版)

#### 中期實作記錄（20260829 完成改碼，**待編譯上機驗證**）

[讀取此節](original-entry/23.md#中期實作記錄20260829-完成改碼待編譯上機驗證)

#### ⛔ 20260831 定案：中期方案（借料改銷帳）已回退 —— **借出只能是「暫停」，不能是「銷帳」**

[讀取此節](original-entry/23.md#-20260831-定案中期方案借料改銷帳已回退--借出只能是暫停不能是銷帳)

### 14.1 設計原則（機台鐵則，RogerYang 20260801 確認）

[讀取此節](original-entry/23.md#141-設計原則機台鐵則rogeryang-20260801-確認)

### 14.2 帳本讀寫點（`Public/HTEditList.cpp`，V3.33.908.7 行號）

[讀取此節](original-entry/23.md#142-帳本讀寫點publichteditlistcppv3339087-行號)

### 14.3 取料幾何：anchor 與吸嘴 X 位移（重要，避免反推各模式 pitch 公式）

[讀取此節](original-entry/23.md#143-取料幾何anchor-與吸嘴-x-位移重要避免反推各模式-pitch-公式)

### 14.4 缺陷：ASM 期間從 Loader 補料到 HP 不寫帳本（20260731 偉測 HHT-30 根因）

[讀取此節](original-entry/23.md#144-缺陷asm-期間從-loader-補料到-hp-不寫帳本20260731-偉測-hht-30-根因)

### 14.5 同區域其他已知陷阱

[讀取此節](original-entry/23.md#145-同區域其他已知陷阱)

### 14.6 缺陷：幽靈帳（ASM 結束後帳本指到空格）→ `HasHotReadyIC_9045()` 永遠回 false（20260826 偉測 HHT-10 根因）

[讀取此節](original-entry/23.md#146-缺陷幽靈帳asm-結束後帳本指到空格-hashotreadyic_9045-永遠回-false20260826-偉測-hht-10-根因)

#### 成因（三段，缺一不可 —— 這也是「為什麼大部分情況都沒事」的答案）

[讀取此節](original-entry/23.md#成因三段缺一不可--這也是為什麼大部分情況都沒事的答案)

### 14.7 帳本三種壞法與「對帳（reconcile）」原則

[讀取此節](original-entry/23.md#147-帳本三種壞法與對帳reconcile原則)

### 14.8 缺陷：一趟取料跨越模式切換 → `case 350` 跳過銷帳（20260909 偉測 HHT-139 根因，**已修**）

[讀取此節](original-entry/23.md#148-缺陷一趟取料跨越模式切換--case-350-跳過銷帳20260909-偉測-hht-139-根因已修)

#### 修正（20260909，`V3.33.912.0_20260908_RogerYang_AI`，未編譯未上機）

[讀取此節](original-entry/23.md#修正20260909v3339120_20260908_rogeryang_ai未編譯未上機)

#### 附帶缺陷（同一個錯誤判別式）

[讀取此節](original-entry/23.md#附帶缺陷同一個錯誤判別式)

#### 症狀與判讀

[讀取此節](original-entry/23.md#症狀與判讀)

## 15. 加熱模式「熱盤填滿才取」的取放優先權 —— 以及 RT 小批量為何永遠不出料

[讀取此節](original-entry/24.md#15-加熱模式熱盤填滿才取的取放優先權--以及-rt-小批量為何永遠不出料)

### 15.1 case 50 的取／放決策（`ainarm9045_2x6_8.cpp`，其他模式檔對稱）

[讀取此節](original-entry/24.md#151-case-50-的取放決策ainarm9045_2x6_8cpp其他模式檔對稱)

### 15.2 `CheckHasSpaceToPlace_9045()` 的判準（`ainarm_SearchPlacePlate.cpp`）

[讀取此節](original-entry/24.md#152-checkhasspacetoplace_9045-的判準ainarm_searchplaceplatecpp)

### 15.3 RT（複測）小批量為何永遠不出料

[讀取此節](original-entry/24.md#153-rt複測小批量為何永遠不出料)

### 15.4 程式已提供的兩個機制（優先用這兩個，不要改通用邏輯）

[讀取此節](original-entry/24.md#154-程式已提供的兩個機制優先用這兩個不要改通用邏輯)

### 15.5 ⚠ 判讀陷阱：`PickHPRec.json` 的 `Site=-1` **不是**「熱盤格空著、team 破碎待補」

[讀取此節](original-entry/24.md#155--判讀陷阱pickhprecjson-的-site-1-不是熱盤格空著team-破碎待補)

### 15.6 InArm 旋轉站 case 1160 靜默凍結

[讀取此節](original-entry/24.md#156-inarm-旋轉站-case-1160-靜默凍結)

# 熱盤落點幾何鐵律（放料／取料／碎片化）

[讀取此節](original-entry/24.md#熱盤落點幾何鐵律放料取料碎片化)

## 1. 放料：**8 格全部 `NULL_IC` 才成立**

[讀取此節](original-entry/25.md#1-放料8-格全部-null_ic-才成立)

## 2. 取料：**跟 booking 走，不要求整塊**

[讀取此節](original-entry/26.md#2-取料跟-booking-走不要求整塊)

## 3. 推論：空格集合恆為「整塊」的聯集

[讀取此節](original-entry/27.md#3-推論空格集合恆為整塊的聯集)

## 4. ⚠ 容忍度只有 1 顆 —— 熱盤「接近滿」是**設計上的常態**

[讀取此節](original-entry/28.md#4--容忍度只有-1-顆--熱盤接近滿是設計上的常態)

## 5. 已知缺陷（未修）：閘門與搜尋的判準不一致

[讀取此節](original-entry/29.md#5-已知缺陷未修閘門與搜尋的判準不一致)

## 合併補充：repo 既有參考（20261001）

[讀取此節](original-entry/30.md#合併補充repo-既有參考20261001)
