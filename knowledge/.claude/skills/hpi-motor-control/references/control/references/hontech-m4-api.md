> 保存來源：`.claude/skills/ht9045-motor-control/references/hontech-m4-api.md`，main `eb7f47e7f`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# Hontech_M4 — 泓格 MotionNet M2X4 C API 參考

> 對應原始碼：`Motor/Hontech_M4.cpp`、`Motor/Hontech_M4.h`  
> 硬體：ICP-DAS MotionNet M2X4 / M1X4 4 軸馬達驅動卡

---

## 概述

C 語言 DLL 函式介面（非 C++ 類別），控制 ICP-DAS MotionNet M2X4/M1X4 4 軸馬達驅動卡。原生支援 2/3/4 軸直線插補、T-Curve + S-Curve 平滑控制及位置觸發同步。

---

## 模組資訊

| 項目 | 說明 |
|------|------|
| 標頭檔 | `Motor/Hontech_M4.h` |
| 實作檔 | `Motor/Hontech_M4.cpp` |
| 硬體 | 泓格 MotionNet M2X4 / M1X4 |
| API 類型 | C 函式（前綴 `_Hon_m4_`） |

---

## API 函式

### 初始化

```cpp
_Hon_m4_initial(cardNo, axisNo);
```

### 單軸移動

```cpp
// 絕對位置移動
_Hon_m4_start_a_move(cardNo, axisNo, pos, speed);

// 相對距離移動
_Hon_m4_start_r_move(cardNo, axisNo, dist, speed);
```

### 多軸直線插補（Trapezoid 曲線）

```cpp
_Hon_m4_start_tr_line2(cardNo, ax1, ax2, pos1, pos2, speed);
_Hon_m4_start_tr_line3(cardNo, ax1, ax2, ax3, p1, p2, p3, speed);
_Hon_m4_start_tr_line4(cardNo, ax1, ax2, ax3, ax4, p1, p2, p3, p4, speed);
```

### 多軸直線插補（S-curve 平滑曲線）

```cpp
_Hon_m4_start_sr_line2(cardNo, ax1, ax2, pos1, pos2, speed);
```

### 回原點

```cpp
_Hon_m4_start_home_move(cardNo, axisNo, speed, mode);
_Hon_m4_set_home_config(cardNo, axisNo, config);
```

### 位置讀寫

```cpp
_Hon_m4_get_position(cardNo, axisNo);
_Hon_m4_set_position(cardNo, axisNo, pos);
```

### 運動狀態

```cpp
_Hon_m4_motion_done(cardNo, axisNo);
```

### Latch 機制

```cpp
_Hon_m4_set_ltc_logic(cardNo, axisNo, logic);
_Hon_m4_get_latch_data(cardNo, axisNo);
```

### 同步運動

```cpp
_Hon_m4_sync_a_move(cardNo, axisNo, pos, speed);
_Hon_m4_sync_r_move(cardNo, axisNo, dist, speed);
_Hon_m4_set_triggered_a_move(cardNo, axisNo, pos, speed);
_Hon_m4_set_triggered_r_move(cardNo, axisNo, dist, speed);
```

---

## 特性

- **直線多軸插補**：支援 2/3/4 軸組合
- **T-Curve**：梯形速度曲線，適合一般移動
- **S-Curve**：平滑 S 型速度曲線，減少機構衝擊
- **位置觸發同步**：在特定位置觸發下一個運動

<!-- preserved-content:end -->
