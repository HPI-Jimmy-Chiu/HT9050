# 原文 26：BuildTooLargeResponse

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsHandshake.cpp`；以 `BuildTooLargeResponse` 定位。
pin `7bf6ff28e23bbfeb6f94c1751d63ae62533313f4`；SHA256 `efb92fa15f728f9beb8ce7db1910d58059554af3eedbcc63bd46fe3bcaa0651d`。
原函式／相鄰註解或header context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
std::string BuildTooLargeResponse() {
    return SimpleResponse("431 Request Header Fields Too Large", 0);
}


<!-- preserved-content:end -->
```
