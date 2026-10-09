# 原文：entire TagSnapshot.h including original threading rationale and historical test comments; no inline-body completion credit（1）

[上層](../index.md)。來源`HT9011UC_Cpp_V3.33.906.0/WebBridge/TagSnapshot.h`、pin`d95fc5fc504d4e2b93a68f754874a72cf1166471`；本頁payload SHA256 `b1e2688e89e89af308b12d240a4a960b4e47451cc7600212b275c3e43a4d22f7`。
完整函式的完成數記於manifest；拆頁與context均不另計。原comment／metadata保存，本輪未執行程式。

```cpp
<!-- preserved-content:start -->
// ===========================================================================
//  WebBridge/TagSnapshot.h
//  The state half of the browser<->handler seam: a double-buffered
//  map<tag,TagValue> the UI thread publishes and socket threads read.
//
//  AI(W906-WebBridge-State) 20260805: new file. NEW-BUILD infrastructure, not
//  a translation of any BCB6 golden source.
//
//  ===========================================================================
//  THREADING CONTRACT -- READ THIS BEFORE CALLING ANYTHING
//  ===========================================================================
//  ARCHITECTURE.md section 5 (D:\HT9045\web\docs\ARCHITECTURE.md):
//
//    "State reaches it through a snapshot the UI thread publishes, not by the
//     server reaching in and reading live objects. A double-buffered
//     map<tag,value> behind one mutex is sufficient; the publisher swaps, the
//     server reads."
//    "Nothing on the socket thread may block on machine I/O. Ever."
//
//  The reason this matters more than usual: the handler's SECS/GEM layer has a
//  30-second host reply budget and it runs on the UI thread. A socket thread
//  that stalls the UI thread stalls a production machine.
//
//  WHO MAY CALL WHAT
//  -----------------
//    UI THREAD ONLY (exactly ONE thread, for the lifetime of the object):
//        beginPublish()
//        stage() / stageFrom()
//        commitPublish()
//        publish()                 (= the three above in one call)
//
//        Rationale: the staging buffer is NOT protected by the mutex. It is
//        owned outright by the publishing thread, which is what makes the
//        publisher's critical section a single O(1) buffer swap instead of an
//        O(n) copy. Two threads staging concurrently is a data race, and no
//        amount of locking inside this class would fix it -- so the class
//        detects the misuse instead (see publisherViolations()).
//
//    ANY THREAD (socket threads, and the UI thread too if it wants):
//        read()
//        generation()
//        diffFrom()
//        publishedTagCount()
//        publisherViolations() / publisherThreadId()
//
//  WHAT THE ONE MUTEX PROTECTS, AND FOR HOW LONG
//  ---------------------------------------------
//  ONE mutex per TagSnapshot, guarding only the front buffer + generation.
//    * commitPublish(): lock -> swap two container objects and increment the
//      generation -> unlock. No allocation, no comparison, no I/O.
//    * read()/diffFrom(): lock -> copy the front buffer + generation -> unlock.
//      The diff comparison itself runs on that private copy, OUTSIDE the lock.
//    * No JSON is ever produced under this lock. The serialiser only ever sees
//      a TagSnapshotView / TagPatch it already owns.
//
//  read() returns a VALUE, never a reference into a buffer that can be swapped
//  underneath the caller. That is deliberate: handing out a const& to the front
//  buffer would be a use-after-swap waiting for the first busy tick.
//
//  windows.h is deliberately NOT included by this header (the lock lives in a
//  pimpl). The socket component includes <winsock2.h>, and a windows.h that
//  arrives first drags in the old winsock.h and produces the classic
//  redefinition storm. Keeping it in the .cpp removes that hazard entirely.
// ===========================================================================
#ifndef WEBBRIDGE_TAGSNAPSHOT_H
#define WEBBRIDGE_TAGSNAPSHOT_H

#include "WebBridge/TagValue.h"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace webbridge {

// The published state: tag name -> value. std::map (not unordered_map) so the
// iteration order is deterministic -- patches and snapshots then serialise
// byte-identically for identical state, which makes wire logs diffable.
typedef std::map<std::string, TagValue> TagMap;

// ---------------------------------------------------------------------------
//  TagSnapshotView -- one consistent, fully-published generation.
//
//  A view is a private copy. `generation` is the count of completed publishes
//  at the moment the copy was taken:
//      0  = nothing has ever been published (tags is empty)
//      n  = the n-th publish, complete
//  A caller must NEVER observe a `tags` that mixes two generations; that is
//  the torn read the mutex + swap exists to prevent, and test_wb_state.cpp
//  asserts it under real concurrent load.
// ---------------------------------------------------------------------------
struct TagSnapshotView {
    TagMap             tags;
    std::uint64_t      generation;

    TagSnapshotView() : tags(), generation(0) {}
};

// ---------------------------------------------------------------------------
//  TagPatch -- what changed between two generations, per connection.
//
//  Section 4 rule 2: send `patch`, not `snapshot`, for updates (~234 tags at
//  10 Hz is pointless traffic).
//
//  `changed`  tags that are new, or whose value differs. Includes the
//             Null -> "" and "" -> Null transitions, which are real changes
//             (section 4 rule 3).
//  `removed`  tags that were present in the peer's last-sent state and are
//             absent now. Kept SEPARATE from `changed` instead of being
//             pre-encoded as null, because "went absent" and "is present and
//             null" are different facts on this side of the seam. The wire has
//             no removal encoding, so the JSON layer maps `removed` to
//             `"tag": null` -- which is exactly right per section 4 rule 3,
//             since a tag that vanished is by definition unknown now.
//  `generation` the snapshot generation the peer reaches once it applies this
//             patch. Store it and hand it back on the next diffFrom() call.
// ---------------------------------------------------------------------------
struct TagPatch {
    TagMap                   changed;
    std::vector<std::string> removed;
    std::uint64_t            generation;

    TagPatch() : changed(), removed(), generation(0) {}

    bool   empty() const { return changed.empty() && removed.empty(); }
    size_t size()  const { return changed.size() + removed.size(); }
};

// ---------------------------------------------------------------------------
//  Pure helpers -- no locking, no shared state, callable from any thread.
//  Factored out of TagSnapshot so the diff SEMANTICS are testable without
//  involving threads at all, and so the locked methods stay trivial.
// ---------------------------------------------------------------------------

// Everything that differs going from `from` to `to`. Does not set .generation.
TagPatch diffMaps(const TagMap& from, const TagMap& to);

// Apply a patch to a peer's last-sent map (also updates nothing else). Lets a
// connection track what it has sent without taking a second full copy.
void applyPatch(TagMap& target, const TagPatch& patch);

// ---------------------------------------------------------------------------
//  TagSnapshot
// ---------------------------------------------------------------------------
class TagSnapshot {
public:
    TagSnapshot();
    ~TagSnapshot();

    // =====================================================================
    //  PUBLISH PATH -- UI THREAD ONLY (one single thread, always the same one)
    // =====================================================================

    // Start a new generation. Clears the staging buffer, so every tick must
    // stage the FULL set of tags it wants visible. That is what makes a tag
    // going away expressible at all: a tag not staged this tick is absent in
    // the next generation, and the next diff reports it in TagPatch::removed.
    void beginPublish();

    // Add/overwrite one tag in the staging buffer. Not visible to readers
    // until commitPublish().
    void stage(const std::string& tag, const TagValue& value);

    // Bulk form of stage(). Existing staged tags with other names are kept.
    void stageFrom(const TagMap& tags);

    // Swap staging -> front and bump the generation. THE critical section: one
    // container swap and one increment, nothing else. Returns the new
    // generation (>= 1).
    std::uint64_t commitPublish();

    // beginPublish() + stageFrom(tags) + commitPublish(). The normal call for
    // a UI timer tick that already has the whole tag set in hand.
    std::uint64_t publish(const TagMap& tags);

    // How many tags the staging buffer currently holds (publisher-side only;
    // no lock, because only the publisher may touch staging).
    size_t stagedTagCount() const;

    // =====================================================================
    //  READ PATH -- ANY THREAD
    // =====================================================================

    // A consistent private copy of the current generation.
    TagSnapshotView read() const;

    // Current generation without copying the map. 0 = nothing published yet.
    std::uint64_t generation() const;

    // Number of tags in the published front buffer.
    size_t publishedTagCount() const;

    // Diff the current generation against what this connection last sent.
    //
    // Fast path: if the generation has not advanced past lastSent.generation,
    // nothing can have changed, so this returns an empty patch after reading
    // only the generation -- no map copy, no comparison. Worth having: with N
    // connections polling faster than the UI tick, this is the common case.
    TagPatch diffFrom(const TagSnapshotView& lastSent) const;

    // Same, for a caller that tracks only the map and not the generation.
    // Always copies and always compares (no fast path available).
    TagPatch diffFrom(const TagMap& lastSent) const;

    // =====================================================================
    //  MISUSE DETECTION -- ANY THREAD
    // =====================================================================
    //  The first thread to enter the publish path is recorded as THE publisher.
    //  Any later publish-path call from a different thread increments
    //  publisherViolations(). It does NOT abort, throw or lock the machine out:
    //  this code sits in a running handler, and killing the process over a
    //  bridge bug would be worse than the bug. It is a loud, testable counter
    //  the bridge is expected to log and surface.
    //
    //  Note this is detection, not protection -- if it ever fires, the staging
    //  buffer has already been raced on. Treat a non-zero count as a defect in
    //  the caller, not as a condition to handle.
    // =====================================================================
    std::uint64_t publisherViolations() const;

<!-- preserved-content:end -->
```
