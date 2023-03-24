/*
 * Parser.c
 *
 *  Created on: Mar 24, 2023
 *      Author: kolya
 */

/**
 * @brief This function parses the string and returns LED_Operation.
 *
 * @param str the string that consists an operation
 * @param str_size the size of input string
 *
 * @retval a struct LED_Operation. Check first the action field.
 */

#include "Parser.h"

#include <string.h>
#include <ctype.h>

#define UDP_ARGUMENT_DELIM " \n"

#define UDP_COMMAND_SVERSION "SVERSION"

#define UDP_COMMAND_LED "LED"
#define UDP_COMMAND_OFF "OFF"
#define UDP_COMMAND_ON "ON"
#define UDP_COMMAND_TOGGLE "TOGGLE"
#define UDP_COMMAND_STATUS "STATUS"

#define UDP_ANSWER_SVERSION "udp_srv_mykola_vojtekh_21032023\r\n"
#define UDP_ANSWER_ERROR "ERROR\r\n"
#define UDP_ANSWER_OK "OK\r\n"

static void toUpperString(char * const str) {
	for (unsigned int index = 0; index < strlen(str); index++) {
		str[index] = toupper(str[index]);
	}
}

LED_Operation_t Parser_parseLedOperation(char * const str) {
	LED_Operation_t result = { 0 };

	toUpperString(str);

	char *next_token = NULL;
	char *token = NULL;

	// Get first argument
	token = strtok_r(str, UDP_ARGUMENT_DELIM, &next_token);
	if (token == NULL) {
		// Make ERROR answer
		result.action = LED_Action_None;
		return result;
	}

	// Check "sversion"
	if (strncmp(token, UDP_COMMAND_SVERSION, strlen(UDP_COMMAND_SVERSION)) == 0) {
		// Check other arguments
		token = strtok_r(NULL, UDP_ARGUMENT_DELIM, &next_token);
		if (token != NULL) {
			// Make ERROR answer
			result.action = LED_Action_None;
			return result;
		}

		// This action is check "version"
		result.action = LED_Action_Version;
		return result;
	}

	// Check "led"
	if (strncmp(token, UDP_COMMAND_LED, strlen(UDP_COMMAND_LED)) == 0){
		// Check length argument. It must be the template length + 1. (4)
		if (strlen(token) != (strlen(UDP_COMMAND_LED) + 1)) {
			// Make ERROR answer
			result.action = LED_Action_None;
			return result;
		}

		// Define led number
		switch (token[3]) {
		case '3':
			result.number = LED3;
			break;
		case '4':
			result.number = LED4;
			break;
		case '5':
			result.number = LED5;
			break;
		case '6':
			result.number = LED6;
			break;

		default:
			// Make ERROR answer
			result.action = LED_Action_None;
			return result;
		}

		// Define operation for specific LED
		// Get and check next token
		token = strtok_r(NULL, UDP_ARGUMENT_DELIM, &next_token);
		if (token == NULL) {
			// Make ERROR answer
			result.action = LED_Action_None;
			return result;
		}

		// Check "off"
		if (strncmp(token, UDP_COMMAND_OFF, strlen(UDP_COMMAND_OFF)) == 0) {
			result.action = LED_Action_Off;
		}

		// Check "on"
		if (strncmp(token, UDP_COMMAND_ON, strlen(UDP_COMMAND_ON)) == 0) {
			result.action = LED_Action_On;
		}

		// Check "toggle"
		if (strncmp(token, UDP_COMMAND_TOGGLE, strlen(UDP_COMMAND_TOGGLE)) == 0) {
			result.action = LED_Action_Toggle;
		}

		// Check "status"
		if (strncmp(token, UDP_COMMAND_STATUS, strlen(UDP_COMMAND_STATUS)) == 0) {
			result.action = LED_Action_CheckState;
		}

		// Check other arguments
		token = strtok_r(NULL, UDP_ARGUMENT_DELIM, &next_token);
		if (token != NULL) {
			// Make ERROR answer
			result.action = LED_Action_None;
			return result;
		}
	}

	return result;
}

int Parser_makeAnswer(const LED_Operation_t operation, char * const answer_buffer, const unsigned int buffer_size) {
	if (operation.action == LED_Action_Version) {
		/*
		 * There is using strlen(UDP_ANSWER_SVERSION) to copy the string without '\0'.
		 * Using sizeof(UDP_ANSWER_ERROR) to copy the string with '\0'
		 */
		unsigned int result_size = strlen(UDP_ANSWER_SVERSION) + sizeof(UDP_ANSWER_OK);
		if (buffer_size < result_size) {
			return -1;
		}

		unsigned int index_offset = 0;

		memcpy(answer_buffer, UDP_ANSWER_SVERSION, strlen(UDP_ANSWER_SVERSION));
		index_offset += strlen(UDP_ANSWER_SVERSION);

		memcpy(answer_buffer + index_offset, UDP_ANSWER_OK, sizeof(UDP_ANSWER_OK));
		return 0;
	}

	if (operation.action == LED_Action_None) {
		if (buffer_size < sizeof(UDP_ANSWER_ERROR)) {
			return -1;
		}

		memcpy(answer_buffer, UDP_ANSWER_ERROR, sizeof(UDP_ANSWER_ERROR)); // sizeof() because '\0' must be copied to the answer
		return 0;
	}

	if (operation.action == LED_Action_CheckState) {
		GPIO_PinState led_state = BSP_LED_ReadState(operation.number);

		const char* led_state_str = NULL;
		switch (led_state) {
		case GPIO_PIN_RESET:
			led_state_str = UDP_COMMAND_OFF;
			break;

		case GPIO_PIN_SET:
			led_state_str = UDP_COMMAND_ON;
			break;

		default:
			break;
		}

		// Define led number
		char led_number;
		switch (operation.number) {
		case LED3:
			led_number = '3';
			break;
		case LED4:
			led_number = '4';
			break;
		case LED5:
			led_number = '5';
			break;
		case LED6:
			led_number = '6';
			break;
		}
		unsigned int result_size = strlen(UDP_COMMAND_LED) +
				1 /* N, where N = {'3', '4', '5', '6'} */ +
				1 /* symbol ' ' */ +
				strlen(led_state_str) +
				1 /* symbol '\n'*/ +
				sizeof(UDP_ANSWER_OK);

		if (buffer_size < result_size) {
			return -1;
		}

		unsigned int index_offset = 0;

		// Copy "LED"
		memcpy(answer_buffer + index_offset, UDP_COMMAND_LED, strlen(UDP_COMMAND_LED));
		index_offset += strlen(UDP_COMMAND_LED);

		// Copy N, where N = {'3', '4', '5', '6'}
		answer_buffer[index_offset] = led_number;
		index_offset++;

		// Copy ' '
		answer_buffer[index_offset] = ' ';
		index_offset++;

		// Copy the state, where state = {"ON", "OFF"}
		memcpy(answer_buffer + index_offset, led_state_str, strlen(led_state_str));
		index_offset += strlen(led_state_str);

		// Copy "\r\n"
		memcpy(answer_buffer + index_offset, "\r\n", strlen("\r\n"));

		return 0;
	}

	// Others commands(LED_ON, LED_OFF, LED_Toggle makes answer "OK"
	if (buffer_size < sizeof(UDP_ANSWER_OK)) {
		return -1;
	}

	// Copy "OK"
	memcpy(answer_buffer, UDP_ANSWER_OK, strlen(UDP_ANSWER_OK));

	return 0;
}

