#include <Simulator.hpp>
#include <functional>

#include <iostream>

using namespace rain3;

InternStateTransition Simulator::updateInternState(uint64_t NextAddrs) {
  // Entrying Region: Interpreter -> NativeExecuting
  if (CurrentState == State::Interpreting && isRegionEntrance(NextAddrs)) {
    CurrentState  = State::NativeExecuting;
    CurrentRegion = getRegionByEntry(NextAddrs); 
    return InternStateTransition::InterToNative;
  } 
  // Exiting Region: NativeExecution -> Dispatcher
  else if (CurrentState == State::NativeExecuting && !CurrentRegion->hasAddress(NextAddrs)) {
    // Region Transition: NativeExecution (Dispatcher) -> NativeExecution
    if (isRegionEntrance(NextAddrs)) {
      CurrentRegion = getRegionByEntry(NextAddrs);
      return InternStateTransition::NativeToNative;
    } 
    // Exiting Native Execution: NativeExecution (Dispatcher) -> Interpreter
    else {
      CurrentState  = State::Interpreting;
      CurrentRegion = nullptr;
      return InternStateTransition::NativeToInter;
    }
  } else {
    if (CurrentState == State::Interpreting) return InternStateTransition::StayedInter;
    else return InternStateTransition::StayedNative;
  }
}

bool Simulator::run(trace_io::raw_input_pipe_t InstStream, RFTHandler RFTFunc, WaitQueueHandler WaitQueueFunc) {
  // Current instruction.
  Region* LastRegion = nullptr;
  trace_io::trace_item_t LastInst, CurrentInst;

  // Fetch the next instruction from the trace
  if (!InstStream.get_next_instruction(LastInst) || !InstStream.get_next_instruction(CurrentInst)) {
    cerr << "Error: input trace has no instruction items." << endl;
    return false;
  } 

  // Iterate over all the instructions
  do {
    auto LastStateTransition = updateInternState(CurrentInst.addr);

    // Only handle when not in native execution
    if ((LastStateTransition == NativeToInter || LastStateTransition == StayedInter) && !isWaitingCompile(CurrentInst.addr)) {
      Maybe<Region> MayRegion = RFTFunc(LastInst, CurrentInst, LastStateTransition);

      // If a region was indeed created, then call the region queue handler and update the statistics
      if (!MayRegion.isNothing()) { 
				RegionWaitQueue.push_back(MayRegion.get());	
        Statistics.addRegionInfo(MayRegion.get(), RegionWaitQueue.size());
			}
    }

		std::vector<Region*> Compiled = WaitQueueFunc(RegionWaitQueue);

		for (Region* R : Compiled) { 
			addRegion(R);
		}

    Statistics.updateData(CurrentInst.addr, LastInst.addr, LastStateTransition, CurrentRegion, LastRegion);

    LastInst   = CurrentInst;
    LastRegion = CurrentRegion;
  } while (InstStream.get_next_instruction(CurrentInst));

  return true;
}
