# 原文 03／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp`；定位 `W906_TakeCarry`；種類 `complete_cpp_functions`。
來源 commit `5d86796f2cc4a2b1c9c1f3a474dbb4127df085eb`；完整摘錄 SHA256 `c3ca3d79f87d98d82accefa27baca4305b52341b95fbc02b65f606f9e6d799ca`。
原註解、裁決、gate 及歷史 test 敘述保留；歷史驗證不是本輪實測。

```cpp
<!-- preserved-content:start -->
static void W906_TakeCarry(std::vector<webbridge::WebCommand>& out) { for (std::size_t k = 0; k < g_carry.size(); ++k) out.push_back(g_carry[k]); g_carry.clear(); g_carryRunnable = false; }
<!-- preserved-content:end -->
```
