import sympy as sym
import numpy as np

from sympy.abc import x, y

def eval(fun, val):
    val_x , val_y = val
    fun_eval = fun.subs(x, val_x) + fun.subs(y, val_y)
    return fun_eval

def func_muilti(val):
    x_, y_ = val
    func = sym.poly(x**2 + 2*y**2)
    return eval(func, [x_, y_]), func

def func_gradient(fun, val):
    x_, y_ = val
    _, function = fun(val)
    diff_x = sym.diff(function, x)
    diff_y = sym.diff(function, y)
    grad_vector = np.array([eval(diff_x, [x_, y_]), eval(diff_y, [x_, y_])], dtype=float)
    return grad_vector, [diff_x, diff_y]

def gradient_decent(fun, init_point, lr=1e-2, eps=1e-5):
    cnt = 0
    val = init_point
    diff, _ = func_gradient(fun, val)
    while np.linalg.norm(diff) > eps:
        val -= lr * diff
        diff = func_gradient(fun, val)
        cnt += 1

    print(f"FUNCTION : {fun(val)[1]}, COUNT : {cnt}, LOCAL MINIMUM : {val}, {fun(val)[0]}")

if __name__ == '__main__':
    pt = [np.random.uniform(-2, 2), np.random.uniform(-2, 2)]
    gradient_decent(fun=func_muilti, init_point=pt)