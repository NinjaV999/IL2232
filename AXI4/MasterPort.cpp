/*
 * MasterPort.cpp
 *
 *  Created on: 2019年9月13日
 *      Author: wr
 */

#include "AXI4/MasterPort.hpp"
#include "VC/Packet.hpp"
#include "TDM/Signal.hpp"
#include <cstddef>
#include <cassert>
#include <cmath>

int MasterPort::count = 0;

MasterPort::MasterPort(int slave, MasterNI *t_masterNI, float inj) {
    slave_num = slave;
    masterNI = t_masterNI;

    master_list.resize(32);// 两个 vector  大小为32
    response_list.resize(32);
    for (int id = 0; id < 32; id++) {
        for (int dest = 0; dest < TOT_NUM; dest++) {
            master_list[id][dest] = 0; //master[id][key(slave)]= value (sequence id)] 感觉是到达这个slave的路径
        }
    }
    record_count = 0;
    injection_rate = inj;
    history_count = 0;
    time_counter = 0;
}

#ifndef  MasterNI_to_buffer_fullNI_NoC
void MasterPort::sendReqMessageToPacket_toVCNIPacketBuffer() {//send request
//用于发送请求msg，如果VN则得到该msg对应的packet 并存入 根据索引找到的虚拟通道中 ，若是TDM 生成对应的signal 并存入对应TDM network的 buffer中
    // if(cycles == 500 && masterNI->id == SOURCE)
    // std::cout<<"Average delay in NI:"<< float(time_counter)/floor(CYCLE_LOOP*injection_rate*ID_RATIO) <<std::endl;

    if (masterNI->converterMasterInMasterNI->message_out.size() > 0 &&
        masterNI->converterMasterInMasterNI->message_out.front()->out_cycle_inMessage < cycles) {
//如果 Master converter的 message out 队列 不为空 且 其到达周期<小于当前全局周期， 则我们认为其第一个信号已经到达
#ifdef WITHOUT_MY_MECHANISM
        if (masterNI->id == SOURCE && record_count >= CYCLE_LOOP*injection_rate*(1-ID_RATIO))
          return;
#endif

        Message *message = masterNI->converterMasterInMasterNI->message_out.front();//用于一个 指针 指向该周期
        masterNI->converterMasterInMasterNI->message_out.pop_front();//经该msg从队列中取出
        history_count++;//用于记录请求信号的到达次数
        if (history_count > int(ceil(CYCLE_LOOP * injection_rate * (1 - ID_RATIO))) &&
            history_count <= CYCLE_LOOP * injection_rate && masterNI->id == SOURCE)
            time_counter += cycles - message->signal->cycles; //不知道在算什么？

        //0-> read request; 1-> read response; 2-> write request; 3-> write response;

        assert(message->signal->type == 0 || message->signal->type == 2);//这个判断说明了 message out 权威 req msg
        // for the write transaction, the initial id is between 0 and 15. In the interconnect system, it is between 16 and 31 for write/read transaction distinction.
        // the write and read transactions have no ordering requirement even they have the same ID.
        if (message->signal->type == 2) //如果为写请求，则为了能够进一步区分 ，将其原本的id 控制到 16-31
            message->signal->idInSignal_trans += 16;
        int id = message->signal->idInSignal_trans; //同时将其两个参数拿出来没用
        int dest = message->signal->destination;


        // find the next sequence number for this id & dest in the reference list
        int seq = master_list[id][dest];
        master_list[id][dest] = (seq + 1) % S_TSHR_DEPTH;//S_TSHR_DEPTH=40，seq的值不会超过40

        std::pair<int, int> key = std::make_pair(dest, seq);//将 dest 和 seq 组成为一个键值对
        std::pair<std::pair<int, int>, Message *> triple = std::make_pair(key, (Message *) NULL);// 将 key 和 message组成 一个 新的键值对
        response_list[id].push_back(triple);//并存入到 response list 中，response list 记录的是 sig 初始化 id，目的地 ，seq id 以及对应的响应消息
        record_count++;
        //std::cout << response_list[id].size() << "##########" << std::endl;
        //std::cout  << "masterportcpp record_count"<< record_count << std::endl;
        message->slave_id = dest;
        message->sequence_id = seq;
        message->out_cycle_inMessage = cycles + MASTER_LIST_RECORD_DELAY + DELAY_FROM_M_TO_P;

        message->signal->slave_id = dest;
        message->signal->sequence_id = seq;

        if (message->signal->QoS != 2) { // to VC NoC
            extern int global_Packet_ID;// yz added
            extern int masterport_Pakcet_ID;//added
            extern int global_trans_ID;//added
            Packet *packet = new Packet(message, masterNI->router_num_x, masterNI->NI_num, global_Packet_ID, 0,
                                        masterport_Pakcet_ID,
                                        global_trans_ID);//
                                        // slaveport_Pakcet_ID=0 in this master port
                                        //通过该message 以及global packet id,master port packet id , global trans id 来构建 其对嗯的message
            //cout<<" " <<packet->vnet<<"  "<<message ->signal->type<<" "<<endl;// vnet 0: req vnet 1: resp.  signal: 0/2: req
            global_Packet_ID++;//yz added req msg 转换为 packet
            masterport_Pakcet_ID++;//added
            global_trans_ID++;//added
            delete message;
            packet->signal->NI_arrival_time = cycles;//该 packet 到达 ni的时间 是当前系统的global cycles
            //masterNI 是 basic NI的子类 ，根据 masterNI的 id 来获取 NI——list对应的 NI， 根据 packet的net属性获取一个虚拟通道的 buffer，将当前packet 存入
            masterNI->VC_network->NI_list[masterNI->id]->packetBufferList_xVNToFlitize[packet->vnet]->enqueue(
                    packet);   // takes 2 cycles to arrive here from the AXI4 generator//vnet  = 0
            //cout<<"packet->vnet in converterMastetMeesage_out"<<packet->vnet<<endl;// 0
        } else {// to TDM NoC
            assert(message->signal->QoS == 2);
            Signal *signal = new Signal(message);
            delete message;
            masterNI->TDM_network->ni_list[masterNI->id]->signal_buffer.push_back(signal);
            count++;
            signal->signal->NI_arrival_time = cycles;
            cout << "Master Port send: " << count << endl;
        }
    }
}
#endif

#ifdef  MasterNI_to_buffer_fullNI_NoC

void MasterPort::buffer_fullNI_record() {
    if (masterNI->converterMasterInMasterNI->message_out.size() > 0 &&
        masterNI->converterMasterInMasterNI->message_out.front()->out_cycle_inMessage < cycles) {
        Message *message = masterNI->converterMasterInMasterNI->message_out.front();
#ifdef WITHOUT_MY_MECHANISM
        if (masterNI->id == SOURCE && record_count >= CYCLE_LOOP*injection_rate*(1-ID_RATIO))
          return;
#endif
        //for(std::deque<Message*>::iterator ite = masterNI->converter_master->message_out.begin();ite<masterNI->converter_master->message_out.end();ite++)

        for (int i = 0; i < 500; i++) {
            if (masterNI->basicNI_list_inBasicNI[message->signal->destination]->basicNI_packetWaitNum > 0) {
                //   cout << "actual RLaction_cycles_inMasterPort " << RLaction_cycles_inMasterPort << endl; // to observe actual action change or not
                message->out_cycle_inMessage = message->out_cycle_inMessage + RLaction_cycles_inMasterPort;
                masterNI->converterMasterInMasterNI->message_out.push_back(message);
                masterNI->converterMasterInMasterNI->message_out.pop_front();
                message = masterNI->converterMasterInMasterNI->message_out.front();
                break;//previous miss this breka: previous days means days before 20230213
            } else {
                message = masterNI->converterMasterInMasterNI->message_out.front();
            }
        }
        //  VCNI_packetWaitNum

        masterNI->converterMasterInMasterNI->message_out.pop_front();
        history_count++;
        if (history_count > int(ceil(CYCLE_LOOP * injection_rate * (1 - ID_RATIO))) &&
            history_count <= CYCLE_LOOP * injection_rate && masterNI->id == SOURCE)
            time_counter += cycles - message->signal->cycles;
        //0-> read request; 1-> read response; 2-> write request; 3-> write response;

        assert(message->signal->type == 0 || message->signal->type == 2);
        // for the write transaction, the initial id is between 0 and 15. In the interconnect system, it is between 16 and 31 for write/read transaction distinction.
        // the write and read transactions have no ordering requirement even they have the same ID.
        if (message->signal->type == 2)
            message->signal->id += 16;
        int id = message->signal->id;
        int dest = message->signal->destination;


        // find the next sequence number for this id & dest in the reference list
        int seq = master_list[id][dest];
        master_list[id][dest] = (seq + 1) % S_TSHR_DEPTH;//S_TSHR_DEPTH=40

        std::pair<int, int> key = std::make_pair(dest, seq);
        std::pair<std::pair<int, int>, Message *> triple = std::make_pair(key, (Message *) NULL);
        response_list[id].push_back(triple);
        record_count++;
        //std::cout << response_list[id].size() << "##########" << std::endl;
        //std::cout  << "masterportcpp record_count"<< record_count << std::endl;
        message->slave_id = dest;
        message->sequence_id = seq;
        message->out_cycle_inMessage = cycles + MASTER_LIST_RECORD_DELAY + DELAY_FROM_M_TO_P;

        message->signal->slave_id = dest;
        message->signal->sequence_id = seq;

        if (message->signal->QoS != 2) { // to VC NoC
            extern int global_Packet_ID;// yz added
            extern int masterport_Pakcet_ID;//added
            extern int global_trans_ID;//added
            Packet *packet = new Packet(message, masterNI->router_num_x, masterNI->NI_num, global_Packet_ID, 0,
                                        masterport_Pakcet_ID,
                                        global_trans_ID);//slaveport_Pakcet_ID=0 in this master port

            //add
            //  if (packet->packet_ID ==parameter_UniqueID_trackThisSignalLife )
            //   {
            //     cout<<"MasterPort::record()  MasterPort::masterNI->id "<<MasterPort::masterNI->id << "cycle " <<cycles<<endl;
            //    }
            //add end


            global_Packet_ID++;//yz added
            masterport_Pakcet_ID++;//added
            global_trans_ID++;//added
            delete message;
            packet->signal->NI_arrival_time = cycles;
            masterNI->VC_network->NI_list[masterNI->id]->packetBuffer_list[packet->vnet]->enqueue(
                    packet);   // takes 2 cycles to arrive here from the AXI4 generator
        } else {// to TDM NoC
            assert(message->signal->QoS == 2);
            Signal *signal = new Signal(message);
            delete message;
            masterNI->TDM_network->ni_list[masterNI->id]->signal_buffer.push_back(signal);
            count++;
            signal->signal->NI_arrival_time = cycles;
            cout << "Master Port send: " << count << endl;
        }
    }
}

#endif

void MasterPort::refer() {
    // receive message from NoC response buffer
    Message *message;
    //TDM
    //cout << "!!!!!!" << endl;
    if (masterNI->TDM_network->ni_list[masterNI->id]->signal_buffer_out[1].size() > 0) {
        message = masterNI->TDM_network->ni_list[masterNI->id]->signal_buffer_out[1].front();
       // cout<<message->signal->type<<" message->signal->type  "<<endl;// 1 or 3 . So now is resp message
        if (message->out_cycle_inMessage < cycles) {
            masterNI->TDM_network->ni_list[masterNI->id]->signal_buffer_out[1].pop_front();
            messageBufferInMasterPort.push_back(message);
            message->out_cycle_inMessage = cycles;
        }
    }
    // cout << "#######" << endl;
    //VC
    if (masterNI->VC_network->NI_list[masterNI->id]->packetBufferOut_LeavingVCNI_1.size() > 0) {
        message = masterNI->VC_network->NI_list[masterNI->id]->packetBufferOut_LeavingVCNI_1.front();//actually packet here. inherent message type//receive this packet from NoC
        //cout<<message->signal->destination<<" dest "<<masterNI->id<< " self"<<endl;
        //cout<<message->signal->type<<" message->signal->type  "<<endl;// 1 or 3 . So now is resp message
        if (message->out_cycle_inMessage < cycles) {
            masterNI->VC_network->NI_list[masterNI->id]->packetBufferOut_LeavingVCNI_1.pop_front();
            masterNI->VC_network->NI_list[masterNI->id]->packetBufferOutResp_credit--;//yz add 20230112 packet buffer response credit
            //credit 用于某种方法记录了buffer数量
            messageBufferInMasterPort.push_back(message);
            message->out_cycle_inMessage = cycles;
        }

    }
    if (messageBufferInMasterPort.size() > 0 && messageBufferInMasterPort.front()->out_cycle_inMessage < cycles) {
        message = messageBufferInMasterPort.front();
        //cout<<message->signal->type<<" message->signal->type  "<<endl;// 1 or 3 . So now is resp message
        messageBufferInMasterPort.pop_front();
        int id = message->signal->idInSignal_trans;
        int slave_id = message->slave_id;
        //cout<< " slave_id250line " << slave_id<<" self id "<< masterNI->id << " message->sig dest " <<message->signal->destination <<endl;
        int sequence_id = message->sequence_id;
        // check response list for  InOrder
        std::deque<TRIPLE>::iterator iter;
        iter = response_list[id].begin();
        for (; iter != response_list[id].end(); iter++) {
            // insert signal to corresponding response_list position
            if ((*iter).first.first == slave_id && (*iter).first.second == sequence_id) {
                (*iter).second = message;
                // check if the response of the first record arrives.
                iter = response_list[id].begin();
                while ((*iter).second != NULL && iter < response_list[id].end()) {
                    // deliver the first response message
                    (*iter).second->out_cycle_inMessage = cycles + MASTER_LIST_REFER_DELAY + DELAY_FROM_P_TO_M;
                    //更新 该 msg的 到达时间
                    if ((*iter).second->signal->type == //在网络中传输的是时候 write和read 的 id 需要进行区分 ，但回到master时 又初始化为原来的
                        3) // for write response message, turn its id back to the original value.
                        (*iter).second->signal->idInSignal_trans -= 16;
                    masterNI->converterMasterInMasterNI->messageReceivedResp_in_toBeDeleted.push_back((*iter).second);
                    //将该msg 加入将要被删除的 等待队列
                     //cout<<(*iter).second->signal->type<<" what is here "<<endl;// 1 or 3 here read resp or write resp
                    iter = response_list[id].erase(iter);
                    record_count--;
                    assert(record_count >= 0);
                }
                // if the record list is empty, then re-initial the record list
                /*if(iter==response_list[id].end()){
                    for(int slave=0; slave<slave_num; slave++)
                        master_list[id][slave] = 0;
                }*/
                break;
            }
        }
    }

}

MasterPort::~MasterPort() {
    // TODO Auto-generated destructor stub
}

