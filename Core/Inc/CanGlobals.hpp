#pragma once

#include <atomic>
#include "FreeRTOS.h"
#include "queue.h"

// Atomics for simple CAN-derived signals (read-only from other tasks)
extern std::atomic<bool> g_can_r2d;
extern std::atomic<bool> g_can_vehicle_standstill;
extern std::atomic<int>  g_can_mission_id;
extern std::atomic<bool> g_can_listen_go;
extern std::atomic<bool> g_reset_cmd;

// CAN command queue
extern QueueHandle_t g_can_cmd_queue;