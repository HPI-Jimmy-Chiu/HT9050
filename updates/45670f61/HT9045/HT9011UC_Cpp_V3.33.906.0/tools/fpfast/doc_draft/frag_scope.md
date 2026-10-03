1. **EastSun 的 -O2 線需要一個決定**（不擋本次）。§9.3 矩陣：**沒有任何一種 -O2 組合在 17 個情境全部等於 BCB6 `-Od`**——
   * 今天的設定 `-O2 -ffloat-store -fexcess-precision=standard`：L／R／S 錯 12 項（字面值以 long double 比）。
   * 本次之後的實際效果（`+ -fexcess-precision=fast` 排在後面而勝出）：L／R／S 全對，但 `-ffloat-store` 在 fast 模式下**連運算式的暫存值都捨入成 double**，
     X1／X3／X4 錯——X1 正是 `adam6024.cpp:450`／`:739`／`:770`／`:774` 的 `dDiameter==fDiameter*10` 形狀（BCB6 對 5.6 kit 永遠對不到，這條線會對到）。
     `-O0 -ffloat-store -fexcess-precision=fast` 一樣（所以是 `-ffloat-store` 造成，不是 -O2）。
   * 拿掉 `-ffloat-store`（`-O2 -fexcess-precision=fast`）：L／R／X 對，S 錯（§8.4 那一類：算出來的值留在 80-bit 暫存器；已知四處有 A3 容差）。
   * -O0 的線（機台 F5 的 `build_integ_ship_x86`／`build_integ_dbg_x86`、`build_nonoracle`）加了旗標之後 **17 項全對**。
2. 這個旗標只管「字面值與運算式以什麼精度求值」，**不管 -O2／-O3 把算出來的值留在 80-bit 暫存器**（§8.4 的 S 類；fast 正是允許它的模式）。
   §8.4／§8.5 的 A3 容差、St02 在 `Adam6024Pressure_St02.cpp` 的 A3 與 `(double)` 轉型都**照留**（裁決原文）。
   oracle 的 Release（-O3）在 S 類本來就跟 BCB6 不同，加不加旗標是同一顆執行檔（只差 PE 時間戳 2 bytes）。
3. §8.6 第二點的「推論」（BCB6 `56 == 5.6*10` 為假）今天用 bcc32 實測：**假**，推論成立（X1）。
4. §5.3 的選項表：這次等於 (c)「改求值精度」的全樹版，用的是 fast 不是 standard。§8.6 第一點的 `forms/fContact.cpp:1671`（FP402 形狀，沒改容差）在 WinLibs 上也回到真。
5. 機台端 ctest：WinLibs 線之前因為字面值這一類而紅的測試，現在可能變綠——拿失敗清單逐名比對時，**少掉的那幾支是預期的**，不是誤報。
6. `tools/hmi_shell/build_hmi_shell.bat` 直接叫 g++（WebView2 外殼，沒有機台邏輯、沒有浮點比較），不經 CMake，沒有加。
