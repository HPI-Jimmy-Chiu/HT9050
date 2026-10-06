> 保存來源：`.claude/skills/ht9045-login/references/login-dat-format.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# login.dat 位元組格式與 PW_Editor

## 結構

golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cprod.h:2710-2716`：

```
typedef struct {
    int  RecordCT;              // 位移 0，4 位元組
    char ID[1000][30];          // 位移 4，30000 位元組
    char PassWord[1000][30];    // 位移 30004，30000 位元組
    int  Level[1000];           // 位移 60004，4000 位元組
} PASS_WORD;                    // 共 64004 位元組
extern PASS_WORD USER;
```

- 讀：`ReadPassword()`（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cprod.cpp:1374-1386`）`fread` 整個結構；檔不在就整個清 0。
- 寫：`SavePassword()`（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cprod.cpp:1360-1372`）`fwrite` 整個結構，開檔失敗報 WAR1682。**不做轉換**：記憶體裡是什麼就寫什麼。
- `bUseLoginDatToSetLevel` 模式：檔裡帳號、密碼是 EncodeStr 過的，登入時先 DecodeStr（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:14986-15029`）。
- 下拉選單模式：同一個檔、同一個結構，但密碼是明文、帳號欄空白（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSecurity.cpp:795-811`）。

## PW_Editor（外部工具）

- 原始碼 `D:\HT9045\Password_V1.00.648\password_editor.cpp`：`:73-122` 產生 login.dat，`:97`／`:100` 對帳號、密碼 EncodeStr，一次只編前 30 格。
- 它的 EncodeStr（同檔 `:183-202`）跟 handler（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\common.cpp:267-293`）只有一處不同：轉出剛好是 0 的字元（第 k 個字元＝金鑰第 k 個字元＋1，例如第 1 個字是 'I'）時，PW_Editor 寫 00（字串斷掉），handler 寫金鑰字元（EncodeStr `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\common.cpp:280-281`）；handler 的 DecodeStr 在 `:308` 把金鑰字元還原成 0，20210330 的修正註解只在 `:308`。
- 移植樹做 Q9（寫 login.dat）時：**用 handler 版 EncodeStr**，才讀得回來；寫出 64004 位元組、帳號／密碼 EncodeStr、等級照填。代價：用 handler 版寫的檔，「轉出 0」那個字在 PW_Editor 打開時會顯示錯（PW_Editor 的 DecodeStr `D:\HT9045\Password_V1.00.648\password_editor.cpp:217` 只認 0）——這一格兩邊不可能一致。

## Steven01 這台實測（20260927，只讀、只數格子，沒印內容）

- `D:\HT9045\system\Gerneral.ini` 第 8 行 `CUSTOMER_CODE=791` ⇒ `bUseLoginDatToSetLevel`。
- `D:\HT9045\system\login.dat` 64004 位元組（等於結構大小）；8 格有帳號，DecodeStr 後都是英數字（不解開只有 1 個像英數字）⇒ 確實轉換過。
- `C:\winnt\system32\tech.com` 不存在。
- 這本的 `RecordCT`＝1000；PW_Editor V1.00.648 寫的是實際筆數（`D:\HT9045\Password_V1.00.648\password_editor.cpp:78` 歸零、`:102` 逐筆加，最多 30 格）⇒ 這本可能不是這版 PW_Editor 產生的，或產生後被改過。

<!-- preserved-content:end -->
