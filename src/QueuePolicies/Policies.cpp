#include <Policies.hpp>
#include <algorithm>
#include <iostream>

using namespace rain3;

#define FIFO 0
#define STACK 1
#define DYN 2
#define QueueType FIFO

std::vector<Region*> QueuePolicies::handleWaitQueueParallel(std::vector<Region*>& RegionsQueue) {
  // Start given new tickets to new regions
  for (Region* R : RegionsQueue) 
    if (R->getTicket() == 0)
      R->setTicket(R->getSize()*InterNativeRatio+1);

  std::vector<Region*> Compiled;

  uint32_t Processed = 0;
  for (Region* R : RegionsQueue) {
    if (R->isCompiling()) {
      Processed += 1;
      if (Processed > NumThreads) break;

      R->setTicket(R->getTicket() - 1);
      if (R->isCompiled())  
        Compiled.push_back(R);
    }
  }

  if (QueueType == FIFO) {
    for (Region* R : RegionsQueue) {
      if (!R->isCompiling()) {
        Processed += 1;
        if (Processed > NumThreads) break;

        R->startCompiling();
      }
    }
  } else if (QueueType == STACK) {
    while (true) {
      Region* LastR = nullptr;
      for (Region* R : RegionsQueue) {
        if (!R->isCompiling()) {
          LastR = R;
        }
      }

      Processed += 1;
      if (Processed > NumThreads) break;
      if (LastR != nullptr) LastR->startCompiling();
    }
  } else if (QueueType == DYN) {
    while (true) {
      uint32_t Maximum = 0;
      uint32_t Value = 0;
      bool found = false;
      for (uint32_t i = 0; i < RegionsQueue.size(); i++) {
        if (!RegionsQueue[i]->isCompiling()) {
          if (RegionsQueue[i]->getWeight() > Value || !found) {
            found = true;
            Maximum = i;
            Value = RegionsQueue[i]->getWeight();
          }
        }
      }

      Processed += 1;
      if (Processed > NumThreads) break;
      if (found) RegionsQueue[Maximum]->startCompiling();
    }
  }

  RegionsQueue.erase(
      std::remove_if(RegionsQueue.begin(), RegionsQueue.end(), [](Region* R) -> bool { return R->isCompiled(); }),
      RegionsQueue.end());

  return Compiled;
}
