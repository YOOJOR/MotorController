#ifndef MOTION_CTRL_H
#define MOTION_CTRL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  MOTOR_STOP = 0,
  MOTOR_RUN_POS,
  MOTOR_RUN_NEG
} MotorState_t;

extern volatile MotorState_t state_x;
extern volatile MotorState_t state_y;

void MotionCtrl_Init(void);
void MotionCtrl_SetState_X(MotorState_t state);
void MotionCtrl_SetState_Y(MotorState_t state);
void MotionCtrl_1msTask(void);
void MotionCtrl_TimIrq_X(void);
void MotionCtrl_TimIrq_Y(void);

void BSP_MotorX_SetDir(uint8_t dir_positive);
void BSP_MotorY_SetDir(uint8_t dir_positive);

#ifdef __cplusplus
}
#endif

#endif
