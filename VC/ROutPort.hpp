/*
 * ROutPort.hpp
 *
 *  Created on: 2019年8月22日
 *      Author: wr
 */

#ifndef VC_ROUTPORT_HPP_
#define VC_ROUTPORT_HPP_

#include "VC/Link.hpp"
#include "VC/Port.hpp"

class Link;


/*
 * @brief ROutPort contains only link *outlink and self-ini.
 */
class ROutPort : public Port
{
public:
  ROutPort (int t_id, int t_vn_num, int t_vc_per_vn, int t_vc_priority_per_vn, int t_depth);
  ~ROutPort();
  Link * out_link; //就一个 构造函数 以及 一个指向 out——link的指针
};

#endif /* VC_ROUTPORT_HPP_ */
