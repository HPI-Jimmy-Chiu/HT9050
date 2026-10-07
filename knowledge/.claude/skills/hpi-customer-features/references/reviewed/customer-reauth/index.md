# 客戶碼Web存檔：reauth的頁面分流

本樹是S8局部靜態查證，來源釘住main `de7c2bf77`。補Web reauth的解析、頁面對照、暫存清除與ack caller，不宣稱完整登入／owner／磁碟效果已核對。

| 問題 | Reference |
|---|---|
| payload條件、point對照與HSys差異 | [解析與頁面](parse.md) |
| 清除時序、回覆與未完成部分 | [清除與回覆](clear-ack.md) |
| 4份來源blob／查證界線 | [來源manifest](source-manifest.json) |
| 既有Web閘與proxy／INI | [Web入口](../customer-web-save/index.md)／[proxy與INI](../customer-storage/index.md) |

HT9050與其他Handler使用同一份Skill，機型／客戶／runtime仍分流。V906 Web的tag／reauth契約與V912表單不能按名稱推定等效；V912前段客戶與存檔caller沿用各自舊來源。本次沒有新增客戶差異列，原12列／6382候選／846未決與metadata／原文／來源日期保持。

- [接續實際 callee 的答案消耗與權限結果（局部）](../customer-reauth-callee/index.md)
