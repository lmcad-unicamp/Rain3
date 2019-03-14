#include <RFTs.hpp>

#include <queue>
#include <iostream>

using namespace rain3;

void NETPlus::expand(Region* R) {
	std::queue<uint64_t> s;
	std::unordered_map<uint64_t, uint32_t> distance;
	std::unordered_map<uint64_t, uint64_t> next, parent;

	// Init BFS frontier
	for (auto Addrs : R->Instructions) {
		if (StaticCode->getTraceItem(Addrs).is_branch_inst()) {
			s.push(Addrs);
			distance[Addrs] = 0;
			parent[Addrs] = 0;
		}
	}

	while (!s.empty()) {
		unsigned long long current = s.front();
		s.pop();

		if (distance[current] < DepthLimit) {

			auto targets = StaticCode->getTraceItem(current).getPossibleNextAddrs();

			for (auto target : targets) {
				if (parent.count(target) != 0) continue;

				parent[target] = current;
				// Iterate over all instructions between the target and the next branch
				auto it = StaticCode->find(target);

				while (it != StaticCode->getEnd()) {
					bool isCycle = (R->isEntry(it->first) && !Extended) || (R->hasAddress(it->first) && Extended);

					if ((isCycle) && distance[current] > 0) {
						std::vector<uint64_t> newpath;
						unsigned long long begin = it->first;
						unsigned long long prev = target;
						while (true) {
							auto it = StaticCode->find(begin);
							while (true) {
								newpath.push_back(it->first);
								if (it->first == prev) break;
								--it;
							}
							begin = parent[prev];
							prev  = next[begin];
							if (prev == 0) {
								newpath.push_back(begin);
								break;
							}
						}

						for (auto NewAddrs : newpath) 
							if (!R->hasAddress(NewAddrs))
								R->addAddress(NewAddrs);

						break;
					}

					if (RegionsCache->count(it->first) != 0)
						break;

					if (StaticCode->getTraceItem(it->first).is_branch_inst() && distance.count(it->first) == 0) {
						s.push(it->first);
						distance[it->first] = distance[current] + 1;
						next[it->first] = target;
						break;
					}

					++it;
				}
			}
		}
	}
}

Maybe<Region> NETPlus::handleNewInstruction(trace_io::trace_item_t& LastInst, trace_io::trace_item_t& CurrentInst, InternStateTransition LastStateTransition) {
	if (Recording) {
		if ((wasBackwardBranch(LastInst, CurrentInst)      && !Relaxed) ||
				(RecordingRegion->hasAddress(CurrentInst.addr) &&  Relaxed) || 
				RecordingRegion->getSize() > 200 || LastStateTransition == InterToNative)
			Recording = false;

		if (Recording) {
			RecordingRegion->addAddress(CurrentInst.addr);
		} else {
			RecordingRegion->setMainExit(LastInst.addr);
			expand(RecordingRegion);
			return Maybe<Region>(RecordingRegion);
		}
	} else {
		if ((LastStateTransition == StayedInter && wasBackwardBranch(LastInst, CurrentInst)) || LastStateTransition == NativeToInter) {
			HotnessCounter[CurrentInst.addr] += 1;

			if (isHot(HotnessCounter[CurrentInst.addr])) {
				Recording = true;
				RecordingRegion = new Region();
				RecordingRegion->addAddress(CurrentInst.addr);
				RecordingRegion->setEntry(CurrentInst.addr);
				HotnessCounter[CurrentInst.addr] = 0;
			}
		}
	}
	return Maybe<Region>::Nothing(); 
}
