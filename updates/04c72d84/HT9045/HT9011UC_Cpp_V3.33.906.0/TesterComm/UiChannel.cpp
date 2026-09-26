// ===========================================================================
//  TesterComm/UiChannel.cpp -- see UiChannel.h.  AI(W906-GB-P7) 20260926.
// ===========================================================================
#include "TesterComm/UiChannel.h"

#include <windows.h>   // GetTickCount (Post debounce window)

#include <cstdio>

namespace testercomm {

UiChannel& UiChannel::Instance()
{
    static UiChannel ch;
    return ch;
}

UiChannel::UiChannel() : dropped_(0) {}

void UiChannel::Publish(const std::string& key, const std::string& json)
{
    webbridge::WbGuard g(mu_);
    Snap& s = snaps_[key];
    s.json = json;
    ++s.seq;
}

bool UiChannel::Read(const std::string& key, std::string* json, unsigned long* seq) const
{
    webbridge::WbGuard g(mu_);
    std::map<std::string, Snap>::const_iterator it = snaps_.find(key);
    if (it == snaps_.end() || it->second.seq == 0)
        return false;
    if (json)
        *json = it->second.json;
    if (seq)
        *seq = it->second.seq;
    return true;
}

void UiChannel::Clear(const std::string& key)
{
    webbridge::WbGuard g(mu_);
    snaps_.erase(key);
    cmds_.erase(key);
}

bool UiChannel::Post(const std::string& key, const std::string& command, unsigned windowMs)
{
    webbridge::WbGuard g(mu_);
    const unsigned long now = ::GetTickCount();
    LastPost& last = last_[key];
    if (windowMs > 0 && last.command == command && now - last.tick < windowMs)
        return false;   // same key + same command inside the window: a repeat, not a second click
    last.command = command;
    last.tick = now;
    std::deque<std::string>& q = cmds_[key];
    if (q.size() >= kMaxQueued)
    {
        q.pop_front();   // a flood of clicks must never grow without bound; the newest wins
        ++dropped_;
    }
    q.push_back(command);
    return true;
}

std::vector<std::string> UiChannel::Take(const std::string& key)
{
    std::vector<std::string> out;
    webbridge::WbGuard g(mu_);
    std::map<std::string, std::deque<std::string> >::iterator it = cmds_.find(key);
    if (it == cmds_.end())
        return out;
    out.assign(it->second.begin(), it->second.end());
    it->second.clear();
    return out;
}

unsigned long UiChannel::Dropped() const
{
    webbridge::WbGuard g(mu_);
    return dropped_;
}

std::string JsonEscape(const std::string& s)
{
    std::string o;
    o.reserve(s.size() + 2);
    o += '"';
    for (size_t i = 0; i < s.size(); ++i)
    {
        const unsigned char c = static_cast<unsigned char>(s[i]);
        switch (c)
        {
        case '"':  o += "\\\""; break;
        case '\\': o += "\\\\"; break;
        case '\n': o += "\\n";  break;
        case '\r': o += "\\r";  break;
        case '\t': o += "\\t";  break;
        default:
            if (c < 0x20)
            {
                char b[8];
                std::snprintf(b, sizeof(b), "\\u%04x", c);
                o += b;
            }
            else
            {
                o += static_cast<char>(c);   // UTF-8 passes through (the bridges' texts are ASCII / UTF-8 comments)
            }
        }
    }
    o += '"';
    return o;
}

}  // namespace testercomm
