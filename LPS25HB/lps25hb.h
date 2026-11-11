#ifndef LPS25HB_H
#define LPS25HB_H

#include "sensor_bus.h"
#include <stdint.h>

#define LPS25HB_ADDRESS       0xBAU /* 7-bit address 0x5D, shifted for LL */
#define LPS25HB_WHO_AM_I      0x0FU
#define LPS25HB_WHO_AM_I_VAL  0xBDU
#define LPS25HB_CTRL_REG1     0x20U
#define LPS25HB_PRESS_OUT_XL   0x28U

typedef struct
{
  SensorReadCallback read;
  SensorWriteCallback write;
  int32_t reference_pressure_hpa_x100;
} LPS25HB_Handle;

void LPS25HB_RegisterBus(LPS25HB_Handle *sensor, SensorReadCallback read, SensorWriteCallback write);
uint8_t LPS25HB_Init(LPS25HB_Handle *sensor);
uint8_t LPS25HB_ReadPressure(LPS25HB_Handle *sensor, int32_t *pressure_hpa_x100);
int32_t LPS25HB_GetRelativeAltitudeCm(const LPS25HB_Handle *sensor, int32_t pressure_hpa_x100);

#endif
