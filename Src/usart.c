#include "usart.h"

static void USART2_SendByte(uint8_t value)
{
  while (LL_USART_IsActiveFlag_TXE(USART2) == 0U) {}
  LL_USART_TransmitData8(USART2, value);
}

void MX_USART2_UART_Init(void)
{
  LL_USART_InitTypeDef usart = {0};
  LL_GPIO_InitTypeDef gpio = {0};
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

  gpio.Pin = LL_GPIO_PIN_2;
  gpio.Mode = LL_GPIO_MODE_ALTERNATE;
  gpio.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  gpio.Pull = LL_GPIO_PULL_UP;
  gpio.Alternate = LL_GPIO_AF_7;
  LL_GPIO_Init(GPIOA, &gpio);

  usart.BaudRate = 115200U;
  usart.DataWidth = LL_USART_DATAWIDTH_8B;
  usart.StopBits = LL_USART_STOPBITS_1;
  usart.Parity = LL_USART_PARITY_NONE;
  usart.TransferDirection = LL_USART_DIRECTION_TX;
  usart.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  usart.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART2, &usart);
  LL_USART_ConfigAsyncMode(USART2);
  LL_USART_Enable(USART2);
}

void USART2_Write(const uint8_t *data, uint16_t length)
{
  const uint8_t *end = data + length;

  while (data < end)
  {
    USART2_SendByte(*data);
    data++;
  }

  while (LL_USART_IsActiveFlag_TC(USART2) == 0U) {}
}

void USART2_WriteString(const char *text)
{
  const char *cursor = text;

  while (*cursor != '\0')
  {
    cursor++;
  }

  USART2_Write((const uint8_t *)text, (uint16_t)(cursor - text));
}
