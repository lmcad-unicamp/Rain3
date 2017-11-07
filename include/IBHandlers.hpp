#ifndef IBHANDLERS_HPP
#define IBHANDLERS_HPP

#include <sparsepp/spp.h>
#include <trace_io.h>
#include <Region.hpp>

namespace rain3 {
  class IBHandlers {
    protected:
    spp::sparse_hash_map<uint64_t, uint64_t> LastTarget; 
    spp::sparse_hash_map<uint64_t, uint64_t> OneIfHash; 

    public:
    virtual bool handleIB(trace_io::trace_item_t&, uint64_t, Region*) = 0;
    void restart();
  };

  struct PerfectIBHandler : public IBHandlers {
    bool handleIB(trace_io::trace_item_t&, uint64_t, Region*);
  };

  struct OneIfIBHandler : public IBHandlers {
    bool handleIB(trace_io::trace_item_t&, uint64_t, Region*);
  };
}

#endif
