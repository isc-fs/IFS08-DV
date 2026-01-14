#include "attitude.h"

#define PI_F    3.14159265358979323846f
#define DEG2RAD 0.01745329251994329577f

static inline int32_t f32_to_q31(float x)
{
  if (x >= 0.99999994f) x = 0.99999994f;
  if (x <= -1.0f)       x = -1.0f;
  return (int32_t)(x * 2147483648.0f);
}

static inline float q31_to_f32(int32_t q)
{
  return ((float)q) / 2147483648.0f;
}

/* CORDIC PHASE: returns atan2(y, x) and modulus sqrt(x^2 + y^2) */
static void cordic_phase_mag_f32(CORDIC_HandleTypeDef *hc,
                                 float y, float x,
                                 float *angle_rad, float *mag)
{
  int32_t in[2], out[2];

  /* Clamp to Q1.31 input range */
  if (x >  0.999f) x =  0.999f;
  if (x < -0.999f) x = -0.999f;
  if (y >  0.999f) y =  0.999f;
  if (y < -0.999f) y = -0.999f;

  in[0] = f32_to_q31(x);
  in[1] = f32_to_q31(y);

  static uint8_t configured = 0;
  if (!configured) {
    CORDIC_ConfigTypeDef cfg = {0};
    cfg.Function  = CORDIC_FUNCTION_PHASE;
    cfg.Precision = CORDIC_PRECISION_6CYCLES;
    cfg.Scale     = CORDIC_SCALE_0;
    cfg.NbWrite   = CORDIC_NBWRITE_2;
    cfg.NbRead    = CORDIC_NBREAD_2;
    cfg.InSize    = CORDIC_INSIZE_32BITS;
    cfg.OutSize   = CORDIC_OUTSIZE_32BITS;
    (void)HAL_CORDIC_Configure(hc, &cfg);
    configured = 1;
  }

  (void)HAL_CORDIC_Calculate(hc, in, out, 1, 10);

  /* Typical STM32 HAL scaling for PHASE: q31 * PI gives radians in [-pi, pi) */
  float theta = q31_to_f32(out[0]) * PI_F;
  float r     = q31_to_f32(out[1]);

  if (angle_rad) *angle_rad = theta;
  if (mag)       *mag = r;
}

void attitude_init(attitude_t *a, float alpha)
{
  if (!a) return;
  a->roll = 0.0f;
  a->pitch = 0.0f;
  a->alpha = alpha;
  a->gx_off_dps = 0.0f;
  a->gy_off_dps = 0.0f;
  a->gz_off_dps = 0.0f;
  a->last_ms = 0;
}

void attitude_set_gyro_bias_dps(attitude_t *a, float gx, float gy, float gz)
{
  if (!a) return;
  a->gx_off_dps = gx;
  a->gy_off_dps = gy;
  a->gz_off_dps = gz;
}

void attitude_update_cordic(attitude_t *a,
                            CORDIC_HandleTypeDef *hc,
                            float ax_g, float ay_g, float az_g,
                            float gx_dps, float gy_dps, float gz_dps)
{
  if (!a || !hc) return;

  gx_dps -= a->gx_off_dps;
  gy_dps -= a->gy_off_dps;
  gz_dps -= a->gz_off_dps;
  (void)gz_dps;

  uint32_t now = HAL_GetTick();
  if (a->last_ms == 0) a->last_ms = now;

  float dt = (now - a->last_ms) * 0.001f;
  a->last_ms = now;
  if (dt <= 0.0f || dt > 0.1f) dt = 0.01f;

  float roll_acc = 0.0f, mag_yz = 0.0f;
  cordic_phase_mag_f32(hc, ay_g, az_g, &roll_acc, &mag_yz);

  float den = (mag_yz < 1e-3f) ? 1e-3f : mag_yz;
  float pitch_acc = 0.0f;
  cordic_phase_mag_f32(hc, -ax_g, den, &pitch_acc, NULL);

  float gx = gx_dps * DEG2RAD;
  float gy = gy_dps * DEG2RAD;

  a->roll  = a->alpha * (a->roll  + gx * dt) + (1.0f - a->alpha) * roll_acc;
  a->pitch = a->alpha * (a->pitch + gy * dt) + (1.0f - a->alpha) * pitch_acc;
}