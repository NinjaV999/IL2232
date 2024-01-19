/*
 * Router.hpp
 *
 *  Created on: 2019年8月19日
 *      Author: wr
 */

#ifndef VCROUTER_HPP_
#define VCROUTER_HPP_

#include "VC/FlitBuffer.hpp"
#include "VC/Link.hpp"
#include "VC/NRBase.hpp"
#include "VC/RInPort.hpp"
#include "VC/ROutPort.hpp"
#include "VC/VCNetwork.hpp"
//#include "parameters.hpp"
#include <fstream>
#include <vector>

class RInPort;
class ROutPort;
class VCNetwork;

class VCRouter: public NRBase//vc router 是 NR base的子类
{
public:
  /*
    * @brief VC/VCRouter.hpp
    * t_id[0] tid[y]: x-y dimension
    */
  VCRouter (int* t_id, int in_out_port_num, VCNetwork* t_vcNetwork, int t_vn_num, int t_vc_per_vn, int t_vc_priority_per_vn, int t_in_depth);
//包括 id 输入输出端口数 ，一个指向VCnetwork的 指针 vn的数量vc/vn的数量，priority 应该没用的
  // Main methods
  int getRoute(Flit* t_flit);//进行routing

  void vcRequest();
  void vcRequest_routerSW_priority_responsePacket();// add router vc_request_routerSW_priority_globalPakcetID
//没用

  void getSwitch();
  void getSwitch_routerSW_priority_responsePacket();
//没用

  void outPortDequeue();

  /** @brief yzrunOneStep of VC Router: use other function defined in this class  // wbq: To run Routing, VC_allocation, Switching
   *
   *    vcRequest();
   *    getSwitch();
   *    outPortDequeue();
   */
  void runOneStep();
    //实际上完成了这样一个过程，flit根据下一条的位置找到了outport，
    // 继续找与其link的下一个router的输出端口 同时根觉flit的vnet找到第一个 空的Vc,将flit存入outport的filtbuffer里，再将其存入
    //inport的buffer里，若是headflit在根据flit的dest进行routing,寻找其进入到第二个router里在进行下一条的方向储存起来，并改变inport vc state

  // Main components
  std::vector<RInPort*> in_port_list_inRouter;
  std::vector<ROutPort*> out_port_list_inRouter;

//add
#ifdef    ofstreamSW_VNRouterActiveVC
    ofstream  VNRouterActiveVC;
#endif
    void  writeFile_VNRouterActiveVC();//没用
    int port_utilization_innet;
//addend

  // Network
  VCNetwork* vcNetwork;
  int id[2];

  int rr_port;//round robin port
  int router_port_num;

  int port_total_utilization;

  /** @brief   ~VCRouter yz: delete inport list and outport list
    *
    */
  ~VCRouter ();
};

#endif /* VCROUTER_HPP_ */
