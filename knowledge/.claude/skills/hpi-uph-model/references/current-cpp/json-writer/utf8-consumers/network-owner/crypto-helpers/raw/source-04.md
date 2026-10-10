# 完整原文：Base64.h

[上層](../index.md)。固定pin `a5b072aa8e8a48754515a9139661114b7ad49657`，來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/Base64.h`。
整份39行含原註解／metadata／宣告；[manifest](../source-manifest.json)核全文與function map，context credit0。



```cpp
<!-- preserved-content:start -->
// AI(W906-WebBridge) 20260805: Base64 per RFC 4648 section 4 (the standard
// alphabet, '+' and '/', with '=' padding). Needed for the WebSocket
// Sec-WebSocket-Key / Sec-WebSocket-Accept handshake (RFC 6455 section 4.2.2).
//
// Not URL-safe base64 (RFC 4648 section 5) -- that alphabet is a different
// function and is deliberately not provided, so nobody reaches for this one by
// accident when they need the other.
//
// Layer rule: WebBridge/ must stay independent of VCL / vclcompat.
#ifndef WEBBRIDGE_BASE64_H
#define WEBBRIDGE_BASE64_H

#include <string>
#include <cstddef>

namespace webbridge {

// Encodes arbitrary bytes. Always emits canonical '=' padding, so output length
// is always a multiple of 4. Empty input -> empty output.
std::string Base64Encode(const std::string& data);
std::string Base64Encode(const unsigned char* data, std::size_t len);

// Decodes standard-alphabet base64.
//
// Strictness (this matters for the handshake: a malformed Sec-WebSocket-Key
// must be rejected, not silently coerced):
//   * ASCII whitespace (space, \t, \r, \n, \f, \v) is skipped -- base64 in HTTP
//     headers and MIME bodies is legitimately line-wrapped.
//   * Any other character outside the alphabet -> failure.
//   * Input length (after whitespace removal) not a multiple of 4 -> failure.
//   * '=' is only accepted in the final quantum, at most twice, and only as the
//     last characters -> otherwise failure.
// On failure returns false and leaves *out untouched. On success *out holds the
// decoded bytes (which may contain NUL and non-UTF-8 bytes).
bool Base64Decode(const std::string& text, std::string* out);

} // namespace webbridge

#endif // WEBBRIDGE_BASE64_H

<!-- preserved-content:end -->
```
