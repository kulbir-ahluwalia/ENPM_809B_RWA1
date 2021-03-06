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
#include <tf2_ros/transform_listener.h>
#include <geometry_msgs/TransformStamped.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>

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
      "Logical camera has: '" << image_msg->models.size() << "' objects.");

   for(int i=0;i<image_msg->models.size();i++)
	{if (ros::ok==0)
      {return;}

   //std::cout<<"Type of object is-"<<image_msg->models[i].type<<std::endl;
	 //std::cout<<"No of the object is- "<<i<<std::endl;
	 //std::cout<<"Pose is - ["<<image_msg->models[i].pose.position.x<<","<<image_msg->models[i].pose.position.y<<","<<image_msg->models[i].pose.position.z<<"] ["<<image_msg->models[i].pose.orientation.x<<","	<<image_msg->models[i].pose.orientation.y<<","<<image_msg->models[i].pose.orientation.z<<","<<image_msg->models[i].pose.orientation.w<<"]"<<std::endl;

 geometry_msgs::TransformStamped point_out;
 geometry_msgs::TransformStamped point_in;


  point_in.transform.translation.x=image_msg->models[i].pose.position.x;
  point_in.transform.translation.y=image_msg->models[i].pose.position.y;
  point_in.transform.translation.z=image_msg->models[i].pose.position.z;
  point_in.transform.rotation.x=image_msg->models[i].pose.orientation.x;
  point_in.transform.rotation.y=image_msg->models[i].pose.orientation.y;
  point_in.transform.rotation.z=image_msg->models[i].pose.orientation.z;
  point_in.transform.rotation.w=image_msg->models[i].pose.orientation.w;
  point_in.header.seq=123;
  point_in.header.stamp=ros::Time::now()-ros::Duration(5);
  point_in.header.frame_id="logical_camera_4_frame";
  //point_in.child_frame_id="Nonei";


  tf2_ros::Buffer tfBuffer;
  tf2_ros::TransformListener tfListener(tfBuffer);

  ros::Rate rate(10);
  ros::Duration timeout(5.0);


std::string name=image_msg->models[i].type;
geometry_msgs::TransformStamped transformStamped;
name[33]='0'+i+1;

while(1)
    {point_in.header.stamp=ros::Time::now()-ros::Duration(0.11);
    try{
      //transformStamped = tfBuffer.lookupTransform("world", name,ros::Time(0), timeout);
      tfBuffer.transform(point_in,point_out,"world");
      break;
    }
    catch (tf2::TransformException &ex) {
      //std::cout<<ex.what()<<std::endl;
      ros::Duration(0.2).sleep();
      //std::cout<<"fail"<<std::endl;
      continue;

    }

}   std::cout<<"Object is a -"<<image_msg->models[i].type<<" and is numbered "<<i<<std::endl;
     tf2::Quaternion q(
      point_out.transform.rotation.x,
      point_out.transform.rotation.y,
      point_out.transform.rotation.z,
      point_out.transform.rotation.w);

   tf2::Matrix3x3 m(q);
   double roll, pitch, yaw;
   m.getRPY(roll, pitch, yaw);
std::cout<<"["<<point_out.transform.translation.x<<","<<point_out.transform.translation.y<<","<<point_out.transform.translation.z<<"]"<<"["<<roll<<","<<pitch<<","<<yaw<<"]"<<std::endl;
   

}

}

void logical_camera_callback_1(
  const osrf_gear::LogicalCameraImage::ConstPtr & image_msg)
{
  ROS_INFO_STREAM_THROTTLE(10,
    "Logical camera has: '" << image_msg->models.size() << "' objects.");

 for(int i=0;i<image_msg->models.size();i++)
{if (ros::ok==0)
    {return;}

 //std::cout<<"Type of object is-"<<image_msg->models[i].type<<std::endl;
 //std::cout<<"No of the object is- "<<i<<std::endl;
 //std::cout<<"Pose is - ["<<image_msg->models[i].pose.position.x<<","<<image_msg->models[i].pose.position.y<<","<<image_msg->models[i].pose.position.z<<"] ["<<image_msg->models[i].pose.orientation.x<<","	<<image_msg->models[i].pose.orientation.y<<","<<image_msg->models[i].pose.orientation.z<<","<<image_msg->models[i].pose.orientation.w<<"]"<<std::endl;

geometry_msgs::TransformStamped point_out;
geometry_msgs::TransformStamped point_in;


point_in.transform.translation.x=image_msg->models[i].pose.position.x;
point_in.transform.translation.y=image_msg->models[i].pose.position.y;
point_in.transform.translation.z=image_msg->models[i].pose.position.z;
point_in.transform.rotation.x=image_msg->models[i].pose.orientation.x;
point_in.transform.rotation.y=image_msg->models[i].pose.orientation.y;
point_in.transform.rotation.z=image_msg->models[i].pose.orientation.z;
point_in.transform.rotation.w=image_msg->models[i].pose.orientation.w;
point_in.header.seq=123;
point_in.header.stamp=ros::Time::now()-ros::Duration(5);
point_in.header.frame_id="logical_camera_1_frame";
//point_in.child_frame_id="Nonei";


tf2_ros::Buffer tfBuffer;
tf2_ros::TransformListener tfListener(tfBuffer);

ros::Rate rate(10);
ros::Duration timeout(5.0);


std::string name=image_msg->models[i].type;
geometry_msgs::TransformStamped transformStamped;
name[33]='0'+i+1;

while(1)
  {point_in.header.stamp=ros::Time::now()-ros::Duration(0.11);
  try{
    //transformStamped = tfBuffer.lookupTransform("world", name,ros::Time(0), timeout);
    tfBuffer.transform(point_in,point_out,"world");
    break;
  }
  catch (tf2::TransformException &ex) {
    //std::cout<<ex.what()<<std::endl;
    ros::Duration(0.2).sleep();
    //std::cout<<"fail"<<std::endl;
    continue;

  }

}  std::cout<<"Object is a -"<<image_msg->models[i].type<<" and is numbered "<<i<<std::endl; 
   tf2::Quaternion q(
    point_out.transform.rotation.x,
    point_out.transform.rotation.y,
    point_out.transform.rotation.z,
    point_out.transform.rotation.w);

 tf2::Matrix3x3 m(q);
 double roll, pitch, yaw;
 m.getRPY(roll, pitch, yaw);

std::cout<<"["<<point_out.transform.translation.x<<","<<point_out.transform.translation.y<<","<<point_out.transform.translation.z<<"]"<<"["<<roll<<","<<pitch<<","<<yaw<<"]"<<std::endl;
 
}

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
    "/ariac/logical_camera_4", 10,
    &MyCompetitionClass::logical_camera_callback, &comp_class);

    ros::Subscriber logical_camera_subscriber_1 = node.subscribe(
      "/ariac/logical_camera_1", 10,
      &MyCompetitionClass::logical_camera_callback_1, &comp_class);

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
