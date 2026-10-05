---
description: >
  客戶代碼縮寫對照表（自動套用）。涵蓋問題編號 CUSTOMER 欄位縮寫、
  客戶內部代碼、地區、代理商對應。任何涉及客戶名稱、代理商對應、
  問題編號縮寫的場合均自動生效。
applyTo: "**/*"
---

# 客戶代碼對照表

> 20261005 St01：從 `D:\.github\instructions\customer-code-table.instructions.md` 搬進來（`D:\.github` 退場）。
> 原檔是 Copilot 的 instructions（`applyTo: "**/*"` 自動套用）；放在這裡之後**不會自動套用**，
> 要用時由 make-report-skill／customer-code-manager／ht9045-customer-code-manager 指過來讀。檔名刻意保持原名。

> **此為唯一來源（Single Source of Truth）。**
> 問題編號格式 `#[T][YYMMDD]-[CUSTOMER]-[MC]-[NN]` 中的 `CUSTOMER` 欄位、
> 代理商名稱判斷、路由目錄解析，均以本表為準。

## 客戶縮寫表

| 縮寫 | 客戶（內部代碼） | 地區 | 代理商 |
|------|-----------------|------|--------|
| `ATK` | Amkor Korea（971_AMKOR_Korea） | 韓國 | TeraTech Korea |
| `SCK` | SCK（947_SCK） | 韓國 | TeraTech Korea |
| `HANA` | Hana Micron（865_HANA_MICRON） | 韓國 | TeraTech Korea |
| `TESNA` | Doosan Tesna（843_DoosanTesna） | 韓國 | TeraTech Korea |
| `ASECL` | ASE Chungli（933_ASE_CL） | 台灣 | HPI-TW（直接） |
| `KYEC` | KYEC Lee（921_KYEC_LEE） | 台灣 | HPI-TW（直接） |
| `TFAMDM` | TFAMD Malaysia（807_TFAMD_M） | 馬來西亞 | HTS |
| `AMDUS` | AMD US（808_AMD_US） | 美國 | HPI-USA |
| `AMDSG` | AMD Singapore（982_AMD_SG，原 982_AMD_M 改名） | 新加坡 | HTS |
| `ELMNT` | Element Israel（814_Element_Israel） | 以色列 | HPI-EURO |
| `TFAMDSZ` | TFAMD 蘇州（898_TFAMD_SUZHOU） | 中國 | 鴻勁興業 |
| `FMSH` | 上海復旦微電子（884_FMSH） | 中國 | 鴻勁興業 |
| `JSCC` | JSCC（943_SCC） | 中國 | 鴻勁興業 |
| `VTEST` | V-Test Shanghai（919_VTEST_Shanghai） | 中國 | 鴻勁興業 |

## 代理商地區語言對應

| 代理商 | 地區 | 語言配置 |
|--------|------|----------|
| TeraTech Korea | Korea | 英文 + 韓文 |
| HPI-TW | 台灣 | 繁體中文 |
| HPI-USA | USA / Canada | 英文 |
| HPI-EURO | Europe | 英文 |
| HTS | Singapore / Malaysia | 繁體中文 + 英文 |
| JB-Elite | Philippines / Thailand | 英文 |
| Spandnix | Japan | 繁體中文 + 日文 |
| 鴻勁興業 | 中國 | 繁體中文 |
| ETC | —（無法判定） | — |

## 維護規則

- **新增客戶**：取英文縮寫（通常 3~6 碼大寫），確認不與現有縮寫衝突後插入縮寫表
- **維護流程**：依 `customer-code-manager` SKILL 的步驟執行
- 問題編號格式規則詳見：[`issue-number-format.md`](../issue-number-format/issue-number-format.md)
