// ===========================================================================
//  JsonBridge/Bindings.cpp -- 實例綁定表。手寫，不是產生的。
//
//  AI(W906-JSONBRIDGE-S2) 20260923.  NOT in golden.
//  規格：.claude/skills/ht9045-json-bridge/SKILL.md 4.1
//
//  分工：型別表（gen/sjson_*.gen.cpp）是**產生**的，因為它逐欄對應結構宣告，
//  結構一改就要跟著改；這張表是**手寫**的，因為「哪個實例、能不能寫、
//  存到哪」是設計決定，不是可以從程式碼推出來的東西。
//
//  ⚠ 同一個型別會有好幾個實例，語意不同（20260923 實測 golden 用量）：
//        TestIF_File    11,797 次   檔案裡那份，畫面單位（mm）
//        TestIF          5,598 次   執行中那份，馬達單位（0.01 mm）
//        TestIF_NET         32 次   FTP 快照 —— **不開**（使用者裁決）
//    兩者由 cUnitConvert.h 的 Do*Convert() 轉換，不是同一份資料的兩個名字。
// ===========================================================================
#include "JsonBridge/FieldDesc.h"

#include "vclcompat/vcl_compat.h"
#include "cprod.h"

namespace ht9045 {
namespace sjson {

extern const TypeDesc kType_SYSTEM_DEVICE_FORM;
extern const TypeDesc kType_LAST_LEVEL_SET;
extern const TypeDesc kType_SYSTEM_TEST_IF;
extern const TypeDesc kType_SYSTEM_TEMPERATURE;
extern const TypeDesc kType_SYSTEM_TRAY_FORM;
extern const TypeDesc kType_SYSTEM_TEST_MODE;

const Binding kBindings[] = {
    // -- 設定：檔案裡那份 --------------------------------------------------
    //
    // DeviceForm_File 由 golden TfContact::ReadFile()（cContact.cpp:365）
    // 從 Contact.Data 讀進來，64 處賦值、42 個相異欄位（20260923 實測）。
    // ⚠ 其中一部分在 CosFunction.bContactHeightSaveToContactIni 開著時改讀
    //   寫死路徑的 system\Contact.ini（golden cContact.cpp:371 szDir2），
    //   所以「這個欄位來自哪個檔」是執行期條件，不是靜態事實。
    //   讀方向不受影響（讀的是結構不是檔案）；寫方向（S6）必須處理它。
    //
    // Steven 20260924: sourcePorted false → true。JerryYang ca4e903 把
    //   TfContact::ReadFile 翻進 forms/fContact.cpp:1475，並接上 wb_serve 開機
    //   序列（log「contact.* chain loaded」）。實測 QPM5577_8：IndexContact
    //   = [-134, -134.28]、ForcePerPinN = 0.2156，與 Contact.Data 一致。
    { "deviceForm.file", &DeviceForm_File, &kType_SYSTEM_DEVICE_FORM, false,
      /*sourcePorted*/ true,
      "golden TfContact::ReadFile() @cContact.cpp:365 (Contact.Data); "
      "port forms/fContact.cpp:1475",
      "ported by JerryYang 20260924 (ca4e903); read at wb_serve boot" },

    // 執行中那份。唯讀（使用者 20260923 裁決）——它是 DoDeviceConvert() 的
    // 產物，單位是 0.01 mm，只給要看「機台在用的值」的頁面。
    //
    // Steven 20260924: sourcePorted false → true。DoDeviceConvert 本體在
    //   cUnitConvert.cpp:413；wb_serve 開機讀完檔後呼叫 DoStructUnitConvert()
    //   （tools/wb_serve.cpp:3433），cinitial.cpp 的 GATE N3-G8 也已由
    //   AI(W906-TRAY-READ) 退役。實測 IndexContact = [-13400, -13428]（×100）。
    { "deviceForm.live", &DeviceForm, &kType_SYSTEM_DEVICE_FORM, false,
      /*sourcePorted*/ true,
      "golden DoDeviceConvert() @cUnitConvert.cpp:62; port cUnitConvert.cpp:413",
      "motor units (0.01mm); converted from deviceForm.file by "
      "DoStructUnitConvert at wb_serve boot" },

    // -- 權限：二進位 ------------------------------------------------------
    //
    // AI(W906-JSONBRIDGE-S3) 使用者 20260923：「levelset 屬於二進制檔案，
    //   所以在使用 JSON 的時候，用陣列的方式處理就可以」。
    //
    // LAST_LEVEL_SET 只有一個成員 int AccessLevel[256]，所以 FieldDesc 的
    // 一維 int 陣列這條路直接就通，不需要任何二進位特例。
    //
    // ⚠ 這裡出去的是**記憶體裡那份**，而 golden 的 GetLevelSet()
    //   （cSecurity.cpp:1474）讀完檔之後還會做三種鉗制：客戶碼 CC_KYEC_LEE
    //   強制 [35]/[114]/[128]=2、[104]=3；[163] 一律 3；其餘
    //   CheckRange(0, bSecurityHave5Level?4:3)。所以這裡的值是**鉗制後**的，
    //   不是 levelset.dat 的位元組。那是對的 —— 畫面要顯示的是實際生效的權限。
    //
    // ⚠ 名字層（[00]…）**還沒有**。移植樹 cSecurity.cpp:71-260 那 179 筆
    //   mySecurityPal.push_back 整段在 #if 0 裡（gate SEC1），執行期
    //   mySecurityPal 是空的。名字要嘛在 build 時從 golden 抽，要嘛先解閘。
    //   在那之前這個綁定回的是**索引定址的裸陣列**，誠實但不好讀。
    { "levelSet", &LevelSet, &kType_LAST_LEVEL_SET, false,
      /*sourcePorted*/ true,
      "golden TfSecurity::GetLevelSet() @cSecurity.cpp:1474 (levelset.dat)",
      "256 int32, post-clamp; names pending SEC1 un-gate" },

    // -- S4：最大的一個 ----------------------------------------------------
    //
    // AI(W906-JSONBRIDGE-S4) 20260923.
    //
    // ⚠ 788 個成員，不是 351。351 是「反向大括號配對咬到 cprod.h:2156 註解裡
    //   的 `{`」之後量到的尾巴 —— 那個誤判一度寫進 SKILL.md、commit message
    //   與一封已寄出的信。細節見 SKILL.md §十二。
    //
    // TestIF_File 由 golden TfSetup::ReadFile()（cSetUp.cpp:2199，830 行）填，
    // 而那正是 JerryYang 20260922 交付的四個函式之一 —— 移植樹有它
    // （cSetUp.cpp:416）。所以這是**第一個 sourcePorted=true 的設定綁定**。
    { "testIF.file", &TestIF_File, &kType_SYSTEM_TEST_IF, false,
      /*sourcePorted*/ true,
      "golden TfSetup::ReadFile() @cSetUp.cpp:2199 + TFTestIF::ReadTestIFFile() "
      "@cTesterIF.cpp:563",
      "788 fields; ported by JerryYang 20260922 (HandlerCondition.Data "
      "[Configuration] first reaches TestIF_File in this tree)" },

    // 執行中那份。DoTestIFConvert() 從 _File 整包 memcpy 後把 10 個 mm 欄位
    // ×100 —— ⚠ 移植樹只翻了其中 6 個（cUnitConvert.cpp:376-381），缺
    // dMulti2DSH1Ofs_L/R 與 dMulti2DSH2Ofs_L/R。而且 DoStructUnitConvert()
    // 不在 wb_serve 的開機路徑上（cinitial.cpp:7175 的 #if 0），所以這份
    // 實務上仍是初值。
    //
    // Steven 20260924: 上面「不在開機路徑上」已過期 —— wb_serve 開機會呼叫
    //   DoStructUnitConvert()（tools/wb_serve.cpp:3433），cinitial.cpp 的
    //   N3-G8 也已退役。實測 dSiteXPitch = 3000（檔案 30 ×100）。
    //   sourcePorted 改 true（DoTestIFConvert 本體在 cUnitConvert.cpp:370），
    //   但「10 個 ×100 欄位只翻 6 個」仍成立（20260924 重數），那 4 個
    //   dMulti2DSH* 欄位在 live 裡是未換算的 _File 值。
    { "testIF.live", &TestIF, &kType_SYSTEM_TEST_IF, false,
      /*sourcePorted*/ true,
      "golden DoTestIFConvert() @cUnitConvert.cpp:27; port cUnitConvert.cpp:370",
      "motor units; converted at wb_serve boot by DoStructUnitConvert, but "
      "only 6 of golden's 10 x100 fields are translated (dMulti2DSH1Ofs_L/R, "
      "dMulti2DSH2Ofs_L/R are copied unconverted)" },

    // -- S5：溫控／料盤／測試模式 -------------------------------------------
    //
    // Temperature 的三個欄位（fWorkTemperBase / fSoakTime / iMachineTempMode）
    // 是 WebBridgeTags.cpp 目前唯一從配方檔載進來的活資料（temp.sv/soak/mode），
    // 其餘 ~90 個欄位有 loader 但沒有 tag。這個綁定把整包都送出去。
    { "temperature", &Temperature, &kType_SYSTEM_TEMPERATURE, false,
      /*sourcePorted*/ true,
      "golden TfTemp_Set::ReadTempFile() (Temperature.Data)",
      "223 fields; only 3 of them currently have tags (temp.sv/soak/mode)" },

    { "trayForm", &TrayForm, &kType_SYSTEM_TRAY_FORM, false,
      /*sourcePorted*/ true,
      "golden TfTrayForm::ReadFile() @cTrayForm.cpp:364 (Tray.Data)",
      "44 fields" },

    { "testMode", &TestMode, &kType_SYSTEM_TEST_MODE, false,
      /*sourcePorted*/ true,
      "golden ReadTestMode() @cprod.cpp:3464 (TestMode.Data)",
      "5 fields; ReadTestMode short-circuits when "
      "CosFunction.bLastSetInSetUpFile is false" },
};

const std::size_t kBindingCount = sizeof(kBindings) / sizeof(kBindings[0]);

}  // namespace sjson
}  // namespace ht9045
