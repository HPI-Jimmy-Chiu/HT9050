# 生命期與等待

[上層](index.md)；[constructor](raw/source-01.md#constructor)／[destructor](raw/source-01.md#destructor)／[waitForPush](raw/source-01.md#waitforpush)。

## CommandQueue::CommandQueue與Impl

out-of-line constructor以`new Impl(capacity)`建立所有權。Impl inline constructor初始化queue／capacity／peak／三counter，
並呼叫`CreateEventA(NULL, FALSE, FALSE, NULL)`：auto-reset、初始未發訊號。這支inline constructor僅context，完成credit0。
CreateEventA回傳沒有在constructor檢查，失敗留下空ev時waitForPush走fallback；未呼叫Win32 API實測。
`typedef WbMutex WbLock`、Sync.h合併歷史與Win32-only #error原說明全保留；既有Sync／header沿用不重複保存或計完成。

## CommandQueue::~CommandQueue

ev非空才CloseHandle，接著delete impl_並置0；本段未停止其他執行緒、Join或等待drain／push呼叫結束。
宿主析構順序與所有權生命期仍待完整caller查證，不能僅由這幾行聲稱可與在途操作安全並行。

## CommandQueue::waitForPush

ev空時`Sleep(ms > 2ul ? 2ul : ms)`後false；ev存在時只有`WaitForSingleObject(ev, (DWORD)ms) == WAIT_OBJECT_0`才true。
timeout／其他Win32結果均沿同一bool false分支，未由本段區分或記錄原因。
auto-reset event表示push通知，不能從true推出queue目前非空；通知後其他drain／時序可能已改變queue。
原「push-before-wait leaves event set」「spurious return harmless, caller re-checks size」完整保留為設計契約，caller執行尚未實測。
event不是一個命令一個token的計數，連續SetEvent不等於保存逐命令喚醒次數；具體宿主循環與等待時序另待核。
