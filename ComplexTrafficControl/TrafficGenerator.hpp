/*
 * TrafficGenerator.hpp
 *
 *  Created on: Oct 25, 2019
 *      Author: wr
 */

#ifndef TRAFFICGENERATOR_HPP_
#define TRAFFICGENERATOR_HPP_

#include "AXI4/AXI4Signal.hpp"
#include "AXI4/MasterNI.hpp"
#include <vector>
#include <stdlib.h>
#include <iomanip>
//static int overall_signal_num=0;//yz added
extern int overall_signal_num;//yz added
extern int global_injSignalHighGroupNum; //yz 20230427
class TrafficGenerator
{
public:
/* @brief  int average_data_length is set to be 128  because: TrafficGenerator* generator = new TrafficGenerator(i,0.2,128, count);
     *
 */
  TrafficGenerator (int id, float read_ratio, int average_data_length, int count);
  int id;
  float ratio; // r/all//yz read by all packet
  int data_length; // in Byte //yz 128
  int destination_num;//yz =4

  std::vector<int> destination_list_single; // 一个整数类型的向量
  int refAlpha2;
  int alpha1, beta1, base1, alpha2, beta2, base2;
  int state1, state2; // off -> 0; on -> 1;//0 表示关闭 1 表示开启
  int group;
  int trans_id;

  int traffic_mode[5];//不同的traffic 模型
  int count;

  /*
   * @brief  create_traffic_num set traffic_momde values based on traffic num values.  traffic_num is set in main.cpp
   * for(int i=0; i<5; i++){
   * 	traffic_mode[i] = traffic_num[count][i];
   * 	}
   */
  void create_traffic_num();


  /*
   * @brief  According ID, return group id (1,2,3,..8).
   */
  int group_calculator(int id);


  /*
   * @brief  based on these values, set the new value of state. And if new state=0 return false which means off, new state=1 return true which means on
   */
  bool on_or_off(int alpha, int beta, int base, int& state);


  /*
   * @brief  according to group, return10/100/200/1000
         case 0: return 10;
      case 1: return 100;
      case 2: return 200;
      case 3: return 1000;
   */
  int group_to_ns(int group);


  bool request();
  void inject(MasterNI*);

  static int counta;

  int injectCount_perInterval[100];

  int total_hop, request_num;



  /*
   * @brief  just cout avg hops
   */
  void average_hop();

  static int LCS_NUM[1000];
  static int GRS_NUM[1000];
  static int URS_NUM[1000];

  int LCS_per_node; // 不同服务质量的类型 低延迟
  int URS_per_node;// 不提供低延迟服务或带宽保证。
  int GRS_per_node;//对容量流保证带宽 能够容忍延迟
/*
 *
 *
Latency Critical Service (LCS):

描述： 针对突发但非流式消息传输的快速转发服务。提供低延迟的传递服务，但不保证例如类似 CPU 的主设备的传递带宽。
特点： 适用于对延迟敏感，但不需要传递带宽保证的场景。
Guaranteed Rate Service (GRS):

描述： 一种流式服务，为需要带宽但能容忍延迟（例如 GPU 等主设备）的大容量流提供保证带宽。
特点： 适用于对传递带宽有要求，但可以容忍一定延迟的场景。
Unspecified Rate Service (URS):

描述： 一种用于提供某种公平性（公平对待）的尽力而为的服务。根据当前可用资源尽快、尽可能地传递消息，但不提供低延迟服务或带宽保证（例如 SATA 和 USB 接口）。
特点： 适用于需要某种公平性，但不关心低延迟或带宽保证的场景。
 */


  ~TrafficGenerator ();
};



#endif /* TRAFFICGENERATOR_HPP_ */
