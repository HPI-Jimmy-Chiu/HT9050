# 原資源與驗證界線

scripts、assets、HTML範例原位保留，避免移動破壞相對依賴。

| 資源 | 路徑／用途 |
|---|---|
| 9045模板 | [motionview-template.html](../../ht9045-motionview-html-ui/assets/motionview-template.html) |
| 9045離線範例 | [HHT139完整範例](../../ht9045-motionview-html-ui/example/HT9045W_HHT139_MotionView_Full.html) |
| 9045 smoke | [smoke-test.js](../../ht9045-motionview-html-ui/assets/smoke-test.js) |
| 9045離線抽參數 | [extract_params.py](../../ht9045-motionview-html-ui/scripts/extract_params.py) |
| 9050概念守恆 | [verify-concept.js](../../ht9050-motionview-layout/scripts/verify-concept.js) |
| 9050舊重生／檢查 | [regen-and-check.ps1](../../ht9050-motionview-layout/scripts/regen-and-check.ps1) |
| 9050舊build | [build-main-motionview9050.py](../../ht9050-motionview-layout/scripts/build-main-motionview9050.py) |

本批只驗證文檔、原文保存、metadata、連結及資源不變，未跑HTML動畫、重生腳本或瀏覽器驗證。
日後修改HTML需讀原驗證清單與目前生成限制；不為純文件整理執行會寫page／JSON或執行期設定的腳本。
原`verify_animation_html.py`已在原稿標示不存在，不列為可執行檢查。
