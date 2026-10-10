# 原文：QueuePush sibling adapter including original connection-id fix（1）

[上層](../index.md)。來源`HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin`34c8e2e164014a93699f0b0a712366f5da01ebd2`；本頁payload SHA256 `f8de10fd2a7b1b2b05ff2e1c1dd04265c9b686a2928b95c76de955a927f9b812`。
完整函式完成數見manifest；context／helper／adapter與拆頁不另計。原comment／metadata保存，本輪未執行程式。

```cpp
<!-- preserved-content:start -->
// --- CommandQueue -----------------------------------------------------------
static bool QueuePush(CommandQueue* q, unsigned long long ticket,
                      const std::string& cmd, const std::string& tag,
                      const TagValue& value, unsigned long long connId)   // AI(W906-CONNID) 20260926: 加 connId（見下面 c.connId）
{
    if (!q) return false;
    WebCommand c;
    c.id       = ticket;
    c.cmd      = cmd;
    c.tag      = tag;
    c.hasTag   = !tag.empty();
    c.value    = value;
    c.hasValue = !value.isNull();  c.connId = connId;   // AI(W906-CONNID) 20260926: 以前從沒設 ⇒ wb_serve 收到的每個指令 connId 都是 0，ui.windows.put 把每個分頁都登記成同一條連線、互相蓋掉（St01 18:35 報）
    return q->tryPush(c);
}


<!-- preserved-content:end -->
```
