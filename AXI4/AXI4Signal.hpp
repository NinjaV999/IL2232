/*
 * AXI4Signal.hpp
 *
 *  Created on: 2019年9月9日
 *      Author: wr
 */

#ifndef AXI4_AXI4SIGNAL_HPP_
#define AXI4_AXI4SIGNAL_HPP_

class AXI4Signal  //创建了AXI4Signal类
{
public: //这是一个公共的构造函数用于构造该类的对象
  AXI4Signal (int t_id, int t_type, int t_dest, int t_length, int t_QoS,  int t_signal_id,float t_cycles=0.0);
//该构造函数接受的参数用来初始化对象的属性
  int idInSignal_trans; // 0-15;
  //信号的id
  int type; // 0-> read request (93 bits); 1-> read response (6+y bits); 2-> write request (93+y bits); 3-> write response (6 bits);
 //信号的类型以及其对应的位宽，分别是读请求 ，读响应，写请求 和写相应，读响应会传递数据，写请求会写入数据因此都是以固定位宽+数据位宽组成
  int destination;//int. to be converted to be 3-dimension value in packetcpp.destination[2]
  //数据目的地
  int data_length; // in bits <y bits>
  //数据长度 y bits
  int QoS; // 0->URS; 1->LCS (shared VCs with URS packets); 2->GRS; 3->LCS (individual VC(s) only for LCS packets)
//服务质量，QoS的类型还不清除

  //YZADDED
  int signal_id;
  int signal_trans_createcycles;
  int respSigIniTime; //响应信号的初始时间
  int signalGoToMem;//信号是否去mem
  //
  // behavior-level simulation
  int source_id;
  int slave_id;
  int sequence_id;

  // for test
  float cycles;

  int test_tag;
  int test_tag_qos_convert;
  int NI_arrival_time;


  virtual
  ~AXI4Signal ();
};

#endif /* AXI4_AXI4SIGNAL_HPP_ */
