#include <RFTs.hpp>

constexpr uint16_t HotnessThreshold = 50;

bool rain3::RFTs::isHot(uint16_t Hotness) {
  return Hotness > HotnessThreshold;
}

bool rain3::RFTs::wasBackwardBranch(trace_io::trace_item_t& LastInst, trace_io::trace_item_t& CurrentInst) {
  return LastInst.addr > CurrentInst.addr;
}

