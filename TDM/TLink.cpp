/*
 * TLink.cpp
 *
 *  Created on: 2019年9月4日
 *      Author: wr
 */

#include <TDM/TLink.hpp>
#include <cstddef>

TLink::TLink(TInPort* t_inPort)
{
  inPort = t_inPort;
  outPort = (TOutPort*)NULL;
}

TLink::~TLink ()
{
  // TODO Auto-generated destructor stub
}

