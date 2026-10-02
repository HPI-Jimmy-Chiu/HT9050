# 畫面與 GPIB 共用的 Device 接觸力公式（CalcDeviceForce）

> 版本：`V3.33.906.3`（變更標記 `//AI(ht9045-contact-force) 20260624 (RogerYang)`）
> 行號為 V906.3 約略值，跨版本以符號名稱為準。

## 目的

統一「畫面顯示」與「GPIB 送出」兩路的 Device 接觸力計算，避免兩者數值不一致（此為長期已知痛點：GPIB Force 值與畫面 Force per device 對不上）。

## 共用函式

`cContact.cpp` 新增 `TfContact::CalcDeviceForce()`（約 cContact.cpp:22770）：

```cpp
double TfContact::CalcDeviceForce(double dPinCount, double dForcePerPinN, double dForcePerPinGf, bool bReturnKgf)
{
    if(bReturnKgf)
        return dPinCount * dForcePerPinGf * 0.001;   // gf -> Kgf，與 edForcePerDeviceKG 同源
    else
        return dPinCount * dForcePerPinN;            // N，與 edForcePerDeviceN 同源
}
```

## 呼叫端（兩路共用同一函式）

| 路徑 | 位置 | 用法 |
|---|---|---|
| 畫面 | `ShowArmAndDeviceForce()`（cContact.cpp:1904, ~1919-1920）| `dDeviceN = CalcDeviceForce(..., false)`；`dDeviceGf = CalcDeviceForce(..., true)` |
| GPIB | `ReadFile()`（cContact.cpp:609-614，Korea / TSMC_TAINAN / ASE_KaohSiung / SPIL 等回應 `Force?` 的客戶）| `asArmForce1 = asArmForce2 = FormatFloat("0.0000", CalcDeviceForce(iPinCT, ForcePerPinN, ForcePerPinG, true))` |

## 修正重點（Before → After）

GPIB 路舊公式（cContact.cpp:607-608 已註解）為 `iPinCT * ForcePerPinN / 9.8`（由 N 除以 9.8 換 Kgf），與畫面用的 `ForcePerPinG * 0.001`（由 gf 換 Kgf）來源不同，兩者在 ForcePerPinN 與 ForcePerPinG 不嚴格成 9.8 倍時會對不上。

改為兩路都呼叫 `CalcDeviceForce()` → **畫面值與 GPIB 送出值必定一致**。

## 相關符號

`edForcePerDeviceKG` / `edForcePerDeviceN`（畫面欄位）、`asArmForce1/2` → `WriteArmForce()` → `MSG_CMD_Force`（GPIB `Force?`）、`DeviceForm_File.iPinCT` / `ForcePerPinN` / `ForcePerPinG`。
