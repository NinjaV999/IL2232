/*
 * TRouter.cpp
 *
 *  Created on: 2019年9月4日
 *      Author: wr
 */

#include <TDM/TRouter.hpp>
#include "parameters.hpp"
#include <iostream>


TRouter::TRouter (int* t_id, int t_port_num)
{
  id[0] = t_id[0];
  id[1] = t_id[1];
  port_num = t_port_num;

  for(int i=0; i< port_num; i++){
      TInPort* inPort = new TInPort();
      inPort_list.push_back(inPort);
      TOutPort* outPort = new TOutPort();
      outPort_list.push_back(outPort);
  }
  port_total_utilization = 0;
}

void TRouter::runOneStep(){
  int t_id = id[0]*X_NUM+id[1];
  int cur_slot = (int)cycles%(SLOT_NUM*SLOT_LENGTH);
  int inport = tdm_routing_table[t_id][cur_slot][0];
  if(inport != -1 && inPort_list[inport]->count > 0 && inPort_list[inport]->read()->enqueue_cycle < cycles){
      int outport = tdm_routing_table[t_id][cur_slot][1];
      assert(outport != -1);
      Block* block = inPort_list[inport]->dequeue();
      block->enqueue_cycle = cycles + LINK_TIME - 1;
      outPort_list[outport]->outLink->inPort->enqueue(block);
      port_total_utilization++;
  }
}

int TRouter::port_standard_utilization(){
  int count = 0;
  int t_id = id[0]*X_NUM+id[1];
  for(int i=0; i< SLOT_SIZE; i++){
      if(tdm_routing_table[t_id][i][0]!=-1){
	  count++;
      }
  }
  return count;
}

TRouter::~TRouter ()
{
  TInPort* inPort;
  while(inPort_list.size()!=0){
      inPort = inPort_list.back();
      inPort_list.pop_back();
      delete inPort;
  }

  TOutPort* outPort;
  while(outPort_list.size()!=0){
      outPort = outPort_list.back();
      outPort_list.pop_back();
      delete outPort;
  }
}
