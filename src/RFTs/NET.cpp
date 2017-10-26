#include <sparsepp/spp.h>
#include <RFTs.hpp>

using namespace rain3;

spp::sparse_hash_map<uint64_t, uint16_t> HotnessCounter;
bool Recording          = false;
Region *RecordingRegion = nullptr;

Maybe<Region> RFTs::
handleNewInstructionWithNET(trace_io::trace_item_t& LastInst, trace_io::trace_item_t& CurrentInst, InternStateTransition LastStateTransition) {
  if (Recording) {
    if (wasBackwardBranch(LastInst, CurrentInst) || LastStateTransition == InterToNative)
      Recording = false;

    if (Recording)
      RecordingRegion->addAddress(CurrentInst.addr);
    else
      return Maybe<Region>(RecordingRegion);
  } else {
    if ((LastStateTransition == StayedInter && wasBackwardBranch(LastInst, CurrentInst)) || LastStateTransition == NativeToInter) {
      if (HotnessCounter.count(CurrentInst.addr) == 0) HotnessCounter[CurrentInst.addr] = 1;
      else HotnessCounter[CurrentInst.addr] += 1;

      if (isHot(HotnessCounter[CurrentInst.addr])) {
        Recording = true;
        RecordingRegion = new Region();
        RecordingRegion->setEntry(CurrentInst.addr);
        HotnessCounter[CurrentInst.addr] = 0;
      }
    }
  }
  return Maybe<Region>::Nothing(); 
}
