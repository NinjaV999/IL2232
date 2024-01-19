/*
 * Port.cpp
 *
 *  Created on: 2019年8月19日
 *      Author: wr
 */

#include "VC/Port.hpp"
#include <iostream>
#include "parameters.hpp"


Port::Port (int t_id, int t_vn_num, int t_vc_per_vn, int t_vc_priority_per_vn, int t_depth)
{
   id = t_id;
   vn_num = t_vn_num;
   vc_per_vn = t_vc_per_vn;
   vc_priority_per_vn = t_vc_priority_per_vn;

   depth = t_depth;

   buffer_list.reserve(vn_num*(vc_per_vn + vc_priority_per_vn));//2 * (3) for each port
    //预先为buffer list 分配内存空间，可以看作是每个端口 flit buffer 的数量




   // URS 0~vn_num*vc_per_vn-1;
   //两个vn 每个 vn都有 2个 普通 buffer 和 一个 的优先buffer
   for(int i=0; i<vn_num*vc_per_vn; i++){//  for example, vn_num=2 VC_PER_VN=2 vc_priority_per_vn=1 theni=0~3. So:  i%VC_PER_VN=0,1,0,1  i/vn_num=0,0,1,1,i=0,1,2,3,depth =4 //depth =4 from main: int flit_per_vc = INPORT_FLIT_BUFFER_SIZE; INPORT_FLIT_BUFFER_SIZE=4
       FlitBuffer * t_flitBuffer = new FlitBuffer(i%vc_per_vn, i/vn_num, i, depth);// each vn of 2 vns have 2 buffers for
       buffer_list.push_back(t_flitBuffer);
   }
/*
 * id = t_id; // 所有flit buffer的第几个
  vnet = t_vnet;//在那个 vc newtwork
  vc = t_vc; // 该 vc network 对应的哪个 VC （每个 VC 有一个 flit buffer）
 */
   // LCS vn_num*vc_per_vn~vn_num*(vc_per_vn + vc_priority_per_vn)-1
   //感觉没什么用 因为 我现在 全是 urs
   for(int i=0; i<vn_num*vc_priority_per_vn; i++){// for example, vn_num=2 VC_PER_VN=2 vc_priority_per_vn=1  i=0,1
       FlitBuffer * t_flitBuffer = new FlitBuffer(i%vc_priority_per_vn+vc_per_vn, i/vn_num, i, depth);// each vn of 2 vns have 1 buffers for prioritys
       buffer_list.push_back(t_flitBuffer);
   }
  // cout<<"buffer_list.size()"<<buffer_list.size()<<endl; //for example 168*2*6 buffer
}
//根据某种规则创建 某个 端口的flit buffer

Port::~Port(){
  FlitBuffer* flitBuffer;
  while(buffer_list.size()!=0){
      flitBuffer = buffer_list.back();
      buffer_list.pop_back();
      delete flitBuffer;
  }
}



