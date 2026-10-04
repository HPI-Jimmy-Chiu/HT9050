// AI(W906-F5-CLOSE) 20261004: NB2-1 (i), machine dispatch 4 -- the pure decisions of the F5-close rule (tools/wb_serve.cpp F5Close* at EOF
//   applies them; tools/hmi_shell/hmi_shell.cpp writes the flag; ctest F5CloseDecide pins them).
//   marker      = W906_F5_OPERATOR_CLOSE=1 in the environment (only the F5 launch entries set it: .vscode/launch.json)
//   flagAgeSec  = age of %LOCALAPPDATA%\HT9045_HMI_Shell\f5_close_<port>.flag in seconds, < 0 = no flag
#pragma once

namespace ht9045 {

const double kF5CloseFlagMaxAgeSec = 3600.0;   // the slowest measured F5 build is 352 s (dispatch 5); older = left over by a crash

enum F5CloseStart { kF5StartNormally = 0, kF5DoNotStart = 1, kF5StaleFlag = 2 };

// at start, before anything is loaded: the operator closed this F5's wait page -> do not start
inline F5CloseStart F5CloseAtStart(bool marker, double flagAgeSec)
{
    if (!marker || flagAgeSec < 0.0) return kF5StartNormally;
    return flagAgeSec > kF5CloseFlagMaxAgeSec ? kF5StaleFlag : kF5DoNotStart;
}

// while running: the close is pending (HMI-KEEP does not reopen the window) -- never once the machine runs (SystemStart)
inline bool F5ClosePending(bool marker, double flagAgeSec, bool systemStart)
{
    return marker && flagAgeSec >= 0.0 && flagAgeSec <= kF5CloseFlagMaxAgeSec && !systemStart;
}

// close normally now: pending, and no web page for 2 s
inline bool F5CloseQuitNow(bool pending, int pagesPresent, unsigned long long noPageMs)
{
    return pending && pagesPresent == 0 && noPageMs >= 2000ULL;
}

}  // namespace ht9045
