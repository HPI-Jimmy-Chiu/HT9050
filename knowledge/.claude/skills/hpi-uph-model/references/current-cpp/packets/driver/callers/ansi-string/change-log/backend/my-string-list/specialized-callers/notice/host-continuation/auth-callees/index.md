# Notice auth callee：模式、解除資格與登入狀態

[上層 host](../index.md)；[AuthVerify](../auth-verify.md)。
來源 `144eba7f7b6c3b0e9a5fe1e77912c40ea4ee9b2c`，新增10完整CPP、2選定dispatcher區段。
host四個字面命中含兩份prototype，實際call兩處；不把兩段當完整父函式。

| 讀取問題 | 入口 |
|---|---|
| currentId、blocking與CompleteCommand | [Host dispatcher](dispatch.md) |
| unlock／Press／DoPassword、等級與PTI | [密碼與資格](password.md) |
| 密碼本讀取、mode與raw JSON欄位 | [Book與狀態](state.md) |
| 版本／客戶／機台及仍待查項 | [證據界線](evidence.md) |
| 原文、hash、舊選段覆蓋與分頁 | [來源 manifest](source-manifest.json) |
| 2061來源、11符號詞法查找 | [普查資料](symbol-census.json) |

這是局部reference完成；完整權限鏈、外部reader、timer／DB及UPH S8仍未完成。
沒有讀取帳密／權杖或執行登入、檔案載入、FTP、硬體、runtime、build或測試。

## 接續局部：一般登入與JSON raw writer

本次來源 `367d9d85792fa756db6e898c950d6d79931cbf74`；四完整callee與先前版本pin分開保存。
按stOperatorClick／BookLogin／RawValue／BeforeValue查[接續入口](login-json/index.md)；
客戶分流、權限清零／Logout與Key／comma／正文界線見子頁。
