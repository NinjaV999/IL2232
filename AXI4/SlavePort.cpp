/*
 * SlavePort.cpp
 *
 *  Created on: 2019年9月13日
 *      Author: wr
 */

#include "AXI4/SlavePort.hpp"
#include "parameters.hpp"
#include <cstddef>

int SlavePort::count = 0; //静态变量被所有 成员所共享

SlavePort::SlavePort(int t_master_num, BasicNI *t_NI) {
    //add
    slavePort_RequestPacketToMessageCredit = 0;
    //addend
    master_num = t_master_num;
    basicNI = t_NI;

    slave_list.resize(TOT_NUM); //slave list 是一个向量 其有168个元素 ，每一个元素也是一个向量 ，这个向量的每一个元素是一个整数
    request_list.resize(TOT_NUM);// //request list 是一个向量 其有168个元素 ，每一个元素也是一个向量 ，这个向量的每一个元素是一个整数
    for (int masterID = 0; masterID < TOT_NUM; masterID++) {
        slave_list[masterID].resize(32); //168
        request_list[masterID].resize(32);
        for (int id = 0; id < 32; id++) {  //初始化这两个向量 168 * 32
            slave_list[masterID][id] = 0; // active list
            for (int seq = 0; seq < S_TSHR_DEPTH; seq++)
                request_list[masterID][id][seq] = (Message *) NULL;
        }
    }
}



void SlavePort::record_refer() {
    // receive message from NoC received request buffer
    Message *messageReqReceived;
    // TDM
    if (basicNI->TDM_network->ni_list[basicNI->id]->signal_buffer_out[0].size() > 0) {
        messageReqReceived = basicNI->TDM_network->ni_list[basicNI->id]->signal_buffer_out[0].front();

        if (messageReqReceived->out_cycle_inMessage < cycles) {
            basicNI->TDM_network->ni_list[basicNI->id]->signal_buffer_out[0].pop_front();
            messageBuffer_receivedRequestMessage.push_back(messageReqReceived);
            messageReqReceived->out_cycle_inMessage = cycles;
        }
    }
    // VC
    //************************************************************************************************
    //  received request packet from VC NI, then this NIself need to response
    // 应该是 某个 slave 接收到的 request msg ,从这个vc的buffer 中读取 msg
    if (basicNI->VC_network->NI_list[basicNI->id]->packetBufferOut_LeavingVCNI_0.size() > 0) {
        messageReqReceived = basicNI->VC_network->NI_list[basicNI->id]->packetBufferOut_LeavingVCNI_0.front();//
        // cout<<messageReqReceived->signal->type<<"message->signal->type  "<<endl;
        if (messageReqReceived->out_cycle_inMessage < cycles) {//如果此时到达 ，则从buffer中取出
            basicNI->VC_network->NI_list[basicNI->id]->packetBufferOut_LeavingVCNI_0.pop_front();
            slavePort_RequestPacketToMessageCredit++;// yz add 20230113
            basicNI->VC_network->NI_list[basicNI->id]->packetBufferOutReq_credit--; // yz add 20230112 packet buffer request credit
            //表示 暂存的 packet 减少了
            messageBuffer_receivedRequestMessage.push_back(messageReqReceived);
            messageReqReceived->out_cycle_inMessage = cycles;
        }
    }
    // process messages in buffer， send into AXINI
    Message *message;
    int queue_delay = 0;
    while (messageBuffer_receivedRequestMessage.size() >
           0) { //  && messageBuffer.front()->out_cycle < cycles: virtual messagebuffer, cost 0 cycle from buffer to list;
        message = messageBuffer_receivedRequestMessage.front();
        // cout << messageReqReceived->signal->type << " message->signal->type  " << endl;// 0 or 2, means request signal's message
        messageBuffer_receivedRequestMessage.pop_front();
        slavePort_RequestPacketToMessageCredit--;
        //cout<<cycles<<"cycles slavePort_RequestPacketToMessageCredit"<<slavePort_RequestPacketToMessageCredit<<endl;
        int id = message->signal->idInSignal_trans;
        int NI_id = message->NI_id;
        int sequence_id = message->sequence_id;
        // cout << NI_id << id << sequence_id << endl;
        // if(sequence_id < slave_list[NI_id][id]) // judge if the request is a new series
        //   slave_list[NI_id][id] = 0;
        //将该msg填充到对应的request list 中
        request_list[NI_id][id][sequence_id] = message; //message->signal type is 0 or 2  Here is request
        //cout<<message->signal->type<<"message->signal->type  "<<endl;
        while (request_list[NI_id][id][slave_list[NI_id][id]] !=
               NULL) {   // This is the receive request list
            request_list[NI_id][id][slave_list[NI_id][id]]->out_cycle_inMessage =
                    cycles + queue_delay + DELAY_FROM_P_TO_M +
                    SLAVE_LIST_REFER_DEALY; // 1 cycle delay for each request, adding iteratively
            queue_delay++;
            count++;
            //request_list[NI_id][id][slave_list[NI_id][id]]->
            //存入message_request队列中
            basicNI->enqueue(
                    request_list[NI_id][id][slave_list[NI_id][id]]); //  yz comments basic receive the request message from VCNI
            request_list[NI_id][id][slave_list[NI_id][id]] = (Message *) NULL; // reset
            // slave_list[NI_id][id] =  (slave_list[NI_id][id] + 1)%16;//loss packet//message not injected into network
            slave_list[NI_id][id] =
                    (slave_list[NI_id][id] + 1) % S_TSHR_DEPTH; // 20230110added to avoid not read request_list[][][>16]
           //更新sequence id
            if (printfSW_AXI4SlavePort_SlavePortReceived == 1) {
                // cout << "Slave Port received: " << count << endl;
                cout << "slave_list[NI_id][id] " << NI_id << "	" << id << "	" << slave_list[NI_id][id] << endl;
            }
        }
    }
}

void SlavePort::to_NoC()//message to packet, for master NI and slave NI
{
    //FOR slaveNI one step, the message is from message repsonse message_response.pop_front();
    Message *messageToRespPacket = basicNI->dequeueMessage();//slaveni message_response deque one message
    //从slave 的 message response中取出 第一个msg
    if (messageToRespPacket == NULL) // input buffer is empty or message is not ready
        return;
    // QoS: 0->URS; 1->LCS; 2->GRS; 3->LCS (individual VCs)
    //cout<<messageToRespPacket->signal->type<<" message->signal->type  "<<endl;// 1 or 3 . So now is resp message
    if (messageToRespPacket->signal->QoS != 2) {                                 // to VC NoC
        extern int global_Packet_ID;    // added
        extern int slaveport_Pakcet_ID; // added
        // extern int global_trans_ID;//added
        // slave port global_trans_ID use master port
        // Packet(Message* message, int router_num_x, int* NI_num,int t_packet_ID,int t_slaveport_Pakcet_ID,int t_masterport_Pakcet_ID,int t_global_trans_ID);
        // SHOULD reuse the receive request's global_trans_ID. Now just use some meaningless number
        // packet  request type == message type R req/W req/R resp/W resp
        Packet *packetRespIni = new Packet( messageToRespPacket, basicNI->router_num_x, basicNI->NI_num, global_Packet_ID,
                                    slaveport_Pakcet_ID, -199, -99);//yz20230220: here slaveNI give response
        //cout<<cycles<<" slave port packetr type"<<packetRespIni->type<<endl;

        global_Packet_ID++;    // yz added
        slaveport_Pakcet_ID++; // added

        packetRespIni->out_cycle_inMessage = cycles + DELAY_FROM_M_TO_P;
        delete messageToRespPacket;
        //cout<<"slave port packet type "<< packet->type<<" del message type "<<message->signal->type <<endl;
        basicNI->VC_network->NI_list[basicNI->id]->packetBufferList_xVNToFlitize[packetRespIni->vnet]->enqueue(packetRespIni);

        //将resp msg 准为packet 存入 一个 vc的buffer中方
        assert(packetRespIni->vnet > 0);// YZ should alway send to resp VN
        packetRespIni->signal->NI_arrival_time = cycles;
    } else { // to TDM NoC
        assert(messageToRespPacket->signal->QoS == 2);
        Signal *signal = new Signal(messageToRespPacket);
        signal->out_cycle_inMessage = cycles + DELAY_FROM_M_TO_P;
        delete messageToRespPacket;
        basicNI->TDM_network->ni_list[basicNI->id]->signal_buffer.push_back(signal);
        signal->signal->NI_arrival_time = cycles;
    }
}

SlavePort::~SlavePort() {
    // TODO Auto-generated destructor stub
}
