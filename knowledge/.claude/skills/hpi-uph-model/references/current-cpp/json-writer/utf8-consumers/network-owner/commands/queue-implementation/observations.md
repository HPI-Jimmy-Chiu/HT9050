# 觀察值與統計口徑

[上層](index.md)；原文：[capacity](raw/source-01.md#capacity)、[size](raw/source-01.md#size)、[empty](raw/source-01.md#empty)、
[peakSize](raw/source-01.md#peaksize)、[acceptedCount](raw/source-01.md#acceptedcount)、[rejectedCount](raw/source-01.md#rejectedcount)、[drainedCount](raw/source-01.md#drainedcount)。

capacity／size／empty／peakSize各自取得WbGuard，再讀相應成員；capacity由建構固定，原comment要求同一讀取regime保留。
這些是逐次觀察，不是相互一致的一個snapshot；empty後的push或drain可能改變queue，不能拿觀察值當後續操作保證。
peak更新只在成功push持鎖區內，沒有本段reset；不是handler site數、批量上限或socket待回覆數。

acceptedCount／rejectedCount／drainedCount從各自`std::atomic<unsigned long>`load，再cast std::uint64_t。
64-bit回傳型別不擴大底層counter；原32-bit x86／i586／libatomic理由照原文保存，Windows unsigned long計數寬度與部署版本需分開。
這些counter可有寬度上限／回繞界線，不宣稱無限累積；也不能將鎖外統計與q.size組成永遠成立的方程式。
accepted為入列成功、rejected為入列拒絕、drained為搬出數；宿主執行成功與browser結果不是這三欄定義。
歷史MinGW6.3工具鏈comment不作目前編譯器版本結論；此輪不build、不執行Win32 event或量測延遲。
