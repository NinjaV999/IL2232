/*
 * Network.cpp
 *
 *  Created on: 2019年8月19日
 *      Author: wr
 */

#include "VC/VCNetwork.hpp"

/*
 * @brief connet ni out to router in. connect router with next router
 * one step:run ni one step and router step for every router and ni.
 */
VCNetwork::VCNetwork(int router_num, int router_num_x, int NI_num_total, int *NI_num, int t_vn_num, int t_vc_per_vn,
                     int t_vc_priority_per_vn, int t_in_depth_fromMain) {
    // cout << "VCNetwork Object is being created" << endl;//yz
    int NI_seq = 0;
    routerNum = router_num;
    NINum = NI_num_total; //
    router_list.reserve(router_num); //根据 router数量 预留  router list的 大小
    NI_list.reserve(NI_num_total);//根据 NI数量
    // yz add 20230201 add init for parameterSW_VCNet_bufferFullStatusStatistics

    avgNI_respMessageBufferUtilization_perStatePeriod[parameter_NI_statePeriodNum][NINum] = {0};

    //

    for (int i = 0; i < router_num; i++) { // yz every router in 12*14
        int id[2];
        id[0] = i / router_num_x; //router num  =14  id_x
        id[1] = i % router_num_x;// id _y
        int in_out_port_num = 4 + NI_num[i]; // yz  NI_num[i] is cout to be 1
        //每个router的 的 输入输出端口数量 为 4 + 该router的ni 数 ，似乎总为1
        /*
        if (NI_num[i] != 1){
        cout<<"NI_num[i]"<<NI_num[i]<<endl;
        }
        */
        VCRouter *vcRouter = new VCRouter(id, in_out_port_num, this, t_vn_num, t_vc_per_vn, t_vc_priority_per_vn,
                                          t_in_depth_fromMain); // yz: id is a two-number array
     //创建 一个指针 指向 新的 vcrouter 实例 ，id记录了 其在网络中的 很纵坐标 ，t——vcNetwork 表示其所属的网络
     /*
      * VCRouter(int *t_id, int in_out_port_num, VCNetwork *t_vcNetwork,
                   int t_vn_num, int t_vc_per_vn, int t_vc_priority_per_vn,
                   int t_in_depth)
      */

        router_list.push_back(vcRouter); //将新建的 vcrouter存入 该 vc network 的 list中                                                                                               // yz router list add one router, router list is an empty vector newed in this func.

        // firstly connect NI to corresponding router
        for (int j = 0; j < NI_num[i]; j++) { //遍历 每个 router的  ni
            //创建新的 ni
            //seq 是 一个 vc network中 ni 的 排序 id
            NI *ni = new NI(NI_seq, this, t_vn_num, t_vc_per_vn, t_vc_priority_per_vn,
                            t_in_depth_fromMain); // yz new differnt ni with ni——seq as id, ni-seq++.
            NI_list.push_back(ni);
            NI_seq++;
            // connect NI's outport with router's inport
            ni->NI_out_port->out_link = vcRouter->in_port_list_inRouter[4 +j]->in_link; // router has 0-4 (N=5) inport from router, and 0-4 (N=5) inport from NI. n may change according to position.
            //inlink 有两个属性 分别是 rinport=本身 和 routport=null
            ni->NI_out_port->out_link->rOutPort = ni->NI_out_port;   //out link 的 routport是指向自己本什么的           // for better understanding, the link->out_port is the send port and and link->in_port is receive port
            //感觉的意思是 NI 和router都有自己的 port， 计算部分-》ni-》router-》网络 -》 router -》 ni -》 计算部分
            //因此 每个 vc router 包含了 四个方向的 输入端口 以及 一个 从自己 计算单元部分接受数据 与 ni输出端口匹配的输入端口
            // connect NI's inport with router's outport
            vcRouter->out_port_list_inRouter[4 + j]->out_link = ni->NI_in_port->in_link;// vcroute 的 输出端口 除了 四个方向 还有一个 与ni相连 访问计算单元的接口
            vcRouter->out_port_list_inRouter[4 + j]->out_link->rOutPort = vcRouter->out_port_list_inRouter[4 + j];
        }
    }
    // secondly, connect routers with each other according to the network size.
    for (int i = 0; i < router_num; i++) {
        int x, y; // (x,y)
        x = i / router_num_x;  //坐标（x，y）
        y = i % router_num_x;

        // port: 0->up; 1->right; 2->down; 3->left. each router connects to its right and down routers  //yz only right and down is enough.
        // port 1, right;
        //y是从 0-13 而 router num x为14，y=0-12 时 都有可能往右走
        if (y + 1 < router_num_x) {                                  // The right router exits;
            VCRouter *self = router_list[i]; // yz set the vallues inside vcrotuer： set self and next.
            VCRouter *next = router_list[i + 1]; //保证了 self 和 next 处在一行
            // connect self's outport with next's inport
            self->out_port_list_inRouter[1]->out_link = next->in_port_list_inRouter[3]->in_link; // yz set the vallues inside vcrotuer: set outlink and routport of self + next.
            self->out_port_list_inRouter[1]->out_link->rOutPort = self->out_port_list_inRouter[1];
            //先完成self输出端口的连接，即向右输出端口的 连接的 是右边节点 从左边接受数据的输入端口。
            //连接的输出端口为自己本身
            // connect next's outport with self's inport
            next->out_port_list_inRouter[3]->out_link = self->in_port_list_inRouter[1]->in_link;
            next->out_port_list_inRouter[3]->out_link->rOutPort = next->out_port_list_inRouter[3];
            //同理 因为前面已经保证了 二者 一定存在 同一行， next的 向左输出端口连接的是 左边节点 从 右边 接受数据的 输入端口
        }
        // port 2, down;
        if (x + 1 < router_num / router_num_x) { // The down router exits;
            //同理 确保 self 和 next 在一列
            VCRouter *self = router_list[i];
            VCRouter *next = router_list[i + router_num_x];
            // connect self's outport with next's inport
            self->out_port_list_inRouter[2]->out_link = next->in_port_list_inRouter[0]->in_link;
            self->out_port_list_inRouter[2]->out_link->rOutPort = self->out_port_list_inRouter[2];
            //向下的数据 数出口 连接 从上端接受的数据数入口
            // connect next's outport with self's inport
            next->out_port_list_inRouter[0]->out_link = self->in_port_list_inRouter[2]->in_link;
            //向上的数据输出口连接 从下发接受数据的端口
            next->out_port_list_inRouter[0]->out_link->rOutPort = next->out_port_list_inRouter[0];
        }
    }
}

void VCNetwork::runOneStep() {
    for (int i = 0; i < NINum; i++) {

        VNstatitsticsRun_perRLPeriod();//every step run once //没用
        // cout << "#######" << i << endl;
        NI_list[i]->runOneStep();// 每个 ni 都执行一次

    }

    for (int i = 0; i < routerNum; i++) {
        // cout << "#######" << i << endl;
        router_list[i]->runOneStep(); // 每个 router 都 传一个 flit

    }
    //
    if (((int) cycles) % 1 == 0) { show_VCR_buffer_state(); } //每个cycles都 展示一次
    //
}

int VCNetwork::port_num_f(int router) {
    if (router == 0 || router == 13 || router == 154 || router == 167)
        return 3; //四个角 3个端口
    else if (router <= 13 || router >= 154 || router % 14 == 0 || router % 14 == 13)
        return 4;//四条边 4 个端口
    return 5; //其余5个端口
}

void VCNetwork::port_utilization(int simulate_cycles) {
    for (int i = 0; i < routerNum; i++) {
        cout.setf(ios::fixed);
        // cout << i << " ";
        // cout << setprecision(3) << ((float)router_list[i]->port_total_utilization)/(port_num_f(i)*simulate_cycles/100)<< endl;
        cout << "router id "<<i<< " " <<setprecision(3) <<" used port times" <<((float) router_list[i]->port_total_utilization) << " percentage "
             << ((float) router_list[i]->port_total_utilization) / (port_num_f(i) * simulate_cycles / 100) << endl;
    }
}

void VCNetwork::average_LCS_latency() {
    cout.setf(ios::fixed);
    float a = 0.0;
    int b = 0;
    float c = 0;
    for (int i = 0; i < NINum; i++) {
        a += ((float) NI_list[i]->total_delay);
        // cout<<"i"<<i<<" ((float)NI_list[i]->total_delay);"<<((float)NI_list[i]->total_delay)<<endl;
        c += ((float) NI_list[i]->totaldelay_signal_trans_lcs);
        // cout<<"i "<<i<<" ((float)NI_list[i]->totaldelay_signal_trans_lcs );"<<((float)NI_list[i]->totaldelay_signal_trans_lcs )<<endl;
        if (((float) NI_list[i]->totaldelay_signal_trans_lcs) < 0) {
            cout << i << " i ";
            // cout<<"((float)NI_list[i]->totaldelay_signal_trans_lcs )"<<((float)NI_list[i]->totaldelay_signal_trans_lcs )<<endl;
            c -= ((float) NI_list[i]->totaldelay_signal_trans_lcs);
        }
        b += NI_list[i]->total_num;
    }
    cout << setprecision(1) << a / b << endl;
    cout << "NILIST->totalnumLCS    " << b << endl;
    //cout << "totaldelay_signal_trans_lcs " << setprecision(1) << c / b << endl;
    //  cout<<"NILIST->totalnumLCS    "<<b<<endl;
}

void VCNetwork::average_URS_latency() {

    cout.setf(ios::fixed); //输出京都
    float a = 0.0;
    int b = 0;
    float c = 0.0;
    int OneNI_respCount = 0; //一个ni的 resp的 数量
    int summedVCNetworkTotal_RespURSPakcet =0; //整个 网络的 resp urs的数量
    int OneNI_delayAccumedCount = 0;
    int VCNetworkTotal_callFuncCount_writeAllTypeFlitInfoToFile =0;
    int VCNITotal_RespURSPakcet = 0;
    VCNITotal_RespURSPakcet = NI_list[0]->countReceivedResp;

    float delayFromPeriod = 0;
    for (int i = 0; i < NINum; i++) {
        a += ((float) NI_list[i]->total_delay_URS); //累加了 所有 ni中 记录的参数
        b += NI_list[i]->total_num_URS;
        c += ((float) NI_list[i]->totaldelay_signal_trans_urs);
        OneNI_respCount += NI_list[i]->VCNITotalRespReceCount_URS;
        summedVCNetworkTotal_RespURSPakcet += NI_list[i]->NI_respURSPacketCount;
        VCNetworkTotal_callFuncCount_writeAllTypeFlitInfoToFile += NI_list[i]->callFuncCount_writeAllTypeFlitInfoToFil;
        for (int j = 0; j <parameter_NI_statePeriodNum; j++){ // 200 个 period 300 个 cycle 为一个period 总共有 60000cycle
            delayFromPeriod = delayFromPeriod +NI_list[i]->VCNI_transSigTwoWayDelay_perIntervalList[j];
        }
        // assert( ((float) NI_list[i]->totaldelay_signal_trans_urs) >0);
//        if (((float) NI_list[i]->totaldelay_signal_trans_urs) < 0) {
//            cout << i << " i ";
//            cout << "((float)NI_list[i]->totaldelay_signal_trans_urs ) why negative"
//                 << ((float) NI_list[i]->totaldelay_signal_trans_urs) << endl;
//           // c -= ((float) NI_list[i]->totaldelay_signal_trans_urs);
//        }
        // 0->167 ni
        // cout <<i<<"i_th NI"<< "NILIST URS delay"<< setprecision(1) << ((float)NI_list[i]->total_delay_URS)/NI_list[i]->total_num_URS << endl;
    }
    cout << "  : avgDelay 168NITotalDelay " << c << "VN: avgDelay 168NITotalDelay/VCNITotal_RespURSPakcet" << setprecision(3) << c / VCNITotal_RespURSPakcet
         << " VCNITotal_RespURSPakcet "<< VCNITotal_RespURSPakcet << endl;
    //cout<<" VN: VCNetworkTotal_callFuncCount_writeAllTypeFlitInfoToFile "<< VCNetworkTotal_callFuncCount_writeAllTypeFlitInfoToFile<<endl;
    cout << " this need to be debugged:  VCNetworkTotal_RespURSPakcet  "<<summedVCNetworkTotal_RespURSPakcet<<endl;
     cout << "this need to be compare  "<< setprecision(3) << c <<"  delayFromPeriod "<<  delayFromPeriod<<endl;
//         << " SumOfOneNI_respCount is " << OneNI_respCount << endl;
#ifdef  ofstreamSW_epochTransDelay
    ofstream epochTransDelay;
    epochTransDelay.open("../RecordFiles/RL_RecordFiles/epochTransDelay/epochTransDelay.txt",ios::app);
    epochTransDelay<< c / OneNI_respCount<<endl;
    epochTransDelay.close();
#endif 
    cout << "VN: WBQ totalnumURSCount    " << b << "  ";
    cout << "VN: WBQ allURS avgDelay   " << setprecision(1) << a / b
         << endl; // yz this is alls ni's urs average latency，这个 应该是不论是 resp req 都 是 看作 urs的 发送 ，有 所有urs的 时间 / 所有urs的数量

}

// added
void VCNetwork::average_LCS_latency_head() {//没用
    cout.setf(ios::fixed);
    float a = 0.0;
    int b = 0;
    for (int i = 0; i < NINum; i++) {
        a += ((float) NI_list[i]->total_delay_head);
        b += NI_list[i]->total_num_head;
    }
    cout << setprecision(1) << a / b << endl;
    cout << "NILIST->totalLCSNUM_head    " << b << endl;
}

void VCNetwork::average_LCS_latency_headtail() {
    cout.setf(ios::fixed);
    float a = 0.0;
    int b = 0;
    for (int i = 0; i < NINum; i++) {
        a += ((float) NI_list[i]->total_delay_headtail);
        b += NI_list[i]->total_num_headtail;
    }
    cout << setprecision(1) << a / b << endl;
    cout << "NILIST->totalLCSNUM_headtail    " << b << endl;
}

void VCNetwork::average_URS_latency_head() {
    cout.setf(ios::fixed);
    float a = 0.0;
    int b = 0;
    for (int i = 0; i < NINum; i++) {
        //      a += ((float)NI_list[i]->total_delay_URS_head);
        //      b += NI_list[i]->total_num_URS_head;
    }
    cout << setprecision(1) << a / b << endl; // yz this is alls ni's urs average latency
    cout << "NILIST->totalnumURS_head    " << b << endl;
}

void VCNetwork::average_URS_latency_headtail() {
    cout.setf(ios::fixed);
    float a = 0.0;
    int b = 0;
    for (int i = 0; i < NINum; i++) {
        //      a += ((float)NI_list[i]->total_delay_URS_headtail);
        //      b += NI_list[i]->total_num_URS_headtail;
    }
    cout << setprecision(1) << a / b << endl; // yz this is alls ni's urs average latency
    cout << "NILIST->totalnumURS_headtail    " << b << endl;
}

void VCNetwork::average_URS_transSigTwoWayDelay() {
    cout.setf(ios::fixed);
    for (int j = 0; j < parameter_NI_statePeriodNum; j++) {
        float a = 0.0;
        int b = 0;
        for (int i = 0; i < NINum; i++) {
            a += ((float) NI_list[i]->VCNI_transSigTwoWayDelay_perIntervalList[j]); //把每个period 统计的 双向 延迟 累加
            b += NI_list[i]->VCNI_respPacketReceivedCount_perIntervalList[j]; //每个 period的 resp count（可以认为是每个period的transaction数）
        }
        //cout<<"reward URS average TwoWayDelay" << setprecision(1) << a / b<<" " <<j<<" th RL period"<< endl; // yz this is alls ni's urs average latency
        // cout << "NILIST->totalnumURS_headtail    " << b << endl;
    }
}

void VCNetwork::VNstatitsticsRun_perRLPeriod() { //没用
    int tempCyclesPerPeriod_inVN = paramTotalSimCycle / parameter_NI_statePeriodNum;

    if ((int(cycles) % tempCyclesPerPeriod_inVN) == 0 and cycles > 1) { // cycles > 0 avoid -1 index
        // To understand, assume it is 500 cycles now.
        for (int i = 0; i < TOT_NUM; i++) {
            //  avgNI_respMessageBufferUtilization_perStatePeriod[tempCyclesPerPeriod_inVN][i]=NI_list[i]->
        }

    }

}

void VCNetwork::show_VCR_buffer_state() {
    int VNTotal_flitsActiveVC = 0;
    for (int i = 0; i < routerNum; i++) { //遍历所有router
        int VNPerNI_flitsActiveVC = 0;
        VCRouter *temp = router_list[i];// go through all routers
        //router_monitor << cycles << " RID " << i;
        for (int j = 0; j < temp->router_port_num; j++) {// all ports of one router
            RInPort *tempPort = temp->in_port_list_inRouter[j]; //遍历 每一个 router的 每一个输入端口
            //router_monitor << " PID " << j << " states";
            //for (int k: tempPort->state) { router_monitor << ' ' << k; }// four vc, cout each vc's state
            for (int k: tempPort->state) { //state 是一个数组 ，遍历整个数组 并 接受state的 置
                if (k == 2) {
                    VNPerNI_flitsActiveVC = VNPerNI_flitsActiveVC + 1;
                }
            }
        }//每有一个 state=2的 vc 则 VNPerNI_flitsActiveVC++
        VNTotal_flitsActiveVCList[i] = VNPerNI_flitsActiveVC; //记录当前 router的 活跃vc的 数量存储起来
        VNTotal_flitsActiveVC = VNTotal_flitsActiveVC + VNPerNI_flitsActiveVC; //所有router 活跃的vc
    }
#ifdef ofstremSW_routerMonitor
    router_monitor << "VNTotal_flitsActiveVC" << " " << VNTotal_flitsActiveVC << endl;
#endif
}

// added end
void VCNetwork::show_LCS_distribution() { //没用
    for (int i = 0; i < DISTRIBUTION_NUM; i++)
        cout << NI_list[0]->LCS_delay_distribution[i] << endl;
    cout << "worst case: " << NI_list[0]->worst_LCS << endl;
}

void VCNetwork::show_URS_distribution() { //没用
    for (int i = 0; i < DISTRIBUTION_NUM; i++)
        cout << NI_list[0]->URS_delay_distribution[i] << endl;
    cout << "worst case: " << NI_list[0]->worst_URS << endl;
}

VCNetwork::~VCNetwork() { //析构函数 用于清空 router list
    //cout<<NI_list[0]->countYZSendReq<<" "<< NI_list[2]->countYZSendReq<<" "<<endl;
    // cout << "Object VCNetwork is being deleted" << endl;//yz
    VCRouter *router;
    while (router_list.size() != 0) {
        router = router_list.back();
        router_list.pop_back();
        delete router;
    }

    NI *ni;
    while (NI_list.size() != 0) {
        ni = NI_list.back();
        NI_list.pop_back();
        delete ni;
    }
}
