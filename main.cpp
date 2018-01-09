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

int main(int argv, char** argc) {
  uint64_t addr_limit = 0;
  // Create the input pipe.
  trace_io::raw_input_pipe_t InstStream(argc[1], atoi(argc[2]), atoi(argc[3]));

  if (std::string(argc[4]) == "spec") addr_limit = 0xB2D05E00;
  else addr_limit = 0xF9CCD8A1C5080000;

  rain3::InstructionSet StaticCode;

  std::vector<trace_io::trace_item_t> Insts;

  std::vector<rain3::Simulator*> Simulators;

  // ---------------------------- NET ------------------------- //
  for (int Relaxed = 0; Relaxed < 2; Relaxed++) {
    for (int HT = 32; HT < 17000; HT *= 2) {
      auto Sim = new rain3::Simulator(std::string(argc[1])+"-NET-HT?"+std::to_string(HT)+"-Relaxed?"+std::to_string(Relaxed), 
          new rain3::PerfectIBHandler(), new rain3::QueuePolicies(1, 0), new rain3::NET(HT, Relaxed));

      Sim->configureRFT(&StaticCode);

      Simulators.push_back(Sim);
    }
  }

  // ---------------------------- MRET2 ------------------------- //
  for (int Relaxed = 0; Relaxed < 2; Relaxed++) {
    for (int HT = 32; HT < 17000; HT *= 2) {
      auto Sim = new rain3::Simulator(std::string(argc[1])+"MRET2-HT?"+std::to_string(HT)+"-Relaxed?"+std::to_string(Relaxed), 
          new rain3::PerfectIBHandler(), new rain3::QueuePolicies(1, 0), new rain3::MRET2(HT, Relaxed));

      Sim->configureRFT(&StaticCode);

      Simulators.push_back(Sim);
    }
  }

  // ---------------------------- NETPlus ------------------------- //
  for (int Depth = 4; Depth < 14; Depth += 2) {
    for (int Extended = 0; Extended < 2; Extended++) {
      for (int Relaxed = 0; Relaxed < 2; Relaxed++) {
        for (int HT = 32; HT < 17000; HT *= 2) {
          auto Sim = new rain3::Simulator(std::string(argc[1])+"NETPlus-HT?"+std::to_string(HT)+"-Relaxed?"+std::to_string(Relaxed)+"-Extended?"+std::to_string(Extended)+"-depth?"+std::to_string(Depth), 
              new rain3::PerfectIBHandler(), new rain3::QueuePolicies(1, 0), new rain3::NETPlus(HT, Relaxed, Extended, Depth));

          Sim->configureRFT(&StaticCode);

          Simulators.push_back(Sim);
        }
      }
    }
  }

  // ---------------------------- LEI ------------------------- //
  for (int HT = 32; HT < 33000; HT *= 2) {
    auto Sim = new rain3::Simulator(std::string(argc[1])+"LEI-HT?"+std::to_string(HT), 
        new rain3::PerfectIBHandler(), new rain3::QueuePolicies(1, 0), new rain3::LEI(HT*0.7));

    Sim->configureRFT(&StaticCode);

    Simulators.push_back(Sim);
  }

  std::cout << "We are going to simulate " << Simulators.size() << " DBTs configurations with a " << argc[4] << "(" << addr_limit << ") bench.\n";

  uint32_t TotalInstSimulated = 0;
  // Iterate over all the instructions
  do {
    Insts.clear();

    trace_io::trace_item_t I;
    while (InstStream.get_next_instruction(I) && Insts.size() < 10000000) {
      if (I.addr < addr_limit) {
        Insts.push_back(I);
        StaticCode.addInstruction(I.addr, I.opcode);
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
