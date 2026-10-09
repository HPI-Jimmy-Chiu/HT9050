# 原文 15／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp`；定位 `MbWait_notice_held`；種類 `regions`。
來源 commit `c90d8d22bb2d34c532443386469de717aef2e671`；完整摘錄 SHA256 `733a1b9ffd88a1ecd47c2726485ca5e42b25298e1a93427401928de8e6783063`。
分頁只切閱讀長度；包括原註解、gate、裁決註記，歷史 test 敘述不是本輪實測。

```cpp
<!-- preserved-content:start -->
            } else if (wc.cmd == "dialog.notifyAck") {   //AI(W906-NOTICE-DEFER) 20261003: EastSun「我歸原點後 有異常 排除後再按歸原點 就沒用了」-- 21:56:31 the PAUSE on the motor jam NOTICE came while the "Motor not home yet" box waited and was refused modal-pending: the notice stayed open (the page had closed it) and every later HOME key read "擋關：通知框開著" (golden ScanKey ignores keys while the note is up). Not refused any more: carried to the main loop, which runs it (and answers it) as soon as this box closes
                { const std::string tg = wc.hasTag ? wc.tag : std::string(); std::string nw; if (!::W906_NoteAuthNoticeGate(tg, &nw)) g_modalServer->CompleteCommand((unsigned long long)wc.id, false, nw); else { w906NoticeHeld.push_back(wc); g_w906NoticeGatePassed.insert((unsigned long long)wc.id); if (::W906_NoteNoticeAckRefusal(tg.c_str()) == 0 && w906dlg::NotifyAckDecide(g_alarmSlot, tg) == w906dlg::kNotifyAckRetire) g_modalServer->CompleteCommand((unsigned long long)wc.id, true, "{\"notice\":\"held\",\"requestId\":\"" + tg + "\",\"runsWhen\":\"" + qid + " closes\"}"); } }   //AI(W906-NOTICE-DEFER-4) 20261007: review of 45d4f4d -- the early ok only checked the BtnPauseClick refusal; the held run's auth gate (main loop :4915) could still refuse after the page closed the note (e.g. auto-logout while this box was up) -> the note lived on in C++ and keys read "通知框開著" again. Now the SAME gate runs here first, at the press as golden DoPassword does: refused -> answered refused (the note stays, press again after dialog.auth), not held; passed -> held, its id remembered so :4915 does not run the gate twice (it spends the one-shot pass); early ok only when the slot really holds this notice (NotifyAckDecide == retire) and BtnPauseClick would not refuse.  AI(W906-NOTICE-DEFER-3) 20261007: EastSun「這畫面我按pause 10秒後才有反應」/「我按reset 畫面很久alarm才會被清除」-- the page shows one stop box at a time and kept "Motor not home yet" queued behind the jam note until this ack was answered, while this wait held the ack until that box closed: a deadlock broken only by the page's 15 s timeout. Now the page gets ok at once (note closes, the box shows, as golden: note PAUSE -> MyMessageBox); the close work itself still runs after the box (held below; its later CompleteCommand on this id is a no-op). Not when golden BtnPauseClick would refuse (the note must stay).   //AI(W906-NOTICE-DEFER-2) 20261004: held, handed to g_carry when this wait returns (the w906NoticeHeld line at the top of MbWait); was g_carry.push_back + g_carryRunnable
                std::printf("  [MyMessageBox] %s 等待中收到通知框的 PAUSE（dialog.notifyAck）-> 框關掉後由主迴圈處理\n", qid.c_str());

<!-- preserved-content:end -->
```
