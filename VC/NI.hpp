/*
 * NI.hpp
 *
 *  Created on: 2019年8月23日
 *      Author: wr
 */

#ifndef NI_HPP_
#define NI_HPP_

#include "printfSW.hpp"//sw to printf
#include "parameters.hpp"

#include "VC/FlitBuffer.hpp"
#include "VC/Link.hpp"
#include "VC/NRBase.hpp"
#include "VC/PacketBuffer.hpp"
#include "VC/ROutPort.hpp"
#include "VC/VCNetwork.hpp"
#include "parameters.hpp"

#include <vector>
#include <iostream>
#include <cmath>
//
#include <cstring>
#include "VC/VCNI_MMP.h"


//
extern float cycles; //定义 全局周期
extern int globalTotalRecNumURS;//全局 接收到的 urs packet
extern int globalMaxSignalInNOC;//片上网络中 全局 最大sig 数量
extern int global_adminInterval;//全局准入 间隙
class FlitBuffer; //flit buffer

class VCNetwork;//vcnetwork

class RInPort;//inport

class PacketBuffer;// packet buffer

class ROutPort;//out port


//added begin

extern std::vector<std::vector<int>> LCS_packet_delay; // 没用
extern std::vector<std::vector<int>> URS_packet_delay;// urs packet delay，是一个全局 变量 ，多维向量

//added end
class NI : public NRBase { //NI 是 NR的子类
public:
    NI(int t_id, VCNetwork *t_vcNetwork, int t_vn_num, int t_vc_per_vn, int t_vc_priority_per_vn,
       int t_in_depth_fromVCNetwork);
// id 每创建 一个 ni id++，属于哪个 vc network ，从 vc network 中得到的depth 是 flit buffer的 depth

    int id;
    VCNetwork *vcNetwork;
    int vn_num;
    int vc_per_vn;
    int vc_priority_per_vn;
    //TrafficGenerator * tra1;

    std::vector<PacketBuffer *> packetBufferList_xVNToFlitize; //packet buffer 向量的每一个匀速是一个 packet buffer
    PacketBuffer *leaky_bucket_packet_buffer; //这个可以无限装的阿 ，但是我们要限制 什么时候才能装进来
    //std::vector<std::deque<Packet *> > packet_buffer_out;  // 0 request; 1 response
    std::deque<Packet *>  packetBufferOut_LeavingVCNI_0;  // 0 request; 1 response，双向队列 每个元素是一个 packet
        std::deque<Packet *>  packetBufferOut_LeavingVCNI_1;  // 0 request; 1 response
    std::vector<FlitBuffer *> VCNIFlitBuffer_list; //一个向量 ，每个元素是一个flitbuffer
    std::vector<int> out_vc;
    std::vector<int> state; // 0 I; 1 V; 2 A; // 0 idle 1 wait 2 active

    std::vector<int> priority_vc;
    int count_vc; // starvation forbidden
    std::vector<int> priority_switch;
    int count_switch; // starvation forbidden

    int rr_buffer; // round robin 策略
    int NI_in_depth;

    // for individual priority
    int starvation;
    int rr_priority_record;

    ROutPort *NI_out_port;//ni的 输出端口

    RInPort *NI_in_port; // ni的输入端口
//某些记录数据的变量
    static int countSendReq;
    int countYZSendReq;
    int countYZSendResp;
    static int countReceivedReq ;
    static int countSendResp;
    static int countReceivedResp;// this is shared in this class
    static int total_packet_id;  //added
    static int count_input;

    int total_delay;
    int total_num;
    int total_delay_URS;
    int total_num_URS;
//added
    int total_delay_head;
    int total_num_head;
    int total_delay_URS_head;
    int total_num_URS_head;
//  int total_delay_body;
//  int total_num_body;
//  int total_delay_URS_body;
    //int total_num_URS_body;
    int total_delay_headtail;
    int total_num_headtail;
    int total_delay_URS_headtail;
    int total_num_URS_headtail;

    int packetBufferOutReq_credit;
    int packetBufferOutResp_credit;

    int totaldelay_signal_trans_urs;

    int VCNITotalRespReceCount_URS;
    int totaldelay_signal_trans_lcs;
    //每个周期的双向延时
    int VCNI_transSigTwoWayDelay_perIntervalList[parameter_NI_statePeriodNum] = {0};//reward: sum of delay per interval
    //每个周期当前ni接收到的 resp packet数

    int VCNI_respPacketReceivedCount_perIntervalList[parameter_NI_statePeriodNum] = {
            0};//reward: sum of delay per interval
    int VCNI_requestPacketReceiByOthersCount_perIntervalList[parameter_NI_statePeriodNum] = {0};
    int VCNI_requestPacketReceiByOthersDelay_perIntervalList[parameter_NI_statePeriodNum] = {0};

    void writeAllTypeFlitInfoToFile(Flit* t_flit_fromNIInputCheck );
    void inputAction_TailHeadTailArrive(Flit* t_flit_fromNIInputCheck, int i_fromNIInputCheck);
    void inputAction_headFlitArrive(Flit *t_flit_fromNIInputCheck);
    void recordPacketWhenTailHeadtailArrive_reqURSArrive(Flit * t_flit_FromwriteAllTypeFlitInfoToFile);
    void recordPacketWhenTailHeadtailArrive_respURSArrive(Flit * t_flit_FromwriteAllTypeFlitInfoToFile);
    void  dequeue_VCNIFlitToSelfRouter_withAdmissionControl();
    VCNI_MMP * vcNI_MMP;
    float admControl_waitDelayTotal;
    void  recordAdmControlWaitDelayTotal();
    void wbq_RecordInputCheck(Packet *t_packet);
    int reqWaitFlitizeTime;
    int reqPacketNum;
    int NI_respURSPacketCount;
    int callFuncCount_writeAllTypeFlitInfoToFil;
    void packetDequeue_RLAdmissionControl();
    int NIRLHardSwitch;
    int NI31PacketWaitTimes;
    int NIInjectReqPacketNum;
    int adminInterval;
//added end

    static int worst_LCS;
    static int worst_URS;

    static int URS_delay_distribution[DISTRIBUTION_NUM];
    static int LCS_delay_distribution[DISTRIBUTION_NUM];

    static int URS_latency_single_dis[100];
    static int URS_latency_single_count;
    static int URS_latency_single_total;
    static int URS_latency_single_worst;

    static int LCS_latency_single_dis[100];
    static int LCS_latency_single_count;
    static int LCS_latency_single_total;
    static int LCS_latency_single_worst;

    static int converse_latency_single_dis[100];
    static int converse_latency_single_count;
    static int converse_latency_single_total;
    static int converse_latency_single_worst;

    // send
    bool flitize(Packet *, int);

    /* @bridf  printf NI send xx is in this func.
     *
     * NI id 166NI send 89071
     */
    void packetDequeue();

    void vcAllocation();

    void switchArbitration();

    void dequeue_VCNIFlitToSelfRouter();

    // receive

    void inputCheck();

    // main
    void runOneStep();

    //
    int VCNI_packetWaitNum;
    //
    //jingwei add
    int inj_packet;
    void token_injection();

    float tokens;
    float tokens_gen_rate;
    int capacity;
    //
    ~NI();

};

#endif /* NI_HPP_ */
