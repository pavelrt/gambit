//
//  nfggnm_c_api.cpp
//  
//
//  Created by Pavel Rytir on 1/6/23.
//

#include <iostream>
#include "gambit.h"
#include "solvers/gnm/gnm.h"

using namespace Gambit;
using namespace Gambit::Nash;

List<MixedStrategyProfile<double> >
RandomStrategyPerturbations(const Game &p_game, int p_count)
{
  List<MixedStrategyProfile<double> > profiles;
  for (int i = 1; i <= p_count; i++) {
    MixedStrategyProfile<double> p(p_game->NewMixedStrategyProfile(0.0));
    p.Randomize();
    profiles.push_back(p);
  }
  return profiles;
}

int nfggnm_c(int num_players, double *pay_off_data, int data_length, int *num_strats, int number_of_perturbations, double **equilibriums_buffer, int *equilibriums_buffer_size, int *number_of_equilibriums) {
  bool quiet = false, verbose = false;
  try {
    Array<int> dim(num_players);
    for (int pl = 1; pl <= dim.Length(); pl++) {
      dim[pl] = num_strats[pl-1];
    }
    GameRep *nfg = NewTable(dim);
    // Assigning this to the container assures that, if something goes
    // wrong, the class will automatically be cleaned up
    Game game = nfg;
    nfg->SetTitle("NA");
    nfg->SetComment("NA");
    
    StrategyProfileIterator iter(StrategySupportProfile(static_cast<GameRep *>(nfg)));
    int pl = 1;
    
    for (int i = 0; i < data_length; i++) {
      std::string currentNumber = std::to_string(pay_off_data[i]);
      (*iter)->GetOutcome()->SetPayoff(pl, currentNumber);
      if (++pl > nfg->NumPlayers()) {
        iter++;
        pl = 1;
      }
    }
    
    shared_ptr<StrategyProfileRenderer<double> > renderer;
    renderer = new MixedStrategyNullRenderer<double>();
    NashGNMStrategySolver solver(renderer, verbose);
    
    // Generate the desired number of points randomly
    List<MixedStrategyProfile<double> > perts = RandomStrategyPerturbations(game, number_of_perturbations);
    
    std::vector<MixedStrategyProfile<double>> equilibriumsFound;
    for (int i = 1; i <= perts.size(); i++) {
      auto equilibriumsFoundIter = solver.Solve(game, perts[i]);
      for (int i = 0; i < equilibriumsFoundIter.size(); i++) {
      //for (const auto eq: equilibriumsFoundIter) {
        equilibriumsFound.push_back(equilibriumsFoundIter[i]);
      }
    }
    
    *number_of_equilibriums = (int) equilibriumsFound.size();
    std::vector<double> equilibriums_data;
    for (const auto eq: equilibriumsFound) {
      for (const auto player: eq.GetGame()->Players()) {
        for (const auto strategy: player->Strategies()) {
          equilibriums_data.push_back(eq.GetPayoff(strategy));
        }
      }
    }
    *equilibriums_buffer_size = (int) equilibriums_data.size();
    *equilibriums_buffer = (double *) malloc(*equilibriums_buffer_size * sizeof(double));
    int i = 0;
    for (const auto value: equilibriums_data) {
      *equilibriums_buffer[i] = value;
      i++;
    }
    return 0;
  }
  catch (std::runtime_error &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
