import numpy as np
import  pandas as pd
#%%
txtFileName="routerOutport"
routerOutport_readtxt=pd.read_csv(txtFileName+".txt", sep=' ',index_col=False, names=['outportList'])#,delimiter=' '
print("read outport list done")
#%%
