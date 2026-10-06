# 相鄰主題

- [Shuttle](../../ht9045-shuttle-flow/SKILL.md)：入料放料互鎖、HT9050獨立In／Out與M18出料Y。
- [Index](../../ht9045-index-flow/SKILL.md)：pick／place互鎖與W-44裁決／實作。
- [Tray](../../ht9045-catchtray-flow/SKILL.md)：Loader／空盤／CatchTray交接，InArmXYZSafe判定。
- [OutArm](../../ht9045-outarm-flow/SKILL.md)：出料流程與鏡像基準，不能由InArm反推。
- [Sucker／IO](../../ht9045-sucker-architecture/SKILL.md)／[IO](../../ht9045-io-control/SKILL.md)：物件拓樸、裝置路由、感測與輸出。
- [Temperature](../../ht9045-temperature/SKILL.md)：Heat／Soak與現況接入；HotPlate料帳不是溫控器狀態。
- [State Record](../../ht9045-state-record-analysis/SKILL.md)：函式／Task／關鍵變數與原案例因果鏈。

以上用保留的相容舊名路由，避免依賴另一Draft尚未合併的新路徑。跨主題同一案例以原裁決與版別為準；本批不更動其他入口。

## ASM原入口缺少

舊原文引用ht9045-asm-flow，但目前Git未收錄這支入口。保留原文依據並導向本Skill完整ASM／HotPlate帳本章節，不宣稱缺少的原稿已搬移或全量驗證。

## 模式命名原稿缺少

舊幾何文件引用ht9045-mode-naming-convention.md，目前Git未找到該稿。先依DoInArm_9045_Type／iInArmType與已保存的Type推導查實際配置，不以缺稿推各機型相同。
