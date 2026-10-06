# HT9050 與其他機型

告警共同資料與權限路徑共用，配置差異在同一Skill記錄：

| 配置 | 差異／核對點 |
|---|---|
| HT9050 | 前端MACHINE.id／machine參數選9050頁與mv9050；原硬體資料記八道門、Index開檢單獨定位、無獨立Rotator |
| HT9045／其他Handler | 對應mv9045或該機台layout；原9045資料記十道門，料車／Magazine／Auto3選配依實際配置判定 |
| 共通 | position對機構位置，Code對單元與文本；不把C++軸名／UIcaption／傳入position當彼此可互換 |

門數／方位與料車的原始硬體知識有日期，不能由開發機CSV推出現場裝配；遇到機台設定問題先比對最新snapshot。 [HW知識](../../../ht9050-hw/SKILL.md)、[MotionView](../../../hpi-motionview/SKILL.md)與[保存原文](../source/original-entry.md)分別提供硬體、圖面與歷史依據。

## HT9050 扭力等待逾時與共用告警

目前 main 的 HT9011UC_Cpp_V3.33.906.0/Ht9050TorqueWait.cpp::W906_Ht9050TorqueWaitTimedOut 在 SOFT_SIMULTE 返回 false，非SIM先核對 MOT[MTestZ1].CardType==PCI1203，並以 Eligible 讀 SystemStart／SoftStop／fAllMotorHome／iHome。W906_Ht9050TorqueWaitAlarm 清 chkReadTorque1/2，呼叫 ShowErrorMessage；kAlarmCode 目前使用 WAR0361／WAR0362，kcode 依 CosFunction.bIndexAreaOnlyCanUseSkip 決定。

同 main 的 atester_FinePitch.cpp 在等扭力路徑已有 helper 呼叫，逾時退出設 fAllMotorHome=false、Task=1。HT9050 硬體 reference 所載「E-044 側分支未進main」是帶日期的舊狀態，不能覆蓋目前這份來源碼核對；程式中的 alarm code／kcode TODO 仍保留，不以 Skill 整理代作人員裁決。此告警借用既有 contact torque monitor 文本，和原本偏差檢查的同碼 trigger 要分開。

E-045 驅動器 ERROR_STOP／清警報／重新HOME是另一條機台安全路徑；[原硬體知識](../../../ht9050-hw/SKILL.md)保存裁決與歷史，回答現在狀態仍須另查其 caller、home 狀態與 card gate。本批不把舊 review 記錄宣稱為本次實機驗證。

目前同版本的 EcatAlarmScan.h::W906_EcAlarmStep 按 TEcAlarmIn 的 ht9050／isEcatRow／enable／readOk／pending／outOfPower／homing／armed 判斷。首次警報記住 episode 的 armed；未完成完整HOME時只 raise、不由此helper reset／latch；HOME運行中也不重複reset。已armed時先試一次reset，後續有效sample仍警報才 latch，直到無警報sample或power-out結束episode。不要把 A.9xx 警告、沒有fresh sample、單次poll未回來、HOME前與清不掉的ERROR_STOP混成同一條件；這是helper靜態查證，未驗現場驅動器。
