/*
 * TNetwork.hpp
 *
 *  Created on: 2019年9月5日
 *      Author: wr
 */

#ifndef TDM_TNETWORK_HPP_
#define TDM_TNETWORK_HPP_

#include <vector>
#include "TDM/TNI.hpp"
#include "TDM/TRouter.hpp"
#include <iomanip>
#include <iostream>


class TNetwork
{
public:
  TNetwork (int t_router_num, int t_router_num_x, int t_ni_num_total, int* t_ni_num);

  int router_num, router_num_x, NI_num_total;
  int* NI_num;

  std::vector<TNI*> ni_list;
  std::vector<TRouter*> router_list;

  void queuing();

  int port_num_f(int router);
  void port_utilization(int simulate_cycles);

  void slot_utilization(int cycles);
  void port_standard_utilization();

  void runOneStep();

  virtual
  ~TNetwork ();
};

#endif /* TDM_TNETWORK_HPP_ */
