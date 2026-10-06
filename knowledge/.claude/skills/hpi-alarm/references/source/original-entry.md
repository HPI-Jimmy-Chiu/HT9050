# HT9045 告警解除機制

按需要選取以下章節，原文依順序保留。

- [HT9045 告警解除機制](original-entry/00.md)
- [References](original-entry/01.md)
- [0. 一分鐘結論](original-entry/02.md)
- [1. Golden 依據（檔案：行）](original-entry/03.md)
- [2. 實體面板鍵的完整清單](original-entry/04.md)
- [3. note 是兩段式 —— 這是最容易翻錯的一點](original-entry/05.md)
- [4. Alarm Reset 鍵不解除告警](original-entry/06.md)
- [5. 權限閘 —— 有，而且是 per-alarm-code](original-entry/07.md)
- [6. NonStop 兩頁的解除規則](original-entry/08.md)
- [7. web 端現況與缺口（20260922）](original-entry/09.md)
- [7.5 ⛔ 一定要停機的四類 —— 公司鐵律](original-entry/10.md)
- [7.6 告警佇列 —— 停機與不停機分開兩條](original-entry/11.md)
- [8. 陷阱清單](original-entry/12.md)
- [9. 告警「位置」的顯示 —— `ShowErrorUnit` / `FlushPanel`](original-entry/13.md)

# HT9045 告警解除機制

[讀取此節](original-entry/00.md#ht9045-告警解除機制)

## References

[讀取此節](original-entry/01.md#references)

## 0. 一分鐘結論

[讀取此節](original-entry/02.md#0-一分鐘結論)

## 1. Golden 依據（檔案：行）

[讀取此節](original-entry/03.md#1-golden-依據檔案行)

### note（會停機）

[讀取此節](original-entry/03.md#note會停機)

### message（通常不停機）

[讀取此節](original-entry/03.md#message通常不停機)

## 2. 實體面板鍵的完整清單

[讀取此節](original-entry/04.md#2-實體面板鍵的完整清單)

## 3. note 是兩段式 —— 這是最容易翻錯的一點

[讀取此節](original-entry/05.md#3-note-是兩段式--這是最容易翻錯的一點)

### ScanKey() 前段的一整排 return（`note.cpp:2899-2960`）

[讀取此節](original-entry/05.md#scankey-前段的一整排-returnnotecpp2899-2960)

## 4. Alarm Reset 鍵不解除告警

[讀取此節](original-entry/06.md#4-alarm-reset-鍵不解除告警)

## 5. 權限閘 —— 有，而且是 per-alarm-code

[讀取此節](original-entry/07.md#5-權限閘--有而且是-per-alarm-code)

### 第一層：獨立解除密碼 `DoUnlockPassword()`

[讀取此節](original-entry/07.md#第一層獨立解除密碼-dounlockpassword)

### 第二層：一般密碼 `DoPassword()` / `DoPassword_MBox()`

[讀取此節](original-entry/07.md#第二層一般密碼-dopassword--dopassword_mbox)

### ⚠ message 的非對稱：實體鍵繞過密碼

[讀取此節](original-entry/07.md#-message-的非對稱實體鍵繞過密碼)

### V906 現況（20261002，todo D-034，`AI(W906-D034)`；細節見 `ht9045-login` §10）

[讀取此節](original-entry/07.md#v906-現況20261002todo-d-034aiw906-d034細節見-ht9045-login-10)

### 第三層：SpecialPanel 密碼（`PanSpecialNoteClick`，只有 note）

[讀取此節](original-entry/07.md#第三層specialpanel-密碼panspecialnoteclick只有-note)

## 6. NonStop 兩頁的解除規則

[讀取此節](original-entry/08.md#6-nonstop-兩頁的解除規則)

## 7. web 端現況與缺口（20260922）

[讀取此節](original-entry/09.md#7-web-端現況與缺口20260922)

### ⚠ 20260922 實機驗證抓到的缺陷（已修）

[讀取此節](original-entry/09.md#-20260922-實機驗證抓到的缺陷已修)

### 7.x ⛔ 20260930：kCode==0 的通知型告警框怎麼關（INBOX 119，Jerry J-5；`AI(W906-J5-ACK)`）

[讀取此節](original-entry/09.md#7x--20260930kcode0-的通知型告警框怎麼關inbox-119jerry-j-5aiw906-j5-ack)

### 7.y ⛔ 20261003：C++ 單槽、網頁排隊 —— 「被丟下的框」一律要能關（筆電卡 S-17＝INBOX 137；`AI(W906-S17)`，St01）

[讀取此節](original-entry/09.md#7y--20261003c-單槽網頁排隊--被丟下的框一律要能關筆電卡-s-17inbox-137aiw906-s17st01)

## 7.5 ⛔ 一定要停機的四類 —— 公司鐵律

[讀取此節](original-entry/10.md#75--一定要停機的四類--公司鐵律)

## 7.6 告警佇列 —— 停機與不停機分開兩條

[讀取此節](original-entry/11.md#76-告警佇列--停機與不停機分開兩條)

## 8. 陷阱清單

[讀取此節](original-entry/12.md#8-陷阱清單)

## 9. 告警「位置」的顯示 —— `ShowErrorUnit` / `FlushPanel`

[讀取此節](original-entry/13.md#9-告警位置的顯示--showerrorunit--flushpanel)

### 9.1 完整的一條線

[讀取此節](original-entry/13.md#91-完整的一條線)

### 9.2 V906 的缺口 —— 以及它**不需要**翻譯

[讀取此節](original-entry/13.md#92-v906-的缺口--以及它不需要翻譯)

### 9.3 四個一定會踩的點

[讀取此節](original-entry/13.md#93-四個一定會踩的點)

### 9.4 tsHandler 的版面（改畫面前必讀）

[讀取此節](original-entry/13.md#94-tshandler-的版面改畫面前必讀)

### 9.5 已實作：把 Motion View 內嵌到 `Panel5`

[讀取此節](original-entry/13.md#95-已實作把-motion-view-內嵌到-panel5)

### 9.6 機種差異 —— HT9050 上目前有兩處錯位

[讀取此節](original-entry/13.md#96-機種差異--ht9050-上目前有兩處錯位)

### 9.7 待新增的面板：`palMagazineTray`

[讀取此節](original-entry/13.md#97-待新增的面板palmagazinetray)

### 9.8 `reDescription` 的內容 —— `TfNote::ErrShowToForm()`

[讀取此節](original-entry/13.md#98-redescription-的內容--tfnoteerrshowtoform)

### 9.9 交叉驗證：code 的單元編號 vs position

[讀取此節](original-entry/13.md#99-交叉驗證code-的單元編號-vs-position)

### 9.10 逐 unit 的 mv 覆寫

[讀取此節](original-entry/13.md#910-逐-unit-的-mv-覆寫)

### 9.11 HT9050 的告警頁

[讀取此節](original-entry/13.md#911-ht9050-的告警頁)

### 9.12 `submit()` 第一段不通時必須往下掉（20260924 修）

[讀取此節](original-entry/13.md#912-submit-第一段不通時必須往下掉20260924-修)

### 9.13 `Alert.Note.html` 的寬度是 1184 不是 972（20260924 修）

[讀取此節](original-entry/13.md#913-alertnotehtml-的寬度是-1184-不是-97220260924-修)

### 9.14 ⛔ 更正：`Message` 與 `UnitName` 也是 **Code 查表**，不是 position

[讀取此節](original-entry/13.md#914--更正message-與-unitname-也是-code-查表不是-position)

#### 完整的來源表

[讀取此節](original-entry/13.md#完整的來源表)

#### 前端自己做，C++ 不用碰（使用者 20260924 裁定）

[讀取此節](original-entry/13.md#前端自己做c-不用碰使用者-20260924-裁定)

#### 這條為什麼會錯

[讀取此節](original-entry/13.md#這條為什麼會錯)
