// ===========================================================================
//  sjson_SYSTEM_DEVICE_FORM.gen.cpp -- GENERATED, DO NOT EDIT BY HAND.
//
//  AI(W906-JSONBRIDGE-S2) 20260923.  NOT in golden.
//
//  產生器：tools/gen_sjson.py
//  來源　：cprod.h:1156-1243（結構 SYSTEM_DEVICE_FORM，66 個頂層成員）
//  規格　：.claude/skills/ht9045-json-bridge/SKILL.md 4.1
//
//  重跑：python tools\gen_sjson.py
//
//  ⚠ offset 與 elemSize 都是 offsetof/sizeof —— **編譯器**算的，不是產生器。
//    產生器只知道名字。
// ===========================================================================
#include "JsonBridge/FieldDesc.h"

#include "vclcompat/vcl_compat.h"
#include "cprod.h"

namespace ht9045 {
namespace sjson {

namespace {

const FieldDesc kFields_SYSTEM_DEVICE_FORM[] = {
    { "IndexArmPick", offsetof(SYSTEM_DEVICE_FORM, IndexArmPick), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->IndexArmPick[0]), (std::size_t)(sizeof(((SYSTEM_DEVICE_FORM*)0)->IndexArmPick) / sizeof(((SYSTEM_DEVICE_FORM*)0)->IndexArmPick[0])), (std::size_t)(0) },
    { "IndexPlace", offsetof(SYSTEM_DEVICE_FORM, IndexPlace), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->IndexPlace[0]), (std::size_t)(sizeof(((SYSTEM_DEVICE_FORM*)0)->IndexPlace) / sizeof(((SYSTEM_DEVICE_FORM*)0)->IndexPlace[0])), (std::size_t)(0) },
    { "IndexDrop", offsetof(SYSTEM_DEVICE_FORM, IndexDrop), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->IndexDrop[0]), (std::size_t)(sizeof(((SYSTEM_DEVICE_FORM*)0)->IndexDrop) / sizeof(((SYSTEM_DEVICE_FORM*)0)->IndexDrop[0])), (std::size_t)(0) },
    { "IndexContact", offsetof(SYSTEM_DEVICE_FORM, IndexContact), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->IndexContact[0]), (std::size_t)(sizeof(((SYSTEM_DEVICE_FORM*)0)->IndexContact) / sizeof(((SYSTEM_DEVICE_FORM*)0)->IndexContact[0])), (std::size_t)(0) },
    { "ContactMode", offsetof(SYSTEM_DEVICE_FORM, ContactMode), kFieldInt, sizeof(((SYSTEM_DEVICE_FORM*)0)->ContactMode), (std::size_t)(0), (std::size_t)(0) },
    { "VacuumMode", offsetof(SYSTEM_DEVICE_FORM, VacuumMode), kFieldInt, sizeof(((SYSTEM_DEVICE_FORM*)0)->VacuumMode), (std::size_t)(0), (std::size_t)(0) },
    { "DummyMode", offsetof(SYSTEM_DEVICE_FORM, DummyMode), kFieldInt, sizeof(((SYSTEM_DEVICE_FORM*)0)->DummyMode), (std::size_t)(0), (std::size_t)(0) },
    { "dPress", offsetof(SYSTEM_DEVICE_FORM, dPress), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->dPress), (std::size_t)(0), (std::size_t)(0) },
    { "DropWait", offsetof(SYSTEM_DEVICE_FORM, DropWait), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->DropWait), (std::size_t)(0), (std::size_t)(0) },
    { "iPinCT", offsetof(SYSTEM_DEVICE_FORM, iPinCT), kFieldInt, sizeof(((SYSTEM_DEVICE_FORM*)0)->iPinCT), (std::size_t)(0), (std::size_t)(0) },
    { "XDimension", offsetof(SYSTEM_DEVICE_FORM, XDimension), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->XDimension), (std::size_t)(0), (std::size_t)(0) },
    { "YDimension", offsetof(SYSTEM_DEVICE_FORM, YDimension), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->YDimension), (std::size_t)(0), (std::size_t)(0) },
    { "XDimensionFP", offsetof(SYSTEM_DEVICE_FORM, XDimensionFP), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->XDimensionFP), (std::size_t)(0), (std::size_t)(0) },
    { "YDimensionFP", offsetof(SYSTEM_DEVICE_FORM, YDimensionFP), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->YDimensionFP), (std::size_t)(0), (std::size_t)(0) },
    { "XStartFP", offsetof(SYSTEM_DEVICE_FORM, XStartFP), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->XStartFP), (std::size_t)(0), (std::size_t)(0) },
    { "YStartFP", offsetof(SYSTEM_DEVICE_FORM, YStartFP), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->YStartFP), (std::size_t)(0), (std::size_t)(0) },
    { "VerifyXFP", offsetof(SYSTEM_DEVICE_FORM, VerifyXFP), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->VerifyXFP), (std::size_t)(0), (std::size_t)(0) },
    { "VerifyYFP", offsetof(SYSTEM_DEVICE_FORM, VerifyYFP), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->VerifyYFP), (std::size_t)(0), (std::size_t)(0) },
    { "CCDOffsetFP", offsetof(SYSTEM_DEVICE_FORM, CCDOffsetFP), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->CCDOffsetFP), (std::size_t)(0), (std::size_t)(0) },
    { "CalCCDOffset", offsetof(SYSTEM_DEVICE_FORM, CalCCDOffset), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->CalCCDOffset), (std::size_t)(0), (std::size_t)(0) },
    { "CalCCDIP", offsetof(SYSTEM_DEVICE_FORM, CalCCDIP), kFieldAnsiString, sizeof(((SYSTEM_DEVICE_FORM*)0)->CalCCDIP), (std::size_t)(0), (std::size_t)(0) },
    { "ForcePerPinN", offsetof(SYSTEM_DEVICE_FORM, ForcePerPinN), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->ForcePerPinN), (std::size_t)(0), (std::size_t)(0) },
    { "ForcePerPinG", offsetof(SYSTEM_DEVICE_FORM, ForcePerPinG), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->ForcePerPinG), (std::size_t)(0), (std::size_t)(0) },
    { "DoubleForce", offsetof(SYSTEM_DEVICE_FORM, DoubleForce), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->DoubleForce), (std::size_t)(0), (std::size_t)(0) },
    { "iPinOfDie", offsetof(SYSTEM_DEVICE_FORM, iPinOfDie), kFieldInt, sizeof(((SYSTEM_DEVICE_FORM*)0)->iPinOfDie), (std::size_t)(0), (std::size_t)(0) },
    { "iHeadDeviceCT", offsetof(SYSTEM_DEVICE_FORM, iHeadDeviceCT), kFieldInt, sizeof(((SYSTEM_DEVICE_FORM*)0)->iHeadDeviceCT), (std::size_t)(0), (std::size_t)(0) },
    { "fAireForce", offsetof(SYSTEM_DEVICE_FORM, fAireForce), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->fAireForce), (std::size_t)(0), (std::size_t)(0) },
    { "bSuckShuttleDeviceAfterTested", offsetof(SYSTEM_DEVICE_FORM, bSuckShuttleDeviceAfterTested), kFieldBool, sizeof(((SYSTEM_DEVICE_FORM*)0)->bSuckShuttleDeviceAfterTested), (std::size_t)(0), (std::size_t)(0) },
    { "bSuckShuttleDeviceWaitOnShuttle", offsetof(SYSTEM_DEVICE_FORM, bSuckShuttleDeviceWaitOnShuttle), kFieldBool, sizeof(((SYSTEM_DEVICE_FORM*)0)->bSuckShuttleDeviceWaitOnShuttle), (std::size_t)(0), (std::size_t)(0) },
    { "bShuttleWaitingOutSiteChamber", offsetof(SYSTEM_DEVICE_FORM, bShuttleWaitingOutSiteChamber), kFieldBool, sizeof(((SYSTEM_DEVICE_FORM*)0)->bShuttleWaitingOutSiteChamber), (std::size_t)(0), (std::size_t)(0) },
    { "bTesterSidePush", offsetof(SYSTEM_DEVICE_FORM, bTesterSidePush), kFieldBool, sizeof(((SYSTEM_DEVICE_FORM*)0)->bTesterSidePush), (std::size_t)(0), (std::size_t)(0) },
    { "iSidePushMode", offsetof(SYSTEM_DEVICE_FORM, iSidePushMode), kFieldInt, sizeof(((SYSTEM_DEVICE_FORM*)0)->iSidePushMode), (std::size_t)(0), (std::size_t)(0) },
    { "bPickShuttleDeviceTogether", offsetof(SYSTEM_DEVICE_FORM, bPickShuttleDeviceTogether), kFieldBool, sizeof(((SYSTEM_DEVICE_FORM*)0)->bPickShuttleDeviceTogether), (std::size_t)(0), (std::size_t)(0) },
    { "DropSpeed", offsetof(SYSTEM_DEVICE_FORM, DropSpeed), kFieldInt, sizeof(((SYSTEM_DEVICE_FORM*)0)->DropSpeed), (std::size_t)(0), (std::size_t)(0) },
    { "dKitDiameter", offsetof(SYSTEM_DEVICE_FORM, dKitDiameter), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->dKitDiameter), (std::size_t)(0), (std::size_t)(0) },
    { "dDieForceKitDiameter", offsetof(SYSTEM_DEVICE_FORM, dDieForceKitDiameter), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->dDieForceKitDiameter), (std::size_t)(0), (std::size_t)(0) },
    { "bUseDieForce", offsetof(SYSTEM_DEVICE_FORM, bUseDieForce), kFieldBool, sizeof(((SYSTEM_DEVICE_FORM*)0)->bUseDieForce), (std::size_t)(0), (std::size_t)(0) },
    { "IndexContactBackUp", offsetof(SYSTEM_DEVICE_FORM, IndexContactBackUp), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->IndexContactBackUp[0]), (std::size_t)(sizeof(((SYSTEM_DEVICE_FORM*)0)->IndexContactBackUp) / sizeof(((SYSTEM_DEVICE_FORM*)0)->IndexContactBackUp[0])), (std::size_t)(0) },
    { "IndexContactShuttlePickUp", offsetof(SYSTEM_DEVICE_FORM, IndexContactShuttlePickUp), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->IndexContactShuttlePickUp[0]), (std::size_t)(sizeof(((SYSTEM_DEVICE_FORM*)0)->IndexContactShuttlePickUp) / sizeof(((SYSTEM_DEVICE_FORM*)0)->IndexContactShuttlePickUp[0])), (std::size_t)(0) },
    { "iSocketInitialICCheckPosition", offsetof(SYSTEM_DEVICE_FORM, iSocketInitialICCheckPosition), kFieldInt, sizeof(((SYSTEM_DEVICE_FORM*)0)->iSocketInitialICCheckPosition), (std::size_t)(0), (std::size_t)(0) },
    { "fSocketInitialICCheckPositionOffset", offsetof(SYSTEM_DEVICE_FORM, fSocketInitialICCheckPositionOffset), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->fSocketInitialICCheckPositionOffset), (std::size_t)(0), (std::size_t)(0) },
    { "iIndexTorqueMax", offsetof(SYSTEM_DEVICE_FORM, iIndexTorqueMax), kFieldInt, sizeof(((SYSTEM_DEVICE_FORM*)0)->iIndexTorqueMax), (std::size_t)(0), (std::size_t)(0) },
    { "iIndexTorqueCmp", offsetof(SYSTEM_DEVICE_FORM, iIndexTorqueCmp), kFieldInt, sizeof(((SYSTEM_DEVICE_FORM*)0)->iIndexTorqueCmp), (std::size_t)(0), (std::size_t)(0) },
    { "bPurgeBeforePickShuttle", offsetof(SYSTEM_DEVICE_FORM, bPurgeBeforePickShuttle), kFieldBool, sizeof(((SYSTEM_DEVICE_FORM*)0)->bPurgeBeforePickShuttle), (std::size_t)(0), (std::size_t)(0) },
    { "iPurgeBeforePickShuttleTime", offsetof(SYSTEM_DEVICE_FORM, iPurgeBeforePickShuttleTime), kFieldInt, sizeof(((SYSTEM_DEVICE_FORM*)0)->iPurgeBeforePickShuttleTime), (std::size_t)(0), (std::size_t)(0) },
    { "iPurgeBeforePickShuttleInterval", offsetof(SYSTEM_DEVICE_FORM, iPurgeBeforePickShuttleInterval), kFieldInt, sizeof(((SYSTEM_DEVICE_FORM*)0)->iPurgeBeforePickShuttleInterval), (std::size_t)(0), (std::size_t)(0) },
    { "iPurgeBdforePickShuttleOffSet", offsetof(SYSTEM_DEVICE_FORM, iPurgeBdforePickShuttleOffSet), kFieldInt, sizeof(((SYSTEM_DEVICE_FORM*)0)->iPurgeBdforePickShuttleOffSet), (std::size_t)(0), (std::size_t)(0) },
    { "bIndexUpSpeed", offsetof(SYSTEM_DEVICE_FORM, bIndexUpSpeed), kFieldBool, sizeof(((SYSTEM_DEVICE_FORM*)0)->bIndexUpSpeed), (std::size_t)(0), (std::size_t)(0) },
    { "bUseAddWeight", offsetof(SYSTEM_DEVICE_FORM, bUseAddWeight), kFieldBool, sizeof(((SYSTEM_DEVICE_FORM*)0)->bUseAddWeight), (std::size_t)(0), (std::size_t)(0) },
    { "dZ1Torue", offsetof(SYSTEM_DEVICE_FORM, dZ1Torue), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->dZ1Torue), (std::size_t)(0), (std::size_t)(0) },
    { "dZ2Torue", offsetof(SYSTEM_DEVICE_FORM, dZ2Torue), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->dZ2Torue), (std::size_t)(0), (std::size_t)(0) },
    { "bEnableUseUniversalShuttle", offsetof(SYSTEM_DEVICE_FORM, bEnableUseUniversalShuttle), kFieldBool, sizeof(((SYSTEM_DEVICE_FORM*)0)->bEnableUseUniversalShuttle), (std::size_t)(0), (std::size_t)(0) },
    { "dLoadCellZ1Down", offsetof(SYSTEM_DEVICE_FORM, dLoadCellZ1Down), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->dLoadCellZ1Down), (std::size_t)(0), (std::size_t)(0) },
    { "dLoadCellZ2Down", offsetof(SYSTEM_DEVICE_FORM, dLoadCellZ2Down), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->dLoadCellZ2Down), (std::size_t)(0), (std::size_t)(0) },
    { "iAutoHeightSHTReleaseOfs", offsetof(SYSTEM_DEVICE_FORM, iAutoHeightSHTReleaseOfs), kFieldInt, sizeof(((SYSTEM_DEVICE_FORM*)0)->iAutoHeightSHTReleaseOfs), (std::size_t)(0), (std::size_t)(0) },
    { "iOffsetX", offsetof(SYSTEM_DEVICE_FORM, iOffsetX), kFieldUnsupported, sizeof(((SYSTEM_DEVICE_FORM*)0)->iOffsetX[0][0][0]), (std::size_t)(sizeof(((SYSTEM_DEVICE_FORM*)0)->iOffsetX) / sizeof(((SYSTEM_DEVICE_FORM*)0)->iOffsetX[0])), (std::size_t)(sizeof(((SYSTEM_DEVICE_FORM*)0)->iOffsetX[0]) / sizeof(((SYSTEM_DEVICE_FORM*)0)->iOffsetX[0][0])) },
    { "iOffsetY", offsetof(SYSTEM_DEVICE_FORM, iOffsetY), kFieldUnsupported, sizeof(((SYSTEM_DEVICE_FORM*)0)->iOffsetY[0][0][0]), (std::size_t)(sizeof(((SYSTEM_DEVICE_FORM*)0)->iOffsetY) / sizeof(((SYSTEM_DEVICE_FORM*)0)->iOffsetY[0])), (std::size_t)(sizeof(((SYSTEM_DEVICE_FORM*)0)->iOffsetY[0]) / sizeof(((SYSTEM_DEVICE_FORM*)0)->iOffsetY[0][0])) },
    { "iOffsetR", offsetof(SYSTEM_DEVICE_FORM, iOffsetR), kFieldUnsupported, sizeof(((SYSTEM_DEVICE_FORM*)0)->iOffsetR[0][0][0]), (std::size_t)(sizeof(((SYSTEM_DEVICE_FORM*)0)->iOffsetR) / sizeof(((SYSTEM_DEVICE_FORM*)0)->iOffsetR[0])), (std::size_t)(sizeof(((SYSTEM_DEVICE_FORM*)0)->iOffsetR[0]) / sizeof(((SYSTEM_DEVICE_FORM*)0)->iOffsetR[0][0])) },
    { "iKitDiameterMode", offsetof(SYSTEM_DEVICE_FORM, iKitDiameterMode), kFieldInt, sizeof(((SYSTEM_DEVICE_FORM*)0)->iKitDiameterMode), (std::size_t)(0), (std::size_t)(0) },
    { "UpWait", offsetof(SYSTEM_DEVICE_FORM, UpWait), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->UpWait), (std::size_t)(0), (std::size_t)(0) },
    { "UpSpeed", offsetof(SYSTEM_DEVICE_FORM, UpSpeed), kFieldInt, sizeof(((SYSTEM_DEVICE_FORM*)0)->UpSpeed), (std::size_t)(0), (std::size_t)(0) },
    { "IndexUp", offsetof(SYSTEM_DEVICE_FORM, IndexUp), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->IndexUp[0]), (std::size_t)(sizeof(((SYSTEM_DEVICE_FORM*)0)->IndexUp) / sizeof(((SYSTEM_DEVICE_FORM*)0)->IndexUp[0])), (std::size_t)(0) },
    { "dDropByPassDetect", offsetof(SYSTEM_DEVICE_FORM, dDropByPassDetect), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->dDropByPassDetect), (std::size_t)(0), (std::size_t)(0) },
    { "dSitePushWaitTime", offsetof(SYSTEM_DEVICE_FORM, dSitePushWaitTime), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->dSitePushWaitTime), (std::size_t)(0), (std::size_t)(0) },
    { "DieForcePerPinN", offsetof(SYSTEM_DEVICE_FORM, DieForcePerPinN), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->DieForcePerPinN), (std::size_t)(0), (std::size_t)(0) },
    { "DieForcePerPinG", offsetof(SYSTEM_DEVICE_FORM, DieForcePerPinG), kFieldDouble, sizeof(((SYSTEM_DEVICE_FORM*)0)->DieForcePerPinG), (std::size_t)(0), (std::size_t)(0) },
};

// AI(W906-JSONBRIDGE-S6): 欄位 -> ini 鍵。寫方向要用；讀方向用不到。
// multi=true 表示 golden 有不只一條路徑讀這個欄位，產生器不替你挑。
const IniKey kIniKeys_SYSTEM_DEVICE_FORM[] = {
    { "IndexArmPick", "szDir2", "<S>", "Pick Up1", "0.0", true },
    { "IndexPlace", "szDir2", "<S>", "Place1", "0.0", true },
    { "IndexDrop", "szDir2", "<S>", "Drop1", "0.0", true },
    { "IndexContact", "szDir2", "<S>", "Test Arm1", "1.0, 1.0, true, true, 1.0, fIndexDownPos", true },
    { "ContactMode", "szDir", "Mode", "Contact", "0.0", false },
    { "VacuumMode", "szDir", "Mode", "Vacuum", "0.0", false },
    { "DummyMode", "szDir", "Mode", "Dummy Contact", "0.0", false },
    { "DropWait", "szDir", "Wait Time", "Drop Wait", "0.0", false },
    { "iPinCT", "szDir", "Torque Control", "Pin Number", "0.0", false },
    { "XDimension", "szDir", "Torque Control", "X Dimension", "0.0", false },
    { "YDimension", "szDir", "Torque Control", "Y Dimension", "0.0", false },
    { "ForcePerPinN", "szDir", "Torque Control", "Force Per Pin", "0.0", true },
    { "ForcePerPinG", "szDir", "Torque Control", "Force Per Pin Kg", "DeviceForm_File.ForcePerPinN*1000.0/9.8,", true },
    { "DoubleForce", "szDir", "Torque Control", "Double Force", "3.0", false },
    { "iPinOfDie", "szDir", "Torque Control", "Pin of Die", "10", false },
    { "iHeadDeviceCT", "szDir", "Mode", "Head Device Mode", "0", false },
    { "bTesterSidePush", "szDir", "Mode", "Tester Side Push", "0", false },
    { "iSidePushMode", "szDir", "Mode", "Tester Side Push Mode", "0", false },
    { "DropSpeed", "szDir", "Wait Time", "Drop Speed", "30.0", false },
    { "dKitDiameter", "szDir", "Mode", "Kit Diameter", "3.0", false },
    { "dDieForceKitDiameter", "szDir", "Mode", "Die Force Kit Diameter", "2.0", false },
    { "bUseDieForce", "szDir", "Mode", "bUseDieForce", "false", false },
    { "IndexContactBackUp", "szDir2", "<S>", "ContactBackUp1", "0.0", true },
    { "IndexContactShuttlePickUp", "szDir2", "<S>", "ShuttlePickBackUp1", "0.0", true },
    { "iSocketInitialICCheckPosition", "szDir", "Mode", "iSocketInitialICCheckPosition", "0", false },
    { "iIndexTorqueMax", "szDir", "Torque Control", "iIndexTorqueMax", "120", false },
    { "iIndexTorqueCmp", "szDir", "Torque Control", "iIndexTorqueCmp", "30", false },
    { "iPurgeBeforePickShuttleTime", "szDir", "Mode", "Air Purge Before Pick Shuttle Time", "1", false },
    { "iPurgeBeforePickShuttleInterval", "szDir", "Mode", "Air Purge Before Pick Shuttle Interval", "1", false },
    { "iPurgeBdforePickShuttleOffSet", "szDir", "Mode", "Air Purge Before Pick Shuttle OffSet", "1", false },
    { "bUseAddWeight", "szDir", "Torque Control", "bUseAddWeight", "false", false },
    { "dZ1Torue", "szDir", "Torque Control", "dZ1Torue", "0.0", false },
    { "dZ2Torue", "szDir", "Torque Control", "dZ2Torue", "0.0", false },
    { "dLoadCellZ1Down", "szDir", "Test Arm1", "LoadCellZ1", "0.0", false },
    { "dLoadCellZ2Down", "szDir", "Test Arm2", "LoadCellZ2", "0.0", false },
    { "iAutoHeightSHTReleaseOfs", "szDir", "Mode", "AutoKSHTOfs", "2.0", false },
    { "iKitDiameterMode", "szDir", "Mode", "KitDiameterMode", "999", false },
    { "UpWait", "szDir", "Wait Time", "Up Wait", "0.0", false },
    { "UpSpeed", "szDir", "Wait Time", "Up Speed", "0.0", false },
    { "IndexUp", "szDir", "Test Arm1", "Up", "0.0", true },
    { "dSitePushWaitTime", "szDir", "Wait Time", "Side Push Wait Time", "0.0", false },
    { "DieForcePerPinN", "szDir", "Torque Control", "Die Force Per Pin", "0.0", false },
    { "DieForcePerPinG", "szDir", "Torque Control", "Die Force Per Pin Kg", "30.0", false },
};

}  // namespace

// ⚠ 一定要 extern。C++ 裡命名空間範圍的 `const` 物件預設是**內部連結**，
//   少了 extern 這個定義在別的 TU（Bindings.cpp）看不到，會是
//   「undefined reference to kType_...」。20260923 踩過一次。
extern const TypeDesc kType_SYSTEM_DEVICE_FORM;
extern const TypeDesc kType_SYSTEM_DEVICE_FORM = {
    "SYSTEM_DEVICE_FORM",
    kFields_SYSTEM_DEVICE_FORM,
    sizeof(kFields_SYSTEM_DEVICE_FORM) / sizeof(kFields_SYSTEM_DEVICE_FORM[0]),
    sizeof(SYSTEM_DEVICE_FORM),
    kIniKeys_SYSTEM_DEVICE_FORM,
    sizeof(kIniKeys_SYSTEM_DEVICE_FORM) / sizeof(kIniKeys_SYSTEM_DEVICE_FORM[0])
};

}  // namespace sjson
}  // namespace ht9045
