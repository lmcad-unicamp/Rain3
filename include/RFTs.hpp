#ifndef RFTS_HPP
#define RFTS_HPP

#include <Region.hpp>

namespace rain3 {
  namespace RFTs {
    bool isHot(uint16_t); 
    bool wasBackwardBranch(trace_io::trace_item_t&, trace_io::trace_item_t&); 

    Maybe<Region> handleNewInstructionWithNET(trace_io::trace_item_t&, trace_io::trace_item_t&, InternStateTransition);
  }
}
#endif
