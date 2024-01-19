/*
 * Link.cpp
 *
 *  Created on: 2019年8月19日
 *      Author: wr
 */


#include "VC/Link.hpp"

Link::Link(RInPort* t_rInPort){//link 函数的作用就是 连接一个rinport 对象
  rInPort = t_rInPort;
  rOutPort = NULL;
}
