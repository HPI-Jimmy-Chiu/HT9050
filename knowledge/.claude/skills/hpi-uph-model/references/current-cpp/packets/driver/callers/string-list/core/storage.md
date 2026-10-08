# 核心儲存、Count 與排序

定位`TObject`／`TStrings`／`TStringList`、`items_`／`objects_`／`Count`／`bindAccessors`／`syncCount`、`Add`／`AddObject`／`Insert`／`Delete`／`Clear`、`GetString`／`SetString`／`GetObject`／`SetObject`、`Assign`／`IndexOf`／`First`／`Sort`／`Find`。完整選定定義見 [manifest](source-manifest.json)。

## 介面與索引

`TStrings : TObject`有虛擬destructor與Clear／Add／GetString／SetString／GetCount純虛擬介面，`count()`轉交GetCount。這個基底沒有Count資料欄位或Strings accessor；透過TStrings*要按此介面判斷，不能套用TStringList*的public屬性。
TStringList把字串與TObject*分別存於平行vectors，索引從0開始。constructor以this建Text／CommaText／DelimitedText代理、預設Delimiter為逗號、QuoteChar為雙引號，bindAccessors設定Strings／Objects的owner與Count。
值copy constructor及copy assignment宣告為delete；header歷史copy／move敘述不當成已做ABI／生命期測試。

| 函式 | 選定body行為 |
| --- | --- |
| Add／AddObject | 追加字串與null／指定指標，syncCount，回傳最後一個index |
| Insert | 負index夾到0，超出尾端夾到size；插入字串與null並syncCount |
| Delete | 無效index直接返回；有效則兩vectors刪同index，syncCount |
| Clear | 兩vectors清空並syncCount；沒有delete所指TObject |
| GetString／GetObject | 無效index回空AnsiString／0，讀取依值／指標回傳 |
| SetString／SetObject | 無效index直接返回，有效只更新對應項 |
| First | 空清單回預設AnsiString，否則第一項 |

`GetCount`把items_.size()轉int。Count是**公開int**，選定增刪／SetText／Assign透過syncCount維護；SetString／SetObject不改長度，Sort不改數目且未syncCount。header的read-only註解不是語言層唯讀限制：外部可寫Count，或改accessor.owner，不能以欄位值推導容器一定一致。
本段未驗allocation失敗、size→int溢位或多執行緒競態；平行vectors敘述只到所選正常流程。

## Assign 與 Objects 所有權

`Assign(const TStringList* src)`先拒null與src==this；其餘複製items_、objects_、Delimiter及QuoteChar後syncCount。自我Assign不先清空。Objects複製是指標值，沒有深拷貝、delete或所有權轉移證據。
這和 [Text指派](text.md) 不同：Text重建所有項並把各Objects設0。刪項／Clear只丟棄指標；實際caller誰建立與銷毀TObject仍待查。

## 搜尋與排序

IndexOf線性以AnsiString `==`找第一項，缺席回-1；歷史註解稱case-sensitive，但完整comparison／locale實作尚未在本單元讀完。
Sort先建indices，再stable_sort，以items_[a] < items_[b]比較，按同一indices重建字串與Objects配對。註解內objects_ reset字樣保留；body是保存配對，不能寫成全部設null。
Find是header inline的二分搜尋，使用同一 `<`／`==`，**清單須先依相同比較排序**。它無論命中與否都寫Index；命中後繼續縮左，可定位第一個equal，未命中給插入位置。
本段沒有把Find替換成IndexOf，也沒有執行BCB6 oracle；bytes／locale、客戶的sorted前置與全部caller另續。

回 [入口](index.md)。
