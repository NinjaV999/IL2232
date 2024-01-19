/*
 * TrafficController.hpp
 *
 *  Created on: Oct 25, 2019
 *      Author: wr
 */

#ifndef TRAFFICCONTROLLER_HPP_
#define TRAFFICCONTROLLER_HPP_

#include "TrafficGenerator.hpp"
#include "AXI4/BasicNI.hpp"
#include "AXI4/MasterNI.hpp"
#include <vector>
#include <stdlib.h>
#include <assert.h>

class TrafficGenerator;



class TrafficController
{
public:

  /* @brief    tTrafficController set nilist to be input and generate generator_list
   *  loop i->168 call TrafficGenerator:
   *  	id=i, read_ratio=0.2
   *  	create destination_list with destinatiolist single: destination_list_single.push_back(id);destination_list_single.push_back(destination);
   */
  TrafficController (std::vector<BasicNI*> NI_list);//trafficController 的参数是一个数组 ，其中每一个元素都是 BasicNI 对象的执行很


  std::vector<TrafficGenerator*> generator_list;// 一个存储generator的数组

  std::vector<BasicNI*> NI_list; //

  /* @brief    temp->inject(temp_NI)
   *
   */
  void run();

  /* @brief  temp->create_traffic_num()
  for(iter=generator_list.begin(); iter<generator_list.end(); iter++){
        TrafficGenerator* temp = (*iter);
        temp->create_traffic_num();
in traffic generator.cpp
   void TrafficGenerator::create_traffic_num(){
  for(int i=0; i<5; i++){
      traffic_mode[i] = traffic_num[count][i];
  }
  //yz count is a value of parser input
  */
  void create_generator_traffic_num();



  /*
   * @brief    cout average hop
   */
  void average_hop();



  /*
   * @brief    cout << temp->injectCount_perInterval[i] << endl; for several times
   * for(iter=generator_list.begin(); iter<generator_list.end(); iter++){
   * some if condition
   */
  void injection_distribution();



/*
 * @brief  repeat 'generator_list' times: cout temp->LCS_per_node/temp->GRS_per_node/ temp->URS_per_node
 */
  void LCS_URS_GRS_per_node();

//yz added
    void  changeGenRate(int t_alpha2InControl);
//

  ~TrafficController ();
};


#endif /* TRAFFICCONTROLLER_HPP_ */
