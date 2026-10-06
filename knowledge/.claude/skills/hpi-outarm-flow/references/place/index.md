# 搜盤／放料／Bin reference樹

- [Destroy、放料與後處理](../flow/references/outarm-place.md)。
- [Bin搜尋／分類](../flow/references/outarm-bin.md)。
- [完整Fix Tray Full、E90與換盤路線](../flow/original-entry.md)。
- [Fix3氣缸誤報原調查](../flow/references/fix3-cylinder-jam1940-false-alarm.md)：保留modal／bHangTimePause與Index時序前提，不能只看1～2秒就斷言所有JAM1940都是誤報。

DoOutArmPlaceToAuto_9045、DoOutArmPlaceToAuto、DoOutArmAfterPlaceToAuto與DoSortingBinTray是不同階段；搜到盤、資料交換、Destroy、計數與換盤不能合稱放料已完成。HT9050沒有Fix的裁決查機型層。
