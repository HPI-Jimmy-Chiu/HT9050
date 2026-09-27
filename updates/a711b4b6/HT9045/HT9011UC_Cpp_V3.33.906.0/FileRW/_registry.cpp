// 產生檔 -- tools/gen_formbridge.py（Steven 20260924，S12 第二型）。頁面 → bridge 登錄表。
#include "JsonBridge/FormBridge.h"

namespace ht9045 {
namespace formbridge {

extern const BridgeDesc kBridge_TfHotPlate;

extern const BridgeDesc* const kBridges[] = {
    &kBridge_TfHotPlate,
};
extern const std::size_t kBridgeCount = 1;

}  // namespace formbridge
}  // namespace ht9045
