#ifndef REGION_HPP
#define REGION_HPP

#include <algorithm>
#include <vector>
#include <map>

#include <trace_io.h>

namespace rain3 {
  class InstructionSet { 
    private:
      std::map<uint64_t, trace_io::trace_item_t> Instructions;

    public:
      std::map<uint64_t, trace_io::trace_item_t>::const_iterator find(uint64_t addrs) const {
        return Instructions.find(addrs);
      }

      std::map<uint64_t, trace_io::trace_item_t>::const_iterator getEnd() const {
        return Instructions.end();
      }

      unsigned long long getOpcode(uint64_t addrs) const {
        return Instructions.at(addrs).opcode;
      }

      bool hasInstruction(uint64_t addrs) const {
        return Instructions.count(addrs) != 0;
      }

      void addInstruction(uint64_t addrs, trace_io::trace_item_t inst) {
        Instructions[addrs] = inst;
      }

			trace_io::trace_item_t getTraceItem(uint64_t Addrs) {
				return Instructions[Addrs];
			}

      size_t size() {
        return Instructions.size();
      }
  };

  enum InternStateTransition { InterToNative, NativeToNative, NativeToInter, StayedNative, StayedInter};

  template <class X> class Maybe {
    bool Empty;
    X* Val;

    public:
    Maybe() : Empty(true) {}
    Maybe(X* V) : Empty(false), Val(V) {}
    static Maybe<X> Nothing() { return Maybe<X>(); }
    bool isNothing() { return Empty; }
    X* get() { return Val; }
  };

  class Region {
    private: 
      uint64_t EntryAddrs, MainExitAddrs;

      uint32_t Ticket = 0;

      bool Compiling = false;
      uint32_t Weight = 0;
    public:
      std::vector<uint64_t> Instructions;

      void addAddress(uint64_t NewAddress) {
        Instructions.push_back(NewAddress);
      };

      bool hasAddress(uint64_t NewAddress) {
        return std::find(Instructions.begin(), Instructions.end(), NewAddress) != Instructions.end();
      };

      void setTicket(uint32_t t) { Ticket = t; }

      uint32_t getTicket() { return Ticket; }

      bool isCompiled() { return Ticket == 0; }

      uint32_t getSize() { return Instructions.size(); };

      void startCompiling() { Compiling = true; }
      bool isCompiling() { return Compiling; }

      void increaseWeight() { Weight++; }
      uint32_t getWeight() { return Weight; } 

      uint64_t getEntry()                  { return EntryAddrs; }
      void     setEntry(uint64_t Addrs)    { EntryAddrs = Addrs; }
      void     setMainExit(uint64_t Addrs) { MainExitAddrs = Addrs; }
      bool     isEntry(uint64_t Addrs)     { return Addrs == EntryAddrs; }
      bool     isMainExit(uint64_t Addrs)  { return Addrs == MainExitAddrs; }
  };
}
#endif
