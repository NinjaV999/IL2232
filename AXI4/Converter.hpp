/*
 * Converter.hpp
 *
 *  Created on: 2019年9月9日
 *      Author: wr
 */

#ifndef AXI4_CONVERTER_HPP_
#define AXI4_CONVERTER_HPP_


class AXI4Buffer;
#include <printfSW.hpp>//sw to printf
#include "AXI4/AXI4Signal.hpp"
#include "AXI4/Message.hpp"

#include <deque>
#include <iostream>

extern float cycles;
extern int overall_signal_num;
extern int global_respSignalNum;
class Converter
{
public:
  Converter(int id, float inj);//从 slave 角度 看 ，接受 请求信号 并 生成resp sig

  std::deque<AXI4Signal*> RespSigInConverterSlave_initializedRead, RespSigInConverterSlave_initializedWrite, AXI4_out_read, AXI4_out_write;
  //创建一个双端队列 ，队列中每个元素是一个指向AXI4Signal类对象的指针
  //read resp signal ， write response signal, read request , write request.
  std::deque<Message*> message_in_inConverterSlave, message_out_inConverterSlave;
  //同理是一个message 类的指针，从网络中接受req msg的队列，以及向外发出msg的队列
//对一个发送 先req sig ，先是phit-》filt->packet->msg 以msg形式在网络中传送，然后逐渐 被slave 接受先构成msg ，在网络层面（NoC）
//上是 msg ，而在AXI4 通信协议上 是sig，应该是产生请求信号 ，并获得响应信号，sig和msg的转换是有NI完成的以适应不同的层面的要求
//deque 双端队列
  int NI_id;
  float injection_rate; //注入率

  // from message in (request) to message out (response); only for the converter connected to slave port
  //从message 请求到message 响应只考虑与slave 连接的converter
  void MasterNIresponseToOtherMasterNI();
    void convIniRespToMessage();
  void runOneStep();

  int countDelRecvRespSig_read, countDelRecvRespSig_write;
  static int count, count_b;//在所有类的实例中共用

  virtual
  ~Converter ();
};

#endif /* AXI4_CONVERTER_HPP_ */
