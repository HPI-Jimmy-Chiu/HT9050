# 兩個產生器 —— C 路（golden 表單橋）的 A 形狀／C 形狀怎麼做、怎麼加一個新表單

按需要選取以下章節，原文依順序保留。

- [兩個產生器 —— C 路（golden 表單橋）的 A 形狀／C 形狀怎麼做、怎麼加一個新表單](generators/00.md)
- [一、兩個產生器，各對應 C 路的一種形狀](generators/01.md)
- [二、A 形狀：`tools/gen_formbridge.py`](generators/02.md)
- [三、C 形狀：`tools/gen_editlist.py`](generators/03.md)
- [四、SEC1 gate：新結構的替身全部「唯讀」時先檢查這個](generators/04.md)
- [五、G1 驗收：golden `HTEditList` 的浮點數格式會「正規化」第一次存檔](generators/05.md)
- [六、平行分工規則（20260924 夜，五位工程師各做一個 A 形狀表單）](generators/06.md)
- [七、驗證探針一覽](generators/07.md)
- [八、第七輪審查（20260924 夜，本輪沒有 H 級）](generators/08.md)
- [九、待確認（20260924 深夜第八輪審查後，已解決／已改變的部分見下方對照）](generators/09.md)
- [十、第八輪審查（Fable，20260924 深夜，本輪沒有 H 級落在已註冊的程式）](generators/10.md)
- [十一、下一波（進行中）：四位工程師把四個 A 形狀表單轉 C 形狀](generators/11.md)
- [十二、20260925 C 形狀第二波（commit `16f463b8`）](generators/12.md)
- [十三、通用陷阱：golden `TfMain` 建構子裡「移植樹沒做的初始化」](generators/13.md)
- [十四、通用規則：公式衍生值要在存檔前照 golden 事件相依順序重算](generators/14.md)
- [十五、選取式編輯器（`OffSet`）與動態面板（`BinSel`、`Temp_Set`）的 JSON 整包做法](generators/15.md)
- [十六、待辦（20260925 更新）](generators/16.md)
- [十七、陷阱：`wb_serve.cpp` 開機呼叫接在同一行時，要放在 `//` 註解**前面**（20260926）](generators/17.md)
- [十八、多寫者：「本頁沒改的欄位不把舊值蓋回」（S57／S90，20260926）](generators/18.md)
- [十九、golden 在「表單關窗後」才跑的尾段：網頁沒有關頁事件（S88，20260926）](generators/19.md)
- [二十、`tools/editlist/*.py` 寫死 golden 行號：golden 一更新就可能對錯行（20260926）](generators/20.md)
- [廿一、`.py` 取代字串裡的反斜線：C 字串要 `\\`，Python 原始碼寫四個（`cfb5735a`，20260926）](generators/21.md)
- [廿二、`ATKRecipeInfo` 在移植樹是 NULL：照抄 golden 的呼叫要加保護（`cfb5735a`，20260926）](generators/22.md)
- [廿三、e2e 環境：`wb_serve` 開機會自動啟動 GPIB／RS232Standard 引擎（`HT9045_TESTERCOMM=0` 關掉，20260926）](generators/23.md)
- [廿四、各產生器讀哪一棵 golden（`D:\HT9045_ref` 已退場，20260926）](generators/24.md)
- [廿五、C 路結構也可以沒有頁面：`ACTForm`／`Winway`／`Monitor`（S108～S110，`217e7e5e`，20260927）](generators/25.md)
- [廿六、檔案層原文插入點：C 路用 `members` 的 `#define`，A 形狀只能用區域變數（S98，`217e7e5e`／`7d490f7c`，20260927）](generators/26.md)
- [廿七、forms 門面要呼叫只編進 wb_serve 的本體：函式指標安裝座（S92／S95R，`4c5d7a26`／`bd40ffcb`，20260927）](generators/27.md)
- [廿八、共用檔 `tools/wb_serve.cpp` 的插入慣例（S85，`6db687d4`，20260927）](generators/28.md)
- [廿九、golden 呼叫的函式被別人翻好之後：拿掉對應的 `replace`，`--only` 重產（`51f39926`，20260927）](generators/29.md)

# 兩個產生器 —— C 路（golden 表單橋）的 A 形狀／C 形狀怎麼做、怎麼加一個新表單

[讀取此節](generators/00.md#兩個產生器--c-路golden-表單橋的-a-形狀c-形狀怎麼做怎麼加一個新表單)

## 一、兩個產生器，各對應 C 路的一種形狀

[讀取此節](generators/01.md#一兩個產生器各對應-c-路的一種形狀)

## 二、A 形狀：`tools/gen_formbridge.py`

[讀取此節](generators/02.md#二a-形狀toolsgen_formbridgepy)

### 2.1 表單設定：`tools/formbridge/<Class>.py`

[讀取此節](generators/02.md#21-表單設定toolsformbridgepy)

### 2.2 產生器指令

[讀取此節](generators/02.md#22-產生器指令)

### 2.3 建置：只收兩份產生的正面清單（不 GLOB）

[讀取此節](generators/02.md#23-建置只收兩份產生的正面清單不-glob)

### 2.4 加一個新的 A 形狀表單

[讀取此節](generators/02.md#24-加一個新的-a-形狀表單)

## 三、C 形狀：`tools/gen_editlist.py`

[讀取此節](generators/03.md#三c-形狀toolsgen_editlistpy)

### 3.1 結構設定：`tools/editlist/<struct>.py`

[讀取此節](generators/03.md#31-結構設定toolseditlistpy)

### 3.1a DFM 設計期值、`TTrackBar`／`TUpDown`（20260924 深夜，§62 為了轉 `TfSpeed` 加）

[讀取此節](generators/03.md#31a-dfm-設計期值ttrackbartupdown20260924-深夜62-為了轉-tfspeed-加)

### 3.1b 結構欄位（對照 `IniConfig`／`Ld_UldDelayTime`／`ArmSpeed_File` 現有設定）：

[讀取此節](generators/03.md#31b-結構欄位對照-iniconfigld_ulddelaytimearmspeed_file-現有設定)

### 3.2 第二個以後的 C 形狀結構共用 `FileRW/_EditPage.h`

[讀取此節](generators/03.md#32-第二個以後的-c-形狀結構共用-filerw_editpageh)

### 3.3 開機順序要照 golden `CreateForm` 順序

[讀取此節](generators/03.md#33-開機順序要照-golden-createform-順序)

## 四、SEC1 gate：新結構的替身全部「唯讀」時先檢查這個

[讀取此節](generators/04.md#四sec1-gate新結構的替身全部唯讀時先檢查這個)

## 五、G1 驗收：golden `HTEditList` 的浮點數格式會「正規化」第一次存檔

[讀取此節](generators/05.md#五g1-驗收golden-hteditlist-的浮點數格式會正規化第一次存檔)

## 六、平行分工規則（20260924 夜，五位工程師各做一個 A 形狀表單）

[讀取此節](generators/06.md#六平行分工規則20260924-夜五位工程師各做一個-a-形狀表單)

## 七、驗證探針一覽

[讀取此節](generators/07.md#七驗證探針一覽)

## 八、第七輪審查（20260924 夜，本輪沒有 H 級）

[讀取此節](generators/08.md#八第七輪審查20260924-夜本輪沒有-h-級)

## 九、待確認（20260924 深夜第八輪審查後，已解決／已改變的部分見下方對照）

[讀取此節](generators/09.md#九待確認20260924-深夜第八輪審查後已解決已改變的部分見下方對照)

## 十、第八輪審查（Fable，20260924 深夜，本輪沒有 H 級落在已註冊的程式）

[讀取此節](generators/10.md#十第八輪審查fable20260924-深夜本輪沒有-h-級落在已註冊的程式)

## 十一、下一波（進行中）：四位工程師把四個 A 形狀表單轉 C 形狀

[讀取此節](generators/11.md#十一下一波進行中四位工程師把四個-a-形狀表單轉-c-形狀)

## 十二、20260925 C 形狀第二波（commit `16f463b8`）

[讀取此節](generators/12.md#十二20260925-c-形狀第二波commit-16f463b8)

## 十三、通用陷阱：golden `TfMain` 建構子裡「移植樹沒做的初始化」

[讀取此節](generators/13.md#十三通用陷阱golden-tfmain-建構子裡移植樹沒做的初始化)

## 十四、通用規則：公式衍生值要在存檔前照 golden 事件相依順序重算

[讀取此節](generators/14.md#十四通用規則公式衍生值要在存檔前照-golden-事件相依順序重算)

## 十五、選取式編輯器（`OffSet`）與動態面板（`BinSel`、`Temp_Set`）的 JSON 整包做法

[讀取此節](generators/15.md#十五選取式編輯器offset與動態面板binseltemp_set的-json-整包做法)

### 選取式編輯器：`OffSet`

[讀取此節](generators/15.md#選取式編輯器offset)

### 動態面板：`BinSel`

[讀取此節](generators/15.md#動態面板binsel)

### 動態面板：`Temp_Set`

[讀取此節](generators/15.md#動態面板temp_set)

## 十六、待辦（20260925 更新）

[讀取此節](generators/16.md#十六待辦20260925-更新)

## 十七、陷阱：`wb_serve.cpp` 開機呼叫接在同一行時，要放在 `//` 註解**前面**（20260926）

[讀取此節](generators/17.md#十七陷阱wb_servecpp-開機呼叫接在同一行時要放在--註解前面20260926)

## 十八、多寫者：「本頁沒改的欄位不把舊值蓋回」（S57／S90，20260926）

[讀取此節](generators/18.md#十八多寫者本頁沒改的欄位不把舊值蓋回s57s9020260926)

## 十九、golden 在「表單關窗後」才跑的尾段：網頁沒有關頁事件（S88，20260926）

[讀取此節](generators/19.md#十九golden-在表單關窗後才跑的尾段網頁沒有關頁事件s8820260926)

## 二十、`tools/editlist/*.py` 寫死 golden 行號：golden 一更新就可能對錯行（20260926）

[讀取此節](generators/20.md#二十toolseditlistpy-寫死-golden-行號golden-一更新就可能對錯行20260926)

## 廿一、`.py` 取代字串裡的反斜線：C 字串要 `\\`，Python 原始碼寫四個（`cfb5735a`，20260926）

[讀取此節](generators/21.md#廿一py-取代字串裡的反斜線c-字串要-python-原始碼寫四個cfb5735a20260926)

## 廿二、`ATKRecipeInfo` 在移植樹是 NULL：照抄 golden 的呼叫要加保護（`cfb5735a`，20260926）

[讀取此節](generators/22.md#廿二atkrecipeinfo-在移植樹是-null照抄-golden-的呼叫要加保護cfb5735a20260926)

## 廿三、e2e 環境：`wb_serve` 開機會自動啟動 GPIB／RS232Standard 引擎（`HT9045_TESTERCOMM=0` 關掉，20260926）

[讀取此節](generators/23.md#廿三e2e-環境wb_serve-開機會自動啟動-gpibrs232standard-引擎ht9045_testercomm0-關掉20260926)

## 廿四、各產生器讀哪一棵 golden（`D:\HT9045_ref` 已退場，20260926）

[讀取此節](generators/24.md#廿四各產生器讀哪一棵-goldendht9045_ref-已退場20260926)

## 廿五、C 路結構也可以沒有頁面：`ACTForm`／`Winway`／`Monitor`（S108～S110，`217e7e5e`，20260927）

[讀取此節](generators/25.md#廿五c-路結構也可以沒有頁面actformwinwaymonitors108s110217e7e5e20260927)

## 廿六、檔案層原文插入點：C 路用 `members` 的 `#define`，A 形狀只能用區域變數（S98，`217e7e5e`／`7d490f7c`，20260927）

[讀取此節](generators/26.md#廿六檔案層原文插入點c-路用-members-的-definea-形狀只能用區域變數s98217e7e5e7d490f7c20260927)

## 廿七、forms 門面要呼叫只編進 wb_serve 的本體：函式指標安裝座（S92／S95R，`4c5d7a26`／`bd40ffcb`，20260927）

[讀取此節](generators/27.md#廿七forms-門面要呼叫只編進-wb_serve-的本體函式指標安裝座s92s95r4c5d7a26bd40ffcb20260927)

## 廿八、共用檔 `tools/wb_serve.cpp` 的插入慣例（S85，`6db687d4`，20260927）

[讀取此節](generators/28.md#廿八共用檔-toolswb_servecpp-的插入慣例s856db687d420260927)

## 廿九、golden 呼叫的函式被別人翻好之後：拿掉對應的 `replace`，`--only` 重產（`51f39926`，20260927）

[讀取此節](generators/29.md#廿九golden-呼叫的函式被別人翻好之後拿掉對應的-replace--only-重產51f3992620260927)
