// AI(W906-PROD-G030) 20260926 (Steven 團隊)：golden `fSCKART->SaveTestSummary(iSaveData)`（V912 Automation/SCK_ART.cpp:1620-1646）
//   的共用轉接。Jimmy 20260926 23:4x（TO_STEVEN §4 ④）要：把 G-030 在 forms/fLotInfo.cpp 裡做的轉接抽出來，
//   讓 csystem.cpp 的兩個空樁（W7C1_TfSCKARTSeam::SaveTestSummary :2701、W7C2_TfSCKARTSeam::SaveTestSummary :3913）也能呼叫。
//
//   為什麼要轉接：golden 只有一個 fSCKART、一個 LotSummary；移植樹各有兩份 ——
//     fSCKART：Automation/SCK_ART.h 的真物件（fLotInfo／網頁讀寫的那一個） vs. SCK_ART_Remainder 的 SckArtRemainderState；
//     LotSummary：cSocket.cpp:228 的真物件（ainarm9045.cpp iLoadTotal++、asortarm.cpp AddCount 寫的那一個）
//                 vs. SCK_ART_Remainder gate #5 的替身 W5SckArtRem_LotSummary（SckArtRem_SaveTestSummaryTSV 讀的那一個）。
//   本函式把真的那兩份抄進 SCK_ART_Remainder 用的 state／替身，跑 golden 本體（SckArtRem_SaveTestSummary，已照翻），
//   再把 golden 會改的欄位抄回真的那兩份。兩份都不改結構。
//
//   不跑的分支（照實記進 skipped；skipped 為 NULL 時印到 stdout）：
//     2D sort（Save2DSortingSummary）與 93K SECS ART（SaveTestSummarySECS）要 SckArtRemainderState 的 sInfo_*／sLotEndTime…，
//     真的 fSCKART 沒有這些欄位（客戶專屬 S80／S25）—— 硬跑會用空字串寫出錯的報表。
//
//   會寫的真實檔（log，不是機台參數）：
//     iSaveData==1 且一般機台：D:\HT9045_Log\Summary_Lot\YYYYMM\...Summary.txt（SaveSummaryTrayFeed；W906_SUMMARYLOT_ROOT 可轉開）；
//     SaveTestSummaryTSV：asSummaryPath（D:\HT9045_Log\Summary）\YYYY\MM\<LotID>_<Process>_FT_…txt，並建 D:\HT9045_Log\TestSummary\YYYY\MM\。
#pragma once

#include "vclcompat/vcl_compat.h"   // TStrings（vcl_compat.h:245 using vclcompat::TStrings）

// golden TfSCKART::SaveTestSummary(int iSaveData)。skipped：沒跑的分支說明（可以是 NULL）。
void W906_SckArt_SaveTestSummary(int iSaveData, TStrings* skipped);
