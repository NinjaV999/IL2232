/*
 * MasterNI.cpp
 *
 *  Created on: 2019年9月9日
 *      Author: wr
 */

#include <AXI4/MasterNI.hpp>

MasterNI::MasterNI(int t_id, int t_slave_num, int t_master_num, TNetwork *t_network, VCNetwork *vc_network,
                   int t_router_num_x, int *t_NI_num, std::vector<BasicNI *> t_main_basicNI_list, float inj) : BasicNI(
        t_id, t_slave_num, t_master_num, t_network, vc_network, t_router_num_x, t_NI_num) {
    //basicNI是masterNI的 父类
    //创建这四种类的实例
    converterMasterInMasterNI = new Converter_Master(id, inj);
    converter_slave = new Converter(id, inj);
    masterPort = new MasterPort(slave_num, this, inj);
    slavePortInMasterNI = new SlavePortInMasterNI(master_num, this); //这里的basic NI 指的是master NI，因此 后面的dequeue是使用master的
    //传递参数
    basicNI_list_inVCMasterNI = t_main_basicNI_list;
#ifdef ofstreamSW_MasterPortMessageOut
    MasterPortMessageOut.open(
            "../RecordFiles/NoC_Status/AXI4NI_Status/MasterPort/" + std::to_string(id) + "MasterPortMessageOut.txt",
            ios::app);// cmake in sub direc. so add ../ ios::app to add content without clearing
#endif
    //
}

// send to converter
void MasterNI::enqueue(Message *message) { //进队列函数，将一个指向 message 对象的 指针 存入
    converter_slave->message_in_inConverterSlave.push_back(message);//模仿slave 从 网络中接受到 req msg
}

Message *MasterNI::dequeueMessage() {//模拟slave 输出 resp msg
    if (converter_slave->message_out_inConverterSlave.size() > 0) {
        Message *message = converter_slave->message_out_inConverterSlave.front();
        converter_slave->message_out_inConverterSlave.pop_front();
        return message;
    }
    return (Message *) NULL;
}

void MasterNI::runOneStep() {//其应该是用于完整执行一遍信号传输
#ifdef MasterNIRun_writeFile
   writeFile_MasterPortMessageOut();//write converter masterinto it
#endif
    //updateRLActionInMasterPort();//update action in main is better: control the seq. in one cycle.
    // cout<<id<<" cycles " <<cycles<< "masterNI action"<<masterPort->RLaction_cycles_inMasterPort<<endl;
//    if (int(cycles-1) % int(paramTotalSimCycle / parameter_NI_statePeriodNum) == 0 && cycles > 1 && id ==34) // random use a id like 34
//    {
//        cout<<cycles<<" cycles "<<int(cycles) / int(paramTotalSimCycle / parameter_NI_statePeriodNum) <<" period" <<"RLaction_cycles_inMasterPort "<<masterPort->RLaction_cycles_inMasterPort<<endl;
//    }//comare with the result in main. the action should be the same


    // master d1 ->d3->a2->b1->b2 delete received response packet/message/signal
    // master a1->c1->c2 then VCNI inject the generator's packet
    //将generator 生成的 req sig 转化为 message 存入 mesage_out队列 ，然后删除 master接收到的 resp sig
    // generator new signal in axisignal_in_R/W BY masterNI->converter_master->AXI4_in_read
    converterMasterInMasterNI->runOneStep(); // a.1.  AXI4Signal* AXI4_signal from AXI4_in_read/write to converter_master->message_out  Message* message. //a2. next cycle of d.3.  then get AXI4_signal from converter_master->rmessage_in (not message_out in a.1) to AXI4_out_read/write
    // receive from network and inject to assumed axislave devices.
    //del//  b.1 AXI4Signal* response = converter_master->rAXI4_out_read.front() from a.2 ;// b.2 delete response
#ifdef MasterNI_to_buffer_fullNI_NoC
    masterPort->buffer_fullNI_record(); // c.1//不执行
#else
//用于将masterNI 中 msg out队列中 产生的请求信息 经过一些列的操作，转换成VC网络中 某个 slave NI对应的 packet 并储存下来
    masterPort->sendReqMessageToPacket_toVCNIPacketBuffer(); // c.1. message_out.popfront(message) from a.1 .
    // c.2.masterNI->VC_network->NI_list[masterNI->id]->packetBuffer_list[packet->vnet]->enqueue(packet); //VC NI: In VC NI, check the packetBuffer_list[VN] and Flitize  the packet to flits
    //  receive message from NoC response buffer
    // if NI received the response flits, compose them and put the packet into packet_buffer_out[1] then need to delete it
#endif
    //这个是基于发送的packet ，从某个ni中取得对应的 响应message ，先存储到respose list ，在存储到 masterNI的对队列messageReceivedResp_in_toBeDeleted 中等待被删除
    masterPort->refer(); // d1. message (actually packet) = masterNI->VC_network->NI_list[masterNI->id]->packet_buffer_out[1].front(); //d2.messageBuffer.push_back(message); //d3    masterNI->converter_master->message_in.push_back(messageBuffer.front&pop);

    // sc1 packet_buffer_out[0]-> sc3 message_in
    //-> sa 2 messag_in ->AXI4—OUT—READ
    //  ->SB1  AXI4—OUT—READ->AXI4_in_read   SB2 message_out
    // SD deque the messag_out

    // slave receive signal from assumed aximaster device and inject them into network
    //将两个resp sig队列中的信号转换为 msg 存储到message_out_inConverterSlave，可能会进一步的转换到packet 级，这里的resp sig 可能是
    //是之前存储的
    // 从队列message_in_inConverterSlave取msg，这个队列可能存储的是网络互联中得到的 ，并根据 msg的 type 存储到  AXI4_out_read和 write
    converter_slave->runOneStep(); // Sa.1.  AXI4Signal* AXI4_signal from AXI4_in_read/write to converter_slave->rmessage_out  Message* message. //a2. get AXI4_signal from converter_slave->message_in (not message_out in a.1) to AXI4_out_read/wri
    // SB is not MB. In MB, delete it, in SB, put the resonse signal into AXI4_in_read
    //接受请求信号并生成响应信号，从刚刚的AXI4_out_read和 write队列中获取 请求信号并等到resp sig 存储到将两个resp sig队列RespSigInConverterSlave_initializedRead
    //RespSigInConverterSlave_initializedwrite
    converter_slave->MasterNIresponseToOtherMasterNI(); // SB.1 AXI4_out_read pop_front // get response message AXI4_in_read.push_back(response);
    //basicNI_packetWaitNum = converter_slave->AXI4_out_read.size()+converter_slave->AXI4_out_write.size();// wait resp signal size ? or wait resp and reqA
    //basicNI_packetWaitNum =
    // after sb,2 then go to sa.1 to message_out forMasterNI.CPP to be dequeue in SD.1

    //从vc 中获得 并存到到 message_in_inConverterSlave 中了
    slavePortInMasterNI->record_refer(); // SC.1.message from Netowork received request = basicNI->VC_network->NI_list[basicNI->id]->packet_buffer_out[0].front();
    // SC.2. basicNI->enqueue(request_list[NI_id][id][slave_list[NI_id][id]]);
    // SC.3. use master->enqueue put into converter_slave->message_in
//#ifdef MasterNI_to_buffer_fullNI_NoC
///  slavePort->to_buffer_fullNI_NoC();
//#else
//message_out_inConverterSlave 从这个里面取并转换成packet 存入到 某VC buffer并转化成 flit
    slavePortInMasterNI->to_NoC(); // sd.1  .slaveNIdeque message=message_response.front();2. Packet *packet = new Packet(message, basicNI->router_num_x, basicNI->NI_num, global_Packet_ID, slaveport_Pakcet_ID, -199, -99);
    // 3. packetBuffer_list[VN] enqueue this packet // 4. In VC NI, check the packetBuffer_list[VN] and Flitize  the packet to flits
//#endif
}

//below run in every masterNI->run one step
void
MasterNI::updateRLActionInMasterPort() {// update masterPort->RLAction to be actionPicked_inBasicNI from basicNI. //And actionPicked_inBasicNI comes from main.
    masterPort->RLaction_cycles_inMasterPort = BasicNI::actionPicked_inBasicNI;
}

void MasterNI::writeFile_MasterPortMessageOut() {
#ifdef ofstreamSW_MasterPortMessageOut
    MasterPortMessageOut << id << " id " << cycles << " cycles "
                         << converterMasterInMasterNI->message_out.size() << endl;
#endif
}

MasterNI::~MasterNI() {
#ifdef ofstreamSW_MasterPortMessageOut
    MasterPortMessageOut.close();
#endif
    delete converterMasterInMasterNI;
    delete converter_slave;
    delete masterPort;
    delete slavePortInMasterNI;
}
