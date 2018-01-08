/***************************************************************************
 *   Copyright (C) 2017 by:                                                *
 *   Vanderson M. Rosario (vandersonmr2@gmail.com)                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <arglib.h>
#include <trace_io.h>

#include <Simulator.hpp>
#include <RFTs.hpp>
#include <Policies.hpp>
#include <IBHandlers.hpp>
#include <Region.hpp>

//#define addr_limit 0xF9CCD8A1C5080000 
#define addr_limit 0xB2D05E00

#define USES_STATIC

int main(int argv, char** argc) {
  // Create the input pipe.
  trace_io::raw_input_pipe_t InstStream(argc[1], atoi(argc[2]), atoi(argc[3]));

  rain3::InstructionSet StaticCode;

  std::vector<trace_io::trace_item_t> Insts;

  std::vector<rain3::Simulator*> Simulators;

  for (int Relaxed = 0; Relaxed < 2; Relaxed++) {
    for (int HT = 50; HT < 20000; HT += HT/2) {
      auto Sim = new rain3::Simulator(std::string(argc[1])+"HT"+std::to_string(HT)+"Relaxed?"+std::to_string(Relaxed), 
          new rain3::PerfectIBHandler(), new rain3::QueuePolicies(1, 0), new rain3::NETPlus(HT, Relaxed, true));

#ifdef USES_STATIC
      Sim->configureRFT(&StaticCode);
#endif

      Simulators.push_back(Sim);
    }
  }

  uint32_t TotalInstSimulated = 0;
  // Iterate over all the instructions
  do {
    Insts.clear();

    trace_io::trace_item_t I;
    while (InstStream.get_next_instruction(I) && Insts.size() < 1000000) {
      if (I.addr < addr_limit) {
        Insts.push_back(I);
      
#ifdef USES_STATIC
        StaticCode.addInstruction(I.addr, I.opcode);
#endif
      }
    }

    #pragma omp parallel for
    for (uint32_t i = 0; i < Simulators.size(); i++) 
      for (auto CurrentInst : Insts)
        Simulators[i]->run(CurrentInst);

    TotalInstSimulated += Insts.size();
  } while (Insts.size() != 0 && TotalInstSimulated < 2500000000);


  for (uint32_t i = 0; i < Simulators.size(); i++) 
    delete Simulators[i];

	return 0;
}
