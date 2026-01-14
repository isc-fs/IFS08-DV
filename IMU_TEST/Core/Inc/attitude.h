#ifndef ATTITUDE_H
#define ATTITUDE_H

#include "stm32h7xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  float roll;     /* rad */
  float pitch;    /* rad */
  float alpha;    /* complementary filter coefficient */
  float gx_off_dps;
  float gy_off_dps;
  float gz_off_dps;
  uint32_t last_ms;
} attitude_t;

void attitude_init(attitude_t *a, float alpha);
void attitude_set_gyro_bias_dps(attitude_t *a, float gx, float gy, float gz);

void attitude_update_cordic(attitude_t *a,
                            CORDIC_HandleTypeDef *hcordic,
                            float ax_g, float ay_g, float az_g,
                            float gx_dps, float gy_dps, float gz_dps);

static inline float attitude_roll_deg(const attitude_t *a)  { return a->roll  * 57.29577951308232f; }
static inline float attitude_pitch_deg(const attitude_t *a) { return a->pitch * 57.29577951308232f; }

#ifdef __cplusplus
}
#endif

#endif /* ATTITUDE_H */