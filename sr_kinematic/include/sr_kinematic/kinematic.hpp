#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include "sr_msgs/WheelFeedback.h"
#include "sr_msgs/WheelVelocity.h"
#include "sr_msgs/MinimizeImuMessage.h"
#include "nav_msgs/Odometry.h"
#include "sensor_msgs/Imu.h"
#include "tf/transform_broadcaster.h"
#include "tf/tf.h"
#include "tf/transform_datatypes.h"

#include "boost/thread.hpp"
#include "thread"

class Kinematic
{
    public:
        Kinematic(ros::NodeHandle nh)
        :
        nh_(nh),
        spinner_(boost::thread::hardware_concurrency()),
        is_first_receive(false)
        {
            spinner_.start();

            if(!ros::param::get("~wheel_diameter", wheel_diameter_))
            {
                ROS_WARN_STREAM("wheel_diameter param is not defined.");
            }

            if(!ros::param::get("~wheel_separation", wheel_separation_))
            {
                ROS_WARN_STREAM("wheel_saparation param is not defined.");
            }

            if(!ros::param::get("~publish_frequency", publish_frequency_))
            {
                ROS_WARN_STREAM("wheel_saparation param is not defined.Use default value: 10");
                publish_frequency_ = 10;
            }

            if(!ros::param::get("~cR", cR))
            {
                ROS_WARN_STREAM("cR param is not defined.Use default value: 1.0");
                cR = 1.0;
            }
            
            if(!ros::param::get("~cL", cL))
            {
                ROS_WARN_STREAM("cL param is not defined.Use default value: 1.0");
                cL = 1.0;
            }

            if(!ros::param::get("~cW", cW))
            {
                ROS_WARN_STREAM("cW param is not defined.Use default value: 1.0");
                cW = 1.0;
            }

            if(!ros::param::get("~odom_link_frame", odom_link_frame_))
            {
                ROS_WARN_STREAM("odom_link_frame param is not defined.Use default value: odom");
                odom_link_frame_ = "odom";
            }

            if(!ros::param::get("~base_link_frame", base_link_frame_))
            {
                ROS_WARN_STREAM("base_link_frame param is not defined.Use default value: base_link");
                base_link_frame_ = "base_link";
            }

            if(!ros::param::get("~imu_link_frame", imu_link_frame_))
            {
                ROS_WARN_STREAM("imu_link_frame param is not defined.Use default value: imu_link");
                imu_link_frame_ = "imu_link";
            }

            wheel_speed_feedback_sub_ = nh_.subscribe("wheel_speed_feedback", 100, &Kinematic::wheelSpeedFeedbackCallBack, this);

            cmd_vel_sub_ = nh_.subscribe("cmd_vel", 100, &Kinematic::velocityCommandCallBack, this);

            wheel_velocity_pub_ = nh_.advertise<sr_msgs::WheelVelocity>("wheel_velocity", 100);

            odom_pub_ = nh_.advertise<nav_msgs::Odometry>("odom_raw", 100);

            minimize_imu_sub_ = nh_.subscribe("minimize_imu_data", 100, &Kinematic::minimizeImuMessageCallBack, this);

            imu_pub_ = nh_.advertise<sensor_msgs::Imu>("imu", 100);
  
        }

        void wheelSpeedFeedbackCallBack(const sr_msgs::WheelFeedback::ConstPtr &speed_feedback_msg);

        void velocityCommandCallBack(const geometry_msgs::Twist::ConstPtr &twist_msg);

        void minimizeImuMessageCallBack(const sr_msgs::MinimizeImuMessage::ConstPtr &minimize_imu_msg);

        void odomPublisher();
    private:
        ros::NodeHandle nh_;
        ros::Subscriber wheel_speed_feedback_sub_;
        ros::Subscriber cmd_vel_sub_;
        ros::Publisher wheel_velocity_pub_;
        ros::Publisher odom_pub_;
        ros::Subscriber minimize_imu_sub_;
        ros::Publisher imu_pub_;
        ros::AsyncSpinner spinner_;

        ros::Time last_time;
        ros::Time current_time;
        bool is_first_receive;
        double cR, cL, cW;
        double wheel_diameter_;
        double wheel_separation_;
        double publish_frequency_;
        bool publish_tranform_;
        std::string imu_link_frame_;
        std::string odom_link_frame_;
        std::string base_link_frame_;

        double x_position_;
        double y_position_;
        double theta_;

        double left_wheel_speed_feedback_;
        double right_wheel_speed_feedback_;

        double left_wheel_speed_feedback_prv_, right_wheel_speed_feedback_prv_;
};