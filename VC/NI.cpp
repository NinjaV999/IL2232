/*
 * NI.cpp
 *
 *  Created on: 2019年8月23日
 *      Author: wr
 */

#include "VC/NI.hpp"
#include <cstdlib>
#include "parameters.hpp"

int NI::total_packet_id = 0; // added
// orig
int NI::countSendReq = 0; //静态变量
int NI::countReceivedReq = 0;
int NI::countSendResp = 0;
int NI::countReceivedResp = 0;
int NI::count_input = 0;

int NI::LCS_delay_distribution[DISTRIBUTION_NUM] =
        {0};
int NI::URS_delay_distribution[DISTRIBUTION_NUM] =
        {0};

int NI::worst_LCS = 0;
int NI::worst_URS = 0;

int NI::URS_latency_single_dis[100] =
        {0};
int NI::LCS_latency_single_dis[100] =
        {0};
int NI::converse_latency_single_dis[100] =
        {0};

int NI::URS_latency_single_count = 0;
int NI::URS_latency_single_total = 0;
int NI::URS_latency_single_worst = 0;

int NI::LCS_latency_single_count = 0;
int NI::LCS_latency_single_total = 0;
int NI::LCS_latency_single_worst = 0;

int NI::converse_latency_single_count = 0;
int NI::converse_latency_single_total = 0;
int NI::converse_latency_single_worst = 0;

NI::NI(int t_id, VCNetwork *t_vcNetwork, int t_vn_num, int t_vc_per_vn,
       int t_vc_priority_per_vn, int t_in_depth_fromVCNetwork) { //构造函数
    id = t_id;// ni seq 是 生成 ni的 总个数
    vcNetwork = t_vcNetwork;//属于哪个个 网络
    vn_num = t_vn_num;//
    vc_per_vn = t_vc_per_vn;
    vc_priority_per_vn = t_vc_priority_per_vn;

    //jingwei li adder
    inj_packet=0;
    tokens=4.0;
    capacity=4.0;
    tokens_gen_rate=0.0;
    leaky_bucket_packet_buffer=new PacketBuffer(this, 0);
    //jingwei li end
    // output;
    packetBufferList_xVNToFlitize.reserve(vn_num); //预留vn 数目大小 ，
    for (int i = 0; i < vn_num; i++) {//创建 vn数目大小的packet buffer
        PacketBuffer *t_packetBuffer = new PacketBuffer(this, i); //属于哪个 ni 的 第几个 packetbuffer
        packetBufferList_xVNToFlitize.push_back(t_packetBuffer);
    }
    VCNIFlitBuffer_list.reserve(vn_num * (vc_per_vn + vc_priority_per_vn)); // 创建 这么大小的 flit buffer ，每一个
    //packet buffer 对应 (vc_per_vn + vc_priority_per_vn) 个 flit buffer

    for (int i = 0; i < vn_num * vc_per_vn; i++) {
        // FlitBuffer * t_flitBuffer = new FlitBuffer(i%vc_per_vn, i/vn_num, i, INFINITE);
        FlitBuffer *t_flitBuffer = new FlitBuffer(i % vc_per_vn, i / vn_num, i,
                                                  INFINITE); // from ni to noc, inf. buffer
                                                  //flit buffer的 初始化 为 哪个 vn 的 第一几个 vc 以及该 ni的 第几个buffer
        VCNIFlitBuffer_list.push_back(t_flitBuffer);
        out_vc.push_back(-1); // 对应余每一个 flit buffer 每创建 一个 其 对应的 out_vc[i] 被初始化为-1
        state.push_back(0);//state 被 初始化 为 idle
    }

    for (int i = 0; i < vn_num * vc_priority_per_vn; i++) { //这是 对于 lcs的 ，可能没用
        // FlitBuffer * t_flitBuffer = new FlitBuffer(i%vc_priority_per_vn+vn_num, i/vn_num, i, INFINITE);
        FlitBuffer *t_flitBuffer = new FlitBuffer(
                i % vc_priority_per_vn + vn_num, i / vn_num, i, INFINITE);
        VCNIFlitBuffer_list.push_back(t_flitBuffer);
        out_vc.push_back(-1);
        state.push_back(0);
    }

    rr_buffer = 0;
    NI_in_depth = t_in_depth_fromVCNetwork;

    // output;
    ROutPort *vc_rOutPort = new ROutPort(0, 1, 1, 0, INFINITE);
    //在ni中创建一个 outport ，其 flit buffer 长度 为 10000
    NI_out_port = vc_rOutPort;

    // input;
    /*
     *  // connect NI's inport with router's outport
     vcRouter->out_port_list_inRouter[4+j]->out_link = ni->NI_in_port->in_link;
     vcRouter->out_port_list_inRouter[4+j]->out_link->rOutPort = vcRouter->out_port_list_inRouter[4+j];
     */
    Link *link = new Link((RInPort *) NULL);
    extern int global_rInPort_ID;
    // Infinite: t_depth in RInPort, depth in port, then depth in flitbuffer per VC
    // RInPort * vcInPort = new RInPort(0, vn_num, vc_per_vn, vc_priority_per_vn, INFINITE, this, link,global_rInPort_ID);
    RInPort *vcInPort = new RInPort(0, vn_num, vc_per_vn, vc_priority_per_vn,
                                    parameter_NI_myFlitBufferSize,
                                    this, link, global_rInPort_ID);
    global_rInPort_ID++;
    link->rInPort = vcInPort; // cannot be deleted 20230111
    NI_in_port = vcInPort; //ni 中创建一个 输入端口 ，有vn_num*vc_per_vn, filt buffer长度为 9

    // yzadd
    packetBufferOutReq_credit = 0;     // 20230112
    packetBufferOutResp_credit = 0;     // 20230112
    totaldelay_signal_trans_lcs = 0; // 20230119
    totaldelay_signal_trans_urs = 0; // 20230119
    VCNITotalRespReceCount_URS = 0;//20230213

    memset(VCNI_transSigTwoWayDelay_perIntervalList, 0,
           sizeof(VCNI_transSigTwoWayDelay_perIntervalList));//ini. the list
    memset(VCNI_respPacketReceivedCount_perIntervalList, 0,
           sizeof(VCNI_respPacketReceivedCount_perIntervalList));//ini. the list
    VCNI_MMP *tempVCNI_MMP = new VCNI_MMP();
    vcNI_MMP = tempVCNI_MMP;
    reqWaitFlitizeTime = 0;
    NI_respURSPacketCount = 0;
    NIRLHardSwitch = 1;
    adminInterval = 1;
    //

    count_vc = 0;
    count_switch = 0;

    starvation = 0;
    rr_priority_record = 0;

    total_delay = 0;
    total_num = 0;
    total_delay_URS = 0;
    total_num_URS = 0;
}
//Jingwei add

void NI:: token_injection(){

    tokens=tokens+tokens_gen_rate;
    return ;

}

//jingwei li add end
//add end
bool NI::flitize(Packet *packet, int vn) {
    int base;
#ifdef routerSW_priority_responsePacket
    if (packet->signal->QoS == 3)
    {
        base = vn_num * vc_per_vn + vn * vc_priority_per_vn;
        for (int i = base; i < base + vc_priority_per_vn; i++)
        {
            if (state[i] == 0)
            { // idle
                int length = (packet->length - 1) / FLIT_LENGTH + 1;
                packet->send_out_time_inPacket = cycles; // NI convert packet to flit at this cycle
                if (length == 1)
                {
                    Flit *flit = new Flit(0, 10, vn, i, packet, cycles); // changed for head also tail
                    flit->flit_iniCycle = cycles;
                    VCNIFlitBuffer_list[i]->enqueue(flit);
                }
                else
                {
                    for (int id = 0; id < length; id++)
                    {
                        if (id == 0)
                        {
                            Flit *flit = new Flit(id, 0, vn, i, packet, cycles); // changed for head and tail
                            VCNIFlitBuffer_list[i]->enqueue(flit);
                            // add*****************************************************************
                            if (flit->packet->packet_ID == parameter_UniqueID_trackThisSignalLife && flit->type == 0)
                            {
                                cout << "NI::flitize  ni " << NI::id << "cycle " << cycles << endl;
                            }
                            // add end*****************************************************************
                        }
                        else if (id == length - 1)
                        {
                            Flit *flit = new Flit(id, 1, vn, i, packet, cycles); // body
                            VCNIFlitBuffer_list[i]->enqueue(flit);
                        }
                        else
                        {
                            Flit *flit = new Flit(id, 2, vn, i, packet, cycles);
                            VCNIFlitBuffer_list[i]->enqueue(flit);
                        }
                    }
                }
                state[i] = 1; // wait for VC allocation

                return true;
            }
        }
        return false; // this qos==3 packet cannot find idle vc to
    }
    // priority_globalPakcetID_vcList
#endif
    // for priority packet with individual VCs
    if (packet->signal->QoS == 3) {
        base = vn_num * vc_per_vn + vn * vc_priority_per_vn;
        for (int i = base; i < base + vc_priority_per_vn; i++) {
            //	  cout<< "NI115 line  " <<i<<endl;// indivadual here i= 4-5
            if (state[i] == 0) {                                                         // idle
                int length = (packet->length - 1) / FLIT_LENGTH +
                             1; // round up   // parameter #define FLIT_LENGTH 16 // byte //16*8=128bit  //语文定义
                packet->send_out_time_inPacket = cycles;                         // NI convert packet to flit at current cycle  //(int t_id, int t_type, int t_vnet, int t_vc, Packet* t_packet, float t_cycles)

                if (length == 1) {
                    Flit *flit = new Flit(0, 10, vn, i, packet, cycles); // changed for head also tail
                    flit->flit_iniCycle = cycles;                         // added NI flit_iniCycle
                    VCNIFlitBuffer_list[i]->enqueue(flit);
                } else {
                    for (int id = 0; id < length; id++) {
                        if (id == 0) {
                            Flit *flit = new Flit(id, 0, vn, i, packet, cycles); // changed for head and tail
                            flit->flit_iniCycle = cycles;                         // added NI flit_iniCycle
                            VCNIFlitBuffer_list[i]->enqueue(flit);
                        } else if (id == length - 1) {
                            Flit *flit = new Flit(id, 1, vn, i, packet, cycles); // body
                            flit->flit_iniCycle = cycles;                         // added NI flit_iniCycle
                            VCNIFlitBuffer_list[i]->enqueue(flit);
                        } else {
                            Flit *flit = new Flit(id, 2, vn, i, packet, cycles);
                            flit->flit_iniCycle = cycles; // added NI flit_iniCycle
                            VCNIFlitBuffer_list[i]->enqueue(
                                    flit);// at most 9 flits are injected into this flitbuffer in one cycle
                        }
                    }
                } // line 116 if ends

                state[i] = 1; // wait for VC allocation
                return true;
            }
        }
        reqWaitFlitizeTime++;
        cout << "ni  flitize false " << reqWaitFlitizeTime << endl;
        return false;
    }

        // for normal packet or priority packet with shared VCs
    else {// 对于 urs
#ifdef SHARED_PRI
        if (packet->signal->QoS == 1)
        { // for LCS to use all the VCs
#endif
        base = vn * vc_per_vn; //vn 是输入参数

        for (int i = base; i < base + vc_per_vn; i++) { //first check req vc, then resp vc
            //    cout<< "NI154 line  " <<id <<"  "<<i<<"  "<<packet->type<<endl;// indivadual here i= 0-3
            //可以这样理解 要不是 处理 vn =0 的 两个 vc 要不是 处理 vn=1的2 两个vc
            if (state[i] == 0) { // idle //VCNIFlitBuffer_list state记录的是这里面的 flit buffer 状态 或者说是 Vc状态
                packet->send_out_time_inPacket = cycles; //packet的 发出时间 等于 当前全局时间 ，先判断packet 是否送达
                int length = (packet->length - 1) / FLIT_LENGTH + 1; // round up，记录 flit个数
                if (length == 1) {// 如果这有一个 flit则生成 一个 head and flit
                    Flit *flit = new Flit(0, 10, vn, i, packet, cycles);
                    //flit的 vc 是 绝对 vc 并不是 相对于 vn的 vc
                    flit->flit_iniCycle = cycles; // added NI flit_iniCycle ,flit的 初始时间为 当前全局时间
                    VCNIFlitBuffer_list[i]->enqueue(flit);// 根据flit的vc 将flit存入对应的 flit buffer
                    if (flit->packet->signal->QoS == 1) {
                        priority_vc.push_back(i);
                        priority_switch.push_back(i);
                    }
                } else { // when length !=1 当不为1 时
                    for (int id = 0; id < length; id++) { // 有 id 来记录这是packet1里的哪个flit，这里遍历了所有flit
                        if (id == 0) {//头节点，flit的 vc是 绝对值
                            Flit *flit = new Flit(id, 0, vn, i, packet, cycles);
                            flit->flit_iniCycle = cycles; // added NI flit_iniCycle
                            VCNIFlitBuffer_list[i]->enqueue(flit);
                            if (flit->packet->signal->QoS == 1) {
                                priority_vc.push_back(i);
                                priority_switch.push_back(i);
                            }
                        } else if (id == length - 1) {
                            Flit *flit = new Flit(id, 1, vn, i, packet, cycles);
                            flit->flit_iniCycle = cycles; // added NI flit_iniCycle
                            VCNIFlitBuffer_list[i]->enqueue(flit);
                        } else {
                            Flit *flit = new Flit(id, 2, vn, i, packet, cycles);
                            flit->flit_iniCycle = cycles; // added NI flit_iniCycle
                            VCNIFlitBuffer_list[i]->enqueue(flit);
                        }
                    }
                }
                state[i] = 1; // wait for VC allocation ，ni里的flit buffer 只用三种，与 r in port 不同
                return true;
            }
        } //vn 是由 packet 的 类型 决定 ，如果 这个 vn中的 vc 有一个 空闲的化则会把packet 存入 这 vc，存入该packet 的 所有 flit


#ifdef SHARED_PRI
        }
        else
        { // for URS to use only vc_per_vn without priority (vc_priority_per_vn)
            base = vn * vc_per_vn;
            for (int i = base; i < base + vc_per_vn - vc_priority_per_vn; i++)
            {
                if (state[i] == 0)
                { // idle
                    packet->send_out_time_inPacket = cycles;
                    int length = (packet->length - 1) / FLIT_LENGTH + 1; // round up
                    if (length == 1)
                    {
                        Flit *flit = new Flit(0, 10, vn, i, packet, cycles);
                        buffer_list[i]->enqueue(flit);
                    }
                    else
                    {
                        for (int id = 0; id < length; id++)
                        {
                            if (id == 0)
                            {
                                Flit *flit = new Flit(id, 0, vn, i, packet, cycles);
                                buffer_list[i]->enqueue(flit);
                            }
                            else if (id == length - 1)
                            {
                                Flit *flit = new Flit(id, 1, vn, i, packet, cycles);
                                buffer_list[i]->enqueue(flit);
                            }
                            else
                            {
                                Flit *flit = new Flit(id, 2, vn, i, packet, cycles);
                                buffer_list[i]->enqueue(flit);
                            }
                        }
                    }
                    state[i] = 1; // wait for VC allocation
                    return true;
                }
            }
        }
#endif
        reqWaitFlitizeTime++; //若 该 vn中 没有 一个 vc空闲则 等待时间增加 并返回 falls
        //cout<<"ni  flitize false "<<reqWaitFlitizeTime<<endl;
        return false;
    }
}

/* @brief just cout pakct of RW or RW_resp number
 *
 */
#ifdef VCNIDequeueToSelfRouter_packetAdmissionControl

void NI::packetDequeue_RLAdmissionControl() {

    //if (id == 31)
    { //packetBufferList_xVNToFlitize 存储了 vn个 packet buffer
        if (packetBufferList_xVNToFlitize[1]->packet_queue.size() > 1 && int(cycles) % 10 == 0 &
                                                                         false) { //resp packet wait to be injected?//print SW
//            cout << " current sig ID " << packetBufferList_xVNToFlitize[1]->packet_queue.front()->signal->signal_id
//                 << " ";
            cout << cycles << "id " << id << " state_i " << state[0] << " " << state[1] << " " << state[2] << " "
                 << state[3] << " ";
            cout << "slaveNI wait Packet " << packetBufferList_xVNToFlitize[1]->packet_queue.size();
            if (packetBufferList_xVNToFlitize[1]->packet_queue.size() > 1) {
                NI31PacketWaitTimes = NI31PacketWaitTimes + 1;
            }
            cout << " " << NI31PacketWaitTimes << endl;
        }
    }
    if (vcNI_MMP->VCNIMMPRequest(vcNI_MMP->actionAlpha2)) {}

    if (packetBufferList_xVNToFlitize[0]->packet_num > 10) {
//        cout <<id<< " "<<cycles<<" packetBufferList_xVNToFlitize[0]->packet_num  " << packetBufferList_xVNToFlitize[0]->packet_num<<"  "<<packetBufferList_xVNToFlitize[0]->read()->signal->destination
//             << endl;
    } //52 && i != 115 && i != 122 && i != 129 && i != 136
    int admissionSW = 0;
    if (packetBufferList_xVNToFlitize[0]->packet_num != 0
         && ((id == 17) || (id == 24) || ((id == 101)) || (id == 108)
      //  && ((id == 0) || (id == 7) || ((id == 84)) || (id == 91)
        )
//    &&(packetBufferList_xVNToFlitize[0]->read()->signal->destination == 31 ||
//         packetBufferList_xVNToFlitize[0]->read()->signal->destination == 45 ||
//         packetBufferList_xVNToFlitize[0]->read()->signal->destination == 52 ||
//         packetBufferList_xVNToFlitize[0]->read()->signal->destination == 115 ||
//         packetBufferList_xVNToFlitize[0]->read()->signal->destination == 122 ||
//         packetBufferList_xVNToFlitize[0]->read()->signal->destination == 129 ||
//         packetBufferList_xVNToFlitize[0]->read()->signal->destination == 136)
            ) {
        //cout<<" destination"<< packetBufferList_xVNToFlitize[0]->read()->signal->destination<< endl;
        admissionSW = 0;// if selected nodes, then admission control
    } else {
        admissionSW = 1; //if not selected nodes, then just go
    }
    //   if (int(cycles + id) % global_adminInterval == 0 || admissionSW == 1)
   // if (admissionSW == 1 || int(cycles) % global_adminInterval == 0)
        // if (  (rand() % global_adminInterval < 10 )|| admissionSW == 1)
   // {// if destination mem, then
        //cout<<"id should no 17 "<<id<< endl;

        //  cout<<id<<" vcNI_MMP->actionAlpha2 "<<vcNI_MMP->actionAlpha2 <<" cycles "<<cycles<< endl;
        // }

      //  if (NIRLHardSwitch == 1) { //如果准入开关打开
             // if (vcNI_MMP->VCNIMMPRequest(vcNI_MMP->actionAlpha2))
             // for (int vn = 0; vn < vn_num; vn++) {
             //if (id != 31 && id != 38 && id != 45 && id != 52 && id != 115 && id != 122 && id != 129 &&
             // id != 136) {
             for (int vn = 0; vn < 1; vn++) {  //只对vn =0
                 while (packetBufferList_xVNToFlitize[vn]->packet_num != 0 && tokens >= 1) { //确保tokens 始终大于等于0
                 Packet *temp_packet=packetBufferList_xVNToFlitize[vn]->read();
                 leaky_bucket_packet_buffer->enqueue(temp_packet);
                 packetBufferList_xVNToFlitize[vn]->dequeue();
                 tokens --;
                 }
                  //packetBufferList_xVNToFlitize[vn]->dequeue();}
                 //tokens--;

                 //先把能输出的packet都输出出来
                 //packetBufferList_xVNToFlitize[vn].pop_front();
                 while (leaky_bucket_packet_buffer->packet_num != 0 &&
                         leaky_bucket_packet_buffer->read()->out_cycle_inMessage < cycles) {
                     // 判断 packet的 buffer中是否有 packet 且 发出周期< cycles
                     Packet *packet =  leaky_bucket_packet_buffer->read();// 读出 packet
                     assert(packet->vnet == vn); //判断 vn是不是 packet 的 vnet， 按说 应该是的
                     if (flitize(packet, vn)) { //判断能不能将 packet 准化成 vn 并存入到vn空的vc中，如果能的花
                         leaky_bucket_packet_buffer->dequeue();//popfront/delete packet，则从 packet buffer 中删除这个
                         //packetbuffer的packet dequeue了则表示其向外注入了一个packet，因此每次执行这个指令我进行一次统计就可以得到

                         inj_packet = inj_packet+1;

                         if (globalMaxSignalInNOC < packet->signal->signal_id) {
                             globalMaxSignalInNOC = packet->signal->signal_id;
                         }
                         if (packet->type == 0) {  // read or write orig
                             countSendReq++;
                             countYZSendReq++;
                             if (printfSW_VCNI_Type0NISend == 1) {
                                 cout << "Packet type 0 NI id " << id << " NI send "
                                      << countSendReq << endl;
                             }
                         }
                         if (packet->type == 1) { // read or write resp // VN 0 is req net
                             countSendResp++;
                             countYZSendResp++;
                             cout << "this should not have resp if vn=0" << endl;
                         }
                     } else { //这也是一个对packet的循环阿 ，若ni out port连接 router inport 没有空余vc 则 会 跳出while 循环 对另一个packert buffer 操作      名
                         break;
                     }
                 }
             }
       //  }          //}
             //}
        // }
   // }

    for (int vn = 1; vn < vn_num; vn++) {//0 req vn, 1: resp vn ，这是一个同样的过程对 Vn=1，resp vn 进行了以便操作，似乎只对 req信号进行了调控
        while (packetBufferList_xVNToFlitize[vn]->packet_num != 0 &&
               packetBufferList_xVNToFlitize[vn]->read()->out_cycle_inMessage < cycles) {

            Packet *packet = packetBufferList_xVNToFlitize[vn]->read();
            assert(vn == packet->vnet);

            if (flitize(packet, vn)) { //如果 packet bufferl对应的 flitbuffer 有空位置 则 ，将packet 转化成 flit存入
                packetBufferList_xVNToFlitize[vn]->dequeue();//popfront/delete packet
                if (packet->type == 0) { // read or write orig
                    countSendReq++;
                    countYZSendReq++;
                    NIInjectReqPacketNum++;
                    if (printfSW_VCNI_Type0NISend == 1) {
                        cout << "Packet type 0 NI id " << id << " NI send "
                             << countSendReq << endl;
                    }
                }
                if (packet->type == 1) { // read or write resp
                    countSendResp++; //每次在 ni中完成一个 packet flit的 转换 根据其实req 还是resp 进行 计数增加
                    countYZSendResp++;
                    // cout<< "I am resp net"<<endl;
                }
            } else {
                break;
            }
        }
    }
}

#endif

#ifndef VCNIDequeueToSelfRouter_packetAdmissionControl
void NI::packetDequeue() {
    if (id == 31 & false) {
        if (packetBufferList_xVNToFlitize[1]->packet_num != 0) {
            cout << " current sig ID " << packetBufferList_xVNToFlitize[1]->packet_queue.front()->signal->signal_id
                 << " ";
        }
        cout << cycles << " state_i " << state[0] << " " << state[1] << " " << state[2] << " " << state[3] << " ";
        cout << "slaveNI wait Packet " << packetBufferList_xVNToFlitize[1]->packet_queue.size() << endl;
    }
    for (int vn = 0; vn < vn_num; vn++) {

        while (packetBufferList_xVNToFlitize[vn]->packet_num != 0 &&
               packetBufferList_xVNToFlitize[vn]->read()->out_cycle_inMessage < cycles) {

            Packet *packet = packetBufferList_xVNToFlitize[vn]->read();
            assert(vn == packet->vnet);

            if (flitize(packet, vn)) {
                packetBufferList_xVNToFlitize[vn]->dequeue();//popfront/delete packet
                if (packet->type == 0) { // read or write orig
                    countSendReq++;
                    countYZSendReq++;
                    if (printfSW_VCNI_Type0NISend == 1) {
                        cout << "Packet type 0 NI id " << id << " NI send "
                             << countSendReq << endl;
                    }
                }
                if (packet->type == 1) { // read or write resp
                    countSendResp++;
                    countYZSendResp++;
                    if (printfSW_VCNI_Type1NISend == 1) {
                        cout << "Packet type 1 NI id "
                             << " NI send response"
                             << countSendResp << endl;
                    }
                }
            } else {
                break;
            }
        }
    }
}
#endif

void NI::vcAllocation() {
    // cout << "#######" << endl;
    //  for priority packet (shared VCs) QoS = 1

#ifdef SHARED_VC
    std::vector<int>::iterator iter;
    for (iter = priority_vc.begin(); iter < priority_vc.end();)
    {
        if (count_vc == STARVATION_LIMIT)
            break; // yz break to where?
        int tag = (*iter);
        if (state[tag] == 1)
        {
            int vc = NI_out_port->out_link->rInPort->vc_allocate(buffer_list[tag]->read()); // RInPort::vc_allocate(Flit* t_flit) return vn*vc_per_vn+i(the idle one's id);  //vc id
            if (vc != -1)
            {
                out_vc[tag] = vc;
                state[tag] = 2; // active
                count_vc++;
                iter = priority_vc.erase(iter);
            }
            else
                iter++;
        }
        else
            iter++;
    }
    count_vc = 0;
#endif

    // for priority packet (individual VCs) QoS = 3
    for (int i = vn_num * vc_per_vn;
         i < vn_num * (vc_per_vn + vc_priority_per_vn); i++) {
        int tag = (i + rr_priority_record) % (vn_num * vc_priority_per_vn) + vn_num * vc_per_vn;
        if (state[tag] == 1) {
            int vn_rank = (tag - vn_num * vc_per_vn) / vc_priority_per_vn;
            int vcPriority_IDLEInConnectedInPort = NI_out_port->out_link->rInPort->vc_allocate_priority(
                    vn_rank);
            if (vcPriority_IDLEInConnectedInPort != -1) {
                out_vc[tag] = vcPriority_IDLEInConnectedInPort;
                state[tag] = 2; // active
            }
        }
    }

    // for normal packet QoS = 0
    for (int i = 0; i < vn_num * vc_per_vn; i++) { //对 normal packet 即对 urs packet
        int tag = (i + rr_buffer) % (vn_num * vc_per_vn);
        if (state[tag] == 1) { //当 packet 转换为 flit ，并 将整个packet 以 flit 形式存入到flit buffer 后，flit buffer 变为state=1 ，等待分配

#ifdef SHARED_PRI
            int  vc_IDLEInConnectedInPort = NI_out_port->out_link->rInPort->vc_allocate_normal(buffer_list[tag]->read());
#else//端口是连接好的我们寻找的是该端口对应的vc
            int vc_IDLEInConnectedInPort = NI_out_port->out_link->rInPort->vc_allocate(
                    VCNIFlitBuffer_list[tag]->read()); //找到ni out port,连接的router的 in port 判断其对应的vn中是否有 空的vc buffer
#endif

            if (vc_IDLEInConnectedInPort != -1) { //不为 -1 就是 有 空的vc buffer
                out_vc[tag] = vc_IDLEInConnectedInPort; //存储下该tag 的 ni flit buffer 被分配的 vc
                state[tag] = 2; // active flit buffer切换为数据传送状态
            }
        }
    }

    // in case of no normal packet
#ifdef SHARED_VC
    for (; iter < priority_vc.end();)
    {
        int tag = (*iter);
        if (state[tag] == 1)
        {
            int vc = NI_out_port->out_link->rInPort->vc_allocate(buffer_list[tag]->read());
            if (vc != -1)
            {
                out_vc[tag] = vc;
                state[tag] = 2; // active
                iter = priority_vc.erase(iter);
            }
            else
                iter++;
        }
        else
            iter++;
    }
#endif
}

void NI::switchArbitration() {

    // for priority packet (shared VCs)

#ifdef SHARED_VC
    std::vector<int>::iterator iter;
    for (iter = priority_switch.begin(); iter < priority_switch.end(); iter++)
    {
        if (count_switch == STARVATION_LIMIT)
            break;
        int tag = (*iter);
        if (buffer_list[tag]->cur_flit_num != 0)
        {
            Flit *flit = buffer_list[tag]->read();

            if (state[tag] == 2 && flit->sched_time < cycles && !NI_out_port->out_link->rInPort->buffer_list[out_vc[tag]]->isFull())
            {
                buffer_list[tag]->dequeue();
                flit->vc = out_vc[tag];

                // if(NI_out_port->buffer_list[0]->isFull())
                // cout<< "ni switch" << endl;
                NI_out_port->buffer_list[0]->enqueue(flit);
                NI_out_port->out_link->rInPort->buffer_list[out_vc[tag]]->get_credit();
                flit->sched_time = cycles;
                count_switch++;
                if (flit->type == 1 || flit->type == 10)
                {
                    state[tag] = 0; // idle
                    priority_switch.erase(iter);
                }
                return;
            }
        }
    }
    count_switch = 0;
#endif

    // for priority packet (individual VCs)
    for (int i = vn_num * vc_per_vn; i < vn_num * (vc_per_vn + vc_priority_per_vn); i++) {
        if (starvation == STARVATION_LIMIT) {
            starvation = 0;
            break;
        }
        int tag = (i + rr_priority_record) % (vn_num * vc_priority_per_vn) + vn_num * vc_per_vn;
        if (VCNIFlitBuffer_list[tag]->cur_flit_num != 0) {
            Flit *flit = VCNIFlitBuffer_list[tag]->read();
            if (state[tag] == 2 && flit->sched_time < cycles &&
                !NI_out_port->out_link->rInPort->buffer_list[out_vc[tag]]->isFull()) {
                VCNIFlitBuffer_list[tag]->dequeue();
                flit->vc = out_vc[tag];
                NI_out_port->buffer_list[0]->enqueue(flit);
                NI_out_port->out_link->rInPort->buffer_list[out_vc[tag]]->get_credit();
                flit->sched_time = cycles;
                if (flit->type == 1 || flit->type == 10)
                    state[tag] = 0; // idle
                rr_priority_record = (rr_priority_record + 1) % (vn_num * vc_priority_per_vn);
                starvation++;
                // add*****************************************************************
                if (flit->packet->packet_ID == parameter_UniqueID_trackThisSignalLife & flit->type == 0) {
                    cout << "NI::switch allocation  ni " << NI::id << "cycle "
                         << cycles << endl;
                }
                // add end*****************************************************************
                return;
            }
        }
    }

    // for normal packet 只看对urs的 操作
    for (int i = 0; i < vn_num * vc_per_vn; i++) {
        int tag = (i + rr_buffer) % (vn_num * vc_per_vn);
        if (VCNIFlitBuffer_list[tag]->cur_flit_num != 0) { //flit buffer 不为 0
            Flit *flit = VCNIFlitBuffer_list[tag]->read(); //读出第一个 flit
            if (state[tag] == 2 && flit->sched_time < cycles &&  //当前flit buffer 为 state2 即 已经分配好了 vc且 调度时间下与当前全局时间
                !NI_out_port->out_link->rInPort->buffer_list[out_vc[tag]]->isFull()) {//目标 端口的flit buffer 不满，在传送得一个flit的时候肯定为空 ，因为只有 空 才能被分配 ，后续 就要判断是否是满的
                VCNIFlitBuffer_list[tag]->dequeue(); //删除第一个flit
                flit->vc = out_vc[tag];//将分配的 vc 赋值给flit
                NI_out_port->buffer_list[0]->enqueue(flit);//将该flit存入到ni out port buffer的 缓冲区里
                NI_out_port->out_link->rInPort->buffer_list[out_vc[tag]]->get_credit(); //rinporty 被时有的flit++
                flit->sched_time = cycles;//调度时间为当前时间
                if (flit->type == 1 || flit->type == 10)
                    state[tag] = 0;                         //所是head flit 则置一           // idle
                rr_buffer = (1 + rr_buffer) %
                            (vn_num * vc_per_vn); // rr + 1, the next buffer will have the priority to switch.
                return;
            }
        }
    } //最多只对一个Vc的yig flit 进行操作

    // in case there is no normal packet

#ifdef SHARED_VC
    for (; iter < priority_switch.end(); iter++)
    {
        int tag = (*iter);
        if (buffer_list[tag]->cur_flit_num != 0)
        {
            Flit *flit = buffer_list[tag]->read();
            if (state[tag] == 2 && flit->sched_time < cycles && !NI_out_port->out_link->rInPort->buffer_list[out_vc[tag]]->isFull())
            {
                buffer_list[tag]->dequeue();
                flit->vc = out_vc[tag];

                // if(NI_out_port->buffer_list[0]->isFull())
                // cout<< "ni switch" << endl;
                NI_out_port->buffer_list[0]->enqueue(flit);
                NI_out_port->out_link->rInPort->buffer_list[out_vc[tag]]->get_credit();
                flit->sched_time = cycles;
                if (flit->type == 1 || flit->type == 10)
                {
                    state[tag] = 0; // idle
                    priority_switch.erase(iter);
                }
                return;
            }
        }
    }
#endif

    // for priority packet (individual VCs)
    for (int i = vn_num * vc_per_vn;
         i < vn_num * (vc_per_vn + vc_priority_per_vn); i++) {
        int tag = (i + rr_priority_record) % (vn_num * vc_priority_per_vn) + vn_num * vc_per_vn;
        if (VCNIFlitBuffer_list[tag]->cur_flit_num != 0) {
            Flit *flit = VCNIFlitBuffer_list[tag]->read();
            if (state[tag] == 2 && flit->sched_time < cycles &&
                !NI_out_port->out_link->rInPort->buffer_list[out_vc[tag]]->isFull()) {
                VCNIFlitBuffer_list[tag]->dequeue();
                flit->vc = out_vc[tag];
                NI_out_port->buffer_list[0]->enqueue(flit);
                NI_out_port->out_link->rInPort->buffer_list[out_vc[tag]]->get_credit();
                flit->sched_time = cycles;
                if (flit->type == 1 || flit->type == 10)
                    state[tag] = 0; // idle
                rr_priority_record = (rr_priority_record + 1) % (vn_num * vc_priority_per_vn);
                return;
            }
        }
    }
}

#ifdef  VCNIDequeueToSelfRouter_flitAdmissionControl
void NI::dequeue_VCNIFlitToSelfRouter_withAdmissionControl() {
    if (vcNI_MMP->actionAlpha2 != 100) {
        //cout << "vcNI_MMP->actionAlpha2 " << vcNI_MMP->actionAlpha2 << endl;
    }

    if(vcNI_MMP->VCNIMMPRequest(vcNI_MMP->actionAlpha2) ){
        if (NI_out_port->buffer_list[0]->cur_flit_num != 0 &&
            NI_out_port->buffer_list[0]->read()->sched_time < cycles) {
            Flit *flit = NI_out_port->buffer_list[0]->dequeue();
            NI_out_port->out_link->rInPort->buffer_list[flit->vc]->enqueue(flit);
            flit->sched_time = cycles;
            if (flit->type == 0 || flit->type == 10) {
                VCRouter *vcRouter =
                        dynamic_cast<VCRouter *>(NI_out_port->out_link->rInPort->router_owner);// vcRouter is the NI conenected router
                assert(vcRouter != NULL);

#ifdef SHARED_VC
                if (flit->packet->signal->QoS == 1)
                {
                    NI_out_port->out_link->rInPort->priority_vc.push_back(flit->vc);
                    NI_out_port->out_link->rInPort->priority_switch.push_back(flit->vc);
                }
#endif
                int route_result = vcRouter->getRoute(flit);
                NI_out_port->out_link->rInPort->out_port[flit->vc] = route_result;
                assert(NI_out_port->out_link->rInPort->state[flit->vc] == 1);
                NI_out_port->out_link->rInPort->state[flit->vc] = 2; // wait for vc allocation
            }
        }
    }
}
#endif
#ifndef  VCNIDequeueToSelfRouter_flitAdmissionControl

void NI::dequeue_VCNIFlitToSelfRouter() {// deque NI_out_port->buffer_list[0]
    if (NI_out_port->buffer_list[0]->cur_flit_num != 0 && NI_out_port->buffer_list[0]->read()->sched_time < cycles) {
        //调度时间应该表示的是flit上一次调度完成的时间 ，若 flit上一次调度完成的时间 小于当前时间则表示可以进行此次调度
        Flit *flit = NI_out_port->buffer_list[0]->dequeue();
        //  if(NI_out_port->out_link->rInPort->buffer_list[flit->vc]->isFull()) //这前面 已经判断过了只有不满才能王 out port的flit buffer中存入
        //    cout<< "ni dequeue" << endl;
        NI_out_port->out_link->rInPort->buffer_list[flit->vc]->enqueue(flit);//存入到对应router的 inport中去
        flit->sched_time = cycles;
        if (flit->type == 0 || flit->type == 10) { //若传送的头节点 还要去找 outport的位置
            VCRouter *vcRouter =
                    dynamic_cast<VCRouter *>(NI_out_port->out_link->rInPort->router_owner);// vcRouter is the NI conenected router
            assert(vcRouter != NULL);//创建一个 当前NI 直接连着的router的 vcrouter指针
#ifdef SHARED_VC
            if (flit->packet->signal->QoS == 1)
            {
                NI_out_port->out_link->rInPort->priority_vc.push_back(flit->vc);
                NI_out_port->out_link->rInPort->priority_switch.push_back(flit->vc);
            }
#endif
            int route_result = vcRouter->getRoute(flit); //get route的到的是这个flit 去往下一个router的方向 ，也就是说是
            //预存此时router rinport对应的outport的位置
            NI_out_port->out_link->rInPort->out_port[flit->vc] = route_result;
            assert(NI_out_port->out_link->rInPort->state[flit->vc] == 1);
            NI_out_port->out_link->rInPort->state[flit->vc] = 2; // wait for vc allocation
        }
    }
}

#endif

void NI::inputCheck() {
    for (int i = 0; i < vn_num * (vc_per_vn + vc_priority_per_vn); i++) { //遍历 ni中的每一个vc
        if (NI_in_port->buffer_list[i]->cur_flit_num != 0) { //若vc不为空
            assert(NI_in_port->state[i] == 2); //vc 处于state2 active状态
            Flit *flit = NI_in_port->buffer_list[i]->readLast(); //读出 vc 最后一个flit ，最近存入的flit
            //****************************type 0 head begin
            if (flit->type == //若为头节点
                0) {
                inputAction_headFlitArrive(flit);
            }
            // type 0 head ends ***************************************************************/
            // 1 or 10 type tail begin  ***************************************************************/
            if (flit->sched_time < cycles && (flit->type == 1 || flit->type == //若为tail flit
                                                                 10)) {
                inputAction_TailHeadTailArrive(flit, i);
            }
            // 1 or 10 type tail ends  ***************************************************************/
        } // here if(flit->sched_time < cycles && (flit->type == 1 || flit->type == 10) ends

    } // here if if(NI_in_port->buffer_list[i]->cur_flit_num != 0)
}

void NI::inputAction_headFlitArrive(Flit *t_flit_fromNIInputCheck) {
    // type 0 head begin ***************************************************************/
    Flit *flit = t_flit_fromNIInputCheck;
    if (flit->packet->signal->QoS == 3 || flit->packet->signal->test_tag == 1 ||
        flit->packet->signal->QoS == 1) // type0 LCS(qos=3)
    {  // lcs
        if (LCS_packet_delay[flit->packet->packet_ID][1] == 0) {
            LCS_packet_delay[flit->packet->packet_ID][1] = cycles - flit->packet->send_out_time_inPacket;
        }
        LCS_packet_delay[flit->packet->packet_ID][0] =
                flit->packet->packet_ID;
        LCS_packet_delay[flit->packet->packet_ID][3] =
                (flit->packet->length - 1) / FLIT_LENGTH + 1;
        LCS_packet_delay[flit->packet->packet_ID][4] =
                flit->packet->signal->source_id; // signal source id
        LCS_packet_delay[flit->packet->packet_ID][6] =
                flit->packet->type; // signal dest id
        LCS_packet_delay[flit->packet->packet_ID][7] =
                flit->packet->signal->signal_id;
        LCS_packet_delay[flit->packet->packet_ID][8] =
                flit->flit_iniCycle; // head ini time
        LCS_packet_delay[flit->packet->packet_ID][5] =
                flit->packet->destination[0] * X_NUM + flit->packet->destination[1];
        //flit->packet->sequence_id;
    } else // type0 urs qos=0
    {                                        // cout<<"flit->packet->signal->QoS  "<< flit->packet->signal->QoS<<endl;
        if (flit->packet->signal->QoS == 0) // urs{
            if (URS_packet_delay[flit->packet->packet_ID][1] == 0) {
                URS_packet_delay[flit->packet->packet_ID][1] =
                        cycles - flit->packet->send_out_time_inPacket; // head tail arrive time cost
            }
        URS_packet_delay[flit->packet->packet_ID][0] =
                flit->packet->packet_ID;
        URS_packet_delay[flit->packet->packet_ID][3] =
                (flit->packet->length - 1) / FLIT_LENGTH + 1;
        URS_packet_delay[flit->packet->packet_ID][4] =
                flit->packet->signal->source_id; // signal source id
        URS_packet_delay[flit->packet->packet_ID][6] =
                flit->packet->type; // signal dest id
        URS_packet_delay[flit->packet->packet_ID][7] =
                flit->packet->signal->signal_id;
        URS_packet_delay[flit->packet->packet_ID][8] =
                flit->flit_iniCycle; // head ini time
        URS_packet_delay[flit->packet->packet_ID][5] =
                flit->packet->destination[0] * X_NUM + flit->packet->destination[1];
        //flit->packet->sequence_id;
    }
}
// URS_packet_delay存入的是packet的 属性 ，每个 packet id 为标识， 还有9个属性，分别是 packet id ，头节点到达所用时间， ，
//flit 数，source id ，packet 目的地的绝对位置，type，signal id，
void NI::inputAction_TailHeadTailArrive(Flit *t_flit_fromNIInputCheck, int i_fromNIInputCheck) {
    //flit是 ni port 第 i个 vc最新进入的 flit ，i 为ni  inport,vc ,如果flitbuffer 最后一个 tail flit 则 表明这个flit对应的 packet 刚好传完
    // flit type ==1 or flit type ==10, record delay in line 502 and so on. //flit 0 -> head; 1 -> tail; 2 -> body; 10 -> head_tail;
    Flit *flit = t_flit_fromNIInputCheck;
    int i = i_fromNIInputCheck;
    NI_in_port->state[i] = 0; // idle; 2->0; no need for vc allocation，packet 已经传完不再需要 与ni的outport继续连接
    Packet *packet = flit->packet;//when tail flit comes, firstly do the reaction to the (new) packet, then process this flit itself to avoid when process packet, the flit is  deleted .
    packet->out_cycle_inMessage =
            cycles + LINK_TIME - 1; // 1 cycle delay from NI_network to masterNI/slaveNI  // LINK_TIME is 2
    if (packet->type == 0) { // request
        // if(packetBufferOutReq_credit<=parameter_NI_packetBufferOutReq_depth)
        {
            packetBufferOutReq_credit++; // yz add 20220112
            //cout<<cycles<<" cycles "<<"NI packetBufferOutReq_credit"<<packetBufferOutReq_credit<<endl;//  1 or 2
            packetBufferOut_LeavingVCNI_0.push_back(packet); //若该 packet 为 req 则存入 packet buffer0
            countReceivedReq++;
            //cout<<"countReceivedReq"<<countReceivedReq<<endl;
            if (packet->type == 0)//urs req //==0 for python usage
            {
                recordPacketWhenTailHeadtailArrive_reqURSArrive(flit); //记录某个时间吧
            }
        }
    } else { // response
        // if(packetBufferOutResp_credit<=parameter_NI_packetBufferOutResp_depth)
        {   //若是resp packet则 存入packet out buffer1 里
            packetBufferOutResp_credit++; // yz add 20230112
            //  cout<<cycles<<" cycles "<<"NIpacketBufferOutResp_credit"<<packetBufferOutResp_credit<<endl;// 1or 2
            packetBufferOut_LeavingVCNI_1.push_back(packet);
            countReceivedResp++;
            //cout<<" countReceivedResp "<<countReceivedResp<<endl;
            totaldelay_signal_trans_urs += cycles - packet->signal->signal_trans_createcycles;
            //当前时间-信号创建时间 ，（可以理解为信号创建时间 ，到其对应的resp packet 存入到 目的地ni的 packt buffer 里），将多个
            //resp packet 的传输时间累加，没有分period属于全时间累加
            if (flit->packet->signal->QoS == 0) {
                NI_respURSPacketCount++;
                //每300个cycle记录为 一个period
                int currentPeriod_inNIInputWrite = int(cycles) / int(paramTotalSimCycle /
                                                                     parameter_NI_statePeriodNum);// -499->0:0  0-499: 1  500-999:2
                VCNI_transSigTwoWayDelay_perIntervalList[currentPeriod_inNIInputWrite] =
                        VCNI_transSigTwoWayDelay_perIntervalList[currentPeriod_inNIInputWrite] +
                        cycles - flit->packet->signal->signal_trans_createcycles; //对每个node的 ni 多次执行这个就能统计出 每个period所有axi4信号的双向延迟，这里只统计当前 ni

                VCNI_respPacketReceivedCount_perIntervalList[currentPeriod_inNIInputWrite] =
                        VCNI_respPacketReceivedCount_perIntervalList[currentPeriod_inNIInputWrite] + 1;//计数每个period的 当前ni就受到的resp packet数
                // cout<< " VCNI_transSigTwoWayDelay_perIntervalList "<<currentPeriod_inNIInputWrite <<" cycles " <<cycles  << " "<< VCNI_transSigTwoWayDelay_perIntervalList[currentPeriod_inNIInputWrite]<<endl;
                // cout<<VCNI_respPacketReceivedCount_perIntervalList[currentPeriod_inNIInputWrite]<<endl;
            }


            if (packet->type = 1)//
            {
                //recordPacketWhenTailHeadtailArrive_respURSArrive(flit);
            }
        }
    }
    wbq_RecordInputCheck(packet);

    //add 20230126
    // std::cout<<"Receive packet at cycles: "<< cycles << ", at NI id: " << id << ". Packet length:" <<packet->length <<". slave id:" << packet->slave_id << ". sequence id:" << packet->sequence_id<<std::endl;
    //  ideal_time = (hop+1)*2+flit_num; hop: the routers the packet get through
    while (NI_in_port->buffer_list[i]->cur_flit_num != 0) {    //  在这是一个循环阿 ，若flit buffer 最后一个flit，执行这个函数 会逐个把 vc清空

        Flit *flit_toWrite = NI_in_port->buffer_list[i]->dequeue();//每次都会把 当前vc的 flit取出队列最前端的一个 ，然后 若上面执行的是尾部节点的过程 ，则存入 packetbuffer 后 删除尾部节点
        callFuncCount_writeAllTypeFlitInfoToFil++;
        writeAllTypeFlitInfoToFile(
                flit_toWrite);// thif func may run many times like 1/5/9 times. 1/5/9 is the flits number of  head+body+tail flits.
        //don't need to delete flit_toWrite because this pointer finally will be the same as the  "flit" outside. Don't need to delete twice.
    }
    delete flit;//empty flit buffer in previous " while(!empty){dequeue}"
}

void NI::recordPacketWhenTailHeadtailArrive_reqURSArrive(Flit *t_flit_FromwriteAllTypeFlitInfoToFile) {
    {
        Flit *flit = t_flit_FromwriteAllTypeFlitInfoToFile;
        // URS_packet_delay[flit->packet->packet_ID][10] = flit->packet->signal->signal_trans_createcycles;
        //NI_id 是source ni id
        vcNetwork->NI_list[flit->packet->NI_id]->VCNI_requestPacketReceiByOthersCount_perIntervalList[
                int(cycles) / int(paramTotalSimCycle /
                                  parameter_NI_statePeriodNum)]++;//the senders' list ++
        vcNetwork->NI_list[flit->packet->NI_id]->VCNI_requestPacketReceiByOthersDelay_perIntervalList[
                int(cycles) / int(paramTotalSimCycle /
                                  parameter_NI_statePeriodNum)] =
                vcNetwork->NI_list[flit->packet->NI_id]->VCNI_requestPacketReceiByOthersDelay_perIntervalList[
                        int(cycles) / int(paramTotalSimCycle /
                                          parameter_NI_statePeriodNum)] + cycles -
                flit->packet->out_cycle_inMessage;
        //
    }

}

void NI::recordPacketWhenTailHeadtailArrive_respURSArrive(Flit *t_flit_FromwriteAllTypeFlitInfoToFile) { //没用
    Flit *flit = t_flit_FromwriteAllTypeFlitInfoToFile;

    if (flit->packet->type == 1) {
        int currentPeriod_inNIInputWrite =
                int(cycles) / int(paramTotalSimCycle / parameter_NI_statePeriodNum) +
                1;// -499->0:0  0-499: 1  500-999:2
        VCNI_transSigTwoWayDelay_perIntervalList[currentPeriod_inNIInputWrite] =
                VCNI_transSigTwoWayDelay_perIntervalList[currentPeriod_inNIInputWrite] +
                cycles - flit->packet->signal->signal_trans_createcycles;

        VCNI_respPacketReceivedCount_perIntervalList[currentPeriod_inNIInputWrite] =
                VCNI_respPacketReceivedCount_perIntervalList[currentPeriod_inNIInputWrite] +
                1;// responseReceivedCount per interval//int(cycles)/paramCycleInterval;
        if (VCNI_respPacketReceivedCount_perIntervalList[currentPeriod_inNIInputWrite] > 3) {
//        if (id == 31)
//            cout << " resp_T/HT_flit arrive src " << flit->packet->signal->source_id << " dest " << flit->packet->signal->destination
//                 << " non zero " << cycles << " cycles " << currentPeriod_inNIInputWrite << " period " << id
//                 << " VCni ID " << " Value "
//                 << VCNI_respPacketReceivedCount_perIntervalList[currentPeriod_inNIInputWrite] << endl;
        }
    }
    //if (flit->packet->signal->type == 0)
    {
        VCNITotalRespReceCount_URS++;// the total num of total  received urs resp. signal. should be equal to the sum of all periods “VCNI_respPacketReceivedCount_perIntervalList[currentPeriod_inNIInputWrite]”
    }

}


void NI::writeAllTypeFlitInfoToFile(Flit *t_flit_fromNIInputCheck) {
    Flit *flit = t_flit_fromNIInputCheck;// this flit will go throught the head->body->tail flit in the flit buffer
    Packet *packet = flit->packet;
    //  if flit->type == 1) begin **************************** tail
    if ((flit->type == 1)) { // 1 tail
        Packet *packet = flit->packet;
        if (packet->signal->QoS == 3 || packet->signal->test_tag == 1 ||
            packet->signal->QoS == 1)// qos=1:lcs qos=3: individual lcs.
            // test_tag is used for STD_LATENCY in generator
        {
            // total_delay_tail += cycles - packet->send_out_time_inPacket; //total delay is in NI
            //assert(LCS_packet_delay[flit->packet->packet_ID][2] == 0);
            if (LCS_packet_delay[flit->packet->packet_ID][2] == 0) {
                LCS_packet_delay[flit->packet->packet_ID][2] =
                        cycles - flit->packet->send_out_time_inPacket; // added
                LCS_packet_delay[flit->packet->packet_ID][9] =
                        cycles; // tail arrive time
            } else {
                cout << "whye double write tail time " << endl;
            }
        } else//URS Packet's flit ，若是 urs packet的 tail flit
        {
            if (URS_packet_delay[flit->packet->packet_ID][2] == 0) { //这个flit是 ni inport buffer list 最后的 tail flit ，这个节点取出标志着 该 packet都存入了packet buffer
                URS_packet_delay[flit->packet->packet_ID][2] =
                        cycles - flit->packet->send_out_time_inPacket; // added, 当前时间-packet的 发出时间，到达时间-出发时间
                URS_packet_delay[flit->packet->packet_ID][9] =
                        cycles; // tail arrive time
            }

            //  total_delay_URS_tail += cycles - packet->send_out_time_inPacket;
            //  total_num_URS_tail++;
        }
    }
    // if flit->type == 1) end **************************** tail
    /// added begin for packet delay
    if (flit->type == 0) {//record head flit's info when delete info
        if (flit->packet->signal->QoS == 3 || flit->packet->signal->test_tag == 1 ||
            flit->packet->signal->QoS == 1) { // lcs
            LCS_packet_delay[flit->packet->packet_ID][0] =
                    flit->packet->packet_ID;
            LCS_packet_delay[flit->packet->packet_ID][3] =
                    (flit->packet->length - 1) / FLIT_LENGTH + 1;
            LCS_packet_delay[flit->packet->packet_ID][4] =
                    flit->packet->signal->source_id; // signal source id
            LCS_packet_delay[flit->packet->packet_ID][6] =
                    flit->packet->type; // signal dest id
            LCS_packet_delay[flit->packet->packet_ID][7] =
                    flit->packet->signal->signal_id;
            LCS_packet_delay[flit->packet->packet_ID][8] =
                    flit->flit_iniCycle; // head ini time
            LCS_packet_delay[flit->packet->packet_ID][5] =
                    flit->packet->destination[0] * X_NUM + flit->packet->destination[1];
            //flit->packet->sequence_id;
            if (LCS_packet_delay[flit->packet->packet_ID][1] == 0) {
                LCS_packet_delay[flit->packet->packet_ID][1] =
                        cycles - flit->packet->send_out_time_inPacket;
            }
        } else {                                        // cout<<"flit->packet->signal->QoS  "<< flit->packet->signal->QoS<<endl;
            if (flit->packet->signal->QoS == 0) // urs
            {
                URS_packet_delay[flit->packet->packet_ID][0] =
                        flit->packet->packet_ID;
                URS_packet_delay[flit->packet->packet_ID][3] =
                        (flit->packet->length - 1) / FLIT_LENGTH + 1;
                URS_packet_delay[flit->packet->packet_ID][4] =
                        flit->packet->signal->source_id; // signal source id
                URS_packet_delay[flit->packet->packet_ID][6] =
                        flit->packet->type; // signal dest id
                // URS_packet_delay[flit->packet->packet_ID][5] =flit->packet->destination[0]*X_NUM+flit->packet->destination[1];//packet dest id 0 1 2 //X_NUM=14
                URS_packet_delay[flit->packet->packet_ID][7] =
                        flit->packet->signal->signal_id;
                URS_packet_delay[flit->packet->packet_ID][8] =
                        flit->flit_iniCycle; // head ini time
                URS_packet_delay[flit->packet->packet_ID][5] =
                        flit->packet->destination[0] * X_NUM + flit->packet->destination[1];
                // flit->packet->sequence_id;
                if (URS_packet_delay[flit->packet->packet_ID][1] == 0) {
                    URS_packet_delay[flit->packet->packet_ID][1] =
                            cycles - flit->packet->send_out_time_inPacket; // head tail arrive time
                }
                if (packet->type == 0)//urs req
                {
                    URS_packet_delay[flit->packet->packet_ID][10] = flit->packet->signal->signal_trans_createcycles;
                }
                if (packet->type == 1)//urs resp
                {
                    URS_packet_delay[flit->packet->packet_ID][11] = flit->packet->signal->respSigIniTime;
                    URS_packet_delay[flit->packet->packet_ID][12] = flit->packet->signal->signalGoToMem;//20230426
                }
            }
        }
    }
    //}//0208  while(NI_in_port->buffer_list[i]->cur_flit_num != 0){ ends here

    if ((flit->type == 10)) { // 10 head also tail begin
        Packet *packet = flit->packet;
        if (packet->signal->QoS == 3 || packet->signal->test_tag == 1 || packet->signal->QoS ==
                                                                         1) {                                                            // IF lcs//qos=1:lcs qos=3: individual lcs. test_tag is used for STD_LATENCY in generator
            total_delay_headtail += cycles - packet->send_out_time_inPacket; // total delay is in NI
            total_num_headtail++;
            // if(LCS_packet_delay[flit->packet->packet_ID][2] == 0)
            {
                LCS_packet_delay[flit->packet->packet_ID][0] =
                        flit->packet->packet_ID;
                LCS_packet_delay[flit->packet->packet_ID][1] =
                        cycles - flit->packet->send_out_time_inPacket; // added head time
                LCS_packet_delay[flit->packet->packet_ID][2] =
                        cycles - flit->packet->send_out_time_inPacket; // added tail time
                LCS_packet_delay[flit->packet->packet_ID][3] =
                        (flit->packet->length - 1) / FLIT_LENGTH + 1;;
                LCS_packet_delay[flit->packet->packet_ID][4] =
                        flit->packet->signal->source_id; // signal source id
                LCS_packet_delay[flit->packet->packet_ID][6] =
                        flit->packet->type; // signal dest id
                LCS_packet_delay[flit->packet->packet_ID][5] =
                        flit->packet->destination[0] * X_NUM +
                        flit->packet->destination[1]; // packet dest id 0 1 2 //X_NUM=14
                LCS_packet_delay[flit->packet->packet_ID][7] =
                        flit->packet->signal->signal_id;
                LCS_packet_delay[flit->packet->packet_ID][8] =
                        flit->flit_iniCycle; // head ini time
                LCS_packet_delay[flit->packet->packet_ID][9] = cycles;
            }
        } else { // else URS
            //recordPacketWhenTailHeadtailArrive_respURSArrive(flit);
            total_delay_URS_headtail += cycles - packet->send_out_time_inPacket;
            total_num_URS_headtail++;
            {
                URS_packet_delay[flit->packet->packet_ID][0] =
                        flit->packet->packet_ID;
                URS_packet_delay[flit->packet->packet_ID][1] =
                        cycles - flit->packet->send_out_time_inPacket; // added head time
                URS_packet_delay[flit->packet->packet_ID][2] =
                        cycles - flit->packet->send_out_time_inPacket; // added tail time
                URS_packet_delay[flit->packet->packet_ID][3] =
                        (flit->packet->length - 1) / FLIT_LENGTH + 1;
                URS_packet_delay[flit->packet->packet_ID][4] =
                        flit->packet->signal->source_id; // signal source id
                URS_packet_delay[flit->packet->packet_ID][6] =
                        flit->packet->type; // signal dest id
                URS_packet_delay[flit->packet->packet_ID][5] =
                        flit->packet->destination[0] * X_NUM +
                        flit->packet->destination[1]; // packet dest id 0 1 2 //X_NUM=14
                URS_packet_delay[flit->packet->packet_ID][7] =
                        flit->packet->signal->signal_id;
                URS_packet_delay[flit->packet->packet_ID][8] =
                        flit->flit_iniCycle;                               // head ini time
                URS_packet_delay[flit->packet->packet_ID][9] = cycles; // tail arrive time
                if (packet->type == 0)//urs req
                {
                    URS_packet_delay[flit->packet->packet_ID][10] = flit->packet->signal->signal_trans_createcycles;
                }
                if (packet->type == 1)//urs resp
                {
                    URS_packet_delay[flit->packet->packet_ID][11] = flit->packet->signal->respSigIniTime;
                    URS_packet_delay[flit->packet->packet_ID][12] = flit->packet->signal->signalGoToMem;//20230426
                }
            }
        }
    } ////10 head also tail end
    // add end*****************************************************************
    if (flit->packet->type = 1 & (flit->type == 10 || flit->type == 1)) {//resp + tail/head tail
        // yz add20230119
        // cout<<" flit->packet->signal->signal_trans_createcycles;"<< flit->packet->signal->signal_trans_createcycles<<endl;
        // cout<<" cycles - flit->packet->signal->signal_trans_createcycles;"<< (cycles - flit->packet->signal->signal_trans_createcycles)<<endl;
        // cout<<totaldelay_signal_trans_lcs <<"totaldelay_signal_trans_lcs before "<<endl;
        // totaldelay_signal_trans_lcs += cycles - flit->packet->signal->signal_trans_createcycles;
        // cout<<totaldelay_signal_trans_lcs <<"totaldelay_signal_trans_lcs after "<<endl;
        // cout<<"totaldelay_signal_trans_lcs "<<totaldelay_signal_trans_lcs<<endl;
        //   cout<<" cycles - flit->packet->signal->signal_trans_createcycles;"<< (cycles - flit->packet->signal->signal_trans_createcycles)<<endl;
        assert((cycles - flit->packet->signal->signal_trans_createcycles) > 0); // if exp = 0, means err
        Packet *packet = flit->packet;//avoid give zero to sig ini
        if (packet->signal->QoS == 3 || packet->signal->test_tag == 1 || packet->signal->QoS == 1) {//lcs
            if ((totaldelay_signal_trans_lcs) >= 0 && (cycles - flit->packet->signal->signal_trans_createcycles > 0)) {
                totaldelay_signal_trans_lcs += cycles - flit->packet->signal->signal_trans_createcycles;
            }
        } else {//urs
            if ((totaldelay_signal_trans_urs) >= 0 &&
                (cycles - flit->packet->signal->signal_trans_createcycles > 0)) {

//                cout << "230317 1018linedebug NI id " << id << " sigId NI " << flit->packet->signal->signal_id
//                     << " sig->Type " << flit->packet->signal->type << "  packet->type "
//                     << flit->packet->type << " packet->length " << flit->packet->length << " flit->type " << flit->type
//                     << " sourceID "
//                     << flit->packet->signal->source_id << " destID " << flit->packet->signal->destination << endl;

//                cout << cycles << " debug print totaldelay_signal_trans_urs " << flit->packet->signal->signal_id
//                     << " PacketType " << flit->packet->type<<" vcNetwork->debug_totalRespPacketReceived "<<vcNetwork->debug_totalRespPacketReceived
//                     << " flit type " << flit->type << " req_createcycles "
//                     << packet->signal->signal_trans_createcycles << " two-way_delay "
//                     << (cycles - packet->signal->signal_trans_createcycles)
//                     << endl;//20230220 solved, need to code in slaveNI::response. Otherwise, the resp from SlaveNI's info is not updated

                vcNetwork->debug_totalRespPacketReceived++;
            }
        }

    }
}

void NI::wbq_RecordInputCheck(Packet *t_packet) {
    //WBQ record begins
    //// QoS: 0->URS; 1->LCS (shared VCs with URS packets); 2->GRS; 3->LCS (individual VC(s) only for LCS packets) we do not support "1->shared VCs" mechanism any more
    Packet *packet = t_packet;
    if (packet->signal->QoS == 3 || packet->signal->test_tag == 1 || packet->signal->QoS ==
                                                                     1) {                                                   // qos=1:lcs qos=3: individual lcs. test_tag is used for STD_LATENCY in generator
        total_delay += cycles - packet->send_out_time_inPacket; // total delay is in NI

        total_num++;
    } else {
        total_delay_URS += cycles - packet->send_out_time_inPacket;

        total_num_URS++;//URS packet num  rather than transaction (2-way packets) num
        globalTotalRecNumURS++;
    }
    int delay =
            cycles - packet->send_out_time_inPacket - (abs(packet->NI_id / X_NUM - packet->destination[0]) +
                                                       abs(packet->NI_id % X_NUM - packet->destination[1]) +
                                                       1) * 3 -
            2 - ((packet->length - 1) / FLIT_LENGTH + 1);
    assert(delay >= 0);
#ifdef STD_LATENCY
    if (packet->signal->test_tag == 1)
                {
#else
    if (packet->signal->QoS == 3 || packet->signal->QoS == 1) {
#endif
        // below record latency
        worst_LCS = (worst_LCS < delay) ? delay : worst_LCS;
        if (delay < DISTRIBUTION_NUM - 1)
            LCS_delay_distribution[delay]++;
        else
            LCS_delay_distribution[DISTRIBUTION_NUM - 1]++;
    } else {
        worst_URS = (worst_URS < delay) ? delay : worst_URS;
        if (delay < DISTRIBUTION_NUM - 1)
            URS_delay_distribution[delay]++;
        else
            URS_delay_distribution[DISTRIBUTION_NUM - 1]++;
    }
    if (packet->signal->source_id == 2) {
        int delay = cycles - packet->signal->NI_arrival_time;
        if (packet->signal->QoS == 0) {
            if (delay < 99)
                URS_latency_single_dis[delay]++;
            else
                URS_latency_single_dis[99]++;
            URS_latency_single_count++;
            URS_latency_single_total += delay;
            URS_latency_single_worst =
                    (URS_latency_single_worst < delay) ? delay : URS_latency_single_worst;
        } else if (packet->signal->QoS == 3 && packet->signal->test_tag_qos_convert == 0) {
            if (delay < 99)
                LCS_latency_single_dis[delay]++;
            else
                LCS_latency_single_dis[99]++;
            LCS_latency_single_count++;
            LCS_latency_single_total += delay;
            LCS_latency_single_worst =
                    (LCS_latency_single_worst < delay) ? delay : LCS_latency_single_worst;
        } else {
            if (delay < 99)
                converse_latency_single_dis[delay]++;
            else
                converse_latency_single_dis[99]++;
            converse_latency_single_count++;
            converse_latency_single_total += delay;
            converse_latency_single_worst =
                    (converse_latency_single_worst < delay) ? delay : converse_latency_single_worst;
        }
    }
    ////******************** //WBQ record ends
}

void NI::runOneStep() {
#ifndef VCNIDequeueToSelfRouter_packetAdmissionControl
    packetDequeue();
#endif
#ifdef VCNIDequeueToSelfRouter_packetAdmissionControl
    packetDequeue_RLAdmissionControl();
#endif
    vcAllocation();
    switchArbitration();
#ifndef VCNIDequeueToSelfRouter_flitAdmissionControl
    dequeue_VCNIFlitToSelfRouter();
#endif
#ifdef VCNIDequeueToSelfRouter_flitAdmissionControl
    dequeue_VCNIFlitToSelfRouter_withAdmissionControl();
#endif
    inputCheck();
}

void NI::recordAdmControlWaitDelayTotal() {
    admControl_waitDelayTotal = 0.0;//reset it
    //add it to be more .
}

NI::~NI() {
    //    cout<<"ni "<<id <<" ~ delete "<<reqWaitFlitizeTime<<" countYZSendResp "<<countYZSendResp<<" NI::countSendReq "<<countYZSendReq<<endl;
    NIInjectReqPacketNum = 0;
    countReceivedReq = 0;
    countSendResp = 0;
    countReceivedResp = 0;
    adminInterval = 1;
    FlitBuffer *flitBuffer;
    while (VCNIFlitBuffer_list.size() != 0) {
        flitBuffer = VCNIFlitBuffer_list.back();
        VCNIFlitBuffer_list.pop_back();
        delete flitBuffer;
    }
    PacketBuffer *packetBuffer;
    while (packetBufferList_xVNToFlitize.size() != 0) {
        packetBuffer = packetBufferList_xVNToFlitize.back();
        packetBufferList_xVNToFlitize.pop_back();
        delete packetBuffer;
    }
    delete NI_in_port;
    delete NI_out_port;
}
