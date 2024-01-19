/*
 * parameters.hpp
 *
 *  Created on: 2019年8月15日
 *      Author: wr
 */
#ifndef PARAMETERS_HPP_
#define PARAMETERS_HPP_

#include "FunctionDefineSwitch.h"

//////////config
#define paramCycleInterval 6000
#define paramTotalSimCycle 60000
#define paramInjectControlNumber 50000
#define parameterSW_TrafficGenerator_InjectionRate  2025//or 2023 or 2 //2021 is very small //816 is 2024+ no onlyMem
#define parameter_UniqueID_trackThisSignalLife 0 //20000,2000,5000,2

//main
#define generator_onlyURS_QoS0
#define readFileInjLvlCycles 300
// RL DQN
#define rewardBiasMain  0
//VCNI
#define parameter_NI_statePeriodNum 200//20230131
#define parameter_NI_myFlitBufferSize 9 //NI->vcInPort->flitbuffer size
#define parameter_NI_myRouterOutputPortBufferSize 100//WBQ use 10000,100 is enough, maybe less

#define parameter_NI_packetBufferOutReq_depth 10000//WBQ use 10000,
#define    parameter_NI_packetBufferOutResp_depth 10000//WBQ use 10000
//RL DQN
#define  parameter_GAMMA_inRLDQN 0.99

// SW
#define paramSW_myDestination 0 //0 wbq avg_hop=4(3.7)// 1 my avg_hop
//#define outPortNoInfinite
//Master NI SW



//VCNetwork



//SW IN Generator




//#define routerSW_priority_globalPakcetID
//////////////////////////////////
#define VN_NUM 2 //WBQ2  //In port.cpp, for URS : new flit buffer: 0~vn_num*vc_per_vn-1;  for LCS:   new flit buffer: vn_num*vc_per_vn~vn_num*(vc_per_vn + vc_priority_per_vn)-1
#define VC_PER_VN  4//2//WBQ 1 // URS not in SHARED_VC mode; LCS and URS under SHARED_VC, STD_LATENCY, and SHARED_PRI;  //
#define VC_PRIORITY_PER_VN 0 //only control LCS

// deafulat 1:1  2：1 1:2 2:2
// other 3 modes 2:1  3:1 3:2  4:2
//yz flow control: individual individual shard  total shared
//default 0 individual

//#define SHARED_VC //total 、shared 1
//#define STD_LATENCY  // This should be under the SHARED_VC mode//2// 1+2stadnard
//#define SHARED_PRI   // This mode is based on the SHARED_VC mode, and especially restricts that URS cannot use the PRIORITY VC// yz is this individual shared //3  //1+3 individual shares

#define STARVATION_LIMIT 20 // forbid starvation// no priority packet must go after 20

//#define REAL_TRAFFIC //inject packet mode  independant with 27-30line  generate traffic
#define LCS_URS_TRAFFIC
//#define LCS_GRS_TRAFFIC

#define INPORT_FLIT_BUFFER_SIZE 4; // number  // 4 buffer
#define OUTPORT_FLIT_BUFFER_SIZE 2; // number
#define FLIT_LENGTH 16 // byte //16*8=128bit   //definition in words, not really influence the program
#define INFINITE 10000//00  //10000->10000 00

#define BLOCK_SIZE 16   // byte  //used in tdm 16*8

#define S_TSHR_DEPTH 40 //slave tssr 组件  table ordering register

#define OFF_CHIP_MEMORY_DELAY 0 // wbq 30//debug now 0 need to be careful!

#define CACHE_DELAY 6// simulate cache memory

#define MASTER_LIST_RECORD_DELAY 1  //simulate hardware latency
#define MASTER_LIST_REFER_DELAY 0  //
#define SLAVE_LIST_REFER_DEALY 0  //
// axi4 data  to message  message(in NI) to (packer in VC  SIGNAL/TDM)
#define DELAY_FROM_P_TO_M  0  // packet/signal to message conversion time; AXI4 to message / message to AXI4 conversion time is 1 by default
#define DELAY_FROM_M_TO_P  0  // message to packet/signal conversion time

#define FREQUENCY 2 // GHz  //definition in words, not really influence the program

#define PRINT 100000

#define ID_RATIO 0.25  //
#define CYCLE_LOOP 100//000//100
#define SOURCE 0

//#define WITHOUT_MY_MECHANISM


#define SLOT_SIZE 64
#define SLOT_NUM SLOT_SIZE   // number

#define SLOT_LENGTH 1 // cycle(s)
#define LINK_TIME 2


#define X_NUM 14
#define Y_NUM 12
#define TOT_NUM 168

#define DISTRIBUTION_NUM 20


#endif /* PARAMETERS_HPP_ */
