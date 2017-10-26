#include <Region.hpp>
#include <sparsepp/spp.h>
#include <trace_io.h>
#include <iostream>

namespace rain3 {
  class SimulationStatistics {
    spp::sparse_hash_map<uint64_t, uint32_t> RegionsInterEntryCounter, RegionsNativeEntryCounter, RegionsTotalExecution,
      RegionsTotalMainExits, RegionSpannedFreq; 

    uint32_t TotalNumberOfRegions = 0;
    uint32_t NumberOfCompiledInstructions = 0;
    uint32_t InterTotalExecution = 0;

    void increaseCounter(uint64_t CurrentAddr, spp::sparse_hash_map<uint64_t, uint32_t> Counter) {
        if (Counter.count(CurrentAddr) == 0) Counter[CurrentAddr] = 1; 
        else Counter[CurrentAddr] += 1;
    }

    public:
		~SimulationStatistics() {
			std::cout << InterTotalExecution << "\n";
		}

    void updateData(uint64_t CurrentAddr, uint64_t LastAddr, 
                    InternStateTransition StateTransition, Region* CurrentRegion, Region* LastRegion) {
      if (StateTransition == InterToNative) {
        // Count the total number of entries in a region comming from the interpreter
        increaseCounter(CurrentAddr, RegionsInterEntryCounter); 
      } else if (StateTransition == NativeToNative) {
        // Count the total number of entries in a region comming from other regions
        increaseCounter(CurrentAddr, RegionsNativeEntryCounter); 
      } else if (StateTransition == StayedNative) {
        // Count the total number of entries in a region comming from other regions
        increaseCounter(CurrentRegion->getEntry(), RegionsTotalExecution); 

        if (CurrentRegion->isEntry(CurrentAddr))
          increaseCounter(CurrentAddr, RegionsTotalMainExits); 
      } else if (StateTransition == NativeToInter) {
        InterTotalExecution += 1;
        // Count the total amount of main exities
        if (LastRegion->isMainExit(LastAddr)) 
          increaseCounter(LastRegion->getEntry(), RegionsTotalMainExits); 
      } else if (StateTransition == StayedInter) {
        InterTotalExecution += 1;
      }
    }

		uint32_t Max = 0;
    void addRegionInfo(Region* R, uint32_t RegionWaitQueueSize) {
      TotalNumberOfRegions += 1;
      NumberOfCompiledInstructions += R->getSize();
			if (RegionWaitQueueSize > Max) {
				Max = RegionWaitQueueSize;
				std::cout <<  Max << "\n";
			}
    }
  };
}
