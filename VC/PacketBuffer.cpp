/*
 * PacketBuffer.cpp
 *
 *  Created on: 2019年8月23日
 *      Author: wr
 */

#include "VC/PacketBuffer.hpp"
#include <cassert>

PacketBuffer::PacketBuffer (NI* t_NI, int t_vn_num)
{
  packet_num = 0; //packet的 数
  vn_num = t_vn_num;
  ni_owner = t_NI; //没用过
}

void PacketBuffer::enqueue(Packet* t_packet){
  packet_queue.push_back(t_packet); //packet queue 似乎是无限长度的 ，往 packet buffer中存入一个packet
  packet_num++;
}

Packet* PacketBuffer::dequeue(){
  assert(packet_num!=0);//当 packet num 不为 0的 时候 取出第一个 packet
  Packet* packet = packet_queue.front();
  packet_queue.pop_front();
  packet_num--;
  return packet;//并返回
}

Packet * PacketBuffer::read(){
  assert(packet_num!=0);
  return packet_queue.front();//读 第一个 packet 但不删除
}

PacketBuffer::~PacketBuffer(){
  Packet* packet;
  while(packet_queue.size()!=0){
      packet = packet_queue.front();
      packet_queue.pop_front();
      delete packet; //析构函数 用于清空队列
  }
}

