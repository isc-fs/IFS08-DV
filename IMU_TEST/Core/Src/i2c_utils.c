#include "i2c_utils.h"

void i2c_utils_init(I2C_Utils_t *ctx,
                    I2C_HandleTypeDef *hi2c,
                    GPIO_TypeDef *scl_port, uint16_t scl_pin,
                    GPIO_TypeDef *sda_port, uint16_t sda_pin)
{
  ctx->hi2c = hi2c;
  ctx->scl_port = scl_port;
  ctx->scl_pin = scl_pin;
  ctx->sda_port = sda_port;
  ctx->sda_pin = sda_pin;
}

void i2c_utils_bus_recovery(I2C_Utils_t *ctx)
{
  GPIO_InitTypeDef g = {0};

  /* De-init the peripheral to release the pins */
  HAL_I2C_DeInit(ctx->hi2c);

  /* Configure SCL/SDA as open-drain outputs with internal pull-ups */
  g.Mode = GPIO_MODE_OUTPUT_OD;
  g.Pull = GPIO_PULLUP;
  g.Speed = GPIO_SPEED_FREQ_HIGH;

  g.Pin = ctx->scl_pin;
  HAL_GPIO_Init(ctx->scl_port, &g);

  g.Pin = ctx->sda_pin;
  HAL_GPIO_Init(ctx->sda_port, &g);

  /* Release both lines */
  HAL_GPIO_WritePin(ctx->scl_port, ctx->scl_pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(ctx->sda_port, ctx->sda_pin, GPIO_PIN_SET);
  HAL_Delay(2);

  /* Toggle SCL 9 times */
  for (int i = 0; i < 9; i++) {
    HAL_GPIO_WritePin(ctx->scl_port, ctx->scl_pin, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(ctx->scl_port, ctx->scl_pin, GPIO_PIN_SET);
    HAL_Delay(1);
  }

  /* Generate STOP: SDA low -> SCL high -> SDA high */
  HAL_GPIO_WritePin(ctx->sda_port, ctx->sda_pin, GPIO_PIN_RESET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(ctx->scl_port, ctx->scl_pin, GPIO_PIN_SET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(ctx->sda_port, ctx->sda_pin, GPIO_PIN_SET);
  HAL_Delay(1);

  /* Re-init I2C */
  HAL_I2C_Init(ctx->hi2c);
}

uint8_t i2c_utils_scan(I2C_Utils_t *ctx,
                       uint8_t *found_addrs,
                       uint8_t max_addrs)
{
  uint8_t count = 0;

  for (uint8_t addr = 0x08; addr <= 0x77; addr++) {

    if (HAL_I2C_GetState(ctx->hi2c) != HAL_I2C_STATE_READY) {
      i2c_utils_bus_recovery(ctx);
    }

    if (HAL_I2C_IsDeviceReady(ctx->hi2c,
                              (uint16_t)(addr << 1),
                              1, 20) == HAL_OK) {
      if (found_addrs && count < max_addrs) {
        found_addrs[count] = addr;
      }
      count++;
    }

    HAL_Delay(2);
  }

  return count;
}

HAL_StatusTypeDef i2c_utils_read_u8(I2C_Utils_t *ctx,
                                    uint8_t dev7,
                                    uint8_t reg,
                                    uint8_t *val)
{
  return HAL_I2C_Mem_Read(ctx->hi2c,
                          (uint16_t)(dev7 << 1),
                          reg,
                          I2C_MEMADD_SIZE_8BIT,
                          val,
                          1,
                          100);
}

HAL_StatusTypeDef i2c_utils_write_u8(I2C_Utils_t *ctx,
                                     uint8_t dev7,
                                     uint8_t reg,
                                     uint8_t val)
{
  return HAL_I2C_Mem_Write(ctx->hi2c,
                           (uint16_t)(dev7 << 1),
                           reg,
                           I2C_MEMADD_SIZE_8BIT,
                           &val,
                           1,
                           100);
}

HAL_StatusTypeDef i2c_utils_read(I2C_Utils_t *ctx,
                                 uint8_t dev7,
                                 uint8_t reg,
                                 uint8_t *buf,
                                 uint16_t len)
{
  return HAL_I2C_Mem_Read(ctx->hi2c,
                          (uint16_t)(dev7 << 1),
                          reg,
                          I2C_MEMADD_SIZE_8BIT,
                          buf,
                          len,
                          100);
}