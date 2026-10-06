# 客戶與模式

目前 W906_NoteScreenStartActs 的畫面START在非SIM只放 CC_SIGURD_PeiXing，SIM回true；不影響面板START。W906_NoteWebKeyGate 的 CC_SCK 排除僅作用於它包含的Index掉料條件，不能放大成所有鍵或密碼免檢查。

[原文](source/original-entry.md)保存 D-034、SpecialPanel、GetJemUnlockPassWord、SECS/GEM S10F3的原條件與日期；重新查機台／客戶時應追 [目前main的檔名／function／變數](runtime/current-main.md)，不以名稱前綴或單一CUSTOMER_CODE推所有解除規則。

Yield 的 CC_JCET、CC_Greatek／CC_AMKOR_China／CC_QUALCOMM、bKoreaFunction 與 special low-yield 分支看 [目前Yield](yield/current-main.md)；這些是 DoLowYieldAlarm 的處置，不是所有 Note 畫面 START 客戶 gate。PTI 兩階段的完整原說明與 ClearYieldCount 影響保存在同一 Yield reference，勿只用客戶名稱推所有 Alarm 回應。
