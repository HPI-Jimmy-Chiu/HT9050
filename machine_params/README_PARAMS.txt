HT9050 機台參數快照（machine_params\）—— 2026-10-02 21:18
來源：HT9050 實機（裝 PCIE-1203 的那台）。程式版本：C++ 5825146 ORG-INV: on the 1203 branch the HOME lamp of Motor Test and Teach is inverted (and／web 365189d。
每次機台端推 GitHub 都會重拍一次（整個資料夾鏡像），所以這個分支的 git 歷史就是機台設定的歷史。

資料夾 → 放回機台的位置
  D_HT9045_system\  (561 檔)  → D:\HT9045\system\      機台正本：Gerneral.ini、IO_Table.csv、Mot_Table.csv（wb_serve 直接讀這三個）
  D_HT9045_config\  (53 檔)  → D:\HT9045\config\
  runcfg\           (56 檔)  → D:\HT9045\_integ_ioweb\runcfg\   SetUp.inf（目前工單）、system\teach.ini（教導值）、config\（config.ini、LastSet.ini、Pci1203*.ini …）；logs\ 沒放

注意
  * 這是 HT9050 這一台的設定。別台機台不要整包覆蓋：IO 對照或馬達表錯了，程式會照錯的對照推線圈、動馬達。
  * 放回機台前先備份原本的資料夾。
  * 內容照原樣，沒有遮掉任何值（含密碼檔；EastSun 20261002 裁決）。