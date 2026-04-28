#include "motion_ctrl.h"

#include <stdlib.h>

#include "main.h"
#include "uart_parser.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;

volatile int32_t current_x = 0;
volatile int32_t current_y = 0;
volatile int32_t step_limit = 5;

static int32_t MotionCtrl_Approach(int32_t current, int32_t target,
                                   int32_t limit) {
  int32_t delta = target - current;

  if (delta > limit) {
    delta = limit;
  } else if (delta < -limit) {
    delta = -limit;
  }

  return current + delta;
}

void MotionCtrl_Init(void) {
  __disable_irq();
  current_x = 0;
  current_y = 0;
  step_limit = 5;
  __enable_irq();
}

void MotionCtrl_SetStepLimit(int32_t limit) {
  if (limit <= 0) {
    limit = 1;
  }

  step_limit = limit;
}

void MotionCtrl_GetTargetSnapshot(int32_t* x, int32_t* y) {
  if ((x == NULL) || (y == NULL)) {
    return;
  }

  __disable_irq();
  *x = target_x;
  *y = target_y;
  __enable_irq();
}

void MotionCtrl_1msTask(void) {
  int32_t tx;
  int32_t ty;
  int32_t next_x;
  int32_t next_y;
  int32_t dx;
  int32_t dy;
  int32_t lim;

  MotionCtrl_GetTargetSnapshot(&tx, &ty);
  lim = step_limit;

  next_x = MotionCtrl_Approach(current_x, tx, lim);
  next_y = MotionCtrl_Approach(current_y, ty, lim);

  dx = next_x - current_x;
  dy = next_y - current_y;

  if (dx != 0) {
    BSP_MotorX_SetDir((dx > 0) ? 1U : 0U);
    BSP_MotorX_Pulse((uint16_t)abs(dx));
  }

  if (dy != 0) {
    BSP_MotorY_SetDir((dy > 0) ? 1U : 0U);
    BSP_MotorY_Pulse((uint16_t)abs(dy));
  }

  current_x = next_x;
  current_y = next_y;
}

__weak void BSP_MotorX_SetDir(uint8_t dir_positive) {
  HAL_GPIO_WritePin(MOTOR_DIR1_GPIO_Port, MOTOR_DIR1_Pin,
                    (dir_positive != 0U) ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

static void MotionCtrl_PulseByTimer(TIM_HandleTypeDef* htim, uint32_t channel,
                                    GPIO_TypeDef* en_port, uint16_t en_pin,
                                    uint16_t steps) {
  uint16_t i;

  if ((htim == NULL) || (steps == 0U)) {
    return;
  }

  HAL_GPIO_WritePin(en_port, en_pin, GPIO_PIN_RESET);
  if (HAL_TIM_PWM_Start(htim, channel) != HAL_OK) {
    return;
  }

  for (i = 0U; i < steps; i++) {
    uint32_t t0;

    __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_UPDATE);
    t0 = HAL_GetTick();
    while (__HAL_TIM_GET_FLAG(htim, TIM_FLAG_UPDATE) == RESET) {
      if ((HAL_GetTick() - t0) > 20U) {
        break;
      }
    }
  }

  (void)HAL_TIM_PWM_Stop(htim, channel);
}

__weak void BSP_MotorX_Pulse(uint16_t steps) {
  MotionCtrl_PulseByTimer(&htim3, TIM_CHANNEL_1, MOTOR_EN1_GPIO_Port,
                          MOTOR_EN1_Pin, steps);
}

__weak void BSP_MotorY_SetDir(uint8_t dir_positive) {
  HAL_GPIO_WritePin(MOTOR_DIR2_GPIO_Port, MOTOR_DIR2_Pin,
                    (dir_positive != 0U) ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

__weak void BSP_MotorY_Pulse(uint16_t steps) {
  MotionCtrl_PulseByTimer(&htim1, TIM_CHANNEL_3, MOTOR_EN2_GPIO_Port,
                          MOTOR_EN2_Pin, steps);
}
