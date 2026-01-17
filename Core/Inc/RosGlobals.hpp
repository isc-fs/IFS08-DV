#pragma once

#include <atomic>
#include "FreeRTOS.h"
#include "queue.h"

// Atomic feedback from PC, owned by RosTask
extern std::atomic<float> g_accel_cmd;
extern std::atomic<float> g_steer_cmd;
extern std::atomic<bool>  g_finished_cmd;
extern std::atomic<bool>  g_emergency_cmd;
extern std::atomic<bool>  g_mission_going_cmd;

// ROS command queue
extern QueueHandle_t g_ros_cmd_queue;