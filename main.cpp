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

clarg::argString TracePathFlag("-f", "trace file name", "");
clarg::argInt StartIdxFlag("-sidx", "Start number of trace file name index", 0);
clarg::argInt EndIdxFlag("-eidx", "End number of trace file name index", 0);

clarg::argBool HelpFlag("-h", "display the help message");

clarg::argInt HotnessFlag("-hot", "Hotness threshold for the RFTs", 50);
clarg::argString RFTFlag("-rft", "Region Formation Technique name", "net");

void usage(char* PrgName) {
  cout << "Version: 1.0 (01-01-2019)";
  #ifdef DEBUG
  cout << " (Debug Build) ";
  #endif
  cout << "\n\n";
  cout << "Usage: " << PrgName <<
    " [-f FileName [-fidx startIndex] [-eidx endIndex]\n\n";

  cout << "DESCRIPTION:\n";
  cout << "This program implements the RAIn DBT Simulator\n" <<
    "Vanderson Martins do Rosario <vandersonmr2@gmail.com>, 2018.\n\n";

  cout << "ARGUMENTS:\n";
  clarg::arguments_descriptions(cout, "  ", "\n");
}

int validateArguments() {
  if (!TracePathFlag.was_set()) {
    cerr << "You should provide path for a trace file (-f)!\n";
    return 1;
  }

  return 0;
}


int main(int argc, char** argv) {
  // Linux kernle address limit
  uint64_t addr_limit = 0xB2D05E00;

  if (clarg::parse_arguments(argc, argv)) {
    cerr << "Error when parsing the arguments!" << endl;
    return 1;
  }

  if (HelpFlag.get_value() == true) {
    usage(argv[0]);
    return 1;
  }

  if (validateArguments())
    return 1;

  // Create the input pipe.
  trace_io::raw_input_pipe_t InstStream(TracePathFlag.get_value(), StartIdxFlag.get_value(), EndIdxFlag.get_value());

  rain3::InstructionSet StaticCode;

  std::vector<trace_io::trace_item_t> Insts;

  std::vector<rain3::Simulator*> Simulators;

  int HT = HotnessFlag.get_value();

  std::cout << "Simulating a DBT configuration: \n";

  rain3::RFTs* RFT;
  if      (RFTFlag.get_value() == "net") {
    RFT = new rain3::NET(HT, false);
    std::cout << "NET Select with threshold " << HT << "\n";
  } else if (RFTFlag.get_value() == "mret2") {
    RFT = new rain3::MRET2(HT, false);
    std::cout << "MRET2 Select with threshold " << HT << "\n";
  } else if (RFTFlag.get_value() == "net-r") {
    RFT = new rain3::NET(HT, true);
    std::cout << "NET-R Select with threshold " << HT << "\n";
  } else if (RFTFlag.get_value() == "netplus") {
    RFT = new rain3::NETPlus(HT, false, false);
    std::cout << "NETPlus Select with threshold " << HT << "\n";
  } else if (RFTFlag.get_value() == "netplus-e-r") {
    RFT = new rain3::NETPlus(HT, true, true);
    std::cout << "NETPlus-e-r Select with threshold " << HT << "\n";
  }

  auto Sim = new rain3::Simulator("result", 
      new rain3::OneIfIBHandler(), 
      new rain3::QueuePolicies(1, 0), 
      RFT);
  Sim->configureRFT(&StaticCode);

  uint32_t TotalInstSimulated = 0;
  // Iterate over all the instructions
  do {
    Insts.clear();

    trace_io::trace_item_t I;
    while (InstStream.get_next_instruction(I) && Insts.size() < 1000000000) {
      if (I.addr < addr_limit) {
        Insts.push_back(I);
        StaticCode.addInstruction(I.addr, I);
      }
    }

    for (auto CurrentInst : Insts)
      Sim->run(CurrentInst);

    std::cout << "Simulated " << TotalInstSimulated << " instructions...\n";

    TotalInstSimulated += Insts.size();
  } while (Insts.size() != 0 && TotalInstSimulated < 2500000000);

  delete Sim;

  return 0;
}
