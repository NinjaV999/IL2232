/*
 * Converter.cpp
 *
 *  Created on: 2019年9月9日
 *      Author: wr
 */

#include <AXI4/Converter_Master.hpp>
#include "parameters.hpp"

int Converter_Master::count = 0;
int Converter_Master::count_b = 0;

Converter_Master::Converter_Master(int id, float inj) {
    NI_id = id;
    countDelRecvRespSig_read = countDelRecvRespSig_write = 0;
    injection_rate = inj;
}
//构造函数 在生成对象时初始化该对象的参数

// only for master side
void
Converter_Master::delReceivedRespSig() {  // should take the AXI4 signal transfer time into consideration when calculating the receive time
    //计算到达时间时 应该考虑 AXI4 信号的传输时间，在创建两个队列 用于分别接受回来的resp sig ，这个函数的作用是去删除这些被接收到的 函数
    if (AXI4OutRead_ToDelInConvMaster.size() > 0) {
        AXI4Signal *response = AXI4OutRead_ToDelInConvMaster.front();
        if (response->cycles < cycles) {
            AXI4OutRead_ToDelInConvMaster.pop_front();
            //std::cout << "Receive read response at cycle:" << cycles << ". data length:"<< response->data_length <<std::endl;
            delete response;
            countDelRecvRespSig_read++;//记录了每个对象 删除 read resp sig的次数
            count++;//记录了整个类 删除 resp sig的次数
            if (printfSW_AXI4Converter_MasterReceivedReadResponse == 1) {
                std::cout << "Master Received read response " << count << std::endl;
            }
        }
    }
    if (AXI4OutWrite_ToDelInConvMaster.size() > 0) {
        AXI4Signal *response = AXI4OutWrite_ToDelInConvMaster.front();
        if (response->cycles < cycles) {
            AXI4OutWrite_ToDelInConvMaster.pop_front();
            //std::cout << "Receive write response at cycle:" << cycles << std::endl;
            delete response;
            countDelRecvRespSig_write++;
            count++;
            if (printfSW_AXI4Converter_MasterReceivedWriteResponse == 1) {
                std::cout << "Master Received " << count << std::endl;
            }
        }
    }
}

//dealWithAXI4ToSendReq_in IS ONE step in runOneStep
void Converter_Master::dealWithAXI4ToSendReq_in() {
    // from AXI4_in_read/write to message_out
    if (AXI4ReqFromGenInMasterConverter_in_read.size() > 0) {
        AXI4Signal *AXI4_signal = AXI4ReqFromGenInMasterConverter_in_read.front();
        if (AXI4_signal->cycles < cycles) {
            AXI4ReqFromGenInMasterConverter_in_read.pop_front();//deleted the injected axi signal in converter(converter_master of masterNI)‘axi4_read, which is added in generator.
            AXI4_signal->source_id = NI_id;
            Message *message = new Message(NI_id, AXI4_signal, cycles);
            message->slave_id = message->signal->slave_id;
            message->sequence_id = message->signal->sequence_id;
            //std::cout<<" message->sequence_id"<< message->sequence_id<<"message->signal->sequence_id "<<message->signal->sequence_id<<std::endl;//
#ifdef WITHOUT_MY_MECHANISM
            if(message_out.size()<=CYCLE_LOOP*injection_rate)
#endif
            message_out.push_back(message);
        }
    }
    if (AXI4ToSendReq_in_write.size() > 0) {
        AXI4Signal *AXI4_signal = AXI4ToSendReq_in_write.front();
        if (AXI4_signal->cycles < cycles) {
            AXI4ToSendReq_in_write.pop_front();
            AXI4_signal->source_id = NI_id;
            Message *message = new Message(NI_id, AXI4_signal, cycles);
            message->slave_id = message->signal->slave_id;// axi signal: slave_id = -1; 	  sequence_id = -1;
            message->sequence_id = message->signal->sequence_id;
            // std::cout<<"sequence_id"<<message->sequence_id<<"     slave_id"<<message->slave_id<<std::endl;  slave id can be -1 or other values, have seen 50,161
#ifdef WITHOUT_MY_MECHANISM
            if(message_out.size()<=CYCLE_LOOP*injection_rate)//CYCLE_LOOP// 100 by now
    //    std::cout<<"converter CYCLE_LOOP "<<CYCLE_LOOP<<std::endl;//


#endif
            message_out.push_back(message);// to be in message_out
            //if(message_out.size() > 2){
            //  std::cout<<"converter message_out.size() "<<message_out.size()<<std::endl;
            //}
        }
    }
}

// AXI4_in to message_out; messageReceivedResp_in_toBeDeleted to AXI4_out;
void Converter_Master::runOneStep() {
    dealWithAXI4ToSendReq_in();// send creqted req out 将生成器的产生的sig 转化为 msg 存储到 msg out 队列中（要发出的req msg）
    sendRespMessageToDel();// move received resp to buffer waitting to be deleted 将接收到的 resp msg 转化成 signal 存储起来等待 被 删除
    delReceivedRespSig();// actual del the received resp message 删除 之前存储的 sig
}
void Converter_Master::sendRespMessageToDel(){//这个函数是把将要 删除的 resp msg  转换成 AXI4 信号 存储起来
    if (messageReceivedResp_in_toBeDeleted.size() > 0) {
        Message *message = messageReceivedResp_in_toBeDeleted.front();
//        if ((message->signal->type != 1) & (message->signal->type != 3)) {
//            std::cout << message->signal->type << " message->signal->type  " << std::endl;// for converter master, only 1 and 3
//        }
        if (message->out_cycle_inMessage < cycles) {
            //  std::cout<<" run in master converter?"<<std::endl;// run in master converter, yes
            messageReceivedResp_in_toBeDeleted.pop_front();
            message->signal->cycles = cycles;
            if (message->signal->type == 0 || message->signal->type == 1)  //read request/response
                AXI4OutRead_ToDelInConvMaster.push_back(message->signal);
            else
                AXI4OutWrite_ToDelInConvMaster.push_back(message->signal);
            delete message;
        }
    }

    if (cycles == (PRINT - 1) && countDelRecvRespSig_read != 0 && NI_id == SOURCE) {
        float data = (float) (countDelRecvRespSig_read + countDelRecvRespSig_write) * 5 / PRINT;
        std::cout << "converter line 155 data" << data << std::endl;
    }
}

Converter_Master::~Converter_Master() {

}

