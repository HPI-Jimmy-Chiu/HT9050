# 原文 03／分頁 3

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/forms/fNote_ShowError.cpp`；定位 `W906_NoteNoticeAckLikeGolden`；種類 `complete_cpp_functions`。
來源 commit `c90d8d22bb2d34c532443386469de717aef2e671`；完整摘錄 SHA256 `3af2f5fb72ef93e6b4724de6e21f4d75980b433beea627cd49b753ec0297f166`。
分頁只切閱讀長度；包括原註解、gate、裁決註記，歷史 test 敘述不是本輪實測。

```cpp
<!-- preserved-content:start -->
//                sFixBundleID[2]=fNote->edBundleID->Text;
                sBundleEndInfo=GetBundleInfo(8);
                EventReport(SECS_EVENT.BundleEnd_Fix3);
                EventReport(SECS_EVENT.BundleEnd_IDREAD_Fix3);

                slDupUnloadBundlID->Clear();
                slDupUnloadBundlID->LoadFromFile(aslDupUnloadBundlID);
                slDupUnloadBundlID->Add(sUnloadBundleID);
                slDupUnloadBundlID->SaveToFile(aslDupUnloadBundlID);

                sFixBundleID[2]="";
                RecordProcess("Fix3 Bundle End event report finished.");
                bNeedReportBundleID[eFix3]=false;                               //JerryYang 20250220 : fix AUTO IN OUT
            }
        }
    }
#endif // GATE(W906-J5-ACK) C9
// [W906] SCOPE(W906-J5-ACK): golden :2751-2758 -- L46 AStream "compress one cycle" restarts the machine (fMain->Start; the port's equivalent is StartFromWeb).  The dependency exists; starting motion from an acknowledgement is outside INBOX 119 -- a user decision  [kept as comment lines, not #if 0: tools/start_sites_census.py counts a gated fMain->Start( as a start site (ctest START_SitesCensus pins 34 / 30 / 4)]
//    if(IniConfig.bL46_AStreamErrorCompressOnecycle)                             //Ztex 2024.10.01 Add AStream Error Compress Onecycle
//    {
//        if(iAStreamErrorCompressOnecycle==3)
//        {
//            iAStreamErrorCompressOnecycle=0;
//            fMain->Start("bL46_AStreamErrorCompressOnecycle");
//        }
//    }
    hAutoCleanHangUp.SetSecAndOn(Prod.iHangupMaxTime);                          //Steven 20220823 : 機台有暫停就要重新計算
    tGalilTwoYMoveDelay.SetSecAndOn(60);
#if 0 // GATE(W906-J5-ACK) C10: golden :2761 -- bPLCFlag has no definition in this tree (git grep 20260930: 0 hits)
    bPLCFlag=false;                                                             //KenHsieh 20250307 : fix PLC safedoor 通訊延遲問題
#endif // GATE(W906-J5-ACK) C10
    }
    passSec=(unsigned long)PassTime;
    (void)KeyCode;
    }
    else if (mine)
    {
    W906_Notice.valid=false;                                                    // [W906] pause 3: golden shows no note -- nothing to close
    }
    if (pauseOut)      *pauseOut=pause;
    if (jamCountedOut) *jamCountedOut=jam;
    if (passTimeOut)   *passTimeOut=passSec;
    return mine;
}

<!-- preserved-content:end -->
```
