/*
 * TrafficGenerator.cpp
 *
 *  Created on: Oct 25, 2019
 *      Author: wr
 */

#include <printfSW.hpp>//sw to printf
#include "TrafficGenerator.hpp"

extern vector<vector<int> > destination_list; //全局 二维destination list
extern float cycles;
extern vector<vector<int> > traffic_num;//全局 二维 运动数量
// 静态成员变量
int TrafficGenerator::counta = 0; //记录生成的总的 req  sig
int TrafficGenerator::LCS_NUM[1000] = {0};
int TrafficGenerator::GRS_NUM[1000] = {0};
int TrafficGenerator::URS_NUM[1000] = {0};

//类对象的构造函数
TrafficGenerator::TrafficGenerator(int t_id, float read_ratio, int average_data_length,
                                   int t_count)//
{//160
    id = t_id;//generator self id when newed in controller，这个id 应该代表的是网络中 能发送包的节点位置 共有160个
    data_length = average_data_length;//average_data_length 128
    destination_num = 4;//4 有四个目的地数
    count = t_count;

    int x_id, y_id, d_x_id, d_y_id, destination;

    x_id = t_id / 14;//5/2=2   14rows 12 columns
    y_id = t_id % 14;//5%2=1   用来确定 generator的（X，Y)坐标
//这个 noc x=12 y=14
// 1 DRAM ID;
    if (x_id <= 2)//0  1  2
        d_x_id = 2;//2*14+3=31 2*14+10=38  //i!=31 && i!=38 && i!=45 && i!=52 && i!=115 && i!=122 && i!=129 && i!=136
    else if (x_id > 2 && x_id <= 5)//3 4 5
        d_x_id = 3;
    else if (x_id > 5 && x_id <= 8)//6 7 8
        d_x_id = 8;
    else
        d_x_id = 9;//9 10 11

    if (y_id <= 6)
        d_y_id = 3;
    else
        d_y_id = 10;

    destination = d_x_id * X_NUM + d_y_id;//yz one row  X_NUM 14
    //i!=31 && i!=38 && i!=45 && i!=52 && i!=115 && i!=122 && i!=129 && i!=136 destination的 值
//    int selectortemp1 = rand() % 90;// destination control
//    if (selectortemp1<45){
//        destination = 3 * X_NUM + 3;
//    }// need to be looped later
    destination_list_single.push_back(id);
    int x, y;
#ifdef  paramSW_Generator_onlyMemDestination// only push back mem nots
    for (int i = 0; i < destination_num; i++) {//destination——num=4 i= 0 1 2 4
        destination_list_single.push_back(destination);
    }
#elifdef generatorSW_noMemoryDestination // doesnt push back  destination = d_x_id * X_NUM + d_y_id; push only random destiontions
    for(int i=0; i<destination_num; i++){//destination——num=4 i= 0 1 2 3
      x = (rand() % 2 * 2 - 1) * (rand() % 3 + 1);//wbq destination average hop=4//+-1 *(1 or 2 or 3)
      y = (rand() % 2 * 2 - 1) * (rand() % 3 + 1);//wbq destination average hop=4
      if (paramSW_myDestination == 1) {
          x = (rand() % 2 * 2 - 1) * (rand() % 6 + 1);//yz destination average hop increase
          y = (rand() % 2 * 2 - 1) * (rand() % 6 + 1);//yz destination average hop increase
      }

      if (x_id + x >= 0 && x_id + x <= 11)//in the range
          d_x_id = x_id + x;
      else
          d_x_id = x_id - x;

      if (y_id + y >= 0 && y_id + y <= 13)
          d_y_id = y_id + y;
      else
          d_y_id = y_id - y;

      destination = d_x_id * 14 + d_y_id;
      destination_list_single.push_back(destination);
  }
#else
//one mem + 3 random nodes  3 node IDs; x-> +-(1~3) y-> +-(1~3)//yz because avghop=4 so distiantion is 0 (self） + (3+3）/2
    destination_list_single.push_back(
            destination);//destination for 1-2row left half->2*14+3=31  //destination is the mem controller node
    //   cout<<"\n"<<id <<" mem "<<destination<<"  ";
    for (int i = 0; i < destination_num - 1; i++) {//destination——num=4 i= 0 1 2
        x = (rand() % 2 * 2 - 1) * (rand() % 3 + 1);//wbq destination average hop=4//+-1 *(1 or 2 or 3)
        y = (rand() % 2 * 2 - 1) * (rand() % 3 + 1);//wbq destination average hop=4
        //x和y 的范围都是 -3，+3
//      if (paramSW_myDestination == 1) {
//          x = (rand() % 2 * 2 - 1) * (rand() % 6 + 1);//yz destination average hop increase
//          y = (rand() % 2 * 2 - 1) * (rand() % 6 + 1);//yz destination average hop increase
//      }

        if (x_id + x >= 0 && x_id + x <= 11)//in the range，x只能在【0，11】之间 ，如果加法超出了该坐标则换成减法替代
            d_x_id = x_id + x;
        else
            d_x_id = x_id - x;

        if (y_id + y >= 0 && y_id + y <= 13)//y也是同理
            d_y_id = y_id + y;
        else
            d_y_id = y_id - y;

        destination = d_x_id * 14 + d_y_id;// 一个的 id 对应的是 一个 mem node 和 3 个在average hop 以内的 random node
        //cout<<" generator destination "<<destination <<endl;
        destination_list_single.push_back(destination); //每个 id回望 destinattion——list——sigle存储 一个 id 一个 mem node 和 3 个随机node
        //    cout<< " random dest "<<destination<<"  ";
        //   destination_list_single.push_back(destination);//destination num =2, manual add 2
        //  destination_list_single.push_back(destination);//destination num =2 manual add 2
    }
#endif
    destination_list.push_back(destination_list_single);//SINGLE： SLAVE node id
    //再将这个之前的一个向量 作为一个元素 存储进入   另一个辖区嗯两
    total_hop = 0;
    request_num = 0;
    for (int i = 0; i < 100; i++)//why this in 3 time loop？
        injectCount_perInterval[i] = 0;
//  node group categorizing
    group = group_calculator(id);
    assert(group != 0);

    alpha1 = 7;
    beta1 = 18;
//    alpha1 = 700;
//    beta1 = 1800;
    switch (parameterSW_TrafficGenerator_InjectionRate) { //根据这个注入率 来判断 alpha2 和 beta2的值
        case 2026:
            alpha2 = 140;//3
            beta2 = 1000;//100
            break; //big value
        case 2025:
            alpha2 = 100;//3
            beta2 = 1000;//100
            break; //big value
        case 2024:
            alpha2 = 10;
            beta2 = 100;//100
            break; //big value
        case 2023:
            alpha2 = 10;
            beta2 = 160;//249993;
            break; //self define a value
        case 2022:// this is only one signal for tracking
            alpha2 = 1;
            beta2 = 24300 / 3 * 3 * 3;
            break; //self define a value
        case 2021:// this is only one signal for tracking
            alpha2 = 1;
            beta2 = 243000;
            break; //self define a value
        case 1:
            alpha2 = 7;
            beta2 = 493;
            break; //self define a value
        case 2:
            alpha2 = 7;
            beta2 = 243;
            break; //12
        case 4:
            alpha2 = 7;
            beta2 = 118;
            break;  //24
        case 6:
            alpha2 = 21;
            beta2 = 229;
            break;  //36
        case 8:
            alpha2 = 14;
            beta2 = 111;
            break;  //48
        case 10:
            alpha2 = 14;
            beta2 = 86;
            break;  //60
        case 12:
            alpha2 = 42;
            beta2 = 208;
            break;  //72
        case 14:
            alpha2 = 49;
            beta2 = 201;
            break;  //84
        case 16:
            alpha2 = 28;
            beta2 = 97;
            break;  //96
        case 18:
            alpha2 = 63;
            beta2 = 187;
            break;  //108

        default:
            alpha2 = 0;
            beta2 = 0;
            cout << "alpha beta 0 err" <<
                 endl;
            break;
    }

    refAlpha2 = alpha2;//100
    base1 = alpha1 + beta1; //25
    base2 = alpha2 + beta2;//1100
    state1 = state2 = 0;


    trans_id = 0;

    LCS_per_node = 0;
    URS_per_node = 0;
    GRS_per_node = 0;
}

int TrafficGenerator::group_calculator(int id) { //id to group  //yz why these group?//将generator 分组
    if ((id >= 0 && id <= 4) || (id - 7 >= 0 && id - 7 <= 4) || (id - 84 >= 0 && id - 84 <= 4) ||
        (id - 91 >= 0 && id - 91 <= 4))
        return 1;
    if ((id == 5 || id == 6) || (id >= 12 && id <= 16) || (id >= 21 && id <= 23) || (id == 89 || id == 90) ||
        (id >= 96 && id <= 100) || (id >= 105 && id <= 107))
        return 2;
    if ((id == 28 || id == 35 || id == 112 || id == 119) || (id >= 17 && id <= 20) || (id - 7 >= 17 && id - 7 <= 20) ||
        (id - 84 >= 17 && id - 84 <= 20) || (id - 91 >= 17 && id - 91 <= 20))
        return 3;
    if ((id >= 29 && id <= 34) || (id - 7 >= 29 && id - 7 <= 34) || (id - 84 >= 29 && id - 84 <= 34) ||
        (id - 91 >= 29 && id - 91 <= 34))
        return 4;
    if ((id >= 42 && id <= 47) || (id - 7 >= 42 && id - 7 <= 47) || (id - 84 >= 42 && id - 84 <= 47) ||
        (id - 91 >= 42 && id - 91 <= 47))
        return 5;
    if ((id == 48 || id == 55 || id == 132 || id == 139) || (id >= 56 && id <= 59) || (id - 7 >= 56 && id - 7 <= 59) ||
        (id - 84 >= 56 && id - 84 <= 59) || (id - 91 >= 56 && id - 91 <= 59))
        return 6;
    if ((id >= 60 && id <= 62) || (id >= 67 && id <= 71) || (id == 77 || id == 78) || (id >= 144 && id <= 146) ||
        (id >= 151 && id <= 155) || (id == 161 || id == 162))
        return 7;
    if ((id >= 72 && id <= 76) || (id - 7 >= 72 && id - 7 <= 76) || (id - 84 >= 72 && id - 84 <= 76) ||
        (id - 91 >= 72 && id - 91 <= 76))
        return 8;
    return 0;
}

//总共有8个group ， 调用group to ns 总共有4种情况


int TrafficGenerator::group_to_ns(int group) {
    switch (group % 4) {//8 groups to be 4 cases //control select 3 13 18 30
        case 0:
            return 10;//  4 8 //id  ((id>=29 && id<=34) || (id-7>=29 && id-7<=34) || (id-84>=29 && id-84<=34) || (id-91>=29 && id-91<=34)) ((id>=72 && id<=76) || (id-7>=72 && id-7<=76) || (id-84>=72 && id-84<=76) || (id-91>=72 && id-91<=76))
        case 1:
            return 100;// 1 5
        case 2:
            return 200;//2 6
        case 3:
            return 1000;//3 7
        default:
            assert(1 == 0);
    }
    return 0;
}

void TrafficGenerator::create_traffic_num() {
    for (int i = 0; i < 5; i++) {
        traffic_mode[i] = traffic_num[count][i]; //count 是 构造函数的参数，可能与哪个generator 有关
        //traffic num 存的是什么呢？

    }
}

bool TrafficGenerator::on_or_off(int alpha, int beta, int base, int &state) {//根据state的状态 来判断概率
    if (state == 0) { // off
        if (rand() % base < alpha) {//  STATE1 7/25 //rand() % base  生成一个0-base-1之间的随机数 ，其实是一个概率问题
            if ((cycles == 0 || cycles == 4000) & id == 0) {
                cout << "id " << id << " rand in generator " << rand() << endl;
            }
//如果生成的 随机数下与alpha 则 切换状态 ，并返回true
            state = 1;
            return true;
        } else {
            //否则不切换状态 并返回 false
            return false;
        }
    } else {  // on //同理如果 state =1 则也进行概率判断。 值得注意的是只有state=1是才能够返回 true
        if (rand() % base < beta) {
            state = 0;
            return false;
        } else
            return true;
    }
}

// return the state of traffic model: on or off
bool TrafficGenerator::request() {//0 layer if begin
#ifdef LCS_GRS_TRAFFIC
    if(id!=2){//1 layer if begin// node 2 is a test node 1oogb
#endif
    // if not lcs_grs, for example, it is lcs urs, then follow below to return T/F
    if (int(cycles) % group_to_ns(group) ==   //检查现在的周期 是不是 满足group对应的条件 8个 group有四种可能存在的条件 10 100 200 1000
        0) {//2 layer if begin // first level: thread or process state (state 1);// for nodes in different group, every 10/100/200/1000 cycles check one time.
        //属于不同组的generator 会在 不同的整数周期 参与判断
        if (on_or_off(alpha1, beta1, base1, state1)) {//3 layer begin //7 18 25 state1=0-》1 7/25 而 state1=1-》0 18/25
            if (on_or_off(alpha2, beta2, base2, state2))// 100 1000 1100 state2=0 发生切换的概率1/11 而 state=1 发生切换的概率大为100/110
                //cout<<"on_or_off ture \n";//yz
                //只有这些条件全部返回 true的时候 该函数 才返回true
                //state1=state2=1
                return true;
            return false;
        } else {//3 end another 3begin
            //若 state1=0 则 state2也强制为0
            state2 = 0;
            return false;
        }//3 end
    } else {//2 end another 2 begin//如不自特定的整数周期 则 直接 state1=1是否满足
        if (state1) {//3begin
            if (on_or_off(alpha2, beta2, base2, state2))
                return true;
            return false;
        } else//3 end another 3begin
            return false;
    }//2 end
    //总结 有两种生成请求的方式 ，一种 是根据 generator的组 划分在特定的周期 ，判断stat1和state2是否能打开 ，若能则生成请求，另一种 不考虑分组和特定的周期 只要 state1为1就可以判断state2有不有概率发生转变为1则 就会发生请求
#ifdef LCS_GRS_TRAFFIC  //what is this for？yz
    }else{//    1 end another 1 begin// else: id=2
      int i = rand()%32;
      if(i<5)
    cout<<"generator line 218 else happen i \n";
    return true;
      else
    return false;
  }//1 end
#endif
    // first level: thread or process state (state 1); second level: CPU request injection state (state 2);
}// 0 end

//Jingwei li added


void TrafficGenerator::inject(MasterNI *masterNI) {
    if (int(cycles) / paramCycleInterval < 10)//only record the first 10 intervals
    {//从0 到 594000 step =6000
        injectCount_perInterval[int(cycles) /
                                paramCycleInterval]++;// first/second/.. interval's count ++. For example, interval=2000， all 20k sim cycles are 10 intervals.
    }
    // types: read 0 or write 2:
    int RW_type, data;
    if (group < 5) {//group 1 2 3 4 //对于group小于5个generator
        RW_type = (rand() % 2 == 0 ? 2 : 0); // R/W 1:1  // RW_type=0 or 2，生成一个随机数 0 或  1 如果为0 则 RW——type 为req write
        //否则 为 req read
        //cout<<"RW_type"<<RW_type<<"line 236"<<endl;
        data = 128 * 8;// 数据长度
    } else {//group  5678
#ifndef   allDataLength128_8 //如果没有定义
        RW_type = (rand() % 4 == 0 ? 2 : 0); // R/W 3:1 //RW_type =0 or 2 ratio 3:1
        data = 64 * 8;
        //  cout<<"RW_type"<<RW_type<<"line 242"<<endl;
#endif
#ifdef  allDataLength128_8
        RW_type = (rand() % 2 == 0 ? 2 : 0); // R/W 1:1  // RW_type=0 or 2
        data = 128 * 8;
#endif
    }
// 不论generator 是 什么group 都统一生成 概率生成 数据长度、衡定的读写请求

    // QoS: 0->URS; 1->LCS (shared VCs with URS packets); 2->GRS; 3->LCS (individual VC(s) only for LCS packets) we do not support "1->shared VCs" mechanism any more.
    int selector = rand() % 90; //selctor 为 0-89 里的一个随机数
    int QoS;

#ifdef REAL_TRAFFIC  //
    if(group%4==1){//group 1 5 is selected
      if(selector < 40)
    QoS = 3;
      else if(selector < 80)
    QoS = 2;
      else
    QoS = 0;
  }else if(group%4==2){// 2 6 selected
      if(selector < 60)
    QoS = 3;
      else
    QoS = 2;
  }else if(group%4==3){//3 7 selected
      if(selector < 80)
    QoS = 3;
      else
    QoS = 0;
  }else{// 4 8 selected
      if(selector < 60)
    QoS = 2;
      else
    QoS = 0;
  }

 // cout<<"line 274"<<QoS<<" REAL_TRAFFIC qos"<<endl;
  if(QoS==0)
    URS_per_node++;
  else if(QoS==2)
    GRS_per_node++;
  else
    LCS_per_node++;

#ifdef LCS_GRS_TRAFFIC
   if(id==2){ // target node
       data = 128;
       int i = rand()%100;
       if(i<20)
      QoS = 0;
       else if(i<90)
     QoS = 2;
       else
     QoS = 3;
   }else{
       if(selector >= 65)
         QoS = 2;
       else if(selector < 65*0.5)
         QoS = 3;
       else
         QoS = 0;
   }
#endif
#endif

#ifdef LCS_URS_TRAFFIC
    // QoS: 0->URS; 1->LCS (shared VCs with URS packets); 2->GRS; 3->LCS (individual VC(s) only for LCS packets) we do not support "1->shared VCs" mechanism any more.
    // cout<<"line 304"<<QoS<<" LCS_URS_TRAFFIC qos is seeting"<<endl;//
    if (selector >= 45)
#ifdef generator_onlyURS_QoS0
        QoS = 0;
#else
        QoS = 3;
#endif
    else
        QoS = 0;
#endif

#ifdef SHARED_VC
    //  cout<<"line 314"<<QoS<<"  SHARED_V  qos is seting"<<endl;
  if(QoS == 3)
    QoS = 1;
#endif

    // QoS = 2;
    //cout << "TDM" << endl;


    // destination:
    int dest_id;
//QoS 恒为0 urs
    if (QoS != 2) {
        dest_id = destination_list_single[rand() % 4 + 1];// 若 QoS为 不为2 因为这里 一直为0 则 dest——id 就是 mem 和 3 个随机 node 中选一个
    } else {
        int selector_dest = rand() % traffic_mode[0];
        if (selector_dest < traffic_mode[1]) {
            dest_id = destination_list_single[1];//1

        } else if (selector_dest < traffic_mode[1] + traffic_mode[2]) {
            dest_id = destination_list_single[2];
        } else if (selector_dest < traffic_mode[1] + traffic_mode[2] + traffic_mode[3]) {
            dest_id = destination_list_single[3];
        } else {
            dest_id = destination_list_single[4];
        }
    }

    if ((int) cycles / 1000 < 1000) {// max 100,000 recorded in WBQ's code
        if (QoS == 3)
            LCS_NUM[(int) cycles / 1000]++;
        else if (QoS == 2)
            GRS_NUM[(int) cycles / 1000]++;
        else
            URS_NUM[(int) cycles / 1000]++; //总是执行着一个，每1000个 cycle  分为一段
    }


    total_hop += abs(dest_id / X_NUM - id / X_NUM) + abs(dest_id % X_NUM - id % X_NUM);
    //通过只能 目的地的 id 和generator 的 id 计算出 这一次 传送的 条数 ，通过累加 计算出 总体条数， y方向的跳数 （隔了几行） x方向的跳数 隔了几列
    request_num++; //请求信号数加1
    // AXI4Signal (int t_id, int t_RW_type, int t_dest, int t_length, int t_QoS, float t_cycles=0.0);//type is read or write, length is data size,qos is packet qos type, 0 or 3 for lcs_urs, cycles is current cycle

    AXI4Signal *signal = new AXI4Signal(trans_id, RW_type, dest_id, data, QoS, overall_signal_num, cycles);
    //trans id：就是一个0-15之间的编号，似乎没有什么特殊意义  rw type 信号的 类型 ， data 传输数据的长度 ，dest id 目标节点 ， QoS恒为 0 即URS。 overall_signal_num全局信号数 ， cycles为到达时间2
    signal->signal_trans_createcycles = cycles;//yz 20230125 add signal trans create cycles 。信号迁移开始的时间
    if (signal->destination != 31 && signal->destination != 38 && signal->destination != 45 &&
        signal->destination != 52 && signal->destination != 115 && signal->destination != 122 &&
        signal->destination != 129 && signal->destination != 136) {
        signal->signalGoToMem = -1; // not go to mem
    } else {
        signal->signalGoToMem = 1; // go to mem 如果信号的目的地是 上述八个节点 则 siggotomem=1
    }
    if ((id == 17) || (id == 24) || ((id == 101)) || (id == 108)) {
        global_injSignalHighGroupNum++; //如果generator的 id 为这4 个点则 hotspot 全局注入的信号 ++
    }

    //
    //cout<<"overall_signal_num"<<overall_signal_num<<endl;
    overall_signal_num++;//yz added ，不论是不是 热点 ，但是信号产生了则 全局 信号 数量++
    //traffic genrator 产生信号的数量 认为是 请求信号的数量
    //cout<<trans_id<<"trans_id"<<endl;
#ifdef LCS_GRS_TRAFFIC
    cout<<"line 314"<<QoS<<"  LCS_GRS_TRAFFIC  qos is seting"<<endl;
   // convert the QoS tag accordingly and set the test_tag_qos_convert tag in AXI4 to 1 for trace and test
  if(id == 2){
      if(signal->QoS == 2 && rand()%70 < 60){
      signal->QoS = 3;
      signal->test_tag_qos_convert = 1;
      }
  }
#endif

#ifdef STD_LATENCY
    signal->test_tag=0;
  if(QoS==1){
      signal->QoS = 0;
      signal->test_tag = 1;
  }
#endif

    if (RW_type == 0) { // read
        // std::cout << "Send read request at cycle:" << cycles << std::endl;
        masterNI->converterMasterInMasterNI->AXI4ReqFromGenInMasterConverter_in_read.push_back(signal);
        counta++;//shared
        if (printfSW_TrafficGenerator_Type0GeneratorSend == 1) {
            cout << "RW_type0 Send read request at cycle  " << cycles << "  Generator Send " << counta << endl;
        }
    } else {
        assert(RW_type == 2); // write
        // std::cout << "Send write request at cycle:" << cycles << std::endl;
        masterNI->converterMasterInMasterNI->AXI4ToSendReq_in_write.push_back(signal);
        counta++;//shared
        if (printfSW_TrafficGenerator_Type2GeneratorSend == 1) {
            cout << "RW_type2 Send write request at cycle " << cycles << "  Generator Send " << counta << endl;
        }
    }
    trans_id = (trans_id + 1) % 16;//possible value:0,1,2,3,...15 每个请求信号都会有一个trans id 在 0-15 循环 ，生成一个就+1 ，加到15在加1就变为0
    //cout<<"tran_id"<<trans_id<<endl;

}

void TrafficGenerator::average_hop() {
    cout.setf(
            ios::fixed);// Just set precision of printing.  cout.setf(ios::fixed). makes cout print floats with a fixed number of decimals and cout.precision(3). sets this number to be three.
    //cout << id << " ";
    cout << setprecision(2) << ((float) total_hop) / request_num << endl;//计算请求信号的平均条数
}


TrafficGenerator::~TrafficGenerator() {
    //delete 160 times//for 2k 20k: overall signal num is 178417 for all. If set 0, first is 178417, then159's are zeros
    //if (id==0)
    // overall_signal_num=0;
    // TODO Auto-generated destructor stub
}

/* namespace std */
