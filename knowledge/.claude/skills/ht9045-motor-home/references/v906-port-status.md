# V906 移植樹（C++）的 fAllMotorHome／回原點現況

舊引用路徑保留；[讀取整理後文件](../../hpi-motor-home/references/flow/generic/references/v906-port-status.md)。

## 1. golden 的規則：開、關 Teach／Motor Test 都把 fAllMotorHome 清掉

[讀取此節](../../hpi-motor-home/references/flow/generic/references/v906-port-status.md#1-golden-的規則開關-teachmotor-test-都把-fallmotorhome-清掉)

## 2. S122：移植樹怎麼知道 Teach／Motor Test 關了（`0b166feb`；RULINGS_20260927 第 2 條第 18 題＝B、R80～R83＝A）

[讀取此節](../../hpi-motor-home/references/flow/generic/references/v906-port-status.md#2-s122移植樹怎麼知道-teachmotor-test-關了0b166februlings_20260927-第-2-條第-18-題br80r83a)

## 3. J1：Teach 頁載入時清旗標，改成只在沒運轉時清（Jimmy `8b5a91b5`，已在 main，St01 `5b9dbc6d` 合進來）

[讀取此節](../../hpi-motor-home/references/flow/generic/references/v906-port-status.md#3-j1teach-頁載入時清旗標改成只在沒運轉時清jimmy-8b5a91b5已在-mainst01-5b9dbc6d-合進來)

## 4. ⚠ 已知風險：所有瀏覽器關掉超過 15 秒，運轉中的機台會安靜停下（回報了、沒改）

[讀取此節](../../hpi-motor-home/references/flow/generic/references/v906-port-status.md#4--已知風險所有瀏覽器關掉超過-15-秒運轉中的機台會安靜停下回報了沒改)

## 5. 相關：單軸回原點的本體已照 golden 翻（Jimmy `23c264b9`，SMHOME）

[讀取此節](../../hpi-motor-home/references/flow/generic/references/v906-port-status.md#5-相關單軸回原點的本體已照-golden-翻jimmy-23c264b9smhome)
