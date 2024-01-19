/*
 * TPort.hpp
 *
 *  Created on: 2019年9月4日
 *      Author: wr
 */

#ifndef TDM_TINPORT_HPP_
#define TDM_TINPORT_HPP_

#include "TDM/Block.hpp"
#include "TDM/TLink.hpp"
#include <deque>

class Block;
class TLink;

class TInPort
{
public:
  TInPort ();

  std::deque<Block*> block_list;
  int count;
  TLink* link;

  void enqueue(Block*);
  Block* dequeue();
  Block* read();

  virtual
  ~TInPort ();
};

#endif /* TDM_TINPORT_HPP_ */
