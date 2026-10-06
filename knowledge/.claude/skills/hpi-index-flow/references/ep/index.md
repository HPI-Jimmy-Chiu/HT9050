# EP 壓力回授分支

## 差異與版本

原版 D24／D26 文件描述特定舊版完整流程。V906 `CheckAndRecodrEP()` 的完整舊碼目前位於 `#if 0`，live body 忽略參數並回傳 false；因此不能用原文的 D26_1 開關說明推定 V906 會報 alarm。其他 EP helper 的 gate／接線仍需按目前呼叫鏈核對。

## 入口與流程

- [舊 contact-force 第6節完整原文](pressure-feedback.md)
- [D24／D26 詳細錨點、狀態機、單位與英文回覆](EP-check-D24-D26.md)
- EP 只整合到本 Index 主題；GPIB Force 命令與力量計算仍由 [contact-force 原入口](../../../ht9045-contact-force/SKILL.md) 維護。

## 安全

原版記載 `iD26EPEncoderRange` 是 kPa，而不是 encoder count；D24 借用 D26 容差，D24 會增加 cycle time。這些是原版語意，設定與 alarm 是否有效必須核對執行版本。不因整理文件而改機台設定。

## 查證來源

V906 main 的 `atester.cpp` 中 GOLDEN VERBATIM／slim live 配對；原文保留舊版 source 行號與各客戶條件。
