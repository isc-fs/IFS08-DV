#include "imu_service.h"

void imu_service_init(imu_service_t *s,
                      I2C_HandleTypeDef *i2c,
                      CORDIC_HandleTypeDef *cordic,
                      float alpha)
{
  s->bmi.i2c = i2c;
  s->bmi.acc_addr_7b = BMI088_ACC_ADDR_7B;
  s->bmi.gyr_addr_7b = BMI088_GYR_ADDR_7B;
  s->cordic = cordic;
  attitude_init(&s->att, alpha);
}

bmi088_status_t imu_service_start(imu_service_t *s)
{
  return bmi088_init_minimal(&s->bmi);
}

bmi088_status_t imu_service_step(imu_service_t *s, imu_sample_t *out)
{
  if (!s || !out) return BMI088_ERR_PARAM;

  bmi088_scaled_t meas;
  bmi088_status_t st = bmi088_read_scaled(&s->bmi, &meas);
  if (st != BMI088_OK) return st;

  attitude_update_cordic(&s->att, s->cordic,
                         meas.ax_g, meas.ay_g, meas.az_g,
                         meas.gx_dps, meas.gy_dps, meas.gz_dps);

  out->t_ms = HAL_GetTick();
  out->imu = meas;
  out->roll_deg = attitude_roll_deg(&s->att);
  out->pitch_deg = attitude_pitch_deg(&s->att);
  return BMI088_OK;
}