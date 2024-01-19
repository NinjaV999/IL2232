/*
 * printfSW.hpp
 *
 *  Created on: Oct 13, 2022
 *      Author: yz
 */

#ifndef PRINTFSW_HPP_
#define PRINTFSW_HPP_


//ofstream SW

//#define  ofstremSW_routerMonitor
//#define ofstreamSW_destinationListCheck
//#define   ofstreamSW_MasterPortMessageOut
//#define ofstreamSW_SlaveNIMessageResp

//#define ofstreamSW_LCSPacketDelay
//#define ofstreamSW_VNRouterActiveVC
//#define ofstreamSW_epochTransDelay


//#define ofstreamSW_URSPacketDelay
//#define   ofstreamSW_writeActionListToDisk

//ofstream SW ends




// main percentage of running
#define printfSW_main_ProgramProcessPercentCycles 0

//main
#define printSW_main_VC_port_utilization 0
#define printSW_main_injection_distribution 0
#define printfSW_main_COUTlatency 1
#define printfSW_main_averageHop 0
#define printfSW_main_overAllSigNum 1
#define printfSW_main_globalPacketID 1
#define printfSW_main_average_URS_transSigTwoWayDelay
#define  printSW_main_VC_port_innet 1
//Generator
#define printfSW_TrafficGenerator_Type0GeneratorSend 0
#define printfSW_TrafficGenerator_Type2GeneratorSend 0

//VC/PACKET
#define pritfSW_VCPacket_PacketSignalIDshow 0
//VC/NI
#define printfSW_VCNI_Type0NISend 0
#define printfSW_VCNI_Type1NISend 0
#define printfSW_VCNI_Type0NIReceive 0
#define printfSW_VCNI_TypeElse0NIReceive 0


//AXI4 Converter
#define printfSW_AXI4Converter_MasterReceivedReadResponse 0
#define printfSW_AXI4Converter_MasterReceivedWriteResponse 0
#define printfSW_AXI4Converter_M_SlaveReceivedReadResponse 0
#define printfSW_AXI4Converter_M_SlaveReceivedWriteResponse 0

//AXI4 SlaveNI
#define printfSW_AXI4SlaveNI_SlaveReceived 0

//AXI4 SlavePort
#define printfSW_AXI4SlavePort_SlavePortReceived  0
#define  printfSW_AXI4SlavePort_SlavePortReceived_inMasterNI 0



#endif /* PRINTFSW_HPP_ */
