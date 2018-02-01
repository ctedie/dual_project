/*
 * RGBProject.h
 *
 *  Created on: 13 nov. 2015
 *      Author: Cédric
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "common.h"
//typedef struct
//{
//	uint8_t red;
//	uint8_t green;
//	uint8_t blue;
//
//};

typedef union
{
	uint32_t value;
	struct
	{
		uint8_t blue;
		uint8_t green;
		uint8_t red;

	}color;
}T_COLOR;


void RGB_SetColor(tRGBControl* color);

#endif /* MAIN_H_ */
