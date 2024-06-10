import torch
import numpy as np

class DropoutModel(torch.nn.Module):
    def __init__(self):
        super(DropoutModel, self).__init__()
        self.layer1 = torch.nn.Linear(784, 1200)
        self.dropout1 = torch.nn.Dropout(0.5)
        self.layer2 = torch.nn.Linear(1200, 1200)
        self.dropout2 = torch.nn.Dropout(0.5)
        self.layer3 = torch.nn.Linear(1200, 10)

    def forward(self, x):
        x = F.relu(self.layer1(x))
        x = self.dropout1(x)
        x = F.relu(self.layer2(x))
        x = self.dropout2(x)
        return self.layer3(x)

def L1_Norm(x):
    x_norm = np.abs(x)
    x_norm = np.sum(x_norm)
    return x_norm

def L2_Norm(x):
    x_norm = np.abs(x) ** 2
    x_norm = np.sum(x_norm)
    x_norm = np.sqrt(x_norm)
    return x_norm

def angle(x, y):
    y = np.inner(x, y) / (L2_Norm(x) * L2_Norm(y))
    return np.arccos(y)

def broad():
    t1 = torch.tensor([[1, 4, 5, 8, 10]]) # 1, 3
    t2 = torch.tensor([[1,2],[3,4],[10,11],[-1,2]]) # 4, 1

    print(t1+t2)
    print((t1+t2).size())

if __name__ == '__main__':
    broad()