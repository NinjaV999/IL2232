/*
 * SlaveNI.hpp
 *
 *  Created on: 2019年9月9日
 *      Author: wr
 */

#ifndef AXI4_SLAVENI_HPP_
#define AXI4_SLAVENI_HPP_

#include <printfSW.hpp>//sw to printf
#include "AXI4/BasicNI.hpp"
#include "AXI4/SlavePort.hpp"
#include <fstream>

// switch; ordering unit (slave)
extern int global_respSignalNum;

class SlaveNI : public BasicNI {//SlaveNI 也继承来自 BasicNI
public:
    SlaveNI(int t_id, int slave_num, int master_num, TNetwork *t_network, VCNetwork *vc_network, int router_num_x,
            VCNetwork *vcNetwork, std::vector<BasicNI *> t_main_basicNI_list, int *NI_num);

    SlavePort *slaveNI_slavePort;
    //no master ports in salve NI (AKA mem NI)

    void enqueue(Message *);

    Message *dequeueMessage();

    std::deque<Message *> message_in, message_out;

    std::deque<Message *> message_request, message_response;

    // from message in (request) to message out (response)
    void response();

    void runOneStep();

    static int count;
    //add begin

    std::vector<BasicNI *> basicNI_list_inVCSlaveNI;
    int slaveNI_receivedRequestMessageCredit;
    int slaveNI_ToSendresponseMessageCredit;

    void writeFile_SlaveNIMessageResp();

#ifdef ofstreamSW_SlaveNIMessageResp
    ofstream  SlaveNIMessageResp;
#endif
    int memProcessReqCapacity;
    int memReqAcceptNowNum;
    int memSentOutRespNum;
    //add end
    virtual
    ~SlaveNI();
};

#endif /* AXI4_SLAVENI_HPP_ */
