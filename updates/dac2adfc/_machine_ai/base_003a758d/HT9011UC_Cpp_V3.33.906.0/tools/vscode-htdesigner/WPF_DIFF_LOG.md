# 跟 WPF 不一樣的地方：改動紀錄

EastSun 20260930：「你看還有那邊跟 wpf 不一樣，直接做修改，你每次修改需要自己做記錄」。
這份是**對照 WPF（Visual Studio XAML 設計工具）的改動紀錄**：每一條＝WPF 怎麼做（出處）、這裡原本怎麼做、改成什麼、怎麼測、哪一版。
給使用者看的新功能說明在 CHANGELOG.md；逐項對照表在 README.md「跟 WPF 設計工具的操作對照」。

微軟說明（下面簡稱）：
- 〔XD〕[Create UIs with Visual Studio XAML Designer](https://learn.microsoft.com/en-us/visualstudio/xaml-tools/creating-a-ui-by-using-xaml-designer-in-visual-studio?view=vs-2022)
- 〔WE〕[Working with elements in XAML Designer](https://learn.microsoft.com/en-us/visualstudio/xaml-tools/working-with-elements-in-xaml-designer?view=vs-2022)
- 〔EH〕[How to: Create a Simple Event Handler（WPF Designer）](https://learn.microsoft.com/en-gb/previous-versions/visualstudio/visual-studio-2008/bb675300(v=vs.90))
- 〔WF〕[Windows Forms：How to add or remove an event handler](https://learn.microsoft.com/en-us/dotnet/desktop/winforms/controls/how-to-add-an-event-handler)
- 〔OV〕[WPF and Silverlight Designer Overview](https://learn.microsoft.com/en-us/previous-versions/bb514528(v=vs.110))
- 〔KS〕[XAML Designer Keyboard Shortcuts](https://github.com/MicrosoftDocs/visualstudio-docs/blob/main/docs/xaml-tools/keyboard-shortcuts-for-xaml-designer.md)
- 〔RS〕[Resize or scale an object（Blend）](https://learn.microsoft.com/en-us/previous-versions/visualstudio/design-tools/expression-studio-2/cc294977(v=expression.10))
- 〔MK〕[Artboard modifier keys in Blend](https://learn.microsoft.com/en-us/visualstudio/xaml-tools/artboard-modifier-keys-in-blend?view=vs-2022)
- 〔BK〕[Keyboard shortcuts in Blend for Visual Studio](https://learn.microsoft.com/en-us/visualstudio/xaml-tools/keyboard-shortcuts-in-blend?view=vs-2022)
- 〔CF〕[Configure XAML Designer in Visual Studio](https://learn.microsoft.com/en-us/visualstudio/ide/reference/xaml-designer?view=vs-2022)

## 還沒改的（照順序做，做完移到下面）

EastSun 20260930 晚上：「先朝元件屬性和事件的操作先下手，先讓使用者可以和 wpf 改 code 一樣方便、有效率，並且屬性表格和事件可以像 wpf 讓使用者看起來條理分明」→ 28～31（設定）先放著，先做屬性和事件。

| # | WPF | 這裡現在 |
|---|---|---|
| 6 | 工具箱直接拖到畫面上（〔WE〕） | **做不到**：VS Code 不讓側欄的項目拖進設計畫面（替代：點工具、再點／拖框畫面） |
| — | 工具箱上方的搜尋框（Search Toolbox） | 不用改：VS Code 的樹狀清單本身就能打字篩選（點工具箱後直接打字） |
| 28 | 設定「Zoom by using」：滑鼠滾輪／Ctrl＋滾輪／Alt＋滾輪（〔CF〕） | 固定 Ctrl＋滾輪 |
| 29 | 設定「Default zoom setting」：上次的比例（Last Used）／符合全部（Fit All）（〔CF〕） | 每次開頁面都是 100% |
| 30 | 設定「Default document view」：Design／Split（設計和原始碼並排）（〔CF〕） | 只開設計畫面，要 HTML 另外按 Shift+F7 |
| 31 | 設定「Default margin」：對齊線吸附的間距（〔CF〕Snapping） | 間距吸附固定 8px |
| — | Blend：按住 Alt 拖曳＝複製（〔MK〕） | 不改：這裡 Alt＝拖曳時暫時不吸附；複製用 WinForms／VS 的 Ctrl＋拖曳（0.87） |
| — | Blend：Ctrl／Shift＋滾輪＝上下／左右捲動（〔MK〕） | 不改：VS 的 XAML 設計工具是 Ctrl＋滾輪縮放，這裡照 VS |

## 已經改的

| 日期 | 版本 | WPF（出處） | 原本 | 改成 | 測試 | commit |
|---|---|---|---|---|---|---|
| 0930 | 0.71.0 | 事件只對到會執行的處理函式 | 事件下面列一堆位置 | 只列會執行的 C++ 函式，雙擊直接跳 | 真 VS Code：Exit→W906_Main_CloseProgramOp | 4e9e5e5 |
| 0930 | 0.72.0 | 設計工具幫你接好事件（你只寫函式內容） | 新增事件只加 C++ 函式 | 網頁 htdCpp、伺服器 htd.event 分支、CMake、產生檔一起接好（不存檔） | 編譯檢查（兩個編譯器）；真 VS Code 新增後還原 | c0153f7 |
| 0930 | 0.73.0 | 事件分頁：兩欄；打名稱＋Enter／雙擊新增；空白 Enter＝預設名稱；下拉選現有函式；右鍵 Reset（〔EH〕〔WF〕） | 每列下面一堆位置、空的顯示灰色預設名、名稱斷行 | 照 WPF | 面板：兩欄、空白、Enter、選單；smoke：重設、選現有函式 | d3edc33 |
| 0930 | 0.74.0 | 屬性視窗上方 Name 方塊、閃電按鈕切換事件（〔XD〕） | 名稱在表格裡、事件跟屬性混在一起 | 名稱方塊＋〔屬性〕〔⚡事件〕一次一種清單 | 面板：切換、名稱方塊 Enter 一次 | 6ee8c8d |
| 0930 | 0.75.0 | 工具箱選好按 Enter＝加入（〔WE〕） | Enter 只拿起工具 | Enter＝直接加 | smoke：Enter 加到 Panel2 | d1a5872 |
| 0930 | 0.76.0 | 雙擊元件＝預設事件，空的就建立（〔EH〕） | 空的只跳到 HTML | 空的就新增並接好；F7／Enter 只開不新增 | smoke：Label1 雙擊→Label1Click；F7 只開 .cpp | c69b535 |
| 0930 | 0.77.0 | Shift+F7 在設計畫面和標記之間切換 | 沒有 | 設計畫面→HTML；HTML／C++→設計畫面該元件 | 真 VS Code：HTML 游標→選 spbSave | c69b535 |
| 0930 | 0.78.0 | 文件大綱右鍵＝畫面右鍵（〔WE〕） | 元件樹只有行內按鈕 | 同一套右鍵選單，先選那一列 | smoke：Panel2 那列複製＝Panel2 | c69b535 |
| 0930 | 0.79.0 | 畫面左下工具列：縮放、符合、格線、對齊線（〔XD〕） | 沒有 | 左下工具列 | probe：工具列按鈕、頁面收不到點擊 | d60160c |
| 0930 | 0.79.0 | Tag navigator：上層一路點回去（〔OV〕） | 路徑只是文字 | 路徑可點 | 面板：點 Panel2＝選取 | d60160c |
| 0930 | 0.80.0 | 屬性旁的 property marker＋Reset（〔XD〕〔OV〕） | 只有改過的才有 ↺ | 每列一個方塊，點＝重設選單 | 面板：9 個、1 個實心、選單重設 | 5f76d94 |
| 0930 | 0.81.0 | 分割檢視：XAML 游標在元素任何地方＝選取它 | 只有在開頭標籤裡才選 | 標籤、文字、子元素裡都選；表單 div＝表單 | smoke：caption 文字→spbSave | 29df298 |
| 0930 | 0.81.0 | Arrange by 在屬性視窗最上面（〔XD〕） | 在 DFM 屬性那區裡 | 移到最上面 | 面板：排序按鈕在上方 | 29df298 |
| 0930 | 0.82.0 | FontFamily 是下拉（〔XD〕） | Font.Name 只能打字 | 下拉清單（DFM／頁面／常用字型） | 面板：12 個字型 | 9211507 |
| 0930 | 0.83.0 | 文件大綱裡直接 F2 改名、Ctrl+C／X／V、Delete（〔WE〕） | 元件樹只有 Delete；F2、剪貼要回設計畫面 | 元件樹五個鍵都有，對的是樹上目前那一列（方向鍵移過去的也算；多選＝全部） | smoke：樹上 Panel2、頁面 spbSave → Ctrl+C 複製 Panel2 | 9b851fe |
| 0930 | 0.83.0 | XAML 右鍵「Navigate to Event Handler」（〔EH〕步驟 4） | HTML 裡沒有 | HTML 右鍵「移到事件處理函式」：htdCpp 那一行＝它的 C++；元件裡＝預設事件（同 F7，不新增）；右鍵另有「檢視設計工具」 | smoke：htdCpp 行→TfHotPlate::spbSaveUp；真 VS Code：spbSave→它的 C++ | 9b851fe |
| 0930 | 0.83.0 | 沒選元素時屬性視窗顯示 Window（〔OV〕） | 空白的「點一個元件」 | 一打開頁面就選表單（只在還沒選任何東西時） | probe：第一次 init＝選 @form | 9b851fe |
| 0930 | 0.84.0 | 設計畫面上方的 Information Bar 顯示畫面的問題（〔OV〕） | 頁面的 JS 錯誤只寫在輸出面板 | 上方資訊列：錯誤數＋第一個錯誤；「看全部」＝輸出面板列出；×＝關掉（新錯誤再出現）；點它頁面收不到 | probe：丟一個錯誤→出現、看全部、關掉、頁面沒被選 | 092e959 |
| 0930 | 0.85.0 | 屬性格線分類（Common／Layout／Appearance／Text），Arrange by 預設 Category、套用整個格線（〔XD〕） | 「外觀與版面」一張平的表；排列預設依名稱、只管 DFM 屬性 | 可以改的屬性分 一般／版面／外觀／文字；依類別（預設）／依名稱（A→Z 不分組）／DFM 順序，兩區一起套用 | 面板：預設依類別、版面從 Left 開始；依名稱＝A→Z 沒有分組 | cc067c6 |
| 0930 | 0.86.0 | 工具箱最上面是「指標」（Pointer）：點它＝放下工具回到選取；放好一個元件後工具箱回到指標（〔WE〕） | 工具箱沒有這一項（只能按 Esc），放好後還停在那個工具 | 第一項「指標」；放好／Esc／連點兩下／Enter 加入之後，工具箱選取回到指標（工具箱沒開著時不動它） | smoke：指標放下工具、Enter 在指標上不新增、Esc 和放好後回到指標；真 VS Code：放好後選取＝指標 | 3ab1da5 |
| 0930 | 0.86.0 | 屬性格線的類別可以收合（〔XD〕） | 類別標題不能點 | 點標題／Enter＝收合／展開（▾／▸，記住）；搜尋時照樣找得到收合裡的 | 面板：收合後那類的列不見、其他類還在，搜尋 Left 找得到，Enter 展開；DFM 類別也可以收合 | 3ab1da5 |
| 0930 | 0.86.0 | 滑鼠停在屬性名稱上＝這個屬性的說明（〔XD〕） | 沒有說明 | 常用 VCL 屬性（約 80 個）的白話說明；DFM 屬性後面照樣有「雙擊：跳到 .dfm 第幾行」 | 面板：Left 的說明（可編輯列和 DFM 列） | 3ab1da5 |
| 0930 | 0.87.0 | 按住 Ctrl 拖曳＝複製一份放到放開的地方（WinForms／WPF 設計工具） | Ctrl 拖曳＝移動（Ctrl 只管加選） | Ctrl＋拖曳：原本的回原位，複製的在放開處、同容器、新名稱、選取它，一個 Ctrl+Z；Ctrl＋點（不拖）照舊加選／取消 | probe：Ctrl 拖 20,10＝copyDrop（兩個名稱、原位不動、沒有 edit）、Ctrl＋點選取的＝取消；smoke：兩個一起複製、各在原本的後面、+30／−12；真 VS Code：spbSave 的複製在 Panel2、右移 40、一次復原 | 8342d1f |
| 0930 | 0.88.0 | 文件大綱 Ctrl+H 隱藏、Shift+Ctrl+H 顯示、Ctrl+L 鎖定、Shift+Ctrl+L 解鎖（〔XD〕Document Outline） | 只能點眼睛／鎖頭 | 四個鍵：設計畫面＝選取的（多選全部），元件樹＝樹上選的列；元件樹上 F2 不再被設計畫面的 F2 搶走 | smoke：畫面上 spbSave＋sbtExit 隱藏／顯示、樹上 Panel2＋sbtExit 鎖定／解鎖、按鍵的 when 分開；真 VS Code：spbSave 隱藏再顯示、原始碼沒動 | b8cf665 |
| 0930 | 0.88.0 | 縮放 12.5%～800%（〔XD〕Zoom） | 25%～400% | 清單、＋／－、Ctrl＋滾輪到 12.5%～800%；符合視窗／選取照舊最大 400% | probe：清單兩端、800% 再＋還是 800%、12.5% 標籤、更小會夾住 | b8cf665 |
| 0930 | 0.88.0 | 畫面左下「Toggle artboard background」亮／暗（〔XD〕） | 沒有 | 工具列「背景」：頁面自己的 ↔ 暗色，每頁共用、記住，操作模式照原樣 | probe：變暗、再按回原樣（inline 值還原）；smoke：記住、通知其他頁 | b8cf665 |
| 0930 | 0.89.0 | 右鍵 Layout > Reset Width／Height／…／Reset All（〔WE〕Reset the element layout） | 「版面重設」一次全部改回 DFM（連文字、外觀） | 子選單：重設位置／重設大小／重設全部版面／全部改回 DFM，設計畫面和元件樹都有 | smoke：只位置＝left 54 不含粗體、只大小＝沒有要改的、全部版面＝只有 setLayout，兩個選單都掛子選單；真 VS Code：只重設位置，粗體沒動 | b0a8e40 |
| 0930 | 0.90.0 | Ctrl+Shift+A＝取消全部選取（〔KS〕Clear all selections） | 沒有（只有 Esc＝選上層） | 取消選取（多選一起） | probe：選取訊息＝空 | e8aa4af |
| 0930 | 0.90.0 | F9＝顯示／隱藏控制點（〔KS〕Show/hide element handles） | 沒有 | 選取框、標籤、控制點藏起來，再按回來；點照樣選取 | probe：8 個控制點 → 0 → 8，選取框藏起來 | e8aa4af |
| 0930 | 0.90.0 | Alt＋方向鍵＝複製一份（〔KS〕Duplicate an element） | 沒有作用 | 複製選取的、錯開 1px（Shift＝10px），同 Ctrl＋拖曳；只在設計畫面有焦點時（側欄的 Alt+← 照舊是上一頁） | smoke：Shift+Alt+→ 的複製在右邊 10px，8 個鍵的 when；真 VS Code：複製在 Panel2、一次復原 | e8aa4af |
| 0930 | 0.90.0 | Ctrl+N＝新增元素（〔KS〕Create an element） | 只有右鍵和工具箱 | 工具箱清單（同右鍵「新增元件」），只在設計畫面有焦點時 | smoke：鍵的 when | e8aa4af |
| 0930 | 0.90.0 | Shift＋角落控制點＝保持比例（〔RS〕） | Shift 沒作用 | 依變化大的那一邊等比例放大縮小 | probe：往右 30、往下 2 → 寬高一起放大、比例不變 | e8aa4af |
| 0930 | 0.91.0 | F2＝直接在畫面上編輯元件的文字，Esc 結束（〔KS〕Edit control text） | 畫面上 F2＝改名稱（WPF 改名稱是在屬性視窗的 Name 方塊） | 畫面上 F2＝在元件上開一個文字框（字型、對齊照元件），Enter／點別處＝寫進去（跟屬性面板的 Caption 同一個修改，一個 Ctrl+Z），Esc＝不改；打字時設計畫面的快捷鍵都不作用，Ctrl+Z／Ctrl+Y＝文字框自己的復原。改名稱：屬性面板 Name 方塊、元件樹 F2、右鍵「改名稱」 | probe：F2 開框（Save2）、方向鍵不移動、框裡按滑鼠不被擋、Enter 寫 Save3、Esc 不寫；smoke：F2 的鍵、打字時快捷鍵的 when 全部擋掉、Ctrl+Z 給文字框；真 VS Code：開框、關掉不寫 | 84a2ed1 |
| 0930 | 0.92.0 | 左下工具列「Show/Hide snap grid」和「snapping to gridlines」是兩個按鈕（〔XD〕） | 「格線」一個按鈕＝顯示又吸附 | 「格線」＝畫不畫、「吸附」＝吸不吸，可以只吸不畫、只畫不吸；每頁共用、記住；標題列 # 和右鍵照舊一起開關 | probe：只吸不畫＝格線沒畫但落在 8 的倍數、只畫不吸＝格線有畫但自由移動 5px；工具列四個按鈕送出 gridShow、gridSnap；smoke：兩個分開記住、通知每一頁 | 20adafc |
| 0930 | 0.93.0 | 對齊線也對齊文字的基準線（〔XD〕「when the edges and the text of some controls are aligned」） | 只對齊邊緣和中心 | 移動時自己的文字基準線對旁邊元件的基準線（標籤、按鈕的文字、Panel 標題、輸入框的值），比邊緣近就用它，紅色虛線 | probe：40px 和 12px 的字，小的放在大的基準線下 2px、橫向拖 → 基準線對齊（差 <1px）、虛線 | 5e5b5df |
| 0930 | 0.94.0 | Blend 快捷鍵：Ctrl+Shift+1／2／9 同寬／同高／同大小、Ctrl+=／Ctrl+- 縮放、Ctrl+0／Ctrl+9 符合選取、Ctrl+1 實際大小（〔BK〕） | 只有右鍵和左下工具列 | 8 個鍵，只在設計畫面有焦點、沒在打字時（其他地方照舊是 VS Code 的） | smoke：8 個鍵的指令和 when、放大縮小各送一級；probe：zoomStep 1→1.1→1 | 06847ca |
| 0930 | 0.95.0 | 拖到另一個 Panel 上、放開前按 Alt＝換到那個容器（〔MK〕Reparent an object） | 畫面上拖曳只移位置；換容器只能在元件樹拖 | 放開時按著 Alt：原本的回原位，搬進滑鼠下最裡層的容器、放在放開的位置（座標換成新容器的），一個 Ctrl+Z；同一個容器＝只是移動；分頁拒絕並說明 | probe：放在臨時框上 → reparentDrop（名稱、離滑鼠 −5,−5、目標框＋表單），按鈕回原位、沒有寫；smoke：搬到表單 295,244、同容器只移動、沒有容器不動；真 VS Code：搬進表單（樹上看得到）、一次復原 | b8b9923 |
| 0930 | 0.95.0 | （修正，不是 WPF 差異）放進表單＝放在表單自己裡面 | 貼上／工具箱新增／元件樹拖曳／換容器「放進表單」時，找到表單裡第一個 GroupBox 的 div.cli，跑進那個 GroupBox | insideEnd 只認直接子元素的 div.cli | lib：表單裡有 GroupBox 時放進表單＝表單的最後；smoke：換容器後不在 GroupBox1 裡；真 VS Code 抓到（parent=GroupBox1） | b8b9923 |
| 0930 | 0.96.0 | Alt＋點＝選到下面那一層，再點一次再往下（〔MK〕） | Alt＋點＝最裡層的元素（只有一種） | 第一次同以前（最裡層），之後每次＝目前選取的下一層，到底回到最上面；點擊事件不會再蓋掉 | probe：表單選取 → spbSave → Panel2 → 表單 → spbSave（兩頁都對） | f6d086a |
| 0930 | 0.96.0 | Ctrl＋空白鍵＋點＝放大、Ctrl+Alt＋空白鍵＋點＝縮小（〔MK〕） | 沒有 | 按住時游標變放大鏡、點一下縮放一級，點的那一點留在原處；放開空白鍵回來 | probe：放大到 1.1、放開後遮罩收起、縮小回 1、沒選取沒寫 | f6d086a |
| 0930 | 0.97.0 | Shift＋拖曳＝拉框選取，從哪裡開始都可以（〔MK〕Select by drawing a marquee）；Shift＋點＝多選（〔KS〕） | 只能從空白處拉框；Shift＋點＝加選 | 從元件上按 Shift 拖曳也拉框；不拖＝照舊加選／取消 | probe：從 A 開始拉框框住 B → 只選 B；再 Shift＋點 A → A、B 都選 | e72eb13 |
| — | Blend：Shift＋點第一個和最後一個＝選一串相鄰的（〔BK〕） | 不改：「相鄰」在 Blend 指物件面板的順序；這裡元件樹本身 Shift＋點就是選一串，畫面上的 Shift＋點照 XAML 設計工具＝多選 | — | — | — |
| 0930 | 0.98.0 | 顏色編輯器可以打數值、有滴管（〔XD〕Properties window 的 Brush editor） | 只有色塊和 BCB6 16 色 | 色塊下多了輸入框（#rrggbb、#rgb、clRed…，Enter；錯的標紅不送）和「滴管」（EyeDropper：點畫面任何一點取色） | 面板：#abc→#aabbcc、clRed→#ff0000、zzz 標紅不送；滴管（測試用替身）→ #123456 | 9da7451 |
| 0930 | 0.99.0 | 屬性視窗只有屬性；事件另一頁（〔XD〕Properties window） | 屬性頁下面接了網頁 JS 監聽器、送到 C++ 的命令、資料欄位、用到它的程式…一長串 | 三頁：〔屬性〕只有屬性、〔⚡ 事件〕、〔</> 程式碼〕放那些接線／程式碼的清單 | 面板：屬性頁沒有那六區；〔程式碼〕頁有命令、資料欄位、recipe.doc.put、[Hotplate Form] X Start | 612ae63 |
| 0930 | 0.100.0 | 屬性格線：名稱｜值｜property marker 三欄，值下面沒有字，類別是色帶（〔XD〕Properties window） | 值下面塞「DFM 54」、長說明、16 色整排展開，側欄窄時被切掉 | 固定三欄（colgroup），DFM 值只在不一樣的列顯示、其餘在小方塊提示；長說明收進 ⓘ；顏色列＝顏色框＋數值，16 色和滴管收進 ▾；類別標題色帶 | 面板：每列三格、小方塊都在第三格；DFM 值只在不同的列看得到；16 色預設收起、▾ 打開；系統色說明在 ⓘ | 0aa73e3 |
| 0930 | 0.101.0 | 事件分頁：一種兩欄表，空的就是空的（〔XD〕〔EH〕） | 「事件」「網頁事件（這一頁的 JS）」兩區長得不一樣；網頁事件有一大段說明、灰色預設名稱看起來像有值 | 「元件事件（C++）」「網頁事件（JS）」同樣兩欄、名稱對齊；說明進標題提示；預設名稱只在點進去時出現；＋N 其他監聽器 | 截圖檢查；面板：事件表測試照舊全過 | f290efb |
| 0930 | 0.101.0 | 巡覽到事件處理函式＝游標在函式本體裡（〔EH〕） | 跳到函式名稱那一行 | 事件表雙擊有函式的＝游標停在本體第一行（空本體＝大括號後面） | lib：bodyStart 四種情況；真 VS Code：W906_Main_CloseProgramOp 打開、游標在本體 | f290efb |
| 0930 | 0.102.0 | 屬性視窗一打開就是屬性格線，其他收在展開鈕後面（〔XD〕「advanced properties by selecting a down arrow」） | 「DFM 屬性」「HTML」一直展開、樣式跟格線不一樣，說明文字在表格下面 | 兩區預設收起（記住使用者的選擇；搜尋到時自動打開）；名稱欄、色帶跟格線一樣；說明進標題提示 | 截圖；面板：排列／搜尋／HTML 改值的測試照舊全過 | 5c9f7b0 |
| 0930 | 0.103.0 | 屬性和標記同步（分割檢視：選屬性就看得到 XAML 裡的那一段）；EastSun：「和 wpf 改 code 一樣方便有效率」 | 從屬性要自己去 HTML 找那個值 | 雙擊屬性名稱＝HTML 開在旁邊、那個值選好（style 值、文字、disabled、title；輸入框找外層 span、Panel 找標題 span；沒寫＝元件標籤） | lib：propRange 11 種；smoke：Left→54px、Caption→Save、Font.Name→元件；面板：雙擊送 revealProp | 666bf5e |
| 0930 | 0.104.0 | 屬性視窗上方：Name、型別、搜尋、Arrange by（同一行）（〔XD〕） | 上方 6 行：名稱、分頁、備註、路徑、三個按鈕一行、搜尋、排列一行 | 三個按鈕併到分頁那一行右邊（短名稱＋提示）、排列併到搜尋框旁邊 | 截圖；面板：排列按鈕、分頁的測試照舊全過 | 38deb53 |
| 0930 | 0.105.0 | Blend 數值欄：↑／↓ 加減、Shift＝×10 | 數值欄的 ↑／↓ 只改數字，離開才寫；每改一次一個復原步驟 | ↑／↓ ±1、Shift ±10，停 0.4 秒／離開／Enter 才寫一次（連按＝一個 Ctrl+Z） | 面板：Top 8 → ↑↑↑ Shift↑ ↓ ＝ 20，按鍵時沒送，離開時送一次 20 | 3767b7f |
| 0930 | 0.106.0 | 屬性格線：Esc 放棄編輯、選中的屬性整列標示 | Esc 沒作用（離開欄位就寫進去）；看不出在改哪一列 | Esc＝回到開始編輯時的值、不寫；focus 的那一列反白、名稱變粗 | 面板：Width ↑↑ Esc、Caption 打 XX Esc → 都回原值、沒有送出；那一列背景不同 | 3d00bfe |
| 0930 | 0.107.0 | 事件分頁照字母排（〔XD〕Events） | 照 VCL 宣告／網頁事件分組的順序 | 兩張表都 A→Z | 面板：兩張表都是 A→Z | 42d502e |
| 0930 | 0.108.0 | （修正）屬性視窗不會是空的 | 0.102 起沒有可編輯表的元件，屬性頁只剩兩個收起來的區塊 | 只有上面有可編輯表時才預設收起「DFM 屬性」 | 面板：XST1 的資料（沒有可編輯表）DFM 屬性是打開的 | 454621d |
| 0930 | 0.109.0 | 屬性標記（property marker）每個屬性的值右邊都有一個，點它＝那個屬性的選單（〔XD〕〔WE〕） | 只有 DFM 有寫的列才有方塊；選單只有「重設」 | 每一列都有（不比對的淡一點）；選單＝重設＋跳到 HTML 原始碼；那一列按右鍵＝同一個選單（文字框裡照舊） | 面板：每一列都有、4 個淡的、Top 的選單（重設灰）→ revealProp:Top、右鍵 Width→revealProp:Width、文字框裡右鍵沒有選單 | ba81a97 |
| 0930 | 0.110.0 | 屬性格線：Tab＝下一個屬性的值、Enter＝寫入後留在那一格（值選起來）；事件分頁一樣 Tab 一格一格走；事件右鍵＝Reset（〔XD〕〔EH〕） | Tab 會停在小方塊、色塊、▾、事件列本身；Enter 之後焦點不見；網頁事件沒有右鍵選單 | Tab 只停在值；Enter 寫一次、留在原地、值選好（數值、Caption、Font.Name、網頁事件）；網頁事件右鍵＝跳到程式碼／重設 | 面板：Tab 停的 13 格全是值、事件列不停；Left／Caption／Font.Name／click 各 Enter 一次、焦點還在、只送一次；網頁事件右鍵：跳到＝openJsEvent、重設＝拿掉、空的兩項都灰 | f95e85f |
| 1001 | 0.111.0 | 屬性視窗的值點了就能打（〔XD〕）；清單條理分明（EastSun） | HTML 區要雙擊才能改；〔程式碼〕頁同一行（處理函式寫在綁定那一行）列兩次 | HTML 區點一下值＝輸入框（雙擊照舊）；同一行合成一列「綁定＋處理函式」 | 面板：9 個監聽器合成一列、沒有重複的行；HTML 區點 top＝輸入框有焦點、Esc 不送、id 點了不能改 | （本次） |
| 1001 | 0.111.0 | （修正）改名後選取的是改好名字的元件（〔WE〕Rename） | 真 VS Code 測試偶爾失敗：改名時 JS 檔先改、畫面先重畫，新名字還不在 HTML → 退回選表單，而且把「要選的」也記成表單 | 那一次照樣顯示表單，但標成 initRoot、extension 保留要選的新名字，下一次重畫選回它 | probe：要選的不在頁面＝選表單＋initRoot；smoke：initRoot 不蓋掉 btnSave、一般 init 才會 | （本次） |
