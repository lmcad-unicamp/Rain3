#include <IBHandlers.hpp>
#include <Region.hpp>

using namespace rain3;


bool PerfectIBHandler::handleIB(trace_io::trace_item_t&, uint64_t, Region*) {
  return true;
}

bool OneIfIBHandler::handleIB(trace_io::trace_item_t& IB, uint64_t RealTarget, Region* R) {
  if (R == nullptr) {
    LastTarget[IB.addr] = RealTarget;
    return true; 
  } else {
    if (OneIfHash.count(IB.addr + R->getEntry()) == 0) 
      OneIfHash[IB.addr + R->getEntry()] = LastTarget[IB.addr]; 
    return OneIfHash[IB.addr + R->getEntry()] == RealTarget;
  }
}

void IBHandlers::restart() {
  LastTarget.clear();
  OneIfHash.clear();
}
