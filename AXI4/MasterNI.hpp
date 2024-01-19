/*
 * MasterNI.hpp
 *
 *  Created on: 2019年9月9日
 *      Author: wr
 */

#ifndef AXI4_MASTERNI_HPP_
#define AXI4_MASTERNI_HPP_

#include "AXI4/Converter.hpp"
#include "AXI4/MasterPort.hpp"
#include "AXI4/SlavePortInMasterNI.h"
#include "AXI4/BasicNI.hpp"

#include "TDM/TNetwork.hpp"
#include "VC/VCNetwork.hpp"
#include <iostream>
#include <fstream>
#include "AXI4/Converter_Master.hpp"
using namespace std;
class MasterPort;
// AXI4 receiver/sender; ordering unit (master); converter; switch;
class MasterNI:public BasicNI//继承关系
{
public:
  MasterNI (int t_id, int slave_num, int master_num, TNetwork* t_network, VCNetwork* vc_network, int router_num_x, int* NI_num,std::vector<BasicNI*> t_main_basicNI_list, float inj);

    Converter_Master* converterMasterInMasterNI;
    Converter * converter_slave;
  MasterPort* masterPort;
  SlavePortInMasterNI* slavePortInMasterNI;

  void runOneStep();
  void enqueue(Message*);
  Message* dequeueMessage();

  //
  std::vector<BasicNI*>  basicNI_list_inVCMasterNI;
    void updateRLActionInMasterPort();// update masterPort->RLAction to be actionPicked_inBasicNI from basicNI. //And actionPicked_inBasicNI comes from main.
    void writeFile_MasterPortMessageOut();

#ifdef ofstreamSW_MasterPortMessageOut
    ofstream  MasterPortMessageOut;
#endif
  //end


  virtual
  ~MasterNI ();
};

#endif /* AXI4_MASTERNI_HPP_ */
