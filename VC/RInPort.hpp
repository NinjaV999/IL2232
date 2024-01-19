/*
 * Port.hpp
 *
 *  Created on: 2019年8月15日
 *      Author: wr
 */

#ifndef VC_RINPORT_HPP_
#define VC_RINPORT_HPP_

#include "VC/Flit.hpp"
#include "VC/FlitBuffer.hpp"
#include "VC/Link.hpp"
#include "VC/NRBase.hpp"
#include "VC/Port.hpp"
#include "VC/VCRouter.hpp"
#include "parameters.hpp"

#include <cassert>
#include <vector>


class FlitBuffer;
class VCRouter;
class Link;




/*
  * @brief very importatnt. routing done here.
   */


class RInPort : public Port{ //是port的子类
public:

  /*
    * @brief push 0 to state,-1 to outport and out——vc： times ar dependent on i i=0; i<vn_num*(vc_per_vn+vc_priority_per_vn); i++
     */
  RInPort(int t_id, int t_vn_num, int t_vc_per_vn, int t_vc_priority_per_vn, int t_depth, NRBase * t_owner, Link * t_in_link, int t_global_rInPort_ID);
//构造函数
  void vc_request();
  void vc_request_inPort_routerSW_priority_responsePacket();//add



  int vc_allocate(Flit*);
  int vc_allocate_normal(Flit*);

  int vc_allocate_priority(int);

  void getSwitch();
  void getSwitch_inPort_routerSW_priority_responsePacket();//add

  ~RInPort();

  //GROPC field
  std::vector<int> state; //0->I; 1->R; 2-> V; 3->A; idel ,routing , wait for an out put vc ,active
  /*
   * 0 表示空闲状态，即 VC 是空闲的，没有正在等待的 Flit。
1 表示正在等待 routing，即 VC 已经被预定，等待路由的计算。
2 表示正在等待输出 VC，即已经计算好路由，正在寻找下游节点的输出 VC。
3 表示活跃状态，即正在传输中。
   */
  std::vector<int> out_port;
  std::vector<int> out_vc;

  //Switch arbitration
  int rr_record; //round robin record


  // for shared priority
  std::vector<int> priority_vc;
  int count_vc;
  std::vector<int> priority_switch;
  int count_switch;

  //ADD
  int rInPort_ID;
  //ADD END

  // for individual priority
  int rr_priority_record;
  int starvation;

  NRBase * router_owner;
  Link * in_link;
};



#endif /* VC_RINPORT_HPP_ */
