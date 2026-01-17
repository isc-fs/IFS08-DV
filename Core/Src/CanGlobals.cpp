#include "CanGlobals.hpp"

// Definitions of CAN atomic signals
std::atomic<bool> g_can_r2d{false};
std::atomic<bool> g_can_vehicle_standstill{true};
std::atomic<int>  g_can_mission_id{0};
std::atomic<bool>  g_can_listen_go{false};
std::atomic<bool> g_reset_cmd{false};

// CAN command queue
QueueHandle_t g_can_cmd_queue = nullptr;