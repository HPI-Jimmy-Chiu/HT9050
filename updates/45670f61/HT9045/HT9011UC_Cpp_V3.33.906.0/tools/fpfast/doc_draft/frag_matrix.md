-O2／-O3 矩陣（同一份探針；W＝WinLibs 16.2 `-std=c++14`，O＝oracle 6.3.0；fs＝`-ffloat-store`）：

| 情境 | BCB6 | W -O2 standard | W -O2 fast | W -O2 fs standard（EastSun 今天） | W -O2 fs fast（EastSun 本次之後） | W -O0 fs fast | O -O3（加不加同一顆 exe） | O -O2 fs |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
| L1–L9 | 1 | **0** | 1 | **0** | 1 | 1 | 1 | 1 |
| R1 | 0 | **1** | 0 | **1** | 0 | 0 | 0 | 0 |
| R2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| S1、S2 | 1 | **0** | **0** | **0** | 1 | 1 | **0** | 1 |
| X1 | 0 | 0 | 0 | 0 | **1** | **1** | 0 | **1** |
| X2 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
| X3、X4 | 1 | 1 | 1 | 1 | **0** | **0** | 1 | **0** |

* `-ffloat-store` 在 fast 模式下（兩個編譯器、-O0 或 -O2 都一樣）把**運算式的暫存值**也捨入成 double ⇒ X 類跟 BCB6 不同；
  在 standard 模式下暫存值是 long double 型別，存回去也是 80-bit，所以 X 對、但字面值錯。
* -O2／-O3 不加 `-ffloat-store`：算出來、指派給區域變數的值留在 80-bit 暫存器 ⇒ S 類錯（§8.4，A3 容差修的就是這一類）。
* EastSun 那條線的完整旗標組（`-O2 -fno-strict-aliasing -fwrapv -fno-delete-null-pointer-checks -ffloat-store -fexcess-precision=standard -fno-finite-loops`，
  以及後面再接 `-fexcess-precision=fast`）另外各編一次，17 項答案分別與「W -O2 fs standard」「W -O2 fs fast」兩欄逐項相同。
* oracle -O3 加／不加旗標的兩顆 exe 只差 2 bytes（PE 標頭時間戳）。
