#pragma once
#include "FreeRTOS.h"
#include "queue.h"
#include <cstdint>
#include "CanGlobals.hpp"

// Task entry (use xTaskCreate with this)
extern "C" void CanTask(void *argument);

// CAN command types
enum class CanCmd : uint8_t {
    NONE = 0,
    SEND_CONTROL,
};

struct CanCommandMessage {
    CanCmd cmd;
    float accel;
    float steer;
};

// Helpers to send commands to the CAN queue
static inline void send_can_control(float accel, float steer)
{
    if (g_can_cmd_queue == NULL) return; // queue not ready

    CanCommandMessage msg;
    msg.cmd = CanCmd::SEND_CONTROL;
    msg.accel = accel;
    msg.steer = steer;

    xQueueSend(g_can_cmd_queue, &msg, 0);
}
