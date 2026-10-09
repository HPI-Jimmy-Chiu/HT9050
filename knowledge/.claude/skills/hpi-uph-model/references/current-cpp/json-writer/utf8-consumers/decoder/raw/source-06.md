# 原文 06：header original contract, constants, declarations and WsMessage

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsFrame.h`；以 `header original contract, constants, declarations and WsMessage` 定位。
固定來源 `fd50c7d93d928a1ed48d9cbf5b668e39d471c758`；SHA256 `c280ee0c61525234a6b2cf121d63a5440873c7bde949dd7b440aee7c56c33f8c`。
context，新增完成函式計數0。保留原註解；未執行程式或測試。

```cpp
<!-- preserved-content:start -->
// ===========================================================================
//  WebBridge/WsFrame.h
//
//  RFC 6455 section 5 framing: encode (server -> client) and an INCREMENTAL
//  decode (client -> server).
//
//  SCOPE / LAYERING
//  ----------------
//  Pure bytes in -> bytes out, exactly like WsHandshake: no sockets, no
//  machine state, no <winsock2.h>, no <vcl.h>. Multi-byte lengths are composed
//  and decomposed with explicit shifts rather than htons/htonl so that this
//  file needs no platform networking header at all and its byte order is
//  visible in the source.
//
//  WHY THE DECODER IS INCREMENTAL (and why that is not optional)
//  ------------------------------------------------------------
//  TCP has no message boundaries. recv() will hand you half a frame, two and a
//  half frames, or one byte -- whatever the network happened to deliver. A
//  decoder that assumes "one recv == one frame" works on localhost during
//  development and then corrupts state the first time a real network splits a
//  64 KiB payload. WsDecoder::Feed() therefore accepts arbitrary chunk
//  boundaries and is required (and tested) to produce byte-identical messages
//  when fed one byte at a time.
//
//  WHAT IS COVERED (each item is called out again at its implementation site)
//  -------------------------------------------------------------------------
//   * opcodes: continuation 0x0, text 0x1, binary 0x2, close 0x8, ping 0x9,
//     pong 0xA. Every other opcode is reserved and is a protocol error.
//   * all three payload length forms: 7-bit, 16-bit (126) and 64-bit (127),
//     both in network byte order (big-endian).
//   * masking DIRECTION is enforced, not merely supported: client -> server
//     frames MUST be masked (RFC 6455 section 5.3) and an unmasked one is
//     rejected; server -> client frames MUST NOT be masked, and the encoder
//     defaults to unmasked. The masked encoder exists for tests and for any
//     future client role -- it is not what the server sends.
//   * fragmentation: continuation frames are reassembled, and control frames
//     may be interleaved between the fragments of a data message (they are
//     delivered immediately and do not disturb the reassembly buffer).
//     Control frames are themselves never fragmented -- FIN=0 on a control
//     frame is a protocol error.
//   * a hard maximum message size, enforced BEFORE the payload is buffered, so
//     a hostile or buggy client cannot make the machine PC allocate 4 GiB by
//     announcing a 64-bit length. Exceeding it closes with status 1009.
//   * close frames: 2-byte big-endian status code plus an optional UTF-8
//     reason; a 1-byte payload and illegal status codes are protocol errors.
//
//  C++ DIALECT: C++14-compatible (see the note in WsHandshake.h -- the
//  configured MinGW g++ 6.3.0 accepts -std=c++17 but does not implement it).
// ===========================================================================
#ifndef WEBBRIDGE_WSFRAME_H
#define WEBBRIDGE_WSFRAME_H

#include <cstddef>
#include <string>
#include <vector>

#if defined(_MSC_VER) && (_MSC_VER < 1600)
   typedef unsigned __int8  uint8_t;
   typedef unsigned __int16 uint16_t;
   typedef unsigned __int32 uint32_t;
   typedef unsigned __int64 uint64_t;
#else
#  include <cstdint>
#endif

namespace webbridge {

// --- RFC 6455 section 5.2 opcodes ------------------------------------------
enum WsOpcode {
    kWsContinuation = 0x0,
    kWsText         = 0x1,
    kWsBinary       = 0x2,
    kWsClose        = 0x8,
    kWsPing         = 0x9,
    kWsPong         = 0xA
};

// --- RFC 6455 section 7.4.1 close status codes -----------------------------
enum WsCloseCode {
    kWsCloseNormal          = 1000,
    kWsCloseGoingAway       = 1001,
    kWsCloseProtocolError   = 1002,
    kWsCloseUnsupportedData = 1003,
    kWsCloseNoStatusRcvd    = 1005,  // never sent on the wire
    kWsCloseAbnormal        = 1006,  // never sent on the wire
    kWsCloseInvalidPayload  = 1007,
    kWsClosePolicyViolation = 1008,
    kWsCloseMessageTooBig   = 1009,
    kWsCloseExtensionNeeded = 1010,
    kWsCloseInternalError   = 1011
};

// Default cap on one reassembled message. The tag snapshot for the home screen
// is ~234 tags (ARCHITECTURE.md section 4 rule 2), so an inbound command frame
// is tiny; 1 MiB is generous for anything an operator UI legitimately sends.
const std::size_t kDefaultMaxMessageBytes = 1u * 1024u * 1024u;

// Control frame payloads are capped by the RFC itself, not by policy.
const std::size_t kWsMaxControlPayload = 125;

// ---------------------------------------------------------------------------
//  Encoding (server -> client unless stated otherwise)
// ---------------------------------------------------------------------------

// The general form. `mask` MUST be false for anything the server sends
// (RFC 6455 section 5.1); it exists so tests and any future client role can
// generate conformant client frames.
std::string EncodeFrame(int opcode, const std::string& payload,
                        bool fin = true, bool mask = false, uint32_t maskKey = 0);

std::string EncodeText(const std::string& utf8Payload);
std::string EncodeBinary(const std::string& bytes);
std::string EncodePing(const std::string& payload = std::string());
std::string EncodePong(const std::string& payload = std::string());

// A close frame carrying `code` big-endian followed by `reason` (which must be
// UTF-8 and is truncated so the total payload stays within 125 bytes).
std::string EncodeClose(uint16_t code, const std::string& reason = std::string());

// Client-direction helper: same as EncodeFrame with mask=true.
std::string EncodeMaskedFrame(int opcode, const std::string& payload,
                              uint32_t maskKey, bool fin = true);

// ---------------------------------------------------------------------------
//  A decoded message
// ---------------------------------------------------------------------------
struct WsMessage {
    int opcode;            // kWsText / kWsBinary for data; kWsClose/Ping/Pong for control
    std::string payload;   // data messages: fully reassembled across fragments
                           // close frames: the raw payload (code + reason bytes)

    // Only meaningful when opcode == kWsClose.
    bool hasCloseCode;
    uint16_t closeCode;    // kWsCloseNoStatusRcvd (1005) when hasCloseCode is false
    std::string closeReason;

    WsMessage()
        : opcode(0), hasCloseCode(false), closeCode(kWsCloseNoStatusRcvd) {}

    bool IsControl() const { return (opcode & 0x08) != 0; }
};

// ---------------------------------------------------------------------------

<!-- preserved-content:end -->
```
