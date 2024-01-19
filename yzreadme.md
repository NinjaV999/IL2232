//below before 20230215	
cmake support:
cmake -DCMAKE_PREFIX_PATH=//home/yz/myprojects/2022/kth09/pytorchc++2023/libtorch/ ..
cmake --build . --config Release


# paper description
14*12 mesh ONE ROW 14 ONE COLUMN 12
MMP generator
1 flip 128 bit
2ghz global CLC.
500mhz NI CLC 
6 flip


# Overview 
* 0. NOT related to cycles
	* 1. New VCNetwork
	* 2. New TDM network
	* 3. NEW 160 master NI and 8 Slave NI, NI_list push back 168 NI
* 1. Every cycle run one step:
	* 1. NI_list: every NI run one step
		* 1. packetDequeue();
		* 2. vcAllocation();
		* 3. switchArbitration();
	 * 2. vcNetwork->runOneStep();
	 * 3. tNetwork->runOneStep();
* 2. Details in Ni_list one_step
	* 0. parameter
		* 1. state; // 0 I; 1 V; 2 A;
		* 2. packetBuffer_list: vector of buffer 
		* 3. packet_buffer_out: vector of packet 
	* 1. packetDequeue();
		* 1. out_port->out_link->rInPort->buffer_list[flit->vc]->enqueue(flit);
		* 2. out_port->out_link->rInPort->out_port[flit->vc] = route_result= vcRouter->getRoute(flit);
		* 3. out_port->out_link->rInPort->state[flit->vc] = 2; //wait for vc allocation
	* 2. vcAllocation();
		*  1. 
		*  2.
		*  3. 
	* 3. switchArbitration();
* 5. Flow of one signal
	* 1. Generator generate signal  generator: new AXI4Signal(trans_id, type, dest_id, data, QoS,overall_signal_num, cycles); EACH NI: traffic generator: generate signal (every cycle * prob).
	* 2. Push back  signal into AXI4_in_read or AXI4_write
		*  masterNI->converter_master->AXI4_in_read.push_back(signal) or masterNI->converter_master->AXI4_in_write.push_back(signal)
	 * 3. One step of NI:converter:
		 * 0. Code
			```cpp //ONE STEP: conver signal to be message: create new message, put this message in the back of  message out
				 converter_master->receive() // only for master side:new a axi4signal named response based on AXI4_out_read.front(),this cycle don't delete ; next cycle check the vector and delete.
				 masterPort->record();
			 	 masterPort->refer();//
			 	 // slave receive signal from assumed aximaster device and inject them into network
			 	 converter_slave->runOneStep();
			  	 converter_slave->response();
			 	 slavePort->record_refer();
			 	 slavePort->to_NoC();// new a packet in axi4/slaveport
				 ```
	 
		* 1. Converter::
			* 1. runOneStep():if(AXI4_in_read.size()>0) 
				- 1. new message with this signal
				- 2. message_out.push_back(message);
				- 3. receive():if(AXI4_out_read/write.size()>0)
				- 4. AXI4Signal* response = AXI4_out_read.front() or AXI4Signal* response = AXI4_out_write.front(); then delete response and count some numbers
			* 2. Then get message in message_out
		* 2. MasterPort: 2.1 new packet 2.2 pushback message tomasterNI' message_in 
			* 1. masterPort->record():if(masterNI->converter_master->message_out.size() > 0 
				*  0. Set message values
				* 1. {Message* message = masterNI->converter_master->message_out.front();
				* 3.  message->slave_id = dest; //int dest = message->signal->destination;
				* 4. message->sequence_id = seq;}
				* 5. inject this message to pakcet for VC (or TDM not shown below)
					if(message->signal->QoS != 2){ // to VC NoC
						  Packet* packet = new Packet(message, masterNI->router_num_x, masterNI->NI_num,global_Packet_ID,0,masterport_Pakcet_ID);//slaveport_Pakcet_ID=0 in this master port
						  delete message;
						  packet->signal->NI_arrival_time = cycles;
						  masterNI->VC_network->NI_list[masterNI->id]->packetBuffer_list[packet->vnet]->enqueue(packet);// 0-168's ni's buffer list for read network or for write network // packet buffer enqueue:  packet_queue.push_back(t_packet);packet_num++;
			 		     }
			* 2. masterPort->refer(): now packet in packet buffer then run refer
						//packet_buffer_out is vecotr of packet //In ni.cpp:1.packet_buffer_out.resize(2);2.packet_buffer_out[0?1(based on packet type request0  response1 )].push_back(packet);
					message = masterNI->VC_network->NI_list[masterNI->id]->packet_buffer_out[1].front();
					messageBuffer.push_back(message);
					Now message in messagebuffer of master port
					then masterNI->converter_master->message_in.push_back((*iter).second);//
		* 5.  converter: runOneStep(): if Message_in then 			


# Overview
0 INI 160 NI
1.EACH NI: traffic generator: generate signal (every cycle * prob).
2.converter ONE STEP: conver signal to be message: create new message, put this message in the back of  message out
3.master NI:converter ONE STEP,
	 converter_master->receive() // only for master side:new a axi4signal named response based on AXI4_out_read.front(),this cycle don't delete ; next cycle check the vector and delete.
	 masterPort->record();
 	 masterPort->refer();
 	 
 	 // slave receive signal from assumed aximaster device and inject them into network
 	 converter_slave->runOneStep();
  	 converter_slave->response();
 	 slavePort->record_refer();
 	 slavePort->to_NoC();// new a packet in axi4/slaveport
 
4. Swtich is done in Rinport.cpp
<<<<<<< HEAD
1.generator: new AXI4Signal(trans_id, type, dest_id, data, QoS,overall_signal_num, cycles);
2.Push back  signal into  masterNI->converter_master->AXI4_in_read.push_back(signal) or
			masterNI->converter_master->AXI4_in_write.push_back(signal)
3.Converter::	runOneStep():if(AXI4_in_read.size()>0)
			new message with this signal
	 		message_out.push_back(message);
	 	receive():if(AXI4_out_read/write.size()>0)
			AXI4Signal* response = AXI4_out_read.front() or AXI4Signal* response = AXI4_out_write.front(); then delete response and count some numbers
4.MasterPort::
		
		record():if(masterNI->converter_master->message_out.size() > 0 
	 		Set message values
	 		{Message* message = masterNI->converter_master->message_out.front();
	 		message->slave_id = dest; //int dest = message->signal->destination;
			message->sequence_id = seq;}
			inject this message to pakcet for VC (or TDM not shown below)
			if(message->signal->QoS != 2){ // to VC NoC
				  Packet* packet = new Packet(message, masterNI->router_num_x, masterNI->NI_num,global_Packet_ID,0,masterport_Pakcet_ID);//slaveport_Pakcet_ID=0 in this master port
				  delete message;
				  packet->signal->NI_arrival_time = cycles;
				  masterNI->VC_network->NI_list[masterNI->id]->packetBuffer_list[packet->vnet]->enqueue(packet);// 0-168's ni's buffer list for read network or for write network // packet buffer enqueue:  packet_queue.push_back(t_packet);packet_num++;
	 		     }
 		     
now packet in packet buffer Then masterPort->refer();
				// // packet_buffer_out is vecotr of packet //In ni.cpp:1.packet_buffer_out.resize(2);2.packet_buffer_out[0?1(based on packet type request0  response1 )].push_back(packet);
			messageBuffer.push_back(message);
			message = masterNI->VC_network->NI_list[masterNI->id]->packet_buffer_out[1].front();
			
 		
 		

# important notes
 AXI4Signal* signal = new AXI4Signal(trans_id, type, dest_id, data, QoS,overall_signal_num ,cycles);//type is read or write, length is data size,qos is packet qos type, 0 or 3 for lcs_urs, cycles is current cycle
   // QoS: 0->URS; 1->LCS (shared VCs with URS packets); 2->GRS; 3->LCS (individual VC(s) only for LCS packets) we do not support "1->shared VCs" mechanism any more.
 		 
Swtich is done in Rinport.cpp
round robin:  rr_buffer = (1+rr_buffer)%(vn_num*vc_per_vn);// rr + 1, the next buffer will have the priority to switch.
* 1. in router.cpp, do getSwtich to change the rr_port, in vc_request, only the in_port selected ( based on rr) send the vcrequest.then do outport deque
	* 1. RInPort::vc_request()	
# doygen use0
ubuntu 
doxygen -g (only use at the first time)  then edit doxyfile recursive to be 'yes' to scan sub-folders


cmd : doxygen  will generate it with default doxyfile and go to html folder and click index.html to read it.



## sequence id: given in masterport, 0-40,
  in axi4/ message.hpp int sequence_id; // -1; 4 bits  in axi4/ message
  in masterport.cpp     
  	      		int seq = master_list[id][dest];
  	      message->signal->sequence_id = seq;
    	  		master_list[id][dest] = (seq + 1) % S_TSHR_DEPTH;//S_TSHR_DEPTH=40
  	
