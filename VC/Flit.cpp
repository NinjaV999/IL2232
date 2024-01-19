/*
 * flit.cpp
 *
 *  Created on: 2019年8月15日
 *      Author: wr
 */

#include "VC/Flit.hpp"
#include "parameters.hpp"

const int Flit::length = FLIT_LENGTH;

Flit::Flit(int t_id, int t_type, int t_vnet, int t_vc, Packet* t_packet, float t_cycles){ // ni line Flit* flit = new Flit(0, 10, vn, i, packet, cycles);
  id = t_id;// 构造函数 给类的实例的属性赋值
  type = t_type;
  vnet = t_vnet;
  vc = t_vc;
  out_port = -1;
  packet = t_packet;
  sched_time = t_cycles; //调度时间
}




