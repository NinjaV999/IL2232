import matplotlib.pyplot as plt
from tqdm import tqdm
import numpy as np
import torch
import collections
import random



def moving_average(a, window_size):
    cumulative_sum = np.cumsum(np.insert(a, 0, 0))
    middle = (cumulative_sum[window_size:] - cumulative_sum[:-window_size]) / window_size
    r = np.arange(1, window_size-1, 2)
    begin = np.cumsum(a[:window_size-1])[::2] / r
    end = (np.cumsum(a[:-window_size:-1])[::2] / r)[::-1]
    return np.concatenate((begin, middle, end))




def compute_advantage(gamma, lmbda, td_delta):
    td_delta = td_delta.detach().numpy()
    advantage_list = []
    advantage = 0.0
    for delta in td_delta[::-1]:
        advantage = gamma * lmbda * advantage + delta
        advantage_list.append(advantage)
    advantage_list.reverse()
    return torch.tensor(advantage_list, dtype=torch.float)


file_path = './cmake-build-debug/return.txt'
with open(file_path, 'r') as file:
    numbers = [float(line.strip()) for line in file]

numbers1=numbers[1:]

plt.plot(numbers1)
plt.xlabel('episodes')
plt.ylabel('cumulative rewards')
plt.ylim(0,300)
plt.title('DQN Agent ')
plt.show()


mv_return= moving_average(numbers1,9)
episode_list=list(range(len(numbers1)))
plt.plot(episode_list,mv_return)
plt.xlabel('episodes')
plt.ylabel('returns')
plt.ylim(0,300)
plt.title('DQN')
plt.show()