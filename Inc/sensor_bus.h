#ifndef SENSOR_BUS_H
#define SENSOR_BUS_H

#include <stdint.h>

typedef uint8_t (*SensorReadCallback)(uint8_t address, uint8_t reg, uint8_t *data, uint16_t length);
typedef uint8_t (*SensorWriteCallback)(uint8_t address, uint8_t reg, const uint8_t *data, uint16_t length);

#endif
