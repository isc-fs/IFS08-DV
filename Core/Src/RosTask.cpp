#include "RosTask.hpp"
#include "RosInterface.hpp"

// ------------------------
// RosTask
// ------------------------
extern "C" void RosTask(void *argument)
{
    (void)argument;
    // Create and initialize the ROS interface
    static RosInterface ros_if;
    ros_if.init();

    // Create the queue for AppTask to send commands
    g_ros_cmd_queue = xQueueCreate(4, sizeof(RosCommandMessage));


    RosCommandMessage msg;

    while(1)
    {
        // Process commands from AppTask
        if (xQueueReceive(g_ros_cmd_queue, &msg, pdMS_TO_TICKS(5)) == pdPASS)
        {
            switch(msg.cmd)
            {
                case ROS_CMD_SET_MISSION:
                    ros_if.call_set_mission_service(msg.mission_id);
                    break;

                case ROS_CMD_START_MISSION:
                    ros_if.send_start_mission_action_goal(msg.mission_id);
                    break;

                case ROS_CMD_CANCEL_MISSION:
                    ros_if.cancel_mission_action();
                    break;

                default:
                    break;
            }
        }

        // Spin the ROS executor
        ros_if.spin_some();
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}
