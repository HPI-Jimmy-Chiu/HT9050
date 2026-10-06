# Generated maps 與 caller

兩份生成表維持原路徑及內容，由此直接引用，不建立第二份權威或把舊檔改成路由：

- [ShowErrorUnit panel map](../../../ht9045-alarm-dismissal/references/showerrorunit-panel-map.md)：unit到panel、mv與遮蔽。
- [Unmapped units](../../../ht9045-alarm-dismissal/references/unmapped-units.md)：分類及人工確認欄。

scripts/gen_alarm_unit_map.py 的 REF 直接指向 D:/HT9045/.claude/skills/ht9045-alarm-dismissal/references/unmapped-units.md，讀人工確認欄；scripts/sim_alarm.py 也指向panel map。保存這個caller界線。本批不執行或更改生成器，不重生web/JSON，不改人工裁決欄。

若日後確實要搬表，需同步改並驗證caller，屬另一批工作；本次先保留原文與tool可讀路徑。
