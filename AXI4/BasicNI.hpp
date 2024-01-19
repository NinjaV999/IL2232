/*
 * BasicNI.hpp
 *
 *  Created on: Sep 16, 2019
 *      Author: wr
 */

#ifndef BASICNI_HPP_
#define BASICNI_HPP_

#include "AXI4/Message.hpp"
#include "TDM/TNetwork.hpp"
#include "VC/VCNetwork.hpp"

class BasicNI
{//创建一个bsicNI的类
public:
  BasicNI (int t_id, int slave_num, int master_num, TNetwork* t_network, VCNetwork* vc_network, int router_num_x, int* NI_num);

//用来初始化该对象的属性
  int id;
  int slave_num;
  int master_num;

  TNetwork* TDM_network; //指向TNetwork 对象的指针
  VCNetwork* VC_network;//指向VCNertwork 对象的指针
  int router_num_x;
  int* NI_num;

  //add
  std::vector<BasicNI*>  basicNI_list_inBasicNI;//创建一个不定长的数组 其中元素都是指向basicNI类型对象的指针
  int basicNI_packetWaitNum;
   int actionPicked_inBasicNI;
   void updateRLDQN_actionPicked(int t_actionPicked_inBasicNI);// update actionPicked_inBasicNI to be a value from main.
  //没有用到
  //一组虚拟函数要求派生类（子类）提供它们的具体实现
  virtual void enqueue(Message*)=0;
  virtual Message* dequeueMessage()=0;
  virtual void runOneStep()=0;

  virtual
  ~BasicNI ();
};

#endif /* BASICNI_HPP_ */
