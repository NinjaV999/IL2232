/*
 * Link.hpp
 *
 *  Created on: 2019年8月19日
 *      Author: wr
 */

#ifndef VC_LINK_HPP_
#define VC_LINK_HPP_

#include "VC/Flit.hpp"//yz ask necessary？
#include "VC/RInPort.hpp"
#include "VC/ROutPort.hpp"//yz ask necessary？ just null

class RInPort;
class ROutPort;

/*
 * @brief link: contains 2 ports: RInPort and ROutPort
 */
class Link{
public:
  Link(RInPort* t_rInPort);
  RInPort* rInPort;
  ROutPort* rOutPort;
};



#endif /* VC_LINK_HPP_ */
