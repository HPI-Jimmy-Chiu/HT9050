# Naming Map — Loader / Color / Empty Tray Group

## 氣缸常數對照

| 用途 | Loader | Color | Empty | 定義檔 |
|------|--------|-------|-------|--------|
| 升降主氣缸 | `C_Load_Up` (19) | `C_Color_Up` (21) | `C_Empty_Up` (23) | cmydef.cpp L374~378 |
| 中層氣缸 | — | `C_Color_Middle` | `C_Empty_Middle` | cmydef.cpp |
| Z 分離選擇 | `C_LoaderLoaderZ_Select` | `C_ColorLoaderZ_Select` | `C_EmptyLoaderZ_Select` | cmydef.cpp |
| 壓盤氣缸（僅 Loader） | `C_LoaderUpPress` (59) | — | — | cmydef.cpp L414 |

## 感測器常數對照

| 用途 | Loader | Color | Empty | 定義檔 |
|------|--------|-------|-------|--------|
| 上升安全偵測 | `SnLoaderUpSafedetect` (410) | `SnColorUpSafedetect` (412) | `SnEmptyUpSafedetect` (411) | cmydef.cpp L1287~1289 |
| CCW 到位偵測 | `SnLoaderCCWDete` | `SnColorCCWDete` | `SnEmptyCCWDete` | cmydef.cpp |
| Car 有 Tray | `SnLoaderCarHasTray` | `SnColorCarHasTray` | `SnEmptyCarHasTray` | cmydef.cpp |
| 3DL 有 Tray | `SnLoader3DL` | `SnColor3DL` | `SnEmpty3DL` | cmydef.cpp |

## 函式名稱對照

| 動作 | Loader | Color | Empty |
|------|--------|-------|-------|
| 升降主函式 | `DoSupplyNewICTray()` | `DoLoadNewColorTrayToCar()` | `DoLoadNewEmptyTrayToCar()` |
| 初始化 Task | `InitSupplyNewIC_From_LoaderCarTask()` | `InitLoadNewColorTrayToCarTask()` | `InitLoadNewEmptyTrayToCarTask()` |
| 移到前位 | — | `DoColorTrayToFront()` | `DoEmptyTrayToFront()` |
| 移到後位 | — | `DoColorTrayToRear()` | `DoEmptyTrayToRear()` |
| Auto 取料 | — | `DoAutoColor()` | `DoAutoEmpty()` |
| 卸盤/歸還 | — | `DoUnLoadNewColorToStack()` | `DoUnLoadNewEmptyToStack()` |
| 色感檢查 | `DoInspectTrayColorOnLoader()` | `ForTERAPOWERCheckColorSensor()` | — |
| 浮起檢測（僅 Loader） | `DoLoaderTrackDetectICFloating()` | — | — |

## 原始檔對照

| 機構 | 檔案 |
|------|------|
| Loader | `asendic_Loader.cpp` |
| Color | `asendic_Color.cpp` |
| Empty | `asendic_Empty.cpp` |

## JAM 報警碼對照

| 情境 | JAM碼 | 實際位置 |
|------|-------|----------|
| Color 升降逾時 | `JAM1413` | asendic_Color.cpp |
| Loader 升降相關 | `JAM1411` | acatchtray.cpp（非 asendic_Loader） |
| Empty/Color 相關 | `JAM1412` | asendic_Color.cpp（非 asendic_Empty） |

> ? JAM1411/1412 的實際位置與直覺不同，查詢時以 grep 原始碼為準。

## CylinderUp / Middle / Lower 操作對照

```cpp
// Color
if(CylinderUp(C_Color_Up))       // 升到最高位
if(CylinderMiddle(C_Color_Up))   // 中層位置
if(CylinderLower(C_Color_Up))    // 降到最低位

// Empty
if(CylinderUp(C_Empty_Up))       // 升到最高位
if(CylinderMiddle(C_Empty_Up))   // 中層位置
if(CylinderLower(C_Empty_Up))    // 降到最低位

// Loader
if(Cylinder[C_Load_Up].Push())   // 升到位（Loader 用 Push/Pop）
if(Cylinder[C_Load_Up].Pop())    // 降到位
// 另有 C_LoaderUpPress 獨立壓盤
```

## Loader 獨有機構

Loader 與 Color/Empty 的主要差異：
1. **壓盤氣缸** `C_LoaderUpPress` — Color/Empty 無此機構
2. **IC 浮起檢測** `DoLoaderTrackDetectICFloating()` — 僅 Loader 需要（因為 Loader 上有 IC）
3. **色感檢查** `DoInspectTrayColorOnLoader()` — Loader 在取料前驗證 Tray 顏色
4. **無前後位移動** — Loader 固定位置，不像 Color/Empty 有 Front/Rear 切換
