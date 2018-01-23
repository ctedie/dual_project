/*
 * test_link.c
 *
 *  Created on: 15 janv. 2018
 *      Author: Dev
 */


#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "serial_link.h"
#include "serial_link_frame_protocol.h"

#define NB_MAX_MSG	3


struct message
{
	bool isUsed;
	uint8_t msg[50];
};

struct message m_arrMsg[NB_MAX_MSG] = {{.isUsed = false},{.isUsed = false},{.isUsed = false}};

static uint8_t m_channelNumber = 0;

static void Reception_complete(void *pData, uint8_t *pMsg, uint16_t size);
static uint8_t* Alloc(void);
static void Free(void *pMsg);

bool Test_link_init(void)
{
	m_channelNumber = SerialLinkFrameProtocoleInit(SERIAL1,
												   B115200,
												   BIT_8,
												   PARITY_NONE,
												   STOP_BIT_1,
												   Reception_complete,
												   &m_channelNumber,
												   Alloc,
												   Free
												   );

	uint8_t *pMsg;
	pMsg = Alloc();
	pMsg[12] = 0xCE;
	pMsg[13] = 0xD0;

	return true;
}


static void Reception_complete(void *pData, uint8_t *pMsg, uint16_t size)
{

}

static uint8_t* Alloc(void)
{
	uint8_t i;

	for (i = 0; i < NB_MAX_MSG; ++i)
	{
		if(m_arrMsg[i].isUsed == false)
		{
			m_arrMsg[i].isUsed = true;
			return m_arrMsg[i].msg;
		}

	}
	return NULL;
}

static void Free(void *pMsg)
{
	uint8_t i;

	for (i = 0; i < NB_MAX_MSG; ++i)
	{
		if(m_arrMsg[i].msg == pMsg)
		{
			m_arrMsg[i].isUsed = false;
		}
	}
}
