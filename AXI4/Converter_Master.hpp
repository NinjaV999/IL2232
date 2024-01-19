/*
 * Converter.hpp
 *
 *  Created on: 2019年9月9日
 *      Author: wr
 */

#ifndef AXI4_CONVERTER_MASTER_HPP_
#define AXI4_CONVERTER_MASTER_HPP_


class AXI4Buffer;
#include <printfSW.hpp>//sw to printf
#include "AXI4/AXI4Signal.hpp"
#include "AXI4/Message.hpp"

#include <deque>
#include <iostream>

extern float cycles;
extern int overall_signal_num;
class Converter_Master
{
public:
    Converter_Master(int id, float inj);

  std::deque<AXI4Signal*> AXI4ReqFromGenInMasterConverter_in_read, AXI4ToSendReq_in_write, AXI4OutRead_ToDelInConvMaster, AXI4OutWrite_ToDelInConvMaster;
  //四个队列 份额别存储的 是 从生成器接收到的读请求，从生成器接收到的 写请求， 将要删除的读响应 以及 将要删除的 写响应。
  std::deque<Message*> messageReceivedResp_in_toBeDeleted, message_out;
//以msg 存储的resp将要删除的信号，将要发出的msg
  int NI_id;
  float injection_rate;

  // from message in (request) to message out (response); only for the converter connected to slave port
  void delReceivedRespSig();
    void  sendRespMessageToDel();
    void dealWithAXI4ToSendReq_in();
  void runOneStep();

  int countDelRecvRespSig_read, countDelRecvRespSig_write;
  static int count, count_b;

  virtual
  ~Converter_Master ();
};

#endif /* AXI4_CONVERTER_HPP_ */
