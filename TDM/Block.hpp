/*
 * block.hpp
 *
 *  Created on: 2019年9月4日
 *      Author: wr
 */

#ifndef TDM_BLOCK_HPP_
#define TDM_BLOCK_HPP_

#include "TDM/Signal.hpp"

class Block
{
public:
  Block (int t_id, int t_num, float t_cycle, Signal* sig);

  int id;
  int total_num;
  float enqueue_cycle;

  Signal* signal;

  virtual
  ~Block ();
};

#endif /* TDM_BLOCK_HPP_ */
