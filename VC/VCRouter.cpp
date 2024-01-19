/*
 * Router.cpp
 *
 *  Created on: 2019年8月19日
 *      Author: wr
 */

#include "VC/VCRouter.hpp"
#include "parameters.hpp"
#include <iostream>

extern float cycles;

VCRouter::VCRouter(int *t_id, int in_out_port_num, VCNetwork *t_vcNetwork,
                   int t_vn_num, int t_vc_per_vn, int t_vc_priority_per_vn,
                   int t_in_depth) { //构造函数
    //write file add
//    VNRouterActiveVC.open(
//            "../RecordFiles/VCRouter_Status/" + std::to_string(id[0]) + std::to_string(id[1]) + "VNRouterActiveVC.txt",
//            ios::app);// cmake in sub direc. so add ../ ios::app to add content without clearing
    //write file end

    // Router position in two-dimension site
    id[0] = t_id[0]; // x-axis 这里的id是以坐标形式输入的
    id[1] = t_id[1]; // y-axis

    vcNetwork = t_vcNetwork; //没用到
//预先分配 rinport 和 routport的空间，存储的是只想rinport的指针
    in_port_list_inRouter.reserve(
            in_out_port_num); // yz reserve is standard operation of vector. different from resize: Not really create this element until pushback//4 + NI_num[i]=5
    out_port_list_inRouter.reserve(in_out_port_num);

    for (int i = 0; i < in_out_port_num; i++) {
        // Establish IN_PORT component
        Link *link = new Link((RInPort *) NULL);//创建一个LINK ，并将以个空指针进行赋值
        extern int global_rInPort_ID; //全局rinport的id
        RInPort *t_rInPort = new RInPort(i, t_vn_num, t_vc_per_vn,
                                         t_vc_priority_per_vn, t_in_depth, this,
                                         link, global_rInPort_ID);
        global_rInPort_ID++;//每次创建一个就自增
        link->rInPort = t_rInPort;//将新创建的用于更新link,in link 是他自己
        in_port_list_inRouter.push_back(t_rInPort);//存到该router的list中

        // Establish OUT_PORT component
        // ROutPort * t_rOutPort = new ROutPort(i, 1, 1, 0, INFINITE);//parameter_NI_myRouterOutputPortBufferSize
        ROutPort *t_rOutPort = new ROutPort(
                i, 1, 1, 0, parameter_NI_myRouterOutputPortBufferSize);
        out_port_list_inRouter.push_back(t_rOutPort);// id 为 i，表示第几个输出端口 ，只有一个vn,同时每个Vn只有一个vc,即只有一个flitbuffer
        //且该flitbuffer的大小为100 ，很大可以理解为无穷
        //这个out port list存储了 in out port num个 routport，其每个routport只有一个vc即一个flit buffer 为bufferlist【0】
    }
    rr_port = 0;
    router_port_num = in_out_port_num;
    port_total_utilization = 0;
}

// For each head/head_tail flit coming from the in_link, do routing;
int VCRouter::getRoute(Flit *t_flit) {//对每一个 head flit
    int x = t_flit->packet->destination[0];
    int y = t_flit->packet->destination[1];
    int z = t_flit->packet->destination[2];
    if (y < id[1]) { // turn left若packet的dest<router 的y坐标 ，则向左转
        return 3;
    } else if (y > id[1]) { // turn right ，若dest》router则向右转
        return 1;
    } else { // y direction
        if (x < id[0]) { // turn up 若x坐标 dest《router，向上 传递 ，左上角为 0
            return 0;
        } else if (x > id[0]) { // turn down 若 dest》packet 则向下传
            return 2;
        } else //x和y都与router坐标相等则，可认为已经发送到目的地的
            // arrival
            return (z + 4); // 0->y+(up); 1->x+(right); 2->y-(down); 3->x-(left); 4/4+-> controller
    } //因此 return为 z+4 ,如果送达 则 该数据 需要进入 计算单元 ，z为可以访问 ni的端口数 ，
    assert(1 == 0);
    return -1;
}

void VCRouter::vcRequest() { // for each vc in each port which is in state v(2), do vc request
    /*
     std::vector<int>::iterator iter;
     for(iter=priority_vc.begin(); iter<priority_vc.end();){
     *///对VCrouter的每一个in port 都调用vc-request去分配对应输出端口的vc
    //rr_port的引入保证了 公平性
    for (int i = 0;
         i < router_port_num; i++) {                                                              // port round robin
        in_port_list_inRouter[(i + rr_port) %
                              router_port_num]->vc_request(); // select the port after RR to send request//go through all, but order is determinded by rr. rr->rr+1->rr+2...rr+5
    }
}

void//没用
VCRouter::vcRequest_routerSW_priority_responsePacket() { // for each vc in each port which is in state v(2), do vc request
    for (int i = 0; i <
                    router_port_num; i++) {                                                                                                      // port round robin
        in_port_list_inRouter[(i + rr_port) %
                              router_port_num]->vc_request_inPort_routerSW_priority_responsePacket(); // select the port after RR to send request//go through all, but order is determinded by rr. rr->rr+1->rr+2...rr+5
    }
}

void VCRouter::writeFile_VNRouterActiveVC() {//没用
#ifdef  ofstreamSW_VNRouterActiveVC
    int allActiveVCNum = 0;
    for (int port_i = 0; port_i < router_port_num; port_i++) {
        for (int vc_i = 0; vc_i < in_port_list_inRouter[0]->vn_num * (in_port_list_inRouter[0]->vc_per_vn +
                                                                      in_port_list_inRouter[0]->vc_priority_per_vn); vc_i++) {
            if (in_port_list_inRouter[port_i]->state[vc_i] = 2) {
                allActiveVCNum++;
                //cout<<"router writefiles"<<endl;
            }
        }
    }

    VNRouterActiveVC << id[0] << id[1] << " id " << cycles << " cycles " << allActiveVCNum << endl;
#endif
}

void VCRouter::getSwitch() {
    for (int i = 0; i < router_port_num; i++) { // port round robin
        in_port_list_inRouter[(i + rr_port) % router_port_num]->getSwitch(); //同理每一个端口都调用getswitch ，对之前已经匹配好的vc进行flit传送
    }
    rr_port = (rr_port + 1) % router_port_num; // port_num is  5 according to  cout// next rr_port change
    // cout<<"port_num"<<port_num<<endl;
    // cout<<"rr_port"<<rr_port<<endl;//rr_port=many 0,1,3,4,5
}
//没用
void VCRouter::getSwitch_routerSW_priority_responsePacket() {
    for (int i = 0; i < router_port_num; i++) { // port round robin
        in_port_list_inRouter[(i + rr_port) % router_port_num]->getSwitch_inPort_routerSW_priority_responsePacket();
        /*
         for(int ii=2*2; ii<2*(2+2); ii++){
         int tag = (ii+0)%(2*2)+2*2;// for example,  (o to 1 plus 4) -> (4 or 5)
         if(in_port_list_inRouter[(ii+rr_port)%port_num]->buffer_list[tag]->cur_flit_num > 0 && in_port_list_inRouter[(ii+rr_port)%port_num]->state[tag] == 3 && in_port_list_inRouter[(ii+rr_port)%port_num]->buffer_list[tag]->read()->sched_time < cycles){// state ==3 to be state==13
         Flit* flit = in_port_list_inRouter[(ii+rr_port)%port_num]->buffer_list[tag]->read();
         if (flit->packet->packet_ID ==parameter_UniqueID_trackThisSignalLife & flit->type==0 )
         {
         cout<<"router id [2]  "<<id[0]<<id[1]<<endl;
         }
         }
         }
         */
    }
    rr_port = (rr_port + 1) % router_port_num; // router_port_num is  5 according to  cout// next rr_port change
}

void VCRouter::outPortDequeue() { //遍历所有 port
    for (int i = 0; i < router_port_num; i++) { // port round robin
        if (out_port_list_inRouter[i]->buffer_list[0]->cur_flit_num != 0 &&
            out_port_list_inRouter[i]->buffer_list[0]->read()->sched_time < cycles) {
            //一个输出端口本身就是不包含vc的，一个输出端口就只有一个filt buffer，但每个输出端口都有连接的下一个router的输入端口其存在多个vc
            Flit *flit = out_port_list_inRouter[i]->buffer_list[0]->dequeue();
            // if( out_port_list_inRouter[i]->out_link->rInPort->buffer_list[flit->vc]->isFull())
            // cout<< "router dequeue" << endl;
            //cout << "cycles" << cycles << " router " << id[0] << " " << id[1]
            //     << " outportdeque " << i << " test"
            //     << out_port_list_inRouter[i]->buffer_list[0]->used_credit << endl; //
            out_port_list_inRouter[i]->out_link->rInPort->buffer_list[flit->vc]->enqueue( //这里连接的rinport 可以是ni阿
            //这里的 filt vc 更新成 out_vc 了
                    flit);//某个输出端口连接到了另一个router的输入端口，将flit存入该vc
            flit->sched_time = cycles + LINK_TIME - 1; //传送完之后更改调度时间，即加上了在网络中传输的时间
            port_total_utilization++; //一个router 在 完成从 某个输出端口 到 与其连接的输入端口进行 flit传送的时候 则 自增
            if (flit->type == 0 || flit->type == 10) { //若是头flit，创建下一个连接端口对应的 VC router
                VCRouter *vcRouter =
                        dynamic_cast<VCRouter *>(out_port_list_inRouter[i]->out_link->rInPort->router_owner);
                if (vcRouter != NULL) {        // conect to router //else connect to NI
#ifdef SHARED_VC // for QoS USING shared VC
                    if (flit->packet->signal->QoS == 1)
                    {
                      out_port_list_inRouter[i]->out_link->rInPort->priority_vc.push_back(flit->vc);
                      out_port_list_inRouter[i]->out_link->rInPort->priority_switch.push_back(flit->vc);
                    }

#endif
//若创建成功则记录该flit下一条的结果
                    int route_result = vcRouter->getRoute(flit);
                    out_port_list_inRouter[i]->out_link->rInPort->out_port[flit->vc] =//然后将下一跳的结果更新回到 out port连接的rinport的数组outport中去
                            route_result; // 0/1/2/3/z+4，out——port[]存储的是每个vc进行下一条要用的out port,分别为上下左右以及到达
                    assert(
                            out_port_list_inRouter[i]->out_link->rInPort->state[flit->vc] == 1);
                }
//还没开始发送置1 表示已经预定了
//发送完后置2 表示其已经接受到值 在寻找outport了
                out_port_list_inRouter[i]->out_link->rInPort->state[flit->vc] = 2; // wait for vc allocation
            }
        }
    }
}

// To run Routing, VC_allocation, Switching
void VCRouter::runOneStep() {//实际上完成了这样一个过程，flit根据下一条的位置找到了outport，
    // 继续找与其link的下一个router的输出端口 同时根觉flit的vnet找到第一个 空的Vc,将flit存入outport的filtbuffer里，再将其存入
    //inport的buffer里，若是headflit在根据flit的dest进行routing,寻找其进入到第二个router里在进行下一条的方向储存起来，并改变inport vc state
#ifdef ofstreamSW_VNRouterActiveVC
  writeFile_VNRouterActiveVC();
#endif
#ifdef routerSW_priority_responsePacket
    vcRequest_routerSW_priority_responsePacket();
    getSwitch_routerSW_priority_responsePacket();
#else
    vcRequest(); //找出一个router 所有 inport的 所有满足state=2的 vc 并为其 分配 outport 与其 连接的  inport
    getSwitch();//对 一个 router的 所有 inport的 ，每个端口只有第一个满足条件的 vc 可以穿一个flit 到 对应的 outport buffer里
#endif
    outPortDequeue();//一个router的 所有 outport ，在满足条件的情况下每个outport 的 flit buffer （因为只有一个）都可以传递 一个 flit 去下一个 inport

}

VCRouter::~VCRouter() {
#ifdef ofstreamSW_VNRouterActiveVC
    VNRouterActiveVC.close();
#endif
    RInPort *inPort;
    while (in_port_list_inRouter.size() != 0) {
        inPort = in_port_list_inRouter.back(); // yz vector.back Returns a reference to the last element in the vector. Unlike member vector::end, which returns an iterator just past this element, this function returns a direct reference.
        in_port_list_inRouter.pop_back();      // yz popback Removes the last element in the vector, effectively reducing the container size by one.
        delete inPort;
    }

    ROutPort *outPort;
    while (out_port_list_inRouter.size() != 0) {
        outPort = out_port_list_inRouter.back();
        out_port_list_inRouter.pop_back();
        delete outPort;
    }
}
