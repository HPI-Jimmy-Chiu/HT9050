/* AI(W906-HTDESIGNER) 20261006: the program test\vscode_dbg_it.ps1 debugs -- ten threads (the new threads are what
   ES02's endpoint security kills gdb over), then a line to stop at.
   AI(W906-HTDESIGNER) 20261008 (EastSun「debug 模式下中斷後 按F8 F7 F9 功能都跟BCB一樣」): then a call to step over (F8)
   and one to trace into (F7).
   AI(W906-HTDESIGNER) 20261008 (gap list #19, Attach to Process): "thr wait" runs a loop for up to 60 s -- a program
   already running, for the debugger to attach to. */
#include <windows.h>
#include <stdio.h>
#include <string.h>
static CRITICAL_SECTION cs;
static volatile LONG n;
static volatile LONG ticks;
static DWORD WINAPI work(LPVOID p) { int i; (void)p; for (i = 0; i < 1000; i++) { EnterCriticalSection(&cs); n++; LeaveCriticalSection(&cs); } return 0; }
static int add2(int a)
{
  int b = a + 2;   /* HTD-IN-FUNC */
  return b;
}
static int waitLoop(void)
{
  while (ticks < 600) {
    ticks++;       /* HTD-ATTACH-LOOP */
    Sleep(100);
  }
  return 0;
}
int main(int argc, char **argv) {
  HANDLE h[10];
  int i, r;
  if (argc > 1 && strcmp(argv[1], "wait") == 0) return waitLoop();
  InitializeCriticalSection(&cs);
  for (i = 0; i < 10; i++) h[i] = CreateThread(0, 0, work, 0, 0, 0);
  WaitForMultipleObjects(10, h, TRUE, INFINITE);
  printf("n=%ld\n", n);   /* HTD-STOP-HERE */
  r = add2(5);            /* HTD-CALL1 */
  r = add2(r);            /* HTD-CALL2 */
  printf("r=%d\n", r);
  return 0;
}
