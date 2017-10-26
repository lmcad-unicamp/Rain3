#include <Region.hpp>
#include <SimulationStatistics.hpp>
#include <sparsepp/spp.h>
#include <trace_io.h>

#include <memory>

namespace rain3 {
  typedef std::function<Maybe<Region>(trace_io::trace_item_t&, trace_io::trace_item_t&, InternStateTransition)> RFTHandler;
  typedef std::function<std::vector<Region*>(vector<Region*>&)> WaitQueueHandler;

  class Simulator {
      enum State { Interpreting, NativeExecuting };

      spp::sparse_hash_map<uint64_t, std::unique_ptr<Region>> RegionsCache; 
			std::vector<Region*> RegionWaitQueue;

      State   CurrentState  = State::Interpreting; 
      Region *CurrentRegion = nullptr;

      SimulationStatistics Statistics;

      Region* getRegionByEntry(uint64_t Addrs) { return RegionsCache[Addrs].get(); }

			bool isWaitingCompile(uint64_t Addrs) {
				for (auto R : RegionWaitQueue)
					if (R->getEntry() == Addrs) return true;
				return false; 
			}

      bool isRegionEntrance(uint64_t Addrs) { return RegionsCache.count(Addrs) != 0; }

      InternStateTransition updateInternState(uint64_t);

      void addRegion(Region* R) { RegionsCache[R->getEntry()].reset(R); }

    public:
      bool run(trace_io::raw_input_pipe_t, RFTHandler, WaitQueueHandler);
  };
}
