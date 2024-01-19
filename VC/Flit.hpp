/*
 * flit.hpp
 *
 *  Created on: 2019年8月15日
 *      Author: wr
 */

#ifndef VC_FLIT_HPP_
#define VC_FLIT_HPP_

#include "VC/Packet.hpp"

class Flit{
public:


  /*
   * @brief set values to be the same as input
   *  id = t_id;
  type = t_type;
  vnet = t_vnet;
  vc = t_vc;
  out_port = -1;
  packet = t_packet;
  sched_time = t_cycles;
   */
  Flit(int t_id, int t_type, int t_vnet, int t_vc, Packet* t_packet, float t_cycles);

  const static int length;  // length in byte
  int id;   // The sequence id in a packet
  int type; // 0 -> head; 1 -> tail; 2 -> body; 10 -> head_tail;
  //flit 有四种类型
  int vnet;
  int vc;
  int out_port;
  float sched_time; // if sched_time < cur_time, then the flit can be transferred.
//如果 sched——time《cur time 则 该 flit 可以被传送
  //add
  int flit_iniCycle= -1;//
  //add end
  Packet * packet;
};



#endif /* VC_FLIT_HPP_ */
