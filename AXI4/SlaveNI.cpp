/*
 * SlaveNI.cpp
 *
 *  Created on: 2019年9月9日
 *      Author: wr
 */

#include <AXI4/SlaveNI.hpp>
#include <cstddef>

int SlaveNI::count = 0;

SlaveNI::SlaveNI(int t_id, int t_slave_num, int t_master_num, TNetwork *t_network, VCNetwork *vc_network, int t_router_num_x,VCNetwork *vcNetwork, std::vector<BasicNI*> t_main_basicNI_list, int *t_NI_num) : BasicNI(t_id, t_slave_num, t_master_num, t_network, vc_network, t_router_num_x, t_NI_num)
{
  slaveNI_receivedRequestMessageCredit=0; //定义 用于接受slave NI  中用于接受 req msg的credit
  slaveNI_ToSendresponseMessageCredit=0;//定义 slaveNI 中 用于接受resp credit NI
    slaveNI_slavePort = new SlavePort(master_num, this);
	//
	 basicNI_list_inVCSlaveNI=t_main_basicNI_list;
#ifdef ofstreamSW_SlaveNIMessageResp
    SlaveNIMessageResp.open(
            "../RecordFiles/NoC_Status/AXI4NI_Status/SlaveNIMem/" + std::to_string(id) + "MasterPortMessageOut.txt",ios::app);// cmake in sub direc. so add ../ ios::app to add content without clearing
#endif
    memProcessReqCapacity = 10;
    //
}

// send to a buffer, waiting for response
void SlaveNI::enqueue(Message *message)
{	slaveNI_receivedRequestMessageCredit++;
	message_request.push_back(message); //slaveNI 接受到请求 信号 ，存入 buffer ，credit ++
}

Message *SlaveNI::dequeueMessage()
{	  //yz
  //basicNI->VC_network->NI_list[basicNI->id]->
  basicNI_packetWaitNum=message_response.size();//slave NI's wait num
 // cout<<"slaveNI basicNI_packetWaitNum/message_response "<<basicNI_packetWaitNum<<endl;
	if (message_response.size() > 0)
	{
		/*
		// need a  loop here// before send response,

		for(std::deque <Message>::iterator iter=message_response.begin();iter!=message_response.end();++iter)
		  {
		    if (NI_list[iter->signal->destination])// if response destination full, push_front
		  }
		*/
		Message *response = message_response.front();
		if (response->out_cycle_inMessage < cycles)  //如果响应队列 不为 0 ，且 情形信号已经到达
		{
			message_response.pop_front();//则第一个信号出对列且 将要输出的 resp msg --
			slaveNI_ToSendresponseMessageCredit--;
			return response;
		}
	}
	return (Message *)NULL; //否则 输出一个控制指针
}

// simulate off-chip memory system
void SlaveNI::response()
{
	if (message_request.size() > 0)//msg req 队列 不为空
	{
		Message *request = message_request.front();// 指向队列头
		if (request->out_cycle_inMessage < cycles)//如果 到达时间小于 全局时间
		{
			// Packet* packet = (Packet*) request;
			// std::cout << "handle packet at cycles: " << cycles << ", packet length:" << packet->length << std::endl;
			message_request.pop_front();//取出该队列
			slaveNI_receivedRequestMessageCredit--;// 受到的req msg credit --
			AXI4Signal *AXI4_request = request->signal; //用一个sig存储 其队列对应的sig
			AXI4Signal *AXI4_response = new AXI4Signal(AXI4_request->idInSignal_trans, AXI4_request->type + 1, AXI4_request->source_id, AXI4_request->data_length, AXI4_request->QoS, AXI4_request->signal_id);
			//生成对应的resp sig
            // add
			AXI4_response->source_id = AXI4_request->destination;
            AXI4_response->signal_trans_createcycles = AXI4_request->signal_trans_createcycles;// slaveNI's response
            AXI4_response->respSigIniTime = cycles;// 20230306,add resp ini time
            AXI4_response->signalGoToMem = AXI4_request->signalGoToMem;
            global_respSignalNum ++; //全局响应信号数量+1
            // add end

			Message *response = new Message(id, AXI4_response, cycles + OFF_CHIP_MEMORY_DELAY); // off-chip memory response delay
            //cout<<response->signal->respSigIniTime<<"slave ni response->signal->respSigIniTime"<<endl;
            //生成该响应信号对应的 resp msg
			response->slave_id = request->slave_id;
			response->sequence_id = request->sequence_id;
			delete request;
			delete AXI4_request;
			message_response.push_back(response);//存入 message response ，将要发出的msg credit++
			slaveNI_ToSendresponseMessageCredit++;//yz add20230113
			//cout<<cycles<<" cycles"<<" slaveNI_ToSendresponseMessageCredit"<<slaveNI_ToSendresponseMessageCredit<<endl;
			count++;
			if (printfSW_AXI4SlaveNI_SlaveReceived == 1)
			{
				cout << "S_Slave Received " << count << endl;
			}
		}
	}
}

void SlaveNI::runOneStep()
{// memory node (slave NI ) not in generator.cpp
    //从 VC的 某个buffer中取出 一个msg 并将其存入 msg request 队列中
    slaveNI_slavePort->record_refer();//1.message = basicNI->VC_network->NI_list[basicNI->id]->packet_buffer_out[0].front();  2.basicNI->enqueue request message into message_request, message outcycles change
	//由msg request 队列 取出 第一个msg 生成对应的 resp msg
    response();// message_request to message_response
    slaveNI_slavePort->to_NoC();// 1.slaveNIdeque message=message_response.front();2. Packet *packet = new Packet(message, basicNI->router_num_x, basicNI->NI_num, global_Packet_ID, slaveport_Pakcet_ID, -199, -99);
	//在 某个vc的 buffer中 存入该packet 并转化成 filts

    //3. packetBuffer_list[VN] enqueue this packet // 4. In VC NI, check the packetBuffer_list[VN] and Flitize  the packet to flits
#ifdef SlaveNIRun_writerFile
    writeFile_SlaveNIMessageResp();
#endif
}

//message_response
void SlaveNI::writeFile_SlaveNIMessageResp()
{
#ifdef ofstreamSW_SlaveNIMessageResp
    SlaveNIMessageResp<<id<<" id " <<cycles<<" cycles "<<message_response.size()  << endl;
#endif
}
SlaveNI::~SlaveNI()
{
#ifdef ofstreamSW_SlaveNIMessageResp
    SlaveNIMessageResp.close();
#endif
	delete slaveNI_slavePort;
}
