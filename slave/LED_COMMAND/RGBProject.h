/*
 * RGBProject.h
 *
 *  Created on: 13 nov. 2015
 *      Author: Cédric
 */

#ifndef RGBPROJECT_H_
#define RGBPROJECT_H_

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
#endif /* RGBPROJECT_H_ */
