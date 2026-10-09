# 原文 27：cpp original intro, includes, namespace and GUID

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsHandshake.cpp`；以 `cpp original intro, includes, namespace and GUID` 定位。
pin `7bf6ff28e23bbfeb6f94c1751d63ae62533313f4`；SHA256 `73be29156963c4bf54f4eda5b325b21b58bebae28ed8c67ce8a39389e88ef7f4`。
原函式／相鄰註解或header context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
// ===========================================================================
//  WebBridge/WsHandshake.cpp
//
//  See WsHandshake.h for the contract. Implementation notes are inline; the
//  ones worth reading before changing anything:
//
//   * The header block is located FIRST (search for CRLFCRLF) and only then
//     split into lines. That ordering is what makes the size cap effective:
//     until the terminator is seen, nothing is parsed and nothing is copied.
//   * obs-fold (RFC 7230 section 3.2.4) continuation lines are unfolded into
//     the previous header's value as a single space. Modern clients never send
//     them, but a proxy in front of the machine PC still might, and silently
//     mis-parsing a folded Connection header would break the handshake in a
//     way that is very hard to see from the browser side.
//   * Header names are compared case-insensitively because RFC 7230 says they
//     are case-insensitive, and real clients disagree about the casing of
//     "Sec-WebSocket-Key" in particular.
// ===========================================================================
#include "WsHandshake.h"

#include "Sha1.h"
#include "Base64.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace webbridge {

const char* const kWebSocketGuid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";


<!-- preserved-content:end -->
```
