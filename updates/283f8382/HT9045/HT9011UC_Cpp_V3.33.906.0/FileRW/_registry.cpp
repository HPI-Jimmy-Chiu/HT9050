// 產生檔 -- tools/gen_formbridge.py（Steven 20260924，S12 第二型）。頁面 → bridge 登錄表。
#include "JsonBridge/FormBridge.h"

namespace ht9045 {
namespace formbridge {

extern const BridgeDesc kBridge_TfHotPlate;
extern const BridgeDesc kBridge_TfTeach;   // AI(W906-TEACH-FORMSHOW) 20261002: hand-written FileRW/TeachFormShow_File.cpp -- keep when regenerating
extern const BridgeDesc kBridge_Tfiosetview;   // AI(W906-IOSV-FORMSHOW) 20261002: hand-written FileRW/IoSetViewFormShow_File.cpp -- keep when regenerating

extern const BridgeDesc* const kBridges[] = {
    &kBridge_TfHotPlate,
    &kBridge_TfTeach,
    &kBridge_Tfiosetview,   // AI(W906-IOSV-FORMSHOW) 20261002
};
extern const std::size_t kBridgeCount = 3;   // AI(W906-IOSV-FORMSHOW) 20261002: + Tfiosetview

}  // namespace formbridge
}  // namespace ht9045
