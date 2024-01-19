/*
 * PacketBuffer.hpp
 *
 *  Created on: 2019年8月23日
 *      Author: wr
 */

#ifndef VC_PACKETBUFFER_HPP_
#define VC_PACKETBUFFER_HPP_

#include "VC/NI.hpp"
#include "VC/Packet.hpp"
#include <deque>


class NI;


class PacketBuffer
{
public:
  PacketBuffer (NI* ni_owner, int id);

  std::deque<Packet*> packet_queue;//存储指向packet 指针的双端队列，构成 packet buffer

  void enqueue(Packet*);
  Packet* dequeue();
  Packet * read();
  ~PacketBuffer();

  int packet_num;
  int vn_num;

  NI* ni_owner;

};

#endif /* VC_PACKETBUFFER_HPP_ */
