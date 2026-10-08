# caption、客戶與 runtime 軸

[common_ChangeLog.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/25163d8e4e2df6f64f531bb1c8894fcd1d7b5fd0/HT9011UC_Cpp_V3.33.906.0/common_ChangeLog.cpp)、[cpublic.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/25163d8e4e2df6f64f531bb1c8894fcd1d7b5fd0/HT9011UC_Cpp_V3.33.906.0/cpublic.cpp)、[common.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/25163d8e4e2df6f64f531bb1c8894fcd1d7b5fd0/HT9011UC_Cpp_V3.33.906.0/common.cpp)；完整所選原文、介面／歷史與 byte／body hash 見 [manifest](source-manifest.json)。

CL_Pick 在 i>=0 且 i<n 時建 AnsiString(a[i])，否則空字串。下表都是選定 V906 source 的 caption，golden dfm／舊 comment 原文保留，未作913新對照。

| helper | caption／順序 |
| --- | --- |
| CL_fCleaning_rgCleanKitType | Kit、Tray |
| CL_fCleaning_rgAutoCleanSelectArm | Arm1、Arm2、Arm1 & Arm2 |
| CL_fCleaning_ContactMode | Direct Contact Mode、Drop Contact Mode |
| CL_fContact_coD41 | Inside socket、Above Socket |
| CL_FTestIF_rgInterfaceType | DIO、GP-IB、RS232、TCP/IP |
| CL_FTestIF_rgBaudRate | 19200、9600、4800 |
| CL_FTestIF_rgBitLength | 7 Bits、8 Bits |
| CL_FTestIF_rgParity | Even、Odd、None |
| CL_FTestIF_rgStopBit | 1 Bit、2Bits |

CL_FTestIF_cbGPIBType 的 CUSTOMER_CODE==CC_ASE_KaohSiung 分支9項、第二項256 Bin；一般分支10項、第二項255 Bin、尾端多Delta_Castle。其餘完整名稱與順序在 manifest。這是客戶軸，不能當作 HT9050 專屬差異。

CL_FTestIF_cbDIOType 用 FindFirstFileA(DIOCFGPath+"*.ini")／FindNextFileA；略 hidden、.／..，副檔名 LowerCase()==".ini" 才去掉 extension，以列舉中的 n==i 選名。沒有排序、選到後也未早停；無 explicit FILE_ATTRIBUTE_DIRECTORY 排除。結果依當次 runtime 路徑、檔案／屬性與 Windows 列舉順序；handle 無效回空字串。本輪僅保存函式，未列舉任何機台目錄或證明與 UI list 同步。

CL_fContact_cbContactMode 在 fContactForm==NULL 或 i<0／i>=Items->Count 時回空，否則讀 Items->Strings[i]。cbContactMode／Items 沒有獨立 null guard；不能稱全層級 null-safe。form 建立、list 初始化／銷毀與 thread 時序待另續。

Int 的 Contact.Data Mode／Contact、iSocketInitialICCheckPosition；Tester.Data 的 Mode、GP-IB、DIO、RS-232C；HandlerCondition.Data 的 AutoClean arm／mode 各用對應 helper。bool 的 UseTray 以 bool 作 CleanKit 索引。選到 caption 後的 empty 值不等於 source 提供錯誤回報；歷史 NB2 例外／fallback 描述保留，不代替當前 BCB6／913／runtime 驗證。

| 分軸 | 已查證／未查證 |
| --- | --- |
| HT9050、HT9045／9046 | 這些 helper 無 Type_HT9050 分支；同題共享 facade，不代表每台 configuration／caption 相同 |
| 客戶 | GPIB ASE 條件有正文；其他客戶仍依傳入 file／group／name、CUSTOMER_CODE與版本 |
| runtime | DIO 檔列舉、fContactForm／Items、InitialOK與hook安裝依啟動實況，未操作 |
| 版本 | V906 UTF-8／C++17；V912／V899與913 BCB6／Big5未重新測量或移植 |

回 [names](names.md)、[機型樹](../../../../../../machines/index.md)、[界線](limits.md)。
