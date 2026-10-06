# 原生六頁：方案與實作沿革

- [完整原入口](original-entry.md)：20260928評估、Q53～Q55與原型狀態，帶原日期解讀。
- [native forms plan](references/native-forms-plan.md)、[page inventory](references/page-inventory.md)、[checklist](references/checklist.md)。
- [可重用工具](references/reusable-tools.md)、[名詞](references/glossary.md)、[原型20260928](references/prototype-20260928.md)。
- [已否決的早期解讀](references/evaluation-20260928.md)：保留對照，不改稱當前產品方案。

原文含提案、裁定、原型分支與後續計畫，不由頁面表或腳本存在推成六頁全部已上機。Q54已裁定不用執行期NativeForms.ini，舊提案保留歷史；查目前編譯期W906_NATIVE_FORMS及ui/native實檔，再查每頁動作與互鎖caller。畫面20ms、server pump／IO／tag節拍各自定位，不能把畫面目標套成資料端現況。

IO／Motor／Teach／Home／Shuttle本體與機型差異轉讀對應共用Skill。這次沒有改native程式、建置、執行原型或申請硬體測試權限。
