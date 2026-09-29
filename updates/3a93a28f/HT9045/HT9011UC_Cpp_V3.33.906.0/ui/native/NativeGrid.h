// ===========================================================================
//  ui/native/NativeGrid.h
//
//  AI(W906-NATIVE-PROTO) 20260929 [W906]: 原生視窗用的「不閃、只畫變了的格子」表格控件，外加一個雙緩衝文字標籤。NOT in golden。
//
//  Steven 20260929：「IO與馬達的顯示必須是很有效率的, 得快到20ms一次」「這樣的閃爍是不被允許的」。
//  取代原本的 ListView（comctl32 v5，exe 沒有 manifest ⇒ 沒有 LVS_EX_DOUBLEBUFFER：每次 LVM_REDRAWITEMS 都先把整列擦白再畫，
//  摘要 STATIC 每 200 ms SetWindowText 也是先擦再畫 ⇒ 一直閃）。
//
//  做法（每一步都是為了「沒變就不花時間、變了只花一格的時間」）：
//    1. 控件自己有一張常駐的背景點陣圖（整個客戶區），畫面上看到的永遠是從它 BitBlt 過去的 —— 沒有擦背景（WM_ERASEBKGND 回 1），
//       所以不會出現「先白再畫」的那一瞬間。
//    2. 每一格上次畫了什麼存在快取（GridCell：種類／顏色／文字）。呼叫端只說「哪幾列可能變了」（GridMarkRow），
//       GridFlush 對那幾列（而且只有看得到的）逐格問 owner 新內容，**跟快取一樣的格子不畫**；不一樣的才畫進背景圖。
//    3. 畫完只把變了的那幾個矩形 InvalidateRect(FALSE)，UpdateWindow 同步送 WM_PAINT，只 BitBlt 那些矩形。
//       ⇒ 沒有東西變：0 格、0 次 BitBlt；一顆燈變：一格（約 76×22 px）。
//    4. 捲動、改大小、換篩選（列整批換了）才整張重畫進背景圖（一次 BitBlt，仍然不閃）。
//  不用 manifest／comctl32 v6：加 manifest 會改到整顆 exe（wb_serve 其他視窗、MessageBox 的外觀），OFF 建置也得跟著動；
//  自己畫只多這一個檔，而且每一格花多少時間看得到（GridStats）。
//
//  ⚠ 只讀：本控件沒有任何「按下去做事」的東西；點一下只是選取列（畫面狀態）。「停用的輸出鈕」是畫出來的（kGridButtonOff），不是控件。
//  ⚠ 執行緒：同 NativeHost.h —— 全部在建視窗、泵訊息的那一條（wb_serve 主迴圈）。
// ===========================================================================
#ifndef W906_UI_NATIVE_NATIVEGRID_H
#define W906_UI_NATIVE_NATIVEGRID_H

#include <windows.h>

#include <string>
#include <vector>

namespace w906native {

enum GridAlign { kGridLeft = 0, kGridRight = 1, kGridCenter = 2 };

enum GridKind {
    kGridText = 0,        // 文字
    kGridLed = 1,         // 置中一顆圓燈（ledFill／ledEdge），text 不畫（只給 GridCellText 讀）
    kGridLedText = 2,     // 左邊一顆圓燈＋右邊文字
    kGridButtonOff = 3    // 畫出來的停用按鈕（DrawFrameControl DFCS_INACTIVE）＋灰字；text 空＝這格什麼都不畫
};

struct GridColumn {
    std::wstring title;
    int          width;
    int          align;   // GridAlign
    GridColumn() : width(60), align(kGridLeft) {}
    GridColumn(const wchar_t* t, int w, int a) : title(t), width(w), align(a) {}
};

// 一格的內容（owner 填）。快取比的就是這個 —— 一樣就不畫。
struct GridCell {
    int          kind;     // GridKind
    COLORREF     color;    // 字色
    COLORREF     ledFill;
    COLORREF     ledEdge;
    std::wstring text;
    GridCell() : kind(kGridText), color(0), ledFill(0), ledEdge(0) {}
    bool operator==(const GridCell& o) const
    {
        return kind == o.kind && color == o.color && ledFill == o.ledFill && ledEdge == o.ledEdge && text == o.text;
    }
    bool operator!=(const GridCell& o) const { return !(*this == o); }
};

// owner 的回呼：row＝表格的顯示列（0..rowCount-1），col＝GridColumn 的 index（不是畫面左右順序）。
// 每次都會先把 out 重設成 GridCell()，owner 只要填需要的欄位。
typedef void (*GridCellFn)(void* ctx, int row, int col, GridCell& out);

// 建控件（子視窗，WS_VSCROLL｜WS_HSCROLL｜WS_BORDER｜WS_TABSTOP）。字型用 WM_SETFONT 給。
HWND GridCreate(HWND parent, int id, GridCellFn fn, void* ctx);
// 欄位與畫面順序（order[i]＝畫面第 i 欄是哪個 column index；空＝照 index 順序）。整張重畫。
void GridSetColumns(HWND grid, const std::vector<GridColumn>& cols, const std::vector<int>& order);
// 列數，或列的內容整批換了（篩選、搜尋、點表換了）：整張重畫。選取列超出就取消。
void GridSetRowCount(HWND grid, int rows);
int  GridRowCount(HWND grid);
// 這一列的值可能變了。看不到的列、重複標記都不花時間（真的比較在 GridFlush）。
void GridMarkRow(HWND grid, int row);
void GridMarkAll(HWND grid);    // 所有看得到的列都要比一次（仍然只畫變了的格子）
// 比較髒列、只畫變了的格子、只 BitBlt 那些矩形（同步，回來時螢幕已更新）。回傳這次畫了幾格。
int  GridFlush(HWND grid);
int  GridTopRow(HWND grid);
int  GridVisibleRows(HWND grid);   // 含最下面露出一半的那列
int  GridSelectedRow(HWND grid);   // -1＝沒有

struct GridStats {
    unsigned long flushes;         // GridFlush 次數
    unsigned long rowsChecked;     // 被比較的列（髒而且看得到）
    unsigned long cellsPainted;    // 真的重畫的格子（Flush 的，不含整張重畫）
    unsigned long fullRenders;     // 整張重畫次數（捲動／改大小／換篩選）
    double        lastFlushMs;     // 上一次 GridFlush（含比較、畫進背景圖、BitBlt 上螢幕）
    double        maxFlushMs;
    double        lastFullMs;      // 上一次整張重畫
    GridStats() : flushes(0), rowsChecked(0), cellsPainted(0), fullRenders(0), lastFlushMs(0), maxFlushMs(0), lastFullMs(0) {}
};
GridStats GridGetStats(HWND grid);

// ---- 測試探針 ----
// 經控件的視窗程序問 owner（所以同時驗了訊息路由），回傳那一格的文字。
std::wstring GridCellText(HWND grid, int row, int col);
// 背景圖上某一點的顏色（CLR_INVALID＝沒有背景圖）。用來驗「畫進去的就是快取說的」。
COLORREF GridBackPixel(HWND grid, int x, int y);
bool     GridCellRect(HWND grid, int row, int col, RECT* rc);   // 客戶區座標；看不到回 false

// 整數 → 文字，寫進 out（保留容量：每一拍不配置記憶體）。
void GridSetInt(std::wstring& out, int v);
// ASCII 字串直接放寬（每個位元組一個字）；有非 ASCII 位元組就改走 Widen（UTF-8）。
void GridSetAscii(std::wstring& out, const std::string& s);

// ---- 雙緩衝文字標籤（摘要列用）----
// 跟 STATIC 一樣用 SetWindowTextW／WM_SETFONT；差別：內容沒變不重畫，變了也不擦背景（畫進記憶體再一次 BitBlt）。
HWND LabelCreate(HWND parent, int id, const wchar_t* text);
// 換字：一樣就什麼都不做，回 false；不一樣就同步重畫，回 true。
bool LabelSetText(HWND label, const std::wstring& text);
unsigned long LabelPaints(HWND label);

}  // namespace w906native

#endif  // W906_UI_NATIVE_NATIVEGRID_H
