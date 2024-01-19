/*
 * main.cpp
 *
 *  Created on: 2019年8月20日
 *      Author: wr
 */

//for profiling: sudo sh -c 'echo 1 >/proc/sys/kernel/perf_event_paranoid'

#include "VC/VCNetwork.hpp"
#include "TDM/TNetwork.hpp"
#include "AXI4/MasterNI.hpp"
#include "AXI4/SlaveNI.hpp"
#include "AXI4/AXI4Signal.hpp"
#include "parameters.hpp"
#include "ComplexTrafficControl/TrafficController.hpp"
#include "printfSW.hpp" //sw to printf

#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <time.h>
//一系列 头文件 和 标准化 库
#include "interfaceCPP.hpp"
using namespace std;
//定义
//#define WRITE_LATENCY
#define writeFilePakcetDelay
#define ofstreamSW_LCSPacketDelay
#define ofstreamSW_URSPacketDelay
//#define WRITE_LATENCY
// added
//定义全局变量
std::vector<std::vector<int>> LCS_packet_delay;//std :: vector 动态数组 该数组里的每个元素也是一个动态数组
std::vector<std::vector<int>> URS_packet_delay;//这应该记录了每一个packet的
int overall_signal_num = 0;//总的信号数
int global_Packet_ID = 0;//全局 packet的代号
int global_adminInterval;//动作，准入间隙
int global_respSignalNum;//全局rsp信号数
int global_injSignalHighGroupNum = 0;//从热点注入的packet数，1个signal有1个msg 1个packet和多个flit
int global_actionFromSavedModel = 0;
int global_actionFromSavedModel1 = 0;

int slaveport_Pakcet_ID = 0;
int masterport_Pakcet_ID = 0;
int global_trans_ID = 0;//
int global_rInPort_ID = 0;
int LCS_packetdelay_dim = 13;
string settingFileName = (std::to_string(paramTotalSimCycle) + "_" +
                          std::to_string(parameterSW_TrafficGenerator_InjectionRate) + "_");
//定义一个特殊的字符串
ofstream refDelayPeriod;
ofstream refInjSigPeriod;
ofstream offlineStateRecord;
// add end

float cycles;//周期数
int globalMaxSignalInNOC = 0;
int globalTotalRecNumURS = 0;//全局接收到的urs packet的数量
vector<vector<int>> destination_list;//二维数组 目的地列表
vector<vector<int>> traffic_num;
vector<vector<vector<int>>>
        tdm_routing_table; // router ID, slot ID, inport/outport
vector<vector<int>> tdm_NI_table;              // NI ID, slot ID

int router_num = TOT_NUM; // TOT_NUM=168 定义router的个数
int router_x_num = X_NUM;//一行的node的数量，
int router_y_num = Y_NUM;//一列node的数量
//
#ifdef ofstremSW_routerMonitor
std::ofstream router_monitor;
#endif

//
int find_outport(int current, int next) {//函数找输出端口，根据current-next的值来找输出端口
    switch (current - next) {
        case 1:
            return 3; // left
        case -1:
            return 1; // right
        case Y_NUM:
            return 0; // up
        case -Y_NUM:
            return 2; // down
        default:
            assert(0 == 1);
            return -1;
    }
}

// yz It has in and out port,so is it only for TDM?
void update_tmp_table(int NI_slot, int source, int destination,
                      vector<int> *route_path,
                      vector<vector<vector<int>>

                      > *tmp_routing_table,
                      vector<vector<int>> *tmp_NI_table
) {
// update NI routing table
// assert((*tmp_NI_table)[source][NI_slot] == -1);
    (*tmp_NI_table)[source][NI_slot] =
            destination;

// update routing table
// int hop = abs(source/router_y_num - destination/router_y_num) + abs(source%router_y_num - destination%router_y_num) + 1;
    int hop = route_path->size();
    int last, current, next, inport, outport;
    int slot = NI_slot;
    for (
            int i = 0;
            i < hop;
            i++) {
// cout << (*route_path)[i] << ", ";
        current = (*route_path)[i];
        slot = (slot + LINK_TIME) % SLOT_SIZE; // in parameters.hpp #define SLOT_SIZE 64
        if (i == 0 && i == hop - 1) { // hop == 1
            inport = outport = 4;
        } else if (i == 0) { // source router
            inport = 4;
            next = (*route_path)[i + 1];
            outport = find_outport(current, next);
        } else if (i == hop - 1) { // destination router
            last = (*route_path)[i - 1];
            inport = find_outport(current, last);
            outport = 4;
        } else { // middle router
            next = (*route_path)[i + 1];
            last = (*route_path)[i - 1];
            inport = find_outport(current, last);
            outport = find_outport(current, next);
        }
        (*tmp_routing_table)[current][slot][0] =
                inport;
        (*tmp_routing_table)[current][slot][1] =
                outport;
    }
// cout << endl;
    route_path->

            clear();

}

int check_response_path(int source) {
    if ((source > 41 && source < 84) || (source > 125 && source < 168))
        return 2;
    return 1;
}

bool find_next_hop(int current_route, int destination, vector<int> *route_path,
                   int time, int interval,
                   vector<vector<vector<int>>

                   > tmp_routing_table) {
    int request_time = (time + interval) % SLOT_SIZE;
    int next_route;
    if (tmp_routing_table[current_route][request_time][0] == -1) {
        if (current_route == destination)
            return true;
        else {
// x routing
            if (current_route % router_y_num < destination % router_y_num) { // right 1
                next_route = current_route + 1;
                route_path->
                        push_back(next_route);
                if (
                        find_next_hop(next_route, destination, route_path,
                                      request_time, interval, tmp_routing_table
                        ))
                    return true;
            } else if (current_route % router_y_num > destination % router_y_num) { // left 3
                next_route = current_route - 1;
                route_path->
                        push_back(next_route);
                if (
                        find_next_hop(next_route, destination, route_path,
                                      request_time, interval, tmp_routing_table
                        ))
                    return true;
            }
// y routing
            if (current_route / router_y_num <
                destination / router_y_num) {                                              // down 2
                next_route = current_route + router_y_num; //+14
                route_path->
                        push_back(next_route);
                if (
                        find_next_hop(next_route, destination, route_path,
                                      request_time, interval, tmp_routing_table
                        ))
                    return true;
            } else if (current_route / router_y_num >
                       destination / router_y_num) {                                              // up 0
                next_route = current_route - router_y_num; //-14
                route_path->
                        push_back(next_route);
                if (
                        find_next_hop(next_route, destination, route_path,
                                      request_time, interval, tmp_routing_table
                        ))
                    return true;
            }
// path not available
            route_path->

                    pop_back();

            return false;
        }
    } else { // switch not available
        route_path->

                pop_back();

        return false;
    }
}

int find_a_path(int source, int destination, vector<int> *route_path,
                vector<vector<vector<int>>

                > tmp_routing_table,
                vector<vector<int>> tmp_NI_table
) { // return value: the time in source NI to send out messages to the destination
    for (
            int i = 0;
            i < SLOT_SIZE; i++) {
        if (tmp_NI_table[source][i] == -1) {
            route_path->
                    push_back(source);
            if (
                    find_next_hop(source, destination, route_path, i,
                                  LINK_TIME,
                                  tmp_routing_table))
                return
                        i;
        }
    }
    return -1;
}

void create_tdm_table() {

    // initial number counter
    traffic_num.resize(160);
    for (int i = 0; i < 160; i++) {
        traffic_num[i].resize(5);
        for (int j = 0; j < 5; j++)
            traffic_num[i][j] = 0;
    }

    // initial TDM routing table
    tdm_routing_table.resize(168);
    for (int i = 0; i < 168; i++) {
        tdm_routing_table[i].resize(SLOT_SIZE);
        for (int j = 0; j < SLOT_SIZE; j++) {
            tdm_routing_table[i][j].resize(2);
            tdm_routing_table[i][j][0] = -1;
            tdm_routing_table[i][j][1] = -1;
        }
    }

    // initial NI routing table

    tdm_NI_table.resize(168);
    for (int i = 0; i < 168; i++) {
        tdm_NI_table[i].resize(SLOT_SIZE);
        for (int j = 0; j < SLOT_SIZE; j++)
            tdm_NI_table[i][j] = -1;
    }

    // initial fail source-destination pair for acceleration

    vector<vector<int>> fail_record;

    fail_record.resize(4);
    for (int i = 0; i < 4; i++) {
        fail_record[i].resize(160);
        for (int j = 0; j < 160; j++)
            fail_record[i][j] = 1; // 1 -> valid i-j pair ; 0 -> invalid
    }

    // create TDM routing table according to communication requests

    bool continue_search = true;
    int source, destination;
    vector<int> route_path_forward, route_path_backward_1, route_path_backward_2;

    vector<vector<vector<int>>> tmp_routing_table;
    vector<vector<int>> tmp_NI_table;

    int type_1_num = 0, type_2_num = 0;

    cout << "Slot_size: " << SLOT_SIZE << endl;

    while (continue_search) {

        continue_search = false; // if no new path is established, quite for the next time

        for (int i = 0; i < 4; i++) {

            for (int j = 0; j < 160; j++) {
                if (fail_record[i][j] == 1) {
                    fail_record[i][j] = 0; // invalid
                    // back or forward synchronization
                    tmp_routing_table = tdm_routing_table;
                    tmp_NI_table = tdm_NI_table;

                    source = destination_list[j][0];
                    destination = destination_list[j][i + 1];

                    // to find forward path
                    int result_1 = find_a_path(source, destination,
                                               &route_path_forward,
                                               tmp_routing_table, tmp_NI_table);

                    // cout << j << " ;result_1: " << result_1 << endl;

                    if (result_1 >= 0) {
                        // change tmp_routing_table;
                        //   cout << "##########" << endl;
                        update_tmp_table(result_1, source, destination,
                                         &route_path_forward, &tmp_routing_table,
                                         &tmp_NI_table);

                        // to find backward path
                        if (check_response_path(source) ==
                            1) { // find the group the node belonging to find out how many return paths are required
                            int result_2 = find_a_path(destination, source,
                                                       &route_path_backward_1,
                                                       tmp_routing_table,
                                                       tmp_NI_table);
                            if (result_2 >= 0) {

                                update_tmp_table(result_2, destination, source,
                                                 &route_path_backward_1,
                                                 &tmp_routing_table,
                                                 &tmp_NI_table);

                                // update the TDM router/NI table
                                tdm_routing_table = tmp_routing_table;
                                tdm_NI_table = tmp_NI_table;
                                fail_record[i][j] = 1;
                                continue_search = true;
                                type_1_num++;
                                traffic_num[j][0]++;
                                traffic_num[j][i + 1]++;
                                // cout << "type_1_num: " << type_1_num << "; type_2_num: " << type_2_num << endl;
                            }
                        } else {
                            // find the first backward path
                            int result_2 = find_a_path(destination, source,
                                                       &route_path_backward_1,
                                                       tmp_routing_table,
                                                       tmp_NI_table);
                            if (result_2 >= 0) {
                                update_tmp_table(result_2, destination, source,
                                                 &route_path_backward_1,
                                                 &tmp_routing_table,
                                                 &tmp_NI_table);

                                // find the second backward path
                                int result_3 = find_a_path(
                                        destination, source, &route_path_backward_2,
                                        tmp_routing_table, tmp_NI_table);
                                if (result_3 >= 0) {
                                    update_tmp_table(result_3, destination,
                                                     source,
                                                     &route_path_backward_2,
                                                     &tmp_routing_table,
                                                     &tmp_NI_table);
                                    // update the TDM router/NI table
                                    tdm_routing_table = tmp_routing_table;
                                    tdm_NI_table = tmp_NI_table;
                                    fail_record[i][j] = 1;
                                    continue_search = true;
                                    type_2_num++;
                                    traffic_num[j][0]++;
                                    traffic_num[j][i + 1]++;
                                    // cout << "type_1_num: " << type_1_num << "; type_2_num: " << type_2_num << endl;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    cout << "type_1_num: " << type_1_num << "; type_2_num: " << type_2_num
         << endl;
    cout << "End of routing algorithm." << endl;

}

void initial_tdm_table() {
    ifstream outfile_router, outfile_NI, traffic_mode;

    outfile_router.open("copy_4/file_64/table_router.txt");//每个router里的solt table
    outfile_NI.open("copy_4/file_64/table_ni.txt");//每个连接到NI网络接口的 slot table
    traffic_mode.open("copy_4/file_64/traffic_mode.txt");//在每个slot循环里一个node对其他node的请求

    // initial TDM routing table
    vector<vector<int>> table_single;
    vector<int> slot;

    char a, b;

    for (int i = 0; i < 168; i++) {
        for (int j = 0; j < SLOT_SIZE; j++) {
            outfile_router >> a;
            outfile_router >> b;
            slot.push_back(int(a) - 49);
            slot.push_back(int(b) - 49);
            table_single.push_back(slot);
            slot.clear();
        }
        tdm_routing_table.push_back(table_single);
        table_single.clear();
    }

    int dest;
    vector<int> dest_list;
    for (int i = 0; i < 168; i++) {
        for (int j = 0; j < SLOT_SIZE; j++) {
            outfile_NI >> dest;
            dest_list.push_back(dest - 1);
        }
        tdm_NI_table.push_back(dest_list);
        dest_list.clear();
    }

    int traffic;
    vector<int> traffic_list;
    for (int i = 0; i < 160; i++) {
        for (int j = 0; j < 5; j++) {
            traffic_mode >> traffic;
            traffic_list.push_back(traffic);
        }
        traffic_num.push_back(traffic_list);
        traffic_list.clear();
    }

    // initial NI routing table

    /* for(int i=0;i<168;i++){
     for(int j=0;j<SLOT_SIZE;j++){
     cout << tdm_routing_table[i][j][0] <<  tdm_routing_table[i][j][1] ;
     }
     cout << endl;
     }*/

    /*for(int i=0;i<168;i++){
     for(int j=0;j<SLOT_SIZE;j++){
     cout << tdm_NI_table[i][j];
     }
     cout << endl;
     }*/

    /* for(int i=0;i<160;i++){
     for(int j=0;j<5;j++){
     cout << traffic_num[i][j];
     }
     cout << endl;
     }*/

    outfile_router.close();
    outfile_NI.close();
    traffic_mode.close();
}

int write_file_packetdelay(int t_current_epochs) {
    //std::vector<std::vector<int>> LCS_packet_delay;
    //std::vector<std::vector<int>> URS_packet_delay;
#ifdef ofstreamSW_LCSPacketDelay
    ofstream LCS_packetdelay;
    LCS_packetdelay.open(
            "../RecordFiles/" + settingFileName + "LCS_packetdelay.txt");// cmake in sub direc. so add ../
    for (int i = 0; i < global_Packet_ID; i++) {
        for (int j = 0; j < LCS_packetdelay_dim; j++) {
            LCS_packetdelay << LCS_packet_delay[i][j] << "	"; //
            // cout<<"LCS_packet_delay[i][j]"<<LCS_packet_delay[i][j]<<endl;
        }
        LCS_packetdelay << endl;
    }
    LCS_packetdelay.close();
#endif

#ifdef  ofstreamSW_URSPacketDelay
    ofstream URS_packetdelay;

    URS_packetdelay.open(
            "../RecordFiles/" + settingFileName + "epoch_" + to_string(t_current_epochs) + "_URS_packetdelay.txt");
    for (int i = 0; i < global_Packet_ID; i++) {
        for (int j = 0; j < LCS_packetdelay_dim; j++) {
//这个urs packet delay记录了每个packet的所有状态 ，共有13个维度
            URS_packetdelay << URS_packet_delay[i][j] << "	"; // first is ofstram second is vector
           // printf("开始写文件 \n");
        }
        URS_packetdelay << endl;
    }
    URS_packetdelay.close();
#endif
    return 0;
}

int write_file_destinationlist() {
    //  cout<<destination_list[0][0]<<" destination list"<<endl;
    /*  for(int i=0; i<160;i++){
     for(int j=0; j<5;j++){
     cout<<destination_list[i][j]<<"    ";
     }
     cout<<endl;
     }*/
#ifdef ofstreamSW_destinationListCheck
    ofstream destination_list_check;
    destination_list_check.open(
            "RecordFiles/" + settingFileName + "destination_list_check.txt");
    for (int i = 0; i < 160; i++) {
        for (int j = 0; j < 5; j++) {
            destination_list_check << destination_list[i][j] << " "; // 0~168
        }
        destination_list_check << endl;
    }
    destination_list_check.close();
#endif
    return 0;
}

void main_statisticOneStep(float t_cycles, VCNetwork *t_vcNetwork, std::vector<BasicNI *> t_main_basicNI_list) {
    int cycles = t_cycles;
    VCNetwork *vcNetwork = t_vcNetwork;
    std::vector<BasicNI *> main_basicNI_list = t_main_basicNI_list;
    int tempPeriod = int(paramTotalSimCycle / parameter_NI_statePeriodNum);//300
    //cout<<"main tempPeriod "<<tempPeriod<<endl;

// below usefule 20230206, just comment to avoid messy print results
//    if ( int(cycles) % int( paramTotalSimCycle/parameter_NI_statePeriodNum ) == 0 )
//    {
//        for (int i = 0; i < TOT_NUM ; i++) { //masterNI->basicNI_list_inBasicNI[message->signal->destination]->basicNI_packetWaitNum >0
//            int tempPeriodNum = int(cycles) / int(paramTotalSimCycle / parameter_NI_statePeriodNum);
//            //   cout<<"current tempPeriodNum"<< tempPeriodNum<<endl;
//            vcNetwork->avgNI_respMessageBufferUtilization_perStatePeriod[tempPeriodNum][i] = main_basicNI_list[i]->basicNI_packetWaitNum;
//            if (vcNetwork->avgNI_respMessageBufferUtilization_perStatePeriod[tempPeriodNum][i] > 3) {
//                cout << "state in main " << i << "_th_basicNI at tempPeriodNum_" << tempPeriodNum << " bufferUsed_"
//                     << vcNetwork->avgNI_respMessageBufferUtilization_perStatePeriod[tempPeriodNum][i] << " "
//                     << cycles << " " << endl;
//            }
//        }
//
//    }

}

void main_iniLCSURSPacketDelayList() {//清空
    // addded for delay
    LCS_packet_delay.resize(1000000);

    for (int i = 0; i < 1000000; i++) {
        LCS_packet_delay[i].resize(LCS_packetdelay_dim);
        //其是一个【1000000，13】的数组
        for (int j = 0; j < LCS_packetdelay_dim; j++)
            LCS_packet_delay[i][j] = 0;//全都初始化为0
    }
    URS_packet_delay.resize(1000000);
    for (int i = 0; i < 1000000; i++) {
        URS_packet_delay[i].resize(LCS_packetdelay_dim);
        for (int j = 0; j < LCS_packetdelay_dim; j++)
            URS_packet_delay[i][j] = 0;//同理初始化为0
    }
    // addded for delay end
}

int fuzzyReturnValue(float input1, float input2) {//置0
    if (input1 > 500) //high delay
    {
        return 3;//saturated
    } else if (input1 > 300 && input1 < 500) {

    }
}
//TODO

//this is a demo to run



int main()//重点研究主函数
{
    //global_adminInterval =1;
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //20230206begin
    int srandSeedValue = 0;//设置随机数种子为0
    srand(srandSeedValue);
    cout << rand() << " " << rand() << endl;//输出一个0-randmanx里的一个随机数
    srand(srandSeedValue);
    cout << rand() << " " << rand() << endl;
    rand();
    rand();
    rand();

    float arr[100];  // declare an array of size 100
    int i = 0;
    float num;
    FILE *fp;      // declare a file pointer
    fp = fopen("../RecordFiles/RL_RecordFiles/reward/epoch_0refDelayPeriod.txt", "r");
    if (fp == NULL) {   // check if file exists
        printf("File not found_1!\n");
        return 1;
    }//打开一个文件，若这个文件存不存在则返回file not found_1
    while (fscanf(fp, "%f", &num) != EOF && i < 100) {  // read integers from the file
        arr[i++] = num; //&num 表示将该文件 存储在num的地址里
    }//将这个邮件储存在arr 里，参考延迟存储在

    fclose(fp);  // close the file

    float refInjSignal[100];  // declare an array of size 100，参考注入信号
    FILE *fp_refInjSignal;      // declare a file pointer，还是一个文件类型的指针头
    int i_refInjSignal = 0;
    float num_refInjSignal;
    fp_refInjSignal = fopen("../RecordFiles/RL_RecordFiles/reward/YZRefInjPeriod.txt", "r");
    if (fp_refInjSignal == NULL) {   // check if file exists
        printf("File not found_2!\n");
        return 1;
    }//第二个文件发布开
    while (fscanf(fp_refInjSignal, "%f", &num_refInjSignal) != EOF &&
           i_refInjSignal < 100) {  // read integers from the file
        refInjSignal[i_refInjSignal++] = num_refInjSignal;
    }//将该文件的值存入书阻力
    fclose(fp_refInjSignal);  // close the file 关闭文件
    /* for (int j = 0; j < i_refInjSignal; j++) {
         // printf("%f ", refInjSignal[j]);
     }*/
    /////////////////////////////
    int actionFromTxt[100];  // declare an array of size 100
    FILE *fp_actionFromTxt;      // declare a file pointer
    int i_actionFromTxt = 0;
    float num_actionFromTxt;
    fp_actionFromTxt = fopen("../RecordFiles/RL_RecordFiles/readFiles/readActionFromTxt.txt", "r");
    if (fp_actionFromTxt == NULL) {   // check if file exists
        printf("File not found_3!\n");
        return 1;
    }
    while (fscanf(fp_actionFromTxt, "%f", &num_actionFromTxt) != EOF &&
           i_actionFromTxt < 100) {  // read integers from the file
        actionFromTxt[i_actionFromTxt++] = num_actionFromTxt;
    }
    fclose(fp_actionFromTxt);  // close the file，打开文件读取动作状态，
    /*for (int j = 0; j < i_actionFromTxt; j++) {
        //printf("%d ", actionFromTxt[j]);
    }*/
//////////////////////////
/////////////////////////////
    int randValue[500];  // declare an array of size 100
    FILE *fp_randValueFromTxt;      // declare a file pointer
    int i_randValue = 0;
    float num_randValue;
    fp_randValueFromTxt = fopen("../RecordFiles/RL_RecordFiles/readFiles/randValueFromTxt.txt", "r");
    if (fp_randValueFromTxt == NULL) {   // check if file exists
        printf("File not found_4!\n");
        return 1;
    }
    while (fscanf(fp_randValueFromTxt, "%f", &num_randValue) != EOF && i_randValue < 500) {  // read integers from the file ，不达到文件末尾或者数值小于500时，执行该循环
        randValue[i_randValue++] = num_randValue;
    }
    fclose(fp_randValueFromTxt);  // close the file
    for (int j = 0; j < i_randValue; j++) {
        //printf("%d", j);
        printf("%d ", randValue[j]);

    }
    std::cout<<std::endl;
    ////////////
    //前面读了4个文件并存入了4个数组
    std::ifstream resetfile;//创建读写文件流
    std::ofstream resetfile1;
    int resetReady=0,resetReady1=0;//创建resetReady 暂存储信号
    //below original running main
    //for (int ite_epochs = 20; ite_epochs < 50; ite_epochs++)
    //for(1==1)
    //{//循环次数，从50次开始循环
    //epoch 和 cycle 都是需要python 控制进行增加的
    //定义变量
    //srand(srandSeedValue);//定义随机数种子,只用初始化一次 随机种子没有改变
    float epochRewardDelay = 0;//每轮的奖励延时，每轮都需要初始化
    //globalMaxSignalInNOC = 0;//全局变量在NoC中的最大信号，全局信号 应该需要每轮
    int controlInjLvl;//在这里没有给初始值
    int yzRLWaitUntilInMain = 0;//等待时间
    //cout << " randvalues  " << rand() << " " << rand() << endl;//输出随机值

    //std::cout << ite_epochs << " epochs " << ite_epochs << " epochs " << ite_epochs << " epochs " << ite_epochs
    //         << " epochs "
    //        << ite_epochs << " epochs " << ite_epochs << " epochs " << ite_epochs << " epochs " << std::endl;
    //输出循环轮次

    // overall_signal_num = 0;//每轮一次
    // global_Packet_ID = 0;//每轮一次
    //
    clock_t start, finish; //定义clock 类型的信号 start和finish 用于 记录时间，需要每论进行操作
    //cycles = 0;//cycle 数量，每轮开始都需要对cycle进行赋诗话
    // start = clock();//记录截至到这里所用的时间，每轮开始都要重新计量时间

    int NI_total = 168;//168个ni
    int NI_num[168];//ni 数量的数组
    for (int i = 0; i < 168; i++) {
        NI_num[i] = 1; //全部初始化为1
    }//每个node有一个ni

    int vn = VN_NUM;//只用初始化一次，vn的数量为2
    int vc_per_vn = VC_PER_VN;//每个vn有四个vc
    int vc_priority_per_vn = VC_PRIORITY_PER_VN;//只用初始化一次,没用
    int flit_per_vc = INPORT_FLIT_BUFFER_SIZE; //只用初始化一次 4 定义了一些noc的基本参数，vc可以最多存储4个4flit
    //20230206ends
    //main_iniLCSURSPacketDelayList();// lcs_packet_delay is global so it could be packaged to be a function outside the main fucntion
    //每轮都进行一次

    /*VCNetwork *vcNetwork = new VCNetwork(router_num, router_x_num, NI_total,
                                         NI_num, vn, vc_per_vn,
                                         vc_priority_per_vn,
                                         flit_per_vc); // 168,14,168,int NI_num[168],vn=2,nvpervn=1 or 2 or 3, VC_PRIORITY_PER_VN=1 or 2 is lcs VC num,flit_per_vc=inport buffer
    TNetwork *tNetwork = new TNetwork(router_num, router_x_num, NI_total,
                                      NI_num);*/
    //创建了两个对象，每轮次都需要重新进行创建
    VCNetwork *vcNetwork; //创建一个指向vcNewt work对象的指针
    TNetwork *tNetwork;
    int slave_num = 168;//slave number ，初始化一次
    int master_num = 160;// master number，初始化一次，off chip memeory node 不能作为master
    std::vector<BasicNI *> main_basicNI_list;//创建一个动态数组 每个元素是一个 basicNI的值。每次重新声明都会相会该对象
    TrafficController *controller; //在这里定义这些指针，但在
    //但是我无法在循环中重复申明 因此可能需要main_basicNI_list.clear()
    /*for (int i = 0; i < 168; i++) { //对每一个slave 进行循环,
        //这肯定得每轮一次
        if (i != 31 && i != 38 && i != 45 && i != 52 && i != 115 && i != 122 && i != 129 && i != 136) {
            MasterNI *masterNI = new MasterNI(i, slave_num, master_num, tNetwork,
                                              vcNetwork, router_x_num, NI_num, main_basicNI_list,
                                              0.6); // MasterNI (int t_id, int slave_num, int master_num, TNetwork* t_network, VCNetwork* vc_network, int router_num_x, int* NI_num, float inj);
            main_basicNI_list.push_back(masterNI); //当i！=这些数字是 创建一个 MasterNI类型的对象 并插入该动态数组的尾部
        } else {
            SlaveNI *slaveNI = new SlaveNI(i, slave_num, master_num, tNetwork,
                                           vcNetwork, router_x_num, vcNetwork, main_basicNI_list, NI_num);
            main_basicNI_list.push_back(slaveNI);//如果等于这些数则创建一个SlaveNI 类型的数组存入
        }
    }
    for (int i = 0; i < 168; i++) {
        if (i != 31 && i != 38 && i != 45 && i != 52 && i != 115 && i != 122 && i != 129 && i != 136) {
            main_basicNI_list[i]->basicNI_list_inBasicNI = main_basicNI_list;//->用于类对象访问成员变量
        } else {//main_basicNI_list[i] 即该数组里的一个元素是一个指针指向的是一个类对象 ，->访问该对象里的一个元素，并将这个数组复制给他
            main_basicNI_list[i]->basicNI_list_inBasicNI = main_basicNI_list;
        }
    }*/
    //TrafficController *controller = new TrafficController(
    // main_basicNI_list);//main_basicNI_LIST created by WBQ, rename by YZ，创建一个trafficController对象
    // create_tdm_table();每轮一次
    //initial_tdm_table();//初始化tdm table 时分服用技术，每轮一次
    // controller->create_generator_traffic_num();//设置产生traffic的num，每轮一次

    int previousPakcetID = 0;
    int previousSignalID = 0;
    int previousSignalHighGroupID = 0;//0427yz

    float historyAvgDelay = 0.0;
    float historyRespPacket = 0.0;
    float historyNewPackets = 0.0;//申明一次 但 每轮 都需要初始化
    //
    float tempStateNext_inMain[4] = {0}; //存储state的数组？

    float tempStateCurrent_inMain[4] = {0};

//总共运行6万次
    int simulate_cycles = paramTotalSimCycle; // yz orig 2000，总的模拟周期，之用初始化一次
    int lastCycles = 0;//申明一次 但 每轮都要初始化
    int current_epoch= 0;
    int packet_len[168]={0};
    float avg_packet_len[168]={0};
    std::ofstream  files[168];
    std::ofstream  files2[168];
    std::ofstream  files3[168];
    float tokens_inj_rate[3] ={0};
    while(1) {  //开始每一个epoch

        resetfile.open("resetReady.txt", std::ios::in);//读取数据
        if (resetfile.is_open()) {
            resetfile >> resetReady;
            resetfile.close();
        }
        else
        {
            std::cout << "resetReady.txt can not open 1" << std::endl;
        }
        if (resetReady==1 &&  (int (cycles) == 60000 || int (cycles) == 0) ){//当resetready为1时，先把该信号zhi 0 再写数据，此时 py应该先执行getState，在执行action
            //writeAction 会被阻塞，也就是说state不读完 ，不可能触发下次写state，同理有由于getAction 会阻塞writeState 若Action不读完也不会触发下次写
            resetfile1.open("resetReady.txt", std::ios::out | std::ios::trunc);
            if (resetfile1.is_open()) {
                resetfile1 << 0;  //如果该信号有效的化怎将该信号置为0

                resetfile1.close();
            }
            else {
                std::cout << "resetReady.txt can not open 2" << std::endl;
            }
            srand(srandSeedValue);
            printf("reset 启动 \n");

            //开始写数据
            //调用reset 函数获取当前reset的状态,也就是说以下的一些参数是在每次episode开始都需要重新初始化的
            srand(srandSeedValue);//定义随机数种子
            epochRewardDelay = 0;//每轮的奖励延时
            globalMaxSignalInNOC = 0;//全局变量在NoC中的最大信号
            //int controlInjLvl;//
            yzRLWaitUntilInMain = 0;//
            cout << " randvalues  " << rand() << " " << rand() << endl;//输出随机值


            //std::cout << ite_epochs << " epochs " << ite_epochs << " epochs " << ite_epochs << " epochs " << ite_epochs
            //         << " epochs "
            //        << ite_epochs << " epochs " << ite_epochs << " epochs " << ite_epochs << " epochs " << std::endl;
            //输出循环轮次
            overall_signal_num = 0;
            global_Packet_ID = 0;
            global_injSignalHighGroupNum=0;//这里也要置0
            //
            //clock_t start, finish; //定义clock 类型的信号 start和finish 用于 记录时间
            cycles = 0;//cycle 数量
            start = clock();//记录截至到这里所用的时间

            /*int NI_total = 168;//ni 的总数量
            int NI_num[168];//ni 数量的数组
            for (int i = 0; i < 168; i++) {
                NI_num[i] = 1; //全部初始化为1
            }*/

            /*int vn = VN_NUM;
            int vc_per_vn = VC_PER_VN;
            int vc_priority_per_vn = VC_PRIORITY_PER_VN;
            int flit_per_vc = INPORT_FLIT_BUFFER_SIZE; // 4 定义了一些noc的基本参数
             */
            //20230206ends
            main_iniLCSURSPacketDelayList();// lcs_packet_delay is global so it could be packaged to be a function outside the main fucntion
            //清空两个队列
            //每次进大循环都要重新创建指针但在这里 进行赋值168 14 168 数组村的都是1 2 4，0，4
            vcNetwork = new VCNetwork(router_num, router_x_num, NI_total,
                                      NI_num, vn, vc_per_vn,
                                      vc_priority_per_vn,
                                      flit_per_vc); // 168,14,168,int NI_num[168],vn=2,nvpervn=1 or 2 or 3, VC_PRIORITY_PER_VN=1 or 2 is lcs VC num,flit_per_vc=inport buffer
            tNetwork = new TNetwork(router_num, router_x_num, NI_total,
                                    NI_num);
            //创建了两个对象

            //int slave_num = 168;//slave number
            //int master_num = 160;// master number
            main_basicNI_list.clear();//创建一个动态数组 每个元素是一个 basicNI的，清空
            for (int i = 0; i < 168; i++) { //对每一个节点进行循环
                if (i != 31 && i != 38 && i != 45 && i != 52 && i != 115 && i != 122 && i != 129 && i != 136) {
                    MasterNI *masterNI = new MasterNI(i, slave_num, master_num, tNetwork,
                                                      vcNetwork, router_x_num, NI_num, main_basicNI_list,
                                                      0.6); // MasterNI (int t_id, int slave_num, int master_num, TNetwork* t_network, VCNetwork* vc_network, int router_num_x, int* NI_num, float inj);
                    main_basicNI_list.push_back(masterNI); //这里的masterNI 并不是只有生成req信号的功能，其同时具send req 接受req send resp 结束resp
                    //delete masterNI;//当i！=这些数字是 创建一个 MasterNI类型的对象 并插入该动态数组的尾部
                } else {
                    SlaveNI *slaveNI = new SlaveNI(i, slave_num, master_num, tNetwork,
                                                   vcNetwork, router_x_num, vcNetwork, main_basicNI_list, NI_num);
                    main_basicNI_list.push_back(slaveNI);//如果等于这些数则创建一个SlaveNI 类型的数组存入
                    //delete slaveNI;//slave 是指只有receiv req 、resp sent resp
                }
            }
            for (int i = 0; i < 168; i++) { //将主函数的basic ni list作为属性给 复制到每一个basicni中去，其实创建的时候有但是不全
                if (i != 31 && i != 38 && i != 45 && i != 52 && i != 115 && i != 122 && i != 129 && i != 136) {
                    main_basicNI_list[i]->basicNI_list_inBasicNI = main_basicNI_list;//->用于类对象访问成员变量
                } else {//main_basicNI_list[i] 即该数组里的一个元素是一个指针指向的是一个类对象 ，->访问该对象里的一个元素，并将这个数组复制给他
                    main_basicNI_list[i]->basicNI_list_inBasicNI = main_basicNI_list;
                }
            }
            controller = new TrafficController(main_basicNI_list);//main_basicNI_LIST created by WBQ, rename by YZ，创建一个trafficController对象
            //main_basicNI_list用来traffic genrator 对应的ni的
            // create_tdm_table();
            initial_tdm_table();//初始化tdm table 时分服用技术
            controller->create_generator_traffic_num();//设置产生traffic的num

            previousPakcetID = 0;
            previousSignalID = 0;
            previousSignalHighGroupID = 0;//0427yz
           /* std::cout<<" previousSignalHighGroupID"<<" "<< previousSignalHighGroupID<<std::endl;
            std::cout<<std::endl;
            std::cout<<"reset  global_injSignalHighGroupNum"<<" "<< global_injSignalHighGroupNum<<std::endl;
            std::cout<<std::endl;*/
            historyAvgDelay = 0.0;
            historyRespPacket = 0.0;
            historyNewPackets = 0.0;
            packet_len[168]={0};
            avg_packet_len[168]={0};
            tokens_inj_rate[0]=0.27;
            tokens_inj_rate[1]=0.27;
            tokens_inj_rate[2]=0.27;
            for (int i = 0; i < 168; i++) {
                //printf("create files \n");
                //files[i].open("./no_avg_que_len/file_" + std::to_string(i) + ".txt", std::ios::app);
                files[i].open("./queueing_length/file_" + std::to_string(i) + ".txt", std::ios::app);
                files2[i].open("./inj_rate/file_" + std::to_string(i) + ".txt", std::ios::app);
                files3[i].open("./packet_latency/file_" + std::to_string(i) + ".txt", std::ios::app);
            }
            //
            //float tempStateNext_inMain[7] = {0}; //存储state的数组？

            //float tempStateCurrent_inMain[7] = {0};
            tempStateNext_inMain[4] = {0}; //存储state的数组？

            tempStateCurrent_inMain[4] = {0};
            //对currentState 进行赋值

            //int simulate_cycles = paramTotalSimCycle; // yz orig 2000，总的模拟周期
            lastCycles = 0;
            //
            //
            //按说在这里reset完了之后应该 将值存入到current_state,同时将current state 写入py
            //reset



        }
        //读取从py中读取action，将action 送入到以下迭代过程中
        //take action

        //从下面开始就是cycle=0的过程
        // for (; cycles < simulate_cycles; cycles++) {//省略了cycle的初始化，当cycle<simulate_cycles时进行循环

        if (cycles < paramInjectControlNumber) {//周期小于50000的时候

            if (int(cycles) % (readFileInjLvlCycles * 10) == 0 && cycles < 50000) {//3w，被3000整除且小于
                // if (int(cycles) % readFileInjLvlCycles  == 0  ) { //当cycles 能被3000整除
                lastCycles = cycles + 10 * readFileInjLvlCycles;
                int tempVaule = randValue[int(cycles / readFileInjLvlCycles)];//设置随机数的最大值
                //printf("cycles<lastCycles HHHH");
                tempVaule = 180;//250//180;
                controlInjLvl = tempVaule;
                controller->changeGenRate(tempVaule);//从第0个周期就会值为180

            }
            if (cycles >= lastCycles) {
                if (int(cycles) % 100 == 0) {
                   // printf("cycles>lastCycles");
                    //     cout<<"cycles test the inj lvl "<<cycles<<" >= "<<lastCycles <<endl;
                }
                controlInjLvl = 100;
                controller->changeGenRate(100);
            }
            controller->run(); //0-50000个周期都在生存
        }

        for (int i = 0; i < 168; i++) { //每个cycle每个port都做自己应该感的事情
            main_basicNI_list[i]->runOneStep();
        }


#ifdef paramSW_inMain_RLDQN_pickAction


        /*if (int(cycles) == 0) { //当cycles==0的 执行一些操作
            myRL_DQN->RLDQN_getInputCurrentEpochs(ite_epochs);//tell RLDQN current epochs
            main_RLDQNRunOneStep_at0cycle(myRL_DQN);
            previousPakcetID = global_Packet_ID;
            previousSignalID = overall_signal_num;

        }
        if (int(cycles) % int(paramTotalSimCycle / parameter_NI_statePeriodNum) == 0 &&
            cycles < 1200) //cycles = 0 ini 当 60000/200=300 ，当cycles为 300的整数倍数时且cycles数小于1200时什么操作也不做，因为此时网络并不拥堵给RL
            //学习没有什么意义
        {

        }*/
        if (int(cycles) == 0) { //cycles =0 则重置

            previousPakcetID = global_Packet_ID;
            previousSignalID = overall_signal_num;

        }
        //我在这里从 cycles=0； 就开始执行操作
        if (int(cycles) % int(paramTotalSimCycle / parameter_NI_statePeriodNum) == 0 && int(cycles) >= 300 && int(cycles) < 50000) {//cycles = 0 ini//当cycles能被300整除 ，且 1200<cycles<5000的值时将一些参量送入RL
          //  if (int(cycles) % int(paramTotalSimCycle / parameter_NI_statePeriodNum) == 0 && int(cycles) >= 6300 && int(cycles) < 50000)
          //进行学习parameter_NI_statePeriodNum
          //因为这是cycles循环里的所以以下都是每轮cycles都执行一次
            int currentPeriod_inMain = int(cycles) / int(paramTotalSimCycle / parameter_NI_statePeriodNum);//300
            int onePeriodBeforeCurrent_inMain = currentPeriod_inMain - 1;//current period的前一个period
            //myRL_DQN->RLDQN_getInputCurrentRLPeriod(currentPeriod_inMain); 没有RL块，因此注释掉
            int currentPacketID = global_Packet_ID;
            int currentSignalID = overall_signal_num;
            int statePacketNum = currentPacketID - previousPakcetID;//从6300cycles 开始记录 ，把全局packet记录 为current packet，计算出差值后
            int stateSignalNum = currentSignalID - previousSignalID;
            previousPakcetID = currentPacketID;//把current的赋值给previous， 第一次是6300-0 ，第二次是6600-6300
            previousSignalID = currentSignalID;
            //std::cout<<"global_injSignalHighGroupNum 2"<<" "<<global_injSignalHighGroupNum<<std::endl;
            //std::cout<<std::endl;
            int currentSignalHighGroupID = global_injSignalHighGroupNum;//热点的注入packet数
            int stateSignalHighGroupNum = currentSignalHighGroupID - previousSignalHighGroupID;
            //std::cout<<"stateSignalHighGroupNum"<<" "<<stateSignalHighGroupNum<<std::endl;
            //std::cout<<std::endl;
            previousSignalHighGroupID = currentSignalHighGroupID;


            //historyNewPackets = tempStateNext_inMain[0];
            //historyAvgDelay = tempStateNext_inMain[1];
            //historyRespPacket = tempStateNext_inMain[2];
            //    cout<<currentPeriod_inMain<<" globalTotalRecNumURS "<< globalTotalRecNumURS<<endl;
            //   cout<< " statePacketNum "<<statePacketNum<<" currentPacketID "<<currentPacketID<< " previousPakcetID "<<previousPakcetID<<endl;

            //cout<< " currentPeriod_inMain "<<currentPeriod_inMain<< " myRL_DQN-> "<<myRL_DQN->currentRLPeriod_inRLDQN<<endl;
            //i_period 2.1 update next_state
            float rewardDelay_perInterval = 0;//似乎是计算reward 过程
            float rewardRespCount_perInterval = 0;
            for (int i = 0; i < TOT_NUM; i++) {
                float OneNI_reward_perInterval_inMain = 0;//每个间隙的reward
                float OneNIrewardReceivedRespCount_perInterval_inMain = 0;
                if (vcNetwork->NI_list[i]->VCNI_respPacketReceivedCount_perIntervalList[onePeriodBeforeCurrent_inMain] !=
                    0) { //reward is preivous period's result. This period just begins and the reward of this perios is always zero because nothing happens yet.
                    OneNI_reward_perInterval_inMain = vcNetwork->NI_list[i]->VCNI_transSigTwoWayDelay_perIntervalList[
                            onePeriodBeforeCurrent_inMain];// no avg by pac num.,reward的值是当前 period前一个period的总的双向延迟
                    OneNIrewardReceivedRespCount_perInterval_inMain = vcNetwork->NI_list[i]->VCNI_respPacketReceivedCount_perIntervalList[onePeriodBeforeCurrent_inMain];
                    //前一个period的 一个ni的所有resp packet 数
                    rewardRespCount_perInterval = rewardRespCount_perInterval +
                                                  OneNIrewardReceivedRespCount_perInterval_inMain;// countNet = countNet + countNI
                    rewardDelay_perInterval = rewardDelay_perInterval + OneNI_reward_perInterval_inMain;//所有ni的count和reward
//                    cout << currentPeriod_inMain <<"  " << cycles << " cycles "<< " OneNI_reward_perInterval_inMain "
//                         << OneNI_reward_perInterval_inMain << "  rewardDelay_perInterval "
//                         << rewardDelay_perInterval<< endl;
                }
            }
            // epochRewardDelay = epochRewardDelay +rewardDelay_perInterval;
            rewardDelay_perInterval = rewardDelay_perInterval /     //一个period的平均双向延时
                                      rewardRespCount_perInterval; //rewardDelay_perInterval/rewardRespCount_perInterval;  //rewardRespCount_perInterval  +  rewardDelay_perInterval;// 800 * rewardRespCount_perInterval
            if (rewardRespCount_perInterval == 0) {
                rewardDelay_perInterval = 0;
                cout << " rewardRespCount_perInterval is 0" << endl;
            }
            int inputReward = 0;
            if (rewardRespCount_perInterval != 0 & rewardDelay_perInterval < arr[currentPeriod_inMain - 1] -
                                                                             1) { //seems there is sth about trunction, avoid 1.1<1.1
                inputReward = 1;//1
//                    cout << currentPeriod_inMain << " rewardIsOne" << "  " << rewardDelay_perInterval << " "
//                         << arr[currentPeriod_inMain - 1] << endl;
            }

            /*refInjSigPeriod.open(//写
                    "../RecordFiles/RL_RecordFiles/reward/epoch_" + std::to_string(ite_epochs) +
                    "recordInjSigPeriod.txt", ios::app);
            refInjSigPeriod << stateSignalNum << "  " << statePacketNum << " " << cycles << "  "
                            << randValue[int(cycles / readFileInjLvlCycles)] << "  " << globalMaxSignalInNOC << "  "
                            << rewardRespCount_perInterval << endl;
            refInjSigPeriod.close();*/
            //不需要写文件


            //tempStateNext_inMain[0] = statePacketNum;

           /* tempStateNext_inMain[1] = 0;//tempStateNext_inMain[0];// previous reward
            tempStateNext_inMain[0] = rewardDelay_perInterval;//   / 1000
            tempStateNext_inMain[3] = stateSignalHighGroupNum;
            tempStateNext_inMain[2] = stateSignalNum;// preivous period new signal
            tempStateNext_inMain[4] = controlInjLvl;*/
            //不需要写文件
            /*refDelayPeriod.open(  //写文件
                    "../RecordFiles/RL_RecordFiles/reward/epoch_" + std::to_string(ite_epochs) +
                    "recordSumDelayPeriod.txt", ios::app);
            refDelayPeriod << " inputReward  " << inputReward << " rewardDelay_perInterval  "
                           << rewardDelay_perInterval << " tempStateNext_inMain[0  "
                           << tempStateNext_inMain[0] * 1000 << " rewardRespCount_perInterval  "
                           << rewardRespCount_perInterval << " stateSignalNum  " << stateSignalNum
                           << " statePacketNum  " << statePacketNum << " delay/respCount  "
                           << rewardDelay_perInterval / rewardRespCount_perInterval
                           << " stateSignal  " << stateSignalNum
                           << endl;
            refDelayPeriod.close();*/
            // tempStateNext_inMain[1] = vcNetwork->NI_list[45]->packetBufferList_xVNToFlitize[1]->packet_queue.size();
            //tempStateNext_inMain[2] = statePacketNum;
#ifdef debugManualSetState
            if( myRL_DQN->actionHistoryList_inRLDQN[onePeriodBeforeCurrent_inMain][0]== 14 ){//debug
                    tempStateNext_inMain[0] = 1;//1;
                    tempStateNext_inMain[1] = 1;//1;
                }
                else{
                    tempStateNext_inMain[0] = 0;
                    tempStateNext_inMain[1] = 0;
                }
#endif
            //cout<< " currentPeriod_inMain "<<currentPeriod_inMain <<" preSelected action "<< myRL_DQN->actionHistoryList_inRLDQN[onePeriodBeforeCurrent_inMain][0]<<endl;
            //cout<<endl;
            // cout<<" main_basicNI_list[31]->basicNI_packetWaitNum "<< main_basicNI_list[31]->basicNI_packetWaitNum;
            // cout<<"   vcNetwork->VNTotal_flitsActiveVCList[31] "<<  vcNetwork->VNTotal_flitsActiveVCList[31]<<endl;
            /**************update StateNext **************/
            //myRL_DQN->Qnetwork_getInputStateNext(tempStateNext_inMain);// note it is the new state，
            //这个是把tempStateNext_inMain 赋值给 类中的的元素 由于后续没有调用 预设的weight 份新函数 因此 我认为没什么用
            float NOCRL_state_current[4] = {0};
            NOCRL_state_current[0] =rewardDelay_perInterval/300.0;//rewardDelay_perInterval / 300;
            //所有ni,在一个period(300 cycles)内 response packet到达的时间 - request signal create
            NOCRL_state_current[1] = stateSignalNum/1000.0;//stateSignalNum/ 1000;
            //signal injection number
            NOCRL_state_current[2] = stateSignalHighGroupNum/100.0;//stateSignalHighGroupNum / 100;
            //high group 的 signal injection num
            NOCRL_state_current[3] = rewardRespCount_perInterval/1000.0;//rewardRespCount_perInterval/1000;
            //the number of  response packet
            for(int i=0;i<=3;i ++) {
                printf("current state： %f \n", NOCRL_state_current[i]);

            }
            printf("current cycles: %f \n",cycles);
            if(NOCRL_state_current[1] <10 && NOCRL_state_current[2] <10){
            writeState(NOCRL_state_current,cycles);//reset 会进行一些初始化 初始化之后cycls=0 ，执行一个赋值

            //从这个地方开始是用于下一个状态更新
            getAction( tokens_inj_rate);
            for(int i=0 ;i <3;i++){
                tokens_inj_rate[i]= round(tokens_inj_rate[i]*1000.0)/1000.0;
            printf("根据当前状态选择的action是：%f \n",tokens_inj_rate[i]);}
            }
            //cycls =0 又会被300整除
            //我实际上只需要获取数据并不需要对其进行一些RL操作
           // torch::Tensor tempInput = torch::from_blob(NOCRL_state, {1, 4});//创建一个1*4的张量
            //cout << " tempInput " << tempInput << endl;
           // std::vector<c10::IValue> my_vector;//创建一个动态数组 来存储 张量
           // my_vector.push_back(c10::IValue(tempInput));
            //cout << " my_vector " << my_vector << endl;

           // at::Tensor output = module.forward(my_vector).toTensor();//forward 函数在这里使用计算每个动作的Q值
            //OutTest = output;
//                std::cout << currentPeriod_inMain << " output Result "
//                          << output.slice(/*dim=*/1, /*start=*/0, /*end=*/5) << " " << torch::argmax(output) << '\n';
//                cout << "  torch::argmax(prediction).item<int>();" << torch::argmax(output).item<int>() << endl;
           // global_actionFromSavedModel = torch::argmax(output).item<int>();//获得的动作
            // after T cycles, update the weights according to previous T cycles
            //i_period 2.2 update next_state

            // compute the action now, after update weights based on previous T cycles
            //i+1_period 1.1 input state_current & reward

            //cout<<vcNetwork->NI_list[0]->VCNI_transSigTwoWayDelay_perIntervalList[currentPeriod_inMain]<<" vcNetwork->NI_list[0]->VCNI_transSigTwoWayDelay_perIntervalList[currentPeriod_inMain] "<<endl;
            //cout<<vcNetwork->NI_list[0]->VCNI_respPacketReceivedCount_perIntervalList[currentPeriod_inMain]<<" vcNetwork->NI_list[0]->VCNI_respPacketReceivedCount_perIntervalList[currentPeriod_inMain] "<<endl;
            //should avoid divide by 0
#ifdef debugManualSetState
            if (myRL_DQN->actionHistoryList_inRLDQN[onePeriodBeforeCurrent_inMain][0] == 14) {
                    rewardDelay_perInterval = 1; //encourage it to be 14
                } else {
                    rewardDelay_perInterval =0;//
                }
#endif
//                cout << currentPeriod_inMain << " cycles " << cycles << " reward_perInterval_inMain "
//                     << rewardDelay_perInterval << "  rewardRespCount "
//                     << rewardRespCount_perInterval << endl;

            /**************update reward **************/
           // myRL_DQN->Qnetwork_getInputReward(
               //     inputReward);//
           // epochRewardDelay = epochRewardDelay + myRL_DQN->reward;//累积奖励

            /**************update qnet**************/
            /*if (rewardDelay_perInterval == 0 || cycles < 6000 || cycles > 45000  //avoid <5000 nan
                    ) { //avoid update all 0 state and then  random action
                //cout<<"currentPeriod_inMain not update" <<currentPeriod_inMain<<endl;//myRL_DQN->NIHistoryRespCount_inRLDQN[currentPeriod_inMain] = -2023;
            } else {
                // myRL_DQN->Qnetwork_updateQNetWeights();
            }*/

            /**************update qnet**************/

            /*tempStateCurrent_inMain[0] = tempStateNext_inMain[0];
            tempStateCurrent_inMain[1] = tempStateNext_inMain[1];
            tempStateCurrent_inMain[2] = tempStateNext_inMain[2];
            tempStateCurrent_inMain[3] = tempStateNext_inMain[3];
            tempStateCurrent_inMain[4] = tempStateNext_inMain[4];
            tempStateCurrent_inMain[5] = tempStateNext_inMain[5];
            tempStateCurrent_inMain[6] = tempStateNext_inMain[6];*/


#ifdef debugManualSetState
            if( myRL_DQN->actionHistoryList_inRLDQN[onePeriodBeforeCurrent_inMain][0]== 14 ){//debug
                  //  cout<<"preselect action 14 set stateNow11"<<endl;
                    tempStateCurrent_inMain[0] = 1;//1;
                    tempStateCurrent_inMain[1] = 1;//1;
                }
                else{
                 //   cout<<"preselect action 5 set stateNow00"<<endl;
                    tempStateCurrent_inMain[0] = 0;
                    tempStateCurrent_inMain[1] = 0;
                }
#endif
            /**************update current state **************/
            //myRL_DQN->Qnetwork_getInputStateCurrent(tempStateCurrent_inMain);// tranfer state into Qnet
            //i+1_period 1.2 compute action
            /**************compute action **************/
            //myRL_DQN->Qnetwork_computeActionPick();
           // offlineStateRecord.open(
                   // "../RecordFiles/RL_RecordFiles/Loss/epoch_" + std::to_string(ite_epochs) +
                   // "offlineStateRecord.txt", ios::app);
           /* offlineStateRecord << tempStateNext_inMain[0] << "  " << tempStateNext_inMain[1] << " "
                               << tempStateNext_inMain[2] << " " << tempStateNext_inMain[3] << " "
                               << tempStateNext_inMain[4] << " "
                               <<global_adminInterval
                               // << myRL_DQN->actionPicked
                               // << torch::argmax(OutTest).item<int>()
                               << " " << 0 rewardDelay_perInterval<< " " << rewardRespCount_perInterval << " "
                               << rewardDelay_perInterval * rewardRespCount_perInterval << "  "//inputReward
                               << cycles << "  "
                               << endl;
            offlineStateRecord.close();*/

            /**************transfer action result to controller **************/
            // if (vcNetwork->NI_list[31]->packetBufferList_xVNToFlitize[1]->packet_queue.size() > 0  ) // & cycles< paramInjectControlNumber
           /* myRL_DQN->actionHistoryList_inRLDQN[currentPeriod_inMain][0] = myRL_DQN->actionPicked;// store action into RLDQN list
            myRL_DQN->actionHistoryList_inRLDQN[currentPeriod_inMain][1] = rewardDelay_perInterval;// store reward into RLDQN list
            myRL_DQN->NIHistoryRespCount_inRLDQN[currentPeriod_inMain] = rewardRespCount_perInterval;// save the resp count of one NI
            myRL_DQN->globalMaxSignal_storeInDQN = globalMaxSignalInNOC;
            if (rewardRespCount_perInterval != 0) {
                myRL_DQN->reward_NIHistoryDelay_AvgByPackets[currentPeriod_inMain] =
                        ((rewardDelay_perInterval) * rewardRespCount_perInterval);
            }*/
        }
        //jingwei add
#ifdef WRITE_LATENCY
        if((int(cycles) % 300 ==0 && int(cycles) !=0 ) || int(cycles)==59999) {

            if(int(cycles)% 300 ==0 ){
                int current_period= int(cycles)/300;
                cout<<current_period<<endl;
                int previous_period = current_period-1;
                for (int i = 0; i < 168; i++) {
                    float OneNI_reward_perInterval_inMain = 0;//每个间隙的reward
                    float OneNIrewardReceivedRespCount_perInterval_inMain = 0;
                    float OneNIaverageLatency= 0;


                        OneNI_reward_perInterval_inMain = vcNetwork->NI_list[i]->VCNI_transSigTwoWayDelay_perIntervalList[
                                previous_period];// 比如说在300cycles 时 ，得到是0-300cycles的 ni[i] 端到端平均延迟
                    //cout<<"latecny  "<<i<<"  "<<OneNI_reward_perInterval_inMain<<endl;
                        OneNIrewardReceivedRespCount_perInterval_inMain = vcNetwork->NI_list[i]->VCNI_respPacketReceivedCount_perIntervalList[previous_period];
                    //cout<<"transaction  "<< i<<"  "<<OneNIrewardReceivedRespCount_perInterval_inMain<<endl;
                    //cout<<"avg "<< i<< "  "<<OneNI_reward_perInterval_inMain/OneNIrewardReceivedRespCount_perInterval_inMain<<endl;


                            OneNIaverageLatency=OneNI_reward_perInterval_inMain/OneNIrewardReceivedRespCount_perInterval_inMain;
                            //printf("aveagre latency %d \n", OneNIaverageLatency);
                            files3[i]<<OneNIaverageLatency<<std::endl;

                }

            }
            else if (int (cycles)=59999){
                cycles=cycles+1;
                int current_period= int(cycles)/300;
                int previous_period = current_period-1;
                for (int i = 0; i < 168; i++) {
                    float OneNI_reward_perInterval_inMain = 0;//每个间隙的reward
                    float OneNIrewardReceivedRespCount_perInterval_inMain = 0;
                    float OneNIaverageLatency= 0;


                    OneNI_reward_perInterval_inMain = vcNetwork->NI_list[i]->VCNI_transSigTwoWayDelay_perIntervalList[
                            previous_period];// 比如说在300cycles 时 ，得到是0-300cycles的 ni[i] 端到端平均延迟
                    OneNIrewardReceivedRespCount_perInterval_inMain = vcNetwork->NI_list[i]->VCNI_respPacketReceivedCount_perIntervalList[previous_period];
                    //比如说在300cycles 时 ，得到是0-300cycles的 ni[i] 源节点接收到的响应packet数，可以理解为完成的transaction 数

                        OneNIaverageLatency=OneNI_reward_perInterval_inMain/OneNIrewardReceivedRespCount_perInterval_inMain;
                        printf("aveagre latency %d \n", OneNIaverageLatency);
                        files3[i]<<OneNIaverageLatency<<std::endl;
                }
            }
        }
        //printf("Ni %d, value %d \n",0,vcNetwork->NI_list[0]->inj_packet);
        if((int(cycles) % 300 ==0 && int(cycles) !=0 ) || int(cycles)==59999){  // 每300个周期进行一次记录
          //  printf("111111 Ni %d, value %d \n",0,vcNetwork->NI_list[0]->inj_packet);
            for(int i=0; i<168; i++){
                avg_packet_len[i]=packet_len[i]/300.0;
                files[i]<<avg_packet_len[i]<<std::endl;
               // printf("Ni %d, value %d \n",i,vcNetwork->NI_list[i]->inj_packet);
                files2[i]<<(vcNetwork->NI_list[i]->inj_packet)/300.0<<std::endl;
                vcNetwork->NI_list[i]->inj_packet=0;
                //printf("Ni %d, value %d \n",i,vcNetwork->NI_list[i]->inj_packet);
                avg_packet_len[i]=0;
                packet_len[i]=0;
            }


        }

        for(int i=0; i<168; i++){
            //files[i]<<vcNetwork->NI_list[i]->packetBufferList_xVNToFlitize[0]->packet_num<<std::endl;
            packet_len[i]=packet_len[i]+vcNetwork->NI_list[i]->leaky_bucket_packet_buffer->packet_num;
        }
#endif
// jingwei add end
        int tempVaule = randValue[int(cycles / readFileInjLvlCycles)];
        //printf("controlInjLvl 是: %d \n",controlInjLvl);
        if (controlInjLvl > 0) {//180 and 100, 100 and 180 is controlled//衡成立
            for (int i = 0; i < 168; i++) {
                if (i != 31 && i != 38 && i != 45 && i != 52 && i != 115 && i != 122 && i != 129 && i != 136) {
// printf("动作转换 /n");//只控制req信号的注入 因此只有master
                    // if (ite_epochs == 0 || ite_epochs > 0) { //or >2
                    int actionOptionList[2] = {0}; //0508
                    actionOptionList[0] = 1;//0508 3 和22
                    actionOptionList[1] = 30; //0508结合训练到的网络选择这个动作没有那么多说明网络没有那么堵塞
                    // actionOptionList[2] = 30;
                    //actionOptionList[3] = 30;
                    //在这里我们调小 这个动作 即只需要更少的cycles就能消化掉网络中的packet
                    // actionOptionList[2] = 40;
                    global_adminInterval = actionOptionList[global_actionFromSavedModel];//找动作，根据强化学校找动作就在这里了，现在这里就是恒为0
                    //global_adminInterval=0;每个ni都会是同一个 global admininterval
                    //}
                    // if (ite_epochs == 0) {//

                    // }

                }
            }
        } else {//if not （controlInjLvl > 0), let everything go through，不执行的
            global_adminInterval = 1;
            for (int i = 0; i < 168; i++) {
                if (i != 31 && i != 38 && i != 45 && i != 52 && i != 115 && i != 122 && i != 129 && i != 136) {
                    vcNetwork->NI_list[i]->vcNI_MMP->actionAlpha2 = 101;
                    vcNetwork->NI_list[i]->adminInterval = 1;
                }
            }
            if (int(cycles) % 100 == 0) {
                //   cout << cycles << " tempvale restore " << tempVaule << endl;
            }
        }
       // printf("动作转换结束 \n");
        if (int(cycles) % 50 == 0) {// only adjust the injection cycles
            for (int i = 0; i < 168; i++) { //让master的开关均关闭
                if (i != 31 && i != 38 && i != 45 && i != 52 && i != 115 && i != 122 && i != 129 && i != 136) {
                    vcNetwork->NI_list[i]->NIRLHardSwitch = 0;//应该设置为0
                    // vcNetwork->NI_list[i]->vcNI_MMP->actionAlpha2 = randValue[int(cycles/300)] *1 ;
                } //每隔50个cycles 将开关全关闭
            }

            if (cycles < 50100 && cycles >= 6600) {//cycle> 6600 and cycle<70000，对于这个周期范围内
//cycles = 49800 时还会获取动作 其一直用到50100cycle，在此之后就变成全开
                // int tempVaule = randValue[int(cycles / 300)];

              //   int waitCycles = 0;
                // if (tempVaule > 4) {
                  //   waitCycles = 10
                 //} else {
                   //  waitCycles = 0;
                // }
               // std::cout << "action" << "" << global_adminInterval << std::endl;
                yzRLWaitUntilInMain = cycles +0;//表示由RL进行准入控制;+0 表示50个cycles里 全开，即没有admission control
                //printf("yzRLwait: %d \n", yzRLWaitUntilInMain);
            }
            else {
                for (int i = 0; i < 168; i++) {
                    if (i != 31 && i != 38 && i != 45 && i != 52 && i != 115 && i != 122 && i != 129 && i != 136) {
                        vcNetwork->NI_list[i]->NIRLHardSwitch = 1;
                    }
                }
                yzRLWaitUntilInMain = cycles +0;//6//+ 14;// 14 ;//5;，设置一个等待周期
            }
        }////若不在这个范围内则 全开

       // printf("yzRLwait: %d \n", yzRLWaitUntilInMain);
        if (cycles >= yzRLWaitUntilInMain) {//当周期数大于等待时间则打开开关
            for (int i = 0; i < 168; i++) {
                if (i != 31 && i != 38 && i != 45 && i != 52 && i != 115 && i != 122 && i != 129 &&
                    i != 136) {
                    vcNetwork->NI_list[i]->NIRLHardSwitch = 1;//Switch？ 应该设置为1
                }
            }
        }
         //选择30 表示 50个周期里前30是个不允许通过 后 20个允许注入

        //srand(srandSeedValue);
        //rand();


            if (cycles < 49800 && cycles >= 300) { //0-300个cycles 不调节，使用0-300的网络运行参数作为状态调节
                for (int i = 0; i < 168; i++) {
// 245.4349979599389
                   // vcNetwork->NI_list[i]->tokens_gen_rate=0.27;
                    if (i ==18 || i==21 || i==71 || i==17 || i==19 || i==20 || i==24 || i==27 || i==57 || i==58 || i==72 ) {
                       // cout<<tokens_inj_rate[0]<<endl;
                        //vcNetwork->NI_list[i]->tokens_gen_rate=tokens_inj_rate[0];//需要调节的是这个 tokens的平均产生率
                        vcNetwork->NI_list[i]->tokens_gen_rate=0.27;
                        if(vcNetwork->NI_list[i]->tokens>=0){
                            if(vcNetwork->NI_list[i]->tokens+vcNetwork->NI_list[i]->tokens_gen_rate <= vcNetwork->NI_list[i]->capacity){
                                //如果当前tokens+这个周期产生的tokens<最大容量，则可以把这个周期产生的tokens注入
                                vcNetwork->NI_list[i]->token_injection();
                                            }
                             else if(vcNetwork->NI_list[i]->tokens+vcNetwork->NI_list[i]->tokens_gen_rate > vcNetwork->NI_list[i]->capacity){
                              //如果当前tokens+这个周期产生的tokens>最大容量，则此时的tokens为最大容量，这两个机制共同保证了tokens不超过最大容量
                                     vcNetwork->NI_list[i]->tokens=vcNetwork->NI_list[i]->capacity;
                                }
                            }
                        else{
                            printf("tokens < 0 ! error ! \n");
                        }



                    }

                    if (i ==2 || i==5 || i==15 || i==16 || i==22 || i==25 || i==26 || i== 56 || i==64 || i==65 || i==68 || i==70 || i==77 || i==78 || i==79 || i==93
                            || i==101 || i==162 || i==115 || i==110) {
                        //cout<<tokens_inj_rate[1]<<endl;
                       // vcNetwork->NI_list[i]->tokens_gen_rate=tokens_inj_rate[1];//需要调节的是这个 tokens的平均产生率
                       vcNetwork->NI_list[i]->tokens_gen_rate=0.27;

                        if(vcNetwork->NI_list[i]->tokens>=0){
                            if(vcNetwork->NI_list[i]->tokens+vcNetwork->NI_list[i]->tokens_gen_rate <= vcNetwork->NI_list[i]->capacity){
                                //如果当前tokens+这个周期产生的tokens<最大容量，则可以把这个周期产生的tokens注入
                                vcNetwork->NI_list[i]->token_injection();
                            }
                            else if(vcNetwork->NI_list[i]->tokens+vcNetwork->NI_list[i]->tokens_gen_rate > vcNetwork->NI_list[i]->capacity){
                                //如果当前tokens+这个周期产生的tokens>最大容量，则此时的tokens为最大容量，这两个机制共同保证了tokens不超过最大容量
                                vcNetwork->NI_list[i]->tokens=vcNetwork->NI_list[i]->capacity;
                            }
                        }
                        else{
                            printf("tokens < 0 ! error ! \n");
                        }



                    }


                    if (i ==0 || i==1 || i==3 || i==4 || i==6 || i==9 || i==14 || i== 44 || i==59  || i==60 || i==61 || i==62 || i==63 || i==67 || i==69 || i==74
                        || i==75 || i==76 || i==80 || i==81|| i==82|| i==83|| i==85|| i==86|| i==88|| i==92|| i==95|| i==96 || i==97|| i==98|| i==99|| i==100
                        || i==102|| i==103|| i==108|| i==109|| i==111|| i==140|| i==141|| i==142|| i==147|| i==149|| i==152|| i==153|| i==161|| i==163) {
                      //  cout<<tokens_inj_rate[2]<<endl;
                        //vcNetwork->NI_list[i]->tokens_gen_rate=tokens_inj_rate[2];//需要调节的是这个 tokens的平均产生率
                        vcNetwork->NI_list[i]->tokens_gen_rate=0.27;

                        if(vcNetwork->NI_list[i]->tokens>=0){
                            if(vcNetwork->NI_list[i]->tokens+vcNetwork->NI_list[i]->tokens_gen_rate <= vcNetwork->NI_list[i]->capacity){
                                //如果当前tokens+这个周期产生的tokens<最大容量，则可以把这个周期产生的tokens注入
                                vcNetwork->NI_list[i]->token_injection();
                            }
                            else if(vcNetwork->NI_list[i]->tokens+vcNetwork->NI_list[i]->tokens_gen_rate > vcNetwork->NI_list[i]->capacity){
                                //如果当前tokens+这个周期产生的tokens>最大容量，则此时的tokens为最大容量，这两个机制共同保证了tokens不超过最大容量
                                vcNetwork->NI_list[i]->tokens=vcNetwork->NI_list[i]->capacity;
                            }
                        }
                        else{
                            printf("tokens < 0 ! error ! \n");
                        }



                    } else{
                        vcNetwork->NI_list[i]->tokens_gen_rate=0.27;//需要调节的是这个 tokens的平均产生率
                        if(vcNetwork->NI_list[i]->tokens>=0){
                            if(vcNetwork->NI_list[i]->tokens+vcNetwork->NI_list[i]->tokens_gen_rate <= vcNetwork->NI_list[i]->capacity){
                                //如果当前tokens+这个周期产生的tokens<最大容量，则可以把这个周期产生的tokens注入
                                vcNetwork->NI_list[i]->token_injection();
                            }
                            else if(vcNetwork->NI_list[i]->tokens+vcNetwork->NI_list[i]->tokens_gen_rate > vcNetwork->NI_list[i]->capacity){
                                //如果当前tokens+这个周期产生的tokens>最大容量，则此时的tokens为最大容量，这两个机制共同保证了tokens不超过最大容量
                                vcNetwork->NI_list[i]->tokens=vcNetwork->NI_list[i]->capacity;
                            }
                        }
                        else{
                            printf("tokens < 0 ! error ! \n");
                        }



                    }
                }
            }else {

                for (int i = 0; i < 168; i++) {

                   // if (i != 31 && i != 38 && i != 45 && i != 52 && i != 115 && i != 122 && i != 129 && i != 136) {
                        vcNetwork->NI_list[i]->tokens_gen_rate=0.27;
                        if(vcNetwork->NI_list[i]->tokens>=0){
                            if(vcNetwork->NI_list[i]->tokens+vcNetwork->NI_list[i]->tokens_gen_rate <= vcNetwork->NI_list[i]->capacity){
                                vcNetwork->NI_list[i]->token_injection();
                            }
                            else if(vcNetwork->NI_list[i]->tokens+vcNetwork->NI_list[i]->tokens_gen_rate > vcNetwork->NI_list[i]->capacity){

                                vcNetwork->NI_list[i]->tokens=vcNetwork->NI_list[i]->capacity;
                            }
                        }
                        else{
                            printf("tokens < 0 ! error ! \n");
                        }




                }
            }


            //

#endif

        vcNetwork->runOneStep();// 运行vc将ni的packet 转为flit 并发送一个flit

        main_statisticOneStep(cycles, vcNetwork, main_basicNI_list);//没用
        //} //cycle 循环完毕 这个过程可以理解为 cycle增加 不断迭代的过程
        cycles=cycles+1;



        //printf("cycles : %f \n",cycles);
        //状态更新完毕 将 新的状态存入 next state ，step
        //current state=next state
        //将current state 再次写给py
        /*tempStateNext_inMain[0]=
        tempStateNext_inMain[1]=
        tempStateNext_inMain[2]=
        tempStateNext_inMain[3]=*/

        if(int(cycles) == simulate_cycles){//之前定义的循环是for（cycles;cycles<simulate_cycles;cycles++)
            //也就是说在cycles=59999时 仍在进行上述循环 ，而 cycles =60000时结束上述循环，开始执行下面这一部分
            printf("cycles: %f \n",cycles);
        if (printfSW_main_averageHop == 1) {
                cout << "average hop:" << endl;
                controller->average_hop(); //没用
            }
            //
            if (printSW_main_injection_distribution == 1) {
                cout << "injection_distribution:" << endl;
                controller->injection_distribution(); //没用
            }

            if (printfSW_main_COUTlatency == 1) {
                cout << "average URS latency:" << endl;
                vcNetwork->average_URS_latency();
            }
#ifdef printfSW_main_average_URS_transSigTwoWayDelay //ifdef 预编译指令  如果该变量已经被定义则 进行操作
            vcNetwork->average_URS_transSigTwoWayDelay();
#endif
            /*
             cout << "average LCS latency:" << endl;
             vcNetwork->average_LCS_latency(); //nilist->total_delay/nilist->total_num
             cout << "average LCS_head latency:" << endl;
             vcNetwork->average_LCS_latency_head();
             cout << "average LCS_headtail latency:" << endl;
             vcNetwork->average_LCS_latency_headtail();
             cout << "average URS latency head:" << endl;
             vcNetwork->average_URS_latency_head();

             cout << "average URS latency head:" << endl;
             vcNetwork->average_URS_latency_head();
             cout << "average URS latency headtail:" << endl;
             vcNetwork->average_URS_latency_headtail();
             //cout << "average GRS queuing:" << endl;
             //tNetwork->queuing();
             */

            if (printSW_main_VC_port_utilization == 1) {
                cout << "VC port utilization:" << endl;
                vcNetwork->port_utilization(simulate_cycles);
            }


#ifdef writeFilePakcetDelay
            write_file_packetdelay(1);
#endif
#ifdef writeFileDestinationlist
            write_file_destinationlist();
#endif
            current_epoch= current_epoch+1;
            globalTotalRecNumURS = 0;
            cout << " global_respSignalNum " << global_respSignalNum << " global_injSignalHighGroupNum "
                 << global_injSignalHighGroupNum << endl;
            global_respSignalNum = 0;
            //  cout<<" globalTotalRecNumURS "<< globalTotalRecNumURS<<endl;
            delete vcNetwork;//先删除分配的动态内存空间
            vcNetwork = nullptr;
            delete tNetwork;
            tNetwork = nullptr;
            delete controller;
            controller = nullptr;

            for (int i = 0; i < 168; ++i) {
                files[i].close();
                files2[i].close();
                files3[i].close();
            }

            std::vector<BasicNI *>::iterator iter = main_basicNI_list.begin();//删除basic ni和master ni
            for (int i = 0; i < 168; i++) {
                delete (*iter);
                iter++;
            }

            cout << "overall_signal_num" << overall_signal_num << "  ";
            cout << "global_Packet_ID " << global_Packet_ID << endl;

            finish = clock();
            yzRLWaitUntilInMain = 0;// reset it.
            cout << "time consumption: " << ((double) (finish - start) / CLOCKS_PER_SEC) << endl;

           /* resetfile1.open("resetReady.txt", std::ios::out | std::ios::trunc);
            if (resetfile1.is_open()) {
                resetfile1 << 1;  //如果该信号有效的化怎将该信号置为0

                resetfile1.close();
            }
            else {
                std::cout << "resetReady.txt can not open 2" << std::endl;
            }*/
            // cout << "  epochRewardDelay  " << epochRewardDelay << endl;

            // cout << " stablebaselineInput 1 " << " stablebaselineInput 2 " << " stablebaselineInput 3 "
            // << " stablebaselineInput 4 " << endl;}

        }  //这部分应该是每次执行晚所有cycle 后 必须要执行的操作
    }

    return 0;
}