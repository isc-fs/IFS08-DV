#pragma once
#include <atomic>
#include <cstdint>

// Called by FreeRTOS
void AppTask(void *argument);

// enum class Mission {
// 	ManualDriving,
// 	Inspection,
// 	EBSTest,
// 	Trackdrive,
// 	Autocross,
// 	Skidpad,
// 	Acceleration
// };