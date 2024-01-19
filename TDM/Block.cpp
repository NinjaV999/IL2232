/*
 * block.cpp
 *
 *  Created on: 2019年9月4日
 *      Author: wr
 */

#include "TDM/Block.hpp"

Block::Block (int t_id, int t_num, float t_cycle, Signal* sig)
{
  id = t_id;
  total_num = t_num;
  enqueue_cycle = t_cycle;
  signal = sig;
}

Block::~Block ()
{
  // TODO Auto-generated destructor stub
}

