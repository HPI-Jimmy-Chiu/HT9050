---
name: hpi-motionview
description: "Handler MotionView共用資料與機型差異，整合HT9045／9046多吸嘴模板與HT9050單吸嘴、Z1 Index、Out Y氣缸、layout／概念動畫／正式LIVE投影。分析MotionView參數契約、HTSettings歷史與目前wb_serve HTTP即時讀、unknown／等待、吸嘴與料件守恆、SIM／LIVE、幾何、腳本重生與驗證時使用。"
---

# Handler MotionView

推送基準與後續裁決：[最新main整合](references/main-integration-20261006.md)。

同一Skill整合HT9050與其他機台的共同資料語意及機構差異；入口只選路，細節與原稿放reference樹。

## 先確認

- 先讀 [共同與差異](references/common.md) 與 [目前main接線](references/runtime/current-main.md)；舊JSON-only／直接拖檔／C#通道是保存架構，不能取代目前正式路徑。
- 概念／離線模擬與正式LIVE投影分開；沒發布在籍／Task／整機狀態時顯示未知或等待，不自行推算真實狀態。
- 動畫幾何、layout估值與真實teach座標分開；HT9050的單Z與Out Y二值氣缸不能套9045多軸插值。
- 舊9050 build重生會覆蓋目前B路與軸讀數接線；先按來源／生成對照，不能直接照歷史「必跑」指令重產。
- 本次整理只改Skill Markdown；scripts／assets／HTML範例原位保留，不操作機台或改執行期設定。

## 按問題選路

| 問題 | Reference |
|---|---|
| 正式資料路徑與unknown語意 | [執行期對照](references/runtime/current-main.md) |
| HT9045／HT9050機構與layout差異 | [機型樹](references/machines/index.md) |
| 參數、配位、吸嘴與顯示守恆 | [模板與算法](references/template/index.md) |
| 9050概念動畫／JSON歷史契約 | [layout樹](references/layout/index.md) |
| LIVE／StateRecord與歷史模式 | [模式界線](references/runtime/current-main.md) |
| 原腳本、assets與驗證條件 | [資源及驗證](references/resources.md) |
| UPH與相鄰主題／缺少來源 | [相鄰主題](references/related.md) |

## 查證與交付

核對資料producer→欄位／軸映射→顯示／unknown，再依機型與模式追reference。保存原來源日期、裁決與部署界線；每批核對main、附Change Log，文件檢查不冒稱動畫或機台實測。
