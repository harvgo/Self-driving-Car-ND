#PID - Control
##Reflection
###1. Describe the effect each of the P, I, D components had in your implementation.
* **P** coefficient refers to the **Proportional** parameter of the controller. As its name indicates, this refers directly to the cross track error itself. If the distance from the desired trajectory is farther, the controller steer the well harder than if the car is close to the trajectory. That is, the amount of steering is given in **proportion** of the cross track error.
Low values of **P** produces the controller to overshoot and high values of **P** makes the controller to converge quickly. But if the error is too big, high values of **P** can produce the system to get out of "control".

* **D** parameter is the **Derivative** portion of the controller. It takes into account the change of rate of the perpendicular variation of the cross track error. It other words, it gives a bit of resistance to the controller in order to reduce the oscillations given by **P**.
If **D** is low, the oscillation still remains. On the opposite direction, the damping is almost zero with higher values of **D**, but it takes much time to achieve the desired trajectory.

* **I** coefficient refers to the **Integral** part of the controller. This parameter changes the steering taking into account the previous state errors due to bias (e.g. wheel misalignment and/or environmental factors). 
High value of **I** produces instability in the correction of the path. Otherwise, a low value of **I** takes too long to correct those dynamic changes.

###2. Describe how the final hyperparameters were chosen.
The hyperparameters were chosen trhough manual tuning. The **I** parameter is kept low, because the car is expected to behave without any bias in the simulator. As **P** produces the car to oscillate a lot, I should mantain this value also lower as possible. Otherwise, **D** coefficient is the largest of the three, because it reduces the oscillations produced by **P**.