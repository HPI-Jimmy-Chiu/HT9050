#ifndef MachineTypeH
#define MachineTypeH
//AI(W0-TAIL) 20260626: de-VCL standalone -- was implicitly fed AnsiString/vector by includer.
#include "vclcompat/vcl_compat.h"   //AI(W0-TAIL) 20260626: AnsiString for ChangeToPercentage/ChangeToFloat templates (was <vcl.h> via includer)
#include <vector>                    //AI(W0-TAIL) 20260626: vector for vec_clr / template params (was via includer)
#include <windows.h>                 //AI(W0-TAIL) 20260626: DEFAULT_CHARSET for SetCompomentLanguage (uninstantiated template)
using std::vector;                   //AI(W0-TAIL) 20260626: BCB6 had 'using namespace std' via includer; restrict to vector

//機台資訊-------------------------
#define ALIAS "HT9045"
#define HT9045
#define HANDLER_CONTROL_ATC
#define ATC_HEAD_COUNT 32                                                       //Ifor 20160514 修改ATC Heat 最大數32
#define INDEX_HEAT_COUNT 32
//=============  Specific define  ==============================================
//#define Carry4
//=============  DEBUG TEST  ===================================================
//#define SCAN_SIMULTE
//#define DEBUG
//#define DEBUG_DUTONOFF
//#define DEBUG_HANGUP_NO_HOME
//#define DEBUG_NEW_ALARM_DESCRIPTION
//#define DEBUG_TRY_CATCH
//#define DEBUG_HOME                                                            //wei 20170317 快速回Home開關
//#define DEBUG_32SITE
//#define DEBUG_SPIL
//#define DEBUG_HT1032
//#define DEBUG_GALIL
#define DEBUG_GALIL_CHECK_Y
//#define DEBUG_INDEX_UPH                                                       //Steven 20170818 : 測試UPH用
//#define DEBUG_AUTO_CLEAN                                                      //JerryYang 20160824 Auto clean log
//#define DEBUG_AutoHomeLog
//#define DEBUG_ATC
//#define BOOST_FUNCTION_TEST
//#define DEBUG_SOFTWARE_EXEC_TIME

//#define INDEX_PROTECT_TMOVE
//#define DEBUG_AutoSiteMap
//#define DEBUG_OneCycleHotPlate
//#define DEBUG_ShuttleCanMoveL
//#define DEBUG_OneCycleContinous                                               //Sam 20221124 : 新增連續 OneCycle 功能 Debug
//#define DEBUG_AutoZMotor                                                      //RogerYang 2050512 偉測Auto3Z升降問題
#define USE_CompareCommandPos
//#define USE_EC_CHANGE
//#define DEBUG_COLOR
//#define BETA_VTestSummaryFile                                                 //RogerYang 20250809 偉測Summary文件修改
//=============  USER DEFINE ===================================================
// AI(W906-GATE-SIMCFG) 20260918: 使用者 20260918 手動打開這個旗標（原話：
// 「軟體模擬僅有開啟SOFT_SIMULTE，否則就是機台上能跑，就這兩種」）。
// 下面的 #ifndef 不改變那個決定——**不帶任何旗標時，這裡與他的編輯完全等價**，
// F5 建出來的 build/ 一位元都不會變。
//
// 它只是讓 build system 有辦法挑另一種組態：cmake -DW906_NO_SOFT_SIMULTE=ON。
// 為什麼需要：這棵樹的 fidelity harness 整套是對「旗標關閉」推導的
// （十幾支測試的註解直接寫 "SOFT_SIMULTE is NOT defined in this build"）。
// 旗標一開，14 支測試失敗，逐筆追下去全部落在 #ifdef SOFT_SIMULTE 臂上：
//   * MyLaneIo.cpp CheckPortRangeErr 整段跳過 -> 恆回 0（golden :582）
//   * MyLaneIo.cpp IOInputBit/IOInputByte 讀取失敗回 true/1（golden :424/:485）
//     -> 讀不到的感測器一律 ON -> 氣缸瞬間到位 -> 軌跡與 tick 數全變
//   * Automation/automation.cpp:241 OLPServer->Port 6671 -> 5001
//   * 另有 cinitial / BarCode / HanaART / AGV_E84 各自的臂
// 沒有一筆是缺陷。把那 14 支關掉等於自廢 gate；正確做法是讓 gate 建出貨組態。
#ifndef W906_NO_SOFT_SIMULTE
#define SOFT_SIMULTE
#endif
//#define SOFT_SIMULTE_EtherCAT                                                 //Sam 20230724 : 測試 SensorEtherCAT 用
//#define ASE_KaohSiung
//#define KOREA
//#define HiSilicon
//#define MTK_Version
//#define AMD_Version
//RF90@HT-9045W      HT-9045W@RF90
//#define BETA_VERSION
//#define FOR_EVAN                                                              //吳如春要的版本
//#define FOR_QLE                                                               //渠梁的版本
//#define FOR_NVIDIA_2D_SORT                                                    //Nvidia 2D SORT的版本
#define RTCErrorType                                                            //RogerYang 20260611 : 開放 //wei 20221222 RTC ARM Error
//#define FOR_NVIDIA                                                            //Nvidia 的版本
//#define FOR_ASECL_L8                                                          //ASECL L8的版本
#ifdef SOFT_SIMULTE
    #define DEBUG_ATC
#endif
//=============  PCIE-1203 write path (BU-C4) =================================
//AI(W906-BU-C4) 20260918: 使用者裁決「甲」—— 兩個都開。
//
//  這兩個旗標**不是**「要不要有這個功能」的開關，是**建置決定**。使用者
//  20260918：「軟體模擬僅有開啟 SOFT_SIMULTE，否則就是機台上能跑，就這兩種」。
//  所以沒有對應的命令列參數，也不會有。
//
//  WB_PUMP_1203_CONTROL       wb_serve 是否掛出 1203 的命令面（pci1203.* 指令）
//  WB_PUMP_1203_CONTROL_LIVE  命令是 LIVE 還是 dry run
//                             關 = 每個命令照樣驗證、格式化、記錄，但不呼叫 vendor
//                             開 = 命令真的送到卡上
//
//  ⚠ 跟 HAVE_PCI1203 是**正交**的，不要混：
//      HAVE_PCI1203          vendor SDK 有沒有編進這顆二進位（廠商呼叫的真本體）
//      WB_PUMP_1203_CONTROL  瀏覽器有沒有路可以走到那些呼叫
//  這台筆電沒有 1203 卡，HAVE_PCI1203 是關的，所以即使兩個都開，
//  Pci1203ControlEnable() 會誠實回 "not linked: this binary was built without
//  HAVE_PCI1203"，命令面仍然可以端到端驗（回的是拒絕，不是假的成功）。
//  同事的機台端 HAVE_PCI1203=1，同一份碼就是真的會動。
//
//  ⚠⚠ 在 HAVE_PCI1203=1 的建置上，LIVE 開著代表**網頁上一次點擊可以讓伺服轉動**。
//      那是使用者 20260918 明確要的（「我會跑機台，而且必須」）。
#define WB_PUMP_1203_CONTROL
#define WB_PUMP_1203_CONTROL_LIVE   //AI(W906-IOWEB-P17) 20260925: ON again -- user ruling 20260925 "我現在輸出 就是要一直live 我開啟軟體 就是要控制了 要一直live". Every pci1203.* command, every HW.IoSetView output button, every ENGINE output through the A4-7 route (WB_ENGINE_IO_1203, EOF) and the Pci1203AxisIniTick HLMT write-back (W2) now REACH THE CARD. What that means at start-up is listed at EOF. (Was OFF since IOWEB-P9 20260924 for the first IO-web start.)
//AI(W906-Q34-1) 20260923: 1203 的**唯讀監看**面。與上面兩個控制面旗標正交：
//  這個開的是「讀卡」，那兩個開的是「寫卡」，兩者沒有互相依賴。
//
//  ⚠ 這一行單獨存在沒有意義，而且**會說謊**：`build.install1203Monitor` 這個
//  tag 只看巨集有沒有定義（`WebBridgeTags.cpp:732`），所以只加 define 會讓
//  網頁對操作員說「監看已安裝」，而 `Pci1203Monitor()` 其實恆 0
//  （`WebBridgeTags.cpp:1299`）⇒ 4,481 個 pci1203.* tag 全是 null。
//  因此它必須與 `tools/wb_serve.cpp` 的 `Pci1203MonitorEnable()` 呼叫點
//  **同一顆 commit**。20260922 曾有一次只改這一行的未 commit 半成品，
//  理由正是這個；那份工作在換機時遺失（從未進版控），這裡是重做。
//
//  ⓘ 不會弄紅 `test_wb_tags`：`tests/wb_buildfact_tags.h:42-45` 預寫的警告，
//  真正的觸發條件是**監視器活著**，而測試從不呼叫 Enable()
//  —— Enable() 的唯一呼叫點在 wb_serve，那不是任何測試的來源檔。
#define INSTALL_1203_MONITOR
//AI(W906-P13-STARTRING) 20260923: **啟動 EtherCAT 循環交換。使用者裁決「甲」。**
//
//  ## 為什麼開
//  20260918 已把 1203 的命令面設成 LIVE（上面兩行），但這一個還關著 ——
//  **兩個決定方向相反**。後果是同事的機台端會看到「指令回 SUCCESS 但軸沒動」：
//  命令送到卡上了，可是沒有人在跑 cyclic exchange，PDO 影像是凍住的。
//  而 bu-wave 的政策明寫「回傳 SUCCESS 不等於機台做了事」。
//  ⇒ 使用者 20260923 選「甲：也打開，讓 wb_serve 自己啟動 ring」。
//
//  ## 這**推翻**了一個先前的裁決，所以寫清楚
//  `docs/PCI1203_20260918_INTEGRATION_PLAN.md:442` 當時記的是
//  「❌ 不取 —— 它守著 Acm_MasStartRing，是硬體動作，要使用者在機台旁、
//  周邊淨空時才談」。那個顧慮沒有消失，只是使用者現在做了決定。
//  ⚠ 20260922 起的常設指示已把「機台會動」從裁決理由中移除
//  （判準改成「翻譯決定 vs 操作決定」），這一條屬於操作決定，而他決定了。
//
//  ## 它實際會做什麼（不要靠想的，這是碼裡量到的）
//  `EtherCAT/Pci1203Monitor.cpp:526-554` 的 `#ifdef` 放行一次
//  `Acm_MasStartRing(dev, 0)`：
//    * **只有 ring 0（motion ring）**。ring 1 是 DI/DO 模組與 ECAT-2515 junction，
//      也就是**帶線圈的那一環**，20260912 特地把迴圈從 `ring <= 1` 收窄成只打 0，
//      因為線圈風險全在 ring 1。**啟動 ring 1 是另一個決定，需要它自己的開關。**
//    * 九顆 SERVOPACK 全在 ring 0（實測站號 0/1/3/10/14/30/41/124/153）。
//    * 啟動 cyclic exchange 會把 slave 驅到 OP，並可能把卡片 DO 影像裡
//      既有的值推到實體輸出上。20260912 實測當時 75 個 DO port 全是 0x00，
//      但那是快照不是保證 —— 收窄呼叫就是為了不依賴那個快照。
//
//  ## 在哪一台會真的動
//  | | |
//  |---|---|
//  | 這台筆電 | `HAVE_PCI1203` 對 wb_serve **沒有定義**（只有 ht9045_pci1203_probe 那個 target 有），
//  |          | 且 exe 零個廠商 DLL 相依 ⇒ `Pci1203MonitorEnable()` 是樁，回 "not linked"。**不會動。** |
//  | 同事機台端 | `HAVE_PCI1203=1` ⇒ **開機時就會啟動 ring 0 的循環交換**。 |
//
//  ⇒ **在有卡的機器上第一次跑這版之前，先確認機台周圍沒有人。**
//  要反轉：把下面這行註解掉即可（它沒有配對的第二處狀態）。
//  ⚠ 依常設規則「武裝狀態不可在機器之間分岔」，這一行必須進版控並 push。
//#define WB_PUMP_1203_START_RING   //AI(W906-IOWEB-P9) 20260924: OFF for the first IO-web start -- no Acm_MasStartRing(dev,0) at card open (W1). DI/DO reads do not need it: ring 1 is never started and its reads update (field measurements 20260910-11 predate START_RING).
//=============  Hisi 專用版本 DEFINE Start===================================================
#define HISI_VERSION    "KL_HT9045_V02.30."                                     //海思主版號
#define HISI_FileName   "2.1"                                                   //kevin 20180626 ATC Ver
#define HISI_KYEC       "K301"                                                  //Ifor 20161109 add 京元海思版號
#define HISI_SPIL       "S016"                                                  //Ifor 20161109 add 矽品海思版號
#define HISI_ASE        "B015"                                                  //Ifor 20161109 add 日月光海思版號
#define HISI_JCET       "J012"
#define HISI_SCC        "C012"                                                  //Ifor 20170818 add 江陰SCC海思版號
#define HISI_SIGURD     "SG068"
#define HISI_TFME       "T012"
#define HISI_TFAMD      "T003"
#define HISI_VTEST      "V011.9"                                                //jou 20201201 add VTEST偉測海思版號

#define HISI_ATCVER     "200"                                                   //Ifor 20180808 : add 修改ATC版號再指定版本以上皆可跑
#define Record_Ver      "V02.31B015"                                            //kevin 20200121 紀錄給eKeep
#define Record_VerShow  ""                                                      //kevin 20200121 紀錄給eKeep
#define ASE_AOA         "B011Beta_AOA.V1.63"                                    //KenHsieh 20211015 : ASE要求版號新增AOA
#define SECSGEM_Version "V1.05"
//=============  Hisi 專用版本 DEFINE End  ===================================================
#ifdef SOFT_SIMULTE
    #ifndef DEBUG_HANGUP_NO_HOME
        #define DEBUG_HANGUP_NO_HOME
    #endif
#endif
//=============  Main Version DEFINE ===========================================
/*#ifdef HiSilicon
    #define MainVersion HISI_VERSION
#else
    #ifdef JSCC_Version
        #define MainVersion  "V3.33"
    #else
        #ifdef ASE_KaohSiung
             #define MainVersion "V02.31"
        #else
            #ifdef FOR_NVIDIA_2D_SORT
                 #define MainVersion "VisualSort.01.02"
            #else
                 #define MainVersion "V3.33"
            #endif
        #endif
    #endif
#endif*/
#define ATCVersion "."
//=============  SVN Revision Source DEFINE ====================================
//    #define SVNRevision  "868.0"                                              //JimmyChiu 20250510 : 版本號改由Project --> Version Info修改
    #define CSMCDLLRevision  "3.15.0.0"                                         //Isaac 20190606 : SMCC.dll版本檢查
    #define TemperatureDatavision  "625"                                        //Sam 20210106 : TemperatureData 新增版號來相容
    #define ATPDLLVersion  "1.0.0.1"                                            //JerryYang 20220317 : ATP dll

//注意事項----------------------------------------------------------------------
//  1. Memo跟List請用Clear(); 不要用Delete(i); 因為會吃記憶體
//  2. timeBeginPeriod(1); 只需要使用一次,可以讓cycle time更精準
//  3. 快速Compiler的東西會造成莫名其妙的錯誤!!
//  4. 使用除法和開根號還有%時，要注意變數是不是0
//  5. 注意ItemIndex不可以是-1
//  6. 注意PageControl->ActivePage不要用,會有死機的風險;請愛用PageControl->ActivePageIndex
//  7. USEOBJ("XLSfile.obj"); USEUNIT("XLSfile.pas");
//  8. GetDC()後的變數一定要用ReleaseDC釋放掉
//  9. 盡量不要使用Anstring轉數值的函式如StrToInt 或 .ToInt();要用char轉數值的方式如 atoi(), atof()
// 10. 浮點數最好不要用float, 要用 double, 不然容易會發生數值對不上的問題
//------------------------------------------------------------------------------
//int MachineTypeChoice;//0:HT9045 1:HT9046

//============= Customer Code define ===========================================
#define CC_HONPREC_QC             0
#define CC_HuaTian_NNAJING      729 //華天南京
#define CC_IFXTH_Thai           730 //IFXTH泰國
#define CC_Carsem_Thai          731 //Carsem 泰國

#define CC_Ramos                740 //Ramos Technology
#define CC_TechL_Vitenam        741 //TechL-Vitenam
#define CC_TechL_Korea          742 //TechL-Korea
#define CC_SFA_Semicon          743 //SFA_Semicon_Korea

#define CC_GOOGLE_Israel        750 //GOOGLE 以色列
#define CC_Mellanox_Israel      751 //Mellanox 以色列

#define CC_UTAC_Indonesia       763 //聯測科技 印尼
#define CC_Elmos_Germany        764 //Elmos德國
#define CC_Allegro_Philippines  765 //Allegro菲律賓
#define CC_Morningcore          766 //宸芯
#define CC_Paceis               767 //航芯源
#define CC_Sigmastar            768 //銳力
#define CC_BRAVETEK             769 //博發電子

#define CC_DJI_SZ               770 //大疆創新 深圳
#define CC_JSCC_OS              771 //長電微電子 (JSCC OS部門)
#define CC_TYTAN                772 //成都態坦
#define CC_CAPCON               773 //北京華封

#define CC_NXP_TJ               780 //恩智浦 天津
#define CC_XINYUN               781 //杭州芯云
#define CC_SCX                  783 //深測芯

#define CC_TAIJI_SEMI_SUZHOU    787 //太極半導體(蘇州)

#define CC_FOREHOPE_NINGBO      790 //甬矽 寧波
#define CC_SJ_Semiconductor     791 //盛合晶微
#define CC_SJ_Semiconductor_OS  792 //盛合晶微Open Short
#define CC_30JAVEE              793 //成都三零嘉微電子
#define CC_CAMBRICON            794 //上海寒武紀
#define CC_VATE                 795 //立衛科技
#define CC_ASIAOPTICAL          796 //亞洲光學
#define CC_HXYSEMI              797 //浙江航源芯
#define CC_LEADYO               798 //廣東利揚
#define CC_DENGLIN              799 //成都登臨科技

#define CC_BROADCOM_US          800 //Broadcom-US
#define CC_Tessolve_US          801 //Tessolve-US
#define CC_Infineon             802 //英飛凌
#define CC_TERADYNE_US          803 //泰瑞達 Teradyne-US
#define CC_TeraProbe            804 //TeraProbe 日本
#define CC_TATA                 805 //塔塔-印度
#define CC_STM                  806 //意法半導體 馬爾他
#define CC_IMEC_TAIWAN          809 //台灣愛美科

#define CC_AVAGO_Korea          810 //安華高 韓國
#define CC_PTI_3C               812 //力成 3C
#define CC_SFASemicon           813 //SFA Semicon

#define CC_TSMC_TAINAN          820 //台積電 台南
#define CC_TSMC_HSINCHU         825 //台積電 新竹
#define CC_ChenYuanXiang_CHINA  828 //西安 晨元翔

#define CC_EUROFINS             829 //歐陸

#define CC_HTKJXA_CHINA         830 //西安 天水華天
#define CC_RF360                831 //RF360
#define CC_LBSH                 832 //上海祿比
#define CC_Realtek              833 //瑞昱半導體
#define CC_RIGGER_MICRO         834 //鄭州銳傑微
#define CC_XINITECH             835 //北京芯力
#define CC_GIGA_FORCE_Zhejiang  836 //季豐 浙江
#define CC_GIGA_FORCE_Shanghai  837 //季豐 上海
#define CC_CSAMQ                838 //csamq 長沙安牧泉
#define CC_GONGJIN_SHANGHAI     839 //上海共進

#define CC_SANDISK_CHINA        840 //SANDISK 上海
#define CC_GONGJIN_SUZHOU       841 //蘇州共進
#define CC_Mathilda             842 //馬舍科技
#define CC_DoosanTesna          843 //DOOSAN TESNA
#define CC_Renesas_M            844 //Renesas Malaysia.
#define CC_STK                  845 //STK 日本
#define CC_Renesas              846 //Renesas 日本
#define CC_THINE                847 //Thine 日本
#define CC_SINOICTECH           848 //上海華嶺
#define CC_ChipOn               849 //上海芯旺

#define CC_ChipMos_TAINAN       850 //南茂 台南
#define CC_ChipMos_ZHUBEI       851 //南茂 竹北
#define CC_ITS                  852 //創量科技
#define CC_NEXPERIA_Guangdong   853 //廣州 安世
#define CC_Atec_Semiconductor   854 //上海旻艾半導體.
#define CC_JSSI_Semiconductor   855 //江蘇芯德半導體.
#define CC_Microchip_FR         856 // Microchip 法國
#define CC_Indie_US             857 //Indie 美國.
#define CC_CETC                 858 //中國電子-第58研究所
#define CC_XDXCT                859 //象帝先

#define CC_MAXIM_THAILAND       860 //MAXIM 泰國
#define CC_Microchip_Thai       861 //Microchip 泰國
#define CC_Microchip_Phil       862 //Microchip 菲律賓
#define CC_Microchip_China      863 //Microchip 中國
#define CC_Microchip_US         864 //Microchip 美國
#define CC_HANA_MICRON          865
#define CC_ITestInc             866 //iTest,Inc. 美國
#define CC_EMemory              867 //EMemory 力旺電子
#define CC_CYUEAN               868 //CYUEAN 确安科技
#define CC_PANTHER              869 //PANTHER 鴻谷科技

#define CC_ARDENTEC             870 //欣詮                                      //20160805 Mylin Modify Show Only Ambient or High for ARDENTEC
#define CC_FULCAP               871 //詮容                                      //2016.06.30 ,Brian
#define CC_AOSL                 872 //萬有半導體
#define CC_Nuvoton_Israel       873 //新唐 以色列
#define CC_GT                   874 //寰邦科技
#define CC_Novatek              875 //聯詠科技
#define CC_Sunplus              876 //凌陽科技
#define CC_Amlogic              877 //晶晨半導體
#define CC_Higon                878 //成都海光
#define CC_Kingston             879 //金士頓

#define CC_Spreadtrum           880 //展訊 上海
#define CC_Amazon               881 //Amazon
#define CC_Murata               882 //Murata 村田製作所
#define CC_Goertek              883 //歌爾微電子
#define CC_FMSH                 884 //上海復旦微電子
#define CC_HDSC                 885 //華大半導體
#define CC_SANECHIPS            886 //中興微電子 深圳
#define CC_HABANA               887 //HABANA
#define CC_GIS                  888 //業泓科技
#define CC_CENTER               889 //中芯國際

#define CC_UMC                  890 //聯電
#define CC_ATEC                 891 //艾科
#define CC_WINSTEK              892 //台星科
#define CC_SANAN                893 //三安光電
#define CC_UTAC_TW              894 //聯測科技 台灣
#define CC_BARUN                895 //百齡
#define CC_SIGURD_SUZHOU        896 //矽格 蘇州
#define CC_SILTERRA_CHINKIANG   897 //矽佳 鎮江
#define CC_AMD_SUZHOU           898 //AMD  蘇州
#define CC_YTEC                 899 //久元

#define CC_SPIL_CS              900 //矽品 中山                                  //KevinCheng 20251002 add 矽品中山客戶碼
#define CC_ISE_US               901 //ISE 美國
#define CC_HYGEIA_SUZHOU        902 //海京 蘇州
#define CC_JSI_HAOXING          903 //紹興 長電
#define CC_JINGJIAWEI_CHANGSHA  904 //長沙景嘉微
#define CC_ISE_SH               905 //上海月芯半導體科技
#define CC_INTEL_IL             906 //INTEL 以色列
#define CC_IBM_CANADA           907 //IBM 加拿大
#define CC_PGC                  908 //巨有科技
#define CC_ITESTSEMI            909 //安測半導體

#define CC_SPIL_SHINCHU         910 //矽品 新竹.
#define CC_SPIL_TAICHUNG_LOGIC  911 //矽品 台中.
#define CC_SPIL_CHINA_SUZHOU    912 //矽品 蘇州.
#define CC_HUAWEI               913 //華為.
#define CC_ANST                 914 //蘇州安盛
#define CC_VTEST                915 //無錫偉測半導體
#define CC_TFME_CHINA           916 //通富微電
#define CC_JCET_5               917 //江蘇長電 五廠
#define CC_BOJIAN               918 //蘇州 博劍
#define CC_VTEST_Shanghai       919 //上海偉測半導體

#define CC_KYEC_CHEN            920 //京元 記憶體.銅鑼廠
#define CC_KYEC_LEE             921 //京元 邏輯.
#define CC_KYEC_JCTHIU          922 //京元 科園
#define CC_DL_TEK               923 //東琳.
#define CC_KYEC_XILINX          924 //京元 Xilinx
#define CC_KYEC_STM             925 //京元 STM
#define CC_Advantest_GE         926 //Advantest 德國
#define CC_LATTICESEMI          927 //萊迪思半導體
#define CC_ITRI                 928 //工業技術研究院
#define CC_ASE_KaohSiung_K12    929 //日月光 高雄K12.

#define CC_ASE_SG               930 //日月光 新加坡.
#define CC_ASE_JP               931 //日月光 日本.
#define CC_ASE_Korea            932 //日月光 韓國.
#define CC_ASE_CL               933 //日月光 中壢.
#define CC_ASE_SH               934 //日月光 上海.
#define CC_ASE_N                935 //日月光 NXP.
#define CC_ASE_KaohSiung        936 //日月光 高雄.
#define CC_ASE_M                937 //日月光 Malaysia.
#define CC_ASE_KaohSiung_K3     938 //日月光 高雄K3
#define CC_ASE_KaohSiung_K11    939 //日月光 高雄K11 AOSH-1000.

#define CC_UTAC                 940 //Only for Open/Short Handler
#define CC_SIGURD_HUKOU         941 //矽格 湖口
#define CC_RFMD_BEIJING         942 //RFMD 北京.
#define CC_SCC                  943 //JSCC 江陰
#define CC_SCS                  944 //JSCS 新加坡.
#define CC_SIGURD_ChungXing     945 //矽格 中興
#define CC_SIGURD_PeiXing       946 //矽格 北興
#define CC_SCK                  947 //星科金朋 - 韓國
#define CC_RFMD_USA             948 //RFMD 美國
#define CC_ASE_KS               949 //日月光 崑山.

#define CC_APTOS                950 //群豐.
#define CC_WINBOND              951 //華邦.
#define CC_G_Link               952 //G Link.
#define CC_AMKOR                953 //艾克爾.
#define CC_GIGA                 954 //GIGA.
#define CC_LINGSEN              955 //菱生.
#define CC_Greatek              956 //超豐.
#define CC_PTI                  957 //力成
#define CC_THAILIN              958 //泰林
#define CC_JCET                 959 //江蘇長電

#define CC_OSE                  960 //華泰.
#define CC_MTI                  961 //聚成.
#define CC_NUVOTON              962 //新唐
#define CC_UPRTEK               963 //群耀.
#define CC_Eutrend              964 //聿勤                                      //pig 2012.02.13 EutrendFTP
#define CC_TICP                 965 //高雄典範.                                 //pig 2012.01.06 典範
#define CC_THEIL                966 //同欣電子
#define CC_TERAPOWER            967 //晶兆成
#define CC_RICHTEK              968 //立錡                                      //Alick 20161206 add 新增 客戶碼
#define CC_TSI                  969 //誠遠科技

#define CC_GIGAS                970 //全智
#define CC_AMKOR_Korea          971 //安靠韓國.
#define CC_AMKOR_China          972 //安靠上海.
#define CC_AMKOR_Japan          973 //安靠日本.
#define CC_AMKOR_Philippines    974 //安靠菲律賓
#define CC_UTAC_TH              975 //聯測科技 泰國
#define CC_AnalogDevice_Phil    976 //Analog菲律賓
#define CC_ONSEMI_CA            977 // ONSEMI_Canada
#define CC_INTEL_US             978 //INTEL US
#define CC_INTEL_M              979 // INTEL Malaysia

#define CC_CARSEM_M             980 // Carsem Malaysia
#define CC_UNISEM_M             981 // Unisem Malaysia
#define CC_AMD_M                982 // AMD Malaysia                             //pig 2016.07.21 ModifyAMD
#define CC_INARI_M              983 // INARI Malaysia                           // 2016.10.12 , Joye , ADD Customer Code
#define CC_ONSEMI_M             984 // INARI Malaysia                           // 2016.10.12 , Joye , ADD Customer Code
#define CC_MAXIM                985
#define CC_MARVELL              986
#define CC_ATMEL                987
#define CC_USI                  988 //環鴻科技
#define CC_DYNACARD             989 //新東亞微電子

#define CC_CYPRESS              990
#define CC_GERADTECH_CHINA      991 // 智瑞達
#define CC_GM_TEST              992
#define CC_I_TECH               993
#define CC_WIN_PAC              994
#define CC_SILICON_LABS_SG      995
#define CC_SILICON_LABS_SZ      996
#define CC_Altera_USA           997
#define CC_ETRENDTECH           998 //逸昌科技                                  //KaiHuang 20201214 Add
#define CC_QUALCOMM             999 //高通
#define CC_HTML_Monitor         99999 //HTML_Monitor

//==============================================================================

//=============  Parameter DEFINE ==============================================
#define MAX_SAFE_DOOR_CNT 30
#define MAX_HATCH_DOOR_CNT 12
#define MAX_OutArm3_Row  1
#define MAX_OutArm3_Col  2
#define MAX_ARM_Row      2
#define MAX_ARM_Col      4
#define MAX_Index_Row    2
#define MAX_Index_Col    8
#define MAX_SOCKET_ROW   4
#define MAX_SOCKET_COL   8
#define MAX_SOCKET_TOTAL 32
#define MAX_SENSOR_ITEM 820                                                     //Sam 20190112 LM //Steven 20230908 : 650 --> 710
#define MAX_TTL_BIT 40                                                          //Steven 20161011 : TTL支援8Site
#define MAX_IONFAN 12                                                           //2013-08-28    Dell
#define MAX_IONBAR 9                                                            //Steven 20250610 : 修正記憶體溢位
#define MAX_HTIONFAN 3                                                          //RogerYang 20250825 : Unloader新增3支IonBar，取代4 5 8 ion fan

//#define MAX_Unloader  6
#define MAX_AUTO_TRAY   6                                                       //出料軌道數量
#define MAX_TRACK       9                                                       //入出料軌道數量
#define MAX_FIX_TRAY    6                                                       //FIX盤數量
#define MAX_UNLOAD_TRAY 12                                                      //出料盤數量
#define MAX_MGZ_TRAY    14                                                      //Magazine Tray盤數量
#define RecordTrayCnt   20                                                      //要跟舊版本相容, 不可以亂換順序
const int PITCH_COUNT=5;
const int X_PITCH_COUNT=4;

#define FIRST_HOME    100
#define SECOND_HOME   200
#define THREE_HOME    300
//=============  Parameter const ==============================================
const int TEST_MAX_BIN=256;                                                     //kevin 20140317   100;
//const int MAX_TrayCount=9;
//#define MAX_BinCount  15

//=============  Parameter ENUM ================================================
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Type_HT9046LS
//Index 雙Arm同時壓32 Site (shuttle 2*8)
//Index 單Arm可壓600kg
//Shuttle Pitch 40mm
//加熱 150度
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
enum eMachineType{Type_HT9045       =100,
                  Type_HT9046       =200,
                  Type_HT9046_LS    =300,
                  Type_HT9045_12Site=400,                                       //ChungHung 20130507 add HT9045 updata for 12site 517
                  Type_HT502        =500,
                  Type_HT1032       =600,
                  Type_HT7080       =700,
                  //AI(W906-GL-5j) 20260831: Type_HT9050 added on user ruling ("用 800").
                  //
                  // ⚠ THE VALUE IS 800, **NOT** 500.  D:\HT9045\CLAUDE.md's "HT9050 是什麼"
                  // section says `Type_HT9050 = 500`, and that is WRONG: 500 is already
                  // Type_HT502 (two lines up), 600 is Type_HT1032, 700 is Type_HT7080.
                  // Taking CLAUDE.md literally would have silently aliased HT9050 onto
                  // HT502 -- every `MachineTypeChoice==Type_HT502` comparison in the tree
                  // would then also match HT9050.  800 is the next free slot on the
                  // established 100-interval pattern.  Measured, not assumed.
                  //
                  // ⚠ ADDING THE VALUE CHANGES NO BEHAVIOUR, AND THAT IS DELIBERATE.  [AI(W906-HT9050-FAM) 20260925: STALE since 20260925 -- user ruled HT9050 is HT9046 family (RULINGS_20260925 #5); bd90b948 + W906-HT9050-FAM make 12 dispatch sites test Type_HT9050 -- `python tools/nb2_assist/machine_family_dispatch.py` lists them.]
                  // Nothing selects it yet: MachineTypeChoice is loaded from the machine
                  // config, and no dispatch site tests for Type_HT9050.  Measured 20260831:
                  // MachineTypeChoice appears 220 times across the tree (cinitial.cpp 100,
                  // database.cpp 19, MyLaneIo.cpp 12, acarry.cpp 12, csystem.cpp 12,
                  // cObserver.cpp 9, cSetUp.cpp 9, Command.cpp 7, cConfiguration.cpp 6,
                  // ainarm2.cpp 5, ...).  Deciding what each of those does for HT9050 is a
                  // campaign, not a line -- so this commit adds the identity ONLY and the
                  // dispatch work is queued.  A value that quietly picked up another type's
                  // branches would be worse than no value at all.
                  //
                  // TARGET CONFIG (docs/HAL_MOTTABLE_CLASSIFICATION.md, user 20260824):
                  // Model = HT9050, motors AND IO both via PCIE-1203.  That doc's §CardModel
                  // says classification is driven by Mot_Table.csv's CardModel column, where
                  // `PCI1203` selects TMyEtherCatMotor -- so the motor route is a TABLE
                  // decision, not an enum decision.  Do not duplicate it here.
                  Type_HT9050       =800};

enum eSubMachineType{Type_None      =0,
                     Type_HT9046LA  =1,                                         //Steven 20190307 : Add for HT-9046LA
                     Type_HT9016C   =2,                                         //Jimmychiu 20221123 : Add for HT9016C
                     Type_HT9046AU  =3,                                         //Steven 20240822 : For HT-9046AU
                     Type_HT9046CR  =4};                                        //Steven 20241001 : For HT-9046CR

enum eTestMode{SingleSite   =0,
               DualSite     =1,
//               DualSiteBS   =2,
               TriSite1X3   =2,                                                 //Frank 20160104 add for TSMC 1x3 Site //wei 20160128 add for 1x3_4
               QualSite1X4  =3,
               DualSite2x1  =4,
               QualSite2X2  =5,
               QualSite2X2N =6,                                                 //Frank 20200520 2X2NN Mode
//               QualSite2X2BS=8,
               _6Site2X3    =7,                                                 //ChungHung 20140115 add for 2x3
               _6Site2X3N   =8,                                                 //Steven 20220425 : 2X3NN Mode
//               _8Site1x8    =8,
               _8Site2X4    =9,
               _8Site2X4N   =10,                                                //Wei 20231211 : 2X4NN Mode
               _10Site2X5   =11,                                                //wei 20190614 10 site
               _12Site2X6   =12,
               _16Site2X8   =13,
               _16Site4X4   =14,                                                //Sam 20190226 : 16Site4X4
               _32Site4X8N  =15,                                                //雙Arm 32 Site
               _32Site4X8M  =16,                                                //單Arm 32 Site
               _8Site1X4    =17,                                                //ChungHung 20150528 add for 海思 _8Site1x4
               TotalTestMode                                                    //擺最下面,不可變動   //因為有SECS GEM, 後續的新模式能往後加
               };

enum eIndexPressType
    {   e85KG   =0,
        e240KG  =1,
        e120KG  =2,
        e500KG  =3,
        e260KG  =4,
        e400KG  =5,                                                             //Steven 20131007 : Index 1.5KW, 400KG
        e360KG  =6,                                                             //Steven 20140312 : For HT9045WA 360KG
        e160KG  =7,                                                             //ifor 20170215 (wei) add 160KG
        e640KG  =8,                                                             //Sam 20210628 : add 640KG
        e800KG  =9                                                              //Ifor 20230830 : add 800KG
    };

enum eRotateInPos{eripHotPlate1=0,
                  eripHotPlate2=1
                 };
enum eRotateOutPos{eropAuto1=0,
                   eropAuto2=1,
                   eropAuto3=2,
                  };

enum eRotateType{eCynRotate=0,
                 e1MotRotate=1,
                 e4MotRotate=2,
                 e8MotRotate=3,
                 e1MotRotate1Dut=4,
                 e2MotRotate2Dut=5,
                 eInOutArm1Motor=6,
                 eRotateTypeTotal
                };

enum eUseRotate{eNonRot=0,
                eCynRot=1,
                eMotRot=2,
                };

enum eCoverTray{eEmpty=0,
                eColor=1,
                eNoUse=2,
                };

enum e1DCoverTray{eNoUse1DCoverTray=0,
                  eUse1DCoverTray=1,
                  };

enum eRs232Mode{eRs232Standard  =0,                                             //Steven 20121112 : RS232支援32Bin
                eRs23232Bin     =1
               };

enum eOCRType{eocrUninstal=0, eocrUseMessage=1, eocrUseRS23=2};                 //Steven 20120716 : OCR
enum eOCRYMotType{eocrYMotUninstal=0, eocrYMotInstal=1};                        //Frank 20250214 add
enum eColorSensorMUN_Type{eCSMUN_Uninstall=0, eCSMUN_Install_Loader=1};         //Jimmychiu 20230630 : add color sensor MU-N in Loader
enum eBarCodeType{ebctUninstall=0,                                              //Steven 20120921 : Barcode_2D
                  ebctOutShtAMD=1,
                  ebctInShtIntel=2,
                  ebctUseCCDMode=3,
                  ebctEtherNetCCD=4,                                            //Ifor 20190129 : add Cognex EtherNet
                  ebcUseOCR=5                                                   //Ifor 20210407 add: 自製OCR
                 };
//enum eBarCodeMode{ebcmAMDType=0, ebcmIntelType=1};                            //Steven 20120921 : Barcode_2D
enum eContactMode{DirectContactMode             =0,
                  DropContact                   =1,
                  DirectContactModeDiffentSpeed =2,
                  TMove                         =3,
                  TMoveDrop                     =4,
                  DirectContactSoftEP           =5,
                  DropContactSoftEP             =6,                             //kevin 20130608 ADD DirectContactSoftEP & DropContactSoftEP馬達到測試高度ep再充氣
                  DropPlaceShiftContact         =7,                             //ChungHung 20150528 add for 海思 _8Site1x4
                  DropContactModeDiffentSpeed   =8,                             //Steven 20160512 : DropContactModeDiffentSpeed
                  TMoveSlowContact              =9,                             //Steven 20160130 : TMove Soft contact
                  TMoveDropSlowContact          =10,                            //Steven 20160512 : TMoveDropSlowContact
                  ContactModeTotal};

enum eVacummMode{VacuumONMode=0, VacuumOFFMode=1};
enum eIndexHeatMode{HeadOnly=0, ChamberOnly=1, HeadChamber=2, SocketChamber=3, HeadSocket=4, HeadChamberSocket=5}; //kevin 20131209 add HeadChamberSocket
enum eClearType{ctLoadingCounts=0,
                ctTesterCategory=1,
                ctContactCounts=2,
                ctTraySortCount=3,
                ctScannerCategory=4,
                ctAlarmData=5,
                ctTimeData=6,
                ctContactCountsHis=7,
                ctBinCount=8,
                ctIndexCount=9,
                ctAutoRetestCount=10,                                           //kevin 20150530 add AutoRETEST 只清除搬 auto tray數量及load total數量
                ctFailBinCount=11};

enum eSystemTime{stStartTime    =0,
                 stPauseTime    =1,
                 stPowerOn      =2,
                 stProductTime  =3,
                 stJamTime      =4,
                 stSystemNGTime =5,
                 stContactTest  =6,
                 stHomeTime     =7};                                            //最多就八個

enum eTimeData{tdPowerOn        =0,                                             //Steven 20231120 : 紀錄機台稼動時間
               tdPowerOff       =1,
               tdPauseTime      =2,
               tdStartTime      =3,
               tdContactTest    =4,
               tdHomeTime       =5,
               tdSoakTime       =6,
               tdSOT            =7,
               tdEOT            =8,
               tdLotStart       =9,
               tdLotEnd         =10,
               tdNoteOn         =11,
               tdNoteOff        =12,
               tdMessageOn      =13,
               tdMessageOff     =14,
               tdInitialStart   =15,
               tdTrayFeed       =16,
               tdTotal};

enum eTrayType{
    tNotUse  =0,
    tTrayAuto=1,
    tTrayFix =2,
    tTrayMag =3,
    tTrayBox =4,
};

enum eCoverID{
    tCIDNotUse      =0,
    tCID_Keyence    =1,
    tCID_NFC        =2
};

enum eTempType{
    tTemp130 =0,
    tTemp150 =1,
    tTemp155 =2,
    tTemp175 =3,
    tTemp200 =4,
};

enum eRunStartMode{rsmNull              =-1,
                   rsmContinuStart      =0,
                   rsmInitialStart      =1,
                   rsmContinuRetest     =2,
                   rsmCInitialRetest    =3,
                   rsmAutoSiteMap       =4,
                   rsmQAMode            =5,
                   rsmContinuEQC        =6,
                   rsmInitialEQC        =7,
                   rsmInitial_ART       =8,                                     //ChungHung 20141002 add for KYEC AutoRetest
                   rsmContinuStart_ART  =9,                                     //ChungHung 20141002 add for KYEC AutoRetest
                   rsmContinuRetest_ART =10,                                    //ChungHung 20141002 add for KYEC AutoRetest
                   rsmAutoRetest        =11,                                    //Steven 20140331 : Auto Retest
                   rsmFIFOMode          =12,                                    //Steven 20160303 : FIFO Mode
                   rsmInitial_MRT       =13,                                    //Ifor 20170316 (wei) add KYEC MRT Mode
                   rsmContinuStart_MRT  =14,                                    //Ifor 20170316 (wei) add KYEC MRT Mode
                   rsmRetest_MRT        =15,                                    //Ifor 20170316 (wei) add KYEC MRT Mode
                   rsmRunModeTotal};                                            //擺最下面,不可變動

enum eSocketTempControll{eDut1ea=0,
                         eDut4ea=1,
                         eDut2ea=2};

//---------------------------------------------------------------------------
enum eBinType                                                                   //決定要幾種不一樣的Bin設定
{   eBinRT          =0,                                                         //RT使用的Bin
    eBinFT          =1,                                                         //FT使用的Bin
    eBinOffLine     =2,                                                         //Off-Line使用的Bin
    eBinRT_ART      =3,                                                         //ChungHung 20141002 add for KYEC AutoRetest
    eBinFT_ART      =4,                                                         //ChungHung 20141002 add for KYEC AutoRetest
    eBinRT_MRT      =5,                                                         //Ifor 20170316 (wei) add KYEC MRT Mode
    eBinFT_MRT      =6,                                                         //Ifor 20170316 (wei) add KYEC MRT Mode
    eBinTypeTotal
};

enum eTempControll{tcHotPlate1=0, tcHotPlate2=1, tcShuttle1=2, tcShuttle2=3,
                   tcHead1=4, tcHead2=5, tcHead3=6, tcHead4=7, tcSocket=8, tcChamber=9, tcCCD=10,
                   tcAa1=11, tcAb1=12, tcAc1=13, tcAd1=14, tcBa1=15, tcBb1=16, tcBc1=17, tcBd1=18,
                   tcAa2=19, tcAb2=20, tcAc2=21, tcAd2=22, tcBa2=23, tcBb2=24, tcBc2=25, tcBd2=26,
                   tcHeatGun1=27, tcHeatGun2=28,                                                        //kevin 20120523 增加熱風槍
                   tcDUT1=29, tcDUT2=30, tcDUT3=31, tcDUT4=32,                                          //2013-01-15    Dell DUT(Socket Base)增加為4顆 ,only for 9046LS
                   tcAe1=33, tcAf1=34, tcAg1=35, tcAh1=36, tcBe1=37, tcBf1=38, tcBg1=39, tcBh1=40,
                   tcAe2=41, tcAf2=42, tcAg2=43, tcAh2=44, tcBe2=45, tcBf2=46, tcBg2=47, tcBh2=48,      //Steven 20140923 : Index使用EJ1N版32組加熱器
                   tc2D=49,                                                                             //wei 20160524 2D溫度
                   tcLB=50, tcIndexESD=51,                                                              //kevin 20200207 add Index ESD hot air
                   tcCCD_2=52,                                                                          //Isaac 20201217 : RTC CCD增加第二組感溫
                   tcATCHotAir1=53, tcATCHotAir2=54,
                   tcOutSht1=55, tcOutSht2=56,
                   tcBase1=57,tcBase2=58,tcBase3=59,tcBase4=60,tcBase5=61,tcBase6=62,                   //Ztex 2023.04.19 Add HT-1032 TriTemp Function
                   tcHotPlate3=63, tcHotPlate4=64, tcShuttle3=65, tcShuttle4=66,                        //Ztex 2023.04.19 Add HT-1032 TriTemp Function
                   tcDoor1=67, tcDoor2=68,                                                              //Ztex 2023.10.23 Add Index Door Heater
                   tcLBUp=69, tcLBDown=70,                                                              //Frank 20241231 : add
                   tcTotalCount};                                               //有增加請搜尋: 溫控器要一起改  //超過100的話, Alarm Code要重新處理

enum eHeaterType{eht4Heater=0,                                                  //Index使用4組加熱器
                 eht16Heater=1,                                                 //Index使用KT4H版16組加熱器
                 eht16HeaterEJ1N=2,                                             //Index使用EJ1N版16組加熱器
                 eht32HeaterEJ1N=3,                                             //Index使用EJ1N版32組加熱器
                 eht32HeaterKT4H=4,                                             //Index使用KY4H版32組加熱器
                 eht16HeaterDTME08=5,                                           //Index使用DTME08版16組加熱器
                 eht32HeaterDTME08=6};                                          //Index使用DTME08版32組加熱器

enum eMyTempColor{mtcBelow=0, mtcNormal, mtcOver};
enum eSuckerStatus{SStatusOff=0, SStatusSuck=1, SStatusDestory=2};
enum ePiggyBackTrugger{pbtNone=0, pbtContinualPassBin=1, pbtContinualPassBinBySocket=2, pbtContinualLoader=3, pbtContinualContactCount=4,
                       pbtHome=5, pbtOneCycle=6, pbtCleanout=7, pbtReset=8, pbtOutShuttleLose=9, pbtIndexArmVacOffErr=10, pbtIndexArmPickUpErr=11,  //Steven 20111207 : 誰觸發了Piggy Back   //JerryYang 20160816 add pbtIndexArmVacOffErr
                       pbtRTCAlarm=12, pbtOneCycleRemainSht=13};
enum eUserLanguage{eulEnglish=0,                                                //純英文版
                   eulChinese=1,                                                //繁體中文
                   eulChina=2,                                                  //簡體中文
                   eulKorea=3,                                                  //韓文版
                   eulSingapore=4,                                              //新加坡版
                   eulJapan};                                                   //日文版

enum eIonFanType{e5IOforAll         =0,
                 e1IOforOne         =1,
                 eUnInstallIonFan   =2,
                 e2IoforOne         =3,
                 eIonFanTypeTotal};                                             //擺最下面,不可變動

enum eSpeedPart{InArm       =0,
                IndexArm    =1,
                OutArm      =2,
                TrayArm     =3,
                InRotate    =4,                                                 //Steven 20140428 : For Rotate
                OutRotate   =5,                                                 //Steven 20140428 : For Rotate
                SpeedPartTotal};

enum eATCType{eATCUninstall=0,
              eATC60=1,                                                         //2014-05-30    Dell    for ATC6.0
              eATC30=2,                                                         //20141204 ChungHung add for ATC3.0
              eATCSiliconType=3,
              eATCHonPrecType=4,                                                //Steven 20120410 : Hontech ATC
              eNonChamber=5,
              eNewATCSystem=6,                                                  //Ifor 20151230 :add New ATC Interface
              eWinWay=7,                                                        //Jimmychiu 20210906
              eMikros=8,                                                        //Ifor 20231027 add: Mikros ATC System
              };

enum eATCMixMode{eATCNonMix=0,                                                  //JerryYang 20250728 : add
                 eMixATC60_ATC33=1,
              };

enum eShuttleSensorType{eSensorTakex=0,
                        eSensorLine=1,
                        eSensorCCLink=2,
                        eSensorCCLink3=3,
                        eSensorCanBus=4,
                        eSensorCanBus3=5,
                        eSensorEtherCAT=6,                                      //Sam 20230707 : EtherCAT Shuttle sensor
                        eSensorEtherCAT3=7};

enum eNUECType{eNUEC1=0,                                                        //Sam 20230707 : EtherCAT Shuttle sensor
               eNUEC1A=1};

enum eIOType{eMotionNet=0,
             eISABase  =1,
             ePCI1735U =2,
             ePCI1203  =3,
             ePLCbase  =4
             };

enum eAutoCleanTray {eAutoCleanFix3=0, eAutoCleanHot2, eAutoCleanKit, eAutoCleanAir};

enum ePicker{ePickerA=0,
             ePickerB=1,
             ePickerC=2,
             ePickerD=3,
             ePickerE=4,
             ePickerF=5,
             ePickerG=6,
             ePickerH=7,
             ePickerTotal
            };

enum eSECSOffset{
    OfsLoader=0,            //0
    OfsHP1,                 //1
    OfsHP2,                 //2
    OfsInSh1,               //3
    OfsInSh2,               //4
    OfsOutSh1,              //5
    OfsOutSh2,              //6
    OfsAuto1,               //7
    OfsAuto2,               //8
    OfsAuto3,               //9
    OfsAuto4,               //10 //Steven 20230907 : For HT-9011UC
    OfsAuto5,               //11
    OfsAuto6,               //12
    OfsFix1,                //13
    OfsFix2,                //14
    OfsFix3,                //15
    OfsFix4,                //16 //Steven 20230907 : For HT-9011UC
    OfsFix5,                //17
    OfsFix6,                //18
    OfsAutoClean,           //19
    OfsOCR,                 //20
    OfsRotate_In,           //21
    OfsRotate_Out,          //22
    OfsTopView,             //23 //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
    OfsPADView,             //24 //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
    OfsBGAView,             //25 //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
    OfsLoaderB,             //26 //Steven 20140811 : For 32 Site
    OfsInSh1LB,             //27 //Steven 20140811 : For 32 Site
    OfsInSh1RA,             //28 //Steven 20140811 : For 32 Site
    OfsInSh1RB,             //29 //Steven 20140811 : For 32 Site
    OfsInSh2LB,             //30 //Steven 20140811 : For 32 Site
    OfsInSh2RA,             //31 //Steven 20140811 : For 32 Site
    OfsInSh2RB,             //32 //Steven 20140811 : For 32 Site
    OfsInSh1_AutoClean,     //33 //20140923 wei : For Shuttle Auto Clean
    OfsInSh1LB_AutoClean,   //34 //20140923 wei : For Shuttle Auto Clean
    OfsInSh1RA_AutoClean,   //35 //20140923 wei : For Shuttle Auto Clean
    OfsInSh1RB_AutoClean,   //36 //20140923 wei : For Shuttle Auto Clean
    OfsInSh2_AutoClean,     //37 //20140923 wei : For Shuttle Auto Clean
    OfsInSh2LB_AutoClean,   //38 //20140923 wei : For Shuttle Auto Clean
    OfsInSh2RA_AutoClean,   //39 //20140923 wei : For Shuttle Auto Clean
    OfsInSh2RB_AutoClean,   //40 //20140923 wei : For Shuttle Auto Clean
    OfsAutoSh1,             //41 //wei 20160914 Auto Shuttle Sensor
    OfsAutoSh2,             //42 //wei 20160914 Auto Shuttle Sensor
    OfsPreciser,            //43 //Frank 20180410 (Steven) : InArm Preciser Station
    OfsOutSh1LB,            //44 //Steven 20190802 : For 32 Site Out Shuttle Offset
    OfsOutSh1RA,            //45 //Steven 20190802 : For 32 Site Out Shuttle Offset
    OfsOutSh1RB,            //46 //Steven 20190802 : For 32 Site Out Shuttle Offset
    OfsOutSh2LB,            //47 //Steven 20190802 : For 32 Site Out Shuttle Offset
    OfsOutSh2RA,            //48 //Steven 20190802 : For 32 Site Out Shuttle Offset
    OfsOutSh2RB,            //49 //Steven 20190802 : For 32 Site Out Shuttle Offset
    OfsScanAOI,             //50 //Ifor 20191229 : add
    OfsInPlacement,         //51 //JimmyChiu 20220908 add Pickup Error Placement
    OfsBottom2DID,          //52 //KaiChen 20200414 ：新增Bottom2DID OffSet
    OfsTotal
};

enum eInOffset{
    InOfsLoader=0,            //0
    InOfsHP1,                 //1
    InOfsHP2,                 //2
    InOfsInSh1,               //3
    InOfsInSh2,               //4
    InOfsAutoClean,           //5
    InOfsOCR,                 //6
    InOfsRotate_In,           //7
    InOfsLoaderB,             //8  //Steven 20140811 : For 32 Site
    InOfsInSh1LB,             //9  //Steven 20140811 : For 32 Site
    InOfsInSh1RA,             //10 //Steven 20140811 : For 32 Site
    InOfsInSh1RB,             //11 //Steven 20140811 : For 32 Site
    InOfsInSh2LB,             //12 //Steven 20140811 : For 32 Site
    InOfsInSh2RA,             //13 //Steven 20140811 : For 32 Site
    InOfsInSh2RB,             //14 //Steven 20140811 : For 32 Site
    InOfsInSh1_AutoClean,     //15 //20140923 wei : For Shuttle Auto Clean
    InOfsInSh1LB_AutoClean,   //16 //20140923 wei : For Shuttle Auto Clean
    InOfsInSh1RA_AutoClean,   //17 //20140923 wei : For Shuttle Auto Clean
    InOfsInSh1RB_AutoClean,   //18 //20140923 wei : For Shuttle Auto Clean
    InOfsInSh2_AutoClean,     //19 //20140923 wei : For Shuttle Auto Clean
    InOfsInSh2LB_AutoClean,   //20 //20140923 wei : For Shuttle Auto Clean
    InOfsInSh2RA_AutoClean,   //21 //20140923 wei : For Shuttle Auto Clean
    InOfsInSh2RB_AutoClean,   //22 //20140923 wei : For Shuttle Auto Clean
    InOfsAutoSh1,             //23 //wei 20160914 Auto Shuttle Sensor
    InOfsAutoSh2,             //24 //wei 20160914 Auto Shuttle Sensor
    InOfsPreciser,            //25 //Frank 20180410 (Steven) : InArm Preciser Station
    InOfsPlacement,           //26 //JimmyChiu 20220908 add Pickup Error Placement
    InOfsBottom2DID,          //27 //KaiChen 20200414 ：新增Bottom2DID OffSet
    InOfsTotal
};

enum eOutOffset{
    OutOfsOutSh1=0,         //0
    OutOfsOutSh2,           //1
    OutOfsAuto1,            //2
    OutOfsAuto2,            //3
    OutOfsAuto3,            //4
    OutOfsAuto4,            //5  //Steven 20230907 : For HT-9011UC
    OutOfsAuto5,            //6
    OutOfsAuto6,            //7
    OutOfsFix1,             //8
    OutOfsFix2,             //9
    OutOfsFix3,             //10
    OutOfsFix4,             //11 //Steven 20230907 : For HT-9011UC
    OutOfsFix5,             //12
    OutOfsFix6,             //13
    OutOfsRotate_Out,       //14
    OutOfsTopView_Out,      //15
    OutOfsPADView_Out,      //16
    OutOfsBGA_Out,          //17
    OutOfsOutSh1LB,         //18 //Steven 20190802 : For 32 Site Out Shuttle Offset
    OutOfsOutSh1RA,         //19 //Steven 20190802 : For 32 Site Out Shuttle Offset
    OutOfsOutSh1RB,         //20 //Steven 20190802 : For 32 Site Out Shuttle Offset
    OutOfsOutSh2LB,         //21 //Steven 20190802 : For 32 Site Out Shuttle Offset
    OutOfsOutSh2RA,         //22 //Steven 20190802 : For 32 Site Out Shuttle Offset
    OutOfsOutSh2RB,         //23 //Steven 20190802 : For 32 Site Out Shuttle Offset
    OutOfsScannerAOI,       //24 //Ifor 20191211 : add Scanner AOI
    OutOfsSortSht,          //25 //RogerYang 20250417 for HT9046AU add
    OutOfsTotal
};

enum eSortOffset{            //RogerYang 20250417 for HT9046AU add
    SortOfsSht=0,            //0
    SortOfsAuto4,            //1
    SortOfsAuto5,            //2
    SortOfsAuto6,            //3
    SortOfsTotal
};

enum eOffsetSecs
{
    ofsArmX     =0,
    ofsArmY     =1,
    ofsArmPitch1=2,
    ofsArmPick  =3,
    ofsArmPlace =4,
    ofsArmPitchY=5,
    ofsArmPitch2=6,
    ofsArmPitch3=7,
    ofsArmPitch4=8,
    ofsArmTotal
};

enum eOffsetItem
{
    oiArmX     =1,
    oiArmY     =2,
    oiArmPitch1=3,
    oiArmPitch2=4,
    oiArmPitch3=5,
    oiArmPitch4=6,
    oiArmPitchY=7,
    oiArmPick  =8,
    oiArmPlace =9,
    oiArmPickA_X =10,                                                           //Eastsun 20251218 : 每支吸嘴獨立偏移
    oiArmPickA_Y =11,
    oiArmPickB_X =12,
    oiArmPickB_Y =13,
    oiArmPickC_X =14,
    oiArmPickC_Y =15,
    oiArmPickD_X =16,
    oiArmPickD_Y =17,
    oiArmPickE_X =18,
    oiArmPickE_Y =19,
    oiArmPickF_X =20,
    oiArmPickF_Y =21,
    oiArmPickG_X =22,
    oiArmPickG_Y =23,
    oiArmPickH_X =24,
    oiArmPickH_Y =25,
    oiArmTotal
};

enum eAutoRetest{eartUninstall=0, eartInstall=1};

enum eTrayMap{etmUninstall=0, etmInstall=1, etmDeviceRemain=2};

enum eLaserStatus{  lsNormal    =0,                                             //Steven 20140228 : 雷射測距功能
                    lsChecking  =1,
                    lsNeedCheck =2,
                    lsFail      =3};

enum eBarcodeReadType{  bcUserName      =0,
                        bcTrayForm      =1,
                        bcPlateForm     =2,
                        bcTrayAssign    =3,
                        bcTemperature   =4,
                        bcContact       =5,
                        bcTesterIF      =6,
                        bcBin           =7,
                        bcSetup         =8,
                        bcYield         =9,
                        bcAutoClean     =10,
                        bcBuilder       =11,
                        bcOffset        =12,
                        bcSetupFile     =13,
                        bcExit          =14,
                        bcSECSGEM       =15,
                        bcTotal                                                 //擺在最下面
                     };

enum eEndCode{ec00=0, ec0F, ec10, ec11, ec12, ec13, ec14, ec16, ec18, ecTotal};

enum eCleanKitPosition{eCKPos_Fix3      =0,
                       eCKPos_HP2       =1,
                       eCKPos_CleanKit  =2,
                       eCKPos_CleanAir  =3
                       };

enum eOCROrder{ eSignIn     =0,
                eUser       =1,
                ePassWord   =2,
                eOffline    =3,
                eChangeFile =4,
                eOnline     =5,
                eMatch      =6,
                eMatchOK    =7,
                eMatchNG    =8,
                eConnect    =9,
                eDisConnect =10,
                eError      =11};

enum eOCROCount {   iOCRSearchCount =0,                                         //wei 20151117 OCR Lot check
                    iOCRErrCount    =1,
                    iOCRPassCount   =2,
                    iOCRSkipCount   =3,
                    iOCRSkipContinue=4,
                    iOCRTotalCount  =5,
                    iOCrLotErrCount =6,
                    iOCrLotPassCount=7,
                    iOCrLotSkipCount=8,
                    iOCRLotTotalCount=9};

enum eFunction{ iTray_ID                =0,
                iTray_Map               =1,
                iDeviceRemain           =2,
                iTray_ID2               =3,
                iTray_DeviceCnt1        =4,                                     //Sam 20190405 : Tray Decive Count
                iTray_DeviceCnt2        =5,                                     //Sam 20190405 : Tray Decive Count
                iTray_DeviceCnt3        =6,                                     //Sam 20190405 : Tray Decive Count
                iCoverTrayID_LoaderCar  =7,
                iCoverTrayID_Loader     =8,
                iCoverTrayID_AUTO1      =9,
                iCoverTrayID_AUTO2      =10,
                iCoverTrayID_AUTO3      =11,
//                iCoverTrayID_AUTO4      =12,
//                iCoverTrayID_AUTO5      =13,
//                iCoverTrayID_AUTO6      =14,
                iTotalFunction};

enum eKeyenceFunction{ iLoaderID                    =0,
                       iEmptyID                     =1,
                       iMultileID                   =2,
                       iAuto3ID                     =3,
                       iKeyenceCoverTrayID_LoaderCar=4,
                       iKeyenceCoverTrayID_Loader   =5,
                       iKeyenceCoverTrayID_AUTO1    =6,
                       iKeyenceCoverTrayID_AUTO2    =7,
                       iKeyenceCoverTrayID_AUTO3    =8,
                       iKeyenceCoverTrayID_AUTO4    =9,
                       iKeyenceCoverTrayID_AUTO5    =10,
                       iKeyenceCoverTrayID_AUTO6    =11,
                       iKeyenceCoverTrayID_Color    =12,
                       iKeyenceTotalFunction};

enum eMRSpeedPart{CaselevatorZ  =0,
                  CasArmX       =1,
                  CasArmZ       =2,
                  TrayBracketZ  =3,
                  StackedTrayX  =4,                                             //Steven 20140428 : For Rotate
                  StackedTrayZ  =5,                                             //Steven 20140428 : For Rotate
                  LoadRobotZ    =6,                                             //Sam 20190112 LM
                  UnloadRobotZ  =7,                                             //Sam 20190112 LM
                  MRSpeedPartTotal};

enum eFixAOI{eFix2AIBGACCD1=0,
             eFix2AIBGACCD2=1,
             eTotalFixAOI};                                                     //Sam 20210609 : Fix AOI 軟體升級

enum eInArmType {   e9045_1x1_1     =0,                                         //Ifor 20180518 : add 整合Hot plate 資料轉換
                    e9045_1x4_1_Ac  =1,
                    e9045_1x2_2_13  =2,
                    e9045_1x2_2_14  =3,
                    e9045_1x2_4_Hot =4,
                    e9045_1x3_2_14  =5,
                    e9045_1x3_4     =6,
                    e9045_1x4_2_14  =7,
                    e9045_1x4_4     =8,
                    e9045_1x4_4_13  =9,
                    e9045_1x4_4_Back=10,
                    e9045_1x4_8_Hot =11,
                    e9045_2x1_2_13  =12,
                    e9045_2x2_2_14  =13,
                    e9045_2x2_4_12  =14,
                    e9045_2x2_4_13  =15,
                    e9045_2x2_4_14  =16,
                    e9045_2x2_8_Hot =17,
                    e9045_2x3_6     =18,
                    e9045_2x3_6_14  =19,
                    e9045_2x4_4_13  =21,
                    e9045_2x4_4_14  =20,
                    e9045_2x4_8     =22,
                    e9045_2x5_8     =23,
                    e9045_2x6_8     =24,
                    e9045_2x8_32    =25,
                    e9045_2x8_8     =26,
                   };

enum e2DIDFormat {  eStandard =0,                                               //JerryYang 20200422 2DID format選項改用下拉選單
                    eAMD      =1,
                    eIntel    =2};

enum eAutoCleanMode {   eNormal             =0,
                        eInitialStart       =1,
                        eInitalRetestStart  =2,
                        eInterval           =3,
                        eManual             =4,
                        eSocketAlarm        =5,
                        eFinish             =6,
                        eSecsGem            =7,
                        eACByTime           =8};

enum eFTPAutomation_CheckList{  eCL_Temperature     =0,                         //KaiChen 20190530 ：Sigurd FTP Automation
                                eCL_Alarm           =1,
                                eCL_FT_Yield        =2,
                                eCL_SiteMapping     =3,
                                eCL_Speed           =4,
                                eCL_Contact         =5,
                                eCL_Category        =6,
                                eCL_BinSetting      =7,                         //KaiChen 20200527 ：矽興要求增加
                                eCL_TrayForm        =8,                         //KaiChen 20200527 ：矽興要求增加
                                eCL_HotPlate        =9,                         //KaiChen 20200527 ：矽興要求增加
                                eCL_Total           =10};

//Steven 20200825 : eTrayCount定義換位置
enum e6TrayName                                                                 //使用的Harware Bin數量與名稱
{   eAuto1          =0,
    eAuto2          =1,
    eAuto3          =2,
    eAuto4          =3,
    eAuto5          =4,
    eAuto6          =5,
    eFix1           =6,
    eFix2           =7,
    eFix3           =8,
    eFix4           =9,
    eFix5           =10,
    eFix6           =11,
    eFix7           =12,
    eFix8           =13,
    eFix9           =14,
    eFix10          =15,
    eFix11          =16,
    eFix12          =17,
    eBulkBox        =18,                                                        //kevin 20160819 add bulk box
    eMag1           =19,
    eMag2           =20,
    eMag3           =21,
    eMag4           =22,
    eMag5           =23,
    eMag6           =24,
    eMag7           =25,
    eMag8           =26,
    eMag9           =27,
    eMag10          =28,
    eMag11          =29,
    eMag12          =30,
    eMag13          =31,
    eMag14          =32,                                                        //JerryYang 20220909 : add magazine
    eTrayCount
};

enum e3TrayName                                                                 //使用的Harware Bin數量與名稱
{   e3Auto1          =0,
    e3Auto2          =1,
    e3Auto3          =2,
    e3Fix1           =3,
    e3Fix2           =4,
    e3Fix3           =5,
    e3Fix4           =6,
    e3Fix5           =7,
    e3Fix6           =8,
    e3BulkBox        =9,                                                        //kevin 20160819 add bulk box
    e3Mag1           =10,
    e3Mag2           =11,
    e3Mag3           =12,
    e3Mag4           =13,
    e3Mag5           =14,
    e3Mag6           =15,
    e3Mag7           =16,
    e3Mag8           =17,
    e3Mag9           =18,
    e3Mag10          =19,
    e3Mag11          =20,
    e3Mag12          =21,
    e3Mag13          =22,
    e3Mag14          =23,                                                       //JerryYang 20220909 : add magazine
    e3Auto4          =24,
    e3Auto5          =25,
    e3Auto6          =26,
    e3Fix7           =27,
    e3Fix8           =28,
    e3Fix9           =29,
    e3Fix10          =30,
    e3Fix11          =31,
    e3Fix12          =32,
    e3TrayCount
};

enum eBinPositionName                                                           //Bin要放在Hardware上的位置
{   ePosNoUse       =0,
    ePosAuto1       =1,
    ePosAuto2       =2,
    ePosAuto3       =3,
    ePosAuto4       =4,
    ePosAuto5       =5,
    ePosAuto6       =6,
    ePosFix1        =7,
    ePosFix2        =8,
    ePosFix3        =9,
    ePosFix4        =10,
    ePosFix5        =11,
    ePosFix6        =12,
    ePosFix7        =13,
    ePosFix8        =14,
    ePosFix9        =15,
    ePosFix10       =16,
    ePosFix11       =17,
    ePosFix12       =18,
    eBulkbox        =19,
    ePosMag1        =20,
    ePosMag2        =21,
    ePosMag3        =22,
    ePosMag4        =23,
    ePosMag5        =24,
    ePosMag6        =25,
    ePosMag7        =26,
    ePosMag8        =27,
    ePosMag9        =28,
    ePosMag10       =29,
    ePosMag11       =30,
    ePosMag12       =31,
    ePosMag13       =32,
    ePosMag14       =33,                                                        //JerryYang 20220909 : add magazine
    ePosTrayCount
};

enum e3BinPositionName                                                          //Bin要放在Hardware上的位置
{   e3PosNoUse       =0,
    e3PosAuto1       =1,
    e3PosAuto2       =2,
    e3PosAuto3       =3,
    e3PosFix1        =4,
    e3PosFix2        =5,
    e3PosFix3        =6,
    e3PosFix4        =7,
    e3PosFix5        =8,
    e3PosFix6        =9,
    e3Bulkbox        =10,
    e3PosMag1        =11,
    e3PosMag2        =12,
    e3PosMag3        =13,
    e3PosMag4        =14,
    e3PosMag5        =15,
    e3PosMag6        =16,
    e3PosMag7        =17,
    e3PosMag8        =18,
    e3PosMag9        =19,
    e3PosMag10       =20,
    e3PosMag11       =21,
    e3PosMag12       =22,
    e3PosMag13       =23,
    e3PosMag14       =24,                                                       //JerryYang 20220909 : add magazine
    e3PosAuto4       =25,
    e3PosAuto5       =26,
    e3PosAuto6       =27,
    e3PosFix7        =28,
    e3PosFix8        =29,
    e3PosFix9        =30,
    e3PosFix10       =31,
    e3PosFix11       =32,
    e3PosFix12       =33,
    e3PosTrayCount
};

enum eTrackName                                                                 //使用的Harware Bin數量與名稱
{
    etLoader         =0,
    etEmpty          =1,
    etColor          =2,
    etAuto1          =3,
    etAuto2          =4,
    etAuto3          =5,
    etAuto4          =6,
    etAuto5          =7,
    etAuto6          =8,
    etFix1           =9,
    etFix2           =10,
    etFix3           =11,
    etFix4           =12,
    etFix5           =13,
    etFix6           =14,
    etBulkBox        =15,
    etMag            =16,
    etTrayCount
};

enum eTrayOffset{           //Steven 20230907 : For HT-9011UC
    tOfsLoader=0,           //0
    tOfsEmpty,              //1
    tOfsColor,              //2
    tOfsAuto1,              //3
    tOfsAuto2,              //4
    tOfsAuto3,              //5
    tOfsAuto4,              //6
    tOfsAuto5,              //7
    tOfsAuto6,              //8
    tOfsIndex1,             //9
    tOfsIndex2,             //10
    trayOfsTotal
};

//JimmyChiu 20211020 : Auto alignment mode
//==>
enum eATActionItem
{   eATClose            =0,         //停止或中斷
    eATHome             =1,         //Home
    eATInShuttleSensor  =2,         //InShuttle Sensor
    eATOutShuttleSensor =3,         //OutShuttle Sensor
    eATIndexArm         =4,         //IndexArm
    eATLoader           =5,         //Pick Loader
    eATHP1              =6,         //Place HP2
    eATHP2              =7,         //Place HP1
    eATInShuttl1        =8,         //Place InShuttl1
    eATInShuttl2        =9,         //Place InShuttl2
    eATOutShuttl1       =10,        //Pick OutShuttl1
    eATAuto1            =12,        //Place Auto1
    eATAuto2            =13,        //Place Auto2
    eATAuto3            =14,        //Place Auto3
    eATAuto4            =15,        //Place Auto4
    eATAuto5            =16,        //Place Auto5
    eATAuto6            =17,        //Place Auto6
    eATFix1             =18,        //Place Fix1
    eATFix2             =19,        //Place Fix2
    eATFix3             =20,        //Place Fix3
    eATFix4             =21,        //Place Fix4
    eATFix5             =22,        //Place Fix5
    eATFix6             =23,        //Place Fix6
    eATOutShuttl2       =24,        //Pick OutShuttl1
    eATActionFinish     =25         //Finish
};
//<==
//JimmyChiu 20211020 : Auto alignment mode
//JimmyChiu 20220708 : add Index Arm Axis
//==>
enum eIndexArmAxisType
{   IndexArm_4_Axis     =0,
    IndexArm_3_Axis     =1,
};
//<==
//JimmyChiu 20220708 : add Index Arm Axis
//JimmyChiu 20220927 : Stepper Motor Control in Fix3
//==>
enum eFix3Kinematics
{   Fix3K_Uninstall       =0,
    Fix3K_ShortShuttle    =1,
    Fix3K_UseCylinder     =2,
    Fix3K_UseCylinder46LA =3,
    Fix3K_UseStepperMotor =4,
    Fix3K_Total,//always in tail
};
//<==
//JimmyChiu 20220927 : Stepper Motor Control in Fix3
enum ePickCount
{   ep4Picker       =0,
    ep8Picker       =1,
    ep2Picker       =2,
    ep16Picker      =3,
    ep1Picker       =4,
    epPickerTotal,
};

enum ePickType
{   eptUseCyn   =0,
    eptUseMot   =1,
    eptUseMotCyn=2,
};

enum eBinDispName                                                               //JerryYang 20220909 : //使用的Bin顯示器名稱
{   eBinDispLoader  =0,
    eBinDispEmpty   =1,
    eBinDispColor   =2,
    eBinDispAuto1   =3,
    eBinDispAuto2   =4,
    eBinDispAuto3   =5,
    eBinDispFix1    =6,
    eBinDispFix2    =7,
    eBinDispFix3    =8,
    eBinDispFix4    =9,
    eBinDispFix5    =10,
    eBinDispFix6    =11,
    eBinDispBulkBox =12,
    eBinDispMag1    =13,
    eBinDispMag2    =14,
    eBinDispMag3    =15,
    eBinDispMag4    =16,
    eBinDispMag5    =17,
    eBinDispMag6    =18,
    eBinDispMag7    =19,
    eBinDispMag8    =20,
    eBinDispMag9    =21,
    eBinDispMag10   =22,
    eBinDispMag11   =23,
    eBinDispMag12   =24,
    eBinDispMag13   =25,
    eBinDispMag14   =26,
    eBinDispAuto4   =27,
    eBinDispAuto5   =28,
    eBinDispAuto6   =29,
    eBinDispFix7    =30,
    eBinDispFix8    =31,
    eBinDispFix9    =32,
    eBinDispFix10   =33,
    eBinDispFix11   =34,
    eBinDispFix12   =35,
//    eBinDispLoad2   =36,
    eBinDispTotal
};
#define MAX_BIN_UNIT eBinDispTotal                                              //JerryYang 20230515 : 12->27

enum eMagBinDispType                                                            //JerryYang 20230515 : Magazine BIN DISP版本
{   eMagBinUninstall =0,
    eHTA18           =1,                                                        //HTA18
    eHTBT008         =2,                                                         //HT-BT008
    eTFT             =3
};

enum eTrayArmMode                                                               //Frank 20230419
{   eAboveCoveyor   =0,
    eUnderCoveyor   =1,
};

enum eBtnScannerAOI                                                             //Jimmychiu 20240315 : add Scan IC Both the Top and Bottom sides
{   eBtnAOI_Uninstall           =0,
    eBtnAOI_BottomInstall       =1,
    eBtnAOI_TopBottomInstall    =2,
    eBtnAOI_Total               =3,
};

enum eCylPrAlarmItem  //Sam 20230516 : Pre Alrm Cylinder
{
    eCylName            =0,
    eCylOnOffCnt        =1,
    eCylOnTime          =2,
    eCylOffTime         =3,
    eCylAvgOnTime       =4,
    eCylAvgOffTime      =5,
    eCylTimeOutCnt      =6,
    eCylOnOffCntAlarm   =7,
    eCylOnTimeAlarm     =8,
    eCylOffTimeAlarm    =9,
    eCylTimeOutCntAlarm =10,
    eCylResetCount      =11,
    eCylResetTime       =12,
    eCylPrAlarmItemTotal,
};

enum eGroundMaster                                                              //Sam 20230906 : 新增 GroundMan Msg Log
{
    eAlarmSet       =0,                                                         //0x30  硬體Alarm值設定 => 不需要，以HandlerSys設定之Ohm做判斷
    eMachineOhm     =1,                                                         //0x31  阻值詢問
    eAlarmGet       =2,                                                         //0x32  警報詢問 => 不需要，以HandlerSys設定之Ohm做判斷
    eBoardOhm       =3,                                                         //0x33  讀取偵測板-內部組值
    eBoardReset     =4,                                                         //0x34  偵測板Reset
    eBoardVersion   =5,                                                         //0x39  讀取偵測板-韌體版號
    eGroundTotalCMD =6
};

enum eNNMode
{
    None_NN=0,
    NN_1Row=1,
    NN_2Row=2
};

enum eTrayArmPos                                                                //Sam 20241206 : Tray Arm 新增 Teach 保護
{
    eTrayArmLoader  =0,
    eTrayArmEmpty   =1,
    eTrayArmColor   =2,
    eTrayArmAuto    =3,
};

//Steven 20260924 (S12-C)：golden V912 有、移植樹沒有；FileRW/IniConfig.gen.inc（golden cConfiguration.cpp）要用到，逐字補上（golden MachineType.h:1488-1492）
enum eTrayArmHWType                                                             //Ifor 20260824 : Tray Arm hardware type (HandlerSys option)
{
    eTrayArmStandard=0,                                                         //original Tray Arm
    eTrayArmNewType =1                                                          //new Tray Arm, standby at middle of Empty and Color
};

enum ePLCItem                                                                   //KenHsieh 20250208 : Fix PLC 斷線
{   ePLCEmpty1=0,
    ePLCEmpty2=1,
    ePLCEMG1=2,
    ePLCSafeDoor1=3,
    ePLCSafeDoor2=4,
    ePLCSafeDoor3=5
};
enum eRMSMethod                                                                 //Jimmychiu 20250707 : add RMS connect method
{   eByNetwork       =0,
    eByFTP           =1,
    eRMSMethodTotal
};

enum eAGVPort           //JerryYang 20250505 : add
{
    ePortLoader  =1,
    ePortEmpty   =2,
    ePortColor   =3,
    ePortAuto1   =4,
    ePortAuto2   =5,
    ePortAuto3   =6,
    ePortAuto4   =7,
    ePortAuto5   =8,
    ePortAuto6   =9,
    ePortFix1    =10,
    ePortFix2    =11,
    ePortFix3    =12,
    ePortFix4    =13,
    ePortFix5    =14,
    ePortFix6    =15,

    ePortReserv  =16,           //Steven 20251216 : to avoid memory leakage
    ePortTotal
};

enum eLoadPortState           //JerryYang 20250505 : add
{
    eLoadInit       =0,
    eLoadReadyToLoad=1,
    eLoadHighWIP    =2,
    eLoadTrayArrived=3,
    eLoadLowWIP     =4,
    eLoadEmpty      =5,
    eLoadError      =6
};

enum eUnloadPortState           //JerryYang 20250505 : add
{
    eUnloadInit       =0,
    eReadyToUnload    =1,
    eFullBundle       =2,
    eUnloadTrayArrived=3,
    eUnloadEmpty      =4,
    eUnloadError      =5
};

enum eFixPortState           //JerryYang 20250505 : add
{
    eFixPortInit              =0,
    eFixReadyUnloadFullBundle =1,
    eFixFullTray              =2,
    eFixFTrayArrived          =3,
    eFixEmpty                 =4,
    eFixError                 =5
};

enum e2x8Mode                                                                   //Steven 20240430 : 整合2x8特殊模式
{
    e2x8Standard        =0,
    e2x8Step2           =1,
    e2x8OneByOne        =2,
    e2x8_STMMode        =3,
    e2x8_TW153Mode      =4,
    e2x8CloseEven       =5,
    e2x8CloseEven1By1   =6,
    e2x8CloseOdd        =7,
    e2x8CloseOdd1By1    =8,
    e2x8Run2x2_13       =9,
    e2x8Run2x2_14       =10,
    e2x8Run2x4Standard  =11,
    e2x8Run2x4Step2     =12,
    e2x8ModeTotal
};

enum eInSHSenType {eInSH8Sen=0, eInSHAutoLtc=1};                                //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料

enum MachineState
{
    STATE_STOPPED=0,  // 未開始或已結束
    STATE_RUNNING,    // 運作中
    STATE_PAUSED,     // 暫停中
    STATE_PAUSED_BEFORE_TEST,     // Start前的暫停
    MachineStateTotal,
};
//=============  Template ======================================================
template<class T>
T CheckRange(const T Value, const T Maximum, const T Minimum)
{
    if(Maximum<Minimum)                                                         //如果大小相反
    {
        if(Value>Minimum)
            return Minimum;
        else if(Value<Maximum)
            return Maximum;
        else
            return Value;
    }
    else
    {
        if(Value>Maximum)
            return Maximum;
        else if(Value<Minimum)
            return Minimum;
        else
            return Value;
    }
};
//------------------------------------------------------------------------------
//Steven 20161208 : 針對除法進行判斷,如果分母為0則回傳預設值;否則回傳計算值
//------------------------------------------------------------------------------
/*template<class D>
D Division(const D Numerator, const D Denominator, const T ValueForDivideByZero)
{
    D Value;
    if(Denominator==0)
    {
        Value=ValueForDivideByZero;
    }
    else
    {
        Value=Numerator/Denominator;
    }
    return Value;
}*/
//------------------------------------------------------------------------------
//Steven 20120203 : 將輸入的原件轉為對應的語系字集
//------------------------------------------------------------------------------
template<class T>
void SetCompomentLanguage(const T Compomen, eUserLanguage LanguageSet)
{
    if(LanguageSet==eulKorea)
    {
        Compomen->Font->Charset=DEFAULT_CHARSET;
//        Compomen->Font->Charset=HANGEUL_CHARSET;
    }
    else if(LanguageSet==eulJapan)
    {
        Compomen->Font->Charset=DEFAULT_CHARSET;
//        Compomen->Font->Charset=SHIFTJIS_CHARSET;
    }
    else
    {
        Compomen->Font->Charset=DEFAULT_CHARSET;
    }
};
//------------------------------------------------------------------------------
template <class T> AnsiString ChangeToPercentage(const T Numerator, const T Denominator)
{
    AnsiString str="0.00%";
    if(Denominator!=0)
        str.sprintf("%0.2f%%", ((double)Numerator/(double)Denominator)*100.0);
    return str;
};
//------------------------------------------------------------------------------
template <class T> float ChangeToFloat(const T Numerator, const T Denominator)      //kevin 20150701 取出  float %
{
    double str=0.00;
    if(Denominator!=0)
       str=((double)Numerator/(double)Denominator)*100.0;
    return str;
};
//------------------------------------------------------------------------------
template <class T> float ChangeToFloatNonPcnt(const T Numerator, const T Denominator)    //RogerYang 20250812 : None Percent
{
    double str=0.00;
    if(Denominator!=0)
       str= ((double)Numerator/(double)Denominator);
    return str;
};
//------------------------------------------------------------------------------
template<typename T>
inline void vec_clr(vector<T> & v)
{
    vector<T> t;
    v.swap(t);
}
//------------------------------------------------------------------------------
//AI(W906-IOWEB-P17) 20260925: 引擎的 IO 接到 1203（週末計畫 A4-7）。使用者 20260925：「請幫我接上1203」。
//  放在檔尾而不是 :105-:159 那組 1203 開關旁邊，理由與 IOWEB-P4 把 PCI1203_IO 放在 cmydef.h 檔尾相同：
//  那一段的行號被全樹大量引用（:106／:121／:159 …），插一行就全部位移。
//
//  開 = wb_serve 開機時把 MyLaneIO 的 1203 後端（IOBackend.cpp TPci1203Backend 的 #else 臂）接到
//       EtherCAT/Pci1203IoRoute.cpp：
//         讀 SW[]／Sen[]／Cylinder[] 的 1203 點 -> 1203 監看器的 DI／DO 取樣（不新增任何廠商呼叫）
//         寫                                   -> Pci1203Control::Execute（EastSun 的層：白名單、DRY/LIVE、
//                                                  稽核）；拒寫 ring 0／伺服驅動器是路由自己的前置檢查
//       ⇒ 引擎讀到的是卡片上真的感測器；引擎與 HW.IoSetView 按鈕打出去的輸出走同一條路。
//  關 = TPci1203Backend 照舊是樁：讀 0、寫了什麼都不做（改動前的行為）。
//
//  ⚠ 真的會不會通電**不是這一行決定的**，是 :106 的 WB_PUMP_1203_CONTROL_LIVE：
//      LIVE 關        = 每一筆寫入照樣驗證、定址、記錄，但不呼叫 vendor（DRY RUN）
//      LIVE 開（現在）= 引擎的 SW[].On()／氣缸／吸嘴與網頁按鈕**真的讓線圈通電**（golden 也是如此）
//    使用者 20260925 裁決 LIVE 常開（:106）。**開軟體的那一刻起**會發生、而且要讓機台旁的人知道的：
//      * 引擎閒置時每個 tick 都在驅動塔燈與蜂鳴器（DRY RUN 實測 20260925：ring 1 站 16 port 17-23 =
//        SwTowerRed／Yellow／Green、SwMusic1-4，黃燈 ON）⇒ 一開就亮燈；任何警報會真的響。
//      * 安全門互鎖**不涵蓋** HT9050 的 1203 輸出：沒有引擎物件的 54 個輸出（C_*Off 第二線圈、
//        C_*EdgeClip、C_*DrawerLock、C_InPnPDrop1-4 …）從不被標記，而有物件的 HT9050 氣缸在 golden
//        的 bypass 清單上（cinitial.cpp:5299-5304）；門感測器本身也還沒進 1203 表
//        （WEEKEND_PLAN_20260925.md:278-281）⇒ 開著門也按得動氣缸。
//      * golden 的 OutPortData[4][64][4] 命令快取對 1203 位址越界、14 對輸出共用同一個快取位元  ✅ 20260926 已修（`6367d599`：只放大 OutPortData／BackOutPortData 到 [4][256][32]，MyLaneIo.h:60；1203 輸出超出快取回 2）—— 下面兩行是修正前的狀況
//        （Pci1203IoRoute.h 檔頭）⇒ 引擎的 SW[].Status()／氣缸 GetOutBit() 可能回報到另一個點；
//        卡片本身的讀寫不受影響（路由照 ring／站號／通道），畫面顯示的是卡片回讀。
//      * Index Z1/Z2 剎車（SwFMotorBreaker／SwBMotorBreaker）網頁一律不准放：馬達層還沒武裝，
//        IsIndexMotorOutOfPower() 恆為真（JsonBridge/IoBtnPanelClick.cpp）。
//      * Pci1203AxisIniTick 的 HLMT 極限邏輯回寫（W2）同樣變成真的寫卡。
//      * WB_PUMP_1203_START_RING（:159）**仍然關著**：ring 0（馬達環）不會被啟動；ring 1 的 DI／DO
//        不需要它（實測）。
//  ⚠ 需要 INSTALL_1203_MONITOR（:121，讀）與 WB_PUMP_1203_CONTROL（:105，寫）也都定義；
//    SOFT_SIMULTE 組態不接（MyLaneIO 在那裡維持模擬後端）。
#define WB_ENGINE_IO_1203
//#define WB_ENGINE_MOTOR_1203   //AI(W906-ECAT-ROUTE) 20260929: OFF by default (docs/ENGINE_1203_MOTOR_ROUTE_DESIGN.md 7.1 "預設關"). ON = wb_serve installs EtherCAT/Pci1203MotorRoute.cpp: the ENGINE's (MainProc) TMyEtherCatMotor moves / stops / homes reach the card through Pci1203Control::Execute with NO click (with :106 _LIVE they really move). Also needs INSTALL_1203_MONITOR (:121) + WB_PUMP_1203_CONTROL (:105) + WB_PUMP_1203_START_RING (:159, Q9), a build without SOFT_SIMULTE, and an armed control. To arm: uncomment here AND set WB_ENGINE_MOTOR_1203 = $true in tools/pci1203_control_gate.ps1 $expectActive -- both, or the gate and the tree disagree.
//#define WB_ENGINE_INDEXZ_1203   //AI(W906-INDEXZ) 20260930: OFF by default (INBOX 113 redo of cc426093; RULINGS_20260929 section 5 D2 was ON, this redo keeps it OFF like WB_ENGINE_MOTOR_1203 above). ON = wb_serve installs EtherCAT/Pci1203GaliRoute.cpp: HT9050's Index Z1 (M14 MTestZ1, PCIE-1203) answers golden's Galil strings from the 1203 monitor and moves through Pci1203Control::Execute -- the golden Index flow (DoTestHeadMotor, AutoClean, ProcessMotorHome) presses Z1 with NO click; at install M13/M15/M16 are disabled from Mot_Table (D1), IndexZCanMove={1,1} (D3), the torque hook goes in (D4). Also needs INSTALL_1203_MONITOR (:121) + WB_PUMP_1203_CONTROL (:105) + WB_PUMP_1203_START_RING (:159), a build without SOFT_SIMULTE, an armed control and an OPEN card; first automatic Index run with EastSun present; D13 / D63 must stay 0. To arm: uncomment here AND set WB_ENGINE_INDEXZ_1203 = $true in tools/pci1203_control_gate.ps1 $expectActive -- both, or the gate and the tree disagree.
//------------------------------------------------------------------------------
//  AI(W906-EXIT-NOHOME) 20260930: 主畫面 Exit（關閉軟體）要不要先檢查「所有馬達都在原點」。
//  golden sbCloseProgramClick（V912 main.cpp:29062-29068）：登入等級低於 LevelSet.AccessLevel[6] 時，沒歸零就擋下並跳
//  「Must finish home process before terminal／請在關閉程式前,執行歸零步驟」。
//  EastSun 20260930：「左上 軟體關閉的按鈕 我不需要確認都歸零 我只需要讓客戶確認是否真的要關閉軟體」→ 預設不檢查。
//  恢復 golden：打開下面這一行，重建 wb_serve（FileRW/MainClose.cpp CloseGuards）。
//#define W906_EXIT_REQUIRE_HOME
//------------------------------------------------------------------------------
//AI(W906-VACUNIT-1203) 20260930: golden 的 ECAT-VC8 吸嘴對應（VacuumUnit/VacuumUnit.cpp SetIOTableByECAT_VC8_Sucker／SetSuckISABase／
//  VaccumCopyFormSuck，golden :431-522；golden main.cpp:10497-10498 開機時 fVacuumUnit->Initial() 之後就呼叫）。
//  EastSun 20260930 裁決 R3「照 golden」→ 預設開。
//  開 = Gerneral.ini VacuUnitType==1 時，開機（FileRW/TestIF_File_VacuumUnit.cpp FileRW_VacuumUnit_Boot）把 FTestSuck／BTestSuck
//       （2×8）與 InArmSuck／OutArmSuck（2×4）交給 ECAT-VC8：吸嘴的 On/Off/Sen/ISABase 設成 ePCI1203，真空單元頁面的面板位址
//       改用吸嘴表（IO_Table.csv）的位址；而且（W906）這些吸嘴的開關／感測只在那一站真的是 ECAT-VC8 時才送出
//       （VacuumUnit/Vc8Route.h W906_Vc8SuckerGate），否則走 golden 的失敗分支（MNetLog）並寫出原因 —— BTestSuck 在 ring 1
//       站 0x50／0x51 的 DO 16..19 與上料／Auto1 的氣缸（C_Load_Up、C_LoaderDrawerLock、C_Auto1_Up、C_Auto1DrawerLock）同通道。
//  這台今天（ring 上沒有 VC8；144 列吸嘴全是 Enable=0，ISABase 本來就是 3）：引擎行為不變，只多了拒絕與原因。
//  關（註解掉）= 完全回到今天：開機不建面板、不對應、不登記閘門（面板在頁面第一次打開時才建，用 golden 建構子的位址表）。
#define W906_VC8_SUCKER_REMAP
//------------------------------------------------------------------------------
#endif
