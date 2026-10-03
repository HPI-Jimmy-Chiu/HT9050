// =============================================================================
//  WebBinDispStatus_St02.h -- ST02-C14 part 3: the web "Bin Display Status" pane's tags (golden tsUnloadMap).
//  AI(W906-ST02-C14) 20261002 (St02-E helper).  Body and the tag list: WebBinDispStatus_St02.cpp (its banner).
//
//  tools/wb_serve.cpp does not include this header: its PublishExtraTags sits in an unnamed namespace, so the declaration
//  there is a same-line one at global scope (the W906_PageStreamWanted line before it).  Keep the two in step.
// =============================================================================
#ifndef WebBinDispStatus_St02H
#define WebBinDispStatus_St02H

#include <cstddef>

namespace webbridge { class TagSnapshot; }

// Stage the binsel.disp.* tags (read only; the caller holds the publish window).  Returns how many it staged.
std::size_t W906_StageBinDispStatus_St02(webbridge::TagSnapshot& snap);
// How many tags one W906_StageBinDispStatus_St02 call stages (the count the ctest pins).
std::size_t W906_BinDispStatusTagCount_St02();

#endif
