# 原文 05：TryOneFrame

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsFrame.cpp`；以 `TryOneFrame` 定位。
固定來源 `fd50c7d93d928a1ed48d9cbf5b668e39d471c758`；SHA256 `e9bc1ae5a82f40e8fffacbc1be5bf23a3e2e6f1e0e46f819be755c5c86aed1ab`。
完整函式。保留原註解；未執行程式或測試。

```cpp
<!-- preserved-content:start -->
int WsDecoder::TryOneFrame(std::vector<WsMessage>* out) {
    if (buf_.size() < 2) return 0;
    const unsigned char* p = reinterpret_cast<const unsigned char*>(buf_.data());

    const bool fin = (p[0] & 0x80) != 0;
    const unsigned rsv = static_cast<unsigned>(p[0] & 0x70);
    const int opcode = p[0] & 0x0F;
    const bool masked = (p[1] & 0x80) != 0;

    uint64_t payLen = static_cast<uint64_t>(p[1] & 0x7F);
    std::size_t hdr = 2;

    // --- the three payload length forms ------------------------------------
    if (payLen == 126) {
        if (buf_.size() < 4) return 0;
        payLen = (static_cast<uint64_t>(p[2]) << 8) | static_cast<uint64_t>(p[3]);
        hdr = 4;
    } else if (payLen == 127) {
        if (buf_.size() < 10) return 0;
        uint64_t v = 0;
        for (int i = 0; i < 8; ++i) {
            v = (v << 8) | static_cast<uint64_t>(p[2 + i]);
        }
        // RFC 6455 section 5.2: "the most significant bit MUST be 0". A peer
        // setting it is either broken or probing; either way, refuse.
        if ((v & 0x8000000000000000ULL) != 0) {
            Fail(kWsCloseProtocolError, "64-bit payload length has the MSB set");
            return -1;
        }
        payLen = v;
        hdr = 10;
    }
    if (masked) hdr += 4;

    // --- checks that need only the header ----------------------------------

    // RSV1..3 must be zero: the handshake negotiates no extension, so there is
    // nothing that could give those bits a meaning.
    if (rsv != 0) {
        Fail(kWsCloseProtocolError, "RSV bits set with no extension negotiated");
        return -1;
    }

    const bool isControl = (opcode & 0x08) != 0;

    // Reserved opcodes: data 0x3-0x7 and control 0xB-0xF are undefined.
    if (opcode != kWsContinuation && opcode != kWsText && opcode != kWsBinary &&
        opcode != kWsClose && opcode != kWsPing && opcode != kWsPong) {
        Fail(kWsCloseProtocolError, "reserved opcode");
        return -1;
    }

    if (isControl) {
        // RFC 6455 section 5.5: control frames MUST NOT be fragmented and MUST
        // have a payload of 125 bytes or less. Note this is checked before the
        // payload is waited for, so a control frame claiming 2^40 bytes is
        // rejected instantly instead of buffering.
        if (!fin) {
            Fail(kWsCloseProtocolError, "fragmented control frame");
            return -1;
        }
        if (payLen > kWsMaxControlPayload) {
            Fail(kWsCloseProtocolError, "control frame payload exceeds 125 bytes");
            return -1;
        }
    }

    // --- masking DIRECTION enforcement (RFC 6455 sections 5.1 and 5.3) -----
    // A server MUST close the connection on receiving an unmasked frame, and a
    // client MUST close on receiving a masked one. This is not cosmetic: the
    // mask exists to defeat cache-poisoning of intermediaries, so accepting
    // unmasked client frames would quietly remove that property.
    if (masked != requireMask_) {
        Fail(kWsCloseProtocolError,
             requireMask_ ? "client frame is not masked"
                          : "server frame is masked");
        return -1;
    }

    // --- fragmentation state machine (RFC 6455 section 5.4) ----------------
    if (!isControl) {
        if (opcode == kWsContinuation) {
            if (fragOpcode_ == 0) {
                Fail(kWsCloseProtocolError,
                     "continuation frame with no message in progress");
                return -1;
            }
        } else {
            if (fragOpcode_ != 0) {
                Fail(kWsCloseProtocolError,
                     "new data frame while a fragmented message is in progress");
                return -1;
            }
        }
    }
    // Control frames deliberately fall through all of the above untouched:
    // they MAY be interleaved between the fragments of a data message and must
    // not disturb the reassembly buffer.

    // --- maximum message size, enforced BEFORE buffering the payload -------
    // This is the check that stops a hostile or buggy client from exhausting
    // the machine PC's memory: the 64-bit length form can announce 2^63 bytes,
    // and a decoder that waits for them has already lost. Exceeding the cap is
    // a 1009 close (RFC 6455 section 7.4.1, "Message Too Big").
    if (!isControl) {
        const uint64_t base = (opcode == kWsContinuation)
                                  ? static_cast<uint64_t>(frag_.size())
                                  : 0u;
        if (base + payLen > static_cast<uint64_t>(maxMessage_)) {
            Fail(kWsCloseMessageTooBig, "message exceeds the maximum size");
            return -1;
        }
    }

    // --- do we have the whole payload yet? ---------------------------------
    // Compared in 64-bit so this cannot wrap on a 32-bit build. Safe to reach
    // here only because payLen is now known to be bounded (<= maxMessage_ for
    // data frames, <= 125 for control frames).
    if (static_cast<uint64_t>(buf_.size()) < static_cast<uint64_t>(hdr) + payLen) {
        return 0;
    }
    const std::size_t n = static_cast<std::size_t>(payLen);

    // --- unmask ------------------------------------------------------------
    std::string payload(buf_.data() + hdr, n);
    if (masked) {
        const unsigned char* k = p + hdr - 4;   // key sits just before the payload
        for (std::size_t i = 0; i < n; ++i) {
            payload[i] = static_cast<char>(
                static_cast<unsigned char>(payload[i]) ^ k[i & 3]);
        }
    }
    buf_.erase(0, hdr + n);

    // --- dispatch ----------------------------------------------------------
    if (isControl) {
        WsMessage m;
        m.opcode = opcode;
        m.payload = payload;
        if (opcode == kWsClose) {
            sawClose_ = true;
            if (n == 1) {
                // RFC 6455 section 5.5.1: if there is a body at all it is at
                // least the 2-byte status code.
                Fail(kWsCloseProtocolError, "close frame with a 1-byte payload");
                return -1;
            }
            if (n >= 2) {
                m.hasCloseCode = true;
                m.closeCode = static_cast<uint16_t>(
                    (static_cast<unsigned char>(payload[0]) << 8) |
                     static_cast<unsigned char>(payload[1]));
                m.closeReason = payload.substr(2);
                // RFC 6455 section 7.4: 1000-1003 and 1007-1011 are defined,
                // 3000-4999 are registered/private. 1004/1005/1006 and
                // anything else must never appear on the wire.
                const uint16_t c = m.closeCode;
                const bool legal = (c >= 1000 && c <= 1003) ||
                                   (c >= 1007 && c <= 1011) ||
                                   (c >= 3000 && c <= 4999);
                if (!legal) {
                    Fail(kWsCloseProtocolError, "illegal close status code");
                    return -1;
                }
                if (validateUtf8_ && !IsValidUtf8(m.closeReason)) {
                    Fail(kWsCloseInvalidPayload, "close reason is not valid UTF-8");
                    return -1;
                }
            }
        }
        out->push_back(m);
        return 1;
    }

    // Data frame.
    if (opcode == kWsContinuation) {
        frag_ += payload;
        if (!fin) return 1;                 // still mid-message
        WsMessage m;
        m.opcode = fragOpcode_;
        m.payload = frag_;
        frag_.clear();
        fragOpcode_ = 0;
        if (validateUtf8_ && m.opcode == kWsText && !IsValidUtf8(m.payload)) {
            Fail(kWsCloseInvalidPayload, "text message is not valid UTF-8");
            return -1;
        }
        out->push_back(m);
        return 1;
    }

    if (!fin) {
        // First fragment of a new message.
        fragOpcode_ = opcode;
        frag_ = payload;
        return 1;
    }

    // Unfragmented single-frame message.
    WsMessage m;
    m.opcode = opcode;
    m.payload = payload;
    if (validateUtf8_ && opcode == kWsText && !IsValidUtf8(m.payload)) {
        // RFC 6455 section 8.1: a text message whose payload is not valid
        // UTF-8 is a 1007 failure, not something to hand upstairs.
        Fail(kWsCloseInvalidPayload, "text message is not valid UTF-8");
        return -1;
    }
    out->push_back(m);
    return 1;
}

<!-- preserved-content:end -->
```
