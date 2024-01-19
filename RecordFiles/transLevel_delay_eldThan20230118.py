#%%
import os
import numpy as np
import pandas as pd
#%%
#%%
txtFileName_URS="50000_2_URS_packetdelay"
URS_readcsv=pd.read_csv(txtFileName_URS+".txt", sep='\t',index_col=False, names=['packet_id','head_time','tail_time','length','signal_sourceid','signal_destid','packet_destid','signal_ID','head_iniTime','tail_arriveTime'])#,delimiter=' '
print("read URS done")

#%%
URS_readcsv_nonzero=URS_readcsv.loc[(URS_readcsv != 0).any(1)]
#%%
URS_readcsv_nonzero_sortSigID=URS_readcsv_nonzero.sort_values(by=['signal_ID'],ignore_index=True)
#%%
sum=0
sum_list=[]
sum_time=0
strange_num_urs=0
#for i in range(0,URS_readcsv_nonzero_sortSigID=URS_readcsv_nonzero. ,2):

for i in range(0,URS_readcsv_nonzero_sortSigID.shape[0],2):
    sum_time=sum_time+1
    if (URS_readcsv_nonzero_sortSigID['signal_ID'][i]!=URS_readcsv_nonzero_sortSigID['signal_ID'][i+1]):
        print(i,' id not equal once')
        i=i+1
    if (URS_readcsv_nonzero_sortSigID['signal_ID'][i] != URS_readcsv_nonzero_sortSigID['signal_ID'][i + 1]):
        print(i, ' id ill not equal')
        i = i + 1
    URS_readcsv_nonzero_sortSigID['head_iniTime'][i]
    value=URS_readcsv_nonzero_sortSigID['tail_arriveTime'][i+1]-URS_readcsv_nonzero_sortSigID['head_iniTime'][i]
    if (value<0):
        value = URS_readcsv_nonzero_sortSigID['tail_arriveTime'][i] - URS_readcsv_nonzero_sortSigID['head_iniTime'][i+1]
        sum_list.append(value)
    elif (value < 0):
        print('why <0')
    else:
        sum_list.append(value)
    if (value > 1000):
        strange_num_urs=strange_num_urs+1
       # value=0
    sum=sum+(value)
max_urs=np.max(sum_list)
npavg_urs=np.average(sum_list)
avg_urs=sum/sum_time
#%%
############################################################
#############################################################
txtFileName_LCS="50000_2_URS_packetdelay"
LCS_readcsv=pd.read_csv(txtFileName_LCS+".txt", sep='\t',index_col=False, names=['packet_id','head_timeCost','tail_timeCost','length','signal_sourceid','signal_destid','packet_destid','signal_ID','head_iniTime','tail_arriveTime'])#,delimiter=' '
print("read LCS done")

#%%
LCS_readcsv_nonzero=LCS_readcsv.loc[(LCS_readcsv != 0).any(1)]
#%%
LCS_readcsv_nonzero_sortSigID=LCS_readcsv_nonzero.sort_values(by=['signal_ID'],ignore_index=True)
#%%
sum=0
sum_list_lcs=[]
sum_time=0
strange_num=0
for i in range(0,6872,2):
    sum_time=sum_time+1
    LCS_readcsv_nonzero_sortSigID['head_iniTime'][i]
    value=LCS_readcsv_nonzero_sortSigID['tail_arriveTime'][i+1]-LCS_readcsv_nonzero_sortSigID['head_iniTime'][i]
    if (value<0):
        value = LCS_readcsv_nonzero_sortSigID['tail_arriveTime'][i] - LCS_readcsv_nonzero_sortSigID['head_iniTime'][i+1]
        sum_list_lcs.append(value)
    elif (value < 0):
        print('why <0')
    else:
        sum_list_lcs.append(value)
    if (value > 1000):
        strange_num=strange_num+1
       # value=0
    sum=sum+(value)
max_ls=np.max(sum_list_lcs)
npavg_lcs=np.average(sum_list_lcs)

#
# #%%
# txtFileName_missingRequest="missingRequest"
# missingRequest_readcsv=pd.read_csv(txtFileName_missingRequest+".txt", sep='\t',index_col=False, names=['name','NIID','transName','trainsID','seqName','seqID','trainsNamefromSignal','trainsIDfromSignal','head_iniTime','tail_arriveTime'])#,delimiter=' '
# print("read txtFileName_missingRequest done")
# #%%
# missingRequest_readcsv_dublicatecheck=pd.DataFrame.duplicated(missingRequest_readcsv)
# #%%
# missingRequest_readcsv_uniq=missingRequest_readcsv.drop_duplicates()
#
# #%%