/*
 * BlockBuffer.cpp
 *
 *  Created on: 2019年9月4日
 *      Author: wr
 */

#include <TDM/BlockBuffer.hpp>
#include <cassert>

BlockBuffer::BlockBuffer ()
{
  count = 0;
}

void BlockBuffer::enqueue(Block* block){
  blockBuffer.push_back(block);
  count++;
}

Block* BlockBuffer::dequeue(){
  assert(count>0);
  Block* block = blockBuffer.front();
  blockBuffer.pop_front();
  count--;
  return block;
}

Block* BlockBuffer::read(){
  assert(count>0);
  return blockBuffer.front();
}



BlockBuffer::~BlockBuffer ()
{
  // TODO Auto-generated destructor stub
}

