#include "motion_ctrl.h"

#include <stdlib.h>

#include "main.h"
#include "uart_parser.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;

volatile MotorState_t state_x = MOTOR_STOP;
volatile MotorState_t state_y = MOTOR_STOP;

void MotionCtrl_Init(void) {
  __disable_irq();
  state_x = MOTOR_STOP;
  state_y = MOTOR_STOP;
  __enable_irq();
}

void MotionCtrl_SetState_X(MotorState_t state) {
  if (state == state_x) return;
  state_x = state;
  __disable_irq();
  if (state == MOTOR_STOP) {
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
    HAL_GPIO_WritePin(MOTOR_EN1_GPIO_Port, MOTOR_EN1_Pin, GPIO_PIN_SET);
  } else {
    BSP_MotorX_SetDir((state == MOTOR_RUN_POS) ? 1U : 0U);
    HAL_GPIO_WritePin(MOTOR_EN1_GPIO_Port, MOTOR_EN1_Pin, GPIO_PIN_RESET);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  }
  __enable_irq();
}

void MotionCtrl_SetState_Y(MotorState_t state) {
  if (state == state_y) return;
  state_y = state;
  __disable_irq();
  if (state == MOTOR_STOP) {
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
    HAL_GPIO_WritePin(MOTOR_EN2_GPIO_Port, MOTOR_EN2_Pin, GPIO_PIN_SET);
  } else {
    BSP_MotorY_SetDir((state == MOTOR_RUN_POS) ? 1U : 0U);
    HAL_GPIO_WritePin(MOTOR_EN2_GPIO_Port, MOTOR_EN2_Pin, GPIO_PIN_RESET);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
  }
  __enable_irq();
}

void MotionCtrl_1msTask(void) {
}

void BSP_MotorX_SetDir(uint8_t dir_positive) {
  HAL_GPIO_WritePin(MOTOR_DIR1_GPIO_Port, MOTOR_DIR1_Pin,
                    (dir_positive != 0U) ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void BSP_MotorY_SetDir(uint8_t dir_positive) {
  HAL_GPIO_WritePin(MOTOR_DIR2_GPIO_Port, MOTOR_DIR2_Pin,
                    (dir_positive != 0U) ? GPIO_PIN_RESET : GPIO_PIN_SET);
}
