/*
 * ROutPort.cpp
 *
 *  Created on: 2019年8月22日
 *      Author: wr
 */

#include "VC/ROutPort.hpp"

ROutPort::ROutPort (int t_id, int t_vn_num, int t_vc_per_vn, int t_vc_priority_per_vn, int t_depth)//nicpp 83: ROutPort * vc_rOutPort = new ROutPort(0, 1, 1, 0, INFINITE);
   :Port(t_id, t_vn_num, t_vc_per_vn, t_vc_priority_per_vn, t_depth) //其和 port 端口 类比 没什么特殊
{
  out_link = NULL; //初始指向空指针的左右在于对于后续没有 进行连接的端口 ，其连接的端口直接置空 ，noc边界上的router
}

ROutPort::~ROutPort(){
}

