import numpy as np
import gymnasium as gym
from gymnasium import spaces
import  interfacePy as itf
import matplotlib.pyplot as  plt
#import stable_baselines3
class NoCEnv(gym.Env):
    #动作是一个1*3的向量，分别代表三个组别的tokens 注入率
    def __init__(self):
        super(NoCEnv,self).__init__()
        #define action space and action type
        n_actions = 2
        high=100
        # need to  consider further
        low=0
        self.rewardData=[]#存储的是延时 并不是奖励 剪了以后才是
        self.rewardRef=[]
        self.action_space=spaces.Box(low=0.13, high=0.27, shape=(3,), dtype=np.float32)#
        self.observation_space = spaces.Box(low, high, shape=(4,), dtype=np.float32)#obs space ,each state has 4 features
        self.count=-1
        self.i=0
        # self.resetsignal=0
        self.cycle=0
        self.state=None
        self.next_cycle=0
        self.next_state=None
        #self.done1=False#用于 记录 累积奖励
        self.return1 = 0 # 记录一个epsiode的累积奖励
        self.returnData = [] # 记录每个episode的累积奖励
        self.count_144=-1 #每144 个值更新一次

    def reset(self,seed=None,options=None):
        super().reset(seed=seed, options=options)
        print("reset ,begin")
        #思路 ：不需要进行信号检查 ，但是要确保两个信号均为0
        returnfile=open("./cmake-build-debug/return.txt",'a')
        returnfile.write(str(self.return1)+'\n')
        returnfile.close()
        self.returnData.append(self.return1)# 每次重置之前 ，即 是我们每次完成一个episode的时候 将这个 episode的累积误差 存储下来
        self.return1 = 0 # 记录一个epsiode的累积奖励
        actionfile = open("./cmake-build-debug/actionReady.txt",'w+')
        actionfile.write('0')
        actionfile.close()
        statefile=open("./cmake-build-debug/stateReady.txt",'w+')
        statefile.write('0')#写入0的时候表示 调用reset的时候所有有数据准备信号都zhi 0，表示数据都没准备好
        statefile.close()#模拟每次getstate（）之前都会把stateready置数1，为1的时候
        resetfile=open("./cmake-build-debug/resetReady.txt",'w+')# 写入reset 信号 告诉 noc 可以进行reset
        resetfile.write('1')
        resetfile.close()
        #然后执行读取state 指令 ，并改写 stateready 信号
        stateData,cycleData=itf.getState()
        stateDatalist=[float (stateData) for stateData in stateData.split(' ') if stateData]
        cycleDatalist=float(cycleData  )
        self.state=np.round(np.array(stateDatalist),decimals=3)
        self.cycle=np.array(cycleDatalist)
        #print("reset state:", np.array(stateDatalist))
        # print("reset cycles", np.array(cycleDatalist))
        #rewardDa=self.state[0]#奖励数组永远都不应该将reset的reward记录下来
        #self.rewardData.append(rewardDa)#存入数组 为进行进一步的计算
        # self.count=self.count+1#用一个计数器记录 与环境的交互次数，reset 和 step都是 与环境 进行一次交互
        #self.done1 = False
        # self.count_144=self.count
        #return (np.array([self.state]).astype(np.float32), {},)
        print("shape of reward",np.shape(self.rewardData))
        return np.array(self.state).astype(np.float32), {}


    def step(self,action):
        #print("steps")
        itf.writeAction(action)#写动作
        #statefile=open("./cmake-build-debug/stateReady.txt",'w+')
        #statefile.write('1')#调用reset 函数
        #statefile.close()#模拟每次getstate（）之前都会把stateready置数1，这一步实际上会由cpp代码来执行
        next_stateData,next_cycleData=itf.getState()# 将next state和reward 得到，这两句加一起 相当于step 函数
        next_stateDatalist=[float (next_stateData) for next_stateData in next_stateData.split(' ') if next_stateData]
        next_cycleDatalist=float(next_cycleData)
        self.next_state=np.round(np.array(next_stateDatalist),decimals=3)
        self.next_cycle=np.array(next_cycleDatalist)
        # print("current state:", self.next_state)
        #  print("current cycles", np.array(next_cycleDatalist))
        terminated=False
        truncated=bool(self.next_cycle==50100)# 不确定 每个episode的结束条件
        #self.done1 = terminated or truncated
        rewardDa=self.next_state[0]# 记录state【0】为后续计算数组进行准备
        self.rewardData.append(rewardDa)#存入数组 为进行进一步的计算，因为reset函数不返reward 因此我们可以认为reward是从第一次迭代开始记录的
        #reward=self.rewardData[self.count]-self.rewardData[self.count+1]

        self.count=self.count+1#用一个计数器记录 与环境的交互次数，reset 和 step都是 与环境 进行一次交互
        reward =1/self.rewardData[self.count]#只有在step函数里才输出reward 因此只有在setp里才把count加1并将state【0】记录下来
        #print("reward:",reward)
        #reward=self.rewardRef[self.count % 144]-self.rewardData[self.count]
        #print("rewardRef: \n",self.rewardRef[self.count % 144])
        #print("count: \n",self.count)
        self.return1 = self.return1+reward# 在每次迭代的时候 都加上本次的奖励 构成累积奖励

        info= {}
        return(
            np.array(self.next_state).astype(np.float32),
            reward,
            terminated, # 达到某种条件结束
            truncated,#达到 某个次数 结束 或者异常结束
            info ,
        )

    def render(self):
        my_array=self.returnData[1:]
        print(self.returnData)
        print("\n")
        print(my_array)
        plt.switch_backend('TkAgg')
        plt.plot(my_array)  # 绘制数组的线图
        plt.xlabel('learning episodes')
        plt.ylabel('cumulative expected rewards')
        plt.show()



    def close (self):
        pass

    def loadRef(self):

        statefile2=open("./cmake-build-debug/rewardRef.txt",'r')
        for line in statefile2:
            self.rewardRef.append(float(line.strip()))#.strip()用于移除字符串首尾的制定字符 ，默认是换行或者空格
        statefile2.close()

    # print(self.rewardRef)




from stable_baselines3.common.env_checker import check_env
itf.creatTxT()
env = NoCEnv()
#check_env(env)
#state,info=env.reset()
#print("reset state" ,state)
#done2=False
#i=0
#while (done2 != True):
#  print("py begin")
# k=i%2
#print("k %d" % k )
#next_state,reward,done1, done2,info=env.step(k)
#i=i+1
#print(next_state,reward ,done1 ,done2)
#print("count",env.count)
#print("py end")
#state,info=env.reset()
#print("reset state" ,state)
#done2=False
#i=0
#while (done2 != True):
#   print("py begin")
#  k=i%2
# print("k %d" % k )
# next_state,reward,done1, done2,info=env.step(k)
#i=i+1
#print(next_state,reward ,done1 ,done2)
#print("count",env.count)
#print("py end")

#state,info=env.reset()
#print("reset state" ,state)
#print("rewardData",env.rewardData)
#print("returnData",env.returnData)
#env.render()
#
from stable_baselines3 import PPO
from stable_baselines3.common.evaluation import evaluate_policy
#env.loadRef()
from stable_baselines3 import DQN,A2C,SAC
from stable_baselines3 import PPO
#model = PPO("MlpPolicy", env, verbose=1  )

model = SAC(
    "MlpPolicy",
    env=env,

    learning_rate=1e-3,  #1e-3 10 sac4 sac7-1  sac5 sac8-5  sac1 sac3-2
    batch_size=64,
    buffer_size=200000,
    learning_starts=0,
    target_update_interval=10, #10
    #tau=0.05,
    gamma=0.99, #0.99
    policy_kwargs={"net_arch" : [128, 128]},
    verbose=1,
    tensorboard_log="./tensorboard/NOC_SAC_8/"
)
'''
model=A2C("MlpPolicy",
          env=env,

          learning_rate=1e-4,  #1e-4
          n_steps=64,
          #batch_size=64,
          #buffer_size=200000,
          #learning_starts=0,
          #target_update_interval=10, #10
          #tau=0.5
          gamma=0.97, #0.99
          #exploration_final_eps=0.006,  #最终学习率
          #exploration_fraction=0.6,
          policy_kwargs={"net_arch" : [128, 128]},
          verbose=1,
          tensorboard_log="./tensorboard/NOC_A2C_2newaction2/"
          
          )

'''
'''
model=PPO("MlpPolicy",
    env=env,

    learning_rate=1e-4,  #1e-4
    n_steps=64,
    batch_size=64,
    #buffer_size=200000,
    #learning_starts=0,
    #target_update_interval=10, #10
    #tau=0.5
    gamma=0.98, #0.99
    #exploration_final_eps=0.006,  #最终学习率
    #exploration_fraction=0.6,
    policy_kwargs={"net_arch" : [128, 128]},
    verbose=1,
    tensorboard_log="./tensorboard/NOC_PPO_2newaction2/"
    )
    '''
"""
model = DQN(
    "MlpPolicy",
    env=env,

    learning_rate=1e-4,  #1e-4
    batch_size=64,
    buffer_size=200000,
    learning_starts=0,
    target_update_interval=10, #10
    #tau=0.5
    gamma=0.97, #0.99
    exploration_final_eps=0.006,  #最终学习率
    exploration_fraction=0.6,
    policy_kwargs={"net_arch" : [128, 128]},
    verbose=1,
    tensorboard_log="./tensorboard/NOC_DQN_2newaction2/"
)
"""
model.learn(total_timesteps=32000, log_interval=4, progress_bar=True)
model.save("NoC_SAC_6")
env.render()
#mean_reward,std_reward =  evaluate_policy (model, env, n_eval_episodes=10, render =False)
#print(mean_reward , std_reward)
print("finish trainning \n")






