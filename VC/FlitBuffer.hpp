/*
 * flit_buffer.hpp
 *
 *  Created on: 2019年8月15日
 *      Author: wr
 */

#ifndef VC_FLITBUFFER_HPP_
#define VC_FLITBUFFER_HPP_

#include "VC/Flit.hpp"
#include <deque>

using namespace std;

class FlitBuffer{
public:
  FlitBuffer(int t_vc, int t_vnet, int t_id, int t_depth);

  Flit* read();
  Flit* readLast();
  void enqueue(Flit* flit);//VCRouter.cpp88: out_port_list[i]->out_link->rInPort->buffer_list[flit->vc]->enqueue(flit);
  Flit* dequeue();
  void empty();
  bool isFull();
  void get_credit();

  ~FlitBuffer();

  int id;
  int vc;
  int vnet;
  int depth;
  int cur_flit_num;
  int used_credit;
  deque <Flit*> flit_queue;  //双向队列，元素为指向flit对象的指针
};




#endif /* VC_FLITBUFFER_HPP_ */
