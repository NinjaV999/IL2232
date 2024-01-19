/*
 * TLink.hpp
 *
 *  Created on: 2019年9月4日
 *      Author: wr
 */

#ifndef TDM_TLINK_HPP_
#define TDM_TLINK_HPP_

#include "TDM/TInPort.hpp"
#include "TDM/TOutPort.hpp"

class TInPort;
class TOutPort;

class TLink
{
public:
  TLink (TInPort* t_inPort);

  TInPort* inPort;
  TOutPort* outPort;

  virtual
  ~TLink ();
};

#endif /* TDM_TLINK_HPP_ */
