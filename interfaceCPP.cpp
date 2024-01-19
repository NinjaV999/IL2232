#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include "interfaceCPP.hpp"

//using namespace std;
//定义一个reset（） 函数,必须用getstate 来阻塞writeAction在py里 ，同时 在cpp里 必须用getAction来阻塞 writeState
//否则若writeAction没被阻塞 ，这就会导致 writeSstate也每阻塞，，可能导致state还没读完 ，下一个state就已经开始写入了
//反之若完成 堵塞 ，即writeAction 必须等待 getState 进行完毕，在这个进行过程在中 由于 writeAction被阻塞
//cpp中getAction等不到数据，陷入无限循环，此时writeState 也不会执行 ，也在不断等待中，保证了读写的时间要求

void getAction(float result[3])
{   float result1[3]={0};
    int actionReady = 0;
    int actionType = 0;
    int actionData = 0;
    int resetReady=0;
    std::ifstream  actionfile,resetfile;
    std::ofstream  actionfile2;

    while (1) {
        actionfile.open("actionReady.txt", std::ios::in);//读取数据
        if (actionfile.is_open()) {
            actionfile >> actionReady;
            actionfile.close();
        }
        else
        {
            std::cout << "actionReady.txt can not open 1" << std::endl;
        }


        resetfile.open("resetReady.txt", std::ios::in);//读取数据
        if (resetfile.is_open()) {
            resetfile >> resetReady;
            resetfile.close();
        }
        else
        {
            std::cout << "resetReady.txt can not open " << std::endl;
        }

        if (actionReady == 1) {//wang cheng action write zhi zhou tiaochuxunhuan1
            actionfile2.open("actionReady.txt", std::ios::out | std::ios::trunc);
            if (actionfile2.is_open()) {

                actionfile2 << 0;
                actionfile2.close();
            }
            else {
                std::cout << "actionReady.txt can not open 2" << std::endl;
            }
            std::cout << "Action read " << std::endl;
            //printf("actionRead== %d \n", actionReady);
            actionfile.open("actionData.txt", std::ios::in);
            if (actionfile.is_open()) {
                std::string line;
                std::getline(actionfile, line);

                // 使用字符串流处理
                std::istringstream iss(line);

                char discard; // 用于丢弃括号
                iss >> discard; // 读取左括号

                iss>> result[0] >> result[1]>>result[2];
                actionfile.close();
                iss >> discard;
                //result[1]=result[0];
               // result[2]=result[0];
                //std::cout<<result[0]<<" "<<result[1]<<" "<<result[2]<<std::endl;
            }
            else
            {
                std::cout << "actionData.txt can not open" <<std:: endl;
            }

            /*if (actionType == 0) {
                actionData = 1;//两种动作
            }
            else if (actionType == 1) {
                actionData = 30;
            }*/



            break;
        }
        else if(resetReady == 1 ){
            std::cout<<"reset"<<std::endl;
            break;
        }


        else {
            ;
        }
    }

    return ;
}

void writeState(float state[4], float cycle)

{
    std::ofstream statefile1;
    statefile1.open("stateData.txt",  std::ios::trunc | std:: ios::out);
    /*state[0]=state[0]/300;
    state[1]=state[1]/1000;
    state[2]=state[2]/100;
    state[3]=state[3]/1000;*/
    if (statefile1.is_open()) {

        for (int i = 0; i <= 3; i++)
        {statefile1 << state[i]<<" ";
        }
        statefile1.close();
        std::cout << "stateData write" << std::endl;
    }
    else { std::cout << "stateData.txt can not open" << std::endl; }

    statefile1.open("cycleData.txt", std::ios::trunc | std::ios::out);
    if (statefile1.is_open()) {
        statefile1 << cycle;
        statefile1.close();
        std::cout << "cycleData write" << std::endl;
    }
    else { std::cout << "cycleData.txt can not open" << std::endl; }

//将所有有用信号写完之后 就是 state ready
    statefile1.open("stateReady.txt",  std::ios::trunc | std::ios::out);
    if (statefile1.is_open()) {
        statefile1 << 1;
        statefile1.close();
        //std::cout << "stateReady has been writen" << std::endl;
    }
    else { std::cout << "stateReady.txt can not open" << std::endl;}
    return ;


}

void createTXT()
{
    std::ofstream outfile1("actionData.txt");
    outfile1.close();
    std::ofstream outfile2("actionReady.txt");
    outfile2.close();
    std::ofstream outfile3("stateData.txt");
    outfile3.close();
    std::ofstream outfile4("stateReady.txt");
    outfile4.close();
    std::ofstream outfile5("cycleData.txt");
    outfile5.close();
    std::ofstream outfile6("resetReady.txt");
    outfile6.close();
    return ;
}

void reset (){
    std::ofstream statefile1;
    statefile1.open("stateData.txt",  std::ios::trunc | std:: ios::out);
    double state[4]={2.1,2.2,2.3,2.4};
    double cycle=0;

    if (statefile1.is_open()) {//存储初始化state
        for (int i = 0; i <= 3; i++)
        {statefile1 << state[i]<<" ";
        }
        statefile1.close();
        std::cout << "stateData write" << std::endl;
    }
    else { std::cout << "stateData.txt can not open" << std::endl; }

    statefile1.open("cycleData.txt", std::ios::trunc | std::ios::out);
    if (statefile1.is_open()) {//存储初始化cycle
        statefile1 << cycle;
        statefile1.close();
        std::cout << "cycleData write" << std::endl;
    }
    else { std::cout << "cycleData.txt can not open" << std::endl; }


    statefile1.open("stateReady.txt",  std::ios::trunc | std::ios::out);
    if (statefile1.is_open()) {//准备好数据后将信号stateReady zhi 1
        statefile1 << 1;
        statefile1.close();
        //std::cout << "stateReady has been writen" << std::endl;
    }
    else { std::cout << "stateReady.txt can not open" << std::endl;}
    return ;


}





/*int main() {
    std::cout << "begin" << std::endl;

//createTXT();
    srand((unsigned) time(NULL));
    double state[4];
    std::ifstream resetfile;
    std::ofstream resetfile1;
    int resetReady=0,resetReady1=0;
    int Action=0;

    while(1)
    {   resetfile.open("resetReady.txt", std::ios::in);//读取数据
        if (resetfile.is_open()) {
            resetfile >> resetReady;
            resetfile.close();
        }
        else
        {
            std::cout << "resetReady.txt can not open 1" << std::endl;
        }
        if (resetReady==1){//当resetready为1时，先把该信号zhi 0 再写数据，此时 py应该先执行getState，在执行action
            //writeAction 会被阻塞，也就是说state不读完 ，不可能触发下次写state，同理有由于getAction 会阻塞writeState 若Action不读完也不会触发下次写
            resetfile1.open("resetReady.txt", std::ios::out | std::ios::trunc);
            if (resetfile1.is_open()) {
                resetfile1 << 0;

                resetfile1.close();
            }
            else {
                std::cout << "resetReady.txt can not open 2" << std::endl;
            }
            //开始写数据
            reset();
            //Action = getAction();
            printf("reset 启动 \n");
        }


        Action = getAction();  //目前存在的问题在于 最后一次循环 ，将next state写入后 ，触发Done 一个episode 结束
        //此时我们将不再进行下一个step函数 而是调用reset 函数 ，同时cpp这边也陷入循环 ，但是由于cpp和py读写速率不太一致，这就导致
        //resetReady 已经完成判断 后，才写入resetReady =1，此时reset 并没有运行反而被action block了下一个循环 ，我们需要做的是跳出 action（）
        //并进行下一次循环。break和continue联合实现
        resetfile.open("resetReady.txt", std::ios::in);//读取数据
        if (resetfile.is_open()) {
            resetfile >> resetReady1;
            resetfile.close();
        }
        else
        {
            std::cout << "resetReady.txt can not open 1" << std::endl;
        }
        if (resetReady1==1){
            printf("we need continue \n");
            continue;
        }


        printf("Action is %d \n", Action);
        //  std::cout<<"write state"<<std::endl;
        for (int i = 0; i < 4; i++) {

            state[i] = rand() % 3 + 2;

        }
        printf("state： %lf，%lf,%lf,%lf \n", state[0], state[1], state[2], state[3]);
        double cycle = rand() % 4 + 3;
        writeState(state, cycle);
        //std::cout<<"wait action"<<std::endl;

        std::cout << std::endl;
    }

    return 0;}*/


