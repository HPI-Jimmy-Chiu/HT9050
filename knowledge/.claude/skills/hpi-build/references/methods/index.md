# 加速、量測與隔離評估

- [原加速方法表](../cpp/references/speedup-ideas.md)：Ninja／thin archive、SSD、只建目標、ccache／PCH、PE與防毒觀察，各列保留機器、日期、組態／目錄及前後量測。
- [標頭瘦身與Motor／IO隔離原評估](../cpp/references/header-slimming-and-motor-io-isolation-20261006.md)：原實測／估計／proposal層次保持，Motor／IO另接[馬達主題](../../../hpi-motor-control/SKILL.md)／[IO主題](../../../hpi-io-control/SKILL.md)；沒有把DLL／標頭拆分當已實作。
- [原cpp_build正文](../cpp/original-entry.md)：PCH映射失敗、CMAKE_PROJECT_INCLUDE、W906_FastBuild未掛與compiler切換前對照要求。

要量測才另按當次授權選隔離建置目錄、同commit／旗標／target與負載對照；沒有量過就標未量。既有機台HDD與St01 NVMe數字不能直接承諾這台效益，實際generator／cache與linked targets也需確認。此批沒有安裝Ninja／ccache、套PCH、瘦身標頭、改Motor／IO或重跑bench。
