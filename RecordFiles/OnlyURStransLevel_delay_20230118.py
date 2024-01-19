# %%
import os

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

# %%
# %%
txtFileName_URS = "60000_2025_epoch_0_URS_packetdelay"  # note: to change the file name!
URS_readcsv = pd.read_csv(txtFileName_URS + ".txt", sep='\t', index_col=False,
                          names=['packet_id', 'head_time', 'tail_time', 'length', 'signal_sourceid', 'packet_destid',
                                 'packet_type', 'signal_ID', 'head_iniTime', 'tail_arriveTime', 'sigTranIniTime',
                                 'respSigIniTime', 'signalGoToMem'])  # ,delimiter=' '
print("read URS done", txtFileName_URS)

# %%
URS_readcsv_nonzero = URS_readcsv.loc[(URS_readcsv != 0).any(axis=1)]
# %%
URS_readcsv_nonzero_sortSigID = URS_readcsv_nonzero.sort_values(by=['signal_ID', 'head_iniTime'], ignore_index=True)
# %%
sum = 0
sum_list = []
sum_time = 0
strange_num_urs = 0 \
    # for i in range(0,URS_readcsv_nonzero_sortSigID=URS_readcsv_nonzero. ,2):
bias = 0
transIni_tailFLitArrive_value = 0
transIni_tailFLitArrive_value_list = []
for i in range(0, URS_readcsv_nonzero_sortSigID.shape[0], 2):
    sum_time = sum_time + 1
    if (URS_readcsv_nonzero_sortSigID['signal_ID'][i + bias] != URS_readcsv_nonzero_sortSigID['signal_ID'][
        i + bias + 1]):
        print(i + bias, ' id not equal once')
        bias = bias + 1
    while (URS_readcsv_nonzero_sortSigID['signal_ID'][i + bias] != URS_readcsv_nonzero_sortSigID['signal_ID'][
        i + bias + 1]):
        print(i + bias, ' id still not equal')
        bias = bias + 1
    # URS_readcsv_nonzero_sortSigID['head_iniTime'][i+bias]
    if ((i + bias) > (URS_readcsv_nonzero_sortSigID.shape[0] - 3)):  # here need further attention to avoid overflow
        break
    value = URS_readcsv_nonzero_sortSigID['tail_arriveTime'][i + bias + 1] - \
            URS_readcsv_nonzero_sortSigID['head_iniTime'][i + bias]
    if (value < 0):
        value = URS_readcsv_nonzero_sortSigID['tail_arriveTime'][i + bias] - \
                URS_readcsv_nonzero_sortSigID['head_iniTime'][i + bias + 1]
        sum_list.append(value)
        transIni_tailFLitArrive_value_list.append((URS_readcsv_nonzero_sortSigID['tail_arriveTime'][i + bias] -
                                                   URS_readcsv_nonzero_sortSigID['sigTranIniTime'][i + bias + 1]))
    elif (value < 0):
        print('why <0')
    else:
        sum_list.append(value)
        transIni_tailFLitArrive_value_list.append((URS_readcsv_nonzero_sortSigID['tail_arriveTime'][i + bias + 1] -
                                                   URS_readcsv_nonzero_sortSigID['sigTranIniTime'][i + bias]))
    if (value > 2000):
        strange_num_urs = strange_num_urs + 1
    # value=0
    sum = sum + (value)

max_urs = np.max(sum_list)
print('max_urs travel latency: req head flit ini -  resp  tail flit arrive',
      max_urs)  # not counting waitting in MasterNI
npavg_urs = np.average(sum_list)
print('avg_urs:travel req head flit ini - resp tail flit arrive', npavg_urs)
avg_urs = sum / sum_time
# %%
max_transIni_tailFLitArrive_value = np.max(transIni_tailFLitArrive_value_list)
print('max_urs ini latency: req sig ini -  resp  tail flit arrive', max_transIni_tailFLitArrive_value)
npavg_transIni_tailFLitArrive_value = np.average(transIni_tailFLitArrive_value_list)
print('avg_urs: ini latency req sig ini - resp tail flit arrive', npavg_transIni_tailFLitArrive_value)
# %%
print('max_urs travel latency is ', max_urs, 'npavg_urs travel latency is', npavg_urs,
      '  max_transIni_tailFLitArrive_value ', max_transIni_tailFLitArrive_value,
      'npavg_transIni_tailFLitArrive_value is ', npavg_transIni_tailFLitArrive_value)
############################################################
# %%
debug_2waydelay = []
for i in range(0, URS_readcsv_nonzero_sortSigID.shape[0], 2):
    avoidbug = 0
    # debug_2waydelay.append((URS_readcsv_nonzero_sortSigID['tail_arriveTime'][i + bias+1] -URS_readcsv_nonzero_sortSigID['sigTranIniTime'][i + bias ]))
    # URS_readcsv_nonzero_sortSigID['sigTranIniTime'][i + 1] = ((URS_readcsv_nonzero_sortSigID['tail_arriveTime'][i + bias+1] -URS_readcsv_nonzero_sortSigID['sigTranIniTime'][i + bias ]))
debug_2waydelay = pd.DataFrame(debug_2waydelay)
# %%
twoWayDelay_fiveSegments = []
bias = 0
for i_ite in range(1, URS_readcsv_nonzero_sortSigID.shape[0], 1):  # avoid 0 and 0-1
    if ((i_ite + 1) % 2 == 0):  # i is resp , i-1 is req
        if (URS_readcsv_nonzero_sortSigID['signal_ID'][i_ite + bias] != URS_readcsv_nonzero_sortSigID['signal_ID'][
            i_ite + bias - 1]):
            print('\n', bias, ' bias because not equal at ', URS_readcsv_nonzero_sortSigID['signal_ID'][i_ite], '\n')
            bias = bias + 1
        i = i_ite + bias
        packetID = URS_readcsv_nonzero_sortSigID['signal_ID'][i]
        overallDelay = URS_readcsv_nonzero_sortSigID['tail_arriveTime'][i] - \
                       URS_readcsv_nonzero_sortSigID['sigTranIniTime'][i - 1]  # thi trans. sig tra
        reqWaitTime = URS_readcsv_nonzero_sortSigID['head_iniTime'][i - 1] - \
                      URS_readcsv_nonzero_sortSigID['sigTranIniTime'][i - 1]
        reqTravelTime = URS_readcsv_nonzero_sortSigID['tail_arriveTime'][i - 1] - \
                        URS_readcsv_nonzero_sortSigID['head_iniTime'][i - 1]
        respWaitTime = URS_readcsv_nonzero_sortSigID['head_iniTime'][i] - \
                       URS_readcsv_nonzero_sortSigID['respSigIniTime'][i]
        respTravelTime = URS_readcsv_nonzero_sortSigID['tail_arriveTime'][i] - \
                         URS_readcsv_nonzero_sortSigID['head_iniTime'][i]
        reqSignalSourceID =  URS_readcsv_nonzero_sortSigID['signal_sourceid'][i-1]
        reqSignalDestID = URS_readcsv_nonzero_sortSigID['packet_destid'][i - 1]
        twoWayDelay_fiveSegments.append(
            (packetID, overallDelay, reqWaitTime, reqTravelTime, respWaitTime, respTravelTime,reqSignalSourceID,reqSignalDestID))
    else:
        twoWayDelay_fiveSegments.append((0, 0, 0, 0, 0, 0,0,0))
DFtwoWayDelay_fiveSegments = pd.DataFrame(twoWayDelay_fiveSegments,
                                          columns=['sigID', 'overall', 'req wait', 'req travel', 'resp wait',
                                                   'resp travel','signal_sourceid',"reqSignalDestID" ])
DFtwoWayDelay_fiveSegmentsNonZero = DFtwoWayDelay_fiveSegments.loc[(DFtwoWayDelay_fiveSegments != 0).any(axis=1)]
# %%取出不为0的行的 overall这个参数
print('recevied resp sig count', DFtwoWayDelay_fiveSegmentsNonZero.shape[0])
print('df max 111', DFtwoWayDelay_fiveSegmentsNonZero.max())
print('df mean 222', DFtwoWayDelay_fiveSegmentsNonZero.mean())

# %%
goToMemTrans = []
notgoToMemTrans = []
bias = 0
for i_ite in range(1, URS_readcsv_nonzero_sortSigID.shape[0], 1):  # avoid 0 and 0-1
    if ((i_ite + 1) % 2 == 0):  # i is resp , i-1 is req
        if (URS_readcsv_nonzero_sortSigID['signal_ID'][i_ite + bias] != URS_readcsv_nonzero_sortSigID['signal_ID'][
            i_ite + bias - 1]):
            print('\n', bias, ' bias because not equal at ', URS_readcsv_nonzero_sortSigID['signal_ID'][i_ite], '\n')
            bias = bias + 1
        i = i_ite + bias
        if (URS_readcsv_nonzero_sortSigID['signalGoToMem'][i] ==1):
            packetID = URS_readcsv_nonzero_sortSigID['signal_ID'][i]
            overallDelay = URS_readcsv_nonzero_sortSigID['tail_arriveTime'][i] - \
                           URS_readcsv_nonzero_sortSigID['sigTranIniTime'][i - 1]  # thi trans. sig tra
            reqWaitTime = URS_readcsv_nonzero_sortSigID['head_iniTime'][i - 1] - \
                          URS_readcsv_nonzero_sortSigID['sigTranIniTime'][i - 1]
            reqTravelTime = URS_readcsv_nonzero_sortSigID['tail_arriveTime'][i - 1] - \
                            URS_readcsv_nonzero_sortSigID['head_iniTime'][i - 1]
            respWaitTime = URS_readcsv_nonzero_sortSigID['head_iniTime'][i] - \
                           URS_readcsv_nonzero_sortSigID['respSigIniTime'][i]
            respTravelTime = URS_readcsv_nonzero_sortSigID['tail_arriveTime'][i] - \
                             URS_readcsv_nonzero_sortSigID['head_iniTime'][i]
            goToMemTrans.append(
                (packetID, overallDelay, reqWaitTime, reqTravelTime, respWaitTime, respTravelTime))
        if (URS_readcsv_nonzero_sortSigID['signalGoToMem'][i] == -1):
            packetID = URS_readcsv_nonzero_sortSigID['signal_ID'][i]
            overallDelay = URS_readcsv_nonzero_sortSigID['tail_arriveTime'][i] - \
                           URS_readcsv_nonzero_sortSigID['sigTranIniTime'][i - 1]  # thi trans. sig tra
            reqWaitTime = URS_readcsv_nonzero_sortSigID['head_iniTime'][i - 1] - \
                          URS_readcsv_nonzero_sortSigID['sigTranIniTime'][i - 1]
            reqTravelTime = URS_readcsv_nonzero_sortSigID['tail_arriveTime'][i - 1] - \
                            URS_readcsv_nonzero_sortSigID['head_iniTime'][i - 1]
            respWaitTime = URS_readcsv_nonzero_sortSigID['head_iniTime'][i] - \
                           URS_readcsv_nonzero_sortSigID['respSigIniTime'][i]
            respTravelTime = URS_readcsv_nonzero_sortSigID['tail_arriveTime'][i] - \
                             URS_readcsv_nonzero_sortSigID['head_iniTime'][i]
            notgoToMemTrans.append(
                (packetID, overallDelay, reqWaitTime, reqTravelTime, respWaitTime, respTravelTime))
DFgoToMemTrans = pd.DataFrame(goToMemTrans,
                                          columns=['sigID', 'overall', 'req wait', 'req travel', 'resp wait',
                                                   'resp travel'])
DFnotgoToMemTrans = pd.DataFrame(notgoToMemTrans,
                                          columns=['sigID', 'overall', 'req wait', 'req travel', 'resp wait',
                                                   'resp travel'])
#%%
print("DFgoToMemTransDFgoToMemTransDFgoToMemTransDFgoToMemTrans")
print('recevied resp sig count', DFgoToMemTrans.shape[0])
print('df max', DFgoToMemTrans.max())
print('df mean', DFgoToMemTrans.mean())
print("DFnotgoToMemTransDFnotgoToMemTransDFnotgoToMemTrans ")
print('recevied resp sig count', DFnotgoToMemTrans.shape[0])
print('df max', DFnotgoToMemTrans.max())
print('df mean', DFnotgoToMemTrans.mean())

#%%

injSignalHigh = []
bias = 0
for i_ite in range(1, URS_readcsv_nonzero_sortSigID.shape[0], 1):  # avoid 0 and 0-1
    if ((i_ite + 1) % 2 == 0):  # i is resp , i-1 is req
        if (URS_readcsv_nonzero_sortSigID['signal_ID'][i_ite + bias] != URS_readcsv_nonzero_sortSigID['signal_ID'][
            i_ite + bias - 1]):
            print('\n', bias, ' bias because not equal at ', URS_readcsv_nonzero_sortSigID['signal_ID'][i_ite], '\n')
            bias = bias + 1
        i = i_ite + bias



#%%

txtFileName_URS2 = "60000_2025_epoch_1_URS_packetdelay"  # note: to change the file name!
URS_readcsv2 = pd.read_csv(txtFileName_URS2 + ".txt", sep='\t', index_col=False,
                          names=['packet_id', 'head_time', 'tail_time', 'length', 'signal_sourceid', 'packet_destid',
                                 'packet_type', 'signal_ID', 'head_iniTime', 'tail_arriveTime', 'sigTranIniTime',
                                 'respSigIniTime', 'signalGoToMem'])  # ,delimiter=' '
print("read URS done", txtFileName_URS2)

# %%
URS_readcsv_nonzero2 = URS_readcsv2.loc[(URS_readcsv2 != 0).any(axis=1)]
# %%
URS_readcsv_nonzero_sortSigID2 = URS_readcsv_nonzero2.sort_values(by=['signal_ID', 'head_iniTime'], ignore_index=True)
# %%
twoWayDelay_fiveSegments2 = []
bias = 0
for i_ite in range(1, URS_readcsv_nonzero_sortSigID2.shape[0], 1):  # avoid 0 and 0-1
    if ((i_ite + 1) % 2 == 0):  # i is resp , i-1 is req
        if (URS_readcsv_nonzero_sortSigID2['signal_ID'][i_ite + bias] != URS_readcsv_nonzero_sortSigID2['signal_ID'][
            i_ite + bias - 1]):
            print('\n', bias, ' bias because not equal at ', URS_readcsv_nonzero_sortSigID2['signal_ID'][i_ite], '\n')
            bias = bias + 1
        i = i_ite + bias
        packetID = URS_readcsv_nonzero_sortSigID2['signal_ID'][i]
        overallDelay = URS_readcsv_nonzero_sortSigID2['tail_arriveTime'][i] - \
                       URS_readcsv_nonzero_sortSigID2['sigTranIniTime'][i - 1]  # thi trans. sig tra
        reqWaitTime = URS_readcsv_nonzero_sortSigID2['head_iniTime'][i - 1] - \
                      URS_readcsv_nonzero_sortSigID2['sigTranIniTime'][i - 1]
        reqTravelTime = URS_readcsv_nonzero_sortSigID2['tail_arriveTime'][i - 1] - \
                        URS_readcsv_nonzero_sortSigID2['head_iniTime'][i - 1]
        respWaitTime = URS_readcsv_nonzero_sortSigID2['head_iniTime'][i] - \
                       URS_readcsv_nonzero_sortSigID2['respSigIniTime'][i]
        respTravelTime = URS_readcsv_nonzero_sortSigID2['tail_arriveTime'][i] - \
                         URS_readcsv_nonzero_sortSigID2['head_iniTime'][i]
        reqSignalSourceID =  URS_readcsv_nonzero_sortSigID2['signal_sourceid'][i-1]
        reqSignalDestID = URS_readcsv_nonzero_sortSigID2['packet_destid'][i - 1]
        twoWayDelay_fiveSegments2.append(
            (packetID, overallDelay, reqWaitTime, reqTravelTime, respWaitTime, respTravelTime,reqSignalSourceID,reqSignalDestID))
    else:
        twoWayDelay_fiveSegments2.append((0, 0, 0, 0, 0, 0,0,0))
DFtwoWayDelay_fiveSegments2 = pd.DataFrame(twoWayDelay_fiveSegments2,
                                          columns=['sigID', 'overall', 'req wait', 'req travel', 'resp wait',
                                                   'resp travel','signal_sourceid',"reqSignalDestID" ])
DFtwoWayDelay_fiveSegmentsNonZero2 = DFtwoWayDelay_fiveSegments2.loc[(DFtwoWayDelay_fiveSegments2 != 0).any(axis=1)]
#%%

#取出不为0的行
plt.hist(DFtwoWayDelay_fiveSegmentsNonZero['overall'],bins=300,label= 'Without admssion control')
plt.hist(DFtwoWayDelay_fiveSegmentsNonZero2['overall'],bins=300,label= 'With  admission control')
#plt.xlim(0,600)
plt.legend()
#plt.title("delay hist of every end-to-end transaction  ")
plt.show()
plt.hist(DFtwoWayDelay_fiveSegmentsNonZero['overall'],bins=300,label= 'Without admission control')
plt.hist(DFtwoWayDelay_fiveSegmentsNonZero2['overall'],bins=300,label= 'With  admission control')
plt.xlim(0,600)
plt.legend()
#plt.title("delay hist of every end-to-end transaction (truncated in 600)")
plt.show()

