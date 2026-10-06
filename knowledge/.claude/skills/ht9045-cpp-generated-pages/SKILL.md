---
name: ht9045-cpp-generated-pages
description: >
  評估案（20260928，只評估不實作）：HT9045 V906 的六個硬體／馬達畫面（Main.MotorView、HW.home、HW.IoSetView、
  HW.MotorTest、HW.ShuttleMove、HW.teach）改用 **C++ 內建表單（原生 Win32 視窗）**，其餘畫面維持 HTML。
  Steven 的理由：這六頁 JSON 傳輸量大、與安全高度相關，要由 C++ 主導。主結論：走 A 案——dfm2rc 已產好的 .rc 對話框資源
  ＋復原 2026-08-17 刪掉的 GA-4 Win32 對話框引擎（git 可取回）＋訊息迴圈掛在 wb_serve 主迴圈（同 golden 單執行緒、符合
  1203 卡單執行緒規則）＋golden 處理器直接綁定；六頁估 50～73 人天；順序照 Steven 20260928 裁定 IO → MotorTest → MotorView →
  teach → home → ShuttleMove，IoSetView 拆 1a 唯讀／1b 輸出／1c 其餘；「用 define 隔開 C++ form／HTML form」可以，建議編譯期
  W906_NATIVE_FORMS＋執行期 system\NativeForms.ini 逐頁選（名稱為提案）——Steven 20260929 17:32 Q54：執行期逐頁選不做，只有編譯期開關、只有這六頁。
  附：JSON 流量與安全防線的逐頁量化、與 20260812「UI 用 web」定案的最窄例外條款、分期／驗收／風險、Steven 要決定的 Q-N1～Q-N9。
  另保留同日上午被否決的解讀（「C++ 產生 HTML」三選項評估）作為對照。
  Use when：Steven 問「這幾頁改 C++ 內建 form」「原生視窗」「不要走 web」「JSON 太多」「安全要 C++ 主導」；要查 vclcompat 是不是真視窗、
  dfm2rc 的 .rc 能不能直接用、GA-4 引擎在哪、wb_serve 有沒有訊息迴圈、1203 執行緒規則、六張表單事件翻譯進度；要估原生表單工作量、
  排試點、寫驗收；要盤點某頁的 JSON 流量與會動機台的按鈕；或回頭查「C++ 產生 HTML」為什麼不做。
  關鍵字：C++ 內建表單, 原生視窗, native form, Win32 對話框, DIALOGEX, dfm2rc, rc_out, emit_rc, GA-4, DialogTreeEngine,
  ApplyLayoutEngine, 7b86cfdf, vclcompat 替身, Controls.h, 訊息迴圈, PeekMessage, 主迴圈, Pci1203Control 單執行緒, 頁面表,
  page-state array, fShow, jog 按住, SetCapture, WebCmdGuard, motor.access, io.btnPanelClick, /api/struct/motor/runtime,
  /api/struct/io/runtime, JSON 流量, Origin, allowCmd, 20260812 定案, UI 用 web, 例外條款, HW.MotorTest, HW.teach, HW.IoSetView,
  HW.home, HW.ShuttleMove, Main.MotorView, uhome, uMotorTest, uteach, iosetview, ShuttleMove, 試點, 評估案, 方案。
---

# ht9045-cpp-generated-pages 相容入口

同主題已整合到 [hpi-web-hmi](../hpi-web-hmi/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-web-hmi/references/native/original-entry.md)

## 0. 先讀這一份

[讀取此節](../hpi-web-hmi/references/native/original-entry.md#0-先讀這一份)

## 1. 一句話結論

[讀取此節](../hpi-web-hmi/references/native/original-entry.md#1-一句話結論)

## 1.5 原型現況與每頁都要守的規則（20260929）

[讀取此節](../hpi-web-hmi/references/native/original-entry.md#15-原型現況與每頁都要守的規則20260929)

## 2. 檔案地圖

[讀取此節](../hpi-web-hmi/references/native/original-entry.md#2-檔案地圖)

## 3. 六頁一眼看（原生表單視角）

[讀取此節](../hpi-web-hmi/references/native/original-entry.md#3-六頁一眼看原生表單視角)

## 4. 三步方案（細節在主文件 §7）

[讀取此節](../hpi-web-hmi/references/native/original-entry.md#4-三步方案細節在主文件-7)

## 5. 要 Steven 決定的題（全文與例子在主文件 §7.6）

[讀取此節](../hpi-web-hmi/references/native/original-entry.md#5-要-steven-決定的題全文與例子在主文件-76)

## 6. 怎麼用這份 skill

[讀取此節](../hpi-web-hmi/references/native/original-entry.md#6-怎麼用這份-skill)
