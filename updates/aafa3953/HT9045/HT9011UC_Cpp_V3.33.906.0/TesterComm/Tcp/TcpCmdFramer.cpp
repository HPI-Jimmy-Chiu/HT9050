// ===========================================================================
//  TesterComm/Tcp/TcpCmdFramer.cpp -- see TcpCmdFramer.h.  AI(W906-W10) 20260927 (St02-E).
// ===========================================================================
#include "TesterComm/Tcp/TcpCmdFramer.h"

#include <cstring>

namespace tcpcmd {

namespace {

// W10-FIELDS-TABLE-BEGIN (tools/tcp_cmd_fields_census.py reads the ids between these markers)
// HTSET ids whose branch reads sData[2] (3 fields) / sData[3] (4 fields); every other id -- all HTGR ids included --
// reads sData[0..1] only (2 fields).  720 reads sData[3] but only when it is there (optional 4th, see FieldsFor).
const char* const kThreeFields[] = { "301", "316", "322", "323", "354", "403", "404", "461", "463", "464", "466",
                                     "468", "470", "710", "712", "713", "804", "811", "720" };
const char* const kFourFields[]  = { "350", "702" };
// W10-FIELDS-TABLE-END

const char kHeadGr[] = "HTGR,";
const char kHeadSet[] = "HTSET,";

bool IsSep(char c) { return c == '\r' || c == '\n' || c == ' ' || c == '\t' || c == '\0'; }

// length of the head that starts at `pos` (5 / 6), or 0
size_t HeadAt(const std::string& b, size_t pos)
{
    if (b.compare(pos, 5, kHeadGr) == 0)
        return 5;
    if (b.compare(pos, 6, kHeadSet) == 0)
        return 6;
    return 0;
}

// the bytes from `pos` to the end are the start of a head ("H", "HT", "HTSE", ...) -- wait for more
bool HeadPrefixTail(const std::string& b, size_t pos)
{
    const size_t n = b.size() - pos;
    if (n == 0)
        return false;
    return (n < 5 && b.compare(pos, n, kHeadGr, n) == 0) || (n < 6 && b.compare(pos, n, kHeadSet, n) == 0);
}

size_t FindHead(const std::string& b, size_t from)
{
    const size_t a = b.find(kHeadGr, from);
    const size_t s = b.find(kHeadSet, from);
    return a < s ? a : s;   // npos is the largest size_t
}

// where to cut when dropping everything that cannot become a frame: keep a trailing head prefix for the next read
size_t CutKeepingHeadPrefix(const std::string& b, size_t from)
{
    const size_t start = b.size() > 5 ? b.size() - 5 : from;
    for (size_t k = start > from ? start : from; k < b.size(); ++k)
        if (HeadPrefixTail(b, k))
            return k;
    return b.size();
}

void Report(std::vector<Event>* events, EventKind kind, size_t bytes)
{
    if (events != 0 && bytes > 0)
    {
        Event e;
        e.kind = kind;
        e.bytes = bytes;
        events->push_back(e);
    }
}

}  // namespace

int FieldsFor(const std::string& head, const std::string& id, bool* optionalExtra)
{
    if (optionalExtra)
        *optionalExtra = false;
    if (head != "HTSET")
        return 2;
    for (size_t i = 0; i < sizeof(kFourFields) / sizeof(kFourFields[0]); ++i)
        if (id == kFourFields[i])
            return 4;
    for (size_t i = 0; i < sizeof(kThreeFields) / sizeof(kThreeFields[0]); ++i)
        if (id == kThreeFields[i])
        {
            if (optionalExtra && id == "720")
                *optionalExtra = true;
            return 3;
        }
    return 2;
}

bool NextFrame(std::string& b, std::string* frame, std::vector<Event>* events)
{
    for (;;)
    {
        // a. separators between frames
        size_t s = 0;
        while (s < b.size() && IsSep(b[s]))
            ++s;
        if (s > 0)
            b.erase(0, s);
        if (b.empty())
            return false;

        const size_t hl = HeadAt(b, 0);
        if (hl == 0)
        {
            // b. only the start of a head so far: wait
            if (HeadPrefixTail(b, 0))
                return false;
            // c. noise: drop up to the next head (or all of it, keeping a trailing head prefix)
            const size_t nh = FindHead(b, 1);
            const size_t cut = (nh != std::string::npos) ? nh : CutKeepingHeadPrefix(b, 1);
            Report(events, kDiscard, cut);
            b.erase(0, cut);
            continue;
        }

        // d. a head at 0: the id, then the comma its last field ends with -- before the next head
        const size_t nh = FindHead(b, hl);
        const size_t lim = (nh != std::string::npos) ? nh : b.size();
        size_t end = std::string::npos;
        const size_t c2 = b.find(',', hl);
        if (c2 != std::string::npos && c2 < lim)
        {
            bool optional = false;
            const int need = FieldsFor(b.substr(0, hl - 1), b.substr(hl, c2 - hl), &optional);
            size_t pos = c2;               // the comma after the id is field 2's
            int count = 2;
            while (count < need)
            {
                pos = b.find(',', pos + 1);
                if (pos == std::string::npos || pos >= lim)
                    break;
                ++count;
            }
            if (count == need && pos != std::string::npos && pos < lim)
            {
                end = pos + 1;
                if (optional)
                {
                    // HTSET,720's optional OPID: taken when its comma is already here, on the same line
                    const size_t c4 = b.find(',', end);
                    if (c4 != std::string::npos && c4 < lim)
                    {
                        bool sameLine = true;
                        for (size_t k = end; k < c4; ++k)
                            if (b[k] == '\r' || b[k] == '\n' || b[k] == '\0')
                            {
                                sameLine = false;
                                break;
                            }
                        if (sameLine)
                            end = c4 + 1;
                    }
                }
            }
        }

        if (end != std::string::npos)
        {
            if (end > kCap)
            {
                Report(events, kOverflow, end);   // longer than the limit: dropped whole, never truncated
                b.erase(0, end);
                continue;
            }
            // e. complete
            if (frame)
                frame->assign(b, 0, end);
            b.erase(0, end);
            return true;
        }

        if (nh != std::string::npos)
        {
            Report(events, kIncomplete, nh);      // cut short by a new head
            b.erase(0, nh);
            continue;
        }
        if (b.size() > kCap)
        {
            const size_t cut = CutKeepingHeadPrefix(b, 1);
            Report(events, kOverflow, cut);       // kCap bytes and still no complete frame
            b.erase(0, cut);
            continue;
        }
        return false;                             // wait for the rest (no time-out, as the RS232 program)
    }
}

void Framer::Append(const void* key, const char* data, size_t n)
{
    if (data != 0 && n > 0)
        buf_[key].append(data, n);
}

bool Framer::Next(const void* key, std::string* frame, std::vector<Event>* events)
{
    std::map<const void*, std::string>::iterator it = buf_.find(key);
    if (it == buf_.end())
        return false;
    const bool got = NextFrame(it->second, frame, events);
    if (it->second.empty())
        buf_.erase(it);
    return got;
}

void Framer::Drop(const void* key) { buf_.erase(key); }
void Framer::Clear() { buf_.clear(); }

size_t Framer::Pending(const void* key) const
{
    std::map<const void*, std::string>::const_iterator it = buf_.find(key);
    return it == buf_.end() ? 0 : it->second.size();
}

size_t Framer::Connections() const { return buf_.size(); }

}  // namespace tcpcmd
