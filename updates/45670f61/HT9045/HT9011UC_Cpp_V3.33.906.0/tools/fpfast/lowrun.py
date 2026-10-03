# AI(W906-FPFAST) 20261003 -- run a command at IDLE priority on an affinity mask
# (the "start /low /affinity 3F" of the task, done with the Win32 API so that
# every descendant -- cmd, cmake, ninja, g++, cc1plus, as, ld, ctest -- inherits
# both).  Usage: python lowrun.py [--mask 0x3F] [--cwd DIR] [--env K=V ...] -- cmd args...
# The command's exit code is returned.  No backslash literal in this file.
import ctypes, os, subprocess, sys

IDLE_PRIORITY_CLASS = 0x00000040


def main():
    argv = sys.argv[1:]
    mask = 0x3F
    cwd = None
    env = dict(os.environ)
    while argv and argv[0] != '--':
        if argv[0] == '--mask':
            mask = int(argv[1], 0); argv = argv[2:]
        elif argv[0] == '--cwd':
            cwd = argv[1]; argv = argv[2:]
        elif argv[0] == '--env':
            k, v = argv[1].split('=', 1); env[k] = v; argv = argv[2:]
        elif argv[0] == '--path-prepend':
            env['PATH'] = argv[1] + ';' + env.get('PATH', ''); argv = argv[2:]
        else:
            raise SystemExit('unknown option ' + argv[0])
    argv = argv[1:]
    k = ctypes.windll.kernel32
    k.GetCurrentProcess.restype = ctypes.c_void_p
    k.SetPriorityClass.argtypes = [ctypes.c_void_p, ctypes.c_uint32]
    k.SetProcessAffinityMask.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
    h = k.GetCurrentProcess()
    ok1 = k.SetPriorityClass(h, IDLE_PRIORITY_CLASS)
    ok2 = k.SetProcessAffinityMask(h, mask)
    if not (ok1 and ok2):
        raise SystemExit('could not set priority / affinity')
    sys.stdout.flush()
    p = subprocess.run(argv, cwd=cwd, env=env, creationflags=IDLE_PRIORITY_CLASS)
    sys.exit(p.returncode)


if __name__ == '__main__':
    main()
