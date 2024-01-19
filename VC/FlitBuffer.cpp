/*
 * flit_buffer.cpp
 *
 *  Created on: 2019年8月15日
 *      Author: wr
 */

#include <cassert>
#include "VC/FlitBuffer.hpp"
#include "parameters.hpp"
#include <iostream>


FlitBuffer::FlitBuffer(int t_vc, int t_vnet, int t_id, int t_depth){
    //构造函数
  id = t_id; // 所有flit buffer的第几个
  vnet = t_vnet;//在那个 vc newtwork
  vc = t_vc; // 该 vc network 对应的哪个 VC （每个 VC 有一个 flit buffer）
  cur_flit_num = 0;
  used_credit = 0;
  depth = t_depth;
}

Flit* FlitBuffer::read(){ //读flit ，从 双向队列中读取第一个 flit，如果 当前flit num 不为 0的 情况下
  assert(cur_flit_num != 0); //=0的时候程序终止
  return flit_queue.front();
}


Flit* FlitBuffer::dequeue(){
  assert (cur_flit_num != 0);
  Flit* t_flit = flit_queue.front();//用一个 t flit 指向队列第一个
  flit_queue.pop_front();// 队列第一个取出
  cur_flit_num--;//当前flit 数量减少
  used_credit--; //used credit ,记录被使用的credit，当一个flit 出队列时 则 被使用的credit 就减少
  return t_flit;
}

void FlitBuffer::get_credit(){
  used_credit++; //存入 flit 会导致 被使用的 credit 增加
}

void FlitBuffer::enqueue(Flit* t_flit){
  assert (cur_flit_num != depth);//当flit queue 没有被填满时
  flit_queue.push_back(t_flit);//在队列末端 添加 当前flit
  cur_flit_num++;
}

Flit* FlitBuffer::readLast(){
  assert(cur_flit_num != 0);
  return flit_queue.back(); //读队列作后一个 flit
}

void FlitBuffer::empty(){//没用
  flit_queue.empty();
  cur_flit_num = 0;
  used_credit = 0;
}

bool FlitBuffer::isFull(){// buffer 的 长度 小于 被使用的 credit， 被使用的credit 已经超过了 credit的最大数量 ，则认为 flitbuffer 是满的
  return ( (depth<=used_credit) ? 1:0);
}

FlitBuffer::~FlitBuffer(){
  Flit* flit;
  while(flit_queue.size()!=0){
      flit = flit_queue.front();
      flit_queue.pop_front();
      delete flit; //析构函数 用于清空缓冲区
  }
}



