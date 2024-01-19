/*
 * Signal.cpp
 *
 *  Created on: 2019年9月9日
 *      Author: wr
 */

#include "TDM/Signal.hpp"

Signal::Signal(Message* message):Message(message){
  destination = message->signal->destination;
  int t_type = message->signal->type;
  int data_length = message->signal->data_length;
  switch (t_type){
    case 0: length = (105-1)/8+1;
            break;
    case 1: length = (18+data_length-1)/8+1;
            break;
    case 2: length = (105+data_length-1)/8+1;
            break;
    case 3: length = (18-1)/8+1;
            break;
  }
}

Signal::~Signal ()
{
  // TODO Auto-generated destructor stub
}

