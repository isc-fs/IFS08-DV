#pragma once

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "RosGlobals.hpp"

// FreeRTOS entry point
extern "C" void RosTask(void *argument);

// Commands from AppTask to RosTask
typedef enum {
    ROS_CMD_NONE = 0,
    ROS_CMD_SET_MISSION,
    ROS_CMD_START_MISSION,
    ROS_CMD_CANCEL_MISSION
} RosCmd;

typedef struct {
    RosCmd cmd;
    int mission_id;
} RosCommandMessage;

// Helper functions to send commands to the ROS queue
static inline void send_set_mission_command(int mission_id)
{
    if (g_ros_cmd_queue == NULL) return;

    RosCommandMessage msg;
    msg.cmd = ROS_CMD_SET_MISSION;
    msg.mission_id = mission_id;

    xQueueSend(g_ros_cmd_queue, &msg, 0);
}

static inline void send_start_mission_command(int mission_id)
{
    if (g_ros_cmd_queue == NULL) return;

    RosCommandMessage msg;
    msg.cmd = ROS_CMD_START_MISSION;
    msg.mission_id = mission_id;

    xQueueSend(g_ros_cmd_queue, &msg, 0);
}

static inline void send_cancel_mission_command()
{
    if (g_ros_cmd_queue == NULL) return;

    RosCommandMessage msg;
    msg.cmd = ROS_CMD_CANCEL_MISSION;
    msg.mission_id = 0; // not used

    xQueueSend(g_ros_cmd_queue, &msg, 0);
}
