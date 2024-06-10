import numpy as np

X = np.array([[1,1],[1,2],[2,2],[2,3]])
y = np.dot(X, np.array([1,2])) * 3
lr = 0.031

X_ = np.hstack((X, np.ones((X.shape[0], 1))))
beta = np.random.uniform(-1.0, 1.0, X_.shape[1])

for t in range(5000):
    error = y - X_ @ beta
    grad = -2 * X_.T @ error
    beta = beta - lr * grad

print(beta)