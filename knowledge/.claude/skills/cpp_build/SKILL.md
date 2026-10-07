---
name: cpp_build
description: V906 C++ 移植樹（HT9011UC_Cpp_V3.33.906.0）的建置、驗證與加速——build.bat 各模式、sim／ship 兩組態、「改動算不算完成」的標準、ccache／PCH、加快編譯的提案（實測瓶頸是打包與連結，不是編譯）、踩過的坑，以及大家分享的加速做法。跟 bcb_build（BCB6）並列。Use when：要編譯 V906 C++、覺得編譯太慢、要裝 ccache／Ninja、要判斷一個改動能不能交、gate／代跑之前自我檢查。關鍵字：build.bat, cmake, MinGW, g++ 6.3.0, WinLibs, ccache, PCH, Ninja, thin archive, OBJECT library, response file, 編譯速度, 加快編譯, 兩組態, build_ship, W906_NO_SOFT_SIMULTE, PE check, FShow_Audit, 完成標準。
---

# cpp_build 相容入口

同主題已整合到 [hpi-build](../hpi-build/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-build/references/cpp/original-entry.md)

## 1. 環境

[讀取此節](../hpi-build/references/cpp/original-entry.md#1-環境)

## 2. 怎麼建

[讀取此節](../hpi-build/references/cpp/original-entry.md#2-怎麼建)

## 3. 「完成」的標準——單一 cpp 編過 ≠ 模組完成

[讀取此節](../hpi-build/references/cpp/original-entry.md#3-完成的標準單一-cpp-編過--模組完成)

## 4. 加快編譯

[讀取此節](../hpi-build/references/cpp/original-entry.md#4-加快編譯)

### 4.1 先看實測：慢在打包與連結

[讀取此節](../hpi-build/references/cpp/original-entry.md#41-先看實測慢在打包與連結)

### 4.2 寫好了、但還沒接上的（`cmake/W906_FastBuild.cmake`，20260921）

[讀取此節](../hpi-build/references/cpp/original-entry.md#42-寫好了但還沒接上的cmakew906_fastbuildcmake20260921)

### 4.3 每台都裝 ccache（裝好備用；要等 4.2 接上才會生效）

[讀取此節](../hpi-build/references/cpp/original-entry.md#43-每台都裝-ccache裝好備用要等-42-接上才會生效)

### 4.4 提案（依效益排序；NB2 卡 (j)「機台編譯太慢」是負責的卡）

[讀取此節](../hpi-build/references/cpp/original-entry.md#44-提案依效益排序nb2-卡-j機台編譯太慢是負責的卡)

### 4.4a St01 1006 實測與 Steven 的決定（1006 14:0x）

[讀取此節](../hpi-build/references/cpp/original-entry.md#44a-st01-1006-實測與-steven-的決定1006-140x)

### 4.5 不要做

[讀取此節](../hpi-build/references/cpp/original-entry.md#45-不要做)

## 5. 踩過的坑

[讀取此節](../hpi-build/references/cpp/original-entry.md#5-踩過的坑)

## 6. 大家的好方法（請直接補）

[讀取此節](../hpi-build/references/cpp/original-entry.md#6-大家的好方法請直接補)
