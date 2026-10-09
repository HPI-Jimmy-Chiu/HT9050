# uPlateInfo::LoadFile 的順序與失敗分支

以 `Public/HTEditList.cpp` 的 `uPlateInfo::LoadFile(AnsiString sFileName)` 定位。
它回傳void，原文沒有成功旗標、transaction、rollback或對reader／parser結果的錯誤回報。

1. `HPSuckGroupList->Count>0` 就直接 `Clear()`，先移除清單裡的舊指標。
2. `file_buf=ReadDataFromFile(sFileName)`，直接送進 `cJSON_Parse(file_buf)`。
3. `loaded_root==NULL` 就return，舊群組已先被移除。
4. 用 `cJSON_GetArraySize(loaded_root)` 取得根的child數，依序 `GetArrayItem` 取child。
5. 取child的 `HPSuckTeamList`；只有「是array、長度0、而且不是最後一項」才略過。
6. 其他項目都new group、`LoadJSONFile(r_single)`，再交 `AddHPSuckGroup(HPGroup)`。

reader只在明確fopen失敗分支回NULL。repo的ParseWithOpts對NULL直接回NULL，
所以該路徑不會由parser對NULL做strlen，但LoadFile仍先清掉群組。
reader的未核seek／tell／malloc／fread與尾部未知bytes界線沿用 [reader本體](../readers.md)。
呼叫端沒帶有效長度；檔案NUL、短讀、文字轉換不能靠LoadFile補救或判定。

## 跳過與拒收是兩個不同條件

`LoadFile` 特別保留最後一筆空array group；但缺key、非array、非物件child也會進new group。
`uHPSuckGroup::LoadJSONFile` 遇NULL直接return；key不是array就不新增team。
`AddHPSuckGroup(uHPSuckGroup*)` 若已有最後group且其 `ExtractLastTeam()==NULL`，
直接return，不把新group放進清單。呼叫端沒有對該分支delete新group。
所以最後空group的保留規則不能推成「任意無效或空group都會成功新增」。

`ExtractSuckGroup`／`ExtractSuckTeam` 僅擋Count==0與負index，
沒有一般性的index>=Count guard；這些選定loop／Last入口的index來源已保存，
其他直接caller的index安全性未窮舉。

以上為指定commit本體的靜態推導，未執行缺檔、失敗parse、new失敗或重複LoadFile。
