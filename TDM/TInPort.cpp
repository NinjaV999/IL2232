/*
 * TPort.cpp
 *
 *  Created on: 2019年9月4日
 *      Author: wr
 */

#include <TDM/TInPort.hpp>
#include <cassert>

TInPort::TInPort ()
{
  count = 0;
  TLink* t_link = new TLink(this);
  link = t_link;
}

void TInPort::enqueue(Block* t_block){
  //assert(count<2);
  block_list.push_back(t_block);
  count++;
}

Block* TInPort::dequeue(){
  assert(count>0);
  Block* block = block_list.front();
  block_list.pop_front();
  count--;
  return block;
}

Block* TInPort::read(){
  assert(count>0);
  return block_list.front();
}



TInPort::~TInPort ()
{
  delete link;
}

