#pragma once
#include "stm32h7xx_hal.h"
#include "bmi088.h"
#include "attitude.h"

typedef struct {
  bmi088_t bmi;
  attitude_t att;
  CORDIC_HandleTypeDef *cordic;
} imu_service_t;

typedef struct {
  uint32_t t_ms;
  bmi088_scaled_t imu;
  float roll_deg;
  float pitch_deg;
} imu_sample_t;

void imu_service_init(imu_service_t *s,
                      I2C_HandleTypeDef *i2c,
                      CORDIC_HandleTypeDef *cordic,
                      float alpha);

bmi088_status_t imu_service_start(imu_service_t *s);

// Call periodically from a task/timer (e.g., 100 Hz)
bmi088_status_t imu_service_step(imu_service_t *s, imu_sample_t *out);