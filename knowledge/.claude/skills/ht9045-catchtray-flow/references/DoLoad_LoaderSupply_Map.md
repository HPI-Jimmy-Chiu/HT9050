# DoLoad() 進料限制地圖（Loader 供料狀態機）

舊引用路徑保留；[讀取整理後文件](../../hpi-tray-flow/references/flow/catch-tray/references/DoLoad_LoaderSupply_Map.md)。

## 0. 角色

[讀取此節](../../hpi-tray-flow/references/flow/catch-tray/references/DoLoad_LoaderSupply_Map.md#0-角色)

## 1. 第①層：呼叫層閘門（`csystem.cpp` — DoLoad 被不被呼叫）

[讀取此節](../../hpi-tray-flow/references/flow/catch-tray/references/DoLoad_LoaderSupply_Map.md#1-第①層呼叫層閘門csystemcpp--doload-被不被呼叫)

## 2. 第②層：DoLoad 入口守衛（switch 之前，`2457-2476`）

[讀取此節](../../hpi-tray-flow/references/flow/catch-tray/references/DoLoad_LoaderSupply_Map.md#2-第②層doload-入口守衛switch-之前2457-2476)

## 3. 第③層：狀態機閘門（`LoadTask`，`2478` 起）

[讀取此節](../../hpi-tray-flow/references/flow/catch-tray/references/DoLoad_LoaderSupply_Map.md#3-第③層狀態機閘門loadtask2478-起)

## 4. 客戶 / Config 相關額外限制（散在 case 600 / 800）

[讀取此節](../../hpi-tray-flow/references/flow/catch-tray/references/DoLoad_LoaderSupply_Map.md#4-客戶--config-相關額外限制散在-case-600--800)

## 5. 關鍵結論（給後續下 gate / 排查用）

[讀取此節](../../hpi-tray-flow/references/flow/catch-tray/references/DoLoad_LoaderSupply_Map.md#5-關鍵結論給後續下-gate--排查用)

## 6. 相關函式錨點（`asendic_Loader.cpp` 908.2）

[讀取此節](../../hpi-tray-flow/references/flow/catch-tray/references/DoLoad_LoaderSupply_Map.md#6-相關函式錨點asendic_loadercpp-9082)
