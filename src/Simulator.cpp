#include <Simulator.hpp>
#include <functional>

#include <iostream>

using namespace rain3;

#define CongestionControl

InternStateTransition Simulator::updateInternState(uint64_t NextAddrs, bool ForceNativeExiting) {
  // Entrying Region: Interpreter -> NativeExecuting
  if (CurrentState == State::Interpreting && isRegionEntrance(NextAddrs) && !ForceNativeExiting) {
    CurrentState  = State::NativeExecuting;
    CurrentRegion = getRegionByEntry(NextAddrs); 
    return InternStateTransition::InterToNative;
  } 
  // Exiting Region: NativeExecution -> Dispatcher
  else if (CurrentState == State::NativeExecuting && (!CurrentRegion->hasAddress(NextAddrs) || ForceNativeExiting)) {
    // Region Transition: NativeExecution (Dispatcher) -> NativeExecution
    if (isRegionEntrance(NextAddrs)) {
      LastRegion    = CurrentRegion;
      CurrentRegion = getRegionByEntry(NextAddrs);
      return InternStateTransition::NativeToNative;
    } 
    // Exiting Native Execution: NativeExecution (Dispatcher) -> Interpreter
    else {
      CurrentState  = State::Interpreting;
      LastRegion    = CurrentRegion;
      CurrentRegion = nullptr;
      return InternStateTransition::NativeToInter;
    }
  } else {
    if (CurrentState == State::Interpreting) return InternStateTransition::StayedInter;
    else return InternStateTransition::StayedNative;
  }
}

bool Simulator::run(trace_io::trace_item_t CurrentInst) {
  // Simulate Indirect Branch Handlers
  bool ForceNativeExiting = false;
  if (LastInst.is_indirect_branch_inst() && (LastInst.addr + LastInst.length) != CurrentInst.addr) {
    bool WasAbleToTranslate = IBH->handleIB(LastInst, CurrentInst.addr, CurrentRegion);
    if (CurrentState == NativeExecuting) {
      ForceNativeExiting = !WasAbleToTranslate;
      Statistics.indirectBranching();

      if (ForceNativeExiting)
        Statistics.missedIndirectAddrsTranslation();
    }
  }

  auto LastStateTransition = updateInternState(CurrentInst.addr, ForceNativeExiting);

  if (CurrentState == Interpreting && isWaitingCompile(CurrentInst.addr)) {
    for (auto R : RegionWaitQueue) { 
      if (R->getEntry() == CurrentInst.addr) {
        R->increaseWeight();
        break;
      }
    }
  }

  // Only handle when not in native execution
  if (CurrentState == Interpreting && !isWaitingCompile(CurrentInst.addr)) {
    Maybe<Region> MayRegion = RFT->handleNewInstruction(LastInst, CurrentInst, LastStateTransition);

    // If a region was indeed created, then call the region queue handler and update the statistics
    if (!MayRegion.isNothing()) { 
      RegionWaitQueue.push_back(MayRegion.get());	
      Statistics.addRegionInfo(MayRegion.get(), RegionWaitQueue.size());

#ifdef CongestionControl      
      double Ratio = ((double)RegionWaitQueue.size()) / QP->NumThreads;
      if (Ratio < 0.5) 
        RFT->setHotnessThreshold((RFT->getHotnessThreshold()/2)+1); 
      else if (Ratio < 1)
        RFT->setHotnessThreshold(RFT->getHotnessThreshold()*0.9 + 1); 
      else if (Ratio > 4)
        RFT->setHotnessThreshold(RFT->getHotnessThreshold()*2); 
      else if (Ratio > 2)
        RFT->setHotnessThreshold(RFT->getHotnessThreshold()*1.1); 
#endif
    }
  }

  std::vector<Region*> Compiled = QP->handleWaitQueueParallel(RegionWaitQueue);

  for (Region* R : Compiled) { 
    addRegion(R);

    if (R->getEntry() == CurrentInst.addr) {
      LastRegion = nullptr;
      LastStateTransition = updateInternState(CurrentInst.addr, false);
    }
  }

  Statistics.updateData(FilePrefix, CurrentInst.addr, LastInst.addr, RFT->getNumUsedCounters(), LastStateTransition, 
      CurrentRegion, LastRegion, RegionWaitQueue.size());

  LastInst   = CurrentInst;
  LastRegion = CurrentRegion;
  return true;
}
