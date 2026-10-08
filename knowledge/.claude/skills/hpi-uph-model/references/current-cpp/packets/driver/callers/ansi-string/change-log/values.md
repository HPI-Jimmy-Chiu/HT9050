# numeric gate、printf 實參與 MM 單位

[common_ChangeLog.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/25163d8e4e2df6f64f531bb1c8894fcd1d7b5fd0/HT9011UC_Cpp_V3.33.906.0/common_ChangeLog.cpp)、[cpublic.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/25163d8e4e2df6f64f531bb1c8894fcd1d7b5fd0/HT9011UC_Cpp_V3.33.906.0/cpublic.cpp)、[common.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/25163d8e4e2df6f64f531bb1c8894fcd1d7b5fd0/HT9011UC_Cpp_V3.33.906.0/common.cpp)；完整所選原文、介面／歷史與 byte／body hash 見 [manifest](source-manifest.json)。

| function | gate／特殊分流 | printf 實參與文字 |
| --- | --- | --- |
| W906_ChangeLog_Bool | ret!=bValue 且 InitialOK；HandlerCondition.Data／Configuration／bAutoClean_UseTray 取 CleanKit caption | 一般 %d 接 bool，varargs 預設 promotion 到 int；特殊分支 %s 接 caption.c_str() |
| W906_ChangeLog_Int | ret!=Value 且 InitialOK；FileName.Pos("config.ini")>1、Group O_Count、Name.Pos("O_1")==1 時不記 | 一般 %d 接 int；Contact／Tester／AutoClean 特殊 caption 接 %s；ContactTime 的 %0.1f 接 double(ret)／10，顯示 s |
| W906_ChangeLog_ULong | ret!=Value 且 InitialOK | 正文 %d 接 unsigned long，specifier 與實參型別不相符；generic conv 原樣傳值，不代換格式或保證 ABI |
| W906_ChangeLog_Double | ret!=Str.ToDouble() 在 && InitialOK 之前評估 | 一般 %0.4f 接 double；Kit Diameter 的 %0.1fmm 接 ret*10／Value*10；所選 MM 分支另走 int converter |

所有 hook 先 TempChangeLog(Group,Name)，再以 FileName.Pos("Offset") 決定 Offset prefix。Pos 是 [1-based byte 搜尋](../bytes/index.md)；Int 的 config.ini gate 嚴格 >1，不能改寫為任意位置命中。各 file／group／name 的 else-if 優先序與 fallback 依保存正文，不把所有匹配同時套用。

double 的 gate 比的是舊 ret 與 caller 四位小數 Str 的再解析，並非 ret!=Value。ToDouble 可在 InitialOK=false 時先執行，且這個 hook 不包 catch；CRT locale、NaN／Infinity、parse／range 尚未執行驗證。Test Arm1／Test Arm2 的 contact 名稱、bEnableReadAndCheckTorque 分支會同時設 bResetArm1Value／bResetArm2Value=true；文件查證不呼叫此函式。

## ConvertToMMType → GetFloatFormatString → CutSpaceAtHead

唯一所選宣告是 ConvertToMMType(int i)，以 double(i)/100.0 呼叫 GetFloatFormatString(...,4,2)。Int 的所選 AutoClean 14 個 offset／height key 直接傳 int。Double 的 ShuttlePitchOffset、dAutoClean_XStart_Kit／XPitch_Kit／YStart_Kit／YPitch_Kit 也傳到同一 int 入口，先發生 double→int 轉換，才除100；小數截斷、非有限或超出 int 可表示範圍不可當成已安全支援。

GetFloatFormatString 使用共用 static char str[256]、local fstr[256]="%5.3f"，以字元 '0'+(P1+P2) 與 '0'+P2 改寬度／precision；本 caller 的 P1=4、P2=2 組成 %6.2f。sprintf 不帶 buffer 上限或回傳值檢查，再 CutSpaceAtHead(str)，回傳同一 static pointer。任意 P1／P2 並非完整十進位 format builder；沒有多執行緒／重入隔離。Int／Double hook 在兩次換算後分別立刻建 AnsiString temp1／temp2 再給 %s，避免保留兩個指向同一 str 的 pointer；這個複製時機不能證明整支 helper thread-safe。

CutSpaceAtHead 只跳過 ASCII space；遇 NUL return，否則複製進 local str2[256] 再 strcpy 回 S，沒有 null／長度 guard。全空格輸入在跳到 NUL 後直接 return，沒有把原輸入改空字串。這是正文分析；本輪未給極端值、跑 CRT 或寫記錄。

14 個 Int key、完整所有特殊 case 與原文見 manifest；caption 字串來源見 [caption](captions.md)，printf backend 只適配 AnsiString→c_str 見 [既有實作](../numeric-format/printf-family.md)。型別差異已記，沒有改 C++、臆測實際輸出或稱為機台錯誤。
