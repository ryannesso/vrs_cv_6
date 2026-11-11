#ifndef HTS221_H
#define HTS221_H

#include "sensor_bus.h"
#include <stdint.h>

#define HTS221_ADDRESS       0xBEU /* 7-bit address 0x5F, shifted for LL */
#define HTS221_WHO_AM_I      0x0FU
#define HTS221_WHO_AM_I_VAL  0xBCU
#define HTS221_CTRL_REG1     0x20U

typedef struct
{
  SensorReadCallback read;
  SensorWriteCallback write;
  int16_t t0_out;
  int16_t t1_out;
  int16_t h0_out;
  int16_t h1_out;
  int16_t t0_c_x10;
  int16_t t1_c_x10;
  int16_t h0_rh_x2;
  int16_t h1_rh_x2;
} HTS221_Handle;

void HTS221_RegisterBus(HTS221_Handle *sensor, SensorReadCallback read, SensorWriteCallback write);
uint8_t HTS221_Init(HTS221_Handle *sensor);
uint8_t HTS221_ReadValues(HTS221_Handle *sensor, int16_t *temperature_c_x10, uint16_t *humidity_rh);

#endif
