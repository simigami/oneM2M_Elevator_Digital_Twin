import sympy as sym
import numpy as np

from sympy.abc import x, y

def func(val):
    fun = sym.poly(x**2 + 2*x + 3)
    return fun.subs(x, val), fun

def func_grad(fun, val):
    _, fun = fun(val)
    diff = sym.diff(fun, x)
    return diff.subs(x, val), diff

def gradient_decent(fun, init_point, lr=1e-2, eps=1e-5):
    cnt = 0
    val = init_point
    diff, _ = func_grad(fun, val)
    while np.abs(diff) > eps:
        val -= lr * diff
        diff, _ = func_grad(fun, val)
        cnt += 1

    print(f"FUNCTION : {fun(val)[1]}, COUNT : {cnt}, LOCAL MINIMUM : {val}, {fun(val)[0]}")

if __name__ == '__main__':
    gradient_decent(fun=func, init_point=np.random.uniform(-2, 2))
    #parital_diff_func()