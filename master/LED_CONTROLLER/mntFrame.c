/*
 * mntFrame.c
 *
 *  Created on: 8 août 2017
 *      Author: Dev
 */

#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"

#include "driverlib/gpio.h"

#include "common.h"
#include "led_control_module.h"
#include "mntFrame.h"


static void localLedControl(LED_Control_Frame_t *pFrame);
static void test(void *pData);



static uint8_t frameOK = 0;
static uint16_t* me;

void MNTProcess(uint8_t* pdata)
{
	MNTFrame_t* frame = (MNTFrame_t*)pdata;

	if(frame->dstAddress == 2)
	{
		frameOK++;
		me = (uint16_t*)&frame->data[0];

		switch (frame->command)
		{
			case LED_CONTROL:
				if(frame->dataSize == sizeof(LED_Control_Frame_t))
				{
					localLedControl((LED_Control_Frame_t*)&frame->data[0]);
				}
				else
				{
					//TODO Error management
				}
				break;
			case FILE_TRANSFER:
				break;
			case RGB_CONTROL:
				LED_CONTROL_RgbSend((tRGBControl*)&frame->data[0]);
				break;
			case TEST:
				test(&frame->data[0]);
				break;
			default:
				break;
		}
	}


}


static void localLedControl(LED_Control_Frame_t *pFrame)
{
	uint8_t pin[2] = {GPIO_PIN_1, GPIO_PIN_0};
	uint8_t state[2] = { 0, 0xFF};

	/* Normal Control Test */
	GPIOPinWrite(GPIO_PORTN_BASE, pin[pFrame->ledNumber], state[pFrame->type.normal.state]);
}

static uint32_t m_testCount = 0;

static void test(void *pData)
{
	uint16_t *donnees = (uint16_t*)pData;

}
