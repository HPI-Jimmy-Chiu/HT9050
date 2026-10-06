# 移植樹註解慣例（給 JsonBridge 實作者）

按需要選取以下章節，原文依順序保留。

- [移植樹註解慣例（給 JsonBridge 實作者）](comment-convention/00.md)
- [〇、先看規模](comment-convention/01.md)
- [一、格式](comment-convention/02.md)
- [二、標籤怎麼取（`W906-` 後面那一段）](comment-convention/03.md)
- [三、什麼情況下才寫（**不是每行都寫**）](comment-convention/04.md)
- [四、註解裡寫什麼](comment-convention/05.md)
- [五、標記符號（⚠ 這類）的用法](comment-convention/06.md)
- [六、語言、編碼、與 golden 註解的處置](comment-convention/07.md)
- [七、本專案（JsonBridge）建議用的標籤前綴](comment-convention/08.md)
- [八、一頁檢查表](comment-convention/09.md)

# 移植樹註解慣例（給 JsonBridge 實作者）

[讀取此節](comment-convention/00.md#移植樹註解慣例給-jsonbridge-實作者)

## 〇、先看規模

[讀取此節](comment-convention/01.md#〇先看規模)

## 一、格式

[讀取此節](comment-convention/02.md#一格式)

### 1.1 真實樣本

[讀取此節](comment-convention/02.md#11-真實樣本)

## 二、標籤怎麼取（`W906-` 後面那一段）

[讀取此節](comment-convention/03.md#二標籤怎麼取w906--後面那一段)

### 2.1 三種觀察得到的取法

[讀取此節](comment-convention/03.md#21-三種觀察得到的取法)

### 2.2 續作、修正、整併有固定尾碼

[讀取此節](comment-convention/03.md#22-續作修正整併有固定尾碼)

## 三、什麼情況下才寫（**不是每行都寫**）

[讀取此節](comment-convention/04.md#三什麼情況下才寫不是每行都寫)

### 3.1 檔案／區塊開頭：說明「這個檔為什麼存在」與「哪些不在範圍內」

[讀取此節](comment-convention/04.md#31-檔案區塊開頭說明這個檔為什麼存在與哪些不在範圍內)

### 3.2 `#include`：只要它是為某一次工作而加的

[讀取此節](comment-convention/04.md#32-include只要它是為某一次工作而加的)

### 3.3 守衛／提前 return：「不寫的話看起來可以刪」

[讀取此節](comment-convention/04.md#33-守衛提前-return不寫的話看起來可以刪)

### 3.4 與 golden 不同、或刻意不翻譯的地方

[讀取此節](comment-convention/04.md#34-與-golden-不同或刻意不翻譯的地方)

### 3.5 gate（`#if 0`／stub）：必須寫出「拆掉會怎樣」與「要跟誰一起拆」

[讀取此節](comment-convention/04.md#35-gateif-0stub必須寫出拆掉會怎樣與要跟誰一起拆)

### 3.6 修掉一個「看起來本來就對」的缺陷

[讀取此節](comment-convention/04.md#36-修掉一個看起來本來就對的缺陷)

### 3.7 更正／宣告前一則註解已過期（**移植樹最有特色的一條**）

[讀取此節](comment-convention/04.md#37-更正宣告前一則註解已過期移植樹最有特色的一條)

## 四、註解裡寫什麼

[讀取此節](comment-convention/05.md#四註解裡寫什麼)

## 五、標記符號（⚠ 這類）的用法

[讀取此節](comment-convention/06.md#五標記符號-這類的用法)

## 六、語言、編碼、與 golden 註解的處置

[讀取此節](comment-convention/07.md#六語言編碼與-golden-註解的處置)

## 七、本專案（JsonBridge）建議用的標籤前綴

[讀取此節](comment-convention/08.md#七本專案jsonbridge建議用的標籤前綴)

### 7.1 提議

[讀取此節](comment-convention/08.md#71-提議)

#### ⚠ 更正（20260923）：S8～S11 換成 `SJSON-`，所以這個專案現在有**兩個**標籤家族

[讀取此節](comment-convention/08.md#-更正20260923s8s11-換成-sjson-所以這個專案現在有兩個標籤家族)

### 7.2 為什麼這樣取

[讀取此節](comment-convention/08.md#72-為什麼這樣取)

### 7.3 本專案額外的三條硬性要求（來自 SKILL.md，寫成註解形式）

[讀取此節](comment-convention/08.md#73-本專案額外的三條硬性要求來自-skillmd寫成註解形式)

## 八、一頁檢查表

[讀取此節](comment-convention/09.md#八一頁檢查表)
