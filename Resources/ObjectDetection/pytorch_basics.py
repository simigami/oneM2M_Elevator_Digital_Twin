import torch
import torch.nn as nn
import torch.nn.functional as F
import numpy as np


arr = np.arange(12).reshape(2, 2, 3)
arr2 = np.arange(10).reshape(10, 1)
t_array = torch.FloatTensor(arr).to('cuda')
tt_array = torch.FloatTensor(arr2).to('cuda')

view_array = t_array.view(2, 6)
squeeze_array = torch.squeeze(tt_array)
unsqueeze_array = torch.unsqueeze(squeeze_array, 1)

# print(t_array)
# print(tt_array)
# print(view_array)
# print(squeeze_array)
# print(unsqueeze_array)
# print(f'ndim : {t_array.ndim} , shape : {t_array.shape}, dtype : {t_array.dtype}')
# print(f'ndim : {view_array.ndim} , shape : {view_array.shape}, dtype : {view_array.dtype}')
# print(f'ndim : {squeeze_array.ndim} , shape : {squeeze_array.shape}, dtype : {squeeze_array.dtype}')
# print(f'ndim : {unsqueeze_array.ndim} , shape : {unsqueeze_array.shape}, dtype : {unsqueeze_array.dtype}')

a = torch.zeros(6)
b = a.view(3, 2)
c = a.t().reshape(3, 2)
a.fill_(1)

# print(a)
# print(b)
# print(c)
# print(f'ndim : {a.ndim} , shape : {a.shape}, dtype : {a.dtype}')
# print(f'ndim : {b.ndim} , shape : {b.shape}, dtype : {b.dtype}')
# print(f'ndim : {c.ndim} , shape : {c.shape}, dtype : {c.dtype}')

ma = torch.arange(12).reshape(3, 4)
mb = torch.arange(12).reshape(3, 4)

# print(ma.flatten().dot(mb.flatten()))
# print(mb.mm(ma.t()))

w = torch.tensor([1.0, 2.0, 3.0], requires_grad=True)
al = torch.tensor([4.0, 5.0, 6.0], requires_grad=True)
y = w**2 + al*w + 3
z = y**2 + 25

GV = torch.tensor([1.0, 1.0, 1.0])
z.backward(gradient=GV)

print(w.grad)
print(al.grad)