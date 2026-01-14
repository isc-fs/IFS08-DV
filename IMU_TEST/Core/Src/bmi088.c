#include "bmi088.h"

/* Gyro registers */
#define GYR_CHIP_ID  0x00
#define GYR_RANGE    0x0F
#define GYR_BW       0x10
#define GYR_LPM1     0x11
#define GYR_X_LSB    0x02

/* Accel registers */
#define ACC_CHIP_ID   0x00
#define ACC_CONF      0x40
#define ACC_RANGE     0x41
#define ACC_PWR_CONF  0x7C
#define ACC_PWR_CTRL  0x7D
#define ACC_X_LSB     0x12

#define BMI088_ACC_ID_EXPECTED  0x1Eu
#define BMI088_GYR_ID_EXPECTED  0x0Fu

static bmi088_status_t map_hal(HAL_StatusTypeDef st)
{
  return (st == HAL_OK) ? BMI088_OK : BMI088_ERR_I2C;
}

static bmi088_status_t w_u8(bmi088_t *d, uint8_t dev7, uint8_t reg, uint8_t val)
{
  return map_hal(i2c_utils_write_u8(d->bus, dev7, reg, val));
}

static bmi088_status_t r_u8(bmi088_t *d, uint8_t dev7, uint8_t reg, uint8_t *val)
{
  return map_hal(i2c_utils_read_u8(d->bus, dev7, reg, val));
}

static bmi088_status_t r_buf(bmi088_t *d, uint8_t dev7, uint8_t reg, uint8_t *buf, uint16_t len)
{
  return map_hal(i2c_utils_read(d->bus, dev7, reg, buf, len));
}

static inline int16_t u8_to_i16(uint8_t lsb, uint8_t msb)
{
  return (int16_t)((((uint16_t)msb) << 8) | (uint16_t)lsb);
}

bmi088_status_t bmi088_bind(bmi088_t *dev, I2C_Utils_t *bus,
                            uint8_t acc_addr_7b, uint8_t gyr_addr_7b)
{
  if (!dev || !bus || !bus->hi2c) return BMI088_ERR_PARAM;

  dev->bus = bus;
  dev->acc_addr_7b = acc_addr_7b;
  dev->gyr_addr_7b = gyr_addr_7b;

  /* Defaults matching bmi088_init_minimal() */
  dev->acc_lsb_per_g = 5460.0f;   /* ±6g */
  dev->gyr_lsb_per_dps = 16.4f;   /* ±2000 dps */

  return BMI088_OK;
}

bmi088_status_t bmi088_read_ids(bmi088_t *dev, uint8_t *acc_id, uint8_t *gyr_id)
{
  if (!dev || !dev->bus || !acc_id || !gyr_id) return BMI088_ERR_PARAM;

  bmi088_status_t sa = r_u8(dev, dev->acc_addr_7b, ACC_CHIP_ID, acc_id);
  if (sa != BMI088_OK) return sa;

  bmi088_status_t sg = r_u8(dev, dev->gyr_addr_7b, GYR_CHIP_ID, gyr_id);
  if (sg != BMI088_OK) return sg;

  return BMI088_OK;
}

bmi088_status_t bmi088_init_minimal(bmi088_t *dev)
{
  if (!dev || !dev->bus) return BMI088_ERR_PARAM;

  /* Try bus recovery once if not ready */
  if (HAL_I2C_GetState(dev->bus->hi2c) != HAL_I2C_STATE_READY) {
    i2c_utils_bus_recovery(dev->bus);
  }

  uint8_t acc_id = 0, gyr_id = 0;
  bmi088_status_t sid = bmi088_read_ids(dev, &acc_id, &gyr_id);
  if (sid != BMI088_OK) return sid;

  if (acc_id != BMI088_ACC_ID_EXPECTED || gyr_id != BMI088_GYR_ID_EXPECTED) {
    return BMI088_ERR_ID;
  }

  /* Gyro: normal mode */
  if (w_u8(dev, dev->gyr_addr_7b, GYR_LPM1, 0x00) != BMI088_OK) return BMI088_ERR_I2C;
  HAL_Delay(50);

  /* Gyro: ±2000 dps */
  if (w_u8(dev, dev->gyr_addr_7b, GYR_RANGE, 0x00) != BMI088_OK) return BMI088_ERR_I2C;
  HAL_Delay(2);

  /* Gyro: stable BW/ODR preset */
  if (w_u8(dev, dev->gyr_addr_7b, GYR_BW, 0x02) != BMI088_OK) return BMI088_ERR_I2C;
  HAL_Delay(2);

  /* Accel: active mode */
  if (w_u8(dev, dev->acc_addr_7b, ACC_PWR_CONF, 0x00) != BMI088_OK) return BMI088_ERR_I2C;
  HAL_Delay(10);

  /* Accel: enable (known-good value for your setup) */
  if (w_u8(dev, dev->acc_addr_7b, ACC_PWR_CTRL, 0x04) != BMI088_OK) return BMI088_ERR_I2C;
  HAL_Delay(50);

  /* Accel: ±6g */
  if (w_u8(dev, dev->acc_addr_7b, ACC_RANGE, 0x01) != BMI088_OK) return BMI088_ERR_I2C;
  HAL_Delay(2);

  /* Accel: conservative config (known-good value for your setup) */
  if (w_u8(dev, dev->acc_addr_7b, ACC_CONF, 0xA8) != BMI088_OK) return BMI088_ERR_I2C;
  HAL_Delay(2);

  /* Cache scale factors */
  dev->acc_lsb_per_g = 5460.0f;
  dev->gyr_lsb_per_dps = 16.4f;

  return BMI088_OK;
}

bmi088_status_t bmi088_read_raw(bmi088_t *dev, bmi088_raw_t *out)
{
  if (!dev || !dev->bus || !out) return BMI088_ERR_PARAM;

  uint8_t a[6] = {0};
  uint8_t g[6] = {0};

  bmi088_status_t sa = r_buf(dev, dev->acc_addr_7b, ACC_X_LSB, a, 6);
  if (sa != BMI088_OK) return sa;

  bmi088_status_t sg = r_buf(dev, dev->gyr_addr_7b, GYR_X_LSB, g, 6);
  if (sg != BMI088_OK) return sg;

  out->ax = u8_to_i16(a[0], a[1]);
  out->ay = u8_to_i16(a[2], a[3]);
  out->az = u8_to_i16(a[4], a[5]);

  out->gx = u8_to_i16(g[0], g[1]);
  out->gy = u8_to_i16(g[2], g[3]);
  out->gz = u8_to_i16(g[4], g[5]);

  return BMI088_OK;
}

void bmi088_convert_scaled(const bmi088_t *dev,
                           const bmi088_raw_t *raw,
                           bmi088_scaled_t *out)
{
  if (!dev || !raw || !out) return;

  out->ax_g = (float)raw->ax / dev->acc_lsb_per_g;
  out->ay_g = (float)raw->ay / dev->acc_lsb_per_g;
  out->az_g = (float)raw->az / dev->acc_lsb_per_g;

  out->gx_dps = (float)raw->gx / dev->gyr_lsb_per_dps;
  out->gy_dps = (float)raw->gy / dev->gyr_lsb_per_dps;
  out->gz_dps = (float)raw->gz / dev->gyr_lsb_per_dps;
}

bmi088_status_t bmi088_read_scaled(bmi088_t *dev, bmi088_scaled_t *out)
{
  bmi088_raw_t raw;
  bmi088_status_t st = bmi088_read_raw(dev, &raw);
  if (st != BMI088_OK) return st;

  bmi088_convert_scaled(dev, &raw, out);
  return BMI088_OK;
}