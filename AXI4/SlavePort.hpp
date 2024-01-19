/*
 * SlavePort.hpp
 *
 *  Created on: 2019年9月13日
 *      Author: wr
 */

#ifndef AXI4_SLAVEPORT_HPP_
#define AXI4_SLAVEPORT_HPP_

#include <printfSW.hpp>//sw to printf
#include "AXI4/BasicNI.hpp"
#include "AXI4/Message.hpp"
#include "TDM/TNetwork.hpp"
#include <vector>
#include <deque>

class BasicNI;
class TNetwork;
//
extern int global_Packet_ID;// used in cpp
extern int slaveport_Pakcet_ID;// used in cpp

class SlavePort
{
public:
  typedef std::map<int, Message*> DOUBLE; //Double 类型是 一个整数和指向msg类型的对象的指针

  SlavePort (int t_master_num, BasicNI* t_NI);

  int master_num;
  BasicNI* basicNI;

  std::vector<std::vector<int> > slave_list;
  std::vector<std::vector<DOUBLE> > request_list; // masterID;ID;<sequenceID + message>

  std::deque<Message*> messageBuffer_receivedRequestMessage;

  static int count;

  void record_refer();//从 VC的 某个buffer中取出 一个msg 并将其存入 msg request 队列中
  void to_NoC();//从message resp 队列中 取出第一个 msg ，转换为packet 并存入 对应的 vc的 buffer中
  //add
  void to_buffer_fullNI_NoC();
  int slavePort_RequestPacketToMessageCredit;
  //add end
  virtual
  ~SlavePort ();
};

#endif /* AXI4_SLAVEPORT_HPP_ */
