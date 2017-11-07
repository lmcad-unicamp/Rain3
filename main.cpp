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

int main() {

  // Create the input pipe.
  trace_io::raw_input_pipe_t InstStream("/home/vanderson/dev/mestrado/Rain3/input/out", 201, 203);

  std::vector<trace_io::trace_item_t> Insts;

  std::vector<rain3::Simulator*> Simulators;
  
  for (int HT = 50; HT < 10000; HT += HT)
    Simulators.push_back(new rain3::Simulator("HT"+std::to_string(HT), new rain3::PerfectIBHandler(), new rain3::QueuePolicies(), new rain3::NET(HT)));
  
  // Iterate over all the instructions
  do {
    Insts.clear();

    trace_io::trace_item_t I;
    while (InstStream.get_next_instruction(I) && Insts.size() < 1000000)
      Insts.push_back(I);

    #pragma omp parallel for
    for (uint32_t i = 0; i < Simulators.size(); i++) 
      for (auto CurrentInst : Insts)
        Simulators[i]->run(CurrentInst);
  } while (Insts.size() != 0);


  for (uint32_t i = 0; i < Simulators.size(); i++) 
    delete Simulators[i];

	return 0;
}
