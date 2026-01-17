#include "AppTask.hpp"

#include "RosTask.hpp"
#include "CanTask.hpp"
#include "StateManager.hpp"
#include "EbsManager.hpp"

#include "FreeRTOS.h"
#include "task.h"
#include <atomic>

void reset_ros_globals()
{
    g_mission_going_cmd.store(false);
    g_accel_cmd.store(0.0f);
    g_steer_cmd.store(0.0f);
    g_emergency_cmd.store(false);
    g_finished_cmd.store(false);
}

void reset_can_globals()
{
    g_can_listen_go.store(false);
    g_can_mission_id.store(0);
    g_can_r2d.store(false);
    g_can_vehicle_standstill.store(true);
}

void reset_all()
{
    if (g_mission_going_cmd.load()) {
        send_cancel_mission_command();
    }

    reset_ros_globals();
    reset_can_globals();

    EbsManager::getInstance().reset();
    StateManager::getInstance().reset();

    if (g_ros_cmd_queue != nullptr) {
        xQueueReset(g_ros_cmd_queue);
    }
    if (g_can_cmd_queue != nullptr) {
        xQueueReset(g_can_cmd_queue);
    }

    g_reset_cmd.store(false);
}


void AppTask(void *argument)
{
    (void)argument;


    EbsManager& ebs = EbsManager::getInstance();
    StateManager& stateManager = StateManager::getInstance();

    EBSInitState ebsState{EBSInitState::Start};
    ASState asState{ASState::OFF};

    uint32_t ready_start_time{0};

    // Wait until CanTask creates queue
    while (g_can_cmd_queue == nullptr)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    // Wait until RosTask creates queue
    while (g_ros_cmd_queue == nullptr)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    while (true)
    {
        if (g_reset_cmd.load()) {
            reset_all();
        }

    	stateManager.update();

    	if (ebsState != EBSInitState::WaitLow) {
    		HardwareIO::toggle_watchdog(); // Imposes a max 50ms for the loop or the car will stop
    	}

    	if (stateManager.getSignals().asms_on) {
            if (asState != ASState::READY) {
                ready_start_time = 0;
                g_can_listen_go.store(false);
            }
            if (asState != ASState::DRIVING) {
                send_can_control(0.0f, 0.0f); // Not sure if this is necessary
            }

    		switch (asState) {
				case ASState::OFF:
					if (ebsState != EBSInitState::Done && ebsState != EBSInitState::Failed) {
						ebsState = ebs.initSequenceStep();
					}
					break;

				case ASState::READY:
					if (ready_start_time == 0) {
						ready_start_time = HardwareIO::now_ms();
					}
					else if (HardwareIO::now_ms() - ready_start_time > 5000) {
                        g_can_listen_go.store(true);
					}
					break;

				case ASState::DRIVING:
					if (!g_mission_going_cmd.load()) {
                        // We know that the mission id must be set to some different to 0 as we are in the driving state
						send_start_mission_command(g_can_mission_id.load());
					}
                    else {
                        if (g_emergency_cmd.load() || g_finished_cmd.load()) {
                            ebs.activateEBS();

                            if (g_mission_going_cmd.load()) {
                                send_cancel_mission_command();
                            }
                        }
                        else {
                            // We don't care if we are repeating commands as the ECU expects a continuous stream of orders
                            send_can_control(g_accel_cmd.load(), g_steer_cmd.load());
                        }
                    }
					break;

				case ASState::EMERGENCY:
                    // When entering this state EBS should already be active, 
                    // but just in case
                    ebs.activateEBS();

                    if (g_mission_going_cmd.load()) {
                        send_cancel_mission_command();
                    }
					break;

				case ASState::FINISHED:
                    // EBS should already be active, but it is not activated again 
                    // in case it could interefere with other operations after the run

                    // Check to cancel ros mission just in case
                    if (g_mission_going_cmd.load()) {
                        send_cancel_mission_command();
                    }
					break;
			}
    	}
    	else {
    		ebs.SafeManual(); // Checks pressure storages are empty
    	}
        // simulate 10ms loop
        // vTaskDelay(pdMS_TO_TICKS(10));
    }
}
