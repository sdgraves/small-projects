import numpy as np
from numba import vectorize, cuda, jit
from timeit import default_timer as timer
import math as math

#This program uses the cuda library (see includes) to obtain a small speed-up on the Hamiltonian of a
#double pendulum, which after all takes vector arguments. It also receives a significant speedup from
#the C backend. This program also demonstrates some of the frailties of numerical methods, as the
#output is not satisfactory except over very short distances.

#It might be worth revisiting this file to see if the Shanks series transformation might allow for a
#useful numerical derivative without going all the way to an algebraic approach. (might be linear
#series transformations that would be useful as well)

#1D parameters
I = 1
r = 1
g = 1
theta = 1
globalVar = np.float32( np.cos(1) )


#Double-pendulum parameters
i1 = 1
i2 = 1
r1 = 1
r2 = 1


@jit(["float32(float32, float32, float32, float32)"], nopython=True)
def H(p1, p2, x1, x2):
    KE = 0.5 * ((p1**2)/r1) + (i2/(2*r2)*(p1**2 + p2**2 + (2 * p1 * p2 * np.cos(x1 - x2))))
    PE = ( r1 * np.cos( x1 ) * ( (i1/(r1**2)) + (i2/(r2**2)) ) + ( (i2/r2) * np.cos( x2 ) ) ) * g
    return KE + PE


@jit
def gradH(state):
    eps =  state / math.pow(2,32)
    epsMat = ( eps * np.eye(4) )
    for i in range( 4 ):
        epsMat[i][i] = max( math.pow(2, -32), epsMat[i][i] )
        vec = state + epsMat[i]
        grad = np.zeros(4, dtype=np.float32)
        arg1 = ( H(state[0], state[1], state[2], state[3]) )
        arg2 =  H(vec[0], vec[1], vec[2], vec[3])
        assert( epsMat[i][i] != 0 )
        grad[i] = (arg2 - arg1 ) / epsMat[i][i]
    return grad

def rotate(grad, state):
    delta = np.zeros(4, dtype=np.float32)
    mvec = np.zeros(2, dtype=np.float32)            
    delta[0] = -grad[2]
    delta[1] = -grad[3]
    delta[2] = grad[0]
    delta[3] = grad[1]
    print(np.array_str(delta))
    return delta
    

def populateHField(myState):
    state = myState
    dt = 0.01
    buf = np.zeros((4*4096,4), dtype=np.float32)
    increment = np.zeros(4, dtype=np.float32)
    f = open('dat.csv','w')
    for i in range( 4*4096 ):
        buf[i] = state
        increment = gradH( state ) * dt
        increment = rotate( increment, state )
        state += increment
            
    for i in range( 4*4096 ):
        for j in range( 3 ):
            f.write("%3f" % buf[i][j])
            f.write(",")
        f.write("%3f" % buf[i][3])
        f.write("\n")


myState = np.array([0,0,1,0], dtype=np.float32)
populateHField(myState)
