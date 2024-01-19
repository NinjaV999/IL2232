#%%
import os
import numpy as np
import pandas as pd
#%%
#%%
txtFileName_URS="20000_10_URS_packetdelay"
txtFileName_LCS="20000_10_LCS_packetdelay"
URS_readcsv=pd.read_csv(txtFileName_URS+".txt", sep='\t',index_col=False, names=['packet_id','head_time','tail_time','length','signal_sourceid','signal_destid','packet_destid','signal_ID'])#,delimiter=' '
print("read URS done")
#%%
#URS_readcsv_NoAllZeros=URS_readcsv.loc[(URS_readcsv!=0).any(axis=1)]
#%%
gap_head_tail=[]
average_URS_delay=[]
URS_hop_list=[]
for i  in range(0,URS_readcsv.shape[0]):
   #print(URS_readcsv['tail_time'][i])
    if (URS_readcsv['tail_time'][i] != 0):
        gap_head_tail.append(URS_readcsv['tail_time'][i]-URS_readcsv['head_time'][i])
        average_URS_delay.append(URS_readcsv['tail_time'][i])
        URS_hop_list.append(abs(divmod(URS_readcsv['signal_destid'][i],14)[0]-divmod(URS_readcsv['signal_sourceid'][i],14)[0])+abs(divmod(URS_readcsv['signal_destid'][i],14)[1]-divmod(URS_readcsv['signal_sourceid'][i],14)[1]))
#%%
import matplotlib.pyplot as plt
plt.figure()
ax = plt.gca()
plt.hist(gap_head_tail,rwidth=0.8,density=True)
plt.xlabel('Delay/Cycle')
plt.ylabel('Density')
plt.text(0.6,0.8,'Average Tail-Head interval '+ str(np.mean(gap_head_tail)), horizontalalignment='center',
     verticalalignment='center',transform=ax.transAxes)
plt.text(0.6,0.75,'Number of URS Packet received:  '+ str(len(gap_head_tail)), horizontalalignment='center',
     verticalalignment='center',transform=ax.transAxes)
plt.text(0.6,0.70,'Average Packet delay:  '+ str(np.mean(average_URS_delay)), horizontalalignment='center',
     verticalalignment='center',transform=ax.transAxes)
plt.text(0.6, 0.66, 'Average hop of all URS packets received:  ' + str(np.mean(URS_hop_list)), horizontalalignment='center',
         verticalalignment='center', transform=ax.transAxes)

plt.title(str(os.getcwd()) + "/ "+txtFileName_URS)
plt.savefig(txtFileName_URS)
plt.show()

#%%
LCS_readcsv=pd.read_csv(txtFileName_LCS+".txt", sep='\t',index_col=False, names=['packet_id','head_time','tail_time','length','signal_sourceid','signal_destid','packet_destid','signal_ID'])#,delimiter=' '
print("read LCS done")
#%%
#URS_readcsv_NoAllZeros=URS_readcsv.loc[(URS_readcsv!=0).any(axis=1)]
#%%
gap_head_tail=[]
average_LCS_delay=[]
LCS_hop_list=[]
for i  in range(0,URS_readcsv.shape[0]):
   #print(URS_readcsv['tail_time'][i])
    if (LCS_readcsv['tail_time'][i] != 0):
        gap_head_tail.append(LCS_readcsv['tail_time'][i]-LCS_readcsv['head_time'][i])
        average_LCS_delay.append(LCS_readcsv['tail_time'][i])
        LCS_hop_list.append(abs(divmod(LCS_readcsv['signal_destid'][i], 14)[0] - divmod(LCS_readcsv['signal_sourceid'][i], 14)[0]) + abs(divmod(LCS_readcsv['signal_destid'][i], 14)[1] - divmod(LCS_readcsv['signal_sourceid'][i], 14)[1]))
# %%
#%%
import matplotlib.pyplot as plt
plt.figure()
ax = plt.gca()
plt.hist(gap_head_tail,rwidth=0.8,density=True)
plt.xlabel('Delay/Cycle')
plt.ylabel('Density')
plt.text(0.6,0.8,'Average Tail-Head interval: '+ str(np.mean(gap_head_tail)), horizontalalignment='center',
     verticalalignment='center',transform=ax.transAxes)
plt.text(0.6,0.75,'Number of LCS Packet received:  '+ str(len(gap_head_tail)), horizontalalignment='center',
     verticalalignment='center',transform=ax.transAxes)
plt.text(0.6,0.7,'Average Packet delay:  '+ str(np.mean(average_LCS_delay)), horizontalalignment='center',
     verticalalignment='center',transform=ax.transAxes)
plt.text(0.6, 0.65, 'Average hop of all LCS packets received:  ' + str(np.mean(LCS_hop_list)), horizontalalignment='center',
         verticalalignment='center', transform=ax.transAxes)
plt.title(str(os.getcwd()) +"/ "+ txtFileName_LCS)
plt.savefig(txtFileName_LCS)
plt.show()

#%%
urs_check_sigloss=[]
for i in range(0,URS_readcsv.shape[0]):
    if (URS_readcsv['signal_ID'][i] == 0 ):
        urs_check_sigloss.append(LCS_readcsv['signal_ID'][i])
    else:
        urs_check_sigloss.append(URS_readcsv['signal_ID'][i])
#%%
urs_check_sigloss_sort= np.sort((urs_check_sigloss))
urs_check_sigloss_sort_unique = np.sort(np.unique(urs_check_sigloss))
#%%
index_i=0
index_i_errorlist=[]
for i in range(0,urs_check_sigloss_sort_unique.shape[0]):
    #if (divmod(i,10000)[1] == 0):
     #   print(i,' work ')

    if (index_i != urs_check_sigloss_sort_unique[i]):
        print(index_i, 'error',urs_check_sigloss_sort_unique[i])
        index_i_errorlist.append(index_i)
        index_i=urs_check_sigloss_sort_unique[i]-1
    index_i = index_i + 1