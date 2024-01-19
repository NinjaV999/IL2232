/*
 * TNI.hpp
 *
 *  Created on: 2019年9月4日
 *      Author: wr
 */

#ifndef TDM_TNI_HPP_
#define TDM_TNI_HPP_

#include <vector>
#include <deque>
#include "TDM/TOutPort.hpp"
#include "TDM/BlockBuffer.hpp"
#include "TDM/Signal.hpp"
#include "parameters.hpp"

extern float cycles;
extern std::vector<std::vector<int> > tdm_NI_table;

class BlockBuffer;
class Signal;

class TNI
{
public:
  TNI (int id, int ni_num);

  int id;
  int ni_num;

  std::deque<Signal*> signal_buffer;
  std::vector<std::deque<Signal*> > signal_buffer_out; // 0 request; 1 response;

  std::vector<BlockBuffer*> buffer_list;
  TOutPort* outPort;
  TInPort* inPort;

  static int count_s, count_r;
  static int count_s_res, count_r_res;

  int slot_utilization;
  int available_slot();

  float queuing_delay_sum;
  int num_sum;

  static int GRS_latency_single_dis[100];
  static int GRS_latency_single_count;
  static int GRS_latency_single_total;
  static int GRS_latency_single_worst;

  static int converse_latency_single_dis[100];
  static int converse_latency_single_count;
  static int converse_latency_single_total;
  static int converse_latency_single_worst;


  void runOneStep();

  virtual
  ~TNI ();
};

#endif /* TDM_TNI_HPP_ */
