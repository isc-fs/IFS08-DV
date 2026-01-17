#include "RosGlobals.hpp"

// Definitions of atomic feedback from PC, owned by RosTask
std::atomic<float> g_accel_cmd{0.0f};
std::atomic<float> g_steer_cmd{0.0f};
std::atomic<bool>  g_finished_cmd{false};
std::atomic<bool>  g_emergency_cmd{false};
std::atomic<bool>  g_mission_going_cmd{false};

// ROS command queue
QueueHandle_t g_ros_cmd_queue = nullptr;