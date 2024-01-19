/*
 * Network.hpp
 *
 *  Created on: 2019年8月19日
 *      Author: wr
 */

#ifndef VCNETWORK_HPP_
#define VCNETWORK_HPP_

#include "VC/Link.hpp"
#include "VC/NI.hpp"
#include "VC/VCRouter.hpp"
#include <vector>
#include <iostream>
#include <iomanip>
//
#include <fstream>
#include "VC/RInPort.hpp"
//
#ifdef ofstremSW_routerMonitor
extern std::ofstream router_monitor;
#endif
extern float cycles;

class VCRouter;

class NI;

class VCNetwork {
public:

    /** @brief VCNetwork
     *
     *  If the character is a newline ('\n'), the cursor should
     *  @param int router_num, int router_num_x, int NI_num_total, int* NI_num, int t_vn_num, int t_vc_per_vn, int t_vc_priority_per_vn, int t_in_depth
     *  @return void
     */
    VCNetwork(int router_num, int router_num_x, int NI_num_total, int *NI_num, int t_vn_num, int t_vc_per_vn,
              int t_vc_priority_per_vn, int t_in_depth_fromMain);

    std::vector<VCRouter *> router_list; // 向量 元素 为指向 vc router的 指针
    std::vector<NI *> NI_list;//向量 元素为 指向 NI的指针


    /** @brief yzVCNetwork runOneStep
     * use router and ni list's one step
     */
    void runOneStep();

    /** @brief    yz cout sum of all NI.totaldelay LCS without suffix, URS with suffix of "URS") / sum of NI.totalnum (why name it totalnum in NI?)
        */
    void average_LCS_latency();

    void average_URS_latency();

    /** @brief    yz cout sum of all NI.totaldelay_URS / sum of NI.totalnum (why name it totalnum in NI?)
         */
//yz add begin
    void average_LCS_latency_head();

    void average_LCS_latency_headtail();

    void average_URS_latency_headtail();

    void average_URS_latency_head();

    void average_URS_transSigTwoWayDelay(); // urs的 平均迁移的端到端延时


    // std::vector<BasicNI *>  basicNI_list_inVCNetwork_fromMain;// cannot ini here because vcn is ini before basicNI

    int avgNI_respMessageBufferUtilization_perStatePeriod[parameter_NI_statePeriodNum][TOT_NUM]={0};//it is better to get the totalnum from imports, but here use global value

    void  VNstatitsticsRun_perRLPeriod();

    void show_VCR_buffer_state();
    int debug_totalRespPacketReceived;
    int VNTotal_flitsActiveVCList[TOT_NUM] ={0};

//yz add ends
    int port_num_f(int router);

    /** @brief    yz repeat cout for router——num times.
      * cout << setprecision(3) << ((float)router_list[i]->port_total_utilization)/(port_num_f(i)*simulate_cycles/100)<< endl;
       */
    void port_utilization(int simulate_cycles);

    /** @brief   cout  NI_list[0]——>lcs distribution and worst case, but why nilist[0]？
        */
    void show_LCS_distribution();

    /** @brief   cout  NI_list[0]——>ursdistribution and worst case, but why nilist[0]？
        */
    void show_URS_distribution();


    int routerNum;
    int NINum;

    ~VCNetwork();
};

#endif /* VCNETWORK_HPP_ */
