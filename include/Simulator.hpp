#include <Region.hpp>
#include <IBHandlers.hpp>
#include <Policies.hpp>
#include <SimulationStatistics.hpp>
#include <sparsepp/spp.h>
#include <trace_io.h>

#include <memory>

namespace rain3 {
  typedef std::function<Maybe<Region>(trace_io::trace_item_t&, trace_io::trace_item_t&, InternStateTransition)> RFTHandler;

  class Simulator {
    enum State { Interpreting, NativeExecuting };

    std::string FilePrefix;

    spp::sparse_hash_map<uint64_t, std::unique_ptr<Region>> RegionsCache; 
    std::vector<Region*> RegionWaitQueue;

    State   CurrentState  = State::Interpreting; 
    Region *CurrentRegion = nullptr;
    Region* LastRegion = nullptr;
    trace_io::trace_item_t LastInst;

    IBHandlers* IBH;
    QueuePolicies* QP;
    RFTs* RFT;

    SimulationStatistics Statistics;

    Region* getRegionByEntry(uint64_t Addrs) { return RegionsCache[Addrs].get(); }

    bool isWaitingCompile(uint64_t Addrs) {
      for (auto R : RegionWaitQueue)
        if (R->getEntry() == Addrs) return true;
      return false; 
    }

    bool isRegionEntrance(uint64_t Addrs) { return RegionsCache.count(Addrs) != 0; }

    InternStateTransition updateInternState(uint64_t, bool);

    void addRegion(Region* R) { RegionsCache[R->getEntry()].reset(R); }

    public:
    Simulator(std::string Prefix, IBHandlers* IB, QueuePolicies* Q, RFTs* R) {
      QP = Q;
      IBH = IB;
      RFT = R;
      FilePrefix = Prefix;
    }

    void configureRFT(InstructionSet* SC) { RFT->configure(SC, &RegionsCache); }

    ~Simulator() {
      Statistics.dumpToFile(FilePrefix + "final", true, true);
    }

    bool run(trace_io::trace_item_t);
  };
}
