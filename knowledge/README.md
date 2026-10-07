# knowledge/ —— 筆電的知識（skill、規則、交接檔），明文鏡像

> AI(W906-KNOWLEDGE-GH) 20261002：Jimmy 1002 22:4x §0 第 61 項＝**C**（`RULINGS_20261002.md` 第 23 條）：筆電的 skill／知識明文放公開 GitHub，讓 ES02 與機台端的 Claude 跟筆電有一樣的知識。

* 來源：GitLab main `a4d5d4d5`（每次出更新包時整個資料夾重鏡像；GitLab 上刪掉的檔這裡也會消失）。共 3636 檔、39288 KB。
* 內容：`.claude/skills/`、`.claude/agents/`、`.claude/commands/`、`CLAUDE.md`、`AGENTS.md`、`docs/handoff/`（交接檔，main 上的版本）、`HT9011UC_Cpp_V3.33.906.0/docs/RULINGS_*.md`（使用者的裁決）。
* 怎麼用：把 `knowledge/.claude/`、`knowledge/CLAUDE.md`、`knowledge/AGENTS.md` 複製到你自己的工作樹根目錄（先備份你自己的同名檔）；Claude Code 開新 session 時就會讀到。這裡只是給人與 AI 讀的知識，**不是程式**——程式照舊走 `updates/<rev>/` 更新包。
* 推送前掃過：權杖、私鑰、共用區交付 7z 密碼＝0 筆（7z 密碼照規定不放 GitHub）。程式與設定檔裡的測試密碼照 RULINGS_20261001 第 40 條照常放。
* ⚠ 交接檔裡原本寫著共用區交付包的 7z 密碼（給同事的通知），**鏡像版已遮成「〔交付 7z 密碼：已遮…〕」**；要密碼請問 Jimmy（GitLab 上的原檔沒動）。
