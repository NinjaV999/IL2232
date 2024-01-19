/*
 * Signal.hpp
 *
 *  Created on: 2019年9月9日
 *      Author: wr
 */

#ifndef TDM_SIGNAL_HPP_
#define TDM_SIGNAL_HPP_

#include "AXI4/Message.hpp"

class Signal: public Message
{
public:
  Signal (Message*message);

  int destination;
  int length;  // byte

  virtual
  ~Signal ();
};

#endif /* TDM_SIGNAL_HPP_ */

