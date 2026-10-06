# Bin Display機型分流

| 機型／配置 | 共用點與差異 |
|---|---|
| HT9050 | NUMBER_PANEL／MAGAZINE型態、兩條COM與實際面板接線分層；同一Skill管理TFT／七段差異 |
| HT9050裁決 | RULINGS_20261003第19條：type3改4、目標COM14；先核對實際裝置編號，第二埠同一COM以免誤占COM4，機台端修改後再鏡像 |
| 目前Git快照 | 本批讀main的Gerneral.ini：NUMBER_PANEL_TYPE=3、[NUMBER_PANEL] COM_PORT=COM11、[NUMBER_PANEL2] COM_PORT=COM4、MAGAZINE_BIN_DISP_TYPE=0。這與上述裁決不同，但不是實機測試／故障結論；不替機台端改快照或執行期 |
| 多盤／Magazine配置 | AUTO_EMPTY_COLOR、AUTO3_IS_MAGAZINE、SUPPORT_2_EMPTY_EMPTY決定格位；type4不等於有Magazine，也不能只因generic有Fix就說9050有Fix |
| 其他Handler | type1／2 DIO、type3七段／兩埠、type4 TFT按配置；與HT9050同一主題入口，依references區分 |

- [裁決來源](../../../../../HT9011UC_Cpp_V3.33.906.0/docs/RULINGS_20261003.md)第19／20條。
- [本批Git快照檔](../../../../../machines/HT9050/snapshot/machine_params/D_HT9045_system/Gerneral.ini)／[快照來源與時間](../../../../../machines/HT9050/snapshot/SNAPSHOT_SOURCE.md)。
- [原硬體／協定](../source/references/hardware-and-protocol.md)／[原機型資料](../source/original-entry.md)。
