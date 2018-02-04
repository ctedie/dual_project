/*
 * mntFrame.h
 *
 *  Created on: 8 août 2017
 *      Author: Dev
 */

#ifndef MNTFRAME_H_
#define MNTFRAME_H_

#include <stdint.h>
#include <stdbool.h>


typedef struct
{
	uint32_t isParted:2;
	uint32_t partNumber:15;
	uint32_t totalPart:15;
}MNTFrameParams_t;

typedef struct
{
	uint8_t dstAddress;
	uint8_t srcAddress;
	MNTFrameParams_t frameParams;
	uint8_t command;
	uint16_t dataSize;
	uint8_t data[1024];

}__attribute__((packed)) MNTFrame_t;

enum eLED_COMMANDS
{
	LED_CONTROL = 1,
	FILE_TRANSFER,
	RGB_CONTROL,
	TEST
};

/*--------------------- LED Control Frame ------------------------*/

typedef struct
{
	uint8_t state;
}__attribute__((packed)) NormalControl_t;


typedef struct
{
	uint16_t frequency;
	uint8_t dutyCycle;
}__attribute__((packed)) PWMControl_t;

typedef union
{
	NormalControl_t normal;
	PWMControl_t pwm;
}LED_Control_type_t;

typedef struct
{
	uint8_t ledNumber;
	uint8_t ledControlType;
	LED_Control_type_t type;


}__attribute__((packed))LED_Control_Frame_t;

void MNTProcess(uint8_t* frame);

#endif /* MNTFRAME_H_ */
