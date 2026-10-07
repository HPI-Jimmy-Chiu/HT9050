# Jimmy：HT9050 點位資料已推上去（2026-10-07）

EastSun 交代：把 HT9050 實機的點位資料全部推上 GitHub，並通知你。

## 在哪裡

GitHub 分支 `machine/integ-ioweb`，資料夾 `machine_params\`（每次機台推送都整包重拍，這個分支的歷史就是機台設定的歷史；說明見 `machine_params\README_PARAMS.txt`）。

| 檔案（push 內路徑） | 機台上的位置 | 內容 |
|---|---|---|
| `machine_params\runcfg\system\teach.ini` | `D:\HT9045\_integ_ioweb\runcfg\system\teach.ini` | **教導點位（主要看這個）** |
| `machine_params\D_HT9045_system\Mot_Table.csv` | `D:\HT9045\system\Mot_Table.csv` | 馬達速度／加減速表 |
| `machine_params\D_HT9045_system\IO_Table.csv` | `D:\HT9045\system\IO_Table.csv` | IO 對照（含 Enable） |
| `machine_params\D_HT9045_system\Gerneral.ini` | `D:\HT9045\system\Gerneral.ini` | 機台總設定 |
| `machine_params\D_HT9045_system\lastdata.dat` | `D:\HT9045\system\lastdata.dat` | 最後狀態 |
| `machine_params\runcfg\config\` | `D:\HT9045\_integ_ioweb\runcfg\config\` | config.ini、LastSet.ini、Pci1203Axis.ini、Pci1203Io.ini … |
| `machine_params\D_GPIB9045_system\` | `D:\GPIB9045\system\` | general.ini `[Version] Model=9050GPIB`（開 HT9050 分支） |

這是 HT9050 這一台的值，別台不要整包覆蓋；放回前先備份。

## 重點值（teach.ini，2026-10-07 下午）

**9050 Tray Z（Teach 頁「Tray Z 9050」區塊，五站相同）**
- Loader / Empty / Auto1 / Auto2 / Auto3：Up 21000、Down 20000、Pitch 1900、Home 0、Lift 9500
- Loader 探測：ProbeStart 2、ProbeLimit 5
- key 名稱：`setEditTZ9050{Loader,Empty,Auto1,Auto2,Auto3}{Up,Down,Pitch,Home,Lift}`

**Tray X `[MTrayX]`**
- Loader 191213、Empty 732、Auto1 140718、Auto2 94735、Auto3 46762

**Index Z 對 Shuttle**
- In Shuttle：`setEditIndex1ToSht1Z = -4357`
- Out Shuttle：`[MTestZ1] setEditIndex1ToOutSht1Z = 0`（EastSun 10/07 更正：Index Z 高度在其他頁面已可設定，Teach 頁不需要改；Out Shuttle Z 控制項維持套件 149 的原位置）

**Shuttle 其他**
- In Shuttle1：X -10819、Y 113021、Left 0、Right -69123
- Out Shuttle1／2：X 15471、Y 53278

## 目前機台上的暫時設定（之後要改回）

- IO_Table：安全門 9 列、落料氣缸 Enable 設 0（EastSun 要求暫時關閉）。
- Gerneral.ini：SafePlcIO=0（EastSun 自己改的）。
- Mot_Table 有 4 列的 InitSpeed 大於 JogHigh，等 EastSun 修正：
  - MInArmY：50000 > 20000
  - MOutShuttle1：40000 > 6000
  - MOutShuttle2：10000 > 6000
  - MOutArmX：20000 > 15000
