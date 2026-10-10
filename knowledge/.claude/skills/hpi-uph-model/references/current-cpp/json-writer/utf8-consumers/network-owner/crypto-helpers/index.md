# 握手用SHA-1與Base64 helper

[回socket owner](../index.md)。V906 `WebBridge/Sha1.cpp/.h`、`Base64.cpp/.h`；同一hpi-uph-model樹的握手支援文件。

| 問題 | 入口 |
|---|---|
| Reset／Update／Final、raw digest與hex | [SHA-1狀態及byte口徑](sha1.md) |
| 標準alphabet、padding、decode與out保留 | [Base64編解碼](base64.md) |
| 原key字串＋GUID、額外key gate與adapter | [握手caller](callers.md) |
| 來源／metadata、去重、機型版本與驗證界線 | [證據](evidence.md) |
| 13完整function map與4全文 | [manifest](source-manifest.json)／[有限census](symbol-census.json) |

新增13完整CPP／289函式原文行；四整檔423行保存在4原文頁、4context credit0。Rol inline與宣告不增完成數。
原文：[Sha1.cpp](raw/source-01.md)、[Sha1.h](raw/source-02.md)、[Base64.cpp](raw/source-03.md)、[Base64.h](raw/source-04.md)。
既有握手checker、ComputeAcceptKey及owner沿用，不能把helper保存當browser、HTTP服務或UPH產能已驗證。
原檔RFC編號與安全用途限制完整保留；本段不新增RFC符合性或實機驗證結論。
