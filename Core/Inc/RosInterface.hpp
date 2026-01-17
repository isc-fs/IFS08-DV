#pragma once

#include "rcl/rcl.h"
#include "rclc/rclc.h"
#include "rclc/executor.h"
#include "rclc/action_client.h"
#include "ros2_interface/action/start_mission.h"
#include "ros2_interface/srv/set_mission.h"

#include "RosGlobals.hpp"

class RosInterface {
public:
    RosInterface();
    ~RosInterface();

    void init();
    void spin_some();

    void call_set_mission_service(int mission_id);
    void send_start_mission_action_goal(int mission_id);
    void cancel_mission_action();

private:
    rclc_support_t support;
    rcl_allocator_t allocator;
    rcl_node_t node;
    rclc_executor_t executor;

    // Service Client
    rcl_client_t set_mission_client;
    ros2_interface__srv__SetMission_Request req_set_mission;
    ros2_interface__srv__SetMission_Response res_set_mission;

    // Action Client
    rclc_action_client_t start_mission_client;
    ros2_interface__action__StartMission_SendGoal_Request goal_start_mission;
    ros2_interface__action__StartMission_GetResult_Response res_start_mission;
    ros2_interface__action__StartMission_Feedback msg_feedback;
    rclc_action_goal_handle_t * current_goal_handle;

    static void start_mission_goal_callback(rclc_action_goal_handle_t * goal_handle, bool accepted, void * context);
    static void start_mission_feedback_callback(rclc_action_goal_handle_t * goal_handle, void * ros_feedback, void * context);
    static void start_mission_result_callback(rclc_action_goal_handle_t * goal_handle, void * ros_result, void * context);
    static void start_mission_cancel_callback(rclc_action_goal_handle_t * goal_handle, bool cancelled, void * context);
    static void set_mission_callback(const void * ros_service_response);
};
