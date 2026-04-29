#include "uart_parser.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

volatile int32_t target_x = 0;
volatile int32_t target_y = 0;

static UART_HandleTypeDef* s_huart = NULL;
static UartParserMode_t s_mode = UART_PARSER_MODE_IT;
static volatile uint32_t s_drop_count = 0;

static char s_frame_buf[UART_PARSER_FRAME_MAX_LEN];
static uint16_t s_frame_len = 0;
static bool s_frame_overflow = false;

static uint8_t s_it_rx_byte = 0;

static uint8_t s_dma_rx_buf[UART_PARSER_DMA_BUF_LEN];
static uint16_t s_dma_last_pos = 0;

static void Parser_ResetFrame(void) {
  s_frame_len = 0;
  s_frame_overflow = false;
}

static bool Parser_ParseFrame(const char* frame, int32_t* out_x,
                              int32_t* out_y) {
  char temp[UART_PARSER_FRAME_MAX_LEN];
  char* comma = NULL;
  char* end_x = NULL;
  char* end_y = NULL;
  long x = 0;
  long y = 0;
  size_t len = strlen(frame);

  if (len < 5U) {
    return false;
  }

  if ((frame[0] != '<') || (frame[len - 1U] != '>')) {
    return false;
  }

  if (len - 2U >= sizeof(temp)) {
    return false;
  }

  memcpy(temp, &frame[1], len - 2U);
  temp[len - 2U] = '\0';

  comma = strchr(temp, ',');
  if ((comma == NULL) || (comma == temp) || (*(comma + 1) == '\0')) {
    return false;
  }

  *comma = '\0';

  if (temp[0] == 'L') {
    long cmd = strtol(comma + 1, &end_y, 10);
    if (*end_y == '\0') {
      extern void App_StartLaserCmd(uint8_t cmd);
      App_StartLaserCmd((uint8_t)cmd);
      return false; // MUST return false so it doesn't zero out target_x and target_y!
    }
  }

  x = strtol(temp, &end_x, 10);
  y = strtol(comma + 1, &end_y, 10);

  if ((*end_x != '\0') || (*end_y != '\0')) {
    return false;
  }

  *out_x = (int32_t)x;
  *out_y = (int32_t)y;
  return true;
}

static void Parser_TryCommitFrame(void) {
  int32_t x = 0;
  int32_t y = 0;

  if ((s_frame_len > 0U) || s_frame_overflow) {
    UartParser_OnFrameReceived();
  }

  if (s_frame_overflow || (s_frame_len == 0U)) {
    Parser_ResetFrame();
    return;
  }

  s_frame_buf[s_frame_len] = '\0';
  if (Parser_ParseFrame(s_frame_buf, &x, &y)) {
    target_x = x;
    target_y = y;
  } else {
    s_drop_count++;
  }

  Parser_ResetFrame();
}

static void Parser_PushByte(uint8_t byte) {
  if (byte == '\r') {
    return;
  }

  if (byte == '\n') {
    Parser_TryCommitFrame();
    return;
  }

  if (s_frame_overflow) {
    return;
  }

  if (s_frame_len >= (UART_PARSER_FRAME_MAX_LEN - 1U)) {
    s_frame_overflow = true;
    s_drop_count++;
    return;
  }

  s_frame_buf[s_frame_len++] = (char)byte;
}

static void Parser_DmaConsume(void) {
  uint16_t pos;
  uint16_t i;

  if ((s_huart == NULL) || (s_huart->hdmarx == NULL)) {
    return;
  }

  pos = (uint16_t)(UART_PARSER_DMA_BUF_LEN -
                   __HAL_DMA_GET_COUNTER(s_huart->hdmarx));

  if (pos == s_dma_last_pos) {
    return;
  }

  if (pos > s_dma_last_pos) {
    for (i = s_dma_last_pos; i < pos; i++) {
      Parser_PushByte(s_dma_rx_buf[i]);
    }
  } else {
    for (i = s_dma_last_pos; i < UART_PARSER_DMA_BUF_LEN; i++) {
      Parser_PushByte(s_dma_rx_buf[i]);
    }
    for (i = 0; i < pos; i++) {
      Parser_PushByte(s_dma_rx_buf[i]);
    }
  }

  s_dma_last_pos = pos;
}

HAL_StatusTypeDef UartParser_Init(UART_HandleTypeDef* huart,
                                  UartParserMode_t mode) {
  if (huart == NULL) {
    return HAL_ERROR;
  }

  s_huart = huart;
  s_mode = mode;
  UartParser_Reset();

  if (s_mode == UART_PARSER_MODE_IT) {
    return HAL_UART_Receive_IT(s_huart, &s_it_rx_byte, 1U);
  }

  if ((s_mode == UART_PARSER_MODE_DMA_IDLE) && (s_huart->hdmarx != NULL)) {
    s_dma_last_pos = 0;
    if (HAL_UART_Receive_DMA(s_huart, s_dma_rx_buf, UART_PARSER_DMA_BUF_LEN) !=
        HAL_OK) {
      return HAL_ERROR;
    }

    __HAL_DMA_DISABLE_IT(s_huart->hdmarx, DMA_IT_HT);
    __HAL_UART_ENABLE_IT(s_huart, UART_IT_IDLE);
    return HAL_OK;
  }

  return HAL_ERROR;
}

void UartParser_Reset(void) {
  __disable_irq();
  Parser_ResetFrame();
  s_drop_count = 0;
  target_x = 0;
  target_y = 0;
  __enable_irq();
}

void UartParser_OnUartIrq(UART_HandleTypeDef* huart) {
  if ((huart != s_huart) || (s_mode != UART_PARSER_MODE_DMA_IDLE)) {
    return;
  }

  if ((__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) != RESET) &&
      (__HAL_UART_GET_IT_SOURCE(huart, UART_IT_IDLE) != RESET)) {
    __HAL_UART_CLEAR_IDLEFLAG(huart);
    Parser_DmaConsume();
  }
}

void UartParser_OnRxCpltCallback(UART_HandleTypeDef* huart) {
  if ((huart != s_huart) || (s_mode != UART_PARSER_MODE_IT)) {
    return;
  }

  Parser_PushByte(s_it_rx_byte);
  if (HAL_UART_Receive_IT(s_huart, &s_it_rx_byte, 1U) != HAL_OK) {
    s_drop_count++;
  }
}

void UartParser_OnErrorCallback(UART_HandleTypeDef* huart) {
  if (huart != s_huart) {
    return;
  }

  s_drop_count++;

  if (s_mode == UART_PARSER_MODE_IT) {
    (void)HAL_UART_Receive_IT(s_huart, &s_it_rx_byte, 1U);
  } else if (s_mode == UART_PARSER_MODE_DMA_IDLE) {
    if (s_huart->hdmarx != NULL) {
      (void)HAL_UART_DMAStop(s_huart);
      (void)HAL_UART_Receive_DMA(s_huart, s_dma_rx_buf,
                                 UART_PARSER_DMA_BUF_LEN);
      __HAL_DMA_DISABLE_IT(s_huart->hdmarx, DMA_IT_HT);
      __HAL_UART_ENABLE_IT(s_huart, UART_IT_IDLE);
      s_dma_last_pos = 0;
    }
  }
}

uint32_t UartParser_GetDropCount(void) { return s_drop_count; }

__weak void UartParser_OnFrameReceived(void) {}
