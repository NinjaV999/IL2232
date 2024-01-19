/*
 * TrafficController.cpp
 *
 *  Created on: Oct 25, 2019
 *      Author: wr
 */

#include <TrafficController.hpp>


TrafficController::TrafficController(std::vector<BasicNI *> t_NI_list) {
    //srand( 10);//add because find that the sigNum changed under different Network configuration
    int count = 0; //创建traffic generator，排除 其中8个 nodes ，count 是用来记录traffic genrator的个数的
    for (int i = 0; i < 168; i++) {//i是0-167 中间排除部分数 ，count是 0-159
        if (i != 31 && i != 38 && i != 45 && i != 52 && i != 115 && i != 122 && i != 129 &&
            i != 136) {//8 nodes are off-chip memorycontroller
            TrafficGenerator *generator = new TrafficGenerator(i, 0.2, 128,
                                                               count);//id, read ratio,average_data_length,count

#ifdef veryLargeInjectionRateNode0
            if(i==0 || i == 28){
                generator->beta2=93;
                generator->alpha2=7;//25;
                generator->base2=100;//100;
                cout<<" node 0  based2=100 generator->alpha2 " <<generator->alpha2<< endl;
            }
#endif
#ifdef onlyFirstSubArea
            if ( /* upper 6 rows*/ ((i / 14) <= 5) && (  /* left 7 columns*/ (i % 14) <= 6)) {
                //do nothing

            } else {
                generator->alpha2 = 0;
                generator->refAlpha2 =0;
            }
#endif
            generator_list.push_back(generator); //将generator 存入 生成器中
            count++;
            //cout<<count<<"\n";//yz cout will be 1-160
        }
    }
    NI_list = t_NI_list;
}

void TrafficController::changeGenRate(int t_alpha2InControl) {
    std::vector<TrafficGenerator *>::iterator iter;
    int id_temp = 0;
    for (iter = generator_list.begin(); iter < generator_list.end(); iter++) {
        TrafficGenerator *temp = (*iter);
        id_temp = id_temp + 1;
        if (temp->alpha2 == 0) {  //一开始alpha2的100 beta2=1000 ，state0 -1 1/11 state 0-0 10/11
            //state 1-0 10/11 state 1-1 是 1/11 所以 发生请求的概率很小
            // 发生 alpha2的 概率增大 ，则发生请求的概率增大 ，或者说请求增多
            temp->alpha2 = 0;
        } else {
            //if(((temp->id / 14) <= 5) && (  /* left 7 columns*/ (temp->id % 14) <= 6))
            if((temp->id   == 17) || (temp->id  == 24) || ((temp->id  == 101))  || (temp->id  == 108))
//            if((temp->id   == 0) || (temp->id  == 7 ) || ((temp->id  == 84))  || (temp->id  == 91))
            {
              temp->alpha2 = t_alpha2InControl; //在热点处赋予新的值
            //cout << t_alpha2InControl<<endl;
            }

        }
        //temp->alpha2 = 10;//t_alpha2InControl;//* temp->alpha2;//temp->refAlpha2;//0;
        //  cout << " temp->alpha2 " << id_temp << " " << temp->alpha2 << "  " << temp->refAlpha2 << endl;
    }
}

void TrafficController::run() {
    // find active node;
    std::vector<TrafficGenerator *>::iterator iter;
    //iter理解为指向 向量元素的指针 ，*iter  获取 这个 所指的对象
//对这个向量生成一个迭代器， 即对每一个 traffic generator 都操作以下
    for (iter = generator_list.begin(); iter < generator_list.end(); iter++) {
        TrafficGenerator *temp = (*iter);// temp指针 指向一个 traffic generator 类相的对象
        if (temp->request()) {//request信号为true 时 ，即 当前generator 要发出请求
            MasterNI *temp_NI = (MasterNI *) NI_list[temp->id]; //生成一个 temp_ni的指针指向一个 masterNI类型的对象 ，这个对象是一个存basicNI的
             //的 vector里取了 ，且 索引 与 traffic generator 的 id相同
            assert(temp_NI != NULL);
            temp->inject(temp_NI); //生成请求sig 并存入对应的 masterNI
        }
    }
}

void TrafficController::average_hop() { //没用
    std::vector<TrafficGenerator *>::iterator iter; //构建迭代器
    for (iter = generator_list.begin(); iter < generator_list.end(); iter++) {
        TrafficGenerator *temp = (*iter);
        temp->average_hop(); //会调用average_hop函数 以 输出 average ——hop
    }
    cout << "id:" << endl;
    for (iter = generator_list.begin(); iter < generator_list.end(); iter++) {
        TrafficGenerator *temp = (*iter);
        cout << temp->id << endl;
    }
}

void TrafficController::injection_distribution() { //没用
    std::vector<TrafficGenerator *>::iterator iter;
    for (iter = generator_list.begin(); iter < generator_list.end(); iter++) {
        TrafficGenerator *temp = (*iter);
        if (temp->id == 3 || temp->id == 13 || temp->id == 18 || temp->id == 30) {// select 4 nodes to display
            cout << "interval ID" << temp->id << endl;
            for (int i = 0; i < 10; i++) {
                cout << temp->injectCount_perInterval[i] << endl;
            }
        }

    }
}

void TrafficController::create_generator_traffic_num() { //感觉没用
    std::vector<TrafficGenerator *>::iterator iter;
    for (iter = generator_list.begin(); iter < generator_list.end(); iter++) {
        TrafficGenerator *temp = (*iter);
        temp->create_traffic_num();
    }
}

void TrafficController::LCS_URS_GRS_per_node() { //没用
    std::vector<TrafficGenerator *>::iterator iter;
    cout << "LCS number per node:" << endl;
    for (iter = generator_list.begin(); iter < generator_list.end(); iter++) {
        TrafficGenerator *temp = (*iter);
        cout << temp->LCS_per_node << endl;
    }
    cout << "URS number per node:" << endl;
    for (iter = generator_list.begin(); iter < generator_list.end(); iter++) {
        TrafficGenerator *temp = (*iter);
        cout << temp->URS_per_node << endl;
    }
    cout << "GRS number per node:" << endl;
    for (iter = generator_list.begin(); iter < generator_list.end(); iter++) {
        TrafficGenerator *temp = (*iter);
        cout << temp->GRS_per_node << endl;
    }
}

TrafficController::~TrafficController() {
    std::vector<TrafficGenerator *>::iterator iter = generator_list.begin();
    for (int i = 0; i < 160; i++) {
        delete (*iter);
        iter++;
    }
}


