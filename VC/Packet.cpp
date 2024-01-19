/*
 * Packet.cpp
 *
 *  Created on: 2019年8月15日
 *      Author: wr
 */

#include "VC/Packet.hpp"
#include <iostream>
//type convert: add change type and vnet

Packet::Packet( Message* message, int router_num_x, int* NI_num, int t_packet_ID, int t_slaveport_Pakcet_ID, int t_masterport_Pakcet_ID,int t_global_trans_ID):Message(message){
  packet_ID=t_packet_ID; // 构造函数给属性赋值
  //add
  slaveport_Pakcet_ID=t_slaveport_Pakcet_ID;
  masterport_Pakcet_ID=t_masterport_Pakcet_ID;
  signal_ID_inpacket=message->signal->signal_id; //全局信号数 ，可以得知这个packet是从哪个信号来的

  if(pritfSW_VCPacket_PacketSignalIDshow ==1){//没用
    std::cout<< "signal_id " << message->signal->signal_id <<" ";
    std::cout<< "slaveport_Pakcet_ID " << slaveport_Pakcet_ID <<" ";
    std::cout<< "masterport_Pakcet_ID " << masterport_Pakcet_ID <<" ";
    std::cout<< "packet_ID " << packet_ID <<" "<<std::endl;
   // if (message->signal->signal_id==150){
    //    std::cout<<"150 source_id" <<message->signal->source_id<< " ";
   //     std::cout<< "destination" << message->signal->destination<<std::endl;
  //  }
  }
  global_trans_ID = t_global_trans_ID;//全局前艺术

   //addend
  int RWReqResp_type = message->signal->type;//yz read axi signal int type 0-> read request (93 bits); 1-> read response (6+y bits); 2-> write request (93+y bits); 3-> write response (6 bits);
 //继承了 signal的 type
  int data_length = message->signal->data_length;// yz data length = messaged(inheritate axi signal) data length
 //继承了 signal的 数据长度
  dest_convert(message->signal->destination, router_num_x, NI_num) ;//COVNERT signal->destination to three dimention destination[3]// destination x, y, output port of the router; from 0 ..
  //根据某种算法把 0-167的destnation 变为3个维度的dest 包括（x，y） 以及输出端口
  //std::cout<<"packet ini RWReqResp_type"<<RWReqResp_type<<std::endl;
  switch (RWReqResp_type){ //根据 packet 的 种类对 length 重新 赋值
    case 0: length = (105-1)/8+1;//14 byte ，读请求  req 为0 resp 为 1
            type = 0;
            vnet = 0;
            break;
    case 1: length = (18+data_length-1)/8+1;//131 byte ，读应答
            type = 1;
            vnet = 1;
            break;
    case 2: length = (105+data_length-1)/8+1;//142 byte，写请求
            type = 0;
            vnet = 0;
            break;
    case 3: length = (18-1)/8+1;//3 byte//写应答
            type = 1;
            vnet = 1;// ?
            break;
  }
    send_out_time_inPacket = 0;



}

/*
  * @brief convert dest to be destination0/1/2. router is the temp value for convert yz
   */
void Packet::dest_convert(int dest, int router_num_x, int* NI_num){
  int hist_num = 0, cur_num = 0, router = 0;
  while (cur_num <= dest){
      hist_num = cur_num;
      cur_num += NI_num[router];
      router++;
  }
  router--;
  destination[0] = router/router_num_x;
  destination[1] = router%router_num_x;
  destination[2] = dest-hist_num;
  //std::cout << destination[0] <<destination[1] << destination[2]<<std::endl;
}


