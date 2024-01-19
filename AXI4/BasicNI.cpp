/*
 * BasicNI.cpp
 *
 *  Created on: Sep 16, 2019
 *      Author: wr
 */

#include "AXI4/BasicNI.hpp"
//
BasicNI::BasicNI(int t_id, int t_slave_num, int t_master_num, TNetwork *t_network, VCNetwork *vc_network,
                 int t_router_num_x, int *t_NI_num) {
    id = t_id;
    slave_num = t_slave_num;//168 有168个slave 和 160个mateer
    //cout<<"bascini slave_num "<<slave_num <<std::endl;
    master_num = t_master_num;//160
    //cout<<"bascini  master_num "<< master_num <<std::endl;
    TDM_network = t_network;
    VC_network = vc_network;
    router_num_x = t_router_num_x;
    NI_num = t_NI_num;

    //add begin，这可能也没有用到
    basicNI_packetWaitNum = 0;
    actionPicked_inBasicNI = 0;
    //add end
}
//这个函数名没有用到
void BasicNI::updateRLDQN_actionPicked(int t_actionPicked_inBasicNI) {
    actionPicked_inBasicNI= t_actionPicked_inBasicNI;
}

BasicNI::~BasicNI() {
    // TODO Auto-generated destructor stub
}

