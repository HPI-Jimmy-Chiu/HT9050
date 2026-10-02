---
description: "顯示週報/案件管理在 Claude Code（Hub 模式）的指令地圖、子代理、自然語觸發總覽。"
---

請顯示週報管理系統在 Claude Code（Hub 模式，操作 Weekly_AI 工作區）的完整指令地圖：

## 子代理（Task 工具自動路由，或指名呼叫）
- `weekly-report` — 週報與客戶異常 case 管理
- `ht9045-v899` — V899 程式根因分析與修正
- `case-coordinator` — 客戶異常案件三階段協調入口

## 斜線指令
| 指令 | 用途 |
|------|------|
| `/update-weekly` | 自然語更新工作進度 |
| `/weekly-case-intake` | 客戶新回報/重啟異常，建 case 並回報 `01_intake` 放檔路徑 |
| `/weekly-status` | 查詢狀態/未完成/健康檢查 |
| `/weekly-case-integrity` | 驗證未完成項目與 Customer case 一致性 |
| `/weekly-next-week` | 建立下週週報（已完成轉黑、日期推進）— 破壞性，先確認 |
| `/weekly-help` | 顯示此地圖 |

## 自然語範例（會自動路由到 weekly-report）
| 說法 | 效果 |
|------|------|
| 「今天是新的一周」 | 建立下週週報（先確認再執行） |
| 「力成PTI 反應 HT9045 異常，如圖」 | 建/重啟 case，回報 `01_intake` 放檔路徑 |
| 「甬矽 9016C OneByOne 已提供安裝包」 | 更新該事件進度 |
| 「這週有哪些未完成？」 | 列 open 項目 |
| 「幫我做健康檢查」 | 完整分析 |

## 資料位置（Weekly_AI 工作區，絕對路徑）
- 根目錄：`d:\Work-jimmychiu\document\WeeklyReport\Weekly_AI`
- 唯一真相：`weekly_data.json`；Excel 產出：`output/`
- 客戶 case：`Customer/<客戶>/<CASE>/01_intake~04_release/`
- 詳細 SOP：`weekly-case-flow` skill

## 狀態圖標
✅ done｜🔧 in-progress｜⏳ waiting｜📩 pending-response｜🆕 new
