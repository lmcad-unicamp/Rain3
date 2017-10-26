#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <Region.hpp>
#include <vector>

namespace rain3 {
  namespace QueuePolicies {
    std::vector<Region*> handleWaitQueueParallel(std::vector<Region*>&);
  }
}
#endif
