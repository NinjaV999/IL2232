/*
 * TNetwork.cpp
 *
 *  Created on: 2019年9月5日
 *      Author: wr
 */

#include "TDM/TNetwork.hpp"


TNetwork::TNetwork (int t_router_num, int t_router_num_x, int t_ni_num_total, int* t_ni_num){
     router_num = t_router_num;
     router_num_x = t_router_num_x;
     NI_num_total = t_ni_num_total;
     NI_num = t_ni_num;

     int NI_seq = 0;
     for(int i=0; i<router_num; i++){
          int id[2];
          id[0] = i/router_num_x;
          id[1] = i%router_num_x;
          int in_out_port_num = 4 + NI_num[i];
          TRouter* router = new TRouter(id, in_out_port_num);
          router_list.push_back(router);

          // firstly connect NI to corresponding router
          for(int j=0; j<NI_num[i]; j++){
              TNI* ni = new TNI(NI_seq, NI_num_total);
              ni_list.push_back(ni);
              NI_seq++;
              // connect NI's outport with router's inport
     	      ni->outPort->outLink = router->inPort_list[4+j]->link;
     	      router->inPort_list[4+j]->link->outPort = ni->outPort;

     	      // connect NI's inport with router's outport
     	      router->outPort_list[4+j]->outLink = ni->inPort->link;
     	      ni->inPort->link->outPort = router->outPort_list[4+j];
           }
       }

       // secondly, connect routers with each other according to the network size.
       for(int i=0; i<router_num; i++){
           int x,y; // (x,y)
           x = i/router_num_x;
           y = i%router_num_x;

           // port: 0->up; 1->right; 2->down; 3->left. each router connects to its right and down routers
           // port 1, right;
           if(y+1 < router_num_x){ // The right router exits;
     	       TRouter* self = router_list[i];
     	       TRouter* next = router_list[i+1];
     	       // connect self's outport with next's inport

     	       self->outPort_list[1]->outLink = next->inPort_list[3]->link;
     	       next->inPort_list[3]->link->outPort = self->outPort_list[1];

     	       // connect next's outport with self's inport
     	       next->outPort_list[3]->outLink = self->inPort_list[1]->link;
     	       self->inPort_list[1]->link->outPort = next->outPort_list[3];
           }

           // port 2, down;
           if(x+1 < router_num/router_num_x){ // The down router exits;
               TRouter* self = router_list[i];
     	       TRouter* next = router_list[i+router_num_x];
     	       // connect self's outport with next's inport
     	       self->outPort_list[2]->outLink = next->inPort_list[0]->link;
     	       next->inPort_list[0]->link->outPort = self->outPort_list[2];

     	       // connect next's outport with self's inport
     	       next->outPort_list[0]->outLink = self->inPort_list[2]->link;
     	       self->inPort_list[2]->link->outPort = next->outPort_list[0];
           }
       }
}

void TNetwork::runOneStep(){

  for(int i=0; i<NI_num_total; i++){
      //std::cout << "#######" << i << std::endl;
      ni_list[i]->runOneStep();
  }

  for(int i=0; i<router_num; i++){

      router_list[i]->runOneStep();

  }

}

void TNetwork::queuing(){
  float a = 0.0;
  int b = 0;
  for(int i=0; i<NI_num_total; i++){
      a += ((float)ni_list[i]->queuing_delay_sum);
      b += ni_list[i]->num_sum;
  }
  std::cout.setf(std::ios::fixed);
  std::cout << std::setprecision(3) << a/b << std::endl;
}

int TNetwork::port_num_f(int router){
  if(router==0 || router==13 || router==154 || router==167)
      return 3;
  else if(router<=13 || router>=154 || router%14==0 || router%14==13)
    return 4;
  return 5;
}

void TNetwork::port_utilization(int simulate_cycles){
  for(int i=0; i<router_num; i++){
      std::cout.setf(std::ios::fixed);
      //std::cout << i << " ";
      std::cout << std::setprecision(3) << ((float)router_list[i]->port_total_utilization)/(port_num_f(i)*simulate_cycles/100)<< std::endl;
  }
}

void TNetwork::slot_utilization(int cycles){
  for(int i=0; i<NI_num_total; i++){
      std::cout.setf(std::ios::fixed);
      if(ni_list[i]->slot_utilization==0)
	std::cout << "1" << std::endl;
      else
	std::cout << std::setprecision(3) << (float)ni_list[i]->slot_utilization/(cycles/SLOT_SIZE*ni_list[i]->available_slot())<< std::endl;
  }
}

void TNetwork::port_standard_utilization(){
  for(int i=0; i<router_num; i++){
      std::cout.setf(std::ios::fixed);
      std::cout << std::setprecision(3) << (float)router_list[i]->port_standard_utilization()/(port_num_f(i)*SLOT_SIZE) << std::endl;
  }
}

TNetwork::~TNetwork ()
{
  // TODO Auto-generated destructor stub
}

