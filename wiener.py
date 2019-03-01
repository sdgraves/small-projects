import matplotlib.pyplot as plt
import numpy as np

#This module separates two series, one the output of a random process and the other white noise.

#size of the random sequence
N = 5001

#define a random sequence to excite the process
u = np.random.normal(size=N)
#define the plant producing the random process
t_end = 2
tau = 0.4
t = np.linspace(0, t_end, N)
plant = np.exp(-t/tau)
plant = plant/(np.sqrt(np.dot(plant, plant)))

#produce the output sequence
x = np.convolve( plant, u )[0:N]

#construct a second white noise process to sum with the first
w = np.random.normal(scale=0.5, size=N)

#y represents the measurement of the sum
y = x + w


#construct the autocorrelation of the random process using the known autocorrelation
#of white noise and the deterministic transfer function of the system;
#the plant is real-valued (defined by impulse invariance above) but in general is not.
H = np.fft.fftshift(np.fft.fft(plant))
rev = [plant[plant.size - 1 - i] for i in range( plant.size )]
G = np.fft.fftshift(np.fft.fft(np.conj(rev)))

#autocorrelation of Gaussian signal resulting from process.
A_x = H*G

#transform of autocorrelation of additive white noise w
mag = np.dot(w,w)
A_w = np.array([ mag/N for i in range( w.size )])

#define the filter
F = np.fft.fftshift(A_x / ( A_x + A_w ))

#apply it (circularly extended)
X_est = np.fft.fft(y)*F
x_est = np.fft.ifft(X_est)

#performance metrics
err = x_est - x
mag_err = np.dot( err, err )
sig = np.dot(x,x)
print("SNR increased from", sig/mag, "to", sig/(np.abs(mag_err)))
#plt.plot(w)
#plt.plot(err)

plt.plot(y)
plt.plot(x_est)
plt.plot(x)
plt.show()
