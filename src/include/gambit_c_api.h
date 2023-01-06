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

int nfggnm_c(int num_players, double *pay_off_data, int data_length, int *num_strats, int number_of_perturbations, double **equilibriums_buffer, int &equilibriums_buffer_size, int &number_of_equilibriums);



#ifdef __cplusplus
}
#endif
#endif //GAMBIT_C_API_H
