import tensorflow as tf
import numpy as np
from numba import vectorize, cuda, jit
from timeit import default_timer as timer
import math as math

#Data flow approach to a double-pendulum - this approach produces a parallel implementation and also
#allows for use of the tensorflow gradient function to get an accurate derivative. Decoupling the two
#pendulums here does in fact produce the classic phase portraits, with markedly improved accuracy over
#the earlier purely numerical approach.

#Double-pendulum parameters
m1, m2, r1, r2 = tf.unstack( tf.placeholder(tf.float64, shape=(4,)) )
g = tf.constant([9.806], tf.float64)
dt = tf.placeholder(tf.float64)

#State variables - easier to init here than to run the program again just because of how
#annoying it is to pass very many placeholders in a dict to tensorflow.
state = tf.Variable([0., 0., 0., 0.], dtype=tf.float64)
q1, q2, w1, w2 = tf.unstack( state )


#This part of the graph produces the Hamiltonian from the state variables
KE = 0.5 * m1 * ( r1 * w1 )**2 + (
    0.5 * m2 * (
        ( r1 * w1 )**2 + ( r2 * w2 )**2 + (
            2 * r1 * r2 * w1 * w2 * tf.cos( q1 + q2 )
            )
        )
    )
PE = m1 * r1 * tf.cos( q1 ) + (
    m2 * (
        r1 * tf.cos( q1 ) + r2 * tf.cos( q1 + q2 )
        )
    )
L = tf.add(KE, -1*PE)

#take the partials
p1, p2 = tf.unstack(tf.placeholder(shape=(2,), dtype=tf.float64))
[p1, p2] = tf.gradients(L, [q1, q2])
#Data flow architecture means that tf.gradients(L, p) returns none - but,
#easy to construct H from p,q, L, in accordance with the formal definition.

H = ( q1*p1 + q2*p2 ) - L
vec = tf.gradients(H, [p1, p2, q1, q2])

#remembering that \dot{ P } = - \frac{\partial H}{\partial q},
#the gradient has to be rearranged a little.
a, b, c, d = tf.unstack(vec)
increment = tf.multiply( tf.stack([-c, -d, a, b]), dt )
nextState = state + increment

    
#have to also construct a graph which will use the gradients given to actually progress the state
sess = tf.Session()
buf = np.zeros((4096, 4), dtype=np.float64)
[w, x, y, z] = [0, 0., math.pi/6., 0]
[l, m, n, o] = [0., 0., 0., 0.]
step = 0.001

out = sess.run( increment, {m1:1., m2:1., r1:1., r2:1., w1:w, w2:x, q1:y, q2:z, dt:step} )
start = timer()
f = open('dat.csv', 'w')
for i in range( 4096 ):
    #This is in pq space. The function, however, is now initialized in q-qdot space.
    #best bet is to read off qdot and then track q from the state.
    nextState = np.array([w + l, x + m, y + n, z + o], dtype=np.float64)
    print(np.array_str( nextState ) )
    buf[i] = nextState
    [w, x, y, z] = nextState
    [l, m, n, o] = sess.run( increment, {m1:1., m2:1., r1:1., r2:1., w1:w, w2:x, q1:y, q2:z, dt:step} )
#    print(np.array_str( np.array([l, m, n, o], dtype=np.float32) ) )

for i in range( 4096 ):
    for j in range( 3 ):
        f.write("%3f," % buf[i][j])
    f.write("%3f\n" % buf[i][3])
    
print("Ran in %f" % (timer() - start) )
    
    
