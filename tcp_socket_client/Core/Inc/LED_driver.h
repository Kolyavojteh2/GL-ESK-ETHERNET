/*
 * LED_driver.h
 *
 *  Created on: Mar 24, 2023
 *      Author: kolya
 */

#ifndef INC_LED_DRIVER_H_
#define INC_LED_DRIVER_H_

#include "main.h"

typedef enum {
	LED_Action_None = 0,
	LED_Action_Off,
	LED_Action_On,
	LED_Action_Toggle,
	LED_Action_CheckState,
	LED_Action_Version
} LED_Action;

typedef struct {
	Led_TypeDef number;
	LED_Action action;
} LED_Operation_t;

void LED_driver_runLedOperation(const LED_Operation_t operation);

#endif /* INC_LED_DRIVER_H_ */
