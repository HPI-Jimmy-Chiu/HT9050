# 完整原文：Base64.cpp

[上層](../index.md)。固定pin `a5b072aa8e8a48754515a9139661114b7ad49657`，來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/Base64.cpp`。
整份148行含原註解／metadata／宣告；[manifest](../source-manifest.json)核全文與function map，context credit0。

<a id="decodechar"></a> `int DecodeChar(unsigned char c)`。
<a id="encode-bytes"></a> `std::string Base64Encode(const unsigned char* data, std::size_t len)`。
<a id="encode-string"></a> `std::string Base64Encode(const std::string& data)`。
<a id="decode"></a> `bool Base64Decode(const std::string& text, std::string* out)`。

```cpp
<!-- preserved-content:start -->
// AI(W906-WebBridge) 20260805: Base64 encode/decode, RFC 4648 section 4.
#include "WebBridge/Base64.h"

namespace webbridge {

namespace {

const char kAlphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

// -1 = not in alphabet, -2 = skippable whitespace, -3 = '=' pad.
const int kInvalid    = -1;
const int kWhitespace = -2;
const int kPad        = -3;

int DecodeChar(unsigned char c) {
    if (c >= 'A' && c <= 'Z') { return c - 'A'; }
    if (c >= 'a' && c <= 'z') { return c - 'a' + 26; }
    if (c >= '0' && c <= '9') { return c - '0' + 52; }
    if (c == '+') { return 62; }
    if (c == '/') { return 63; }
    if (c == '=') { return kPad; }
    if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v') {
        return kWhitespace;
    }
    return kInvalid;
}

} // namespace

std::string Base64Encode(const unsigned char* data, std::size_t len) {
    std::string out;
    if (data == 0 || len == 0) {
        return out;
    }
    out.reserve(((len + 2) / 3) * 4);

    std::size_t i = 0;
    while (i + 3 <= len) {
        const unsigned int v = (static_cast<unsigned int>(data[i]) << 16)
                             | (static_cast<unsigned int>(data[i + 1]) << 8)
                             | (static_cast<unsigned int>(data[i + 2]));
        out.push_back(kAlphabet[(v >> 18) & 0x3F]);
        out.push_back(kAlphabet[(v >> 12) & 0x3F]);
        out.push_back(kAlphabet[(v >> 6) & 0x3F]);
        out.push_back(kAlphabet[v & 0x3F]);
        i += 3;
    }

    const std::size_t rem = len - i;
    if (rem == 1) {
        const unsigned int v = static_cast<unsigned int>(data[i]) << 16;
        out.push_back(kAlphabet[(v >> 18) & 0x3F]);
        out.push_back(kAlphabet[(v >> 12) & 0x3F]);
        out.push_back('=');
        out.push_back('=');
    } else if (rem == 2) {
        const unsigned int v = (static_cast<unsigned int>(data[i]) << 16)
                             | (static_cast<unsigned int>(data[i + 1]) << 8);
        out.push_back(kAlphabet[(v >> 18) & 0x3F]);
        out.push_back(kAlphabet[(v >> 12) & 0x3F]);
        out.push_back(kAlphabet[(v >> 6) & 0x3F]);
        out.push_back('=');
    }
    return out;
}

std::string Base64Encode(const std::string& data) {
    if (data.empty()) {
        return std::string();
    }
    return Base64Encode(reinterpret_cast<const unsigned char*>(data.data()),
                        data.size());
}

bool Base64Decode(const std::string& text, std::string* out) {
    if (out == 0) {
        return false;
    }

    // Pass 1: strip whitespace, reject stray characters, and locate padding.
    std::string clean;
    clean.reserve(text.size());
    for (std::size_t i = 0; i < text.size(); ++i) {
        const int v = DecodeChar(static_cast<unsigned char>(text[i]));
        if (v == kInvalid) {
            return false;
        }
        if (v == kWhitespace) {
            continue;
        }
        clean.push_back(text[i]);
    }

    if (clean.empty()) {
        out->clear();
        return true;
    }
    if ((clean.size() % 4) != 0) {
        return false;
    }

    // Padding may only be the last one or two characters.
    std::size_t padCount = 0;
    while (padCount < 2 && clean.size() > padCount &&
           clean[clean.size() - 1 - padCount] == '=') {
        ++padCount;
    }
    for (std::size_t i = 0; i + padCount < clean.size(); ++i) {
        if (clean[i] == '=') {
            return false;   // '=' before the final quantum
        }
    }

    std::string decoded;
    decoded.reserve((clean.size() / 4) * 3);
    for (std::size_t i = 0; i < clean.size(); i += 4) {
        int q[4];
        for (int j = 0; j < 4; ++j) {
            q[j] = DecodeChar(static_cast<unsigned char>(clean[i + j]));
        }
        const bool lastQuantum = (i + 4 == clean.size());
        // '=' anywhere but the tail of the last quantum was rejected above; here
        // we only have to translate the tail pads to zero bits.
        const int pads = lastQuantum ? static_cast<int>(padCount) : 0;
        for (int j = 0; j < 4; ++j) {
            if (q[j] == kPad) { q[j] = 0; }
        }
        const unsigned int v = (static_cast<unsigned int>(q[0]) << 18)
                             | (static_cast<unsigned int>(q[1]) << 12)
                             | (static_cast<unsigned int>(q[2]) << 6)
                             | (static_cast<unsigned int>(q[3]));
        decoded.push_back(static_cast<char>((v >> 16) & 0xFF));
        if (pads < 2) {
            decoded.push_back(static_cast<char>((v >> 8) & 0xFF));
        }
        if (pads < 1) {
            decoded.push_back(static_cast<char>(v & 0xFF));
        }
    }

    out->swap(decoded);
    return true;
}

} // namespace webbridge

<!-- preserved-content:end -->
```
