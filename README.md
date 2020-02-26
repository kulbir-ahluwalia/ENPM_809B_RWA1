# ENPM_809B_rwa2

Follow the following steps to create a competition package. First, create a catkin workspace:-
```
mkdir -p ~/group4_rwa2/src   
cd ~/group4_rwa2/src   
source /opt/ros/melodic/setup.zsh #source /opt/ros/melodic/setup.bash for bash users  
catkin_init_workspace  
```

Create a package named "ariac_example" in the ```src``` folder of the workspace named ```group4_rwa2```:-
```
mkdir ~/group4_rwa2/src/ariac_example
cd ~/group4_rwa2/src/ariac_example
```
Then create a file named package.xml in the "ariac_example" package folder:-
```
subl ~/group4_rwa2/src/ariac_example/package.xml
```
Some useful information about the package.xml file [2]:-
1. This is called the package manifest. Its job is to describe what is needed to build and run your package in a machine readable format. Additionally, it marks the root folder of your package.
2. Many of the entries in the package manifest are used for packagng and releasing your package. Since you're probably not going to release your package, many fields can be neglected or filled with placeholders.

Paste the following in the package.xml file:-
```xml
<?xml version="1.0"?>
<package format="2">

  <name>ariac_example</name>
  <version>0.1.0</version>
  <description>An example of an ARIAC competitor's package.</description>
  <maintainer email="kulbir@terpmail.umd.edu">Kulbir Singh Ahluwalia</maintainer>
  <maintainer email="spatan07@terpmail.umd.edu">Patan Sanaulla Khan</maintainer>

  <license>Apache 2.0</license>

  <buildtool_depend>catkin</buildtool_depend>

  <depend>osrf_gear</depend>
  <depend>roscpp</depend>
  <depend>sensor_msgs</depend>
  <depend>std_srvs</depend>
  <depend>tf2_geometry_msgs</depend>
  <depend>trajectory_msgs</depend>

</package>
```
Make the CMakeLists.txt file. This must be in the same folder as your package manifest.
```zsh
subl ~/group4_rwa2/src/ariac_example/CMakeLists.txt
```
Paste the following content into the CMakeLists.txt file. **Be sure to comment catkin_python_setup()** :-
```
cmake_minimum_required(VERSION 2.8.3)
project(ariac_example)

find_package(catkin REQUIRED COMPONENTS
  osrf_gear
  roscpp
  sensor_msgs
  std_srvs
  trajectory_msgs
)

## Uncomment this if the package has a setup.py. This macro ensures
## modules and global scripts declared therein get installed
## See http://ros.org/doc/api/catkin/html/user_guide/setup_dot_py.html
#catkin_python_setup()

catkin_package()

###########
## Build ##
###########

include_directories(include ${catkin_INCLUDE_DIRS})

## Declare a C++ executable
add_executable(ariac_example_node src/ariac_example_node.cpp)
add_dependencies(ariac_example_node ${catkin_EXPORTED_TARGETS})
target_link_libraries(ariac_example_node ${catkin_LIBRARIES})

#############
## Install ##
#############

# all install targets should use catkin DESTINATION variables
# See http://ros.org/doc/api/catkin/html/adv_user_guide/variables.html

## Mark executable scripts (Python etc.) for installation
install(PROGRAMS
  script/ariac_example_node.py
  script/tf2_example.py
  DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

## Mark executables and/or libraries for installation
install(TARGETS ariac_example_node
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

## Mark cpp header files for installation
# install(DIRECTORY include/${PROJECT_NAME}/
#   DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
#   FILES_MATCHING PATTERN "*.h"
#   PATTERN ".svn" EXCLUDE
# )

## Mark other files for installation (e.g. launch and bag files, etc.)
install(FILES
 config/sample_gear_conf.yaml
 DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/config
)

#############
## Testing ##
#############

## Add gtest based cpp test target and link libraries
# catkin_add_gtest(${PROJECT_NAME}-test test/test_ariac_example.cpp)
# if(TARGET ${PROJECT_NAME}-test)
#   target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME})
# endif()

## Add folders to be run by python nosetests
# catkin_add_nosetests(test)

```
## ARIAC Competition Configuration
mkdir -p ~/group4_rwa2/src/ariac_example/config  
subl ~/group4_rwa2/src/ariac_example/config/sample_gear_conf.yaml  
  
Copy the following content into the ```sample_gear_conf.yaml``` file:-
```
sensors:
  break_beam_1:
    type: break_beam
    pose:
      xyz: [1.6, 2.25, 0.91]
      rpy: [0, 0, 'pi']
  proximity_sensor_1:
    type: proximity_sensor
    pose:
      xyz: [0.95, 2.6, 0.92]
      rpy: [0, 0, 0]
  logical_camera_1:
    type: logical_camera
    pose:
      xyz: [-0.3, 0.383, 1.27]
      rpy: [0.2, 1.5707, 0]
  logical_camera_2:
    type: logical_camera
    pose:
      xyz: [0.3, 3.15, 1.5]
      rpy: [0, 'pi/2', 0]
  logical_camera_3:
    type: logical_camera
    pose:
      xyz: [0.3, -3.15, 1.5]
      rpy: [0, 'pi/2', 0]
  depth_camera_1:
    type: depth_camera
    pose:
      xyz: [-0.3, -0.383, 1.2]
      rpy: [0, 1.5707, 0]
  laser_profiler_1:
    type: laser_profiler
    pose:
      xyz: [1.21, 4, 1.6]
      rpy: ['-pi', 'pi/2', 'pi/2']

```

## Creating a C++ Node to Interface with the Competition
To create the src folder in the package ```ariac_example```:-
```
mkdir -p ~/group4_rwa2/src/ariac_example/src/
```
To create a C++ ROS Node:-
```
subl ~/group4_rwa2/src/ariac_example/src/ariac_example_node.cpp
```
Copy the following content into the ```ariac_example_node.cpp``` file:-
```
// Copyright 2016 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// %Tag(FULLTEXT)%
// %Tag(INCLUDE_STATEMENTS)%
#include <algorithm>
#include <vector>

#include <ros/ros.h>

#include <osrf_gear/LogicalCameraImage.h>
#include <osrf_gear/Order.h>
#include <osrf_gear/Proximity.h>
#include <sensor_msgs/JointState.h>
#include <sensor_msgs/LaserScan.h>
#include <sensor_msgs/Range.h>
#include <std_msgs/Float32.h>
#include <std_msgs/String.h>
#include <std_srvs/Trigger.h>
#include <trajectory_msgs/JointTrajectory.h>
// %EndTag(INCLUDE_STATEMENTS)%

// %Tag(START_COMP)%
/// Start the competition by waiting for and then calling the start ROS Service.
void start_competition(ros::NodeHandle & node) {
  // Create a Service client for the correct service, i.e. '/ariac/start_competition'.
  ros::ServiceClient start_client =
    node.serviceClient<std_srvs::Trigger>("/ariac/start_competition");
  // If it's not already ready, wait for it to be ready.
  // Calling the Service using the client before the server is ready would fail.
  if (!start_client.exists()) {
    ROS_INFO("Waiting for the competition to be ready...");
    start_client.waitForExistence();
    ROS_INFO("Competition is now ready.");
  }
  ROS_INFO("Requesting competition start...");
  std_srvs::Trigger srv;  // Combination of the "request" and the "response".
  start_client.call(srv);  // Call the start Service.
  if (!srv.response.success) {  // If not successful, print out why.
    ROS_ERROR_STREAM("Failed to start the competition: " << srv.response.message);
  } else {
    ROS_INFO("Competition started!");
  }
}
// %EndTag(START_COMP)%

/// Example class that can hold state and provide methods that handle incoming data.
class MyCompetitionClass
{
public:
  explicit MyCompetitionClass(ros::NodeHandle & node)
  : current_score_(0), arm_1_has_been_zeroed_(false), arm_2_has_been_zeroed_(false)
  {
    // %Tag(ADV_CMD)%
    arm_1_joint_trajectory_publisher_ = node.advertise<trajectory_msgs::JointTrajectory>(
      "/ariac/arm1/arm/command", 10);

    arm_2_joint_trajectory_publisher_ = node.advertise<trajectory_msgs::JointTrajectory>(
      "/ariac/arm2/arm/command", 10);
    // %EndTag(ADV_CMD)%
  }

  /// Called when a new message is received.
  void current_score_callback(const std_msgs::Float32::ConstPtr & msg) {
    if (msg->data != current_score_)
    {
      ROS_INFO_STREAM("Score: " << msg->data);
    }
    current_score_ = msg->data;
  }

  /// Called when a new message is received.
  void competition_state_callback(const std_msgs::String::ConstPtr & msg) {
    if (msg->data == "done" && competition_state_ != "done")
    {
      ROS_INFO("Competition ended.");
    }
    competition_state_ = msg->data;
  }

  /// Called when a new Order message is received.
  void order_callback(const osrf_gear::Order::ConstPtr & order_msg) {
    ROS_INFO_STREAM("Received order:\n" << *order_msg);
    received_orders_.push_back(*order_msg);
  }

  // %Tag(CB_CLASS)%
  /// Called when a new JointState message is received.
  void arm_1_joint_state_callback(
    const sensor_msgs::JointState::ConstPtr & joint_state_msg)
  {
    ROS_INFO_STREAM_THROTTLE(10,
      "Joint States arm 1 (throttled to 0.1 Hz):\n" << *joint_state_msg);
    // ROS_INFO_STREAM("Joint States:\n" << *joint_state_msg);
    arm_1_current_joint_states_ = *joint_state_msg;
    if (!arm_1_has_been_zeroed_) {
      arm_1_has_been_zeroed_ = true;
      ROS_INFO("Sending arm to zero joint positions...");
      send_arm_to_zero_state(arm_1_joint_trajectory_publisher_);
    }
  }

  void arm_2_joint_state_callback(
    const sensor_msgs::JointState::ConstPtr & joint_state_msg)
  {
    ROS_INFO_STREAM_THROTTLE(10,
      "Joint States arm 2 (throttled to 0.1 Hz):\n" << *joint_state_msg);
    // ROS_INFO_STREAM("Joint States:\n" << *joint_state_msg);
    arm_2_current_joint_states_ = *joint_state_msg;
    if (!arm_2_has_been_zeroed_) {
      arm_2_has_been_zeroed_ = true;
      ROS_INFO("Sending arm 2 to zero joint positions...");
      send_arm_to_zero_state(arm_2_joint_trajectory_publisher_);
    }
  }
  // %EndTag(CB_CLASS)%

  // %Tag(ARM_ZERO)%
  /// Create a JointTrajectory with all positions set to zero, and command the arm.
  void send_arm_to_zero_state(ros::Publisher & joint_trajectory_publisher) {
    // Create a message to send.
    trajectory_msgs::JointTrajectory msg;

    // Fill the names of the joints to be controlled.
    // Note that the vacuum_gripper_joint is not controllable.
    msg.joint_names.clear();
    msg.joint_names.push_back("shoulder_pan_joint");
    msg.joint_names.push_back("shoulder_lift_joint");
    msg.joint_names.push_back("elbow_joint");
    msg.joint_names.push_back("wrist_1_joint");
    msg.joint_names.push_back("wrist_2_joint");
    msg.joint_names.push_back("wrist_3_joint");
    msg.joint_names.push_back("linear_arm_actuator_joint");
    // Create one point in the trajectory.
    msg.points.resize(1);
    // Resize the vector to the same length as the joint names.
    // Values are initialized to 0.
    msg.points[0].positions.resize(msg.joint_names.size(), 0.0);
    // How long to take getting to the point (floating point seconds).
    msg.points[0].time_from_start = ros::Duration(0.001);
    ROS_INFO_STREAM("Sending command:\n" << msg);
    joint_trajectory_publisher.publish(msg);
  }
  // %EndTag(ARM_ZERO)%

  /// Called when a new LogicalCameraImage message is received.
  void logical_camera_callback(
    const osrf_gear::LogicalCameraImage::ConstPtr & image_msg)
  {
    ROS_INFO_STREAM_THROTTLE(10,
      "Logical camera: '" << image_msg->models.size() << "' objects.");
  }

  /// Called when a new Proximity message is received.
  void break_beam_callback(const osrf_gear::Proximity::ConstPtr & msg) {
    if (msg->object_detected) {  // If there is an object in proximity.
      ROS_INFO("Break beam triggered.");
    }
  }

private:
  std::string competition_state_;
  double current_score_;
  ros::Publisher arm_1_joint_trajectory_publisher_;
  ros::Publisher arm_2_joint_trajectory_publisher_;
  std::vector<osrf_gear::Order> received_orders_;
  sensor_msgs::JointState arm_1_current_joint_states_;
  sensor_msgs::JointState arm_2_current_joint_states_;
  bool arm_1_has_been_zeroed_;
  bool arm_2_has_been_zeroed_;
};

void proximity_sensor_callback(const sensor_msgs::Range::ConstPtr & msg) {
  if ((msg->max_range - msg->range) > 0.01) {  // If there is an object in proximity.
    ROS_INFO_THROTTLE(1, "Proximity sensor sees something.");
  }
}

void laser_profiler_callback(const sensor_msgs::LaserScan::ConstPtr & msg) {
  size_t number_of_valid_ranges = std::count_if(
    msg->ranges.begin(), msg->ranges.end(), [](const float f) {return std::isfinite(f);});
  if (number_of_valid_ranges > 0) {
    ROS_INFO_THROTTLE(1, "Laser profiler sees something.");
  }
}

// %Tag(MAIN)%
int main(int argc, char ** argv) {
  // Last argument is the default name of the node.
  ros::init(argc, argv, "ariac_example_node");

  ros::NodeHandle node;

  // Instance of custom class from above.
  MyCompetitionClass comp_class(node);

  // Subscribe to the '/ariac/current_score' topic.
  ros::Subscriber current_score_subscriber = node.subscribe(
    "/ariac/current_score", 10,
    &MyCompetitionClass::current_score_callback, &comp_class);

  // Subscribe to the '/ariac/competition_state' topic.
  ros::Subscriber competition_state_subscriber = node.subscribe(
    "/ariac/competition_state", 10,
    &MyCompetitionClass::competition_state_callback, &comp_class);

  // %Tag(SUB_CLASS)%
  // Subscribe to the '/ariac/orders' topic.
  ros::Subscriber orders_subscriber = node.subscribe(
    "/ariac/orders", 10,
    &MyCompetitionClass::order_callback, &comp_class);
  // %EndTag(SUB_CLASS)%

  // Subscribe to the '/ariac/joint_states' topic.
  ros::Subscriber arm_1_joint_state_subscriber = node.subscribe(
    "/ariac/arm1/joint_states", 10,
    &MyCompetitionClass::arm_1_joint_state_callback, &comp_class);

  ros::Subscriber arm_2_joint_state_subscriber = node.subscribe(
    "/ariac/arm2/joint_states", 10,
    &MyCompetitionClass::arm_2_joint_state_callback, &comp_class);

  // %Tag(SUB_FUNC)%
  // Subscribe to the '/ariac/proximity_sensor_1' topic.
  ros::Subscriber proximity_sensor_subscriber = node.subscribe(
    "/ariac/proximity_sensor_1", 10, proximity_sensor_callback);
  // %EndTag(SUB_FUNC)%

  // Subscribe to the '/ariac/break_beam_1_change' topic.
  ros::Subscriber break_beam_subscriber = node.subscribe(
    "/ariac/break_beam_1_change", 10,
    &MyCompetitionClass::break_beam_callback, &comp_class);

  // Subscribe to the '/ariac/logical_camera_1' topic.
  ros::Subscriber logical_camera_subscriber = node.subscribe(
    "/ariac/logical_camera_1", 10,
    &MyCompetitionClass::logical_camera_callback, &comp_class);

  // Subscribe to the '/ariac/laser_profiler_1' topic.
  ros::Subscriber laser_profiler_subscriber = node.subscribe(
    "/ariac/laser_profiler_1", 10, laser_profiler_callback);

  ROS_INFO("Setup complete.");
  start_competition(node);
  ros::spin();  // This executes callbacks on new data until ctrl-c.

  return 0;
}
// %EndTag(MAIN)%
// %EndTag(FULLTEXT)%
```

To run the example:-
```
cd ~/group4_rwa2
catkin_make
```
Now source setup.zsh to load the environment variables.
```zsh
source ~/group4_rwa2/devel/setup.zsh
source ~/group4_rwa2/devel/setup.bash #for bash users
```

To launch the competition with our configuration:-
```
rosrun osrf_gear gear.py -f `catkin_find --first-only --share osrf_gear`/config/sample.yaml ~/group4_rwa2/src/ariac_example/config/sample_gear_conf.yaml

```

# References
1. Installing ARIAC - https://bitbucket.org/osrf/ariac/wiki/2019/tutorials/installation
2. Creating a Competition Package - https://bitbucket.org/osrf/ariac/wiki/2019/tutorials/hello_world
 
