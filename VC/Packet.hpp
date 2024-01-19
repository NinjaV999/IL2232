/*
 * Packet.hpp
 *
 *  Created on: 2019年8月15日
 *      Author: wr
 */

#ifndef PACKET_HPP_
#define PACKET_HPP_

#include "AXI4/Message.hpp"
#include "printfSW.hpp"
class Packet: public Message{ //packet 是 message的 子类 ，有msg 生成，一个msg对应一个packet
public:
  Packet( Message* message, int router_num_x, int* NI_num,int t_packet_ID,int t_slaveport_Pakcet_ID,int t_masterport_Pakcet_ID,int t_global_trans_ID);
//t_global_trans_I这应该是指 在整体上trans 发生的次数
  int length;  // length in byte
  int type; // 0 -> request; 1 -> response;
  int vnet;
  int destination[3];  // x, y, output port of the router; from 0 ..

  void dest_convert(int dest, int router_num_x, int* NI_num);
  //added

  /*
   * @brief 0 or 1
   * packet type: int t_type = message->signal->type;// read axi signal int type 0-> read request (93 bits); 1-> read response (6+y bits); 2-> write request (93+y bits); 3-> write response (6 bits);
  switch (t_type){
    case 0: length = (105-1)/8+1;
            type = 0;
            vnet = 0;
            break;
    case 1: length = (18+data_length-1)/8+1;
            type = 1;
            vnet = 1;
            break;
    case 2: length = (105+data_length-1)/8+1;
            type = 0;
            vnet = 0;
            break;
    case 3: length = (18-1)/8+1;
            type = 1;
            vnet = 1;
            break;
  }
   */
  int signal_ID_inpacket;
  int packet_ID;
  int slaveport_Pakcet_ID;
  int masterport_Pakcet_ID;
  int global_trans_ID;
  //added end
  int send_out_time_inPacket;

};


#endif /* PACKET_HPP_ */
