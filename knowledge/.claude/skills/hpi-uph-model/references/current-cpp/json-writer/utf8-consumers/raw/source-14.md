# 原文 14：WsDecoder complete class declaration and validation configuration

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsFrame.h`；以 `WsDecoder complete class declaration and validation configuration` 定位。
固定來源 `b1bbe408a035595c81615cbc22543a326fba12b2`；SHA256 `8cd0aac30c802be25085daf7a0f716e37cb2d185409fbf4594c3e558e3435b00`。
context，新增完成函式計數0。未執行程式／測試。

```cpp
<!-- preserved-content:start -->
// ---------------------------------------------------------------------------
//  WsDecoder -- incremental, one instance per connection
// ---------------------------------------------------------------------------
class WsDecoder {
public:
    // `requireMaskedInput` true is the server role: inbound frames must be
    // masked. Construct with false only to decode a server's frames (tests).
    explicit WsDecoder(bool requireMaskedInput = true,
                       std::size_t maxMessageBytes = kDefaultMaxMessageBytes);

    // Feed received bytes. Completed messages are APPENDED to *out (existing
    // contents are left alone), in arrival order.
    //
    // Returns true while the stream is still well-formed. On false the stream
    // has failed: CloseCode()/CloseReason() give the status the caller must
    // send via EncodeClose() before closing the socket, and every later Feed()
    // returns false without doing anything.
    bool Feed(const char* data, std::size_t len, std::vector<WsMessage>* out);
    bool Feed(const std::string& chunk, std::vector<WsMessage>* out);

    bool Failed() const { return failed_; }
    uint16_t CloseCode() const { return closeCode_; }
    const std::string& CloseReason() const { return closeReason_; }

    // True once a close frame has been received from the peer.
    bool SawClose() const { return sawClose_; }

    // Bytes held for a partially received frame, plus bytes held for a
    // partially reassembled fragmented message. For diagnostics/tests.
    std::size_t PendingBytes() const { return buf_.size(); }
    std::size_t FragmentBytes() const { return frag_.size(); }

    // Text payloads and close reasons are UTF-8 validated by default and a
    // violation closes with 1007 (RFC 6455 sections 5.6 and 8.1). Turn off
    // only if a non-conformant peer must be tolerated.
    void SetValidateUtf8(bool on) { validateUtf8_ = on; }

    void Reset();

    // Exposed because it is genuinely useful to callers and is tested here.
    static bool IsValidUtf8(const std::string& s);

private:
    // Try to consume one whole frame from the front of buf_.
    // Returns: 1 progress made, 0 need more bytes, -1 protocol failure.
    int TryOneFrame(std::vector<WsMessage>* out);
    bool Fail(uint16_t code, const char* reason);

    std::string buf_;        // received-but-unparsed bytes
    std::string frag_;       // reassembly buffer for the current data message
    int  fragOpcode_;        // kWsText/kWsBinary while fragmenting, else 0
    bool requireMask_;
    bool validateUtf8_;
    bool failed_;
    bool sawClose_;
    uint16_t closeCode_;
    std::string closeReason_;
    std::size_t maxMessage_;
};

<!-- preserved-content:end -->
```
