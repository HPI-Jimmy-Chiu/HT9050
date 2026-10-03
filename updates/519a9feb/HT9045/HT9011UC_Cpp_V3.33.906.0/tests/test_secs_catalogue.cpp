// =============================================================================
//  test_secs_catalogue.cpp -- P4 STEP 1: 證明 SV/EC 登錄表**建得起來**
//
//  Wave: AI(W906-P4-SECS) 20260920.  Suite name (add_test): SecsCatalogue
//
//  ## 這支測試存在的理由
//
//  計畫書原本寫：「`tests/test_uHGemClass.cpp:178` 已經在跑
//  `g.AddSV(); g.AddEC();`，所以這個登錄表在本樹是已經可以被建起來的」。
//  **那個證據是錯的。** 那支測試建的是基底類別 `HTGem`，而
//
//      // SECSGEM/uHGemClass.h:227-228
//      virtual void AddSV()  {};
//      virtual void AddEC()  {};
//
//  是空的 inline virtual。它證明的是「空函式不會當掉」。
//  真正的 762 筆登錄在衍生類別 `HT9045Gem` 裡，那支測試從來沒有實例化過它。
//
//  ## 而真正的阻擋是一個 NULL 全域
//
//      // SECSGEM/uHGemHT9045_SV.cpp:415-425
//      Self->HGemPtr = HGem;                  // golden :61
//      if (Self->HGemPtr == NULL) return;     // PORT-ONLY guard
//
//  `THGem *HGem = NULL;`（`uHGemEquipment.cpp:3526`）**全樹零個賦值點**，
//  所以今天叫 `AddSV()` 會立刻 return，一筆都不登錄。
//  ⇒ 這是 §0.5 的「相依不存在」，缺的相依是**一個活的 `THGem` 實例**。
//
//  ## ⚠ 本檔最重要的一條斷言不是「有 762 筆」
//
//  是 **PART 1：在補上相依之前，它真的是 0 筆**。
//  沒有那一條的話，「762」證明不了是我補的相依起了作用 ——
//  也可能它本來就會自己登錄，而我只是站在旁邊。
//  （「不可能當掉的 gate 不是 gate」的同一個道理。）
//
//  ## 安全性（量過，不是推測）
//
//  `THGem::THGem()`（`uHGemEquipment.cpp:220`，本體 305 行）建了
//  `TClientSocket`/`TServerSocket` 並設 `Port=5100`/`6000`，
//  但**沒有任何 `Active=true`，也沒有 `->Open()`**，也不讀寫檔。
//  ⇒ 建構它不會開埠、不會碰真實檔。
// =============================================================================
#include "SECSGEM/uHGemEquipment.h"     // THGem, extern THGem *HGem
#include "SECSGEM/uHGemHT9045.h"        // HT9045Gem
#include "SECSGEM/SecsSvEcRegistration.h"
#include "CosFunction.h"                  // CosFunction.bEnable_SECS_GEM（第二道守衛）
#include "SECSGEM/SecsSvRead.h"          // AI(W906-P4-SECS): 讀一筆 SV 的值
#include "cprod.h"                       // RunInfo / IniConfig（對照組）

#include <cstdio>
#include <cstdlib>
#include <string>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

int main()
{
    printf("==== P4-1 SecsCatalogue: SV/EC 登錄表建得起來嗎 ====\n");

    // -----------------------------------------------------------------------
    //  PART 1 -- ★ 先證明「缺相依時真的是 0 筆」
    //
    //  這一條比最後的筆數更重要：它是唯一能證明「是我補的相依起了作用」的
    //  對照組。少了它，762 也可能是它本來就會做的事。
    // -----------------------------------------------------------------------
    printf("\n[1] ★ 對照組：HGem 還是 NULL 時，一筆都不該登錄\n");
    CHECK(HGem == NULL, "前置：THGem *HGem 開站時是 NULL（uHGemEquipment.cpp:3526）");
    {
        THGem probe;                       // 只是要一個 SvEcReg 來看
        CHECK(probe.SvEcReg.SV_ID != NULL, "SvEcReg.SV_ID 已配置（ctor 有建）");
        const int before = probe.SvEcReg.SV_ID ? probe.SvEcReg.SV_ID->Count : -1;
        CHECK(before == 0, "全新的 THGem 其 SV_ID 是空的");

        HT9045Gem h(AnsiString(""), &probe);
        h.AddSV();                         // HGem 仍是 NULL -> 守衛應該擋下
        h.AddEC();
        const int after = probe.SvEcReg.SV_ID ? probe.SvEcReg.SV_ID->Count : -1;
        printf("     HGem==NULL 時 AddSV/AddEC 之後 SV_ID->Count = %d\n", after);
        CHECK(after == 0,
              "★ HGem 是 NULL 時 AddSV()/AddEC() 一筆都沒登錄（守衛確實會擋）");
    }

    // -----------------------------------------------------------------------
    //  PART 2 -- 補上缺的相依，再跑一次
    // -----------------------------------------------------------------------
    printf("\n[2] 只補 HGem 還不夠 —— 還有第二道守衛\n");
    {
        THGem gem;
        HGem = &gem;
        HT9045Gem h0(AnsiString(""), &gem);
        h0.AddSV();
        printf("     只設 HGem、bEnable_SECS_GEM 仍為 %s 時 -> SV_ID->Count = %d\n",
               CosFunction.bEnable_SECS_GEM ? "true" : "false",
               gem.SvEcReg.SV_ID->Count);
        CHECK(CosFunction.bEnable_SECS_GEM == false,
              "前置：CosFunction.bEnable_SECS_GEM 預設是 false");
        CHECK(gem.SvEcReg.SV_ID->Count == 0,
              "★ 只補 HGem 還是 0 筆 —— 第二道守衛是 golden 自己的 "
              "`if(CosFunction.bEnable_SECS_GEM==true)`（uHGemHT9045_SV.cpp:428）");
        HGem = NULL;
    }

    printf("\n[3] 兩個相依都補上之後\n");
    {
        THGem gem;
        HGem = &gem;                       // 相依一：那三處註解要求的那一行
        // 相依二：golden 自己的客戶功能旗標。
        // ⚠ 這裡是**測試 fixture**，不是產品行為 —— 這台機器
        //   （CUSTOMER_CODE=868 CC_CYUEAN）的 FUNC_CC_CYUEAN()
        //   **沒有**設這個旗標，所以產品上它是 false。見本檔結尾的說明。
        const bool savedSecs = CosFunction.bEnable_SECS_GEM;
        CosFunction.bEnable_SECS_GEM = true;

        // ⚠ 登錄會寫進 HGem->SvEcReg（不是 h 自己的），因為
        //   AddSV 的本體拿的是 `&HGem->SvEcReg`。
        HT9045Gem h(AnsiString(""), &gem);
        h.AddSV();
        const int afterSV = gem.SvEcReg.SV_ID->Count;
        printf("     AddSV() 之後  SV_ID->Count = %d\n", afterSV);
        fflush(stdout);   // 下一步曾經 SegFault，不 flush 就看不到上面這行

        // ⛔ **相依三**：`AddEC()` 在這棵樹會 SegFault。
        //    它解參考 `fLotInfo`（`SetECDataPointer(1006, ...,
        //    fLotInfo->edtSysLotID, ...)`，uHGemHT9045_EC.cpp:462），
        //    而 `fLotInfo` 在這個行程是 NULL。
        //    gdb 實證：#0 HT9045Gem_AddEC  #1 HT9045Gem::AddEC  #2 main。
        //    ⇒ 不呼叫它。測試的工作是**報告缺什麼**，不是掛掉。
        //    ⚠ 注意 AddSV() 活過來了 —— 表示 `fMain` 在本行程非 NULL
        //      （SVID 1011 就在用 `fMain->palMainStatus`）。
        //      所以缺的不是「所有表單」，就是 `fLotInfo` 這一個。
        const int afterEC = afterSV;

        CHECK(afterSV > 0, "★ 補上相依之後 AddSV() 真的登錄了東西（0 -> >0）");
        // ⚠ 計畫書原本寫的門檻是 745，但那個數字**從來不是量出來的**。
        //    實測：`HT9045Gem::AddSV()` 單獨貢獻 **741** 筆。
        //    另外 21 筆 GEM 標準 SV（GemClock / GemControlState /
        //    GemLinkState / SECSCommunicationMode…）不在這支函式裡，
        //    而在 `THGem::FormCreate()`（uHGemEquipment.cpp:2220-2232+）——
        //    那是 VCL 的表單生命週期處理常式，**相依四**。
        //    741 + 21 = 762，與靜態掃描的結果一致。
        //    ⇒ 這裡斷言 741（這支函式真正負責的那些）。要湊到 762
        //      必須另外證明呼叫 FormCreate() 是安全的，那是下一步。
        //AI(W906-S09-ST) 20260930: recalibrated 741 -> 769 after St02 lifted 4 SV gates in
        //   SECSGEM/uHGemHT9045_SV.cpp (merge 53868b63): [G2] :455 SVID 1012/1013 (2, golden :74-75),
        //   [G19] :481 SVID 1038/1039 (2, golden :96-97), [G20] :606 SVID 1164-1179 (16, golden :215-230),
        //   [G16] :1154 SVID 37501-37508 (8, golden :739-746) => 741 + 28 = 769 (measured, SV_ID->Count).
        //   16 `#if 0 // GATE` blocks remain in that file; each lift moves this number again.  AI(W906-B21-SECSCAT) 20261001: St02 MR !62 lifted [G3] :459 SVID 1014-1016 (3, golden :76-78) => 769 + 3 = 772 (measured, gate b21a); 15 blocks remain.  AI(W906-ST02-C16) 20261003 (St02-E): ST02-C16 (Q83 = A, as V912) comments out SV 2022 "Use Die Force" (SECSGEM/uHGemHT9045_SV.cpp:790; it is EC 2022 now) => 772 - 1 = 771.
        CHECK(afterSV == 771,
              "★★ AddSV() 登錄 771 筆（741 + St02 解開的 5 個 gate 共 31 筆 − C16 改成 EC 的 SV 2022）");
        CHECK(afterEC == afterSV,
              "（AddEC 因缺 fLotInfo 而刻意未呼叫 —— 見上面註解）");

        // --- 平行清單的長度必須一致，否則按 index 取欄位會錯行 ---
        const int n = gem.SvEcReg.SV_ID->Count;
        CHECK(gem.SvEcReg.SV_TYPE->Count == n, "SV_TYPE 與 SV_ID 等長");
        CHECK(gem.SvEcReg.SV_NAME->Count == n, "SV_NAME 與 SV_ID 等長");
        CHECK(gem.SvEcReg.SV_UNIT->Count == n, "SV_UNIT 與 SV_ID 等長");
        CHECK(gem.SvEcReg.SV_Remark->Count == n, "SV_Remark 與 SV_ID 等長");
        CHECK((int)gem.SvEcReg.SV_Ptr->Count == n, "SV_Ptr 與 SV_ID 等長");

        // --- 抽樣看內容像不像真的目錄 ---
        printf("\n     前 5 筆：\n");
        for (int i = 0; i < 5 && i < n; ++i) {
            printf("       ID=%-8s type=%-4s name=%-32s unit=%-8s ptr=%s\n",
                   gem.SvEcReg.SV_ID->Strings[i].c_str(),
                   gem.SvEcReg.SV_TYPE->Strings[i].c_str(),
                   gem.SvEcReg.SV_NAME->Strings[i].c_str(),
                   gem.SvEcReg.SV_UNIT->Strings[i].c_str(),
                   gem.SvEcReg.SV_Ptr->Items[i] ? "non-null" : "NULL");
        }

        int named = 0, withPtr = 0;
        for (int i = 0; i < n; ++i) {
            if (AnsiString(gem.SvEcReg.SV_NAME->Strings[i]).Length() > 0) ++named;
            if (gem.SvEcReg.SV_Ptr->Items[i] != NULL)         ++withPtr;
        }
        printf("\n     有名字的 %d/%d，指標非 NULL 的 %d/%d\n", named, n, withPtr, n);

        // ★ 在決定要不要鏡像 GetECDataValue 的 303 行之前，先量這個：
        //   VCL_NAME "0"=裸指標 "1"=TObject*（VCL 元件）"2"=AnsiString*
        //   如果絕大多數是 "0"，那「VCL 元件的 dynamic_cast 階梯」就不是
        //   主要成本，讀值可以先用一個小很多的派發覆蓋大部分項目。
        int vcl0 = 0, vcl1 = 0, vcl2 = 0, vclOther = 0;
        for (int i = 0; i < n; ++i) {
            const AnsiString v = AnsiString(gem.SvEcReg.VCL_NAME->Strings[i]);
            if      (v == "0") ++vcl0;
            else if (v == "1") ++vcl1;
            else if (v == "2") ++vcl2;
            else               ++vclOther;
        }
        printf("     VCL_NAME 分布：裸指標(0)=%d  TObject*(1)=%d  AnsiString*(2)=%d  其他=%d\n",
               vcl0, vcl1, vcl2, vclOther);

        // SECS-II 型別分布（決定裸指標那一側要處理幾種）
        int tAscii = 0, tOther = 0;
        for (int i = 0; i < n; ++i) {
            const int ty = atoi(AnsiString(gem.SvEcReg.SV_TYPE->Strings[i]).c_str());
            if (ty == 64) ++tAscii; else ++tOther;
        }
        printf("     SECS 型別：ASCII(64)=%d  其他=%d\n", tAscii, tOther);
        CHECK(vcl0 + vcl1 + vcl2 + vclOther == n, "VCL_NAME 分布加總等於筆數");
        CHECK(n > 0 && named == n,
              "每一筆都有名字（目錄可讀）"
              " —— ⚠ 帶 n>0：原本寫成 named==n，在空清單上 0==0 必然通過，\n                 那是一條不可能當掉的斷言");
        CHECK(withPtr > 0, "★ 至少有一部分帶著指向真實全域的指標（不是空殼）");

        // -------------------------------------------------------------------
        //  讀值（P4-2）—— 本波只做 VCL_NAME "0"/"2"
        // -------------------------------------------------------------------
        printf("\n[4] 讀值：能讀幾筆、讀不到的有沒有誠實回報\n");
        int okRead = 0, notSup = 0, failRead = 0;
        for (int i = 0; i < n; ++i) {
            AnsiString v, why;
            if (ht9045::SecsSvReadValue(gem.SvEcReg, i, v, why)) ++okRead;
            else if (!ht9045::SecsSvReadSupported(gem.SvEcReg, i)) ++notSup;
            else ++failRead;
        }
        printf("     讀到 %d 筆，本波不支援 %d 筆，支援但讀失敗 %d 筆（共 %d）\n",
               okRead, notSup, failRead, n);
        CHECK(okRead + notSup + failRead == n, "三個計數加總等於筆數");
        CHECK(okRead >= 500,
              "★ 讀到的筆數與分布相符（裸指標 402 + AnsiString* 120 = 522）");
        //AI(W906-S09-ST) 20260930: 219 -> 229 = the 10 TObject* entries among the 28 lifted above
        //   ([G2] fMain->edTorue0/1 TEdit* x2, [G16] fObserver->lbSerialNumber01..04/lbFirmwareNumber01..04 x8);
        //   the other 18 ([G19] iGrossUPH/iNetUPH, [G20] iSVByBinCount[0..15]) are int* and read (522 -> 540).
        CHECK(notSup == 229,
              "★ 不支援的正好是那 229 筆 TObject*（219 + St02 解開的 10 筆，不是隨便失敗的）");

        // ★★ 完成條件要的是「值與直接讀那個全域相同」。
        //    挑三筆**來源已知**的逐一對照 —— 這是唯一能證明讀出來的是
        //    真的那個變數，而不是碰巧格式對的垃圾。
        printf("\n[5] ★★ 對照真實全域（完成條件：值要相同）\n");
        {
            // ⚠ 這一段第一版是直接比 `讀到 == 全域`，而在這個行程裡三個全域
            //    **都是空字串**（沒人跑過 LoadMachineConfig）⇒ 三條都是
            //    "" == ""，**零訊號**：一個永遠回空字串的實作也會全綠。
            //    ⇒ 先塞成各自不同的已知值再比，最後還原。
            const AnsiString sMD = RunInfo.MachineDefine;
            const AnsiString sMT = IniConfig.sMachineType;
            const AnsiString sSV = RunInfo.SoftwareVersion;

            RunInfo.MachineDefine   = AnsiString("P4-MD-1000");
            IniConfig.sMachineType  = AnsiString("P4-MT-1001");
            RunInfo.SoftwareVersion = AnsiString("P4-SV-1003");

            struct Expect { const char* svid; const char* what; AnsiString want; };
            const Expect kExp[] = {
                { "1000", "RunInfo.MachineDefine",    RunInfo.MachineDefine    },
                { "1001", "IniConfig.sMachineType",   IniConfig.sMachineType   },
                { "1003", "RunInfo.SoftwareVersion",  RunInfo.SoftwareVersion  },
            };
            for (size_t e = 0; e < sizeof(kExp)/sizeof(kExp[0]); ++e) {
                const int idx = gem.SvEcReg.SV_ID->IndexOf(AnsiString(kExp[e].svid));
                AnsiString v, why;
                const bool got = (idx >= 0) &&
                                 ht9045::SecsSvReadValue(gem.SvEcReg, idx, v, why);
                printf("       SVID %-5s %-26s 讀到=\"%s\"  全域=\"%s\"\n",
                       kExp[e].svid, kExp[e].what,
                       got ? v.c_str() : "(讀不到)", kExp[e].want.c_str());
                // ★ 帶 Length()>0：值必須是**非空**的，否則又退回空比較
                CHECK(got && AnsiString(kExp[e].want).Length() > 0
                          && v == kExp[e].want,
                      "★★ SVID 值與直接讀全域逐字相同（且非空）");
            }

            // ★ 三筆讀到的必須**彼此不同** —— 否則一個「回同一個字串」的
            //   實作也能通過上面每一條。
            {
                AnsiString v0, v1, v3, w;
                ht9045::SecsSvReadValue(gem.SvEcReg,
                    gem.SvEcReg.SV_ID->IndexOf(AnsiString("1000")), v0, w);
                ht9045::SecsSvReadValue(gem.SvEcReg,
                    gem.SvEcReg.SV_ID->IndexOf(AnsiString("1001")), v1, w);
                ht9045::SecsSvReadValue(gem.SvEcReg,
                    gem.SvEcReg.SV_ID->IndexOf(AnsiString("1003")), v3, w);
                CHECK(v0 != v1 && v1 != v3 && v0 != v3,
                      "★ 三筆讀到的值彼此不同（證明是逐筆解參考，不是回同一個東西）");
            }

            RunInfo.MachineDefine   = sMD;
            IniConfig.sMachineType  = sMT;
            RunInfo.SoftwareVersion = sSV;
            CHECK(RunInfo.MachineDefine == sMD
                  && IniConfig.sMachineType == sMT
                  && RunInfo.SoftwareVersion == sSV, "三個全域已還原");
        }

        // ★ 擾動：改全域，讀出來要跟著變（P2b 的那一條）
        printf("\n[6] ★ 擾動：改全域，讀值要跟著變\n");
        {
            const int idx = gem.SvEcReg.SV_ID->IndexOf(AnsiString("1000"));
            const AnsiString saved = RunInfo.MachineDefine;
            AnsiString a, b, why;
            RunInfo.MachineDefine = AnsiString("P4-PROBE-AAA");
            ht9045::SecsSvReadValue(gem.SvEcReg, idx, a, why);
            RunInfo.MachineDefine = AnsiString("P4-PROBE-BBB");
            ht9045::SecsSvReadValue(gem.SvEcReg, idx, b, why);
            RunInfo.MachineDefine = saved;
            printf("       改成 AAA 讀到 \"%s\"，改成 BBB 讀到 \"%s\"\n",
                   a.c_str(), b.c_str());
            CHECK(a == AnsiString("P4-PROBE-AAA") && b == AnsiString("P4-PROBE-BBB")
                  && a != b,
                  "★ 改全域讀值跟著變 —— 這條路是活的，不是快照或罐頭");
            CHECK(RunInfo.MachineDefine == saved, "全域已還原");
        }

        CosFunction.bEnable_SECS_GEM = savedSecs;   // 還原 fixture
        HGem = NULL;                       // 還原，別留給下一支測試
    }

    CHECK(HGem == NULL, "收尾：HGem 還原成 NULL");

    printf("\n==== 結果: %d PASS, %d FAIL ====\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
