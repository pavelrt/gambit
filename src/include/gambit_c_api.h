//
// Created by xankr on 28.02.2022.
//
//#include <functional>
#ifndef GAMBIT_C_API_H
#define GAMBIT_C_API_H
#ifdef __cplusplus
extern "C"
{
#endif

double* nfggnm_c(const int num_players, const double *pay_off_data, const int data_length, const int *num_strats, const int number_of_perturbations, int *equilibriums_buffer_size, int *number_of_equilibriums);



#ifdef __cplusplus
}
#endif
#endif //GAMBIT_C_API_H
