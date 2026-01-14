#ifndef I2C_UTILS_H
#define I2C_UTILS_H

#include "stm32h7xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  I2C_HandleTypeDef *hi2c;

  GPIO_TypeDef *scl_port;
  uint16_t scl_pin;

  GPIO_TypeDef *sda_port;
  uint16_t sda_pin;
} I2C_Utils_t;

void i2c_utils_init(I2C_Utils_t *ctx,
                    I2C_HandleTypeDef *hi2c,
                    GPIO_TypeDef *scl_port, uint16_t scl_pin,
                    GPIO_TypeDef *sda_port, uint16_t sda_pin);

/* Recover stuck I2C bus by toggling SCL and issuing a STOP */
void i2c_utils_bus_recovery(I2C_Utils_t *ctx);

/* Scan the bus, optionally collecting found addresses */
uint8_t i2c_utils_scan(I2C_Utils_t *ctx,
                       uint8_t *found_addrs,
                       uint8_t max_addrs);

/* Basic register helpers */
HAL_StatusTypeDef i2c_utils_read_u8(I2C_Utils_t *ctx,
                                    uint8_t dev7,
                                    uint8_t reg,
                                    uint8_t *val);

HAL_StatusTypeDef i2c_utils_write_u8(I2C_Utils_t *ctx,
                                     uint8_t dev7,
                                     uint8_t reg,
                                     uint8_t val);

HAL_StatusTypeDef i2c_utils_read(I2C_Utils_t *ctx,
                                 uint8_t dev7,
                                 uint8_t reg,
                                 uint8_t *buf,
                                 uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* I2C_UTILS_H */