#include "app_entry.h"

// C++ Task Headers
#include "AppTask.hpp"
#include "RosTask.hpp"
#include "CanTask.hpp"

// FreeRTOS headers are needed here for creating tasks and starting the scheduler
#include "FreeRTOS.h"
#include "task.h"

// This is the definition of the function declared in app_entry.h
extern "C" void cpp_main(void)
{
    // Create APP task
    xTaskCreate(
        AppTask,
        "APP",
        1024,        // stack size
        nullptr,
        3,           // priority
        nullptr
    );

    // Create ROS task
    xTaskCreate(
        RosTask,
        "ROS",
        2048,        // ROS needs more stack
        nullptr,
        2,           // lower priority than APP
        nullptr
    );

    // Create CAN Task
    xTaskCreate(
        CanTask,
        "CAN",
        1024,        // stack size
        nullptr,
        2,           // lower priority than APP
        nullptr
    );

    // Start the FreeRTOS scheduler.
    // This call never returns.
    vTaskStartScheduler();

    // The program should never reach here.
    for (;;) {}
}