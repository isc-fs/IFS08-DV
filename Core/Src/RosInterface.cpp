#include "RosInterface.hpp"
#include <string.h>
#include <stdlib.h>

RosInterface::RosInterface() {
    // Zero-initialize all members
    memset(&support, 0, sizeof(support));
    allocator = rcl_get_default_allocator();
    memset(&node, 0, sizeof(node));
    memset(&executor, 0, sizeof(executor));
    memset(&set_mission_client, 0, sizeof(set_mission_client));
    memset(&req_set_mission, 0, sizeof(req_set_mission));
    memset(&res_set_mission, 0, sizeof(res_set_mission));
    memset(&start_mission_client, 0, sizeof(start_mission_client));
    memset(&goal_start_mission, 0, sizeof(goal_start_mission));
    memset(&msg_feedback, 0, sizeof(msg_feedback));
    current_goal_handle = NULL;
}

RosInterface::~RosInterface() {
    // Clean up all allocated resources
    rcl_client_fini(&set_mission_client, &node);
    rclc_action_client_fini(&start_mission_client, &node);
    rclc_executor_fini(&executor);
    rcl_node_fini(&node);
    rclc_support_fini(&support);
}

void RosInterface::init() {
    // Initialize micro-ROS support
    rclc_support_init(&support, 0, NULL, &allocator);

    // Initialize node
    rclc_node_init_default(&node, "supervisor_node", "", &support);

    // Initialize service client
    rclc_client_init_default(&set_mission_client,
                             &node,
                             ROSIDL_GET_SRV_TYPE_SUPPORT(ros2_interface, srv, SetMission), // this does need srv in the middle
                             "set_mission");

    // Initialize action client
    rclc_action_client_init_default(&start_mission_client,
                                    &node,
                                    ROSIDL_GET_ACTION_TYPE_SUPPORT(ros2_interface, StartMission), // idk why but it doesn't need action in the middle
                                    "start_mission");

    // Initialize executor
    rclc_executor_init(&executor, &support.context, 3, &allocator);

    // Add service to executor
    rclc_executor_add_client(&executor, &set_mission_client, &res_set_mission, &RosInterface::set_mission_callback);

    // Add feedback and result callbacks to executor
    rclc_executor_add_action_client(&executor,
                                    &start_mission_client,
                                    1,
                                    &res_start_mission,
                                    &msg_feedback,
                                    &RosInterface::start_mission_goal_callback,
                                    &RosInterface::start_mission_feedback_callback,
                                    &RosInterface::start_mission_result_callback,
                                    &RosInterface::start_mission_cancel_callback, // cancel callback place
                                    this);
}

void RosInterface::spin_some() {
    rclc_executor_spin_some(&executor, 5);
}

// ------------------------
// Callbacks
// ------------------------
void RosInterface::set_mission_callback(const void * ros_service_response) {
    // IDEA: ignore this for the moment and hope everything goes right as it will be sent again when the mission starts
    // or try to resend the mission (keep in mind the recursive aspect of it)
    const ros2_interface__srv__SetMission_Response * res = 
        (const ros2_interface__srv__SetMission_Response *)ros_service_response;
    if (res && res->accepted) {
        // Mission accepted - perhaps set a flag or log
    } else {
        // Mission rejected - handle error
    }
}

void RosInterface::start_mission_goal_callback(rclc_action_goal_handle_t * goal_handle, bool accepted, void * context) {
    RosInterface * self = static_cast<RosInterface*>(context);
    if (accepted) {
        // Goal was accepted
        self->current_goal_handle = goal_handle;
        g_mission_going_cmd.store(true);
    } else {
        // Goal was rejected
        // IDEA: set emergency to true (as given the ready signal the car would already be in driving state, 
        // or we could change how the flowchart is processed to dont even get to driving until the mission is accepted, so we would remain in ready, 
        // but this last option could not be under regulation)
        self->current_goal_handle = NULL;
        g_mission_going_cmd.store(false);
    }
}

void RosInterface::start_mission_feedback_callback(rclc_action_goal_handle_t * goal_handle, void * ros_feedback, void * context) {
    (void)goal_handle;
    (void)context;
    ros2_interface__action__StartMission_Feedback * fb =
        (ros2_interface__action__StartMission_Feedback *)ros_feedback;

    if (fb) {
        g_accel_cmd.store(fb->acceleration);
        g_steer_cmd.store(fb->steering);
        g_finished_cmd.store(fb->finished);
        g_emergency_cmd.store(fb->emergency);
    }
}

void RosInterface::start_mission_result_callback(rclc_action_goal_handle_t * goal_handle, void * ros_result, void * context) {
    (void)goal_handle;
    RosInterface * self = static_cast<RosInterface*>(context);
    ros2_interface__action__StartMission_GetResult_Response * res =
        (ros2_interface__action__StartMission_GetResult_Response *)ros_result;

    if (res) {
        g_finished_cmd.store(res->result.finished);
        g_emergency_cmd.store(res->result.emergency);
    }
    g_mission_going_cmd.store(false);
    self->current_goal_handle = NULL; // Reset handle on completion
}

void RosInterface::start_mission_cancel_callback(rclc_action_goal_handle_t * goal_handle, bool cancelled, void * context) {
    RosInterface * self = static_cast<RosInterface*>(context);
    if (cancelled) {
        // Handle cancellation, e.g., stop motors, log, etc.
        // IDEA: set emergency to true
        g_emergency_cmd.store(true);
        g_mission_going_cmd.store(false);
        self->current_goal_handle = NULL; // Reset handle on cancel
    } else {
        // Handle non-cancellation case if needed
    }
}

// ------------------------
// Access to interface
// ------------------------
void RosInterface::call_set_mission_service(int mission_id) {
    ros2_interface__srv__SetMission_Request__init(&req_set_mission);
    req_set_mission.mission_id = mission_id;

    int64_t sequence_number;
    rcl_send_request(&set_mission_client, &req_set_mission, &sequence_number);
}

void RosInterface::send_start_mission_action_goal(int mission_id) {
    ros2_interface__action__StartMission_SendGoal_Request__init(&goal_start_mission);
    goal_start_mission.goal.mission_id = mission_id;
    // Generate a simple UUID (not cryptographically secure, but sufficient for embedded)
    for (int i = 0; i < 16; ++i) {
        goal_start_mission.goal_id.uuid[i] = (uint8_t)rand();
    }

    rclc_action_send_goal_request(&start_mission_client,
                                  &goal_start_mission,
                                  NULL);
}

void RosInterface::cancel_mission_action() {
    if (current_goal_handle != NULL) {
        rclc_action_send_cancel_request(current_goal_handle);
        g_mission_going_cmd.store(false);
        current_goal_handle = NULL; // Reset after cancel
    }
}
