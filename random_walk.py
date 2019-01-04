import numpy as np
from random import random
import matplotlib.pyplot as plt

#This is a demo using a random walk along one dimension to demonstrate
#the property of Markov processes that the more terms they contain, the
#smaller the standard deviation of the end state as a proportion of the
#mean. This is a cool result for statistical mechanics as this is the
#motivation behind ignoring fluctuations, and was fun to play around with.

p = 0.5
q = 1 - p
vecs = [ np.zeros(20), np.zeros(50), np.zeros(200), np.zeros(1000) ]
trials = 10000
res = np.zeros(shape=(trials, 4))
numBins = 100

for k in range( trials ):
    x = 0
    for i in vecs:
        for j in range( len(i) ):
            i[j] = ( random() > p )
        res[k,x] = np.linalg.norm(i, ord=1)#L1 norm or sum( a for a in i )
        x += 1

#normalize the results & shift them
#Then, to avoid having to use a histogram, go to a cumulative probability distribution
for i in range( 4 ):
    res[:,i] -= ( len( vecs[i] ) / 2 )
    res[:,i] *= 1/( len( vecs[i] ) )
    #Comment out below to remove sort before histogram
    res[:,i] = np.sort(res[:,i], axis=None)
    
plt.show( plt.plot( res ) )
#plt.show( plt.hist( res, bins=numBins ) )
print("Done")
