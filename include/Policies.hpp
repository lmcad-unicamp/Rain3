#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <Region.hpp>
#include <vector>

namespace rain3 {
  class QueuePolicies {
    private:
      uint8_t  NumThreads = 1;
      uint32_t InterNativeRatio = 50;

    public:
      QueuePolicies(uint8_t NT, uint32_t INR) : NumThreads(NT), InterNativeRatio(INR) {};

      std::vector<Region*> handleWaitQueueParallel(std::vector<Region*>&);
  };
}
#endif
