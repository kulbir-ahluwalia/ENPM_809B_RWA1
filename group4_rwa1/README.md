## Build Instructions
```
mkdir -p ~/catkin_ws/src
cd ~/catkin_ws/
catkin_make
source devel/setup.bash
cd src/
copy group4_rwa1 package here
cd ..
catkin_make
```

## Running the code


Instructions for running package:
```
Open a new terminal
cd ~/catkin_ws
source devel/setup.bash
roslaunch group4_rwa1 group4_rwa1.launch


Open a new terminal
cd ~/catkin_ws
source devel/setup.bash
rosrun group4_rwa1 ariac_example_node
```
