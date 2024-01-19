/*
 * BlockList.hpp
 *
 *  Created on: 2019年9月4日
 *      Author: wr
 */

#ifndef TDM_BLOCKBUFFER_HPP_
#define TDM_BLOCKBUFFER_HPP_

#include <deque>
#include "TDM/Block.hpp"

class Block;

class BlockBuffer
{
public:
  BlockBuffer ();

  std::deque<Block*> blockBuffer;
  int count;

  void enqueue(Block*);
  Block* read();
  Block* dequeue();

  virtual
  ~BlockBuffer ();
};

#endif /* TDM_BLOCKBUFFER_HPP_ */
