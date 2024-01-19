/*
 * TRouter.hpp
 *
 *  Created on: 2019年9月4日
 *      Author: wr
 */

#ifndef TDM_TROUTER_HPP_
#define TDM_TROUTER_HPP_

#include <vector>
#include <map>
#include <utility>
#include <cassert>
#include "TDM/TInPort.hpp"
#include "TDM/TOutPort.hpp"
#include "TDM/TLink.hpp"

extern float cycles;
extern std::vector<std::vector<std::vector<int> > > tdm_routing_table;

class TInPort;
class TOutPort;

class TRouter
{
public:
  TRouter (int* t_id, int t_port_num);

  int id[2];
  int port_num;
  std::vector<TInPort*> inPort_list;
  std::vector<TOutPort*> outPort_list;
  std::map<std::pair<int, int>, int> table;

  int port_total_utilization;

  int port_standard_utilization();

  void runOneStep();

  virtual
  ~TRouter ();
};

#endif /* TDM_TROUTER_HPP_ */
