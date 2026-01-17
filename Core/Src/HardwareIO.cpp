#include "HardwareIO.hpp"
#include "main.h" // include CubeMX generated HAL handles (hcan, huart, etc.)
#include "stm32h7xx_hal.h" // adjust to your series

void HardwareIO::init() {
    // Nothing here — CubeMX init runs before main loop.
}

void HardwareIO::toggle_watchdog() {
    // Toggle a pin used as watchdog signal to vehicle
}

void HardwareIO::set_AS_close_SDC(bool on) {
    // HAL_GPIO_WritePin(GPIOX, GPIO_PIN_Y, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

bool HardwareIO::read_TS_activated() {
	// return HAL_GPIO_ReadPin(GPIO_TS_PORT, GPIO_TS_PIN) == GPIO_PIN_SET;
	return false;
}

bool HardwareIO::read_SDC_is_ready() {
    // pin 4
    // return HAL_GPIO_ReadPin(GPIOX, GPIO_PIN_Y) == GPIO_PIN_SET;
    return false;
}

bool HardwareIO::read_asms_on() {
    // return HAL_GPIO_ReadPin(GPIOX, GPIO_PIN_Y) == GPIO_PIN_SET;
    return false;
}

bool HardwareIO::read_sdc_res_open() {
    // return HAL_GPIO_ReadPin(GPIOX, GPIO_PIN_Y) == GPIO_PIN_SET;
    return false;
}


float HardwareIO::read_main_storage_pressure() {
    // Read ADC channel, convert to pressure units
    return 0.0f;
}

float HardwareIO::read_actuator1_storage_pressure() { return 0.0f; }
float HardwareIO::read_actuator2_storage_pressure() { return 0.0f; }
float HardwareIO::read_brake_pressure() { return 0.0f; }

void HardwareIO::enable_ebs_actuator_1(bool en) {
    // HAL_GPIO_WritePin(...)
}
void HardwareIO::enable_ebs_actuator_2(bool en) {
    // HAL_GPIO_WritePin(...)
}

// TODO: add function to read IMU data
void HardwareIO::read_imu() {
    
}

uint32_t HardwareIO::now_ms() {
    return HAL_GetTick();
}
