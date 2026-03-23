#ifndef MOTION_CTRL_H
#define MOTION_CTRL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern volatile int32_t current_x;
extern volatile int32_t current_y;
extern volatile int32_t step_limit;

void MotionCtrl_Init(void);
void MotionCtrl_SetStepLimit(int32_t limit);
void MotionCtrl_GetTargetSnapshot(int32_t* x, int32_t* y);
void MotionCtrl_1msTask(void);

void BSP_MotorX_SetDir(uint8_t dir_positive);
void BSP_MotorY_SetDir(uint8_t dir_positive);
void BSP_MotorX_Pulse(uint16_t steps);
void BSP_MotorY_Pulse(uint16_t steps);

#ifdef __cplusplus
}
#endif

#endif
