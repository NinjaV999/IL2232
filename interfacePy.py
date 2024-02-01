import  numpy as np
def getState():
    #print("begin")
    while(1):#开始读取数据

        statefile=open("./cmake-build-debug/stateReady.txt",'r')
        stateReady=statefile.read()
        statefile.close()
        #print(stateReady)

        #print(stateReady,type(stateReady))
        if stateReady=='1':
            statefile3=open("./cmake-build-debug/stateReady.txt",'w+')
            statefile3.write('0')
            #print("stateReady=0")
            statefile3.close()
            #print(stateReady,type(stateReady))
            #print(111)
            #print(stateReady)
            statefile2=open("./cmake-build-debug/stateData.txt",'r')
            stateData=statefile2.readline()
            statefile2.close()
            # print(stateData)
            statefile2=open("./cmake-build-debug/cycleData.txt",'r')
            cycleData=statefile2.read()
            statefile2.close()


            #print("State read")
            break
        else:
            pass



    return stateData,cycleData


def writeAction(actionType):
    actionfile=open("./cmake-build-debug/actionData.txt",'w+')
    actionfile.write(str(actionType))
    actionfile.close()
    actionfile1=open("./cmake-build-debug/actionReady.txt",'w+')
    actionfile1.write('1')
    actionfile1.close()
    #print("action write" )


def creatTxT():
    file1=open("./cmake-build-debug/actionData.txt",'w+')
    pass
    file1.close()

    file2=open("./cmake-build-debug/actionReady.txt",'w+')
    pass
    file2.close()

    file3=open("./cmake-build-debug/stateData.txt",'w+')
    pass
    file3.close()

    file4=open("./cmake-build-debug/cycleData.txt",'w+')
    pass
    file4.close()

    file5=open("./cmake-build-debug/stateReady.txt",'w+')
    pass
    file5.close()

    file6=open("./cmake-build-debug/resetReady.txt",'w+')
    pass
    file6.close()

    file7=open("./cmake-build-debug/finish.txt",'w+')
    file7.write('0')
    file7.close()






#for i in range (0,4):
    #print("wait state ")
  #  stateData,rewardData=getState()
   # stateDatalist=[float (stateData) for stateData in stateData.split(' ') if stateData]
    #rewardDatalist=float(rewardData  )

   # state=np.array(stateDatalist)
    #reward=np.array(rewardDatalist)
    #print(state,reward)
    #print("Action selection")
    #k=i%2
    #print("action type is %d " % k)
    #writeAction(k)
    #print("\n")
    #print("\n")
