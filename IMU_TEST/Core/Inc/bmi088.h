#ifndef BMI088_H
#define BMI088_H

#include "stm32h7xx_hal.h"
#include "i2c_utils.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BMI088_ACC_ADDR_7B  0x18u
#define BMI088_GYR_ADDR_7B  0x68u

typedef enum {
  BMI088_OK = 0,
  BMI088_ERR_PARAM = -1,
  BMI088_ERR_I2C = -2,
  BMI088_ERR_ID = -3
} bmi088_status_t;

typedef struct {
  I2C_Utils_t *bus;
  uint8_t acc_addr_7b;
  uint8_t gyr_addr_7b;

  float acc_lsb_per_g;     /* matches the configured range */
  float gyr_lsb_per_dps;   /* matches the configured range */
} bmi088_t;

typedef struct {
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
} bmi088_raw_t;

typedef struct {
  float ax_g, ay_g, az_g;
  float gx_dps, gy_dps, gz_dps;
} bmi088_scaled_t;

bmi088_status_t bmi088_bind(bmi088_t *dev, I2C_Utils_t *bus,
                            uint8_t acc_addr_7b, uint8_t gyr_addr_7b);

bmi088_status_t bmi088_read_ids(bmi088_t *dev, uint8_t *acc_id, uint8_t *gyr_id);

bmi088_status_t bmi088_init_minimal(bmi088_t *dev);

bmi088_status_t bmi088_read_raw(bmi088_t *dev, bmi088_raw_t *out);

void bmi088_convert_scaled(const bmi088_t *dev,
                           const bmi088_raw_t *raw,
                           bmi088_scaled_t *out);

bmi088_status_t bmi088_read_scaled(bmi088_t *dev, bmi088_scaled_t *out);

#ifdef __cplusplus
}
#endif

#endif /* BMI088_H */