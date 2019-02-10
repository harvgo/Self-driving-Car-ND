# Model Predictive Control Project
Last project of the term 2 of the Udacity Self-Driving Car Nanodegree

## Implementation

### The Model
The vehicle model employed for this project is the kinematic model of a bicicle, where two wheels aligned with the forward wheel as the steerable one. Because of the latter, the change of heading direction implies that the model is non-linear.

The equations used for describing the model are as follow:
$$\begin{align}
x_{t+1} &= x_t + v_t*\cos(\psi_t)*dt \\
y_{t+1} &= y_t + v_t*\sin(\psi_t)*dt \\
\psi_{t+1} &= \psi_t + \frac{v_t}{L_f}*\delta_t*dt \\
v_{t+1} &= v_t + a_t*dt \\
\text{cte}_{t+1} &= f(x_t) - y_t + (v_t*\sin(e\psi_t)*dt) \\
e\psi_{t+1} &= \psi_t - \psi\text{des}_t + \frac{v_t}{L_f}*\delta_t*dt
\end{align}$$

### Timestep Length and Elapsed Duration ($N$ & $dt$)
Based on the lectures, $N$  is the number of steps given to the model to predict. The greater $N$ is, the farther is the prediction of the model, producing smoother controlling of the vehicle.

On the other hand, $dt$ is the lapse between the steps for the prediction, giving the prediction horizon $T$ as the total time for future estimation, as
$$T = N * dt$$

Hence, having a balance between the number of steps, $N$, and the time duration between them, $dt$, the optimizer will manage the possible best trajectory for the car.

Based on the suggestion provided by Udacity, the values of $N=10$ and $dt=0.1$ give an horizon of $T=1s$, for appropriate prediction. This in accordance to a possible changing environment.

### Polynomnial Fitting and MPC Preprocessing
The waypoints are given in global coordinates. Hence, it is necessary to transform them to car's coordinates. In that way, the value of $x$ and $y$, as well as the value of orientation $\psi$ are now $0$.

The functions used to make the transofrmation from global to local coordinates is as follows:

```c++
// Eigen vectors required for polyfit
Eigen::VectorXd ptsx_trans(ptsx.size());
Eigen::VectorXd ptsy_trans(ptsy.size());

// Transform waypoints ptsx, ptsy to car's coordinates
for (size_t i = 0; i < ptsx.size(); i++) {
  double dx = ptsx[i] - px;
  double dy = ptsy[i] - py;
  ptsx_trans[i] = dx * cos(0-psi) - dy * sin(0-psi);
  ptsy_trans[i] = dx * sin(0-psi) + dy * cos(0-psi);
}
```
Then, it is possible to fit the transformed points to a 3rd-order polynomial, using

```c++
auto coeffs = polyfit(ptsx_trans, ptsy_trans, 3);
```

### Model Predictive Control with Latency
Delay in the car's actuators is expected in real life. So, this must be taken into account for real applications. The idea behind is to predict where the can will be in the future, using the latency value as the time step for prediction. In this particular case, the latency is of $100 ms$ $(0.1 s)$.

This new value of timestep allows to find the position, heading, and velocity of the vehicle,  as well as the cross track and heading errors. The model used is the same descripted before, but with the advantages of using the car's local reference frame.

```c++
// Predict the car's state 100 ms into the future to compensate for latency
const double latency = 0.1;
double x_pred = v * latency;
double y_pred = 0;
double psi_pred = -v * delta / Lf * latency;
double v_pred = v + a * latency;
double cte_pred = cte + v * sin(epsi) * latency;
double epsi_pred = epsi + psi_pred;

Eigen::VectorXd state(6);
state << x_pred, y_pred, psi_pred, v_pred, cte_pred, epsi_pred;
```
