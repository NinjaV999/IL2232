/*
 * Port.hpp
 *
 *  Created on: 2019年8月19日
 *      Author: wr
 */

#ifndef VC_PORT_HPP_
#define VC_PORT_HPP_

#include "VC/Flit.hpp"
#include "VC/FlitBuffer.hpp"
#include <vector>


class Port
{
public:
  Port(int t_id, int t_vn_num, int t_vc_per_vn, int t_vc_priority_per_vn, int t_depth);
  virtual ~Port();

  int id;
  int vn_num; //vc network 的 数量
  int vc_per_vn;//每个 vn vc的数量
  int depth;
  int vc_priority_per_vn; //每个 vn中各个 vc的 数量级

  std::vector<FlitBuffer*> buffer_list;// 一个 vector ,用于存储flit buffer
};

#endif /* VC_PORT_HPP_ */
