import numpy as np
import sympy as sym
import matplotlib.pyplot as plt

def softmax(x):
    e_x = np.exp(x-np.max(x))
    return e_x / e_x.sum()

x = np.array([1, 2, 3, 4, 1, 2, 3])
print(softmax(x))