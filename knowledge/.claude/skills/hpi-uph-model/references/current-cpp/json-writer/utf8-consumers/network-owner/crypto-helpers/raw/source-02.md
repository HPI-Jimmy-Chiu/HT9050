# 完整原文：Sha1.h

[上層](../index.md)。固定pin `a5b072aa8e8a48754515a9139661114b7ad49657`，來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/Sha1.h`。
整份61行含原註解／metadata／宣告；[manifest](../source-manifest.json)核全文與function map，context credit0。



```cpp
<!-- preserved-content:start -->
// AI(W906-WebBridge) 20260805: SHA-1 (RFC 3174) for the WebSocket handshake.
//
// Scope: this exists ONLY to compute the Sec-WebSocket-Accept value required by
// RFC 6455 section 4.2.2 (SHA-1 of the client key concatenated with the GUID,
// then base64). SHA-1 is cryptographically broken for signatures; it is used
// here because the WebSocket standard mandates it, not as a security primitive.
// Do not use this for anything that needs collision resistance.
//
// Layer rule: WebBridge/ must stay independent of VCL / vclcompat. Standard
// headers only.
#ifndef WEBBRIDGE_SHA1_H
#define WEBBRIDGE_SHA1_H

#include <string>
#include <cstddef>

namespace webbridge {

// Length of a SHA-1 digest in raw bytes (RFC 3174 section 1: 160 bits).
const std::size_t kSha1DigestBytes = 20;

// Streaming SHA-1. Usage:
//   Sha1 h; h.Update(a); h.Update(b); std::string d = h.Final();
// Final() is idempotent: it may be called repeatedly and always returns the
// digest of everything fed in up to the first Final() call. Reset() restarts.
class Sha1 {
public:
    Sha1();

    void Reset();

    void Update(const unsigned char* data, std::size_t len);
    void Update(const std::string& data);

    // Returns the 20 raw digest bytes as a std::string (which is a byte
    // container here, not text -- it will contain NUL and non-UTF-8 bytes).
    std::string Final();

private:
    void ProcessBlock(const unsigned char* block);

    unsigned int   h_[5];        // intermediate hash state H0..H4
    unsigned char  buf_[64];     // partial 512-bit block
    std::size_t    bufLen_;      // bytes currently in buf_
    unsigned long long bitCount_;// total message length in bits
    bool           finalized_;
    std::string    digest_;      // cached result once finalized_
};

// One-shot convenience: 20 raw bytes.
std::string Sha1Raw(const std::string& data);

// Lowercase hex of the 20-byte digest (40 characters).
std::string Sha1Hex(const std::string& data);

// Lowercase hex of an arbitrary byte string (helper, used by tests and logs).
std::string ToHex(const std::string& bytes);

} // namespace webbridge

#endif // WEBBRIDGE_SHA1_H

<!-- preserved-content:end -->
```
