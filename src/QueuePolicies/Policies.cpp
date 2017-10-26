#include <Policies.hpp>
#include <algorithm>
#include <iostream>

using namespace rain3;

constexpr uint8_t  NumThreads = 4;
constexpr uint16_t InterNativeRatio = 100;

std::vector<Region*> QueuePolicies::handleWaitQueueParallel(std::vector<Region*>& RegionsQueue) {
	// Start given new tickets to new regions
	for (Region* R : RegionsQueue) {
		if (R->getTicket() == 0)
			R->setTicket(R->getSize()*InterNativeRatio);
	}

	std::vector<Region*> Compiled;

	uint32_t Processed = 0;
	for (Region* R : RegionsQueue) {
		Processed += 1;
		if (Processed > NumThreads) break;

		R->setTicket(R->getTicket() - 1);
		if (R->getTicket() == 0)  
			Compiled.push_back(R);
	}

	RegionsQueue.erase(
		std::remove_if(RegionsQueue.begin(), RegionsQueue.end(), [](Region* R) -> bool { return R->isCompiled(); }),
		RegionsQueue.end());

	return Compiled;
}
