# edtCustomerCode：可改性與文字型別

來源：[_EditList.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Cpp_V3.33.906.0/FileRW/_EditList.cpp)的 `ELEditable`／`ELOperable`／`ELApplyProxies`／`KindOf`／`FieldOf`／`TypeOk`；[HSys.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Cpp_V3.33.906.0/FileRW/HSys.cpp)的 `BeforeApply`；[HSys.gen.inc](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Cpp_V3.33.906.0/FileRW/HSys.gen.inc)的 `HS_CreateSaveProxies`／`HS_CreateContainerProxies`／`kHS_ParentOf`／`HS_DfmState`／`HS_FormShow`。

## 可改性

`ELEditable(form,name)`先查 `P().readOnly`，再呼叫ELOperable。後者沿parentOf最多64層走祖先，找到control時檢查Enabled／Visible，TTabSheet另查TabVisible；沒有直接用ActivePageIndex判斷可改性，缺少control也不在此處直接拒絕。

`edtCustomerCode`由HS_CreateSaveProxies建立為TEdit；實際登記的祖先依序是 `edtCustomerCode → GroupBox3 → pnlHandler1 → TabSheet1 → pcSetting → Panel2`。HS_FormShow把 `tsCustomerCode.TabVisible`設false，但tsCustomerCode不在這條父鏈，不能單靠此敘述推定edtCustomerCode不能改。HS_DfmState中此欄位的選讀敘述是Text="0"；沒有完成readOnly集合與所有祖先狀態寫者的全量查證，不推定運行中的最終可改值。

## presence、可改與型別分開

[PageSave](../customer-web-save/transport.md)先檢查mustSend／proxy，再過濾不可改control，之後呼叫ELApplyProxies。payload有某欄位不等於該值會套入：若ELEditable當時拒絕，該值可被忽略；若存檔流程仍繼續，callee讀的是保留的proxy值。完整頁面與其他閘仍須一起看。

KindOf把TCustomEdit分為PK::Text，FieldOf對應`text`，TypeOk檢查cJSON string。ELApplyProxies先收集並檢查待套用項，bad非空時拒絕；通過後才把字串寫到TCustomEdit.Text。這裡的string型別檢查不能證明內容是合法客戶碼、合法整數或在允許範圍內；後續HS_SaveSystemSet仍用atoi轉換，其他前段驗證未全量核對。

本次BeforeApply body重播rgHeaterType、rgTTLCard與rgRotateKit_Type的可改／索引／事件；沒有在這個function中重播edtCustomerCode。這不等於其他callback或其他階段不會再改客戶碼。
