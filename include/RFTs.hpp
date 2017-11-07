#ifndef RFTS_HPP
#define RFTS_HPP
#include <sparsepp/spp.h>

#include <Region.hpp>

namespace rain3 {
  class RFTs {
    protected:
    spp::sparse_hash_map<uint64_t, uint16_t> HotnessCounter;
    bool Recording = false;
    Region *RecordingRegion = nullptr;
    uint16_t HotnessThreshold;

    bool isHot(uint16_t); 
    bool wasBackwardBranch(trace_io::trace_item_t&, trace_io::trace_item_t&); 

    public:
    uint32_t getNumUsedCounters() { return HotnessCounter.size(); };

    RFTs(uint16_t HT) : HotnessThreshold(HT) {};

    virtual Maybe<Region> handleNewInstruction(trace_io::trace_item_t&, trace_io::trace_item_t&, InternStateTransition) = 0;
    void restart();
  };

  class NET : public RFTs {
    public:
    NET(uint16_t HT) : RFTs(HT) {};
    Maybe<Region> handleNewInstruction(trace_io::trace_item_t&, trace_io::trace_item_t&, InternStateTransition);
  };
}
#endif
