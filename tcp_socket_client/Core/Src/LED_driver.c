/*
 * LED_driver.c
 *
 *  Created on: Mar 24, 2023
 *      Author: kolya
 */

#include "LED_driver.h"

void LED_driver_runLedOperation(const LED_Operation_t operation) {
	if (operation.action == LED_Action_None || operation.action == LED_Action_Version || operation.action == LED_Action_CheckState) {
		return;
	}

	if (operation.action == LED_Action_Off) {
		BSP_LED_Off(operation.number);
		return;
	}

	if (operation.action == LED_Action_On) {
		BSP_LED_On(operation.number);
		return;
	}

	if (operation.action == LED_Action_Toggle) {
		BSP_LED_Toggle(operation.number);
		return;
	}
}
