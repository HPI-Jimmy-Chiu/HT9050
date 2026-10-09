# 原文：WebBridgeServer::Impl::PumpSnapshot（1）

[上層](../index.md)。來源`HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin`d95fc5fc504d4e2b93a68f754874a72cf1166471`；本頁payload SHA256 `bc281f5d625dd40dc36f82dab03d1e231a67d770b06c0f5a43f294befa42fb3d`。
完整函式的完成數記於manifest；拆頁與context均不另計。原comment／metadata保存，本輪未執行程式。

```cpp
<!-- preserved-content:start -->
// -----------------------------------------------------------------------------
//  Snapshot -> per-connection patch frames.
// -----------------------------------------------------------------------------
void WebBridgeServer::Impl::PumpSnapshot(bool force)
{
    const unsigned long long gen = sib::SnapGeneration(snapshot);
    if (!force && gen == lastGen_) return;
    lastGen_ = gen;

    if (conns_.empty()) return;

    //AI(W906-WSFANOUT) 20260926: EastSun "遮 CW 極限 3 秒才顯示" -- measured: the socket thread at 100% of a core, every HTTP request
    //  waiting ~0.7 s for its first byte. This loop diffed ~6,400 tags and COPIED the whole map once PER WS CONNECTION (14 HMI
    //  windows) on every generation (~5/s: the 1203 Poll republishes). Same frames as before, computed once per BASELINE instead:
    //  lastSent is a shared immutable map, so connections in step share one pointer, one diff, one encoded frame, and `= cur` is O(1).
    std::shared_ptr<std::map<std::string, TagValue> > cur(new std::map<std::string, TagValue>());
    const std::chrono::steady_clock::time_point pumpT0 = std::chrono::steady_clock::now();  sib::SnapRead(snapshot, *cur);   //AI(W906-STREAM-S1) 20260930: time copy + diff + encode of one generation (added after the loop)
    std::vector<std::pair<std::shared_ptr<const std::map<std::string, TagValue> >, std::string> > frames;   // baseline -> frame ("" = no change)
    const std::map<std::string, TagValue> kEmpty;
    for (size_t i = 0; i < conns_.size(); ++i) {
        Conn& c = conns_[i];
        if (!c.isWs || !c.sentSnapshot || c.closeAfterFlush) continue;
        size_t f = 0;  while (f < frames.size() && frames[f].first != c.lastSent) ++f;   // held in `frames`, so a baseline cannot be freed and its address reused
        if (f == frames.size()) {
            // Deltas per BASELINE: a client that joined mid-run has a different baseline from one that has been watching for an hour.
            const std::map<std::string, TagValue>& base = c.lastSent ? *c.lastSent : kEmpty;
            std::map<std::string, TagValue> delta;
            for (std::map<std::string, TagValue>::const_iterator it = cur->begin(); it != cur->end(); ++it) {
                std::map<std::string, TagValue>::const_iterator prev = base.find(it->first);
                if (prev == base.end() || !sib::ValuesEqual(prev->second, it->second)) delta[it->first] = it->second;
            }
            // A tag that disappeared becomes null == "unknown / not installed" (ARCHITECTURE.md section 4 rule 3).
            for (std::map<std::string, TagValue>::const_iterator it = base.begin(); it != base.end(); ++it)
                if (cur->find(it->first) == cur->end()) delta[it->first] = sib::MakeNull();
            frames.push_back(std::make_pair(c.lastSent, delta.empty() ? std::string() : "{\"type\":\"patch\",\"data\":" + sib::ObjectFrom(delta) + "}"));
        }
        if (frames[f].second.empty()) continue;
        SendJson(c, frames[f].second);
        c.lastSent = cur;
        WbGuard sl(statsMx);
        ++stats.patchesSent;  stats.patchBytes += frames[f].second.size();   //AI(W906-STREAM-S1) 20260930
    }  { WbGuard sp(statsMx); ++stats.pumpRuns; stats.pumpUs += (unsigned long long)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - pumpT0).count(); stats.pumpTags = cur->size(); }   //AI(W906-STREAM-S1) 20260930: one generation diffed (also when no connection had a change)
}


<!-- preserved-content:end -->
```
