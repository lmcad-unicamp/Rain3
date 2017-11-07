#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <Region.hpp>
#include <vector>

namespace rain3 {
  class QueuePolicies {
    public:
      uint8_t  NumThreads = 1;
      uint32_t InterNativeRatio = 50;

      std::vector<Region*> handleWaitQueueParallel(std::vector<Region*>&);
  };
}
#endif
