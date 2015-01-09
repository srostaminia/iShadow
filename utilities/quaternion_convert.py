import numpy as np

def QtoRM(qw,qx,qy,qz,px,py,pz):
    Q = np.matrix([[1 - 2*qy**2 - 2*qz**2,2*qx*qy - 2*qz*qw,2*qx*qz + 2*qy*qw],
                   [2*qx*qy + 2*qz*qw,1 - 2*qx**2 - 2*qz**2,2*qy*qz - 2*qx*qw],
                   [2*qx*qz - 2*qy*qw,2*qy*qz + 2*qx*qw,1 - 2*qx**2 - 2*qy**2]])
    #theta = 3.14
    P = np.matrix([[px],[py],[pz]])

    QP = np.array(Q*P).T
  
    return QP.tolist()

q0 = 0.996002912
q1 = 0.011618956922
q2 = -0.00900928489
q3 = -0.08810281

px = 0
py = 0
pz = 1

print QtoRM(q0, q1, q2, q3, px, py, pz)
