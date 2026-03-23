#ifndef UART_PARSER_H
#define UART_PARSER_H

#include <stdint.h>

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UART_PARSER_FRAME_MAX_LEN 48U
#define UART_PARSER_DMA_BUF_LEN 64U

typedef enum {
  UART_PARSER_MODE_IT = 0,
  UART_PARSER_MODE_DMA_IDLE = 1
} UartParserMode_t;

extern volatile int32_t target_x;
extern volatile int32_t target_y;

HAL_StatusTypeDef UartParser_Init(UART_HandleTypeDef* huart,
                                  UartParserMode_t mode);
void UartParser_Reset(void);
void UartParser_OnUartIrq(UART_HandleTypeDef* huart);
void UartParser_OnRxCpltCallback(UART_HandleTypeDef* huart);
void UartParser_OnErrorCallback(UART_HandleTypeDef* huart);
uint32_t UartParser_GetDropCount(void);
void UartParser_OnFrameReceived(void);

#ifdef __cplusplus
}
#endif

#endif
