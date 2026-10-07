# 共同證據與判讀

先確認State Record的來源日期、機台身分與運行版本，依Ver.txt、MainForm畫面或包內的Version資料交叉核對；沒有版本／欄位就列缺口。對照客戶實際版本的source，不能只用最新開發版或不同機型作結論。

讀包內system／config／recipe快照的CUSTOMER_CODE、Model、選配／ATC／測試模式等，再查Task／case。這是已提供記錄包的資料，不把開發機runtime當成客戶現場，也不為分析自動覆寫runtime。

MainProcMonitor有欄位才讀Alive、CallCount、LastEnter、SilentSec、SaveTime。比較LastEnter、最後正常Task變化與SaveTime；人工擷取的SaveTime不等於異常開始。Alive=N單獨不足以判當機，CallCount需多份擷取才能判持續變化；時間倒序、跨日、缺欄或單次快照要說明可信度。

正常等待、可疑延遲、疑似主迴圈停擺分開，附時間差與證據；不要固定套歷史案例的5秒門檻，先確認該版本writer及當時設定。CSV中的Task數相同不代表同一機構／條件。

要追程式時以實際版本的MainProc／ThreadProcess／Synchronize、Task／case與等待變數定位；目前V912為BCB6／Big5，V906移植樹為C++17／UTF-8。保存原文的「cpp皆Big5」是原workspace情境，不能覆蓋目前版本分流。

完整SOP見[原State Record入口](source/original-entry.md)與[analysis-sop](source/references/analysis-sop.md)；快速輸出見[原data-analysis](quick/original-entry.md)。歷史命令、模型假設與案例原樣保留，不能整理成全部版本已驗證的當前行為。

本次只整理Skill／文件及source導讀，沒有建立State Record、操作API／機台、執行備份還原或跑客戶案例。後續實機／模擬測試依專案入口與當次授權、同步與還原規則辦理。
