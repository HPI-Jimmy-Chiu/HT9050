# E-09 機台畫面：被遮住或被截斷的視窗／頁面（版面實測）

> 量測：`tools/webprobe/e09_layout_probe.py`，無頭 Edge 開 background.html，視窗大小 1920x1032、畫面縮放 110%（main.html 🔍）。每個視窗用 openWin(id, true) 開一次、每個分頁都切過，量完關掉。假伺服器，碰不到 wb_serve 或機台檔。
> 樹：`D:\AI_TempFile\st01m-mainpush`。

## 有問題的視窗（1／1 個開得起來的視窗）

| 視窗 | 頁 | 框超出可見範圍（左,上,右,下 px） | 內容 vs 視窗 | 捲動 | 被裁掉的控制項 | 被蓋住的控制項 | 視窗本身被蓋 |
|---|---|---|---|---|---:|---:|---|
| setup | Setup.SetUp.html | 0,0,0,0 | 1074x948 in 1068x898 | x-scroll y-scroll | 20 | 2 | - |

## 明細（每個視窗最多列 12 個）

### setup（page/Setup.SetUp.html）
- 被裁掉 20：[Check Torque] label#chkOffCenterkit.ckb"NS7000 bias kit " by div.cli"NS7000 bias kit " (529,133 275x19 in 516,124 280x561) [box only]；[Check Torque] label#chkNS7000CS.ckb"NS7000 change so" by div.cli"NS7000 bias kit " (529,152 275x18 in 516,124 280x561) [box only]；[Check Torque] label#cbNS8000H.ckb"NS8000H change s" by div.cli"NS7000 bias kit " (529,226 275x18 in 516,124 280x561) [box only]；[Check Torque] label#chkOctal80.ckb"Octal site X pit" by div.cli"NS7000 bias kit " (529,296 275x18 in 516,124 280x561) [box only]；[Check Torque] label#cbOctal16Site.ckb"Octal site use 1" by div.cli"NS7000 bias kit " (529,314 275x18 in 516,124 280x561) [box only]；[Check Torque] label#cbOctal12Site.ckb"Octal site use 1" by div.cli"NS7000 bias kit " (529,332 275x18 in 516,124 280x561) [box only]；[Check Torque] label#cb16DirectHeater.ckb"One by one direc" by div.cli"NS7000 bias kit " (529,349 275x18 in 516,124 280x561) [box only]；[Check Torque] label#cb12Site10DirectHeater.ckb"12 site use 10 h" by div.cli"NS7000 bias kit " (529,367 275x18 in 516,124 280x561) [box only]；[Check Torque] label#cbSquareOctalLayout.ckb"2x2 site use 2x4" by div.cli"NS7000 bias kit " (529,243 275x18 in 516,124 280x561) [box only]；[Check Torque] label#cb2CableLayoutKit.ckb"Use 2 cable SLK" by div.cli"NS7000 bias kit " (529,420 275x18 in 516,124 280x561) [box only]；[Check Torque] label#cb1CableLayoutKit.ckb"Use 1 cable SLK" by div.cli"NS7000 bias kit " (529,438 275x18 in 516,124 280x561) [box only]；[Check Torque] label#cbQualSite2X2Shift.ckb"SLK shuttle X sh" by div.cli"NS7000 bias kit " (529,456 275x18 in 516,124 280x561) [box only]…
- 被蓋住 2：[Check Torque] span#Label4.lb"X Pitch :mm" under input#edPreciserXPitch.ed；[Check Torque] span#Label5.lb"Y Pitch :mm" under input#edPreciserYPitch.ed

