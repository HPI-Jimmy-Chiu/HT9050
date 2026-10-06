> 保存來源：`.claude/skills/ht9045-config/references/config-help-json.md`，main `f57d93f15`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# Config 說明資料（HTML 模擬）

`page/Config.Configuration.html` 的 `MemoA` 到 `MemoP` 只讀
`JSON/Config-help.json`。此 JSON 由 Config skill 的 YAML 與 i18n 翻譯檔產生；HTML 不得讀取
`.ini`、`.dfm` 或 YAML。

## 編輯方式

1. 在 `data/<群組>/<區段>.yaml` 指定 `ui` 元件、`variables`、可選的 `ecid` 和 `ec_type`。
2. 在下列七個檔案以相同 key 填寫內容：`caption_id`、`desc_id`、`when_to_use_id`、`warning_id`、`typical_value_id`。
   `i18n/en.yaml`、`i18n/zh-TW.yaml`、`i18n/vi.yaml`、`i18n/ja.yaml`、`i18n/ko.yaml`、`i18n/id.yaml`、`i18n/th.yaml`。
3. 先執行 `D:\AI_TempFile\_gen_config_help_json.py`，再執行
   `D:\AI_TempFile\_gen_json_shim.py`。

範例：

```yaml
# data/A/A99.yaml
section: A99
group: A
caption_id: A99.caption
desc_id: A99.desc
when_to_use_id: A99.when
warning_id: A99.warning
typical_value_id: A99.typical
ui:
  -
    component: cbA99
    type: TCheckBox
    role: master
ecid: 35099
ec_type: ECBool
```

```yaml
# i18n/zh-TW.yaml（其他六個語系使用同一個 key）
A99.caption: "功能名稱"
A99.desc: "此功能的操作與效果說明。"
A99.when: "需要此生產條件時啟用。"
A99.warning: "啟用前確認相關硬體或流程。"
A99.typical: "預設關閉。"
```

未填的語系會回退至英文。頁面僅顯示其關聯 UI 元件在 `Config.json.uiMap` 中有至少一個可見變體的段落；因此顯示決策仍以 BCB6 `cConfiguration.cpp` 產生的設定資料為準。
<!-- preserved-content:end -->
