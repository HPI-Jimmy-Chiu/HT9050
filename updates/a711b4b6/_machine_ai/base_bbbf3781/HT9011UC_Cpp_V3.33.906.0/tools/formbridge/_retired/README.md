# tools/formbridge/_retired —— 改走 C 形狀的表單（gen_formbridge.py 不讀這個資料夾）

| 表單 | 改由 | 原因 |
|---|---|---|
| `TfSpeed.py` | `tools/gen_editlist.py` STRUCTS `ArmSpeed_File`（prefix SP）→ `FileRW/ArmSpeed_File.gen.inc`＋`.cpp` | golden 存檔鈕 `spbSaveClick` 不呼叫 `SaveSetupFile`、自己逐鍵寫 139 處；A 形狀的「空跑 save」涵蓋不到。TTrackBar／TUpDown 的 Position／OnChange 需要 VCL 語意（`filerw::ELTrackBar`）。Steven 20260924。 |
| `TfTrayAssignment.py` | `tools/editlist/TrayForm.py`（C 形狀，進行中 20260924 夜） | 審查第 8 輪 H-2：A 形狀的 display 在 HTTP 執行緒跑 fTrayAssignment->ReadFile（改執行期全域 TrayForm／Prod.iTrayType）；C 路在主迴圈 WS editlist.get 跑 golden FormShow。 |
| `TfContact.py` | `tools/editlist/DeviceForm_File.py`（C 形狀，進行中 20260924 夜） | A 形狀沒有建構子、非 void 回傳、ScrollBar Position、選取狀態；display 改執行期全域在 HTTP 執行緒。 |
| `TfDIOFrom.py` | `tools/editlist/TTLCfg.py`（C 形狀，FileRW/TTLCfg.cpp） | Steven 20260925：DIO 檔名照 golden TFTestIF::InitcbDIOType／cbDIOTypeChange 的 cbDIOType->Text，不用 sDioName 繞；A 形狀 display 在 HTTP 執行緒改全域 TTLCfg。 |
| `TfBinSel.py` | `tools/editlist/BinSelect.py`（C 形狀，FileRW/BinSelect.cpp） | Steven 20260925：資料在 fBinSel 的 sXxx[tag] 字串清單與 MyBinPanel[tag]，JSON 直接交換（golden ReadFunctionData／SaveFunctionData），不模擬面板。 |
| `THandlerSystem.py` | `tools/editlist/HSys.py`（C 形狀，FileRW/HSys.cpp） | Steven 20260925：開頁照 golden `THandlerSystem::FormShow`、讀檔照 `SYSTEM_MODULAR::ReadGeneralIni`；開頁可以補寫 Gerneral.ini（Steven 裁決，H-1 解除：C 路在主迴圈跑）。bHandlerModel 錯＝ELMessage＋拒存，不結束程式。 |
| `TfOffSet.py` | `tools/editlist/Offset_File.py`（C 形狀，FileRW/Offset_File.cpp） | Steven 20260925：全部 offset 整包 JSON（開頁對每個選取跑一次 golden 按鈕事件），點按鈕才顯示該組；存檔逐組跑 golden spbSaveClick 的 SaveFile 段、尾端只跑一次。 |
