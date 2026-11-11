#include "i2c.h"

static uint8_t I2C_WaitFor(uint32_t flag)
{
  uint32_t timeout = 1000000U;
  while ((READ_BIT(I2C1->ISR, flag) == 0U) && (--timeout != 0U))
  {
    if (LL_I2C_IsActiveFlag_NACK(I2C1) != 0U)
    {
      LL_I2C_ClearFlag_NACK(I2C1);
      return 0U;
    }
  }
  return timeout != 0U;
}

void MX_I2C1_Init(void)
{
  LL_I2C_InitTypeDef init = {0};
  LL_GPIO_InitTypeDef gpio = {0};

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  gpio.Pin = LL_GPIO_PIN_6 | LL_GPIO_PIN_7;
  gpio.Mode = LL_GPIO_MODE_ALTERNATE;
  gpio.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  gpio.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  gpio.Pull = LL_GPIO_PULL_UP;
  gpio.Alternate = LL_GPIO_AF_4;
  LL_GPIO_Init(GPIOB, &gpio);

  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);
  NVIC_SetPriority(I2C1_EV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
  NVIC_EnableIRQ(I2C1_EV_IRQn);

  init.PeripheralMode = LL_I2C_MODE_I2C;
  init.Timing = 0x2000090E;
  init.AnalogFilter = LL_I2C_ANALOGFILTER_ENABLE;
  init.DigitalFilter = 0U;
  init.OwnAddress1 = 1U;
  init.TypeAcknowledge = LL_I2C_ACK;
  init.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
  LL_I2C_Init(I2C1, &init);
  LL_I2C_Enable(I2C1);
}

uint8_t I2C_MasterWrite(uint8_t address, uint8_t reg, const uint8_t *data, uint16_t length)
{
  uint16_t i;
  LL_I2C_HandleTransfer(I2C1, address, LL_I2C_ADDRSLAVE_7BIT, length + 1U,
      LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);
  if (I2C_WaitFor(LL_I2C_ISR_TXIS) == 0U) return 0U;
  LL_I2C_TransmitData8(I2C1, reg);
  for (i = 0U; i < length; i++)
  {
    if (I2C_WaitFor(LL_I2C_ISR_TXIS) == 0U) return 0U;
    LL_I2C_TransmitData8(I2C1, data[i]);
  }
  if (I2C_WaitFor(LL_I2C_ISR_STOPF) == 0U) return 0U;
  LL_I2C_ClearFlag_STOP(I2C1);
  return 1U;
}

uint8_t I2C_MasterRead(uint8_t address, uint8_t reg, uint8_t *data, uint16_t length)
{
  uint16_t i;
  if (length == 0U) return 0U;

  LL_I2C_HandleTransfer(I2C1, address, LL_I2C_ADDRSLAVE_7BIT, 1U,
      LL_I2C_MODE_SOFTEND, LL_I2C_GENERATE_START_WRITE);
  if (I2C_WaitFor(LL_I2C_ISR_TXIS) == 0U) return 0U;
  LL_I2C_TransmitData8(I2C1, reg);
  if (I2C_WaitFor(LL_I2C_ISR_TC) == 0U) return 0U;

  LL_I2C_HandleTransfer(I2C1, address, LL_I2C_ADDRSLAVE_7BIT, length,
      LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_READ);
  for (i = 0U; i < length; i++)
  {
    if (I2C_WaitFor(LL_I2C_ISR_RXNE) == 0U) return 0U;
    data[i] = LL_I2C_ReceiveData8(I2C1);
  }
  if (I2C_WaitFor(LL_I2C_ISR_STOPF) == 0U) return 0U;
  LL_I2C_ClearFlag_STOP(I2C1);
  return 1U;
}

void I2C1_EV_IRQHandler(void)
{
  /* Transfers are completed synchronously by I2C_MasterRead/Write. */
}
