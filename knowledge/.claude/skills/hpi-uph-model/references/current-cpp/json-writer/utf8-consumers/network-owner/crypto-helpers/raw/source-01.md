# 完整原文：Sha1.cpp

[上層](../index.md)。固定pin `a5b072aa8e8a48754515a9139661114b7ad49657`，來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/Sha1.cpp`。
整份175行含原註解／metadata／宣告；[manifest](../source-manifest.json)核全文與function map，context credit0。

<a id="constructor"></a> `Sha1::Sha1()`。
<a id="reset"></a> `void Sha1::Reset()`。
<a id="processblock"></a> `void Sha1::ProcessBlock(const unsigned char* block)`。
<a id="update-bytes"></a> `void Sha1::Update(const unsigned char* data, std::size_t len)`。
<a id="update-string"></a> `void Sha1::Update(const std::string& data)`。
<a id="final"></a> `std::string Sha1::Final()`。
<a id="sha1raw"></a> `std::string Sha1Raw(const std::string& data)`。
<a id="tohex"></a> `std::string ToHex(const std::string& bytes)`。
<a id="sha1hex"></a> `std::string Sha1Hex(const std::string& data)`。

```cpp
<!-- preserved-content:start -->
// AI(W906-WebBridge) 20260805: SHA-1 per RFC 3174, method 1 (section 6.1).
#include "WebBridge/Sha1.h"

namespace webbridge {

namespace {

inline unsigned int Rol(unsigned int value, unsigned int bits) {
    return (value << bits) | (value >> (32u - bits));
}

} // namespace

Sha1::Sha1() {
    Reset();
}

void Sha1::Reset() {
    // RFC 3174 section 6.1: initial H0..H4.
    h_[0] = 0x67452301u;
    h_[1] = 0xEFCDAB89u;
    h_[2] = 0x98BADCFEu;
    h_[3] = 0x10325476u;
    h_[4] = 0xC3D2E1F0u;
    bufLen_    = 0;
    bitCount_  = 0;
    finalized_ = false;
    digest_.clear();
}

void Sha1::ProcessBlock(const unsigned char* block) {
    unsigned int w[80];

    // Big-endian word expansion (RFC 3174 section 6.1 step a/b).
    for (int t = 0; t < 16; ++t) {
        w[t] = (static_cast<unsigned int>(block[t * 4 + 0]) << 24)
             | (static_cast<unsigned int>(block[t * 4 + 1]) << 16)
             | (static_cast<unsigned int>(block[t * 4 + 2]) << 8)
             | (static_cast<unsigned int>(block[t * 4 + 3]));
    }
    for (int t = 16; t < 80; ++t) {
        w[t] = Rol(w[t - 3] ^ w[t - 8] ^ w[t - 14] ^ w[t - 16], 1);
    }

    unsigned int a = h_[0], b = h_[1], c = h_[2], d = h_[3], e = h_[4];

    for (int t = 0; t < 80; ++t) {
        unsigned int f, k;
        if (t < 20) {
            f = (b & c) | ((~b) & d);
            k = 0x5A827999u;
        } else if (t < 40) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1u;
        } else if (t < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDCu;
        } else {
            f = b ^ c ^ d;
            k = 0xCA62C1D6u;
        }
        const unsigned int temp = Rol(a, 5) + f + e + w[t] + k;
        e = d;
        d = c;
        c = Rol(b, 30);
        b = a;
        a = temp;
    }

    h_[0] += a;
    h_[1] += b;
    h_[2] += c;
    h_[3] += d;
    h_[4] += e;
}

void Sha1::Update(const unsigned char* data, std::size_t len) {
    if (finalized_ || data == 0 || len == 0) {
        return;
    }
    bitCount_ += static_cast<unsigned long long>(len) * 8ull;

    std::size_t offset = 0;
    if (bufLen_ > 0) {
        const std::size_t need = 64 - bufLen_;
        const std::size_t take = (len < need) ? len : need;
        for (std::size_t i = 0; i < take; ++i) {
            buf_[bufLen_ + i] = data[i];
        }
        bufLen_ += take;
        offset  += take;
        if (bufLen_ < 64) {
            return;
        }
        ProcessBlock(buf_);
        bufLen_ = 0;
    }
    while (offset + 64 <= len) {
        ProcessBlock(data + offset);
        offset += 64;
    }
    for (std::size_t i = offset; i < len; ++i) {
        buf_[bufLen_++] = data[i];
    }
}

void Sha1::Update(const std::string& data) {
    if (data.empty()) {
        return;
    }
    Update(reinterpret_cast<const unsigned char*>(data.data()), data.size());
}

std::string Sha1::Final() {
    if (finalized_) {
        return digest_;
    }

    // RFC 3174 section 4: append 0x80, pad with zeros to 56 mod 64, then the
    // 64-bit big-endian bit length.
    // Padding bytes deliberately bypass Update() so they do not count toward
    // bitCount_ (the length field must describe the message, not the padding).
    const unsigned long long bits = bitCount_;
    buf_[bufLen_++] = 0x80;
    if (bufLen_ == 64) {
        ProcessBlock(buf_);
        bufLen_ = 0;
    }
    while (bufLen_ != 56) {
        buf_[bufLen_++] = 0x00;
        if (bufLen_ == 64) {
            ProcessBlock(buf_);
            bufLen_ = 0;
        }
    }
    for (int i = 7; i >= 0; --i) {
        buf_[bufLen_++] = static_cast<unsigned char>((bits >> (i * 8)) & 0xFFull);
    }
    ProcessBlock(buf_);
    bufLen_ = 0;

    digest_.resize(kSha1DigestBytes);
    for (int i = 0; i < 5; ++i) {
        digest_[i * 4 + 0] = static_cast<char>((h_[i] >> 24) & 0xFFu);
        digest_[i * 4 + 1] = static_cast<char>((h_[i] >> 16) & 0xFFu);
        digest_[i * 4 + 2] = static_cast<char>((h_[i] >> 8) & 0xFFu);
        digest_[i * 4 + 3] = static_cast<char>((h_[i]) & 0xFFu);
    }
    finalized_ = true;
    return digest_;
}

std::string Sha1Raw(const std::string& data) {
    Sha1 h;
    h.Update(data);
    return h.Final();
}

std::string ToHex(const std::string& bytes) {
    static const char kHex[] = "0123456789abcdef";
    std::string out;
    out.reserve(bytes.size() * 2);
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        const unsigned char b = static_cast<unsigned char>(bytes[i]);
        out.push_back(kHex[(b >> 4) & 0x0F]);
        out.push_back(kHex[b & 0x0F]);
    }
    return out;
}

std::string Sha1Hex(const std::string& data) {
    return ToHex(Sha1Raw(data));
}

} // namespace webbridge

<!-- preserved-content:end -->
```
