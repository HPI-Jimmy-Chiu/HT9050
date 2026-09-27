// ===========================================================================
//  ContactForceLoad.h  --  AI(W906-P2a-CF) 20260919
//
//  ContactForce.cpp 那個「純計算核心」的**驅動**。
//
//  為什麼是另一個檔而不是塞進 ContactForce.cpp
//  ---------------------------------------------------------------------------
//  `ContactForce.h:1-16` 自己把範圍鎖死了：
//      Translation scope: CALC CORE ONLY. …  every global-config reference
//      are NOT translated here; they belong to a UI layer that depends on VCL.
//  而這支驅動必然要碰 `CosFunction` / `CUSTOMER_CODE` / `EP_Install` /
//  `INSTALL_DOUBLE_EP` 與磁碟 IO。塞進去就是把那條分界抹掉。
//  ⇒ 分成兩個 TU，計算核心維持它的「零組態相依」性質。
//
//  它補的是什麼洞
//  ---------------------------------------------------------------------------
//  20260919 量到：`ContactForceTables()` 與四支 `Load*SlkTable()`
//  **全樹呼叫點都是 0**。四張表永遠是空的、`bLoaded` 永遠 false。
//  `ContactForce.h:319` 自己的警告說得很清楚：
//      it starts EMPTY … A caller that reads it without checking bLoaded is
//      reproducing exactly the silent-30.0 defect this wave exists to prevent.
//  缺的不是 accessor（那個早就公開了，§C11.3 說「外面拿不到」是過期敘述），
//  是**去讀 ini 並呼叫那四支的驅動**。這個檔就是那個驅動。
//
//  golden 的對應
//  ---------------------------------------------------------------------------
//      ctor      ContactForce.cpp:421-652   四張表的填充 + CSV 來源
//      ReadFile  ContactForce.cpp:963-1178  每一筆的 LoadRate / Offset
//  兩者都只取**資料半邊**：widget 指派、TrackBar Position、TabVisible
//  一律不翻（那是 UI 層，`ContactForce.h` 已經把它們做成回傳值或另一支
//  純函式，例如 `SlkTrackBarPosition()`）。
// ===========================================================================
#ifndef CONTACTFORCELOAD_H
#define CONTACTFORCELOAD_H

// ---------------------------------------------------------------------------
// LoadContactForceTables
//   把 `ContactForceTables()` 的四張表從 `D:\HT9045\system\ContactInfo.ini`
//   載入（golden ContactForce.cpp:428 的 FileName）。
//
//   可以重複呼叫：每次都會先 Clear()，所以不會累加。
//
//   golden 的兩個前置條件照翻：
//     * `CosFunction.bUseDynamicKitDiameter==false` -> 什麼都不做（golden :430）
//     * 檔案不存在 -> 表仍然照 CSV 預設值建起來，但**不套每筆的 ini 值**
//       （golden :1035 `bHasFile==false` 就 return），而 golden 的 else 是
//       `WriteFile()` —— 寫檔屬於延後的那一半，見 .cpp 的 GATE。
//
//   //AI(W906-FRW-S57) 20260926: 同一次也照 golden 讀 Gerneral.ini（INIFileGeneral 有開才讀）：
//     * 建構子 :542-544 的 EP_MAXKPA／EP_MAXA／EP_MINMPA（不看 EP_Install）＋ ReadFile :983-1012 的 12 鍵（EP_Install!=0）；
//     * ReadFile :1014-1036 的 dIndexZOffset[3][15]（[0..1] 讀 [Test Arm]、夾 0～10；[2] 是固定階梯）—— 0925 盤點 P4。
//     缺鍵照 golden 補寫預設值。本頁（Setup.ContactForce，FileRW/ContactForce.cpp）存檔之後也會重跑本函式。
//
//   回傳：四張表裡總共載入幾筆（0 代表沒載到 —— 呼叫端要能分辨
//   「沒載」與「載了但是空的」，所以不要用 void）。
// ---------------------------------------------------------------------------
int LoadContactForceTables();

#endif // CONTACTFORCELOAD_H
