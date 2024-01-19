/*
 * Message.hpp
 *
 *  Created on: 2019年9月10日
 *      Author: wr
 */

#ifndef MESSAGE_HPP_
#define MESSAGE_HPP_

#include "AXI4/AXI4Signal.hpp"

class Message
{
public:

  /*
   * @brief set self parameters to be input values  yz
  NI_id = source_NI;
    signal = t_signal;
    out_cycle = t_cycles;//yz what is tycles and outcycle
    slave_id = -1;
    sequence_id = -1;
    */

  Message(int source_NI, AXI4Signal* signal, float cycles);
  /*
   * @brief cp to be self
    */
  Message(Message* message);

  int NI_id;
  // + 8-bit NI_id: read request (101 bits); read response (14+y bits); write request (101+y bits); write response (14 bits);
  AXI4Signal* signal;
  float out_cycle_inMessage;

  // for re-order buffer
  int slave_id;  // -1; 8 bits; covered by the dest (request) and NI_id (response)
  int sequence_id; // -1; 4 bits
  // + 4-bit sequence_id: read request (105 bits); read response (18+y bits); write request (105+y bits); write response (18 bits);

  virtual
  /*
   * @brief ~Message do nothing but delete yz
    */
  ~Message ();
};

#endif /* MESSAGE_HPP_ */
