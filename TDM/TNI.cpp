/*
 * TNI.cpp
 *
 *  Created on: 2019年9月4日
 *      Author: wr
 */

#include <TDM/TNI.hpp>
#include <iostream>

int TNI::count_s = 0;
int TNI::count_r = 0;
int TNI::count_s_res = 0;
int TNI::count_r_res = 0;



int TNI::converse_latency_single_dis[100] = {0};
int TNI::GRS_latency_single_dis[100] = {0};

int TNI::converse_latency_single_count = 0;
int TNI::converse_latency_single_total = 0;
int TNI::converse_latency_single_worst = 0;
int TNI::GRS_latency_single_count = 0;
int TNI::GRS_latency_single_total = 0;
int TNI::GRS_latency_single_worst = 0;

TNI::TNI (int t_id, int t_ni_num)
{
  id = t_id;
  ni_num = t_ni_num;

  for(int i=0; i<ni_num; i++){
      BlockBuffer* blockBuffer = new BlockBuffer();
      buffer_list.push_back(blockBuffer);
  }

  outPort = new TOutPort();
  inPort = new TInPort();

  signal_buffer_out.resize(2);
  buffer_list.resize(ni_num);
  slot_utilization = 0;

  queuing_delay_sum = 0.0;
  num_sum = 0;
}

void TNI::runOneStep(){

  // signal to blocks
  while(signal_buffer.size()!=0 && signal_buffer.front()->out_cycle_inMessage < cycles){
      Signal* t_signal = signal_buffer.front();
      signal_buffer.pop_front();
      int block_num = (t_signal->length-1)/(BLOCK_SIZE*8)+1;//bandwidth 128
      int dest = t_signal->destination;
      if(t_signal->signal->type%2==0){
	  count_s++;
	  std::cout << "TNI send request" << count_s << std::endl;
      }
      if(t_signal->signal->type%2==1){
	  count_s_res++;
	  std::cout << "TNI send response" << count_s_res << std::endl;
      }

      for(int id=0; id<block_num; id++){
	  Block* t_block = new Block(id, block_num, cycles, t_signal);
	  buffer_list[dest]->enqueue(t_block);
      }
  }

  // output
  int slot = (int)cycles%(SLOT_NUM*SLOT_LENGTH);
  int dest = tdm_NI_table[id][slot];
  if(dest != -1 && buffer_list[dest]->count > 0 && buffer_list[dest]->read()->enqueue_cycle < cycles ){
      Block* block = buffer_list[dest]->dequeue();

      if(block->id==0){ //head flit
          queuing_delay_sum += cycles-block->signal->signal->NI_arrival_time;
          num_sum++;
      }

      block->enqueue_cycle = cycles + LINK_TIME - 1;
      outPort->outLink->inPort->enqueue(block);
      slot_utilization++;
  }
  //input
  if(inPort->count > 0 && inPort->read()->enqueue_cycle < cycles){
      Block* block = inPort->dequeue();
      if(block->id == block->total_num-1){ //tail
	  block->signal->out_cycle_inMessage = cycles;
	  if(block->signal->signal->type%2==0){
		  count_r++;
		  std::cout << "TNI receive request" << count_r << std::endl;
	  }
	  if(block->signal->signal->type%2==1){
		  count_r_res++;
		  std::cout << "TNI receive response" << count_r_res << std::endl;
	  }
	  if(block->signal->signal->type == 0 || block->signal->signal->type == 2) // request
	       signal_buffer_out[0].push_back(block->signal);
	  else // response
	       signal_buffer_out[1].push_back(block->signal);
	  if(block->signal->signal->source_id == 2){
	      int delay = cycles - block->signal->signal->NI_arrival_time;
	      if(block->signal->signal->test_tag_qos_convert == 1){
		    if(delay < 99)
		       converse_latency_single_dis[delay]++;
		    else
		      converse_latency_single_dis[99]++;
		    converse_latency_single_count++;
		    converse_latency_single_total += delay;
		    converse_latency_single_worst = (converse_latency_single_worst<delay)?delay:converse_latency_single_worst;
	      }else{
		  if(delay < 99)
		    GRS_latency_single_dis[delay]++;
		  else
		    GRS_latency_single_dis[99]++;
		  GRS_latency_single_count++;
		  GRS_latency_single_total += delay;
		  GRS_latency_single_worst = (GRS_latency_single_worst<delay)?delay:GRS_latency_single_worst;
	      }
	  }
      }
      delete block;
  }
}

int TNI::available_slot(){
  int count = 0;
  for(int i=0; i<SLOT_SIZE;i++)
    if(tdm_NI_table[id][i]!=-1)
      count++;
  return count;
}

TNI::~TNI ()
{
  BlockBuffer* tmp;
  while(buffer_list.size()!=0){
      tmp = buffer_list.back();
      buffer_list.pop_back();
      delete tmp;
  }
}

