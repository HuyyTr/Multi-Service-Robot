#include "sr_kinematic/kinematic.hpp"

void Kinematic::wheelSpeedFeedbackCallBack(const sr_msgs::WheelFeedback::ConstPtr &speed_feedback_msg)
{
    left_wheel_speed_feedback_ = speed_feedback_msg->left_wheel_speed_feedback;
    right_wheel_speed_feedback_ = speed_feedback_msg->right_wheel_speed_feedback;


    if (!is_first_receive)
    {
        last_time = ros::Time::now();
        left_wheel_speed_feedback_prv_ = 0.0;
        right_wheel_speed_feedback_prv_ = 0.0;
        is_first_receive = true;
    }
    current_time = ros::Time::now();
    theta_ = theta_ + ((cR*right_wheel_speed_feedback_prv_ - cL*left_wheel_speed_feedback_prv_)/(cW*wheel_separation_))*(wheel_diameter_/2)*(2*3.1415/60)*(current_time.toSec() - last_time.toSec());
    x_position_ = x_position_ + ((cL*left_wheel_speed_feedback_prv_ + cR*right_wheel_speed_feedback_prv_)/2)*(wheel_diameter_/2)*(2*3.1415/60)*cos(theta_)*(current_time.toSec() - last_time.toSec());
    y_position_ = y_position_ + ((cL*left_wheel_speed_feedback_prv_ + cR*right_wheel_speed_feedback_prv_)/2)*(wheel_diameter_/2)*(2*3.1415/60)*sin(theta_)*(current_time.toSec() - last_time.toSec());
    last_time = ros::Time::now();
    left_wheel_speed_feedback_prv_ = left_wheel_speed_feedback_;
    right_wheel_speed_feedback_prv_ = right_wheel_speed_feedback_;
}

void Kinematic::velocityCommandCallBack(const geometry_msgs::Twist::ConstPtr &twist_msg)
{
    sr_msgs::WheelVelocity vel_msg;
    vel_msg.speed = (twist_msg->linear.x/(wheel_diameter_/2))/(2*3.1415/60);
    vel_msg.steer = (-twist_msg->angular.z*wheel_separation_/(wheel_diameter_/2))/(2*3.1415/60);

    wheel_velocity_pub_.publish(vel_msg);
}

void Kinematic::minimizeImuMessageCallBack(const sr_msgs::MinimizeImuMessage::ConstPtr &minimize_imu_msg)
{
    sensor_msgs::Imu imu_msg;

    imu_msg.header.frame_id = imu_link_frame_;
    imu_msg.header.stamp = ros::Time::now();


    tf::Quaternion tf_quat = tf::createQuaternionFromYaw(minimize_imu_msg->yaw);
    // theta_ = minimize_imu_msg->yaw;
    geometry_msgs::Quaternion msg_quat;
    tf::quaternionTFToMsg(tf_quat, msg_quat);

    imu_msg.orientation = msg_quat;

    imu_msg.linear_acceleration.x = minimize_imu_msg->linear_acceleration[0];
    imu_msg.linear_acceleration.y = minimize_imu_msg->linear_acceleration[1];
    imu_msg.linear_acceleration.z = minimize_imu_msg->linear_acceleration[2];

    imu_msg.angular_velocity.x = minimize_imu_msg->angular_velocity[0];
    imu_msg.angular_velocity.y = minimize_imu_msg->angular_velocity[1];
    imu_msg.angular_velocity.z = minimize_imu_msg->angular_velocity[2];

    imu_pub_.publish(imu_msg);
}

void Kinematic::odomPublisher()
{
    ros::Rate rate(40);
    nav_msgs::Odometry odom_msg;

    while ((nh_.ok()))
    {
        odom_msg.header.stamp = ros::Time::now();
        odom_msg.header.frame_id = odom_link_frame_;
        geometry_msgs::Quaternion odom_quat = tf::createQuaternionMsgFromYaw(theta_);
        odom_msg.pose.pose.position.x = x_position_;
        odom_msg.pose.pose.position.y = -y_position_;
        odom_msg.pose.pose.position.z = 0.0;
        odom_msg.pose.pose.orientation = odom_quat;

        odom_msg.child_frame_id = base_link_frame_;
        odom_msg.twist.twist.linear.x = ((cL*left_wheel_speed_feedback_ + cR*right_wheel_speed_feedback_)/2)*(wheel_diameter_/2)*(2*3.1415/60);
        odom_msg.twist.twist.linear.y = 0.0;
        odom_msg.twist.twist.angular.z = ((cR*right_wheel_speed_feedback_ - cL*left_wheel_speed_feedback_)/(cW*wheel_separation_))*(wheel_diameter_/2)*(2*3.1415/60);
        odom_pub_.publish(odom_msg);
        rate.sleep();
    }
    
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "sr_kinematic");
    ros::NodeHandle nh;
    Kinematic sr_kinematic(nh);
    std::thread odom_pub_thread(&Kinematic::odomPublisher, &sr_kinematic);
    odom_pub_thread.join();

    ros::waitForShutdown();
}