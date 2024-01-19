/*
 * AXI4Signal.cpp
 *
 *  Created on: 2019年9月9日
 *      Author: wr
 */

#include "AXI4/AXI4Signal.hpp"

AXI4Signal::AXI4Signal (int t_id, int t_type, int t_dest, int t_length, int t_QoS,  int t_signal_id,float t_cycles)
{//构造函数的定义 用于初始化 成员属性
  //add
  signal_id=t_signal_id;//全局信号数 这是第几个信号
  signal_trans_createcycles=0;//信号trasns 开始的时间
  respSigIniTime=0;
  //addd end
    idInSignal_trans = t_id;//0-15之间的一个编号 似乎没有什么特殊意义
  type = t_type;
  destination = t_dest;
  data_length = t_length;//128*8bit数
  QoS = t_QoS;
//行为级别模拟
  source_id = -1;

  slave_id = -1;
  sequence_id = -1;

  cycles = t_cycles;//?

  test_tag = 0;
  test_tag_qos_convert = 0;

  NI_arrival_time = 0; //到达Ni的时间
}

AXI4Signal::~AXI4Signal () //析构函数用于释放资源
{
  // TODO Auto-generated destructor stub
}

