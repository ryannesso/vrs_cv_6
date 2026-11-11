#include "lps25hb.h"

static uint8_t read_registers(LPS25HB_Handle *sensor, uint8_t reg, uint8_t *data, uint16_t length)
{
  return sensor->read(sensor->reference_pressure_hpa_x100 == 0 ? LPS25HB_ADDRESS : LPS25HB_ADDRESS,
                      (length > 1U) ? (uint8_t)(reg | 0x80U) : reg, data, length);
}

void LPS25HB_RegisterBus(LPS25HB_Handle *sensor, SensorReadCallback read, SensorWriteCallback write)
{
  sensor->read = read;
  sensor->write = write;
}

uint8_t LPS25HB_Init(LPS25HB_Handle *sensor)
{
  uint8_t who_am_i;
  uint8_t control = 0x90U; /* power on, 1 Hz output data rate, BDU */
  if ((sensor->read == 0) || (sensor->write == 0)) return 0U;
  if (!read_registers(sensor, LPS25HB_WHO_AM_I, &who_am_i, 1U) ||
      who_am_i != LPS25HB_WHO_AM_I_VAL) return 0U;
  if (!sensor->write(LPS25HB_ADDRESS, LPS25HB_CTRL_REG1, &control, 1U)) return 0U;
  sensor->reference_pressure_hpa_x100 = 0;
  return 1U;
}

uint8_t LPS25HB_ReadPressure(LPS25HB_Handle *sensor, int32_t *pressure_hpa_x100)
{
  uint8_t raw_bytes[3];
  int32_t raw;
  if (!read_registers(sensor, LPS25HB_PRESS_OUT_XL, raw_bytes, 3U)) return 0U;
  raw = (int32_t)raw_bytes[0] | ((int32_t)raw_bytes[1] << 8) |
        ((int32_t)raw_bytes[2] << 16);
  if ((raw & 0x00800000L) != 0) raw |= (int32_t)0xFF000000L;
  *pressure_hpa_x100 = (raw * 100L) / 4096L;
  if (sensor->reference_pressure_hpa_x100 == 0)
    sensor->reference_pressure_hpa_x100 = *pressure_hpa_x100;
  return 1U;
}

int32_t LPS25HB_GetRelativeAltitudeCm(const LPS25HB_Handle *sensor, int32_t pressure_hpa_x100)
{
  int32_t difference = sensor->reference_pressure_hpa_x100 - pressure_hpa_x100;
  if (sensor->reference_pressure_hpa_x100 == 0) return 0;
  /* Local linear approximation: about 8.43 m per hPa near sea level. */
  return (difference * 843L) / 100L;
}
