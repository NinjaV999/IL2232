/*
 * Converter.cpp
 *
 *  Created on: 2019年9月9日
 *      Author: wr
 */

#include <AXI4/Converter.hpp>
#include "parameters.hpp"

int Converter::count = 0;
int Converter::count_b = 0;

Converter::Converter(int id, float inj) {
    NI_id = id;
    countDelRecvRespSig_read = countDelRecvRespSig_write = 0;//计数接收到的读请求信号，接收到的写请求信号
    injection_rate = inj;
}


// simulate cache system, only for slave side
void Converter::MasterNIresponseToOtherMasterNI() {//masterNI相应到其他masterNI

    if (AXI4_out_read.size() > 0) { //如果AXI4 队列中 有待处理的读请求信号
        AXI4Signal *request = AXI4_out_read.front(); //则创建一个指针，指向第一个请求
        if (request->cycles < cycles) {//如果请求信号的cycles小于当前converter的cycles，则表示请求以到达
            AXI4_out_read.pop_front();//取出该请求 ，并生成该请求对应的响应信号response
            //AXI4Signal (int t_id, int t_type, int t_dest, int t_length, int t_QoS,  int t_signal_id,float t_cycles)
            AXI4Signal *response = new AXI4Signal(request->idInSignal_trans, request->type + 1, request->source_id,
                                                  request->data_length, request->QoS, request->signal_id,
                                                  cycles + CACHE_DELAY); // inherit; cache access delay

                                                  //response信号继承了原有request信号的一些属性，继承了id，更改了信号了信号类型，响应信号的destination为请求信号的源，cycles为当前converter时间+cache delay
                                                  //req sig 的到达时间加上cache delay 为 resp sig的 到达时间
                                                  //add
            response->source_id = request->destination;//response 的dest 为 req的source
            response->signal_trans_createcycles = request->signal_trans_createcycles;
            response->signalGoToMem = request->signalGoToMem;
            global_respSignalNum++;//完成上述操作后表示全局的resp signal 生成了
            //std::cout<<"request->signal_trans_createcycles;"<<request->signal_trans_createcycles<<std::endl;
            //add end

            response->test_tag = request->test_tag;
            response->test_tag_qos_convert = request->test_tag_qos_convert;

            response->slave_id = request->slave_id;
            response->sequence_id = request->sequence_id;

            delete request; //删除指针 释放内存
            RespSigInConverterSlave_initializedRead.push_back(response);// 读的resp signal 被储存在该队列里
            count_b++;
            if (printfSW_AXI4Converter_M_SlaveReceivedReadResponse == 1) {
                std::cout << "M_Slave Received " << count_b << std::endl;
            }
        }
    }
    if (AXI4_out_write.size() > 0) { //同理 ，如果队列中有待处理的write 信号
        AXI4Signal *request = AXI4_out_write.front(); //生成一个指针 指向该信号
        if (request->cycles < cycles) {
            AXI4_out_write.pop_front();//从队列中取出该信号
            AXI4Signal *response = new AXI4Signal(request->idInSignal_trans, request->type + 1, request->source_id,
                                                  request->data_length, request->QoS, request->signal_id,
                                                  cycles + CACHE_DELAY); // inherit; cache access delay

                                                  //resp signal 继承 req signal的 部分属性
            //add
            response->source_id = request->destination;
            response->signal_trans_createcycles = request->signal_trans_createcycles;
            response->signalGoToMem = request->signalGoToMem;
            global_respSignalNum ++; //全局相应信号数 加1，记录的是生成响应信号的数量
            //std::cout<<"request->signal_trans_createcycles;"<<request->signal_trans_createcycles<<std::endl;
            //add end

            response->test_tag = request->test_tag;
            response->test_tag_qos_convert = request->test_tag_qos_convert;

            response->slave_id = request->slave_id;
            response->sequence_id = request->sequence_id;

            delete request;
            RespSigInConverterSlave_initializedWrite.push_back(response);//将该resp sig 存入队列

            count_b++;
            if (printfSW_AXI4Converter_M_SlaveReceivedWriteResponse == 1) {
                std::cout << "M_Slave Received " << count_b << std::endl;
            }
        }
    }
}//接受请求信号并生成响应信号


void Converter::convIniRespToMessage() {//将signal 转换为message
    // from AXI4_in_read/write to message_out_inConverterSlave
    if (RespSigInConverterSlave_initializedRead.size() > 0) {//若 read resp sig 队列不为0， 有待处理信号
        //  std::cout<<" will this happen in conver_slave?"<<std::endl;// will happen
        AXI4Signal *AXI4_signal = RespSigInConverterSlave_initializedRead.front();//指向队列开头信号
        if (AXI4_signal->cycles < cycles) {//其到达时间 < 当前converter cycles
            RespSigInConverterSlave_initializedRead.pop_front();//deleted the injected axi signal in converter(converter_master of masterNI)‘axi4_read, which is added in generator.
            //从队列中 删除该 AXI4 信号
            AXI4_signal->source_id = NI_id;//其原来的soruce id 是 req sig的destnation id,现在为 ni_id
            AXI4_signal->respSigIniTime = cycles;//20230306,add resp ini time to read,resp signal initialize cycles 为当前converter id
            Message *message = new Message(NI_id, AXI4_signal, cycles);//基于该AXI4 signal 创建一个message
            message->slave_id = message->signal->slave_id;
            message->sequence_id = message->signal->sequence_id;
            //std::cout<<" message->sequence_id"<< message->sequence_id<<"message->signal->sequence_id "<<message->signal->sequence_id<<std::endl;//
#ifdef WITHOUT_MY_MECHANISM
            if(message_out_inConverterSlave.size()<=CYCLE_LOOP*injection_rate)
#endif
            message_out_inConverterSlave.push_back(message);//存储这个 read resp msg
        }
    }
    if (RespSigInConverterSlave_initializedWrite.size() > 0) {  //同理如果 write reponse signal的队列不为0
        AXI4Signal *AXI4_signal = RespSigInConverterSlave_initializedWrite.front();//生成一个指向该类的指针
        if (AXI4_signal->cycles < cycles) {//判断response sig  是否到达
            RespSigInConverterSlave_initializedWrite.pop_front();//到达后 从队列中取出该 sig
            AXI4_signal->source_id = NI_id;
            AXI4_signal->respSigIniTime = cycles;//20230306,add resp ini time to write
            Message *message = new Message(NI_id, AXI4_signal, cycles);
            message->slave_id = message->signal->slave_id;// axi signal: slave_id = -1; 	  sequence_id = -1;
            message->sequence_id = message->signal->sequence_id;
            // std::cout<<"sequence_id"<<message->sequence_id<<"     slave_id"<<message->slave_id<<std::endl;  slave id can be -1 or other values, have seen 50,161
#ifdef WITHOUT_MY_MECHANISM
            if(message_out_inConverterSlave.size()<=CYCLE_LOOP*injection_rate)//CYCLE_LOOP// 100 by now
    //    std::cout<<"converter CYCLE_LOOP "<<CYCLE_LOOP<<std::endl;//


#endif
            message_out_inConverterSlave.push_back(message);// read response sig 和 write response sig 转换的msg都存入到这个队列中了
            // to be in message_out_inConverterSlave
            //if(message_out_inConverterSlave.size() > 2){
            //  std::cout<<"converter message_out_inConverterSlave.size() "<<message_out_inConverterSlave.size()<<std::endl;
            //}
        }
    }
}

// AXI4_in to message_out_inConverterSlave; message_in_inConverterSlave to AXI4_out;
void Converter::runOneStep() {// 某个执行一次的函数
    convIniRespToMessage();//将 resp sig 转换为 msg
    // from message_in_inConverterSlave to AXI4_out
    if (message_in_inConverterSlave.size() > 0) {//这个队列存储的msg 会被转换为 req sig，在 masterNI.cpp中生存这个队列
        Message *message = message_in_inConverterSlave.front();
        //std::cout<<"will this happen?"<<std::endl;
        //std::cout<<message->signal->type<<" message->signal->type  "<<std::endl;// for masterNI SlaveConverter, only 0, 2
        if (message->out_cycle_inMessage < cycles) {
            message_in_inConverterSlave.pop_front();
            message->signal->cycles = cycles;
            if (message->signal->type == 0 || message->signal->type == 1)  //read request/response
                AXI4_out_read.push_back(message->signal);//将read 信号 存储到 out read
            else
                AXI4_out_write.push_back(message->signal); //将write sig 存储到 out write
            delete message;
        }
    }

    if (cycles == (PRINT - 1) && countDelRecvRespSig_read != 0 && NI_id == SOURCE) {
        float data = (float) (countDelRecvRespSig_read + countDelRecvRespSig_write) * 5 / PRINT;
        std::cout << "converter line 155 data" << data << std::endl; //在某种 判断 条件下 计算得到某个数据
    }
}
//runonestep 函数的作用：将两个resp sig队列中的信号转换为 msg 存储到message_out_inConverterSlave（out 代表将要发出这些msg ）
// 从队列message_in_inConverterSlave取msg，这个队列可能存储的是上级的请求信号，并根据 msg的 type 存储到  AXI4_out_read和 write
//in 表示该队列 将会接受msg
Converter::~Converter() {

}

