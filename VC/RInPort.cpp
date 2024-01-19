/*
 * Port.cpp
 *
 *  Created on: 2019年8月15日
 *      Author: wr
 */

#include "VC/RInPort.hpp"

extern float cycles;

RInPort::RInPort(int t_id, int t_vn_num, int t_vc_per_vn, int t_vc_priority_per_vn, int t_depth, NRBase * t_owner, Link * t_in_link,int t_global_rInPort_ID)
        :Port(t_id, t_vn_num, t_vc_per_vn, t_vc_priority_per_vn, t_depth)
{
    for(int i=0; i<vn_num*(vc_per_vn+vc_priority_per_vn); i++){
        state.push_back(0); //开始都为idle// 3个整数性向量 ，分别存入 0 -1 -1 ，从存入 vn_num*(vc_per_vn+vc_priority_per_vn)个值 ，其代表的是每个端口的flit buffer数量
        out_port.push_back(-1);// 存储 输入端口 到 输出端口 下一跳的方向 上下左右 ni
        out_vc.push_back(-1);//-1 则 找不到 对应的flit buffer
    }
    rr_record = 0;
    rr_priority_record = 0;
    router_owner = t_owner;
    in_link = t_in_link;
    count_vc = 0;
    count_switch = 0;
    starvation = 0;
    rInPort_ID=t_global_rInPort_ID;//全局rinport的數量
    //cout<<" rInPort_ID"<< rInPort_ID<<std::endl; For example, overall 1008 rinport=168*6
}


int RInPort::vc_allocate(Flit* t_flit){ //vitual channel 的 分配
    int vn = t_flit->vnet; //這個 flit 對應的vnet

    for(int i=0; i<vc_per_vn; i++){ // 對該Vn下的每個vc 進行判斷 如果idle ，則切換為等待 routing ，並返回 該 vc的id，若沒有
        // 則返回-1
        if(state[vn*vc_per_vn+i] == 0){ //idle
            state[vn*vc_per_vn+i] = 1;  //wait for routing
            return vn*vc_per_vn+i;  //vc id
        }
    }
    return -1; // no vc available
}

int RInPort::vc_allocate_normal(Flit* t_flit){ //沒用 應該還是 沒有 考慮 urs的 原因
    int vn = t_flit->vnet;

    for(int i=0; i<vc_per_vn-vc_priority_per_vn; i++){
        if(state[vn*vc_per_vn+i] == 0){ //idle
            state[vn*vc_per_vn+i] = 1;  //wait for routing
            return vn*vc_per_vn+i;  //vc id
        }
    }
    return -1; // no vc available
}

int RInPort::vc_allocate_priority(int vn_rank){//這個感覺是用來判斷 priority的 狀態的  vn_rank 應該是 0-vn net -1,没用
//我 QoS 恒为 0的 应该 没有pri
    for(int i=0; i<vc_priority_per_vn; i++){
        int tag = vn_num*vc_per_vn+vc_priority_per_vn*vn_rank+i;
        if(state[tag] == 0){
            state[tag] = 1;
            return tag;
        }
    }
    return -1;
}

void RInPort::vc_request_inPort_routerSW_priority_responsePacket(){ //没用
#ifdef  routerSW_priority_responsePacket
    // for priority packet (individual VCs) QoS = 3
   for(int i=vn_num*vc_per_vn; i<vn_num*(vc_per_vn+vc_priority_per_vn); i++){
       int tag;
        tag= (i+rr_priority_record)%(vn_num*vc_priority_per_vn)+vn_num*vc_per_vn;
       if(state[tag] == 2){// gothrough to see whethen case 1 happens, then  state to be 13
	   Flit* flit_PortFront = buffer_list[tag]->flit_queue.front();
	   if(flit_PortFront->packet->signal->type == 1 || flit_PortFront->packet->signal->type == 3){//if case 1 happens
	//       cout<<" rinport line 77"<<flit_PortFront->packet->signal->signal_id<<endl;
	       assert(flit_PortFront->type == 0 || flit_PortFront->type == 10);//tail?
	       VCRouter* vcRouter = dynamic_cast<VCRouter*>(router_owner);
	       assert(vcRouter != NULL); // only vc router will call this methed
	       int vn_rank = (tag-vn_num*vc_per_vn)/vc_priority_per_vn;
	       int vc_result = vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->vc_allocate_priority(vn_rank);//in connected inport find one tag whose state[tag]=0, set it to be 1
	       if (vc_result != -1){
		  state[tag] = 13; //set current tag_th vc active//3->13
		  out_vc[tag] = vc_result; //record the output vc in the streaming down node
	       }
	   }
       }

       tag = (i+rr_priority_record)%(vn_num*vc_priority_per_vn)+vn_num*vc_per_vn;
      // cout<<"rinport tag "<<tag<<endl;// one example: 2or 3
       if(state[tag] == 2){// if port' state[tag]=2, means (ready)flits in tag_th vc
       	  Flit* flit = buffer_list[tag]->flit_queue.front();
       	  assert(flit->type == 0 || flit->type == 10);
       	  VCRouter* vcRouter = dynamic_cast<VCRouter*>(router_owner);
       	  assert(vcRouter != NULL); // only vc router will call this methed
       	  int vn_rank = (tag-vn_num*vc_per_vn)/vc_priority_per_vn;
       	  int vc_result = vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->vc_allocate_priority(vn_rank);//in connected inport find one tag whose state[tag]=0, set it to be 1
       	  if (vc_result != -1){
       	      state[tag] = 3; //set current tag_th vc active
       	      out_vc[tag] = vc_result; //record the output vc in the streaming down node
       	  }
       }
   }

   // for URS packet, or let's say, for non priority VC.
  for(int i=0; i<vn_num*vc_per_vn; i++){
      int tag = (i+rr_record)%(vn_num*vc_per_vn);
      /*
       * below for state=13
       */
      if(state[tag] == 2){// case 1  state to be 13
      	   Flit* flit_PortFront = buffer_list[tag]->flit_queue.front();
      	   if(flit_PortFront->packet->signal->type == 1 || flit_PortFront->packet->signal->type == 3){//if case 1 happens
	       assert(flit_PortFront->type == 0 || flit_PortFront->type == 10);//tail?
	       VCRouter* vcRouter = dynamic_cast<VCRouter*>(router_owner);
	       assert(vcRouter != NULL); // only vc router will call this methed
    #ifdef SHARED_PRI
	      int vc_result = vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->vc_allocate_normal(flit_PortFront);
    #else
	      int vc_result = vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->vc_allocate(flit_PortFront);// VC RESULT to be  vc
    #endif
	       if (vc_result != -1){
		  state[tag] = 13; //set current tag_th vc active//3->13
		  out_vc[tag] = vc_result; //record the output vc in the streaming down node
	       }
      	   }
      }
      /*
       * below for set the stat which is state 2 to be state=3, if it is already 13 which means it !==2, it skip to be 3.
       */
      if(state[tag] == 2){
	  Flit* flit = buffer_list[tag]->flit_queue.front();
	  assert(flit->type == 0 || flit->type == 10);
	  VCRouter* vcRouter = dynamic_cast<VCRouter*>(router_owner);
	  assert(vcRouter != NULL); // only vc router will call this methed
#ifdef SHARED_PRI
	  int vc_result = vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->vc_allocate_normal(flit);
#else
	  int vc_result = vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->vc_allocate(flit);
#endif
	  if (vc_result != -1){
	      state[tag] = 3; //active
	      out_vc[tag] = vc_result; //record the output vc in the streaming down node
	  }
      }
  }



#endif
}
void RInPort::vc_request(){
    // cout<<"warning ifdef  routerSW_priority_responsePacket: inport 76 vc request not skip"<<endl;
    // for priority packet (shared VCs) QoS = 1
#ifdef  routerSW_priority_responsePacket
    cout<<"shoud  use the inport vcrequest with priority packetID"<<endl;
#endif

#ifdef SHARED_VC
    std::vector<int>::iterator iter;
   for(iter=priority_vc.begin(); iter<priority_vc.end();){
       if(count_vc == STARVATION_LIMIT)
 	  break;
       int tag = (*iter);
       if(state[tag] == 2){
 	  Flit* flit = buffer_list[tag]->flit_queue.front();
 	  assert(flit->type == 0 || flit->type == 10);
 	  VCRouter* vcRouter = dynamic_cast<VCRouter*>(router_owner);
 	  assert(vcRouter != NULL); // only vc router will call this methed
 	  int vc_result = vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->vc_allocate(flit); //  int vc_allocate(Flit*);
 	  if (vc_result != -1){
 	      //vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->priority_vc.push_back(vc_result);
 	      //vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->priority_switch.push_back(vc_result);
 	      state[tag] = 3; //active
 	      out_vc[tag] = vc_result; //record the output vc in the streaming down node
 	      count_vc++;
 	      iter = priority_vc.erase(iter);
 	  }else
    	    iter++;
       }else
	 iter++;
   }
   count_vc = 0;
#endif

    // for priority packet (individual VCs) QoS = 3
    for(int i=vn_num*vc_per_vn; i<vn_num*(vc_per_vn+vc_priority_per_vn); i++){ //由于我们只有 URS 的 packet 因此 flit buffer 最多只会到 vn num*vcper vn -1
        int tag = (i+rr_priority_record)%(vn_num*vc_priority_per_vn)+vn_num*vc_per_vn;
        // cout<<"rinport tag "<<tag<<endl;// one example: 2or 3
        if(state[tag] == 2){
            Flit* flit = buffer_list[tag]->flit_queue.front();
            assert(flit->type == 0 || flit->type == 10);
            VCRouter* vcRouter = dynamic_cast<VCRouter*>(router_owner);//router owner指向的是其父类，转换NULLL
            assert(vcRouter != NULL); // only vc router will call this methed
            int vn_rank = (tag-vn_num*vc_per_vn)/vc_priority_per_vn;
            int vc_result = vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->vc_allocate_priority(vn_rank);
            if (vc_result != -1){
                state[tag] = 3; //active
                out_vc[tag] = vc_result; //record the output vc in the streaming down node
                // if (tag!=out_vc[tag])
                {
                    // 		cout<<"tag "<<tag<<" "<<"rinport out_vc[tag] "<<" "<<out_vc[tag]<<endl;
                }
                //  	cout<<"vn_rank "<<vn_rank<<" "<<"rinport out_vc[tag] "<<" "<<out_vc[tag]<<endl;// 0 or 1
                // 	cout<<"tag "<<tag<<" "<<"rinport out_vc[tag] "<<" "<<out_vc[tag]<<endl;// just 2/3
                //cout<<"rinport out_vc[0] "<<out_vc[0]<<endl;//
                //cout<<"rinport out_vc[1] "<<out_vc[1]<<endl;//
                //cout<<"rinport out_vc[2] "<<out_vc[2]<<endl;//
                //cout<<"rinport out_vc[3] "<<out_vc[3]<<endl;//
            }
        }
    }

    // for URS packet
    for(int i=0; i<vn_num*vc_per_vn; i++){ //遍历每一个 normal vc
        int tag = (i+rr_record)%(vn_num*vc_per_vn); // tag 为一个 【0，vn_num*vc_per_vn)的 值，
        if(state[tag] == 2){
            Flit* flit = buffer_list[tag]->flit_queue.front(); // buffer list 是在 父类 port 里定义的，其存储了 多个 flit buffer
            //取出对应先判断flit buffer 的状态 ，如果为 2 则 取出该filt buffer的第一个 flit
            assert(flit->type == 0 || flit->type == 10);//assert 判断失败就会终止程序 ， flit 要么是 head 要么是 head and tail
            //必须是head flit 找 vc
            VCRouter* vcRouter = dynamic_cast<VCRouter*>(router_owner);//将router owner转哈成 crout的指针
            assert(vcRouter != NULL); // only vc router will call this methed
#ifdef SHARED_PRI
            int vc_result = vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->vc_allocate_normal(flit);
#else
            int vc_result = vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->vc_allocate(flit);
            //寻找输出端口 连接的另一个router输入端口的第一个空的VC
            // vc result 是一个编号 ，其是刚刚取出的filt的 对应的 net的 首个等待 routing的 vc，outport【tag】表示vctag下一跳的方向 ，并找出该方向对应的输出端口
#endif
            if (vc_result != -1){ //如果有这样一个 等待routing的 vc
                // cout<<" rinport tag "<<tag<<endl;
                state[tag] = 3; //active 将 该 tag状态active，正在传出flit
                out_vc[tag] = vc_result; //record the output vc in the streaming down node
                //在下游节点中记录输出vc
            }
        }
    }

    // in case of no normal packet
#ifdef SHARED_VC

    for(; iter<priority_vc.end();){
       int tag = (*iter);
       if(state[tag] == 2){
 	  Flit* flit = buffer_list[tag]->flit_queue.front();
 	  assert(flit->type == 0 || flit->type == 10);
 	  VCRouter* vcRouter = dynamic_cast<VCRouter*>(router_owner);
 	  assert(vcRouter != NULL); // only vc router will call this methed
 	  int vc_result = vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->vc_allocate(flit);
 	  if (vc_result != -1){
 	      //vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->priority_vc.push_back(vc_result);
 	      //vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->priority_switch.push_back(vc_result);
 	      state[tag] = 3; //active
 	      out_vc[tag] = vc_result; //record the output vc in the streaming down node
 	      iter = priority_vc.erase(iter);
 	  }else
    	    iter++;
       }else
	 iter++;
   }
#endif

}



void RInPort::getSwitch(){

    // for priority packet
#ifdef SHARED_VC
    std::vector<int>::iterator iter;
  for(iter=priority_switch.begin(); iter<priority_switch.end();iter++){
      if(count_switch == STARVATION_LIMIT)
	  break;
      int tag = (*iter);
      if(buffer_list[tag]->cur_flit_num > 0 && state[tag] == 3 && buffer_list[tag]->read()->sched_time < cycles){
	  Flit* flit = buffer_list[tag]->read();//  return flit_queue.front();
	  flit->vc = out_vc[tag];
	  VCRouter* vcRouter = dynamic_cast<VCRouter*>(router_owner);//The dynamic_cast keyword casts a datum from one pointer or reference type to another, performing a runtime check to ensure the validity of the cast
	  assert(vcRouter != NULL); // only vc router will call this method
	  if(!vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->buffer_list[flit->vc]->isFull()){// when not full, go into the if below.// Wtihout !: full return 1.  With !: not full return 1.
	      buffer_list[tag]->dequeue();

	      //if(vcRouter->out_port_list_inRouter[out_port[tag]]->buffer_list[0]->isFull())
	     	    // cout<< "router switch" << endl;
	      vcRouter->out_port_list_inRouter[out_port[tag]]->buffer_list[0]->enqueue(flit);//flit_queue.push_back(t_flit); cur_flit_num++;
	      vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->buffer_list[flit->vc]->get_credit();
	      flit->sched_time = cycles;
	      count_switch++;
	      if(flit->type == 1 || flit->type == 10 ){// 0head 1 tail 10 heattail
		  state[tag] = 0; //idle
		  priority_switch.erase(iter);
	      }
	      return;
	  }
      }
  }
  count_switch = 0;
#endif

    // for LCS packet (individual VC)
    for(int i=vn_num*vc_per_vn; i<vn_num*(vc_per_vn+vc_priority_per_vn); i++){ //vc round robin; pick up non-empty buffer with state A (3)//For example, vnnum=2, vcpervn=2, vcprior=1 then i=4,5
        if(starvation == STARVATION_LIMIT){
            starvation = 0;
            break;
        }
        //cout<<"rr_priority_record"<<rr_priority_record<<endl;
        int tag = (i+rr_priority_record)%(vn_num*vc_priority_per_vn)+vn_num*vc_per_vn;// for example,  (o to 1 plus 4) -> (4 or 5)
        //cout<<"rinport tag "<<tag<<endl;//cout 4 or 5s
        if(buffer_list[tag]->cur_flit_num > 0 && state[tag] == 3 && buffer_list[tag]->read()->sched_time < cycles){
            Flit* flit = buffer_list[tag]->read();
            flit->vc = out_vc[tag];
            VCRouter* vcRouter = dynamic_cast<VCRouter*>(router_owner);
            assert(vcRouter != NULL); // only vc router will call this methed
            if(!vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->buffer_list[flit->vc]->isFull()){
                buffer_list[tag]->dequeue();
                vcRouter->out_port_list_inRouter[out_port[tag]]->buffer_list[0]->enqueue(flit);
                //add begun
                vcRouter->out_port_list_inRouter[out_port[tag]]->buffer_list[0]->get_credit();//yz 0112 added
                if (vcRouter->id[0] ==0 && vcRouter->id[1] == 0){
                    //  		  cout<<cycles<<" cycles "<<"vcRouter->out_port_list_inRouter[out_port[tag]]->buffer_list[0] get credit"<<vcRouter->out_port_list_inRouter[out_port[tag]]->buffer_list[0]->used_credit<<endl;
                    // cout<<"rinport id00 out_port[tag] "<< out_port[tag]<<" in cycles "<< cycles <<endl ;
                }
                //add end
                vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->buffer_list[flit->vc]->get_credit();//add here to avoid overflow in one cycle: many router fill in one almost full inport without notification

                flit->sched_time = cycles;
                if(flit->type == 1 || flit->type == 10 ){
                    state[tag] = 0; //idle
                }
                rr_priority_record = (rr_priority_record+1)%(vn_num*vc_priority_per_vn);// range is 0 to (vn*prio -1)
                starvation++;
                return;
            }
            else{
                //if(vcRouter->id[0]==8 && vcRouter->id[1])
                //cout<<"cycles"<<cycles<<" RInPort  vcRouterFull id[8][3]" <<out_port[tag]<<endl;
                //  cout<<"cycles"<<cycles<<" RInPort  vcRouterFull"<<"current out_port[tag] is "<<vcRouter->id[0] <<" "<< vcRouter->id[1]<<" " <<out_port[tag]<<endl;
            }
        }
    }



    // for normal packet
    //  for(int i=vn_num*vc_per_vn; i>0; i--){ //vc round robin; pick up non-empty buffer with state A (3)
    for(int i=0; i<vn_num*vc_per_vn; i++){ //vc round robin; pick up non-empty buffer with state A (3)
        //还是遍历每一个vc，这 保证了每次只处理一个vc，且循环顺序的改变 不会让其每次都值处理 0
        int tag = (i+rr_record)%(vn_num*vc_per_vn); //tag
        if(buffer_list[tag]->cur_flit_num > 0 && state[tag] == 3 && buffer_list[tag]->read()->sched_time < cycles){
           //当前vc的flit数 不为0,且 vc处于active 状态 ，且 vc的 第一个flit的调度时间sched time 《 当前 cycles
            Flit* flit = buffer_list[tag]->read();// 创建一个指针 指向 该 flit
            flit->vc = out_vc[tag]; //让该 flit 的vc变为 之前求得的空vc的id
            VCRouter* vcRouter = dynamic_cast<VCRouter*>(router_owner);
            assert(vcRouter != NULL); // only vc router will call this methed
            if(!vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->buffer_list[flit->vc]->isFull()){
                buffer_list[tag]->dequeue();
                vcRouter->out_port_list_inRouter[out_port[tag]]->buffer_list[0]->enqueue(flit);//可以理解为 buffer list 【0】 是一个公共缓冲区
                vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->buffer_list[flit->vc]->get_credit(); //目标 buffer 被使用的 credit ++
                //add begun
                vcRouter->out_port_list_inRouter[out_port[tag]]->buffer_list[0]->get_credit();//yz 0112 added // 公共缓冲区的被使用的credit ++
                if (vcRouter->id[0] ==0 && vcRouter->id[1] == 0){
                    //    		  cout<<cycles<<" cycles "<<"vcRouter->out_port_list_inRouter[out_port[tag]]->buffer_list[0] get credit"<<vcRouter->out_port_list_inRouter[out_port[tag]]->buffer_list[0]->used_credit<<endl;
                }
                //add end
                flit->sched_time = cycles;//将调度时间 赋值为当前水煎
                if(flit->type == 1 || flit->type == 10 ){
                    state[tag] = 0; //idle //如果这个flit 是tail flit 或者是 head tail flit 则 可以认为 原来的vc 空了
                }
                rr_record = (rr_record+1)%(vn_num*vc_per_vn);//rr_record 的 作用 每次都是从 tag =0 开始 遍历vc ,只要这个类实例 不被删除 rr——会一直累加
                return; // 由于 return的 存在 rr的自增只能执行一次 ，其保证了 0123，1230，2301这样的tag 取法 保证了公平
            }
        }
    }

    // in case of no normal packet
#ifdef SHARED_VC

    for(; iter<priority_switch.end();iter++){
      int tag = (*iter);
      if(buffer_list[tag]->cur_flit_num > 0 && state[tag] == 3 && buffer_list[tag]->read()->sched_time < cycles){
	  Flit* flit = buffer_list[tag]->read();
	  flit->vc = out_vc[tag];
	  VCRouter* vcRouter = dynamic_cast<VCRouter*>(router_owner);
	  assert(vcRouter != NULL); // only vc router will call this methed
	  if(!vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->buffer_list[flit->vc]->isFull()){
	      buffer_list[tag]->dequeue();

	      //if(vcRouter->out_port_list_inRouter[out_port[tag]]->buffer_list[0]->isFull())
	     	    // cout<< "router switch" << endl;
	      vcRouter->out_port_list_inRouter[out_port[tag]]->buffer_list[0]->enqueue(flit);
	      vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->buffer_list[flit->vc]->get_credit();
	      flit->sched_time = cycles;
	      if(flit->type == 1 || flit->type == 10 ){
		  state[tag] = 0; //idle
		  priority_switch.erase(iter);
	      }
	      return;
	  }
      }
  }
#endif

    // for LCS packet (individual VC) //repeat  becaue jump out of starvation
    for(int i=vn_num*vc_per_vn;i<vn_num*(vc_per_vn+vc_priority_per_vn); i++){ //vc round robin; pick up non-empty buffer with state A (3)
        int tag = (i+rr_priority_record)%(vn_num*vc_priority_per_vn)+vn_num*vc_per_vn;
        if(buffer_list[tag]->cur_flit_num > 0 && state[tag] == 3 && buffer_list[tag]->read()->sched_time < cycles){
            Flit* flit = buffer_list[tag]->read();
            flit->vc = out_vc[tag];
            VCRouter* vcRouter = dynamic_cast<VCRouter*>(router_owner);
            assert(vcRouter != NULL); // only vc router will call this methed
            if(!vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->buffer_list[flit->vc]->isFull()){
                buffer_list[tag]->dequeue();
                vcRouter->out_port_list_inRouter[out_port[tag]]->buffer_list[0]->enqueue(flit);
                //add begun
                vcRouter->out_port_list_inRouter[out_port[tag]]->buffer_list[0]->get_credit();//yz 0112 added
                if (vcRouter->id[0] ==0 && vcRouter->id[1] == 0){
                    //  cout<<cycles<<" cycles "<<"vcRouter->out_port_list_inRouter[out_port[tag]]->buffer_list[0] get credit"<<vcRouter->out_port_list_inRouter[out_port[tag]]->buffer_list[0]->used_credit<<endl;
                }
                //add end
                vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->buffer_list[flit->vc]->get_credit();
                flit->sched_time = cycles;
                if(flit->type == 1 || flit->type == 10 ){
                    state[tag] = 0; //idle
                }
                rr_priority_record = (rr_priority_record+1)%(vn_num*vc_priority_per_vn);
                return;
            }
        }
    }
}
void RInPort::getSwitch_inPort_routerSW_priority_responsePacket(){ //没用
    // for LCS packet (individual VC)
    for(int i=vn_num*vc_per_vn; i<vn_num*(vc_per_vn+vc_priority_per_vn); i++){ //vc round robin; pick up non-empty buffer with state A (3)//For example, vnnum=2, vcpervn=2, vcprior=1 then i=4,5
        if(starvation == STARVATION_LIMIT){
            starvation = 0;
            break;
        }
        int tag = (i+rr_priority_record)%(vn_num*vc_priority_per_vn)+vn_num*vc_per_vn;// for example,  (o to 1 plus 4) -> (4 or 5)
        if(buffer_list[tag]->cur_flit_num > 0 && state[tag] == 13 && buffer_list[tag]->read()->sched_time < cycles){// state ==3 to be state==13//yz add for priority resp.
#ifdef routerSW_priority_responsePacket
            Flit* flit = buffer_list[tag]->read();
	  flit->vc = out_vc[tag];
	  VCRouter* vcRouter = dynamic_cast<VCRouter*>(router_owner);
	  assert(vcRouter != NULL); // only vc router will call this methed
	  if(!vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->buffer_list[flit->vc]->isFull()){
	      buffer_list[tag]->dequeue();
	      vcRouter->out_port_list_inRouter[out_port[tag]]->buffer_list[0]->enqueue(flit);
	      //add begun
	      if (vcRouter->id[0] ==0 && vcRouter->id[1] == 0){
		// cout<<"rinport id00 out_port[tag] "<< out_port[tag]<<" in cycles "<< cycles <<endl ;
	      }
	      //add end
	      vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->buffer_list[flit->vc]->get_credit();
	      flit->sched_time = cycles;
	      if(flit->type == 1 || flit->type == 10 ){
		  state[tag] = 0; //idle
	      }
	      rr_priority_record = (rr_priority_record+1)%(vn_num*vc_priority_per_vn);// range is 0 to (vn*prio -1)
	      starvation++;
	      return;
	  }
#endif
        }
        else{
            if(buffer_list[tag]->cur_flit_num > 0 && state[tag] == 3 && buffer_list[tag]->read()->sched_time < cycles){// WBQ state ==3
                Flit* flit = buffer_list[tag]->read();
                flit->vc = out_vc[tag];
                VCRouter* vcRouter = dynamic_cast<VCRouter*>(router_owner);
                assert(vcRouter != NULL); // only vc router will call this methed
                if(!vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->buffer_list[flit->vc]->isFull()){
                    buffer_list[tag]->dequeue();
                    vcRouter->out_port_list_inRouter[out_port[tag]]->buffer_list[0]->enqueue(flit);
                    //vcRouter->out_port_list_inRouter[out_port[tag]]->buffer_list[0]->get_credit();//yz 0112 added
                    vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->buffer_list[flit->vc]->get_credit();
                    flit->sched_time = cycles;
                    if(flit->type == 1 || flit->type == 10 ){
                        state[tag] = 0; //idle
                    }
                    rr_priority_record = (rr_priority_record+1)%(vn_num*vc_priority_per_vn);// range is 0 to (vn*prio -1)
                    starvation++;
                    //add***********************************************
                    if (flit->packet->packet_ID ==parameter_UniqueID_trackThisSignalLife & flit->type==0 )
                    {
                        cout<<"getSwitch_inPort_routerSW_priority_responsePacket()  "<<cycles<<endl;
                    }
                    //add end ***********************************************
                    return;
                }
            }
        }
        // for normal packet
        for(int i=0; i<vn_num*vc_per_vn; i++){ //vc round robin; pick up non-empty buffer with state A (3)
            int tag = (i+rr_record)%(vn_num*vc_per_vn);
            if(buffer_list[tag]->cur_flit_num > 0 && state[tag] == 13 && buffer_list[tag]->read()->sched_time < cycles){
                Flit* flit = buffer_list[tag]->read();
                flit->vc = out_vc[tag];
                VCRouter* vcRouter = dynamic_cast<VCRouter*>(router_owner);
                assert(vcRouter != NULL); // only vc router will call this methed
                if(!vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->buffer_list[flit->vc]->isFull()){
                    buffer_list[tag]->dequeue();
                    vcRouter->out_port_list_inRouter[out_port[tag]]->buffer_list[0]->enqueue(flit);
                    vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->buffer_list[flit->vc]->get_credit();
                    flit->sched_time = cycles;
                    if(flit->type == 1 || flit->type == 10 ){
                        state[tag] = 0; //idle
                    }
                    rr_record = (rr_record+1)%(vn_num*vc_per_vn);
                    return;
                }
            }
            else {
                if(buffer_list[tag]->cur_flit_num > 0 && state[tag] == 3 && buffer_list[tag]->read()->sched_time < cycles){
                    Flit* flit = buffer_list[tag]->read();
                    flit->vc = out_vc[tag];
                    VCRouter* vcRouter = dynamic_cast<VCRouter*>(router_owner);
                    assert(vcRouter != NULL); // only vc router will call this methed
                    if(!vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->buffer_list[flit->vc]->isFull()){
                        buffer_list[tag]->dequeue();
                        vcRouter->out_port_list_inRouter[out_port[tag]]->buffer_list[0]->enqueue(flit);
                        vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->buffer_list[flit->vc]->get_credit();
                        flit->sched_time = cycles;
                        if(flit->type == 1 || flit->type == 10 ){
                            state[tag] = 0; //idle
                        }
                        rr_record = (rr_record+1)%(vn_num*vc_per_vn);
                        return;
                    }
                }
            }
        }
        // for LCS packet (individual VC)
        for(int i=vn_num*vc_per_vn;i<vn_num*(vc_per_vn+vc_priority_per_vn); i++){ //vc round robin; pick up non-empty buffer with state A (3)
            int tag = (i+rr_priority_record)%(vn_num*vc_priority_per_vn)+vn_num*vc_per_vn;
            if(buffer_list[tag]->cur_flit_num > 0 && state[tag] == 13 && buffer_list[tag]->read()->sched_time < cycles){
                Flit* flit = buffer_list[tag]->read();
                flit->vc = out_vc[tag];
                VCRouter* vcRouter = dynamic_cast<VCRouter*>(router_owner);
                assert(vcRouter != NULL); // only vc router will call this methed
                if(!vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->buffer_list[flit->vc]->isFull()){
                    buffer_list[tag]->dequeue();
                    vcRouter->out_port_list_inRouter[out_port[tag]]->buffer_list[0]->enqueue(flit);
                    vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->buffer_list[flit->vc]->get_credit();
                    flit->sched_time = cycles;
                    if(flit->type == 1 || flit->type == 10 ){
                        state[tag] = 0; //idle
                    }
                    rr_priority_record = (rr_priority_record+1)%(vn_num*vc_priority_per_vn);
                    return;
                }
            }
            else{
                if(buffer_list[tag]->cur_flit_num > 0 && state[tag] == 3 && buffer_list[tag]->read()->sched_time < cycles){
                    Flit* flit = buffer_list[tag]->read();
                    flit->vc = out_vc[tag];
                    VCRouter* vcRouter = dynamic_cast<VCRouter*>(router_owner);
                    assert(vcRouter != NULL); // only vc router will call this methed
                    if(!vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->buffer_list[flit->vc]->isFull()){
                        buffer_list[tag]->dequeue();
                        vcRouter->out_port_list_inRouter[out_port[tag]]->buffer_list[0]->enqueue(flit);
                        vcRouter->out_port_list_inRouter[out_port[tag]]->out_link->rInPort->buffer_list[flit->vc]->get_credit();
                        flit->sched_time = cycles;
                        if(flit->type == 1 || flit->type == 10 ){
                            state[tag] = 0; //idle
                        }
                        rr_priority_record = (rr_priority_record+1)%(vn_num*vc_priority_per_vn);
                        return;
                    }
                }
            }
        }
    }
}

RInPort::~RInPort(){

}





