# 適用機型與仍待閉合

同一[HT9050機型樹](../../../../../machines/ht9050/index.md)與[其他Handler](../../../../../machines/ht9045.md)共用UPH Skill。這次只查V906所選9050取料body及上層case250；一般DoCatchFromLoader完整body、V912取料、所有caller／機型分派沒有查完，不能推所有Handler同取料或同計數。

兩完整取料函式文字已讀；完整DoCatchTray只保存body hash，讀的是短入口／case250。source裡的Frank／EastSun歷史註解保留，不升格成本批實機或裁決重審。

安全互鎖、馬達／氣缸／感測器、教導與W906_TrayZ9050 helper、所有旗標／層數writer、counter／CalculateUPH鏈、SECS送達、Task5200後續、thread時序、容量／site／校正待補；沒有C++／build、程式、IO、API／LIVE、Home、機台或runtime操作。

回[取料入口](index.md)、[前層dispatch](../index.md)與[來源](../../../../../resources.md)。
