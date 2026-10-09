# 原文：WebBridgeServer::Impl::AckJson（1）

[上層](../index.md)。來源`HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin`d95fc5fc504d4e2b93a68f754874a72cf1166471`；本頁payload SHA256 `f8fc3df61c41116d959ba748695a467464069888d91e06cc63397e55e7a0f5fb`。
完整函式的完成數記於manifest；拆頁與context均不另計。原comment／metadata保存，本輪未執行程式。

```cpp
<!-- preserved-content:start -->
std::string WebBridgeServer::Impl::AckJson(double id, bool ok, const std::string& error)
{
    std::ostringstream os;
    os << "{\"type\":\"ack\",\"id\":";
    // Ids are integers on the wire (js/transport/ws.js counts 1,2,3...).
    const long long i = static_cast<long long>(id);
    if (static_cast<double>(i) == id) os << i; else os << id;
    os << ",\"ok\":" << (ok ? "true" : "false");
    if (!ok) {
        os << ",\"error\":" << sib::QuoteString(error);
    } else if (error.size() >= 2u && error[0] == '{' && error[error.size() - 1u] == '}') {
        //Steven 20260916
        // The third parameter is named `error`, but on SUCCESS a handler may pass
        // a JSON object instead -- the command's result. It used to be dropped
        // here, and that silently disabled the browser's whole write contract:
        //
        //   * wb_serve's system.file.put builds {"changed":N,"identical":N,
        //     "notFound":N} and hands it to CompleteCommand(). It never arrived,
        //     so every ack the browser saw was a bare {"type":"ack","ok":true}.
        //   * ht9045_wire_engine.js refuses to write when preview reports any
        //     notFound (rule 2, "the mapping is wrong, not the data"), and asks
        //     the operator to confirm the `changed` list. With both fields
        //     absent it read notFound as empty -- rule 2 could never fire -- and
        //     changed as empty, so save() always stopped at "nothing changed,
        //     not writing". No page could save anything, and nothing reported an
        //     error: the operator pressed Save and got a green message.
        //
        // Splice the object inline (drop its braces) rather than nesting it under
        // a "result" key, because that is the shape the client already reads
        // (p.changed / p.notFound straight off the ack).
        //
        // Every other call site passes an empty string on success and is
        // unaffected. A non-empty success string that is NOT a JSON object is
        // also left out, exactly as before -- this only ever adds fields that a
        // handler deliberately built.
        const std::string inner = error.substr(1u, error.size() - 2u);
        if (!inner.empty()) os << "," << inner;
    }
    os << "}";
    return os.str();
}


<!-- preserved-content:end -->
```
