#pragma once
#include <cstdint>

class HardwareIO {
public:
    static void init();

    // Digital controls
    static void set_AS_close_SDC(bool on);
    static void toggle_watchdog();
    static bool read_SDC_is_ready();
    static bool read_TS_activated();
    static bool read_asms_on();
    static bool read_sdc_res_open();

    // Pressure sensors (ADC)
    static float read_main_storage_pressure();
    static float read_actuator1_storage_pressure();
    static float read_actuator2_storage_pressure();
    static float read_brake_pressure();

    // Actuator control lines
    static void enable_ebs_actuator_1(bool en);
    static void enable_ebs_actuator_2(bool en);

    // IMU
    static void read_imu();

    // Optional: time helpers (wrap HAL_GetTick)
    static uint32_t now_ms();
};
