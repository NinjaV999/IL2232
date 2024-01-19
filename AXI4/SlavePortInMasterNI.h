//
// Created by yz on 2/22/23.
//

#ifndef HELLO_SLAVEPORTINMASTERNI_H
#define HELLO_SLAVEPORTINMASTERNI_H

#include <printfSW.hpp>//sw to printf
#include "AXI4/BasicNI.hpp"
#include "AXI4/Message.hpp"
#include "TDM/TNetwork.hpp"
#include <vector>
#include <deque>

class BasicNI;
class TNetwork;
//
extern int global_Packet_ID;// used in cpp
extern int slaveport_Pakcet_ID;// used in cpp

class SlavePortInMasterNI
{
public:
    typedef std::map<int, Message*> DOUBLE;

    SlavePortInMasterNI (int t_master_num, BasicNI* t_NI);

    int master_num;
    BasicNI* basicNI;

    std::vector<std::vector<int> > slave_list;
    std::vector<std::vector<DOUBLE> > request_list; // masterID;ID;<sequenceID + message>

    std::deque<Message*> messageBuffer_receivedRequestMessage;

    static int count;

    void record_refer();
    void to_NoC();
    //add
    void to_buffer_fullNI_NoC();
    int slavePort_RequestPacketToMessageCredit;
    //add end
    virtual
    ~SlavePortInMasterNI ();
};
#endif //HELLO_SLAVEPORTINMASTERNI_H
