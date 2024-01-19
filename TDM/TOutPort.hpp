/*
 * TOutPort.hpp
 *
 *  Created on: 2019年9月4日
 *      Author: wr
 */

#ifndef TDM_TOUTPORT_HPP_
#define TDM_TOUTPORT_HPP_

#include "TDM/TLink.hpp"

class TLink;

class TOutPort
{
public:
  TOutPort ();

  TLink* outLink;

  virtual
  ~TOutPort ();
};

#endif /* TDM_TOUTPORT_HPP_ */
