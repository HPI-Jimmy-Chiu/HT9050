/* AI(W906-HTDESIGNER) 20261006: the program test\vscode_dbg_it.ps1 debugs -- ten threads (the new threads are what
   ES02's endpoint security kills gdb over), then a line to stop at. */
#include <windows.h>
#include <stdio.h>
static CRITICAL_SECTION cs;
static volatile LONG n;
static DWORD WINAPI work(LPVOID p) { int i; (void)p; for (i = 0; i < 1000; i++) { EnterCriticalSection(&cs); n++; LeaveCriticalSection(&cs); } return 0; }
int main(void) {
  HANDLE h[10];
  int i;
  InitializeCriticalSection(&cs);
  for (i = 0; i < 10; i++) h[i] = CreateThread(0, 0, work, 0, 0, 0);
  WaitForMultipleObjects(10, h, TRUE, INFINITE);
  printf("n=%ld\n", n);   /* HTD-STOP-HERE */
  return 0;
}
