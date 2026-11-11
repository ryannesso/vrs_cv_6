#include "hts221.h"

static uint16_t make_u16(const uint8_t *bytes)
{
  return (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
}

static int16_t make_s16(const uint8_t *bytes)
{
  return (int16_t)make_u16(bytes);
}

static uint8_t read_registers(HTS221_Handle *sensor, uint8_t reg, uint8_t *data, uint16_t length)
{
  return sensor->read(HTS221_ADDRESS, (length > 1U) ? (uint8_t)(reg | 0x80U) : reg, data, length);
}

void HTS221_RegisterBus(HTS221_Handle *sensor, SensorReadCallback read, SensorWriteCallback write)
{
  sensor->read = read;
  sensor->write = write;
}

uint8_t HTS221_Init(HTS221_Handle *sensor)
{
  uint8_t who_am_i;
  uint8_t control = 0x85U; /* power on, 1 Hz, block data update */
  uint8_t calibration[16];
  uint16_t t0_c_x8;
  uint16_t t1_c_x8;
  if ((sensor->read == 0) || (sensor->write == 0)) return 0U;
  if (!read_registers(sensor, HTS221_WHO_AM_I, &who_am_i, 1U) ||
      who_am_i != HTS221_WHO_AM_I_VAL) return 0U;
  if (!sensor->write(HTS221_ADDRESS, HTS221_CTRL_REG1, &control, 1U)) return 0U;
  if (!read_registers(sensor, 0x30U, calibration, sizeof(calibration))) return 0U;

  sensor->h0_rh_x2 = calibration[0];
  sensor->h1_rh_x2 = calibration[1];
  t0_c_x8 = (uint16_t)calibration[2] | ((uint16_t)(calibration[5] & 0x03U) << 8);
  t1_c_x8 = (uint16_t)calibration[3] | ((uint16_t)(calibration[5] & 0x0CU) << 6);
  sensor->t0_c_x10 = (int16_t)((t0_c_x8 * 10U) / 8U);
  sensor->t1_c_x10 = (int16_t)((t1_c_x8 * 10U) / 8U);
  sensor->h0_out = make_s16(&calibration[6]);
  sensor->h1_out = make_s16(&calibration[10]);
  sensor->t0_out = make_s16(&calibration[12]);
  sensor->t1_out = make_s16(&calibration[14]);
  return 1U;
}

uint8_t HTS221_ReadValues(HTS221_Handle *sensor, int16_t *temperature_c_x10, uint16_t *humidity_rh)
{
  uint8_t raw[4];
  int16_t temperature_raw;
  int16_t humidity_raw;
  int32_t humidity_x2;
  if (!read_registers(sensor, 0x28U, raw, sizeof(raw))) return 0U;
  humidity_raw = make_s16(&raw[0]);
  temperature_raw = make_s16(&raw[2]);
  *temperature_c_x10 = sensor->t0_c_x10 +
      ((int32_t)(temperature_raw - sensor->t0_out) *
       (sensor->t1_c_x10 - sensor->t0_c_x10)) /
      (sensor->t1_out - sensor->t0_out);
  humidity_x2 = sensor->h0_rh_x2 +
      ((int32_t)(humidity_raw - sensor->h0_out) *
       (sensor->h1_rh_x2 - sensor->h0_rh_x2)) /
      (sensor->h1_out - sensor->h0_out);
  if (humidity_x2 < 0) humidity_x2 = 0;
  if (humidity_x2 > 200) humidity_x2 = 200;
  *humidity_rh = (uint16_t)(humidity_x2 / 2);
  return 1U;
}
