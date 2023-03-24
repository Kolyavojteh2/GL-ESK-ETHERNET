/*
 * Parser.h
 *
 *  Created on: Mar 24, 2023
 *      Author: kolya
 */

#ifndef INC_PARSER_H_
#define INC_PARSER_H_

#include "LED_driver.h"

LED_Operation_t Parser_parseLedOperation(char * const str);

int Parser_makeAnswer(const LED_Operation_t operation, char * const answer_buffer, const unsigned int buffer_size);

#endif /* INC_PARSER_H_ */
