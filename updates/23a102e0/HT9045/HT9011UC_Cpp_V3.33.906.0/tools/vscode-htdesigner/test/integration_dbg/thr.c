/* AI(W906-HTDESIGNER) 20261006: the program test\vscode_dbg_it.ps1 debugs -- ten threads (the new threads are what
   ES02's endpoint security kills gdb over), then a line to stop at.
   AI(W906-HTDESIGNER) 20261008 (EastSun「debug 模式下中斷後 按F8 F7 F9 功能都跟BCB一樣」): then a call to step over (F8)
   and one to trace into (F7). */
#include <windows.h>
#include <stdio.h>
static CRITICAL_SECTION cs;
static volatile LONG n;
static DWORD WINAPI work(LPVOID p) { int i; (void)p; for (i = 0; i < 1000; i++) { EnterCriticalSection(&cs); n++; LeaveCriticalSection(&cs); } return 0; }
static int add2(int a)
{
  int b = a + 2;   /* HTD-IN-FUNC */
  return b;
}
int main(void) {
  HANDLE h[10];
  int i, r;
  InitializeCriticalSection(&cs);
  for (i = 0; i < 10; i++) h[i] = CreateThread(0, 0, work, 0, 0, 0);
  WaitForMultipleObjects(10, h, TRUE, INFINITE);
  printf("n=%ld\n", n);   /* HTD-STOP-HERE */
  r = add2(5);            /* HTD-CALL1 */
  r = add2(r);            /* HTD-CALL2 */
  printf("r=%d\n", r);
  return 0;
}
