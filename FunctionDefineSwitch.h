//
// Created by yz on 2/23/23.
//

#ifndef HELLO_FUNCTIONDEFINESWITCH_H
#define HELLO_FUNCTIONDEFINESWITCH_H

//VCNI admission controls//78.2 89.4 in main
//#define  VCNIDequeueToSelfRouter_flitAdmissionControl// 20230227
#define  VCNIDequeueToSelfRouter_packetAdmissionControl

//#define MasterNI_to_buffer_fullNI_NoC //SW to find messages' destinations with non-full resp. injection buffer.
//#define onlyFirstSubArea //only left upper network  // 76 will send to 117 as it is in range add 3 or minus 3


//
//#define routerSW_priority_responsePacket //202312

//SW in Generator
//#define veryLargeInjectionRateNode0
//#define  RL_totallyRandomAction

//all 128*8
#define allDataLength128_8
// SW IN RLDQN
#define paramSW_inMain_RLDQN_pickAction
#define param_RL_SwitchOn_inMain
#define Param_RL_writeFilePakcetDelay_InMain
//#define paramSW_Generator_onlyMemDestination // this is in 202302
// #define  generatorSW_noMemoryDestination  // no mem only random this is lower priority than onlyMemDesi


//Write file function switch
//#define MasterNIRun_writeFile
//#define SlaveNIRun_writerFile
#endif //HELLO_FUNCTIONDEFINESWITCH_H
