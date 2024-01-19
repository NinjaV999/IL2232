/*
 * Message.cpp
 *
 *  Created on: 2019年9月10日
 *      Author: wr
 */

#include "AXI4/Message.hpp"


Message::Message(int source_NI, AXI4Signal* t_signal, float t_cycles){
  NI_id = source_NI;
  signal = t_signal;
  out_cycle_inMessage = t_cycles;
  slave_id = -1;
  sequence_id = -1;
}

Message::Message(Message* message){//yz copy message to be self?
  NI_id = message->NI_id;
  signal = message->signal;
  out_cycle_inMessage = message->out_cycle_inMessage;

  slave_id = message->slave_id;
  sequence_id = message->sequence_id;
}

Message::~Message ()
{
  // TODO Auto-generated destructor stub
}

