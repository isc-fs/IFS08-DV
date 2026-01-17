#include "CanTask.hpp"
#include "CanInterface.hpp"
#include <atomic>

// ------------------------
// CanTask
// ------------------------
extern "C" void CanTask(void *argument)
{
    (void)argument;
    // Create and initialize the CAN interface
    static CanInterface can;
    can.init();

    // create queue (tune depth)
    g_can_cmd_queue = xQueueCreate(8, sizeof(CanCommandMessage));

    CanCommandMessage msg;

    for (;;)
    {
        // process outgoing requests (short timeout)
        if (xQueueReceive(g_can_cmd_queue, &msg, pdMS_TO_TICKS(10)) == pdPASS)
        {
            switch (msg.cmd)
            {
                case CanCmd::SEND_CONTROL:
                    can.sendControl(msg.accel, msg.steer);
                    break;
                default:
                    break;
            }
        }

        // read and send imu data and set vehicle_standstill atomic
        can.sendIMUData();

        vTaskDelay(pdMS_TO_TICKS(5)); // tune to your timing requirements
    }
}