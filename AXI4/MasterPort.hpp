/*
 * MasterPort.hpp
 *
 *  Created on: 2019年9月13日
 *      Author: wr
 */

#ifndef AXI4_MASTERPORT_HPP_
#define AXI4_MASTERPORT_HPP_

#include <utility>
#include <map>
#include <deque>
#include <vector>

#include "AXI4/Message.hpp"
#include "AXI4/MasterNI.hpp"


class MasterNI;


class MasterPort
{
public:
  typedef std::pair<std::pair<int,int>, Message*> TRIPLE;//自定义了一个三元组 （int，int，message）

  MasterPort (int t_slave_num, MasterNI* t_masterNI, float inj);

  int slave_num;
  MasterNI* masterNI;

  float injection_rate;

  // for record reference
  //add masterlist: 32*168 value of master_lis[?][?] is seq_id
  std::vector<std::map<int,int> > master_list;  // vector: id, from 0 to 15 <read>, from 16 to 31 <write>; map: slaveID and the sequenceID (0, 1, ..) to this slave//for(int id=0;id<32; id++) {for(int dest=0; dest<TOT_NUM; dest++){ master_list[id][dest] = 0;
  //构成一个向量 每个元素是一个键值对， key 和 value 都是 int 类型，第一个id可能是信号transaction的 初始id、
  // for M-TSHR
 std::vector<std::deque<TRIPLE> > response_list;  // vector: id, from 0 to 15 <read>, from 16 to 31 <write>; <<slaveID, sequenceID>, response>
  //定义一个向量每个元素都是 一个双端对列， 每个双端对列 都 是一个三元组
  void sendReqMessageToPacket_toVCNIPacketBuffer();
  //总结该函数 用于将masterNI 中 msg out队列中 产生的请求信息 经过一些列的操作，转换成VC网络中 某个 slave NI对应的 packet 并储存下来
  void refer();
  //这个是基于发送的packet ，从某个ni中取得对应的 响应message ，先存储到respose list ，在存储到 masterNI的对队列messageReceivedResp_in_toBeDeleted 中等待被删除

  std::deque<Message*> messageBufferInMasterPort;//一个中间队列 ，用于暂存从VC中获取的resp msg

  int record_count;
  int history_count;
  int time_counter;

  static int count;

  //add
  void buffer_fullNI_record();
  int RLaction_cycles_inMasterPort=0;
  //add end
  virtual
  ~MasterPort ();
};

#endif /* AXI4_MASTERPORT_HPP_ */
